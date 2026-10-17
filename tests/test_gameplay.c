/*
 * Isolated production-function regressions; no server or game data required.
 * From mudfix:
 * cc -std=gnu17 -DMERC -Dunix -ffunction-sections -fdata-sections \
 *   -IGenesis/src tests/test_gameplay.c Genesis/src/act_info.c \
 *   Genesis/src/classes/werewolf/wwauspice.c Genesis/src/classes/mage/mage.c \
 *   -Wl,--gc-sections -o /tmp/test_gameplay
 * /tmp/test_gameplay
 * Add -fsanitize=address,undefined -fno-omit-frame-pointer for sanitizers.
 */
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "../Genesis/src/merc.h"

static CHAR_DATA target;
static CHAR_DATA *expected_caster;
static AFFECT_DATA captured_affect;
static int affect_calls, lookup_calls, safe_calls, move_calls;
static int follower_added, follower_stopped, die_calls;
static bool refuse_safe;
static char looked_up[MAX_INPUT_LENGTH];
long gsn_charm_person = 73;

char *one_argument(char *argument, char *word)
{
    while (isspace((unsigned char)*argument))
        ++argument;
    while (*argument && !isspace((unsigned char)*argument))
        *word++ = tolower((unsigned char)*argument++);
    *word = '\0';
    while (isspace((unsigned char)*argument))
        ++argument;
    return argument;
}

bool str_cmp(const char *a, const char *b)
{
    return strcasecmp(a, b) != 0;
}

void send_to_char(const char *message, CHAR_DATA *ch)
{
    (void)message;
    (void)ch;
}

void act(const char *message, CHAR_DATA *ch, const void *a,
         const void *b, long type)
{
    (void)message;
    (void)ch;
    (void)a;
    (void)b;
    (void)type;
}

long get_trust(CHAR_DATA *ch)
{
    (void)ch;
    return 0;
}

static void test_stat_limits_and_costs(void)
{
    const long classes[] = {CLASS_NONE, CLASS_VAMPIRE, CLASS_WEREWOLF,
        CLASS_SORCERER, CLASS_DROW, CLASS_HIGHLANDER, CLASS_MAGE,
        CLASS_DRAGON, CLASS_NINJA, CLASS_MONK, CLASS_ELEM};
    const int limits[][5] = {
        {8,8,8,8,8}, {10,9,11,9,8}, {11,9,10,8,9}, {9,9,8,11,10},
        {9,11,8,10,9}, {11,9,10,8,9}, {8,9,9,11,10}, {11,8,10,9,9},
        {10,11,9,9,8}, {9,10,9,8,11}, {9,8,9,10,11}
    };
    const int stats[] = {STAT_STR, STAT_DEX, STAT_CON, STAT_INT, STAT_WIS};
    CHAR_DATA ch = {0};
    for (unsigned i = 0; i < sizeof(classes)/sizeof(classes[0]); ++i)
    {
        ch.class = classes[i];
        for (int j = 0; j < 5; ++j)
        {
            if (stat_max(&ch, stats[j]) != limits[i][j])
                fprintf(stderr, "class %ld stat %d: expected %d, got %d\n",
                        classes[i], stats[j], limits[i][j], stat_max(&ch, stats[j]));
            assert(stat_max(&ch, stats[j]) == limits[i][j]);
            ch.stats[stats[j]] = 5;
            assert(stat_cost(&ch, stats[j]) == 250000);
            ch.stats[stats[j]] = 6;
            assert(stat_cost(&ch, stats[j]) == 720000);
        }
    }
}

void bug(const char *message, long value)
{
    (void)message;
    (void)value;
    assert(!"unexpected production bug report");
}

CHAR_DATA *get_char_room(CHAR_DATA *ch, char *name)
{
    assert(ch == expected_caster);
    assert(!strcmp(name, "target"));
    ++lookup_calls;
    return &target;
}

CHAR_DATA *get_char_world(CHAR_DATA *ch, char *name)
{
    assert(ch == expected_caster);
    assert(strlen(name) < sizeof(looked_up));
    strcpy(looked_up, name);
    ++lookup_calls;
    return !strcmp(name, "target") ? &target : NULL;
}

void stop_follower(CHAR_DATA *ch)
{
    assert(ch == &target);
    assert(ch->master != NULL);
    ++follower_stopped;
    ch->master = NULL;
}

void add_follower(CHAR_DATA *ch, CHAR_DATA *master)
{
    assert(ch == &target);
    assert(master == expected_caster);
    assert(ch->master == NULL);
    ++follower_added;
    ch->master = master;
}

void affect_to_char(CHAR_DATA *ch, AFFECT_DATA *affect)
{
    assert(ch == &target);
    captured_affect = *affect;
    ++affect_calls;
}

bool is_affected(CHAR_DATA *ch, long sn)
{
    assert(ch == &target);
    assert(sn == 91);
    return FALSE;
}

long skill_lookup(const char *name)
{
    assert(!strcmp(name, "Friction Curse"));
    return 91;
}

void char_from_room(CHAR_DATA *ch)
{
    assert(ch == expected_caster);
    assert(ch->in_room != NULL);
    ch->in_room = NULL;
    ++move_calls;
}

void char_to_room(CHAR_DATA *ch, ROOM_INDEX_DATA *room)
{
    assert(ch == expected_caster);
    assert(ch->in_room == NULL);
    assert(room != NULL);
    ch->in_room = room;
    ++move_calls;
}

bool is_safe(CHAR_DATA *ch, CHAR_DATA *victim)
{
    assert(ch == expected_caster);
    assert(victim == &target);
    assert(ch->in_room == target.in_room);
    ++safe_calls;
    return refuse_safe;
}

int number_range(int from, int to)
{
    /* Real spellpower runs: ten deterministic successful d10 rolls. */
    assert(from == 1 && to == 10);
    ++die_calls;
    return 10;
}

int number_percent(void)
{
    assert(!"NPC target must not invoke player magic resistance");
    return 100;
}

void hurt_person(CHAR_DATA *ch, CHAR_DATA *victim, long damage)
{
    (void)ch;
    (void)victim;
    (void)damage;
    assert(!"unexpected hearts-blood damage or paradox backlash");
}

void update_pos(CHAR_DATA *ch)
{
    (void)ch;
    assert(!"unexpected paradox backlash");
}

static void test_channels(void)
{
    CHAR_DATA ch;
    const long all = CHANNEL_SORCTALK | CHANNEL_CHAT | CHANNEL_IMMTALK |
        CHANNEL_MUSIC | CHANNEL_NEWBIE | CHANNEL_SHOUT | CHANNEL_YELL |
        CHANNEL_HOWL | CHANNEL_INFO | CHANNEL_VAMPTALK | CHANNEL_TELL |
        CHANNEL_MIKTALK | CHANNEL_ELEM | CHANNEL_MONK | CHANNEL_HIGHTALK |
        CHANNEL_MAGETALK | CHANNEL_DROW | CHANNEL_TELEPATHY | CHANNEL_OOC;
    const long unrelated = (1L << 28) | CHANNEL_HONOR | CHANNEL_LOG;

    memset(&ch, 0, sizeof(ch));
    assert((all & unrelated) == 0);
    ch.deaf = unrelated;
    do_channels(&ch, "-all");
    assert(ch.deaf == (unrelated | all));
    do_channels(&ch, "+all");
    assert(ch.deaf == unrelated);
    ch.deaf = 0;
    do_channels(&ch, "-all");
    assert(ch.deaf == all);
    do_channels(&ch, "+all");
    assert(ch.deaf == 0);
}

static void test_geas(void)
{
    CHAR_DATA caster, former_master;
    PC_DATA player;
    memset(&caster, 0, sizeof(caster));
    memset(&player, 0, sizeof(player));
    memset(&former_master, 0, sizeof(former_master));
    memset(&target, 0, sizeof(target));
    caster.class = CLASS_WEREWOLF;
    caster.pcdata = &player;
    player.auspice[AUSPICE_PHILODOX] = 5;
    target.act = ACT_IS_NPC;
    target.level = 100;
    target.master = &former_master;
    expected_caster = &caster;
    affect_calls = lookup_calls = follower_added = follower_stopped = 0;
    memset(&captured_affect, 0, sizeof(captured_affect));

    do_geas(&caster, "target");
    assert(lookup_calls == 1);
    assert(follower_added == 1 && follower_stopped == 1);
    assert(target.master == &caster);
    assert(affect_calls == 1);
    assert(captured_affect.type == gsn_charm_person);
    assert(captured_affect.duration == 25);
    assert(captured_affect.location == 0);
    assert(captured_affect.modifier == 0);
    assert(captured_affect.bitvector == AFF_CHARM);
}

static void test_friction_curse(char *argument, long quint, bool safe)
{
    CHAR_DATA caster;
    ROOM_INDEX_DATA origin, destination;
    AREA_DATA origin_area, destination_area;
    memset(&caster, 0, sizeof(caster));
    memset(&target, 0, sizeof(target));
    memset(&origin, 0, sizeof(origin));
    memset(&destination, 0, sizeof(destination));
    memset(&origin_area, 0, sizeof(origin_area));
    memset(&destination_area, 0, sizeof(destination_area));
    origin.area = &origin_area;
    destination.area = &destination_area;
    caster.in_room = &origin;
    target.in_room = &destination;
    target.act = ACT_IS_NPC;
    caster.class = CLASS_MAGE;
    caster.spheres[MFOR] = 3;
    caster.spheres[MCOR] = 6;
    caster.arete = 10;
    caster.mana = 100000;
    caster.quint[BODY] = 100;
    expected_caster = &caster;
    refuse_safe = safe;
    affect_calls = lookup_calls = safe_calls = move_calls = die_calls = 0;
    looked_up[0] = '\0';
    memset(&captured_affect, 0, sizeof(captured_affect));

    do_friction_curse(&caster, argument);
    assert(lookup_calls == 1 && !strcmp(looked_up, "target"));
    assert(die_calls == 10);
    assert(safe_calls == 1);
    assert(move_calls == 4);
    assert(caster.in_room == &origin);
    assert(target.in_room == &destination);
    if (safe)
    {
        assert(affect_calls == 0);
        return;
    }
    assert(affect_calls == 1);
    assert(captured_affect.type == 91);
    /* Real spellpower: 10 successes + 1, minus correspondence 6. */
    assert(captured_affect.duration == (5 + quint) * 10);
    assert(captured_affect.location == APPLY_NONE);
    assert(captured_affect.modifier == 0 && captured_affect.bitvector == 0);
    assert(caster.quint[BODY] == 100 - quint);
    assert(caster.mana == 100000 - 3500 - 6 * 6 * 100 - quint * 300);
    assert(caster.wait == 10);
}

int main(void)
{
    test_stat_limits_and_costs();
    test_channels();
    test_geas();
    test_friction_curse("target 3", 3, FALSE);
    test_friction_curse("target", 0, FALSE);
    test_friction_curse("target 3", 3, TRUE);
    puts("gameplay regressions passed");
    return 0;
}
