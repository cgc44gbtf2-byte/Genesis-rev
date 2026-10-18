/* Full-engine class crafting, cost, equipment and persistence regressions. */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include "merc.h"

extern char *string_space, *top_string;
extern bool fBootDb;
extern void init_mm(void);
extern OBJ_DATA *obj_free;
static CHAR_DATA actor;
static PC_DATA pc;
static DESCRIPTOR_DATA descriptor;
static ROOM_INDEX_DATA room;
static AREA_DATA area;
static char output[65536];
static OBJ_INDEX_DATA indices[40];
static int index_count;
static const struct craft {
    DO_FUN *fn;
    long class_id, vnum, primal, quest, drow, blood, type;
    const char *arg;
} crafts[] = {
    {do_vamparmor, CLASS_VAMPIRE, 29630,100,400,0,0,ITEM_ARMOR,"mark"},
    {do_vamparmor, CLASS_VAMPIRE, 29631,100,400,0,0,ITEM_ARMOR,"trenchcoat"},
    {do_vamparmor, CLASS_VAMPIRE, 29632,100,400,0,0,ITEM_ARMOR,"vest"},
    {do_ninjaarmor, CLASS_NINJA, 29600,100,400,0,0,ITEM_ARMOR,"belt"},
    {do_ninjaarmor, CLASS_NINJA, 29601,100,400,0,0,ITEM_ARMOR,"boots"},
    {do_ninjaarmor, CLASS_NINJA, 29602,100,400,0,0,ITEM_ARMOR,"gi"},
    {do_ninjaarmor, CLASS_NINJA, 29603,100,400,0,0,ITEM_ARMOR,"cloak"},
    {do_wolfarmor, CLASS_WEREWOLF, 29640,100,400,0,0,ITEM_ARMOR,"vest"},
    {do_wolfarmor, CLASS_WEREWOLF, 29641,100,400,0,0,ITEM_ARMOR,"mask"},
    {do_wolfarmor, CLASS_WEREWOLF, 29642,100,400,0,0,ITEM_ARMOR,"talisman"},
    {do_monkarmor, CLASS_MONK, 29610,100,400,0,0,ITEM_ARMOR,"necklace"},
    {do_monkarmor, CLASS_MONK, 29611,100,400,0,0,ITEM_ARMOR,"habit"},
    {do_monkarmor, CLASS_MONK, 29612,100,400,0,0,ITEM_ARMOR,"belt"},
    {do_monkarmor, CLASS_MONK, 29613,100,400,0,0,ITEM_ARMOR,"tiger"},
    {do_monkarmor, CLASS_MONK, 29614,100,400,0,0,ITEM_ARMOR,"dragon"},
    {do_elemarmor, CLASS_ELEM, 29620,100,400,0,0,ITEM_ARMOR,"bracer"},
    {do_elemarmor, CLASS_ELEM, 29621,100,400,0,0,ITEM_WEAPON,"sword"},
    {do_drowarmor, CLASS_DROW, 29652,0,400,8000,0,ITEM_ARMOR,"cloak"},
    {do_drowarmor, CLASS_DROW, 29653,0,400,8000,0,ITEM_ARMOR,"helmet"},
    {do_drowarmor, CLASS_DROW, 29654,0,400,8000,0,ITEM_ARMOR,"breastplate"},
    {do_drowarmor, CLASS_DROW, 29655,0,400,8000,0,ITEM_ARMOR,"bracer"},
    {do_drowarmor, CLASS_DROW, 29658,0,400,8000,0,ITEM_MISSILE,"crossbow"},
    {do_magearmor, CLASS_MAGE, 29660,100,400,0,0,ITEM_ARMOR,"robes"},
    {do_magearmor, CLASS_MAGE, 29661,100,400,0,0,ITEM_ARMOR,"hood"},
    {do_magearmor, CLASS_MAGE, 29662,100,400,0,0,ITEM_ARMOR,"talisman"},
    {do_forge, CLASS_HIGHLANDER, 29650,50,100,0,0,ITEM_WEAPON,""},
    {do_shadowforge, CLASS_VAMPIRE, 29639,0,0,0,50,ITEM_WEAPON,"slash"},
    {do_shadowforge, CLASS_VAMPIRE, 29639,0,0,0,50,ITEM_WEAPON,"pierce"},
    {do_shadowforge, CLASS_VAMPIRE, 29639,0,0,0,50,ITEM_WEAPON,"blunt"}
};

static char *intern(const char *text)
{
    FILE *fp = tmpfile();
    assert(fp && fputs(text, fp) >= 0 && fputs("~", fp) >= 0);
    rewind(fp);
    fBootDb = TRUE;
    char *value = fread_string(fp);
    fBootDb = FALSE;
    fclose(fp);
    return value;
}

static OBJ_INDEX_DATA *index_for(long vnum, long type)
{
    OBJ_INDEX_DATA *p = get_obj_index(vnum);
    if (p != NULL) return p;
    assert(index_count < (int)(sizeof(indices)/sizeof(indices[0])));
    p = &indices[index_count++];
    p->vnum = vnum; p->item_type = type;
    p->name = intern("gear"); p->short_descr = intern("a piece of gear");
    p->description = intern("Some gear is here.");
    p->wear_flags = ITEM_TAKE | ITEM_WEAR_BODY | ITEM_WEAR_HOLD;
    p->weight = 1;
    if (type == ITEM_WEAPON) { p->value[1] = 10; p->value[2] = 20; p->value[3] = TYPE_SLASH; }
    p->next = obj_index_hash[vnum % MAX_KEY_HASH];
    obj_index_hash[vnum % MAX_KEY_HASH] = p;
    return p;
}

static void fixture(long class_id)
{
    while (actor.carrying != NULL) extract_obj(actor.carrying);
    while (room.contents != NULL) extract_obj(room.contents);
    free_string(actor.clan); free_string(actor.morph);
    memset(&pc, 0, sizeof(pc)); memset(&descriptor, 0, sizeof(descriptor));
    memset(&room, 0, sizeof(room));
    clear_char(&actor);
    actor.name = "Gearhero"; actor.pcdata = &pc; actor.desc = &descriptor;
    actor.class = class_id; actor.level = LEV_AWA;
    actor.position = POS_STANDING; actor.vampgen = 9;
    actor.hit = actor.max_hit = actor.mana = actor.max_mana = 100000;
    actor.logon = current_time; actor.stance[0] = -1;
    actor.in_room = &room; room.people = char_list = &actor;
    room.area = &area; room.light = 1;
    descriptor.character = &actor; descriptor.connected = CON_PLAYING;
    descriptor.outbuf = output; descriptor.outsize = sizeof(output);
    descriptor.fcommand = TRUE; descriptor_list = &descriptor;
    output[0] = '\0';
    pc.pwd = pc.bamfin = pc.bamfout = "";
    pc.cdest = pc.title = pc.decap_char = "";
    pc.marriage = pc.email = pc.covenant = "";
    pc.cov_time = pc.leader = pc.vleader = pc.love = pc.conding = "";
    pc.cland[0] = pc.cland[1] = pc.cland[2] = -1;
    pc.powers[DISC_OBTENEBRATION] = 6;
    pc.ninja[NINJA_CHIKYU] = 1;
}

static void invoke(DO_FUN *fn, const char *text)
{
    char argument[MAX_INPUT_LENGTH];
    snprintf(argument, sizeof(argument), "%s", text);
    descriptor.outtop = 0; output[0] = '\0';
    fn(&actor, argument);
}

static void craft_boundaries(void)
{
    for (size_t i = 0; i < sizeof(crafts)/sizeof(crafts[0]); ++i) {
        const struct craft *c = &crafts[i];
        if (get_obj_index(c->vnum) == NULL) {
            fixture(c->class_id);
            actor.primal = c->primal; pc.quest = c->quest;
            pc.classpoints[CP_CURRENT] = c->drow;
            actor.blood[BLOOD_CURRENT] = c->blood;
            invoke(c->fn, c->arg);
            assert(!actor.carrying && actor.primal == c->primal &&
                   pc.quest == c->quest && pc.classpoints[CP_CURRENT] == c->drow &&
                   actor.blood[BLOOD_CURRENT] == c->blood);
        }
        index_for(c->vnum, c->type);
        fixture(c->class_id);
        actor.primal = c->primal; pc.quest = c->quest;
        pc.classpoints[CP_CURRENT] = c->drow; actor.blood[BLOOD_CURRENT] = c->blood;
        invoke(c->fn, c->arg);
        if (!actor.carrying || actor.carrying->pIndexData->vnum != c->vnum)
            fprintf(stderr, "gear case %zu class %ld vnum %ld: %s\n",
                    i, c->class_id, c->vnum, output);
        assert(actor.carrying && actor.carrying->pIndexData->vnum == c->vnum);
        assert(!str_cmp(actor.carrying->questmaker, actor.name));
        assert(!str_cmp(actor.carrying->questowner, actor.name));
        assert(actor.primal == 0 && pc.quest == 0 &&
               pc.classpoints[CP_CURRENT] == 0 && actor.blood[BLOOD_CURRENT] == 0);
        fixture(c->class_id);
        actor.primal = c->primal; pc.quest = c->quest;
        pc.classpoints[CP_CURRENT] = c->drow; actor.blood[BLOOD_CURRENT] = c->blood;
        if (c->quest) --pc.quest; else --actor.blood[BLOOD_CURRENT];
        long before = pc.quest, blood = actor.blood[BLOOD_CURRENT];
        invoke(c->fn, c->arg);
        assert(actor.carrying == NULL && actor.primal == c->primal &&
               pc.quest == before && actor.blood[BLOOD_CURRENT] == blood &&
               pc.classpoints[CP_CURRENT] == c->drow);
        if (c->primal || c->drow) {
            fixture(c->class_id);
            actor.primal = c->primal; pc.quest = c->quest;
            pc.classpoints[CP_CURRENT] = c->drow;
            if (c->primal) --actor.primal; else --pc.classpoints[CP_CURRENT];
            long primal = actor.primal, drow = pc.classpoints[CP_CURRENT];
            invoke(c->fn, c->arg);
            assert(!actor.carrying && actor.primal == primal &&
                   pc.quest == c->quest && pc.classpoints[CP_CURRENT] == drow);
        }
        fixture(CLASS_NONE);
        actor.primal = 10000; pc.quest = 10000;
        invoke(c->fn, c->arg);
        assert(!actor.carrying && actor.primal == 10000 && pc.quest == 10000);
    }
}

static void missing_sorcerer(void)
{
    /* The original missing-template branch exits the entire game process. */
    pid_t pid = fork();
    assert(pid >= 0);
    if (pid == 0) {
        fixture(CLASS_SORCERER);
        actor.primal = 10;
        invoke(do_sorccreate, "book");
        assert(actor.primal == 10 && actor.carrying == NULL);
        _exit(0);
    }
    int status;
    assert(waitpid(pid, &status, 0) == pid && WIFEXITED(status) && !WEXITSTATUS(status));
}

static void sorcerer(void)
{
    index_for(OBJ_VNUM_PROTOPLASM, ITEM_TRASH);
    const char *words[] = {"book", "pen", "page"};
    const long types[] = {ITEM_BOOK, ITEM_PEN, ITEM_PAGE};
    for (int i = 0; i < 3; ++i) {
        fixture(CLASS_SORCERER);
        actor.primal = i < 2 ? 10 : 0;
        invoke(do_sorccreate, words[i]);
        assert(actor.carrying && actor.carrying->item_type == types[i] && actor.primal == 0);
    }
    fixture(CLASS_SORCERER);
    OBJ_DATA *obj = create_object(index_for(72001, ITEM_WEAPON), 0);
    obj_to_char(obj, &actor);
    invoke(do_enchant, "gear");
    int n = 0;
    for (AFFECT_DATA *a = obj->affected; a; a = a->next) {
        assert(a->modifier == 5); ++n;
    }
    assert(n == 2 && IS_SET(obj->quest, QUEST_SORCENCHANT));
    invoke(do_enchant, "gear");
    n = 0; for (AFFECT_DATA *a = obj->affected; a; a = a->next) ++n;
    assert(n == 2); /* Existing zero-cost, once-only +5/+5 enchant preserved. */
}

static void forge(void)
{
    fixture(CLASS_HIGHLANDER);
    index_for(29650, ITEM_WEAPON);
    /* Real engine free-list allocation, but an exactly sized allocation lets
     * ASan detect reading beyond value[4], the struct's final member. The
     * legacy permanent arena otherwise hides this intra-arena overrun. */
    OBJ_DATA *guarded = calloc(1, sizeof(*guarded));
    assert(guarded);
    guarded->next = obj_free; obj_free = guarded;
    OBJ_DATA *source = create_object(index_for(72001, ITEM_WEAPON), 0);
    assert(source == guarded);
    obj_to_char(source, &actor);
    free_string(source->questowner); source->questowner = str_dup(actor.name);
    const long values[4] = {123, 25, 50, TYPE_PIERCE};
    for (int i = 0; i < 4; ++i) source->value[i] = values[i];
    actor.primal = 50; pc.quest = 100;
    invoke(do_forge, "gear");
    assert(actor.carrying && actor.carrying->pIndexData->vnum == 29650);
    assert(actor.carrying->next_content == NULL && actor.primal == 0 && pc.quest == 0);
    for (int i = 0; i < 4; ++i) assert(actor.carrying->value[i] == values[i]);
}

static void mage_costs(void)
{
    fixture(CLASS_MAGE);
    OBJ_DATA *obj = create_object(index_for(72001, ITEM_WEAPON), 0);
    obj_to_char(obj, &actor);
    actor.spheres[MPRI] = 3; actor.arete = 0; /* Guaranteed failed casting roll. */
    actor.quint[BODY] = 5; actor.mana = 5000;
    invoke(do_mageenchant, "gear");
    assert(actor.mana == 0 && actor.quint[BODY] == 0);
    assert(!IS_SET(obj->quest, QUEST_AGGRAVATED));
    actor.quint[BODY] = 2; actor.mana = 4999;
    SET_BIT(actor.xtra, XTRA_HEARTS_BLOOD);
    long hit = actor.hit;
    invoke(do_mageenchant, "gear");
    assert(actor.hit == hit && actor.mana == 4999 && actor.quint[BODY] == 2);
    actor.mana = 5000;
    invoke(do_mageenchant, "gear");
    assert(actor.mana == 0 && actor.quint[BODY] == 0);
    assert(hit - actor.hit >= 600 && hit - actor.hit <= 1500);
    actor.mana = 5000; actor.quint[BODY] = -1;
    hit = actor.hit;
    invoke(do_mageenchant, "gear");
    assert(actor.mana == 5000 && actor.quint[BODY] == -1 && actor.hit == hit);
    actor.quint[BODY] = 2; actor.hit = 1000;
    invoke(do_mageenchant, "gear");
    assert(actor.mana == 5000 && actor.quint[BODY] == 2 && actor.hit == 1000);
    actor.hit = 100000; actor.quint[BODY] = 5; actor.arete = 100;
    invoke(do_mageenchant, "gear");
    assert(IS_SET(obj->quest, QUEST_AGGRAVATED) &&
           actor.mana == 0 && actor.quint[BODY] == 0);
    actor.mana = 5000; actor.quint[BODY] = 5;
    invoke(do_mageenchant, "gear");
    assert(actor.mana == 5000 && actor.quint[BODY] == 5);
}

static OBJ_DATA *armor(long flags, long spell, long quest, long slot)
{
    OBJ_DATA *o = create_object(index_for(72002, ITEM_ARMOR), 0);
    o->spectype = flags; o->value[3] = spell; o->quest = quest;
    obj_to_char(o, &actor);
    equip_char(&actor, o, slot);
    return o;
}

static void equipment(void)
{
    fixture(CLASS_DROW);
    OBJ_DATA *gear[8];
    const long slots[] = {WEAR_BODY,WEAR_HEAD,WEAR_LEGS,WEAR_FEET,
        WEAR_HANDS,WEAR_ARMS,WEAR_ABOUT,WEAR_WAIST};
    for (int i = 0; i < 8; ++i) gear[i] = armor(SITEM_DROW, 0, 0, slots[i]);
    assert(pc.atm == 75);
    unequip_char(&actor, gear[0]);
    assert(pc.atm == 70);
    for (int i = 1; i < 8; ++i) unequip_char(&actor, gear[i]);
    assert(pc.atm == 0);
    fixture(CLASS_DROW);
    OBJ_DATA *a = armor(SITEM_DROW, 20, 0, WEAR_BODY);
    OBJ_DATA *b = armor(SITEM_DROW, 20, 0, WEAR_HEAD);
    assert(pc.atm == 70 && IS_ITEMAFF(&actor, ITEMA_REFLECT));
    unequip_char(&actor, a);
    assert(pc.atm == 60 && IS_ITEMAFF(&actor, ITEMA_REFLECT));
    unequip_char(&actor, b);
    assert(pc.atm == 0 && !IS_ITEMAFF(&actor, ITEMA_REFLECT));
    fixture(CLASS_MAGE);
    armor(0, 20, 0, WEAR_BODY);
    OBJ_DATA *artifact = armor(0, 20, QUEST_ARTIFACT, WEAR_HEAD);
    assert(artifact->wear_loc == WEAR_HEAD && IS_ITEMAFF(&actor, ITEMA_ARTIFACT));
    OBJ_DATA *second = armor(0, 0, QUEST_ARTIFACT, WEAR_FEET);
    assert(second->wear_loc == WEAR_NONE && second->carried_by == NULL);
    fixture(CLASS_DROW);
    a = armor(0, 57, 0, WEAR_BODY);
    b = armor(0, 8, 0, WEAR_HEAD);
    unequip_char(&actor, a);
    assert(IS_AFFECTED(&actor, AFF_SANCTUARY)); /* Equivalent spell IDs share ownership. */
    unequip_char(&actor, b);
    assert(!IS_AFFECTED(&actor, AFF_SANCTUARY));
    fixture(CLASS_HIGHLANDER);
    armor(0, 20, 0, WEAR_BODY);
    OBJ_DATA *right = create_object(index_for(72001, ITEM_WEAPON), 0);
    right->spectype = SITEM_SILVER; right->value[0] = 20000;
    obj_to_char(right, &actor); equip_char(&actor, right, WEAR_WEAPON);
    OBJ_DATA *left = create_object(index_for(72001, ITEM_WEAPON), 0);
    left->spectype = SITEM_SILVER; left->value[0] = 20000;
    obj_to_char(left, &actor); equip_char(&actor, left, WEAR_HOLD);
    assert(IS_ITEMAFF(&actor, ITEMA_RIGHT_SILVER) &&
           IS_ITEMAFF(&actor, ITEMA_LEFT_SILVER) && pc.atm == 50);
    unequip_char(&actor, right);
    assert(!IS_ITEMAFF(&actor, ITEMA_RIGHT_SILVER) &&
           IS_ITEMAFF(&actor, ITEMA_LEFT_SILVER) && pc.atm == 50);
    unequip_char(&actor, left);
    assert(!IS_ITEMAFF(&actor, ITEMA_LEFT_SILVER) &&
           IS_ITEMAFF(&actor, ITEMA_REFLECT) && pc.atm == 50);
}

static void class_restrictions(void)
{
    const long ids[] = {CLASS_VAMPIRE,CLASS_WEREWOLF,CLASS_NINJA,CLASS_MONK,
        CLASS_ELEM,CLASS_DROW,CLASS_HIGHLANDER,CLASS_MAGE,CLASS_DRAGON,
        CLASS_SORCERER,CLASS_NONE,CLASS_VAMPIRE|CLASS_WEREWOLF};
    const long owners[] = {CLASS_VAMPIRE,CLASS_WEREWOLF,CLASS_NINJA,CLASS_MONK,
        CLASS_ELEM,CLASS_DROW,CLASS_HIGHLANDER,CLASS_MAGE};
    const long flags[] = {SITEM_VAMPIRE,SITEM_WEREWOLF,SITEM_NINJA,SITEM_MONK,
        SITEM_ELEM,SITEM_DROW,SITEM_HIGHLANDER,SITEM_MAGE};
    for (size_t c = 0; c < sizeof(ids)/sizeof(ids[0]); ++c)
        for (int f = 0; f < 8; ++f) {
            fixture(ids[c]);
            OBJ_DATA *o = armor(flags[f], 0, 0, WEAR_BODY);
            assert((o->wear_loc == WEAR_BODY) == (ids[c] == owners[f]));
        }
    static MOB_INDEX_DATA mobile;
    mobile.short_descr = "a gear-test mobile";
    for (int f = 0; f < 8; ++f) {
        fixture(owners[f]);
        SET_BIT(actor.act, ACT_IS_NPC);
        actor.pIndexData = &mobile; actor.pcdata = NULL;
        OBJ_DATA *o = armor(flags[f], 0, 0, WEAR_BODY);
        assert(o->wear_loc == WEAR_NONE && o->carried_by == NULL);
    }
}

static void dragon_persistence(void)
{
    fixture(CLASS_DRAGON);
    actor.vampgen = 1;
    SET_BIT(actor.polyaff, POLY_DRAGONFORM);
    SET_BIT(actor.affected_by, AFF_POLYMORPH);
    SET_BIT(actor.vampaff, VAM_DISGUISED);
    armor(0, 20, QUEST_ARTIFACT, WEAR_BODY);
    assert(pc.atm == 75);
    free_string(actor.carrying->questowner);
    actor.carrying->questowner = str_dup(actor.name);
    assert(save_char_obj_checked(&actor));
    DESCRIPTOR_DATA d = {0};
    bool loaded = load_char_obj(&d, "Gearhero");
    if (!loaded || d.character->pcdata->atm != 75)
        fprintf(stderr, "dragon reload found=%d class=%ld gen=%ld poly=%ld atm=%ld gear=%p\n",
                loaded, d.character->class, d.character->vampgen,
                d.character->polyaff, d.character->pcdata->atm, (void*)d.character->carrying);
    assert(loaded && d.character->pcdata->atm == 75);
    while (d.character->carrying) extract_obj(d.character->carrying);
    free_char(d.character);
    /* Save/reload with no gear must still recover innate Dragon resistance. */
    while (actor.carrying) extract_obj(actor.carrying);
    assert(pc.atm == 50 && save_char_obj_checked(&actor));
    memset(&d, 0, sizeof(d));
    assert(load_char_obj(&d, "Gearhero") && !d.character->carrying &&
           d.character->pcdata->atm == 50);
    free_char(d.character);
    armor(0, 20, QUEST_ARTIFACT, WEAR_BODY);
    invoke(do_dragonform, "");
    assert(!IS_POLYAFF(&actor, POLY_DRAGONFORM) && pc.atm == 50);
}

int main(int argc, char **argv)
{
    char root[] = "/tmp/anathema-gear-XXXXXX", original[4096];
    assert(getcwd(original, sizeof(original)) && mkdtemp(root) && chdir(root) == 0);
    assert(mkdir("area",0700) == 0 && mkdir("player",0700) == 0 &&
           mkdir("backup",0700) == 0 && chdir("area") == 0);
    string_space = malloc(1024*1024); assert(string_space);
    top_string = string_space; *top_string = '\0'; current_time = 2000000000; init_mm();
    if (argc > 1) {
        if (!strcmp(argv[1],"forge")) forge();
        else if (!strcmp(argv[1],"sorcerer")) missing_sorcerer();
        else if (!strcmp(argv[1],"mage")) mage_costs();
        else if (!strcmp(argv[1],"equipment")) equipment();
        else abort();
    } else {
        missing_sorcerer(); craft_boundaries(); sorcerer(); forge();
        mage_costs(); equipment(); class_restrictions(); dragon_persistence();
    }
    fixture(CLASS_NONE);
    clan_registry_close();
    char_list = NULL; descriptor_list = NULL;
    if (fpReserve) { fclose(fpReserve); fpReserve = NULL; }
    unlink("../player/Gearhero"); unlink("../backup/Gearhero");
    assert(chdir("..") == 0 && rmdir("area") == 0 &&
           rmdir("player") == 0 && rmdir("backup") == 0);
    assert(chdir(original) == 0 && rmdir(root) == 0);
    puts("class gear: crafting/cost boundaries, enchantment, reforging, restrictions, "
         "overlapping equipment effects and Dragon persistence passed");
    return 0;
}
