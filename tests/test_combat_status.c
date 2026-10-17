/* Full production engine linked without running its main(). Synthetic
 * characters only: no sockets, historical accounts or world files loaded. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../Genesis/src/merc.h"

static CHAR_DATA chars[4];
static PC_DATA players[4];
static DESCRIPTOR_DATA descriptors[4];
static char output[4][32768];
static ROOM_INDEX_DATA room, elsewhere;
static AREA_DATA area;
static char *names[] = {"monk", "target", "other", "mobile"};
static int special_calls;
void mobile_update(void);

static bool test_special(CHAR_DATA *ch)
{
    assert(ch == &chars[3]);
    ++special_calls;
    return TRUE;
}

static void reset(void)
{
    memset(players, 0, sizeof(players));
    memset(descriptors, 0, sizeof(descriptors));
    memset(output, 0, sizeof(output));
    memset(&room, 0, sizeof(room));
    memset(&elsewhere, 0, sizeof(elsewhere));
    memset(&area, 0, sizeof(area));
    room.area = &area;
    room.light = 1;
    room.people = chars;
    char_list = chars;
    descriptor_list = NULL;
    first_fight = last_fight = NULL;
    for (int i = 0; i < 4; ++i)
    {
        clear_char(&chars[i]);
        chars[i].name = chars[i].short_descr = names[i];
        chars[i].contingency = "";
        chars[i].pcdata = &players[i];
        chars[i].in_room = &room;
        chars[i].level = LEV_AWA;
        chars[i].next = i < 3 ? &chars[i + 1] : NULL;
        chars[i].next_in_room = chars[i].next;
        chars[i].style[0] = STYLE_UNARMED;
        players[i].flag = FLAG_PK;
        players[i].love = players[i].conding = "";
        players[i].monk[MONK_TECH] = 7;
        descriptors[i].character = &chars[i];
        descriptors[i].connected = CON_PLAYING;
        descriptors[i].outbuf = output[i];
        descriptors[i].outsize = sizeof(output[i]);
        chars[i].desc = &descriptors[i];
    }
    chars[0].class = chars[2].class = CLASS_MONK;
    chars[3].act = ACT_IS_NPC;
    chars[3].desc = NULL;
    chars[3].pcdata = NULL;
}

static void assert_free_rejection(CHAR_DATA *victim)
{
    assert(chars[0].mana == 10000);
    assert(chars[0].combat_pause == 0 && chars[0].wait == 0);
    assert(victim->combat_pause == 0 && victim->stun_immunity == 0);
    assert(chars[0].fighting == NULL && victim->fighting == NULL);
}

static void test_validation(void)
{
    do_stunfist(NULL, "");
    reset();
    do_stunfist(&chars[3], "target");
    assert_free_rejection(&chars[1]);
    chars[0].class = CLASS_NONE;
    do_stunfist(chars, "target");
    assert_free_rejection(&chars[1]);
    reset();
    players[0].monk[MONK_TECH] = 6;
    do_stunfist(chars, "target");
    assert_free_rejection(&chars[1]);
    reset();
    chars[0].pcdata = NULL;
    do_stunfist(chars, "target");
    assert_free_rejection(&chars[1]);
    reset();
    do_stunfist(chars, "");
    do_stunfist(chars, "missing");
    do_stunfist(chars, "monk");
    assert_free_rejection(&chars[1]);
    chars[0].position = POS_RESTING;
    do_stunfist(chars, "target");
    assert_free_rejection(&chars[1]);
    reset();
    chars[1].position = POS_SLEEPING;
    do_stunfist(chars, "target");
    assert_free_rejection(&chars[1]);
    reset();
    chars[1].in_room = &elsewhere;
    do_stunfist(chars, "target");
    assert_free_rejection(&chars[1]);
    reset();
    chars[0].in_room = NULL;
    do_stunfist(chars, "target");
    assert_free_rejection(&chars[1]);
    reset();
    chars[0].mana = STUN_FIST_COST - 1;
    do_stunfist(chars, "target");
    assert(chars[0].mana == STUN_FIST_COST - 1);
    chars[0].mana = 10000;
    assert_free_rejection(&chars[1]);

    reset();
    room.room_flags = ROOM_SAFE;
    do_stunfist(chars, "target");
    assert_free_rejection(&chars[1]);
    reset();
    chars[3].act |= ACT_NOKILL;
    do_stunfist(chars, "mobile");
    assert_free_rejection(&chars[3]);
    reset();
    chars[1].desc = NULL;
    do_stunfist(chars, "target");
    assert_free_rejection(&chars[1]);
    reset();
    chars[1].affected_by = AFF_ETHEREAL;
    do_stunfist(chars, "target");
    assert_free_rejection(&chars[1]);
    reset();
    chars[0].affected_by = AFF_CHARM;
    chars[0].master = &chars[1];
    do_stunfist(chars, "target");
    assert_free_rejection(&chars[1]);
}

static void test_cast_and_immunity(bool npc)
{
    CHAR_DATA *target;
    reset();
    target = &chars[npc ? 3 : 1];
    chars[0].mana = STUN_FIST_COST;
    do_stunfist(chars, npc ? "mobile" : "target");
    assert(chars[0].mana == 0);
    assert(chars[0].fighting == target && target->fighting == chars);
    assert(chars[0].combat_pause == STUN_FIST_RECOVERY);
    assert(target->combat_pause == STUN_FIST_DURATION);
    assert(target->stun_immunity == STUN_FIST_IMMUNITY);
    assert(target->hit == 10000 && target->position == POS_FIGHTING);
    for (int i = 0; i < STUN_FIST_DURATION; ++i)
    {
        assert(IS_COMBAT_PAUSED(target));
        /* Actual production attack entry points: no damage, no initiative
         * progression, no side effects while interrupted (also for NPCs). */
        long hp = chars[0].hit, init = target->init;
        multi_hit(target, chars);
        one_hit(target, chars, TYPE_UNARMED, "fist");
        violence_update();
        assert(chars[0].hit == hp && target->init == init);
        do_stunfist(&chars[2], npc ? "mobile" : "target");
        assert(chars[2].mana == 10000 && chars[2].combat_pause == 0);
        assert(target->combat_pause == STUN_FIST_DURATION - i);
        assert(target->stun_immunity == STUN_FIST_IMMUNITY - i);
        combat_status_update();
    }
    assert(!IS_COMBAT_PAUSED(target));
    assert(IS_COMBAT_PAUSED(chars)); /* attacker recovery is longer */
    do_stunfist(&chars[2], npc ? "mobile" : "target");
    assert(chars[2].mana == 10000); /* immunity outlasts interruption */
    assert(target->stun_immunity == STUN_FIST_IMMUNITY - STUN_FIST_DURATION);

    /* Commands actually resume after the interruption, not after immunity. */
    if (!npc)
    {
        char argument[] = "brief";
        interpret(target, argument);
        assert(IS_SET(target->act, PLR_BRIEF));
    }

    /* Real violence loop resumes initiative processing after expiry.
     * Keep initiative above zero so this check needs no world combat data. */
    target->init = 10;
    violence_update();
    assert(target->init == 9);
    descriptors[0].outtop = 0;
    one_hit(target, chars, TYPE_UNARMED, "fist");
    assert(descriptors[0].outtop > 0); /* attack or avoidance message */
    for (int i = STUN_FIST_DURATION; i < STUN_FIST_IMMUNITY; ++i)
        combat_status_update();
    assert(!IS_COMBAT_PAUSED(chars) && target->stun_immunity == 0);
    do_stunfist(&chars[2], npc ? "mobile" : "target");
    assert(chars[2].mana == 10000 - STUN_FIST_COST);
    assert(target->combat_pause == STUN_FIST_DURATION);
}

static void test_named_target_and_guards(void)
{
    reset();
    chars[0].fighting = &chars[3];
    do_stunfist(chars, "target");
    assert(chars[1].combat_pause == STUN_FIST_DURATION);
    assert(chars[3].combat_pause == 0);
    reset();
    chars[0].fighting = &chars[1];
    do_stunfist(chars, "");
    assert(chars[1].combat_pause == STUN_FIST_DURATION);
    reset();
    chars[0].combat_pause = 2;
    do_stunfist(chars, "target");
    assert(chars[0].mana == 10000 && chars[1].combat_pause == 0);
    chars[0].combat_pause = 0;
    chars[1].combat_pause = 2;
    do_stunfist(chars, "target");
    assert(chars[0].mana == 10000 && chars[1].combat_pause == 2);

    for (int i = 0; i < 4; ++i)
    {
        reset();
        chars[i].combat_pause = 1;
        char argument[] = "brief";
        interpret(&chars[i], argument);
        assert(!IS_SET(chars[i].act, PLR_BRIEF));
        /* All loaded actors tick, irrespective of connection or room. */
        chars[i].desc = NULL;
        chars[i].in_room = NULL;
        chars[i].stun_immunity = 1;
        chars[i].wait = 7;
        combat_status_update();
        assert(chars[i].combat_pause == 0 && chars[i].stun_immunity == 0);
        assert(chars[i].wait == 7); /* no coupling to legacy descriptor lag */
        combat_status_update();
        assert(chars[i].combat_pause == 0 && chars[i].stun_immunity == 0);
    }
    chars[0].combat_pause = chars[0].stun_immunity = 10;
    clear_char(chars);
    assert(chars[0].combat_pause == 0 && chars[0].stun_immunity == 0);
}

static void test_npc_actions_resume(void)
{
    reset();
    char_list = &chars[3];
    chars[3].spec_fun = test_special;
    chars[3].combat_pause = 1;
    special_calls = 0;
    mobile_update();
    assert(special_calls == 0);
    char blocked[] = "say interrupted";
    interpret(&chars[3], blocked);
    assert(descriptors[0].outtop == 0);
    combat_status_update();
    mobile_update();
    assert(special_calls == 1);
    char resumed[] = "say recovered";
    interpret(&chars[3], resumed);
    assert(descriptors[0].outtop > 0);
}

int main(void)
{
    test_validation();
    test_cast_and_immunity(FALSE);
    test_cast_and_immunity(TRUE);
    test_named_target_and_guards();
    test_npc_actions_resume();
    puts("full-engine Stun Fist regressions passed");
    return 0;
}
