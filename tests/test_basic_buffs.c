/* Full-engine tests using synthetic characters and anonymous temporary files.
 * No archived accounts/world data, server sockets or gameplay stubs are used. */
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Genesis/src/merc.h"

extern void init_mm(void);
extern void char_update(void);
extern void fwrite_char(CHAR_DATA *, FILE *);
extern void fread_char(CHAR_DATA *, FILE *);
extern char *string_space, *top_string;

static CHAR_DATA chars[4];
static PC_DATA players[4];
static DESCRIPTOR_DATA descriptors[4];
static char output[4][65536];
static ROOM_INDEX_DATA room, elsewhere;
static AREA_DATA area;
static char *names[] = {"caster", "target", "other", "mobile"};
static const char *spells[] = {"armor", "bless", "shield", "stone skin"};
static const long slots[] = {1, 3, 67, 66};
static const long costs[] = {5, 5, 12, 12};
static const long beats[] = {12, 12, 18, 18};
static long skills[4];

static void reset(void)
{
    for (int i = 0; i < 4; ++i)
        while (chars[i].affected)
            affect_remove(&chars[i], chars[i].affected);
    memset(players, 0, sizeof(players));
    memset(descriptors, 0, sizeof(descriptors));
    memset(output, 0, sizeof(output));
    memset(&room, 0, sizeof(room));
    memset(&elsewhere, 0, sizeof(elsewhere));
    memset(&area, 0, sizeof(area));
    room.area = elsewhere.area = &area;
    room.light = 1;
    room.people = chars;
    char_list = chars;
    descriptor_list = NULL;
    first_fight = last_fight = NULL;
    current_time = 1800000000;
    for (int i = 0; i < 4; ++i)
    {
        clear_char(&chars[i]);
        chars[i].name = chars[i].short_descr = names[i];
        chars[i].contingency = "";
        chars[i].pcdata = &players[i];
        chars[i].class = CLASS_NONE;
        chars[i].level = LEV_AWA;
        chars[i].position = POS_STANDING;
        chars[i].hit = chars[i].max_hit = 1000;
        chars[i].mana = chars[i].max_mana = 1000;
        chars[i].armor = 249;
        chars[i].hitroll = 149;
        chars[i].stats[STAT_INT] = chars[i].stats[STAT_WIS] = 10;
        chars[i].style[0] = STYLE_UNARMED;
        chars[i].save_time = current_time;
        chars[i].in_room = &room;
        chars[i].next = chars[i].next_in_room = i < 3 ? &chars[i + 1] : NULL;
        players[i].flag = FLAG_PK;
        players[i].love = players[i].conding = "";
        players[i].pwd = players[i].bamfin = players[i].bamfout = "";
        players[i].cdest = players[i].title = players[i].decap_char = "";
        players[i].marriage = players[i].email = players[i].covenant = "";
        players[i].cov_time = players[i].leader = players[i].vleader = "";
        players[i].cland[0] = players[i].cland[1] = players[i].cland[2] = -1;
        descriptors[i].connected = CON_PLAYING;
        descriptors[i].character = &chars[i];
        descriptors[i].outbuf = output[i];
        descriptors[i].outsize = sizeof(output[i]);
        chars[i].desc = &descriptors[i];
    }
    chars[3].act = ACT_IS_NPC;
    chars[3].pcdata = NULL;
    chars[3].desc = NULL;
}

static int affect_count(CHAR_DATA *ch)
{
    int n = 0;
    for (AFFECT_DATA *p = ch->affected; p; p = p->next) ++n;
    return n;
}

static void cast(int which, const char *target)
{
    char command[128];
    snprintf(command, sizeof(command), "'%s' %s", spells[which], target);
    do_cast(chars, command);
}

static void test_casting(void)
{
    for (int s = 0; s < 4; ++s)
    {
        CHAR_DATA *target;
        reset();
        target = &chars[s == 3 ? 0 : 1];
        chars[0].mana = costs[s];
        cast(s, s == 3 ? "caster" : "target");
        assert(chars[0].mana == 0 && chars[0].wait == beats[s] - 5);
        assert(is_affected(target, skills[s]));
        assert(affect_count(target) == (s == 1 ? 2 : 1));
        assert(chars[2].affected == NULL);
        if (s == 1)
            assert(target->hitroll == 155 && target->saving_throw == -6);
        else
            assert(target->armor == 249 + (s == 3 ? 40 : 20));

        /* Same spell neither stacks nor refreshes, and costs no cast mana. */
        target->affected->duration = 3;
        chars[0].mana = 1000; chars[0].wait = 0;
        cast(s, s == 3 ? "caster" : "target");
        assert(chars[0].mana == 1000 && chars[0].wait == 0);
        assert(target->affected->duration == 3);
        assert(affect_count(target) == (s == 1 ? 2 : 1));

        reset(); chars[0].mana = costs[s] - 1;
        cast(s, "");
        assert(chars[0].mana == costs[s] - 1 && chars[0].wait == 0);
        assert(chars[0].affected == NULL);
        reset(); chars[0].wait = 1;
        cast(s, "");
        assert(chars[0].mana == 1000 && chars[0].affected == NULL);
        reset(); chars[0].combat_pause = 1;
        cast(s, "");
        assert(chars[0].mana == 1000 && chars[0].affected == NULL);
        reset(); chars[0].level = LEV_SLE;
        cast(s, "");
        assert(chars[0].mana == 1000 && chars[0].affected == NULL);
        reset();
        cast(s, "absent");
        assert(chars[0].mana == 1000 && chars[0].wait == 0);
        reset(); room.added_flags |= ROOM2_SILENCE;
        cast(s, "");
        assert(chars[0].mana == 1000 && chars[0].affected == NULL);

        reset(); chars[0].act |= PLR_WOLFMAN;
        chars[0].mana = costs[s] * 2 - 1;
        cast(s, "");
        assert(chars[0].affected == NULL);
        chars[0].mana = costs[s] * 2;
        cast(s, "");
        assert(chars[0].mana == 0 && chars[0].affected != NULL);
    }
    reset();
    cast(3, "target");
    assert(chars[0].mana == 1000 && chars[1].affected == NULL);
    /* A blessed caster may still bless another unblessed character. */
    cast(1, "");
    chars[0].wait = 0;
    cast(1, "target");
    assert(chars[0].mana == 990 && chars[1].hitroll == 155);
}

static void test_classes_and_targets(void)
{
    static const long classes[] = {CLASS_NONE, CLASS_VAMPIRE, CLASS_WEREWOLF,
        CLASS_SORCERER, CLASS_DROW, CLASS_HIGHLANDER, CLASS_MAGE, CLASS_DRAGON,
        CLASS_NINJA, CLASS_MONK, CLASS_ELEM};
    static const long planes[] = {AFF_ETHEREAL, AFF_DEEPUMBRA, AFF_SHADOWPLANE};
    for (size_t c = 0; c < sizeof(classes) / sizeof(classes[0]); ++c)
        for (int s = 0; s < 4; ++s)
        {
            reset(); chars[0].class = classes[c];
            cast(s, "");
            assert(chars[0].mana == 1000 - costs[s]);
            assert(chars[0].wait >= 1 && is_affected(chars, skills[s]));
        }
    for (size_t p = 0; p < sizeof(planes) / sizeof(planes[0]); ++p)
        for (int s = 0; s < 3; ++s)
        {
            reset(); chars[1].affected_by |= planes[p];
            cast(s, "target");
            assert(chars[0].mana == 1000 && chars[1].affected == NULL);
            (*skill_table[skills[s]].spell_fun)(skills[s], 100, chars, &chars[1]);
            assert(chars[1].affected == NULL);
        }
    for (int s = 0; s < 4; ++s)
    {
        reset();
        (*skill_table[skills[s]].spell_fun)(skills[s], 100, NULL, chars);
        (*skill_table[skills[s]].spell_fun)(skills[s], 100, chars, NULL);
        assert(chars[0].affected == NULL);
        (*skill_table[skills[s]].spell_fun)(skills[(s + 1) % 4], 100, chars, chars);
        assert(chars[0].affected == NULL);
        char_from_room(&chars[1]); char_to_room(&chars[1], &elsewhere);
        (*skill_table[skills[s]].spell_fun)(skills[s], 100, chars, &chars[1]);
        assert(chars[1].affected == NULL);
    }
    /* Item casting uses the same duplicate/plane rules, including NPCs.
     * Stone Skin's TAR_CHAR_SELF is retained by the generic item dispatcher. */
    for (int npc = 0; npc < 2; ++npc)
        for (int s = 0; s < 4; ++s)
        {
            reset();
            CHAR_DATA *caster = &chars[npc ? 3 : 0];
            CHAR_DATA *target = s == 3 ? caster : &chars[1];
            obj_cast_spell(skills[s], LONG_MAX, caster, &chars[1], NULL);
            assert(is_affected(target, skills[s]));
            assert(target->affected->duration >= 1 && target->affected->duration <= 108);
            int n = affect_count(target);
            caster->wait = 0;
            obj_cast_spell(skills[s], 100, caster, &chars[1], NULL);
            assert(caster->wait == 0 && affect_count(target) == n);
            assert(caster->mana == 1000);
        }
}

static void test_actual_combat_stats(void)
{
    for (int s = 0; s < 4; ++s)
    {
        reset();
        long before = s == 1 ? breakavoid(chars) : avoidblow(chars);
        cast(s, "");
        long after = s == 1 ? breakavoid(chars) : avoidblow(chars);
        assert(after == before + 1);
        affect_strip(chars, skills[s]);
        assert((s == 1 ? breakavoid(chars) : avoidblow(chars)) == before);
        assert(chars[0].armor == 249 && chars[0].hitroll == 149);
    }
    reset();
    for (int s = 0; s < 4; ++s)
        (*skill_table[skills[s]].spell_fun)(skills[s], 100, chars, chars);
    assert(chars[0].armor == 329 && chars[0].hitroll == 161);
    assert(affect_count(chars) == 5);
}

static void check_saves_at(long level, int threshold)
{
    int rolls[4096];
    init_mm();
    for (int i = 0; i < 4096; ++i) rolls[i] = number_percent();
    init_mm();
    for (int i = 0; i < 4096; ++i)
        assert(saves_spell(level, chars) == (rolls[i] < threshold));
}

static void check_saves(int threshold)
{
    check_saves_at(80, threshold);
}

static void test_extreme_resistance(void)
{
    const struct { long wisdom, level; int threshold; } cases[] = {
        {LONG_MAX, LONG_MAX, 85}, {LONG_MIN, LONG_MIN, 15},
        {0, LONG_MAX, 15}, {0, LONG_MIN, 85},
        {LONG_MAX / 5, LONG_MAX, 55 - LONG_MAX % 5},
        {LONG_MIN / 5, LONG_MIN, 55 - LONG_MIN % 5},
        {LONG_MAX / 5 + 1, LONG_MAX, 60 - LONG_MAX % 5},
        {LONG_MIN / 5 - 1, LONG_MIN, 50 - LONG_MIN % 5},
        {LONG_MAX / 5 + 7, LONG_MAX, 85},
        {LONG_MIN / 5 - 9, LONG_MIN, 15},
    };
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i)
    {
        reset(); chars[0].stats[STAT_WIS] = cases[i].wisdom;
        check_saves_at(cases[i].level, cases[i].threshold);
        spell_bless(skills[1], 100, chars, chars);
        check_saves_at(cases[i].level, UMIN(85, cases[i].threshold + 12));
    }
}

static void test_bless_resistance(void)
{
    reset();
    check_saves(25); /* 50 + (5 + 10 WIS * 5 - 80). */
    chars[0].saving_throw = -1000; /* Dormant unrelated aggregate stays inert. */
    check_saves(25);
    chars[0].saving_throw = 0;
    spell_bless(skills[1], 100, chars, chars);
    check_saves(37);
    /* Even malformed duplicate saved Bless affects cannot multiply the bonus. */
    AFFECT_DATA extra = {0};
    extra.type = skills[1]; extra.duration = 1;
    extra.location = APPLY_SAVING_SPELL; extra.modifier = -12;
    affect_to_char(chars, &extra);
    check_saves(37);
    affect_strip(chars, skills[1]);
    assert(chars[0].saving_throw == 0 && chars[0].hitroll == 149);
    check_saves(25);
    spell_bless(skills[1], 100, chars, chars);
    /* The existing maximum threshold remains 85. */
    chars[0].stats[STAT_WIS] = 19;
    int rolls[4096];
    init_mm();
    for (int i = 0; i < 4096; ++i) rolls[i] = number_percent();
    init_mm();
    for (int i = 0; i < 4096; ++i)
        assert(saves_spell(1, chars) == (rolls[i] < 85));
}

static void test_bounds_and_expiry(void)
{
    for (int s = 0; s < 4; ++s)
    {
        reset();
        chars[0].stats[STAT_INT] = LONG_MAX;
        cast(s, "");
        assert(chars[0].wait >= 1 && chars[0].affected->duration <= 108);
        reset();
        chars[0].stats[STAT_INT] = LONG_MIN;
        cast(s, "");
        assert(chars[0].wait >= 1 && chars[0].affected->duration >= 1);
        reset();
        if (s == 1) chars[0].hitroll = LONG_MAX;
        else chars[0].armor = LONG_MAX;
        cast(s, "");
        assert(chars[0].mana == 1000 && chars[0].affected == NULL);
    }
    reset(); chars[0].saving_throw = LONG_MIN;
    cast(1, "");
    assert(chars[0].mana == 1000 && chars[0].affected == NULL);
    reset();
    for (int s = 0; s < 4; ++s)
        (*skill_table[skills[s]].spell_fun)(skills[s], LONG_MIN, chars, chars);
    assert(chars[0].hitroll == 150 && chars[0].saving_throw == -1);
    for (AFFECT_DATA *p = chars[0].affected; p; p = p->next)
        p->duration = 1;
    char_update();
    assert(chars[0].affected != NULL && chars[0].affected->duration == 0);
    char_update();
    assert(chars[0].affected == NULL);
    assert(chars[0].armor == 249 && chars[0].hitroll == 149);
    assert(chars[0].saving_throw == 0);
}

static void test_save_roundtrip(void)
{
    reset();
    for (int s = 0; s < 4; ++s)
        (*skill_table[skills[s]].spell_fun)(skills[s], 100, chars, chars);
    FILE *fp = tmpfile();
    assert(fp);
    fwrite_char(chars, fp);
    assert(!ferror(fp)); rewind(fp);
    assert(!strcmp(fread_word(fp), "#PLAYER"));
    CHAR_DATA loaded;
    PC_DATA pc = {0};
    clear_char(&loaded); loaded.pcdata = &pc;
    fread_char(&loaded, fp); fclose(fp);
    assert(loaded.armor == 329 && loaded.hitroll == 161 && loaded.saving_throw == -12);
    assert(affect_count(&loaded) == 5);
    for (int s = 0; s < 4; ++s)
    {
        assert(is_affected(&loaded, skills[s]));
        affect_strip(&loaded, skills[s]);
    }
    assert(loaded.armor == 249 && loaded.hitroll == 149 && loaded.saving_throw == 0);
}

int main(void)
{
    string_space = calloc(1, 1024 * 1024);
    assert(string_space); top_string = string_space;
    for (int s = 0; s < 4; ++s)
    {
        skills[s] = skill_lookup(spells[s]);
        assert(skills[s] > 0 && skills[s] < MAX_SKILL);
        assert(slot_lookup(slots[s]) == skills[s]);
        assert(skill_table[skills[s]].skill_level == LEV_AWA);
    }
    test_casting(); test_classes_and_targets(); test_actual_combat_stats();
    test_bless_resistance(); test_extreme_resistance();
    test_bounds_and_expiry(); test_save_roundtrip();
    reset();
    puts("basic buff full-engine regressions passed");
    return 0;
}
