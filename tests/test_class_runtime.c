/* Actual class handlers; synthetic service contracts. A lethal damage stub
 * unlinks and frees its target to expose post-damage use under ASan.
 * No production accounts, areas or objects are loaded. */
#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "../Genesis/src/merc.h"

static CHAR_DATA caster, opponent, *target;
static PC_DATA caster_pc, opponent_pc, target_pc;
static ROOM_INDEX_DATA home, remote;
static EXIT_DATA exit_north;
static OBJ_DATA crossbow;
static OBJ_DATA ammunition;
static OBJ_INDEX_DATA ammunition_index, crossbow_index;
static int spent_bolts;
static long attack_roll = 100;
static bool lethal, protected_target, missing_skill;
static bool relocate_caster;
static int hit_calls, damage_calls, poison_calls, observed_victim_lag;
static CHAR_DATA *last_target;
static int caster_moves;

char *one_argument(char *s, char *out)
{
    while (isspace((unsigned char)*s)) ++s;
    while (*s && !isspace((unsigned char)*s)) *out++ = *s++;
    *out = '\0';
    while (isspace((unsigned char)*s)) ++s;
    return s;
}
bool str_cmp(const char *a, const char *b) { return strcasecmp(a, b) != 0; }
void send_to_char(const char *s, CHAR_DATA *ch) { (void)s; (void)ch; }
void act(const char *s, CHAR_DATA *ch, const void *a, const void *b, long t)
{ (void)s; (void)ch; (void)a; (void)b; (void)t; }
long get_trust(CHAR_DATA *ch) { return ch->level; }
long get_monk(CHAR_DATA *ch, long p) { (void)ch; (void)p; return 8; }
long get_ninja(CHAR_DATA *ch, long p) { (void)ch; (void)p; return 10; }
long get_highland(CHAR_DATA *ch, long p) { (void)ch; (void)p; return 6; }
long get_breed(CHAR_DATA *ch, long p) { (void)ch; (void)p; return 0; }
long get_tribe(CHAR_DATA *ch, long p) { (void)ch; (void)p; return 5; }
bool get_drow(CHAR_DATA *ch, long p) { (void)ch; (void)p; return TRUE; }
long char_damroll(CHAR_DATA *ch) { (void)ch; return attack_roll; }
int char_stat(CHAR_DATA *ch, int stat) { (void)ch; (void)stat; return 2; }
int number_range(int a, int b) { assert(a <= b); return b; }
int number_percent(void) { return 100; }
bool is_same_group(CHAR_DATA *ch, CHAR_DATA *v) { return ch == v; }
bool is_safe(CHAR_DATA *ch, CHAR_DATA *v) { return protected_target || ch == v; }
bool can_see(CHAR_DATA *ch, CHAR_DATA *v)
{ (void)ch; (void)v; return TRUE; /* No hidden actors in this isolated fixture. */ }
bool check_avoid(CHAR_DATA *ch, CHAR_DATA *v, bool dt)
{ (void)ch; (void)v; (void)dt; return FALSE; }
bool hurt_fighting(CHAR_DATA *ch, CHAR_DATA *v)
{
    if (is_safe(ch, v)) return FALSE;
    if (!ch->fighting) ch->fighting = v;
    v->fighting = ch;
    return TRUE;
}
void char_from_room(CHAR_DATA *ch)
{
    if (ch == &caster) ++caster_moves;
    CHAR_DATA **p = &ch->in_room->people;
    while (*p && *p != ch) p = &(*p)->next_in_room;
    assert(*p == ch);
    *p = ch->next_in_room;
    ch->next_in_room = NULL;
    ch->in_room = NULL;
}
void char_to_room(CHAR_DATA *ch, ROOM_INDEX_DATA *room)
{
    if (ch == &caster) ++caster_moves;
    assert(ch->in_room == NULL);
    ch->in_room = room;
    ch->next_in_room = room->people;
    room->people = ch;
}
CHAR_DATA *get_char_room(CHAR_DATA *ch, char *name)
{
    CHAR_DATA *p;
    for (p = ch->in_room->people; p; p = p->next_in_room)
        if (!strcmp(p->name, name)) return p;
    return NULL;
}
static void strike(CHAR_DATA *ch, CHAR_DATA *v, long dam)
{
    bool found = FALSE;
    for (CHAR_DATA *p = ch->in_room->people; p; p = p->next_in_room)
        if (p == v) found = TRUE;
    assert(found && v->hit > 0 && v->position != POS_DEAD);
    last_target = v;
    observed_victim_lag = v->wait;
    if (lethal)
    {
        assert(v == target);
        char_from_room(v);
        if (caster.fighting == v) caster.fighting = NULL;
        free(v);
        target = NULL;
    }
    else
    {
        v->hit -= dam;
        if (relocate_caster)
        {
            char_from_room(ch);
            char_to_room(ch, &remote);
        }
    }
}
void one_hit(CHAR_DATA *ch, CHAR_DATA *v, long dt, char *style)
{ (void)dt; (void)style; ++hit_calls; strike(ch, v, 100); }
void hurt_person(CHAR_DATA *ch, CHAR_DATA *v, long dam)
{ ++damage_calls; strike(ch, v, dam); }
void do_stance(CHAR_DATA *ch, char *arg) { (void)ch; (void)arg; }
void do_majesty(CHAR_DATA *ch, char *arg)
{ (void)arg; REMOVE_BIT(ch->itemaffect, ITEMA_PEACE); }
long skill_lookup(const char *s) { (void)s; return missing_skill ? -1 : 1; }
void affect_to_char(CHAR_DATA *ch, AFFECT_DATA *af)
{ ++poison_calls; SET_BIT(ch->affected_by, af->bitvector); }
OBJ_DATA *get_eq_char(CHAR_DATA *ch, long wear)
{
    assert(ch == &caster);
    return wear == WEAR_HOLD || wear == WEAR_WEAPON ? &crossbow : NULL;
}

void extract_obj(OBJ_DATA *obj)
{
    assert(obj == &ammunition && caster.carrying == obj);
    caster.carrying = obj->next_content;
    ++spent_bolts;
}

static void fresh(long class)
{
    free(target);
    memset(&caster, 0, sizeof(caster));
    memset(&opponent, 0, sizeof(opponent));
    memset(&caster_pc, 0, sizeof(caster_pc));
    memset(&opponent_pc, 0, sizeof(opponent_pc));
    memset(&target_pc, 0, sizeof(target_pc));
    memset(&home, 0, sizeof(home));
    memset(&remote, 0, sizeof(remote));
    memset(&crossbow, 0, sizeof(crossbow));
    crossbow.value[3] = TYPE_PIERCE;
    memset(&exit_north, 0, sizeof(exit_north));
    target = calloc(1, sizeof(*target));
    assert(target);
    caster.pcdata = &caster_pc;
    opponent.pcdata = &opponent_pc;
    target->pcdata = &target_pc;
    caster.name = "caster"; opponent.name = "opponent"; target->name = "target";
    caster.class = class;
    caster.hit = caster.max_hit = opponent.hit = target->hit = 10000;
    opponent.max_hit = target->max_hit = 10000;
    caster.mana = 10000;
    caster.position = opponent.position = target->position = POS_STANDING;
    caster_pc.flag = opponent_pc.flag = target_pc.flag = FLAG_PK;
    caster_pc.conding = "";
    caster.fighting = &opponent;
    for (int i = ELEM_FIRE; i <= ELEM_INFIN; ++i) caster_pc.powers[i] = 8;
    caster_pc.dragskills = DRAG_BREATH | DRAG_TAIL | DRAG_DUST | DRAG_WINGSOUT;
    caster.polyaff = POLY_DRAGONFORM;
    char_to_room(target, &home);
    char_to_room(&opponent, &home);
    char_to_room(&caster, &home);
    lethal = protected_target = missing_skill = FALSE;
    relocate_caster = FALSE;
    hit_calls = damage_calls = poison_calls = observed_victim_lag = 0;
    last_target = NULL;
    caster_moves = 0;
}

static void test_chain(DO_FUN *fn, long class, int expected, long cost)
{
    fresh(class);
    fn(&caster, "target");
    assert(last_target == target && opponent.hit == 10000);
    assert(hit_calls == expected && caster.mana == 10000 - cost);
    fresh(class);
    lethal = TRUE;
    target->act = ACT_IS_NPC;
    target->pcdata = NULL;
    fn(&caster, "target");
    assert(target == NULL && hit_calls == 1);
    assert(caster.mana == 10000 - (fn == do_repeat ? 600 : cost));
    assert(caster.wait > 0);
    fresh(class);
    fn(&caster, "missing");
    assert(hit_calls == 0 && caster.mana == 10000 && caster.wait == 0);
    fresh(class);
    relocate_caster = TRUE;
    fn(&caster, "target");
    assert(hit_calls == 1 && caster.in_room == &remote);
}

static void test_stoneblast(void)
{
    fresh(CLASS_ELEM);
    char_from_room(&opponent);
    caster.fighting = NULL;
    do_stoneblast(&caster, "");
    assert(hit_calls == 2 && caster.mana == 7200 && caster.wait == 13);
    fresh(CLASS_ELEM);
    char_from_room(&opponent);
    caster.fighting = NULL;
    lethal = TRUE;
    target->act = ACT_IS_NPC;
    target->pcdata = NULL;
    do_stoneblast(&caster, "");
    assert(hit_calls == 1 && !target && caster.mana == 7200 && caster.wait == 13);
    fresh(CLASS_ELEM);
    char_from_room(&caster);
    char_to_room(&caster, &home); /* room order: caster, opponent, target */
    char_from_room(target);
    char_to_room(target, &home); /* first enemy target, then opponent */
    relocate_caster = TRUE;
    do_stoneblast(&caster, "");
    assert(hit_calls == 1 && caster.in_room == &remote && opponent.hit == 10000);
}

static void test_icicles(void)
{
    fresh(CLASS_ELEM);
    caster_pc.powers[ELEM_WATER] = 1;
    caster.mana = 149;
    do_iciclestorm(&caster, "target");
    assert(caster.mana == 149 && caster.wait == 0 && hit_calls == 0);
    caster.mana = 150;
    protected_target = TRUE;
    do_iciclestorm(&caster, "target");
    assert(caster.mana == 150 && caster.wait == 0 && hit_calls == 0);
    protected_target = FALSE;
    do_iciclestorm(&caster, "target");
    assert(caster.mana == 0 && caster.wait == 12 && hit_calls == 1);
    assert(last_target == target && opponent.hit == 10000);
}

static void test_control_death(void)
{
    fresh(CLASS_HIGHLANDER);
    lethal = TRUE;
    target->act = ACT_IS_NPC;
    target->pcdata = NULL;
    do_bash(&caster, "target");
    assert(!target && damage_calls == 1 && observed_victim_lag == 10);
    assert(caster.wait == 20 && opponent.hit == 10000);
    fresh(CLASS_WEREWOLF);
    lethal = TRUE;
    target->act = ACT_IS_NPC;
    target->pcdata = NULL;
    do_falling(&caster, "target");
    assert(!target && damage_calls == 1 && observed_victim_lag == 12);
    assert(caster.wait == 24);
}

static void shooting_fixture(void)
{
    fresh(CLASS_DROW);
    attack_roll = 100;
    caster.fighting = NULL;
    caster.affected_by |= AFF_HIDE;
    caster.more |= MORE_DARKNESS | MORE_SILENCE;
    caster_pc.classpoints[CP_CURRENT] = 1000;
    crossbow.item_type = ITEM_MISSILE;
    memset(&ammunition, 0, sizeof(ammunition));
    ammunition_index.vnum = OBJ_VNUM_DROW_BOLT;
    crossbow_index.vnum = OBJ_VNUM_DROW_CROSSBOW;
    crossbow.pIndexData = &crossbow_index;
    crossbow.value[0] = 9315; /* Existing saves must work without migration. */
    crossbow.value[3] = 3;
    ammunition.item_type = ITEM_AMMO;
    ammunition.pIndexData = &ammunition_index;
    ammunition.value[3] = 3;
    ammunition.wear_loc = WEAR_NONE;
    caster.carrying = &ammunition;
    spent_bolts = 0;
    exit_north.to_room = &remote;
    home.exit[0] = &exit_north;
    char_from_room(target);
    char_to_room(target, &remote);
}

static void assert_home(void)
{
    assert(caster_moves == 0);
    assert(IS_AFFECTED(&caster, AFF_HIDE));
    assert(IS_MORE(&caster, MORE_DARKNESS) && IS_MORE(&caster, MORE_SILENCE));
    assert(caster.in_room == &home && home.people == &caster);
    assert(caster.next_in_room == &opponent && opponent.next_in_room == NULL);
    for (CHAR_DATA *p = remote.people; p; p = p->next_in_room) assert(p != &caster);
}

static void test_crossbow(void)
{
    shooting_fixture();
    lethal = TRUE;
    target->act = ACT_IS_NPC;
    target->pcdata = NULL;
    do_shoot(&caster, "north target");
    assert(!target && damage_calls == 1 && poison_calls == 0);
    assert_home();
    assert(caster_pc.classpoints[CP_CURRENT] == 500);
    assert(spent_bolts == 1 && !caster.carrying);
    shooting_fixture();
    missing_skill = TRUE;
    do_shoot(&caster, "north target");
    assert(damage_calls == 1 && poison_calls == 0);
    assert_home();
    shooting_fixture();
    protected_target = TRUE;
    do_shoot(&caster, "north target");
    assert(damage_calls == 0 && poison_calls == 0);
    assert(spent_bolts == 0 && caster_pc.classpoints[CP_CURRENT] == 1000);
    assert_home();
    shooting_fixture();
    do_shoot(&caster, "north missing");
    assert(damage_calls == 0);
    assert_home();
    shooting_fixture();
    do_shoot(&caster, "north target");
    assert(damage_calls == 1 && poison_calls == 1 && target->position == POS_SLEEPING);
    assert_home();
    shooting_fixture();
    attack_roll = LONG_MAX;
    do_shoot(&caster, "north target");
    assert(damage_calls == 1 && spent_bolts == 1);
    assert_home();
    shooting_fixture();
    attack_roll = -1;
    do_shoot(&caster, "north target");
    assert(damage_calls == 1 && spent_bolts == 1);
    assert_home();
}

static void test_named_single(DO_FUN *fn, long class)
{
    fresh(class);
    fn(&caster, "target");
    assert(hit_calls == 1 && last_target == target && opponent.hit == 10000);
    fresh(class);
    fn(&caster, "missing");
    assert(hit_calls == 0 && caster.wait == 0);
}

static void test_combat_only_targeting(void)
{
    DO_FUN *handlers[] = {do_watercontrol, do_tsumazuku};
    long classes[] = {CLASS_ELEM, CLASS_NINJA};
    for (int i = 0; i < 2; ++i)
    {
        fresh(classes[i]); caster_pc.powers[ELEM_WATER] = 10;
        handlers[i](&caster, "target");
        assert(target->wait == (i == 0 ? 20 : 15) && opponent.wait == 0);
        assert(caster.mana == (i == 0 ? 6000 : 10000));
        fresh(classes[i]); caster_pc.powers[ELEM_WATER] = 10;
        handlers[i](&caster, "missing");
        assert(caster.wait == 0 && caster.mana == 10000 && opponent.wait == 0);
        protected_target = TRUE;
        handlers[i](&caster, "target");
        assert(caster.wait == 0 && caster.mana == 10000 && target->wait == 0);
        protected_target = FALSE;
        handlers[i](&caster, "caster");
        assert(caster.wait == 0 && caster.mana == 10000);
        caster.fighting = NULL;
        handlers[i](&caster, "target");
        assert(caster.wait == 0 && caster.mana == 10000 && target->wait == 0);
    }
}

int main(void)
{
    test_combat_only_targeting();
    test_chain(do_flarearrow, CLASS_ELEM, 3, 500);
    test_chain(do_windblades, CLASS_ELEM, 3, 800);
    test_chain(do_repeat, CLASS_MONK, 5, 3000);
    test_icicles();
    test_stoneblast();
    test_control_death();
    test_named_single(do_shuurai, CLASS_NINJA);
    test_named_single(do_dbreath, CLASS_DRAGON);
    test_named_single(do_stail, CLASS_DRAGON);
    test_named_single(do_acidbolt, CLASS_DROW);
    test_crossbow();
    free(target);
    puts("cross-class runtime regressions passed");
    return 0;
}
