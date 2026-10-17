/* Production engine with one explicit contract injection: a lethal callback
 * removes and frees a synthetic NPC, including a reflected NPC caster.
 * All other damage, saves, safety, casting and interpretation are real. */
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Genesis/src/merc.h"

extern void init_mm(void);
extern void weather_update(void);
extern char *string_space, *top_string;
static CHAR_DATA chars[3], *mobile;
static PC_DATA players[3];
static DESCRIPTOR_DATA descriptors[3];
static char output[3][32768];
static ROOM_INDEX_DATA room;
static AREA_DATA area;
static bool inject_death;
static long energy, weather;

static void remove_mobile(void)
{
    CHAR_DATA **p = &char_list;
    stop_fighting(mobile, TRUE);
    while (*p && *p != mobile) p = &(*p)->next;
    assert(*p == mobile);
    *p = mobile->next;
    char_from_room(mobile);
    free(mobile);
    mobile = NULL;
}

void combat_test_hurt_person(CHAR_DATA *ch, CHAR_DATA *victim, long dam)
{
    if (inject_death && victim == mobile)
    {
        remove_mobile();
        return;
    }
    hurt_person(ch, victim, dam);
}

static void reset(void)
{
    assert(!mobile);
    memset(players, 0, sizeof(players));
    memset(descriptors, 0, sizeof(descriptors));
    memset(output, 0, sizeof(output));
    memset(&room, 0, sizeof(room));
    memset(&area, 0, sizeof(area));
    room.area = &area; room.light = 1; room.people = chars;
    char_list = chars; descriptor_list = NULL;
    first_fight = last_fight = NULL;
    for (int i = 0; i < 3; ++i)
    {
        clear_char(&chars[i]);
        chars[i].name = chars[i].short_descr = i == 0 ? "caster" : i == 1 ? "target" : "other";
        chars[i].contingency = "";
        chars[i].class = CLASS_MAGE;
        chars[i].level = LEV_AWA;
        chars[i].pcdata = &players[i];
        chars[i].in_room = &room;
        chars[i].next = chars[i].next_in_room = i == 2 ? NULL : &chars[i + 1];
        chars[i].hit = chars[i].max_hit = 1000;
        chars[i].mana = 1000;
        chars[i].stats[STAT_INT] = 10;
        chars[i].position = POS_STANDING;
        chars[i].style[0] = STYLE_UNARMED;
        players[i].flag = FLAG_PK;
        players[i].love = players[i].conding = "";
        descriptors[i].connected = CON_PLAYING;
        descriptors[i].character = &chars[i];
        descriptors[i].outbuf = output[i];
        descriptors[i].outsize = sizeof(output[i]);
        chars[i].desc = &descriptors[i];
    }
    chars[0].fighting = &chars[2];
    chars[1].fighting = chars;
    chars[0].hit = 500;
    inject_death = FALSE;
    weather_info.change = 0;
}

static void add_mobile(void)
{
    mobile = calloc(1, sizeof(*mobile));
    assert(mobile);
    clear_char(mobile);
    mobile->act = ACT_IS_NPC;
    mobile->name = mobile->short_descr = "mobile";
    mobile->hit = mobile->max_hit = 1000;
    mobile->position = POS_STANDING;
    mobile->next = char_list; char_list = mobile;
    char_to_room(mobile, &room);
}

static void test_weather(void)
{
    reset();
    do_cast(chars, "'control weather' invalid");
    assert(chars[0].mana == 1000 && chars[0].wait == 0 && weather_info.change == 0);
    do_cast(chars, "'control weather' better");
    assert(chars[0].mana == 975 && chars[0].wait == 7 && weather_info.change > 0);
    long trend = weather_info.change;
    do_cast(chars, "'control weather' worse");
    assert(chars[0].mana == 975 && weather_info.change == trend);
    chars[0].wait = 0;
    weather_info.change = LONG_MIN;
    do_cast(chars, "'control weather' worse");
    assert(weather_info.change >= -144 && weather_info.change < 0);
    chars[0].wait = 0; chars[0].mana = 24;
    trend = weather_info.change;
    do_cast(chars, "'control weather' better");
    assert(chars[0].mana == 24 && chars[0].wait == 0 && weather_info.change == trend);
    chars[0].mana = 1000; chars[0].stats[STAT_INT] = LONG_MAX;
    weather_info.change = LONG_MAX;
    do_cast(chars, "'control weather' better");
    assert(chars[0].wait >= 1 && weather_info.change > 0 && weather_info.change <= 144);
    reset(); chars[0].combat_pause = 1;
    do_cast(chars, "'control weather' better");
    assert(chars[0].mana == 1000 && weather_info.change == 0);
    reset();
    time_info.hour = 1; time_info.month = 6;
    weather_info.change = 144; weather_info.mmhg = 1000;
    weather_info.sky = SKY_CLOUDLESS;
    weather_update();
    assert(weather_info.change == 12 && weather_info.mmhg == 1012);
    weather_info.change = -144;
    weather_update();
    assert(weather_info.change == -12 && weather_info.mmhg == 1000);
}

static void test_energy(void)
{
    bool success = FALSE;
    for (int attempt = 0; attempt < 128 && !success; ++attempt)
    {
        reset();
        chars[1].mana = 3;
        do_cast(chars, "'energy drain' target");
        assert(chars[0].mana == 965 && chars[0].wait == 7);
        assert(chars[2].hit == 1000 && chars[1].level == LEV_AWA);
        long dealt = 1000 - chars[1].hit;
        assert(dealt >= 0 && dealt <= 55);
        assert(chars[0].hit == 500 + dealt);
        if (dealt > 0)
        {
            assert(chars[1].mana >= 0 && chars[1].mana < 3);
            success = TRUE;
        }
        long hp = chars[1].hit, mana = chars[0].mana;
        do_cast(chars, "'energy drain' target");
        assert(chars[1].hit == hp && chars[0].mana == mana);
    }
    assert(success);
    reset(); players[1].atm = 100;
    for (int i = 0; i < 64; ++i)
        spell_energy_drain(energy, 100, chars, &chars[1]);
    assert(chars[1].hit == 1000 && chars[1].mana == 1000 && chars[0].hit == 500);
    reset(); chars[0].mana = 34;
    do_cast(chars, "'energy drain' target");
    assert(chars[0].mana == 34 && chars[0].wait == 0 && chars[1].hit == 1000);
    reset();
    do_cast(chars, "'energy drain' missing");
    do_cast(chars, "'energy drain' caster");
    assert(chars[0].mana == 1000 && chars[0].wait == 0);
    room.room_flags |= ROOM_SAFE;
    do_cast(chars, "'energy drain' target");
    assert(chars[0].mana == 1000 && chars[1].hit == 1000);
    reset(); chars[1].affected_by |= AFF_ETHEREAL;
    do_cast(chars, "'energy drain' target");
    assert(chars[0].mana == 1000 && chars[1].hit == 1000);
    reset(); chars[0].hit = 999;
    for (int i = 0; i < 64; ++i) spell_energy_drain(energy, LONG_MAX, chars, &chars[1]);
    assert(chars[0].hit <= 1000 && chars[1].hit >= 0);
}

static void test_extraction(void)
{
    bool extracted = FALSE;
    for (int i = 0; i < 128 && !extracted; ++i)
    {
        reset(); add_mobile(); inject_death = TRUE;
        do_cast(chars, "'energy drain' mobile");
        extracted = !mobile;
        assert(chars[0].mana == 965);
        if (extracted) assert(chars[0].hit == 500);
        if (mobile) remove_mobile();
    }
    assert(extracted);
    /* Object casting permits an NPC caster. Stargazer reflection can remove
     * that caster inside damage: neither damage nor item lag may touch it. */
    extracted = FALSE;
    for (int i = 0; i < 256 && !extracted; ++i)
    {
        reset(); add_mobile(); inject_death = TRUE;
        chars[1].class = CLASS_WEREWOLF;
        players[1].tribes[TRIBE_STARGAZERS] = 5;
        obj_cast_spell(energy, 100, mobile, &chars[1], NULL);
        extracted = !mobile;
        if (mobile) remove_mobile();
    }
    assert(extracted);
}

int main(void)
{
    string_space = calloc(1, 1024 * 1024);
    assert(string_space); top_string = string_space;
    current_time = 123456; init_mm();
    energy = skill_lookup("energy drain"); weather = skill_lookup("control weather");
    assert(energy > 0 && weather > 0);
    assert(slot_lookup(11) == weather && slot_lookup(25) == energy);
    test_weather(); test_energy(); test_extraction();
    puts("restored spell engine regressions passed");
    return 0;
}
