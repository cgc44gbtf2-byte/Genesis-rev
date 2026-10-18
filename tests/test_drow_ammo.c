/* Real engine: individual projectiles, payment boundaries and old-save support. */
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "merc.h"

extern char *string_space, *top_string;
extern bool fBootDb;
extern void init_mm(void);
extern long get_cost(CHAR_DATA *keeper, OBJ_DATA *obj, bool buying);
static CHAR_DATA actor, target;
static PC_DATA pc;
static DESCRIPTOR_DATA descriptor;
static ROOM_INDEX_DATA home, away;
static EXIT_DATA north;
static AREA_DATA area;
static MOB_INDEX_DATA mobile;
static OBJ_INDEX_DATA indices[6];
static char output[65536];
static OBJ_DATA *crossbow;

static char *intern(const char *s)
{
    FILE *fp = tmpfile();
    assert(fp && fputs(s, fp) >= 0 && fputs("~", fp) >= 0);
    rewind(fp); fBootDb = TRUE;
    char *result = fread_string(fp);
    fBootDb = FALSE; fclose(fp);
    return result;
}

static void prototype(int n, long vnum, long type)
{
    OBJ_INDEX_DATA *p = &indices[n];
    p->vnum = vnum; p->item_type = type; p->weight = 7;
    p->wear_flags = ITEM_TAKE | ITEM_WEAR_HOLD;
    p->name = intern("gear bolt crossbow"); p->short_descr = intern("some gear");
    p->description = intern("Some gear is here.");
    p->value[0] = type == ITEM_MISSILE ? OBJ_VNUM_DROW_BOLT : 100;
    p->value[1] = 25; p->value[2] = 50; p->value[3] = 3;
    p->next = obj_index_hash[vnum % MAX_KEY_HASH];
    obj_index_hash[vnum % MAX_KEY_HASH] = p;
}

static void fixture(void)
{
    while (actor.carrying) extract_obj(actor.carrying);
    while (target.affected) affect_remove(&target, target.affected);
    free_string(actor.clan); free_string(actor.morph);
    memset(&pc, 0, sizeof(pc)); memset(&home, 0, sizeof(home));
    memset(&away, 0, sizeof(away)); memset(&north, 0, sizeof(north));
    memset(&descriptor, 0, sizeof(descriptor));
    clear_char(&actor); clear_char(&target);
    actor.name = "Boltuser"; actor.pcdata = &pc;
    actor.desc = &descriptor; actor.level = LEV_AWA; actor.class = CLASS_DROW;
    actor.vampgen = 9; actor.damroll = 10; actor.stance[0] = -1;
    actor.hit = actor.max_hit = actor.mana = actor.max_mana = 100000;
    actor.position = target.position = POS_STANDING;
    actor.in_room = &home; home.people = &actor;
    actor.affected_by = AFF_HIDE; actor.more = MORE_DARKNESS | MORE_SILENCE;
    actor.logon = current_time;
    target.name = "target"; target.short_descr = "a target";
    target.act = ACT_IS_NPC; target.pIndexData = &mobile;
    target.in_room = &away; away.people = &target;
    target.hit = target.max_hit = 100000; target.level = 1;
    home.area = away.area = &area; home.light = away.light = 1;
    home.vnum = 71000; away.vnum = 71001;
    north.to_room = &away; home.exit[DIR_NORTH] = &north;
    actor.next = &target; target.next = NULL; char_list = &actor;
    descriptor.character = &actor; descriptor.connected = CON_PLAYING;
    descriptor.outbuf = output; descriptor.outsize = sizeof(output);
    descriptor.fcommand = TRUE; descriptor_list = &descriptor;
    pc.pwd = pc.bamfin = pc.bamfout = pc.title = pc.decap_char = "";
    pc.cdest = pc.marriage = pc.email = pc.covenant = "";
    pc.cov_time = pc.leader = pc.vleader = pc.love = pc.conding = "";
    pc.cland[0] = pc.cland[1] = pc.cland[2] = -1;
    pc.classpoints[CP_CURRENT] = 1000;
    crossbow = create_object(&indices[0], 0);
    free_string(crossbow->questowner); crossbow->questowner = str_dup(actor.name);
    obj_to_char(crossbow, &actor); equip_char(&actor, crossbow, WEAR_HOLD);
    assert(crossbow->wear_loc == WEAR_HOLD);
}

static OBJ_DATA *bolt(int index)
{
    OBJ_DATA *ammo = create_object(&indices[index], 0);
    obj_to_char(ammo, &actor);
    return ammo;
}

static long bolts(CHAR_DATA *ch)
{
    long count = 0;
    for (OBJ_DATA *o = ch->carrying; o; o = o->next_content)
        if (o->pIndexData->vnum == OBJ_VNUM_DROW_BOLT) ++count;
    return count;
}

static void shoot(const char *text)
{
    char arg[MAX_INPUT_LENGTH];
    snprintf(arg, sizeof(arg), "%s", text);
    descriptor.outtop = 0; output[0] = '\0';
    do_shoot(&actor, arg);
    assert(actor.in_room == &home && home.people == &actor &&
           actor.next_in_room == NULL && away.people == &target &&
           target.next_in_room == NULL);
    assert(IS_AFFECTED(&actor, AFF_HIDE) &&
           IS_MORE(&actor, MORE_DARKNESS) && IS_MORE(&actor, MORE_SILENCE));
}

static void dry_and_success(void)
{
    fixture(); shoot("north target");
    assert(pc.classpoints[CP_CURRENT] == 1000 && target.hit == 100000);
    assert(strstr(output, "compatible") != NULL);
    fixture(); bolt(1); bolt(1);
    shoot("n target");
    assert(pc.classpoints[CP_CURRENT] == 500 && bolts(&actor) == 1);
    assert(target.hit < 100000 && IS_AFFECTED(&target, AFF_SLEEP));
    shoot("north target");
    assert(pc.classpoints[CP_CURRENT] == 0 && bolts(&actor) == 0);
    shoot("north target");
    assert(pc.classpoints[CP_CURRENT] == 0);
    fixture(); bolt(1); pc.classpoints[CP_CURRENT] = 499;
    shoot("north target");
    assert(bolts(&actor) == 1 && pc.classpoints[CP_CURRENT] == 499);
    fixture(); bolt(1); actor.class = CLASS_MAGE;
    shoot("north target");
    assert(bolts(&actor) == 1 && pc.classpoints[CP_CURRENT] == 1000);
    fixture(); bolt(1); unequip_char(&actor, crossbow);
    shoot("north target");
    assert(bolts(&actor) == 1 && pc.classpoints[CP_CURRENT] == 1000);
    fixture(); bolt(1); actor.in_room = NULL;
    do_shoot(&actor, "north target");
    assert(pc.classpoints[CP_CURRENT] == 1000 && bolts(&actor) == 1);
    actor.in_room = &home;
}

static void rejection_and_misses(void)
{
    const char *invalid[] = {"", "north", "sideways target", "north Boltuser",
                            "north target extra"};
    for (int i = 0; i < 5; ++i) {
        fixture(); bolt(1); shoot(invalid[i]);
        assert(bolts(&actor) == 1 && pc.classpoints[CP_CURRENT] == 1000);
    }
    for (int i = 0; i < 3; ++i) {
        fixture(); bolt(1);
        if (i == 0) SET_BIT(home.room_flags, ROOM_SAFE);
        if (i == 1) SET_BIT(target.act, ACT_NOKILL);
        if (i == 2) SET_BIT(away.room_flags, ROOM_SAFE);
        shoot("north target");
        assert(bolts(&actor) == 1 && pc.classpoints[CP_CURRENT] == 1000 &&
               target.hit == 100000);
    }
    const long barriers[] = {EX_CLOSED, EX_WALL_GRANITE, EX_WALL_WATER,
                             EX_WALL_WIND, EX_WALL_FLAME};
    for (int i = 0; i < 7; ++i) {
        fixture(); bolt(1);
        if (i == 0) home.exit[DIR_NORTH] = NULL;
        if (i >= 1 && i <= 5) SET_BIT(north.exit_info, barriers[i - 1]);
        shoot(i == 6 ? "north absent" : "north target");
        assert(bolts(&actor) == 0 && pc.classpoints[CP_CURRENT] == 500 &&
               target.hit == 100000);
    }
}

static void ownership_and_compatibility(void)
{
    for (int i = 0; i < 9; ++i) {
        fixture(); OBJ_DATA *ammo = bolt(1);
        if (i == 0) ammo->item_type = ITEM_TRASH;
        if (i == 1) ammo->pIndexData = &indices[3];
        if (i == 2) ammo->value[3] = 2;
        if (i == 3) {
            free_string(ammo->questowner); ammo->questowner = str_dup("Other");
        }
        if (i == 4) ammo->wear_loc = WEAR_BODY;
        if (i == 5) ammo->chobj = &target;
        if (i == 6) obj_to_obj(create_object(&indices[4], 0), ammo);
        if (i == 7) SET_BIT(ammo->quest, QUEST_ARTIFACT);
        if (i == 8) ammo->pIndexData = NULL;
        shoot("north target");
        assert(pc.classpoints[CP_CURRENT] == 1000 && target.hit == 100000);
        ammo->pIndexData = &indices[1]; ammo->chobj = NULL;
        ammo->wear_loc = WEAR_NONE;
    }
    fixture(); OBJ_DATA *ammo = bolt(1);
    obj_from_char(ammo);
    OBJ_DATA *bag = create_object(&indices[5], 0); obj_to_char(bag, &actor);
    obj_to_obj(ammo, bag); shoot("north target");
    assert(pc.classpoints[CP_CURRENT] == 1000 && ammo->in_obj == bag);
    fixture(); bolt(1); ammo = bolt(1);
    free_string(ammo->questowner); ammo->questowner = str_dup("Other");
    shoot("north target");
    assert(pc.classpoints[CP_CURRENT] == 500 && bolts(&actor) == 1 &&
           actor.carrying == ammo);
    fixture(); ammo = bolt(1);
    free_string(ammo->questowner); ammo->questowner = str_dup("BOLTUSER");
    shoot("north target"); assert(bolts(&actor) == 0);
    fixture(); crossbow->value[0] = 9315; bolt(1);
    shoot("north target");
    assert(bolts(&actor) == 0 && crossbow->value[0] == 9315);
    fixture(); crossbow->value[0] = 9315; ammo = bolt(2);
    shoot("north target");
    assert(pc.classpoints[CP_CURRENT] == 1000 && actor.carrying == ammo);
    fixture(); crossbow->pIndexData = &indices[3]; crossbow->value[0] = 9315;
    bolt(2); shoot("north target"); /* A custom bow honors its own configured ammo. */
    assert(pc.classpoints[CP_CURRENT] == 500);
    crossbow->pIndexData = &indices[0];
    fixture(); crossbow->value[0] = -1; bolt(1);
    shoot("north target");
    assert(pc.classpoints[CP_CURRENT] == 1000 && bolts(&actor) == 1);
    fixture(); crossbow->value[3] = -1; bolt(1);
    shoot("north target");
    assert(pc.classpoints[CP_CURRENT] == 1000 && bolts(&actor) == 1);
}

static void crafting_and_persistence(void)
{
    fixture();
    while (actor.carrying) extract_obj(actor.carrying);
    indices[0].value[0] = 9315;
    pc.classpoints[CP_CURRENT] = 8000; pc.quest = 400;
    do_drowarmor(&actor, "crossbow");
    assert(actor.carrying && actor.carrying->value[0] == OBJ_VNUM_DROW_BOLT &&
           pc.classpoints[CP_CURRENT] == 0 && pc.quest == 0 &&
           indices[0].value[0] == 9315);
    while (actor.carrying) extract_obj(actor.carrying);
    indices[0].value[0] = 72099; /* Do not overwrite operator customizations. */
    pc.classpoints[CP_CURRENT] = 8000; pc.quest = 400;
    do_drowarmor(&actor, "crossbow");
    assert(actor.carrying && actor.carrying->value[0] == 72099);
    indices[0].value[0] = OBJ_VNUM_DROW_BOLT;
    fixture(); crossbow->value[0] = 9315; bolt(1); bolt(1);
    shoot("north absent");
    assert(save_char_obj_checked(&actor));
    DESCRIPTOR_DATA loaded = {0};
    assert(load_char_obj(&loaded, "Boltuser"));
    CHAR_DATA *ch = loaded.character;
    assert(bolts(ch) == 1 && ch->pcdata->classpoints[CP_CURRENT] == 500);
    OBJ_DATA *weapon = get_eq_char(ch, WEAR_HOLD);
    assert(weapon && weapon->value[0] == 9315);
    while (ch->carrying) extract_obj(ch->carrying);
    free_char(ch);
}

static void craft(const char *text)
{
    char argument[MAX_INPUT_LENGTH];
    snprintf(argument, sizeof(argument), "%s", text);
    descriptor.outtop = 0; output[0] = '\0';
    do_drowarmor(&actor, argument);
}

static void assert_no_craft(const char *text)
{
    long dp = pc.classpoints[CP_CURRENT], quest = pc.quest;
    long count = actor.carry_number, weight = actor.carry_weight;
    craft(text);
    assert(pc.classpoints[CP_CURRENT] == dp && pc.quest == quest &&
           actor.carry_number == count && actor.carry_weight == weight &&
           bolts(&actor) == 0);
}

static void bolt_crafting(void)
{
    fixture();
    actor.primal = 7; actor.gold = 11; pc.quest = 9;
    pc.classpoints[CP_TOTAL] = 23;
    craft("bolts");
    assert(bolts(&actor) == 1 && pc.classpoints[CP_CURRENT] == 900 &&
           actor.primal == 7 && actor.gold == 11 && pc.quest == 9 &&
           pc.classpoints[CP_TOTAL] == 23);
    OBJ_DATA *ammo = actor.carrying;
    assert(ammo->item_type == ITEM_AMMO && ammo->cost == 0 && ammo->points == 0 &&
           !str_cmp(ammo->questmaker, actor.name) && !ammo->questowner[0]);
    SHOP_DATA shop = {0};
    MOB_INDEX_DATA seller_index = {0};
    CHAR_DATA seller = {0};
    shop.buy_type[0] = ITEM_AMMO; shop.profit_sell = 50;
    seller_index.pShop = &shop; seller.pIndexData = &seller_index;
    assert(get_cost(&seller, ammo, FALSE) == 0);
    shoot("north target"); /* Default recipe produces usable ammunition. */
    assert(bolts(&actor) == 0 && pc.classpoints[CP_CURRENT] == 400);
    fixture(); pc.quest = 9; actor.gold = 11; craft("bolt");
    ammo = actor.carrying;
    obj_from_char(ammo); obj_to_room(ammo, &home);
    do_sacrifice(&actor, "gear");
    assert(home.contents == NULL && pc.quest == 9 && actor.gold == 11 &&
           pc.classpoints[CP_CURRENT] == 900);

    fixture(); pc.classpoints[CP_CURRENT] = 2000;
    craft("bolt 20");
    assert(bolts(&actor) == 20 && pc.classpoints[CP_CURRENT] == 0 &&
           actor.carry_number == 21 && actor.carry_weight == 21 * indices[1].weight);
    fixture(); pc.classpoints[CP_CURRENT] = 1999;
    assert_no_craft("bolts 20"); /* No partial batch or partial deduction. */
    fixture(); pc.classpoints[CP_CURRENT] = 99;
    assert_no_craft("bolt");
    const char *invalid[] = {"bolts 0", "bolts -1", "bolts +1", "bolts 21",
        "bolts 1.5", "bolts banana", "bolts 2 extra",
        "bolts 99999999999999999999999999999999"};
    for (size_t i = 0; i < sizeof(invalid)/sizeof(invalid[0]); ++i) {
        fixture(); assert_no_craft(invalid[i]);
    }
    fixture(); actor.class = CLASS_MAGE;
    assert_no_craft("bolts");
    fixture(); SET_BIT(actor.act, ACT_IS_NPC); actor.pcdata = NULL;
    assert_no_craft("bolts"); actor.pcdata = &pc; REMOVE_BIT(actor.act, ACT_IS_NPC);

    fixture();
    long hash = OBJ_VNUM_DROW_BOLT % MAX_KEY_HASH;
    assert(obj_index_hash[hash] == &indices[1]);
    obj_index_hash[hash] = indices[1].next;
    assert_no_craft("bolts");
    obj_index_hash[hash] = &indices[1];
    for (int i = 0; i < 4; ++i) {
        fixture();
        long type = indices[1].item_type, weight = indices[1].weight;
        long flags = indices[1].wear_flags, caliber = indices[1].value[3];
        if (i == 0) indices[1].item_type = ITEM_TRASH;
        if (i == 1) indices[1].weight = -1;
        if (i == 2) indices[1].wear_flags = 0;
        if (i == 3) indices[1].value[3] = -1;
        assert_no_craft("bolts");
        indices[1].item_type = type; indices[1].weight = weight;
        indices[1].wear_flags = flags; indices[1].value[3] = caliber;
    }
    fixture();
    long count = actor.carry_number, weight = actor.carry_weight;
    actor.carry_number = LONG_MAX;
    assert_no_craft("bolts"); actor.carry_number = count;
    actor.carry_weight = LONG_MAX;
    assert_no_craft("bolts"); actor.carry_weight = weight;

    fixture(); craft("bolts 3");
    actor.name = "Friend"; /* Supplies remain tradable; maker isn't an owner lock. */
    shoot("north target");
    actor.name = "Boltuser";
    assert(bolts(&actor) == 2 && pc.classpoints[CP_CURRENT] == 200);
    assert(save_char_obj_checked(&actor));
    DESCRIPTOR_DATA loaded = {0};
    assert(load_char_obj(&loaded, "Boltuser"));
    assert(bolts(loaded.character) == 2 &&
           loaded.character->pcdata->classpoints[CP_CURRENT] == 200);
    for (OBJ_DATA *o = loaded.character->carrying; o; o = o->next_content)
        if (o->item_type == ITEM_AMMO)
            assert(o->cost == 0 && o->points == 0 && !o->questowner[0] &&
                   !str_cmp(o->questmaker, "Boltuser"));
    while (loaded.character->carrying) extract_obj(loaded.character->carrying);
    free_char(loaded.character);
}

int main(void)
{
    char root[] = "/tmp/anathema-ammo-XXXXXX", original[4096];
    assert(getcwd(original, sizeof(original)) && mkdtemp(root) && chdir(root) == 0);
    assert(mkdir("area",0700) == 0 && mkdir("player",0700) == 0 &&
           mkdir("backup",0700) == 0 && chdir("area") == 0);
    string_space = malloc(1024*1024); assert(string_space);
    top_string = string_space; *top_string = '\0'; current_time = 2000000000; init_mm();
    prototype(0, OBJ_VNUM_DROW_CROSSBOW, ITEM_MISSILE);
    prototype(1, OBJ_VNUM_DROW_BOLT, ITEM_AMMO);
    prototype(2, 9315, ITEM_AMMO); prototype(3, 72010, ITEM_MISSILE);
    prototype(4, 72011, ITEM_TRASH); prototype(5, 72012, ITEM_CONTAINER);
    mobile.short_descr = "a target";
    dry_and_success(); rejection_and_misses(); ownership_and_compatibility();
    bolt_crafting(); crafting_and_persistence();
    fixture();
    while (actor.carrying) extract_obj(actor.carrying);
    clan_registry_close(); char_list = NULL; descriptor_list = NULL;
    if (fpReserve) { fclose(fpReserve); fpReserve = NULL; }
    unlink("../player/Boltuser"); unlink("../backup/Boltuser");
    assert(chdir("..") == 0 && rmdir("area") == 0 &&
           rmdir("player") == 0 && rmdir("backup") == 0 &&
           chdir(original) == 0 && rmdir(root) == 0);
    puts("Drow ammunition: costs, compatibility, ownership, miss/safety boundaries, "
         "room invariants, bounded crafting, trading/resale and old-save persistence passed");
    return 0;
}
