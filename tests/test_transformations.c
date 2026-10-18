/* Real-engine lifecycle tests; files are confined to a private scratch tree. */
#define ATOMIC_SAVE_TESTING
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "merc.h"
#include "atomic_save.h"

extern void fread_char(CHAR_DATA *, FILE *);
extern void fread_obj(CHAR_DATA *, FILE *);
extern void init_mm(void);
extern char *string_space, *top_string;
extern bool fBootDb;
static CHAR_DATA actor;
static PC_DATA player;
static DESCRIPTOR_DATA descriptor;
static ROOM_INDEX_DATA room;
static AREA_DATA area;
static char output[65536];
static OBJ_INDEX_DATA armor_index;
static const struct form_case {
    const char *enter, *leave, *morph;
    long flag;
    DO_FUN *command;
} forms[] = {
    {"bat", "human", "A black vampire bat", POLY_BAT, do_change},
    {"wolf", "human", "A dire wolf", POLY_WOLF, do_change},
    {"mist", "human", "A white mist", POLY_MIST, do_change},
    {"", "", "A huge serpent", POLY_SERPENT, do_serpent},
    {"", "", "A pool of shadow", POLY_SHADOW, do_shadowbody},
    {"", "", "Dragon", POLY_DRAGONFORM, do_dragonform}
};

static void reset(int which)
{
    while (actor.carrying != NULL) extract_obj(actor.carrying);
    while (actor.affected != NULL) affect_remove(&actor, actor.affected);
    if (actor.morph != NULL) free_string(actor.morph);
    memset(&player, 0, sizeof(player));
    memset(&descriptor, 0, sizeof(descriptor));
    memset(&room, 0, sizeof(room));
    memset(&area, 0, sizeof(area));
    memset(output, 0, sizeof(output));
    clear_char(&actor);
    actor.name = "Lifecycle";
    actor.pcdata = &player;
    actor.class = which == 5 ? CLASS_DRAGON : CLASS_VAMPIRE;
    actor.level = LEV_AWA;
    actor.vampgen = which == 5 ? AGE_YOUNG : 9;
    actor.contingency = "";
    actor.hit = actor.max_hit = actor.mana = actor.max_mana = 10000;
    actor.blood[0] = actor.blood[1] = 100;
    actor.primal = 1234;
    actor.logon = actor.save_time = current_time;
    actor.stance[0] = -1;
    actor.style[0] = STYLE_UNARMED;
    actor.in_room = &room;
    room.vnum = 900;
    room.area = &area;
    room.people = &actor;
    room.light = 1;
    char_list = &actor;
    descriptor_list = &descriptor;
    actor.desc = &descriptor;
    descriptor.character = &actor;
    descriptor.connected = CON_PLAYING;
    descriptor.outbuf = output;
    descriptor.outsize = sizeof(output);
    player.pwd = player.bamfin = player.bamfout = "";
    player.cdest = player.title = player.decap_char = "";
    player.marriage = player.email = player.covenant = "";
    player.cov_time = player.leader = player.vleader = "";
    player.love = player.conding = "";
    player.cland[0] = player.cland[1] = player.cland[2] = -1;
    player.powers[DISC_PROTEAN] = 4;
    player.powers[DISC_SERPENTIS] = 4;
    player.powers[DISC_OBTENEBRATION] = 3;
    player.dragcol = RED_MAGIC;
    atomic_save_test_fault = SAVE_FAULT_NONE;
}

static void command(int which, int entering)
{
    char argument[MAX_INPUT_LENGTH];
    snprintf(argument, sizeof(argument), "%s",
             entering ? forms[which].enter : forms[which].leave);
    forms[which].command(&actor, argument);
}

static void assert_live(int which, int transformed)
{
    assert(!!IS_AFFECTED(&actor, AFF_POLYMORPH) == transformed);
    assert(!!IS_POLYAFF(&actor, forms[which].flag) == transformed);
    if (transformed) assert(strstr(actor.morph, forms[which].morph) != NULL);
    else assert(!strcmp(actor.morph, ""));
    if (which == 0)
        assert(!!IS_VAMPAFF(&actor, VAM_FLYING) == transformed &&
               !!IS_VAMPAFF(&actor, VAM_SONIC) == transformed);
    if (which == 2 || which == 4)
        assert(!!IS_AFFECTED(&actor, AFF_ETHEREAL) == transformed);
    assert(actor.hit == 10000 && actor.mana == 10000 &&
           actor.blood[0] == 100 && actor.primal == 1234);
    assert(actor.affected == NULL && actor.armor == 100 &&
           actor.hitroll == 0 && actor.damroll == 0);
}

static void assert_saved(const char *path, long poly, long aff, long vamp,
                         const char *morph, long atm, long skills)
{
    FILE *fp = fopen(path, "r");
    CHAR_DATA loaded;
    PC_DATA pc;
    assert(fp != NULL && !strcmp(fread_word(fp), "#PLAYER"));
    clear_char(&loaded);
    memset(&pc, 0, sizeof(pc));
    loaded.pcdata = &pc;
    fread_char(&loaded, fp);
    assert(fclose(fp) == 0);
    assert(loaded.polyaff == poly);
    assert(loaded.affected_by == aff);
    assert(loaded.vampaff == vamp);
    assert(!strcmp(loaded.morph, morph));
    assert(pc.atm == atm);
    assert(pc.dragskills == skills);
    assert(loaded.hit == 10000 && loaded.mana == 10000 &&
           loaded.blood[0] == 100 && loaded.primal == 1234);
}

static void assert_current_saved(void)
{
    assert_saved("../player/Lifecycle", actor.polyaff, actor.affected_by,
                 actor.vampaff, actor.morph, player.atm, player.dragskills);
    assert_saved("../backup/Lifecycle", actor.polyaff, actor.affected_by,
                 actor.vampaff, actor.morph, player.atm, player.dragskills);
}

static void give_equipment_and_buff(void)
{
    OBJ_DATA *obj = create_object(&armor_index, 1);
    AFFECT_DATA af = {0};
    obj_to_char(obj, &actor);
    equip_char(&actor, obj, WEAR_BODY);
    assert(obj->wear_loc == WEAR_BODY);
    af.type = skill_lookup("armor");
    assert(af.type > 0);
    af.duration = 5;
    af.location = APPLY_HITROLL;
    af.modifier = 7;
    af.bitvector = AFF_DETECT_INVIS;
    affect_to_char(&actor, &af);
    assert(actor.affected != NULL && actor.hitroll == 7);
}

static void assert_saved_equipment(const char *path)
{
    CHAR_DATA loaded;
    PC_DATA pc;
    FILE *fp = fopen(path, "r");
    assert(fp != NULL && !strcmp(fread_word(fp), "#PLAYER"));
    clear_char(&loaded);
    memset(&pc, 0, sizeof(pc));
    loaded.pcdata = &pc;
    fread_char(&loaded, fp);
    assert(!strcmp(fread_word(fp), "#OBJECT"));
    fread_obj(&loaded, fp);
    assert(fclose(fp) == 0);
    assert(loaded.affected == NULL);
    assert(loaded.carrying != NULL &&
           loaded.carrying->pIndexData == &armor_index &&
           loaded.carrying->wear_loc == WEAR_NONE);
    extract_obj(loaded.carrying);
    assert(loaded.carrying == NULL);
}

static void test_roundtrips(int characterize)
{
    for (int i = 0; i < 6; ++i)
    {
        reset(i);
        give_equipment_and_buff();
        command(i, 1);
        assert_live(i, 1);
        assert(actor.carrying != NULL &&
               actor.carrying->wear_loc == WEAR_NONE);
        if (!characterize)
        {
            assert_current_saved();
            assert_saved_equipment("../player/Lifecycle");
            assert_saved_equipment("../backup/Lifecycle");
        }
        command(i, 0);
        assert_live(i, 0);
        if (!characterize) assert_current_saved();
    }
}

static void test_reset_boundary_and_legacy_wrapper(void)
{
    reset(0);
    do_autosave(&actor, "");
    long old_time = actor.save_time;
    give_equipment_and_buff();
    actor.polyaff = POLY_BAT; /* Deliberately incomplete state must not save. */
    ++current_time;
    reset_form_stats(&actor);
    assert(actor.save_time == old_time && actor.affected == NULL &&
           actor.carrying->wear_loc == WEAR_NONE && actor.hitroll == 0);
    assert_saved("../player/Lifecycle", 0, 0, 0, "", 0, 0);
    assert_saved("../backup/Lifecycle", 0, 0, 0, "", 0, 0);
    actor.polyaff = 0;
    clear_stats(&actor); /* Existing extension-facing contract still saves. */
    assert(actor.save_time == current_time);
    assert_current_saved();
    reset_form_stats(NULL);
}

static void test_dragon_features(void)
{
    reset(5);
    player.dragskills = DRAG_CLAWS | DRAG_HORNS | DRAG_WINGS |
                       DRAG_FANGS | DRAG_NIGHT;
    command(5, 1);
    assert(IS_DRAGAFF(&actor, DRAG_HORNSOUT) &&
           IS_DRAGAFF(&actor, DRAG_WINGSOUT) &&
           IS_VAMPAFF(&actor, VAM_CLAWS) &&
           IS_VAMPAFF(&actor, VAM_FANGS) &&
           IS_VAMPAFF(&actor, VAM_NIGHTSIGHT));
    assert_current_saved();
    command(5, 0);
    assert_live(5, 0);
    assert(!IS_DRAGAFF(&actor, DRAG_HORNSOUT) &&
           !IS_DRAGAFF(&actor, DRAG_WINGSOUT) &&
           !IS_VAMPAFF(&actor, VAM_CLAWS) &&
           !IS_VAMPAFF(&actor, VAM_FANGS) &&
           !IS_VAMPAFF(&actor, VAM_NIGHTSIGHT));
    assert_current_saved();
}

static void test_rejected_class_and_npc(void)
{
    for (int i = 0; i < 6; ++i)
    {
        reset(i);
        do_autosave(&actor, "");
        long old_time = actor.save_time;
        ++current_time;
        actor.class = CLASS_NONE;
        actor.hitroll = 7;
        command(i, 1);
        assert(actor.hitroll == 7 && actor.polyaff == 0 &&
               actor.save_time == old_time && !strcmp(actor.morph, ""));
        actor.act = ACT_IS_NPC;
        actor.pcdata = NULL;
        command(i, 1);
        assert(actor.hitroll == 7 && actor.polyaff == 0 &&
               actor.save_time == old_time);
        actor.pcdata = &player;
    }
}

static void test_failed_save_retries(void)
{
    const enum atomic_save_fault faults[] = {
        SAVE_FAULT_CREATE, SAVE_FAULT_FLUSH, SAVE_FAULT_SYNC,
        SAVE_FAULT_CLOSE, SAVE_FAULT_RENAME, SAVE_FAULT_DIRSYNC
    };
    for (int i = 0; i < 6; ++i)
        for (size_t f = 0; f < sizeof(faults)/sizeof(faults[0]); ++f)
            for (int entering = 0; entering < 2; ++entering)
            {
                reset(i);
                if (!entering) command(i, 1);
                do_autosave(&actor, "");
                long poly = actor.polyaff, aff = actor.affected_by;
                long vamp = actor.vampaff, atm = player.atm;
                long skills = player.dragskills;
                char morph[MAX_STRING_LENGTH];
                snprintf(morph, sizeof(morph), "%s", actor.morph);
                atomic_save_test_fault = faults[f];
                command(i, entering);
                assert_live(i, entering);
                if (faults[f] == SAVE_FAULT_DIRSYNC)
                    assert_current_saved(); /* Rename visible, durability uncertain. */
                else
                {
                    assert_saved("../player/Lifecycle", poly, aff, vamp, morph, atm, skills);
                    assert_saved("../backup/Lifecycle", poly, aff, vamp, morph, atm, skills);
                }
                atomic_save_test_fault = SAVE_FAULT_NONE;
                do_autosave(&actor, "");
                assert_current_saved();
            }
}

static void test_independent_save_destinations(void)
{
    /* The two files are independently atomic, not a two-file transaction. */
    const char *destinations[] = {"../player", "../backup"};
    for (int d = 0; d < 2; ++d)
    {
        reset(0);
        do_autosave(&actor, "");
        assert(rename(destinations[d], "../unavailable") == 0);
        command(0, 1);
        assert_live(0, 1);
        assert_saved("../unavailable/Lifecycle", 0, 0, 0, "", 0, 0);
        assert_saved(d == 0 ? "../backup/Lifecycle" : "../player/Lifecycle",
                     actor.polyaff, actor.affected_by, actor.vampaff,
                     actor.morph, player.atm, player.dragskills);
        assert(rename("../unavailable", destinations[d]) == 0);
        do_autosave(&actor, "");
        assert_current_saved();
    }
}

int main(int argc, char **argv)
{
    char root[] = "/tmp/anathema-forms-XXXXXX", cwd[4096];
    int characterize = argc == 2 && !strcmp(argv[1], "characterize");
    assert(getcwd(cwd, sizeof(cwd)) && mkdtemp(root));
    assert(chdir(root) == 0 && mkdir("player", 0700) == 0 &&
           mkdir("backup", 0700) == 0 && mkdir("run", 0700) == 0);
    assert(chdir("run") == 0);
    string_space = malloc(4 * 1024 * 1024);
    assert(string_space != NULL);
    top_string = string_space;
    current_time = 123456;
    init_mm();
    leader_board.bestpk_number = leader_board.tt_number = LONG_MAX;
    leader_board.worstpk_number = LONG_MIN;
    armor_index.vnum = 905;
    /* Real prototype strings live in the shared read-only string pool;
     * create_object borrows them rather than owning string literals. */
    FILE *prototype_text = tmpfile();
    assert(prototype_text != NULL);
    fputs("lifecycle armor~", prototype_text);
    rewind(prototype_text);
    fBootDb = TRUE;
    armor_index.name = armor_index.short_descr = fread_string(prototype_text);
    fBootDb = FALSE;
    assert(fclose(prototype_text) == 0);
    armor_index.description = str_empty;
    armor_index.item_type = ITEM_ARMOR;
    armor_index.wear_flags = ITEM_TAKE | ITEM_WEAR_BODY;
    armor_index.value[0] = 20;
    obj_index_hash[armor_index.vnum % MAX_KEY_HASH] = &armor_index;
    test_roundtrips(characterize);
    if (!characterize)
    {
        test_reset_boundary_and_legacy_wrapper();
        test_dragon_features();
        test_rejected_class_and_npc();
        test_failed_save_retries();
        test_independent_save_destinations();
    }
    reset(0);
    unlink("../player/Lifecycle");
    unlink("../backup/Lifecycle");
    assert(chdir("..") == 0 && rmdir("player") == 0 &&
           rmdir("backup") == 0 && rmdir("run") == 0);
    assert(chdir(cwd) == 0 && rmdir(root) == 0);
    if (fpReserve != NULL) fclose(fpReserve);
    puts(characterize ? "transformation in-memory characterization passed" :
         "transformation persistence/failure/retry regressions passed");
    return 0;
}
