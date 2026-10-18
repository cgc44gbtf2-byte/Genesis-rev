/*
 * Isolated actual-function regressions, synthetic fixtures only.
 * From mudfix:
 * cc -std=gnu17 -O1 -g -ffunction-sections -fdata-sections \
 *   tests/test_powers.c Genesis/src/{ninja,monk,elemental,sorcerer,drow}.c \
 *   -Wl,--gc-sections \
 *   -o /tmp/test_powers && /tmp/test_powers
 * For sanitizers add -fsanitize=address,undefined -fno-omit-frame-pointer.
 * Unused spell-chain dependencies fail explicitly if reached.
 */
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "../Genesis/src/merc.h"

/* This isolated power fixture has no Highlander guard; the real-engine
 * Counter suite exercises guarded contact attacks and their side effects. */
bool highlander_counter_parry(CHAR_DATA *ch, CHAR_DATA *victim,
                             OBJ_DATA *weapon, long dt)
{
    (void)ch; (void)victim; (void)weapon; (void)dt;
    return FALSE;
}

void adv_spell_damage(CHAR_DATA *ch, OBJ_DATA *book, OBJ_DATA *page,
                      char *argument, int lastcolor);

static CHAR_DATA chars[5];
static PC_DATA players[5];
static ROOM_INDEX_DATA room;
static long dealt[5];
static int hits[5], safe_calls;
static bool protected_target, allow_fighting, group_members;
static char last_message[MAX_STRING_LENGTH];
long chainspell;

char *one_argument(char *argument, char *word)
{
    while (isspace((unsigned char)*argument)) ++argument;
    while (*argument && !isspace((unsigned char)*argument))
        *word++ = tolower((unsigned char)*argument++);
    *word = '\0';
    while (isspace((unsigned char)*argument)) ++argument;
    return argument;
}

bool str_cmp(const char *a, const char *b) { return strcasecmp(a, b) != 0; }
void send_to_char(const char *text, CHAR_DATA *ch)
{
    (void)ch;
    snprintf(last_message, sizeof(last_message), "%s", text);
}
void act(const char *text, CHAR_DATA *ch, const void *a, const void *b, long type)
{
    (void)text; (void)ch; (void)a; (void)b; (void)type;
}
void act2(const char *text, CHAR_DATA *ch, const void *a, const void *b, long type)
{
    act(text, ch, a, b, type);
}
long get_trust(CHAR_DATA *ch) { (void)ch; return 0; }
long get_ninja(CHAR_DATA *ch, long power)
{
    assert(ch == &chars[0] && (power == NINJA_SORA || power == NINJA_CHIKYU));
    return 10;
}
long get_monk(CHAR_DATA *ch, long power)
{
    assert(ch == &chars[0] && power == MONK_MANTRA);
    return players[0].monk[MONK_MANTRA] ? players[0].monk[MONK_MANTRA] : 5;
}
bool get_drow(CHAR_DATA *ch, long power)
{
    assert(ch == &chars[0] && power == DROW_SKIP);
    return TRUE;
}
long get_color(CHAR_DATA *ch, long color)
{
    assert(ch == &chars[0] && color == PURPLE_MAGIC);
    return 10;
}
CHAR_DATA *get_char_room(CHAR_DATA *ch, char *name)
{
    assert(ch == &chars[0] && !strcmp(name, "target"));
    return &chars[1];
}
bool is_safe(CHAR_DATA *ch, CHAR_DATA *victim)
{
    assert(ch == &chars[0] && victim != ch);
    ++safe_calls;
    return protected_target;
}
bool is_same_group(CHAR_DATA *ch, CHAR_DATA *victim)
{
    return ch == victim || group_members;
}
bool hurt_fighting(CHAR_DATA *ch, CHAR_DATA *victim)
{
    assert(ch == &chars[0] && victim != ch);
    return allow_fighting && !protected_target;
}
void hurt_person(CHAR_DATA *ch, CHAR_DATA *victim, long damage)
{
    int i = (int)(victim - chars);
    assert(ch == &chars[0] && i > 0 && i < 5);
    dealt[i] = damage;
    ++hits[i];
}
void set_fighting(CHAR_DATA *ch, CHAR_DATA *victim) { ch->fighting = victim; }
int number_range(int from, int to)
{
    if (from == 400 && to == 600) return 600;
    if (from == 100 && to == 140) return 120;
    if (from == 1 && to == 500) return 200;
    if (from == 1 && to == 40) return 40;
    assert(!"unexpected random draw");
    return from;
}
void update_pos(CHAR_DATA *ch) { (void)ch; }
int number_door(void) { return 0; }
long get_tribe(CHAR_DATA *ch, long tribe) { (void)ch; (void)tribe; return 0; }
int number_percent(void) { return 100; }
void move_char(CHAR_DATA *ch, long door) { (void)ch; (void)door; assert(0); }
void stop_fighting(CHAR_DATA *ch, bool both) { (void)ch; (void)both; assert(0); }
char *const dir_name[] = {"north", "east", "south", "west", "up", "down"};
/* Retained by the damage function's unused next-page branches. */
#define UNREACHED() assert(!"unexpected spell-chain dependency")
CHAR_DATA *get_char_world(CHAR_DATA *ch, char *arg)
{ (void)ch; (void)arg; UNREACHED(); return NULL; }
OBJ_DATA *get_obj_world(CHAR_DATA *ch, char *arg)
{ (void)ch; (void)arg; UNREACHED(); return NULL; }
void obj_from_room(OBJ_DATA *obj) { (void)obj; UNREACHED(); }
void obj_to_room(OBJ_DATA *obj, ROOM_INDEX_DATA *where)
{ (void)obj; (void)where; UNREACHED(); }
void char_from_room(CHAR_DATA *ch) { (void)ch; UNREACHED(); }
void char_to_room(CHAR_DATA *ch, ROOM_INDEX_DATA *where)
{ (void)ch; (void)where; UNREACHED(); }
void do_look(CHAR_DATA *ch, char *arg)
{ (void)ch; (void)arg; UNREACHED(); }
void free_string(char *str) { (void)str; UNREACHED(); }
char *str_dup(const char *str) { (void)str; UNREACHED(); return NULL; }
int char_stat(CHAR_DATA *ch, int stat)
{ (void)ch; (void)stat; UNREACHED(); return 0; }
long char_hitroll(CHAR_DATA *ch) { (void)ch; UNREACHED(); return 0; }
long char_damroll(CHAR_DATA *ch) { (void)ch; UNREACHED(); return 0; }
long char_ac(CHAR_DATA *ch) { (void)ch; UNREACHED(); return 0; }
long skill_lookup(const char *str) { (void)str; UNREACHED(); return 0; }
bool is_affected(CHAR_DATA *ch, long sn)
{ (void)ch; (void)sn; UNREACHED(); return FALSE; }
void enhance_stat(long sn, long level, CHAR_DATA *ch, CHAR_DATA *victim,
                  long apply_bit, long bonuses, long affect_bit)
{
    (void)sn; (void)level; (void)ch; (void)victim;
    (void)apply_bit; (void)bonuses; (void)affect_bit;
    UNREACHED();
}
#undef UNREACHED

static void reset(long class)
{
    int i;
    memset(chars, 0, sizeof(chars));
    memset(players, 0, sizeof(players));
    memset(&room, 0, sizeof(room));
    memset(dealt, 0, sizeof(dealt));
    memset(hits, 0, sizeof(hits));
    for (i = 0; i < 5; ++i)
    {
        chars[i].pcdata = &players[i];
        chars[i].in_room = &room;
        chars[i].position = POS_STANDING;
        players[i].flag = FLAG_PK;
        chars[i].next_in_room = i < 4 ? &chars[i + 1] : NULL;
    }
    chars[0].class = class;
    chars[0].mana = 20000;
    room.people = &chars[0];
    protected_target = group_members = FALSE;
    allow_fighting = TRUE;
    safe_calls = 0;
    chainspell = 0;
    last_message[0] = '\0';
}

static void test_bomb(void)
{
    reset(CLASS_NINJA);
    players[0].ninja[NINJA_KI] = 30;
    protected_target = TRUE;
    do_bomuzite(&chars[0], "target");
    assert(safe_calls == 1 && chars[1].position == POS_STANDING);
    assert(players[0].ninja[NINJA_KI] == 30 && chars[0].wait == 0);
    protected_target = FALSE;
    do_bomuzite(&chars[0], "target");
    assert(chars[1].position == POS_SLEEPING);
    assert(players[0].ninja[NINJA_KI] == 10 && chars[0].wait == 8);
}

static void test_favour(void)
{
    int i;
    reset(CLASS_MONK);
    group_members = TRUE;
    for (i = 0; i < 5; ++i) chars[i].more = MORE_FAVOUR;
    do_almighty(&chars[0], "group");
    assert(chars[0].mana == 20000 && chars[0].wait == 0);
    assert(strstr(last_message, "already blessed") != NULL);
    chars[2].more = 0;
    do_almighty(&chars[0], "group");
    assert(IS_MORE(&chars[2], MORE_FAVOUR));
    assert(chars[0].mana == 0 && chars[0].wait == 24);

    reset(CLASS_MONK);
    group_members = TRUE;
    for (i = 0; i < 5; ++i) chars[i].act = ACT_IS_NPC;
    chars[0].act = 0;
    chars[0].more = MORE_FAVOUR;
    do_almighty(&chars[0], "group");
    assert(chars[0].mana == 20000 && chars[0].wait == 0);
}

static void test_disc(void)
{
    reset(CLASS_DROW);
    chars[0].affected_by = AFF_HIDE;
    do_skipdisc(&chars[0], "");
    assert(chars[0].affected_by == (AFF_HIDE | AFF_FLYING));
    do_skipdisc(&chars[0], "");
    assert(chars[0].affected_by == AFF_HIDE);
    /* An unrelated item bit at the same numeric position is not flight. */
    chars[0].itemaffect = AFF_FLYING;
    do_skipdisc(&chars[0], "");
    assert(IS_AFFECTED(&chars[0], AFF_FLYING));
    do_skipdisc(&chars[0], "");
    assert(chars[0].affected_by == AFF_HIDE);
    assert(chars[0].itemaffect == AFF_FLYING);
}

static void test_quake(void)
{
    int caster_flying, victim_flying;
    for (caster_flying = 0; caster_flying < 2; ++caster_flying)
        for (victim_flying = 0; victim_flying < 2; ++victim_flying)
        {
            reset(CLASS_ELEM);
            players[0].powers[ELEM_EARTH] = 2;
            chars[0].affected_by = caster_flying ? AFF_FLYING : 0;
            /* Also exercise the old (wrong) caster item-affect field. */
            chars[0].itemaffect = caster_flying ? AFF_FLYING : 0;
            chars[1].affected_by = victim_flying ? AFF_FLYING : 0;
            do_localquake(&chars[0], "target");
            assert(hits[1] == !victim_flying);
            if (!victim_flying) assert(dealt[1] == 240);
            /* Casting at an immune target still spends the established cost. */
            assert(chars[0].mana == 20000 - 2 * 18 * 30);
            assert(chars[0].wait == 2);
        }
    reset(CLASS_ELEM);
    players[0].powers[ELEM_EARTH] = 2;
    allow_fighting = FALSE;
    do_localquake(&chars[0], "target");
    assert(chars[0].mana == 20000 && chars[0].wait == 0 && hits[1] == 0);
}

static void test_area(bool sorcerer, bool reverse)
{
    OBJ_DATA book, page;
    reset(sorcerer ? CLASS_SORCERER : CLASS_ELEM);
    players[0].powers[ELEM_INFIN] = 10;
    players[1].atm = 25;
    players[2].atm = 100;
    players[3].atm = 0;
    chars[4].act = ACT_IS_NPC;
    if (reverse)
    {
        room.people = &chars[4];
        chars[4].next_in_room = &chars[3];
        chars[3].next_in_room = &chars[2];
        chars[2].next_in_room = &chars[1];
        chars[1].next_in_room = &chars[0];
        chars[0].next_in_room = NULL;
    }
    if (sorcerer)
    {
        memset(&book, 0, sizeof(book));
        memset(&page, 0, sizeof(page));
        page.toughness = PURPLE_MAGIC;
        page.victpoweron = "cast";
        page.victpoweroff = "cast";
        page.chpoweroff = "blast";
        adv_spell_damage(&chars[0], &book, &page, "", -1);
        assert(dealt[1] == 900 && hits[1] == 1);
        assert(hits[2] == 0);
        assert(dealt[3] == 1200 && dealt[4] == 1200);
        assert(chars[0].mana == 19200 && chars[0].wait == 15);
    }
    else
    {
        do_ultimawave(&chars[0], "");
        assert(dealt[1] == 6000 && hits[1] == 1);
        assert(dealt[2] == 0 && hits[2] == 1);
        assert(dealt[3] == 8000 && dealt[4] == 8000);
        assert(chars[0].mana == 0 && chars[0].wait == 30);
        assert(players[0].obeah == 60);
    }
    assert(hits[3] == 1 && hits[4] == 1 && hits[0] == 0);
}

int main(void)
{
    reset(CLASS_MONK);
    players[0].monk[MONK_MANTRA] = 8;
    chars[1].hit = 100;
    chars[1].max_hit = 10000;
    chars[0].mana = 5000;
    do_touch(&chars[0], "target");
    assert(chars[1].hit == 100 && chars[0].mana == 5000 && chars[0].wait == 0);
    chars[0].mana = 7200;
    do_touch(&chars[0], "target");
    assert(chars[1].hit == 4900 && chars[0].mana == 0);
    reset(CLASS_NINJA);
    chars[0].fighting = &chars[1];
    players[0].ninja[NINJA_KI] = 19;
    do_mienaku(&chars[0], "");
    assert(players[0].ninja[NINJA_KI] == 19);
    players[0].ninja[NINJA_KI] = 20;
    do_mienaku(&chars[0], "");
    assert(players[0].ninja[NINJA_KI] == 0);
    test_bomb();
    test_favour();
    test_disc();
    test_quake();
    test_area(FALSE, FALSE);
    test_area(FALSE, TRUE);
    test_area(TRUE, FALSE);
    test_area(TRUE, TRUE);
    puts("class power regressions passed");
    return 0;
}
