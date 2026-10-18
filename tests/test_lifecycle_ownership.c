/* Real-engine ownership teardown; no live players or production world. */
#include <assert.h>
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include "merc.h"

extern char *string_space, *top_string;
extern bool fBootDb;
extern void init_mm(void);
extern NOTE_DATA *new_note(void);
static ROOM_INDEX_DATA home, altar, disconnect_room;
static AREA_DATA area;
static OBJ_INDEX_DATA object_index;
static MOB_INDEX_DATA mobile_index;

static char *pooled(const char *text)
{
    FILE *fp = tmpfile();
    assert(fp != NULL);
    fprintf(fp,"%s~",text);
    rewind(fp);
    fBootDb = TRUE;
    char *result = fread_string(fp);
    fBootDb = FALSE;
    assert(fclose(fp) == 0);
    return result;
}

static int live(CHAR_DATA *target)
{
    for (CHAR_DATA *ch = char_list; ch != NULL; ch = ch->next)
        if (ch == target) return 1;
    return 0;
}

static CHAR_DATA *player(char *name)
{
    DESCRIPTOR_DATA shell = {0};
    load_char_obj(&shell,name);
    CHAR_DATA *ch = shell.character;
    assert(ch != NULL && ch->pcdata != NULL);
    ch->desc = NULL;
    ch->level = LEV_AWA;
    ch->next = char_list;
    char_list = ch;
    char_to_room(ch,&home);
    return ch;
}

static CHAR_DATA *mobile(void)
{
    CHAR_DATA *ch = create_mobile(&mobile_index);
    char_to_room(ch,&home);
    return ch;
}

static OBJ_DATA *object(CHAR_DATA *owner)
{
    OBJ_DATA *obj = create_object(&object_index,0);
    obj_to_room(obj,&home);
    obj->chobj = owner;
    return obj;
}

static void cleanup(void)
{
    while (char_list != NULL) extract_char(char_list,TRUE);
    while (object_list != NULL) extract_obj(object_list);
    assert(descriptor_list == NULL && mobile_index.count == 0 &&
           object_index.count == 0);
}

static int connect_player(CHAR_DATA *ch)
{
    int sockets[2];
    assert(socketpair(AF_UNIX,SOCK_STREAM,0,sockets) == 0);
    DESCRIPTOR_DATA *d = alloc_perm(sizeof(*d));
    memset(d,0,sizeof(*d));
    d->descriptor = sockets[0];
    d->outsize = 2000;
    d->outbuf = alloc_mem(d->outsize);
    d->host = str_dup("private-test");
    d->connected = CON_PLAYING;
    d->character = ch;
    d->next = descriptor_list;
    descriptor_list = d;
    ch->desc = d;
    return sockets[1];
}

static void objects(void)
{
    CHAR_DATA *owner = player("Owner");
    OBJ_DATA *old = object(owner), *current = object(owner);
    owner->pcdata->chobj = current;
    SET_BIT(owner->extra,EXTRA_OSWITCH);
    SET_BIT(owner->affected_by,AFF_POLYMORPH);
    extract_obj(old);
    assert(owner->pcdata->chobj == current && current->chobj == owner);
    extract_obj(current);
    assert(owner->pcdata->chobj == NULL && owner->in_room == &altar &&
           !IS_EXTRA(owner,EXTRA_OSWITCH) && !IS_AFFECTED(owner,AFF_POLYMORPH));
    CHAR_DATA *other = player("Other");
    current = object(other);
    other->pcdata->chobj = current;
    owner->pcdata->chobj = current; /* Obsolete outward reference. */
    CHAR_DATA *pet = mobile();
    pet->wizard = owner;
    owner->pcdata->familiar = pet;
    extract_char(owner,FALSE);
    assert(owner->pcdata->chobj == NULL && current->chobj == other &&
           other->pcdata->chobj == current && live(pet) &&
           owner->pcdata->familiar == pet && pet->wizard == owner);
    owner->pcdata->chobj = current;
    old = object(owner); /* Stray inward reference. */
    extract_char(owner,TRUE);
    assert(current->chobj == other && other->pcdata->chobj == current &&
           old->chobj == NULL);
    CHAR_DATA unregistered = {0};
    extract_obj(object(&unregistered)); /* Never dereference a non-live link. */
    cleanup();
}

static void familiars(void)
{
    CHAR_DATA *owner = player("Owner");
    CHAR_DATA *old = mobile(), *current = mobile();
    old->wizard = current->wizard = owner;
    owner->pcdata->familiar = current;
    extract_char(old,TRUE);
    assert(owner->pcdata->familiar == current && current->wizard == owner);
    extract_char(current,TRUE);
    assert(owner->pcdata->familiar == NULL);
    CHAR_DATA *other = player("Other");
    current = mobile();
    current->wizard = other;
    other->pcdata->familiar = current;
    CHAR_DATA *third = player("Third");
    owner->pcdata->propose = other;
    other->pcdata->propose = third; third->pcdata->propose = other;
    owner->embrace = other;
    other->embrace = third; third->embrace = other;
    other->embraced = third->embraced = 1;
    owner->pcdata->familiar = current; /* No longer owns this familiar. */
    extract_char(owner,TRUE);
    assert(live(current) && current->wizard == other &&
           other->pcdata->familiar == current &&
           other->pcdata->propose == third && third->pcdata->propose == other &&
           other->embrace == third && third->embrace == other &&
           other->embraced == 1);
    old = mobile();
    old->wizard = other; /* No outward reference: still must not dangle. */
    extract_char(other,TRUE);
    assert(!live(current) && live(old) && old->wizard == NULL);
    cleanup();
}

static void heads(void)
{
    CHAR_DATA *owner = player("Owner");
    OBJ_DATA *head = object(owner);
    owner->pcdata->chobj = head;
    SET_BIT(owner->more,MORE_HEAD);
    SET_BIT(owner->affected_by,AFF_POLYMORPH);
    ROOM_INDEX_DATA **slot = &room_index_hash[altar.vnum % MAX_KEY_HASH];
    ROOM_INDEX_DATA *saved = *slot;
    assert(saved == &altar);
    *slot = altar.next;
    extract_obj(head);
    assert(owner->in_room == &home && owner->pcdata->chobj == NULL &&
           owner->hit == 1 && owner->position == POS_RESTING &&
           !IS_MORE(owner,MORE_HEAD) && !IS_AFFECTED(owner,AFF_POLYMORPH));
    *slot = saved;
    cleanup();
}

static void switching(void)
{
    CHAR_DATA *owner = player("Owner"), *body = mobile();
    int peer = connect_player(owner);
    DESCRIPTOR_DATA *d = owner->desc;
    do_switch(owner,"beast");
    assert(d->original == owner && d->character == body && owner->desc == NULL);
    extract_char(body,TRUE);
    assert(d->original == NULL && d->character == owner && owner->desc == d);
    body = mobile();
    do_switch(owner,"beast");
    extract_char(owner,TRUE);
    assert(descriptor_list == NULL && d->original == NULL &&
           d->character == NULL && body->desc == NULL);
    assert(close(peer) == 0);
    cleanup();

    owner = player("Disconnected"); body = mobile();
    owner->pcdata->familiar = body; body->wizard = owner;
    peer = connect_player(owner); d = owner->desc;
    do_switch(owner,"beast");
    close_socket(d); /* Link loss is not player deletion or familiar loss. */
    assert(live(owner) && live(body) && owner->desc == NULL &&
           body->desc == NULL && owner->pcdata->familiar == body &&
           body->wizard == owner && !IS_EXTRA(owner,EXTRA_SWITCH) &&
           d->character == NULL && d->original == NULL);
    assert(close(peer) == 0);
    cleanup();

    owner = player("Quitter"); body = mobile();
    peer = connect_player(owner); d = owner->desc;
    do_switch(owner,"beast");
    char empty[] = "";
    do_quit(owner,empty); /* The same path used by idle auto-quit. */
    assert(!live(owner) && live(body) && descriptor_list == NULL &&
           d->original == NULL && d->character == NULL && body->desc == NULL);
    assert(close(peer) == 0);
    cleanup();
}

static void note_quit(void)
{
    for (int state = CON_NOTE_TO; state <= CON_NOTE_FINISH; ++state) {
        CHAR_DATA *owner = player("Noter");
        int peer = connect_player(owner);
        DESCRIPTOR_DATA *d = owner->desc;
        d->connected = state;
        owner->pcdata->in_progress = new_note();
        owner->gold = 777 + state;
        assert(save_char_obj_checked(owner));
        char empty[] = "";
        do_quit(owner,empty);
        assert(!live(owner) && descriptor_list == NULL &&
               char_free != NULL && char_free->next != char_free &&
               pcdata_free != NULL && pcdata_free->next != pcdata_free);
        assert(close(peer) == 0);
        cleanup();
        owner = player("Noter");
        assert(owner->gold == 777 + state && owner->pcdata->in_progress == NULL);
        cleanup();
    }
}

static void class_reset(void)
{
    CHAR_DATA *owner = player("Owner"), *pet = mobile();
    CHAR_DATA *other = player("Other");
    owner->class = CLASS_MONK;
    owner->pcdata->familiar = pet;
    pet->wizard = owner;
    owner->pcdata->propose = other;
    other->pcdata->propose = owner;
    owner->embrace = other; other->embrace = owner;
    owner->embraced = other->embraced = 1;
    OBJ_DATA *stray = object(owner);
    int peer = connect_player(owner);
    DESCRIPTOR_DATA *d = owner->desc;
    do_switch(owner,"beast");
    do_clearclass(owner,"self");
    /* Preserve the old reset's NPC survival, but not its orphaned owner. */
    assert(live(pet) && pet->wizard == NULL && owner->pcdata->familiar == NULL &&
           stray->chobj == NULL && other->pcdata->propose == NULL &&
           other->embrace == NULL && other->embraced == ARE_NONE &&
           d->character == owner && d->original == NULL &&
           owner->desc == d && pet->desc == NULL);
    cleanup();
    assert(close(peer) == 0);
}

static void remove_saves(const char *path)
{
    DIR *dir = opendir(path);
    assert(dir != NULL);
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        char name[1024];
        assert(snprintf(name,sizeof(name),"%s/%s",path,entry->d_name) <
               (int)sizeof(name));
        assert(unlink(name) == 0);
    }
    assert(closedir(dir) == 0 && rmdir(path) == 0);
}

int main(int argc, char **argv)
{
    char root[] = "/tmp/anathema-ownership-XXXXXX", cwd[4096];
    assert(getcwd(cwd,sizeof(cwd)) && mkdtemp(root));
    assert(chdir(root) == 0 && mkdir("player",0700) == 0 &&
           mkdir("backup",0700) == 0 && mkdir("run",0700) == 0 &&
           chdir("run") == 0);
    string_space = malloc(4 * 1024 * 1024);
    assert(string_space != NULL);
    top_string = string_space;
    current_time = 123456;
    init_mm();
    fpReserve = fopen(NULL_FILE,"r");
    assert(fpReserve != NULL);
    leader_board.bestpk_number = leader_board.tt_number = LONG_MAX;
    leader_board.worstpk_number = LONG_MIN;
    ROOM_INDEX_DATA *rooms[] = {&home,&altar,&disconnect_room};
    long vnums[] = {900,ROOM_VNUM_ALTAR,ROOM_VNUM_DISCONNECTION};
    for (int i = 0; i < 3; ++i) {
        rooms[i]->vnum = vnums[i]; rooms[i]->area = &area;
        rooms[i]->name = "Private room"; rooms[i]->description = "";
        rooms[i]->light = 1;
        rooms[i]->next = room_index_hash[vnums[i] % MAX_KEY_HASH];
        room_index_hash[vnums[i] % MAX_KEY_HASH] = rooms[i];
    }
    object_index.vnum = 91001;
    object_index.name = object_index.short_descr = pooled("a test form");
    object_index.description = str_empty;
    object_index.item_type = ITEM_TREASURE;
    obj_index_hash[object_index.vnum % MAX_KEY_HASH] = &object_index;
    mobile_index.vnum = 91002;
    mobile_index.player_name = pooled("beast");
    mobile_index.short_descr = pooled("a beast");
    mobile_index.long_descr = mobile_index.description = str_empty;
    mobile_index.level = 1;
    mobile_index.act = ACT_IS_NPC;
    if (argc == 1 || !strcmp(argv[1],"objects")) objects();
    if (argc == 1 || !strcmp(argv[1],"familiars")) familiars();
    if (argc == 1 || !strcmp(argv[1],"heads")) heads();
    if (argc == 1 || !strcmp(argv[1],"switch")) switching();
    if (argc == 1 || !strcmp(argv[1],"notes")) note_quit();
    if (argc == 1 || !strcmp(argv[1],"reset")) class_reset();
    assert(fclose(fpReserve) == 0);
    assert(chdir("..") == 0);
    remove_saves("player"); remove_saves("backup");
    assert(rmdir("run") == 0 && chdir(cwd) == 0 && rmdir(root) == 0);
    puts("lifecycle ownership: forms, heads, familiars, switching, quit and class reset passed");
    return 0;
}
