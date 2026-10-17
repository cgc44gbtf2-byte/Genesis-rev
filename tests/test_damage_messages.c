/* Real-engine display tests. Synthetic actors only; no world/account loading. */
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Genesis/src/merc.h"

extern void init_mm(void);
extern char *string_space, *top_string;
extern void dam_message(CHAR_DATA *, CHAR_DATA *, long, long, char *);
static CHAR_DATA actors[4];
static PC_DATA players[4];
static DESCRIPTOR_DATA descriptors[4];
static ROOM_INDEX_DATA room;
static AREA_DATA area;
static char output[4][65536];
static char *names[] = {"caster", "target", "observer", "numbers"};

static void reset(void)
{
    memset(players, 0, sizeof(players));
    memset(descriptors, 0, sizeof(descriptors));
    memset(output, 0, sizeof(output));
    memset(&room, 0, sizeof(room)); memset(&area, 0, sizeof(area));
    room.area = &area; room.light = 1; room.people = actors;
    char_list = actors; descriptor_list = NULL;
    first_fight = last_fight = NULL;
    for (int i = 0; i < 4; ++i)
    {
        clear_char(&actors[i]);
        actors[i].name = actors[i].short_descr = names[i];
        actors[i].pcdata = &players[i];
        actors[i].level = LEV_AWA; actors[i].position = POS_STANDING;
        actors[i].in_room = &room; actors[i].desc = &descriptors[i];
        actors[i].hit = actors[i].max_hit = 1000;
        actors[i].mana = 123; actors[i].primal = 456;
        actors[i].blood[0] = 50; actors[i].blood[1] = 100;
        actors[i].next = actors[i].next_in_room = i < 3 ? &actors[i + 1] : NULL;
        descriptors[i].character = &actors[i];
        descriptors[i].connected = CON_PLAYING;
        descriptors[i].outbuf = output[i];
        descriptors[i].outsize = sizeof(output[i]);
    }
    SET_BIT(actors[3].act, PLR_SDAM);
}

static void unchanged(void)
{
    for (int i = 0; i < 4; ++i)
    {
        assert(actors[i].hit == 1000 && actors[i].max_hit == 1000);
        assert(actors[i].mana == 123 && actors[i].primal == 456);
        assert(actors[i].blood[0] == 50 && actors[i].blood[1] == 100);
        assert(actors[i].wait == 0 && actors[i].combat_pause == 0);
        assert(actors[i].fighting == NULL && actors[i].in_room == &room);
        assert(actors[i].class == CLASS_NONE);
    }
}

static const char *expected(long damage)
{
    if (damage <= 50) return " like a wimp.";
    if (damage <= 250) return " lightly.";
    if (damage <= 500) return " hard.";
    if (damage <= 1000) return " incredibly hard!";
    if (damage <= 2500) return " with ULTIMATE power!";
    if (damage <= 5000) return " with UNGODLY vengeance!";
    return " with UNMERCIFUL strength!!";
}

static void test_boundaries_and_recipients(void)
{
    const long damage[] = {0, 1, 49, 50, 51, 249, 250, 251, 499, 500,
        501, 999, 1000, 1001, 2499, 2500, 2501, 4999, 5000, 5001, LONG_MAX};
    const long types[] = {TYPE_HIT, TYPE_UNARMED, TYPE_SLASH, TYPE_PIERCE,
        TYPE_BLUNT, TYPE_SPELL};
    char noun[] = "test strike";
    for (size_t t = 0; t < sizeof(types) / sizeof(types[0]); ++t)
        for (size_t d = 0; d < sizeof(damage) / sizeof(damage[0]); ++d)
        {
            reset();
            dam_message(&actors[0], &actors[1], damage[d], types[t], noun);
            for (int i = 0; i < 4; ++i)
            {
                assert(strstr(output[i], expected(damage[d])));
                assert(strstr(output[i], noun));
                /* Numeric visibility stays attacker/victim plus opted-in viewer. */
                /* A space-bracket is the number suffix, not an ANSI CSI. */
                assert((strstr(output[i], " [") != NULL) == (i != 2));
            }
            assert(strstr(output[0], "Your test strike"));
            assert(strstr(output[1], "you"));
            assert(strstr(output[2], "target"));
            assert(strstr(output[3], "target"));
            unchanged();
        }
}

static void test_large_and_missing_names(void)
{
    char noun[8192];
    memset(noun, 'x', sizeof(noun) - 1); noun[sizeof(noun) - 1] = '\0';
    reset();
    dam_message(&actors[0], &actors[1], 5001, TYPE_SLASH, noun);
    for (int i = 0; i < 4; ++i)
    {
        assert(descriptors[i].outtop > 0 && descriptors[i].outtop < 1024);
        assert(strstr(output[i], "xxx"));
    }
    unchanged();
    reset();
    dam_message(&actors[0], &actors[1], 5001, TYPE_SLASH, NULL);
    assert(strstr(output[0], "Your attack"));
    for (int i = 0; i < 4; ++i) assert(strstr(output[i], expected(5001)));
    unchanged();
}

int main(void)
{
    string_space = calloc(1, 1024 * 1024);
    assert(string_space); top_string = string_space;
    current_time = 123456; init_mm();
    test_boundaries_and_recipients();
    test_large_and_missing_names();
    puts("Gathering wording / Genesis damage-boundary regressions passed");
    return 0;
}
