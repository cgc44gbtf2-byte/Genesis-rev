/* Actual engine and atomic-save fault injection; private synthetic data only. */
#define ATOMIC_SAVE_TESTING
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
#include "atomic_save.h"

extern char *string_space, *top_string;
extern bool fBootDb;
extern void init_mm(void), char_update(void);
extern NOTE_DATA *new_note(void);
static ROOM_INDEX_DATA room, altar, disconnected;
static AREA_DATA area;
static MOB_INDEX_DATA mobile_index;
static OBJ_INDEX_DATA armor_index;
static int peers[32], peer_count;

static CHAR_DATA *player(char *name, bool online)
{
    DESCRIPTOR_DATA shell = {0};
    load_char_obj(&shell,name);
    CHAR_DATA *ch = shell.character;
    ch->desc = NULL;
    ch->level = LEV_AWA;
    ch->next = char_list; char_list = ch;
    char_to_room(ch,&room);
    if (online) {
        int sockets[2];
        assert(socketpair(AF_UNIX,SOCK_STREAM,0,sockets) == 0);
        assert(peer_count < 32); peers[peer_count++] = sockets[1];
        DESCRIPTOR_DATA *d = alloc_perm(sizeof(*d));
        memset(d,0,sizeof(*d));
        d->descriptor = sockets[0]; d->host = str_dup("private-test");
        d->outsize = 16384; d->outbuf = alloc_mem(d->outsize);
        d->character = ch; ch->desc = d; d->connected = CON_PLAYING;
        d->next = descriptor_list; descriptor_list = d;
    }
    return ch;
}

static int live(CHAR_DATA *ch)
{
    for (CHAR_DATA *p = char_list; p != NULL; p = p->next)
        if (p == ch) return 1;
    return 0;
}

static void clean(void)
{
    atomic_save_test_fault = SAVE_FAULT_NONE;
    while (char_list != NULL) extract_char(char_list,TRUE);
    assert(descriptor_list == NULL);
    while (peer_count) assert(close(peers[--peer_count]) == 0);
}

static void metadata(void)
{
    CHAR_DATA *ch = player("Timestamp",FALSE);
    assert(save_char_obj_checked(ch));
    time_t good = ch->save_time;
    ++current_time;
    atomic_save_test_fault = SAVE_FAULT_RENAME;
    assert(!save_char_obj_checked(ch));
    assert(ch->save_time == good);
    clean();
}

static void backup(void)
{
    CHAR_DATA *ch = player("Backup",FALSE);
    assert(save_char_obj_checked(ch));
    time_t good = ch->save_time;
    ++current_time;
    save_char_obj_backup(ch);
    assert(ch->save_time == good);
    clean();
}

static void switched(void)
{
    CHAR_DATA *ch = player("Switched",TRUE);
    CHAR_DATA *body = create_mobile(&mobile_index);
    char_to_room(body,&room);
    DESCRIPTOR_DATA *d = ch->desc;
    do_switch(ch,"beast");
    assert(d->original == ch && ch->desc == NULL);
    char_update();
    assert(access("../player/Switched",F_OK) == 0 &&
           access("../backup/Switched",F_OK) == 0 &&
           d->character == body && d->original == ch && ch->desc == NULL);
    ch->gold = 876;
    do_save(body,""); /* Save the original, never serialize the NPC shell. */
    assert(d->original == ch && d->character == body &&
           strstr(d->outbuf,"Saved.") != NULL && IS_EXTRA(ch,EXTRA_SWITCH));
    FILE *fp=fopen("../player/Switched","r"); assert(fp);
    char line[1024]; long extra=-1;
    while (fgets(line,sizeof(line),fp))
        if (sscanf(line,"Extra %ld",&extra)==1) break;
    assert(fclose(fp)==0 && extra>=0 && !(extra & EXTRA_SWITCH));
    clean();
    ch=player("Switched",TRUE);
    assert(ch->gold==876 && !IS_EXTRA(ch,EXTRA_SWITCH));
    clean();
}

static void legacy_switch(void)
{
    FILE *fp=fopen("../player/Legacyswitch","w"); assert(fp);
    fprintf(fp,"#PLAYER\nName Legacyswitch~\nLevel 2\nExtra %ld\nEnd\n#END\n",
            (long)(EXTRA_SWITCH|EXTRA_NEWPASS));
    assert(fclose(fp)==0);
    CHAR_DATA *ch=player("Legacyswitch",FALSE);
    assert(!IS_EXTRA(ch,EXTRA_SWITCH) && IS_EXTRA(ch,EXTRA_NEWPASS));
    DESCRIPTOR_DATA shell={0};
    assert(load_char_short(&shell,"Legacyswitch"));
    assert(!IS_EXTRA(shell.character,EXTRA_SWITCH) &&
           IS_EXTRA(shell.character,EXTRA_NEWPASS));
    free_char(shell.character);
    clean();
}

static void manual(void)
{
    CHAR_DATA *ch = player("Manual",TRUE);
    atomic_save_test_fault = SAVE_FAULT_RENAME;
    do_save(ch,"");
    assert(strstr(ch->desc->outbuf,"Saved.") == NULL);
    atomic_save_test_fault = SAVE_FAULT_NONE;
    ch->desc->outtop=0; ch->desc->outbuf[0]='\0';
    do_save(ch,"");
    assert(strstr(ch->desc->outbuf,"Saved.") != NULL && ch->save_pending == 0);
    ch->level=LEV_SLE;
    ch->desc->outtop=0; ch->desc->outbuf[0]='\0';
    do_save(ch,"");
    assert(strstr(ch->desc->outbuf,"not eligible") != NULL &&
           strstr(ch->desc->outbuf,"Saved.") == NULL && ch->save_pending == 0);
    clean();
}

static void quit_failure(void)
{
    CHAR_DATA *ch = player("Quitfail",TRUE);
    ch->desc->connected = CON_NOTE_TEXT;
    ch->pcdata->in_progress = new_note();
    NOTE_DATA *note = ch->pcdata->in_progress;
    OBJ_DATA *gear = create_object(&armor_index,0);
    obj_to_char(gear,ch); equip_char(ch,gear,WEAR_BODY);
    assert(save_char_obj_checked(ch));
    atomic_save_test_fault = SAVE_FAULT_RENAME;
    char empty[] = "";
    do_quit(ch,empty);
    assert(live(ch) && ch->desc != NULL && ch->pcdata->in_progress == note &&
           gear->carried_by == ch && gear->wear_loc == WEAR_BODY);
    /* The test owns the draft; extract_char does not discard drafts itself. */
    free_note(note); ch->pcdata->in_progress = NULL;
    clean();
}

static long saved_gold(const char *directory, const char *name)
{
    char path[256], line[2048];
    assert(snprintf(path,sizeof(path),"%s/%s",directory,name) < (int)sizeof(path));
    FILE *fp = fopen(path,"r"); assert(fp);
    long gold = LONG_MIN;
    while (fgets(line,sizeof(line),fp) != NULL)
        if (sscanf(line,"Gold %ld",&gold) == 1) break;
    assert(fclose(fp) == 0 && gold != LONG_MIN);
    return gold;
}

static void independent_recovery(void)
{
    CHAR_DATA *ch = player("Partial",TRUE);
    ch->gold = 100;
    assert(player_save_state(ch,PLAYER_SAVE_ALL) == PLAYER_SAVE_ALL);
    time_t good = ch->save_time;
    assert(rename("../player","../unavailable") == 0);
    ++current_time; ch->gold = 200;
    do_save(ch,"");
    assert(strstr(ch->desc->outbuf,"Primary save failed; only the backup saved") != NULL);
    assert(ch->save_time == good && ch->save_pending == PLAYER_SAVE_PRIMARY &&
           ch->save_retry_delay[0] == 30 &&
           saved_gold("../backup","Partial") == 200);
    time_t deadline_base = ch->save_retry_time[0];
    ++current_time; ch->gold = 300;
    player_autosave_tick(); /* Healthy backup stays current during primary delay. */
    assert(ch->save_time == good && ch->save_retry_time[0] == deadline_base &&
           saved_gold("../backup","Partial") == 300);
    assert(rename("../unavailable","../player") == 0);
    current_time = deadline_base + 30;
    player_autosave_tick();
    assert(ch->save_pending == 0 && ch->save_time == current_time &&
           saved_gold("../player","Partial") == 300);

    assert(rename("../backup","../unavailable") == 0);
    ++current_time; ch->gold = 400;
    ch->desc->outtop=0; ch->desc->outbuf[0]='\0';
    do_save(ch,"");
    assert(strstr(ch->desc->outbuf,"Player saved, but backup failed") != NULL);
    assert(ch->save_pending == PLAYER_SAVE_BACKUP &&
           ch->save_time == current_time && saved_gold("../player","Partial") == 400);
    time_t backup_attempt = ch->save_retry_time[1];
    ++current_time; ch->gold = 500;
    player_autosave_tick();
    assert(ch->save_retry_time[1] == backup_attempt &&
           saved_gold("../player","Partial") == 500);
    assert(rename("../unavailable","../backup") == 0);
    current_time = backup_attempt + 30;
    player_autosave_tick();
    assert(ch->save_pending == 0 && saved_gold("../backup","Partial") == 500);
    clean();
    ch = player("Partial",FALSE); /* Native reload, with no persisted retry state. */
    assert(ch->gold == 500 && ch->save_pending == 0 && ch->save_retry_delay[0] == 0);
    clean();
}

static void fairness_and_backoff(void)
{
    CHAR_DATA *a = player("Fairone",TRUE), *b = player("Fairtwo",TRUE);
    a->save_attempt_time = current_time - 100;
    b->save_attempt_time = current_time - 50;
    atomic_save_test_fault = SAVE_FAULT_RENAME;
    player_autosave_tick();
    assert(a->save_pending == PLAYER_SAVE_ALL && b->save_pending == 0);
    player_autosave_tick();
    assert(b->save_pending == PLAYER_SAVE_ALL); /* Failure did not monopolize selection. */
    time_t attempted = a->save_attempt_time;
    ++current_time;
    player_autosave_tick();
    assert(a->save_attempt_time == attempted && b->save_attempt_time == attempted);
    atomic_save_test_fault = SAVE_FAULT_NONE;
    current_time += 30;
    player_autosave_tick();
    assert((a->save_pending == 0) != (b->save_pending == 0)); /* One-player budget. */
    ++current_time; player_autosave_tick();
    assert(a->save_pending == 0 && b->save_pending == 0);
    clean();

    a = player("Backoff",TRUE);
    atomic_save_test_fault = SAVE_FAULT_RENAME;
    for (unsigned int i = 0; i < 7; ++i) {
        assert(!save_char_obj_checked(a));
        unsigned int expected = i == 0 ? 30 : i == 1 ? 60 : i == 2 ? 120 :
            i == 3 ? 240 : 300;
        assert(a->save_retry_delay[0] == expected);
        current_time += expected;
    }
    /* A disconnected player's failed side still receives automatic recovery. */
    close_socket(a->desc);
    atomic_save_test_fault = SAVE_FAULT_NONE;
    a->gold = 999;
    player_autosave_tick();
    assert(a->save_pending == 0 && saved_gold("../player","Backoff") == 999 &&
           access("../backup/Backoff",F_OK) != 0);
    clean();
}

static void migration_and_faults(void)
{
    CHAR_DATA *ch = player("Migration",TRUE);
    DESCRIPTOR_DATA *d = ch->desc;
    d->password_save_pending = TRUE;
    d->connected = CON_GET_NAME;
    time_t attempted = ch->save_attempt_time;
    assert(player_save_state(ch,PLAYER_SAVE_ALL) == 0 &&
           ch->save_attempt_time == attempted && d->password_save_pending);
    d->connected = CON_PLAYING;
    atomic_save_test_fault = SAVE_FAULT_RENAME;
    assert(player_save_state(ch,PLAYER_SAVE_ALL) == 0 && d->password_save_pending);
    atomic_save_test_fault = SAVE_FAULT_NONE;
    assert(player_save_state(ch,PLAYER_SAVE_BACKUP) == PLAYER_SAVE_BACKUP &&
           d->password_save_pending);
    atomic_save_test_fault = SAVE_FAULT_DIRSYNC;
    assert(player_save_state(ch,PLAYER_SAVE_PRIMARY) == 0 && d->password_save_pending);
    atomic_save_test_fault = SAVE_FAULT_NONE;
    current_time += 300;
    player_autosave_tick();
    assert(!d->password_save_pending && ch->save_pending == 0);
    clean();

    ch = player("Faults",TRUE);
    enum atomic_save_fault faults[] = {SAVE_FAULT_CREATE,SAVE_FAULT_FLUSH,
        SAVE_FAULT_SYNC,SAVE_FAULT_CLOSE,SAVE_FAULT_RENAME,SAVE_FAULT_DIRSYNC};
    for (size_t i = 0; i < sizeof(faults)/sizeof(faults[0]); ++i) {
        atomic_save_test_fault = SAVE_FAULT_NONE;
        assert(save_char_obj_checked(ch));
        time_t durable = ch->save_time;
        ++current_time;
        atomic_save_test_fault = faults[i];
        assert(!save_char_obj_checked(ch) && ch->save_time == durable &&
               (ch->save_pending & PLAYER_SAVE_PRIMARY));
        atomic_save_test_fault = SAVE_FAULT_NONE;
        current_time += 30; player_autosave_tick();
        assert(ch->save_pending == 0 && ch->save_time == current_time);
    }
    /* A backwards clock must not postpone a pending retry for hours. */
    atomic_save_test_fault = SAVE_FAULT_RENAME;
    assert(!save_char_obj_checked(ch));
    atomic_save_test_fault = SAVE_FAULT_NONE;
    current_time -= 1000; player_autosave_tick();
    assert(ch->save_pending == 0);
    clean();
}

static void remove_saves(const char *path)
{
    DIR *dir = opendir(path); assert(dir);
    struct dirent *e;
    while ((e = readdir(dir)) != NULL) {
        if (e->d_name[0] == '.') continue;
        char name[1024];
        assert(snprintf(name,sizeof(name),"%s/%s",path,e->d_name) < (int)sizeof(name));
        assert(unlink(name) == 0);
    }
    assert(closedir(dir) == 0 && rmdir(path) == 0);
}

int main(int argc, char **argv)
{
    char root[] = "/tmp/anathema-autosave-XXXXXX", cwd[4096];
    assert(getcwd(cwd,sizeof(cwd)) && mkdtemp(root));
    assert(chdir(root) == 0 && mkdir("player",0700) == 0 &&
           mkdir("backup",0700) == 0 && mkdir("run",0700) == 0 &&
           chdir("run") == 0);
    string_space = malloc(4*1024*1024); assert(string_space);
    top_string = string_space;
    current_time = 123456; init_mm();
    fpReserve = fopen(NULL_FILE,"r"); assert(fpReserve);
    leader_board.bestpk_number = leader_board.tt_number = LONG_MAX;
    leader_board.worstpk_number = LONG_MIN;
    ROOM_INDEX_DATA *rooms[] = {&room,&altar,&disconnected};
    long vnums[] = {900,ROOM_VNUM_ALTAR,ROOM_VNUM_DISCONNECTION};
    for (int i = 0; i < 3; ++i) {
        rooms[i]->vnum=vnums[i]; rooms[i]->area=&area;
        rooms[i]->name="Private room"; rooms[i]->description=""; rooms[i]->light=1;
        rooms[i]->next=room_index_hash[vnums[i]%MAX_KEY_HASH];
        room_index_hash[vnums[i]%MAX_KEY_HASH]=rooms[i];
    }
    FILE *fp=tmpfile(); assert(fp);
    fputs("beast~",fp); rewind(fp); fBootDb=TRUE;
    mobile_index.player_name=mobile_index.short_descr=fread_string(fp);
    fBootDb=FALSE; assert(fclose(fp)==0);
    mobile_index.long_descr=mobile_index.description=str_empty;
    mobile_index.vnum=92001; mobile_index.level=1; mobile_index.act=ACT_IS_NPC;
    armor_index.name=armor_index.short_descr=mobile_index.player_name;
    armor_index.description=str_empty; armor_index.vnum=92002;
    armor_index.item_type=ITEM_ARMOR;
    armor_index.wear_flags=ITEM_TAKE|ITEM_WEAR_BODY; armor_index.value[0]=1;
    obj_index_hash[armor_index.vnum%MAX_KEY_HASH]=&armor_index;
    if (argc==1 || !strcmp(argv[1],"metadata")) metadata();
    if (argc==1 || !strcmp(argv[1],"backup")) backup();
    if (argc==1 || !strcmp(argv[1],"switched")) switched();
    if (argc==1 || !strcmp(argv[1],"legacy")) legacy_switch();
    if (argc==1 || !strcmp(argv[1],"manual")) manual();
    if (argc==1 || !strcmp(argv[1],"quit")) quit_failure();
    if (argc==1 || !strcmp(argv[1],"recovery")) independent_recovery();
    if (argc==1 || !strcmp(argv[1],"fairness")) fairness_and_backoff();
    if (argc==1 || !strcmp(argv[1],"migration")) migration_and_faults();
    assert(fclose(fpReserve)==0 && chdir("..")==0);
    remove_saves("player"); remove_saves("backup");
    assert(rmdir("run")==0 && chdir(cwd)==0 && rmdir(root)==0);
    puts("autosave/recovery regressions passed");
    return 0;
}
