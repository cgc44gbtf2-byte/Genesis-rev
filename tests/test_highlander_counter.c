/* Original Counter, linked to the real safety, interpreter and damage engine.
 * No archive world, accounts, sockets or player saves are touched. */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Genesis/src/merc.h"

extern void init_mm(void);
extern char *string_space, *top_string;
static CHAR_DATA actors[2];
static PC_DATA pcs[2];
static DESCRIPTOR_DATA descs[2];
static OBJ_DATA swords[2];
static ROOM_INDEX_DATA room;
static AREA_DATA area;
static char output[2][32768];

static void fixture(void)
{
    memset(pcs, 0, sizeof(pcs));
    memset(descs, 0, sizeof(descs));
    memset(swords, 0, sizeof(swords));
    memset(output, 0, sizeof(output));
    memset(&room, 0, sizeof(room));
    memset(&area, 0, sizeof(area));
    room.area = &area; room.light = 1; room.people = actors;
    first_fight = last_fight = NULL;
    descriptor_list = NULL; char_list = actors;
    for (int i = 0; i < 2; ++i)
    {
        CHAR_DATA *ch = &actors[i];
        clear_char(ch);
        ch->name = ch->short_descr = i ? "beta" : "alpha";
        ch->contingency = "";
        ch->pcdata = &pcs[i]; ch->desc = &descs[i];
        ch->class = CLASS_HIGHLANDER; ch->level = LEV_AWA; ch->status = 100;
        ch->position = POS_STANDING;
        ch->hit = ch->max_hit = 100000;
        ch->mana = ch->max_mana = 10000;
        ch->in_room = &room;
        ch->next = ch->next_in_room = i ? NULL : &actors[1];
        ch->style[0] = STYLE_ONEHAND; ch->style[STYLE_ONEHAND] = 200;
        for (int s = 0; s < 5; ++s) ch->stats[s] = 5;
        pcs[i].love = pcs[i].conding = ""; pcs[i].flag = FLAG_PK;
        for (int p = 0; p < 4; ++p) pcs[i].highland[p] = 6;
        descs[i].character = ch; descs[i].connected = CON_PLAYING;
        descs[i].outbuf = output[i]; descs[i].outsize = sizeof(output[i]);
        swords[i].item_type = ITEM_WEAPON;
        swords[i].value[1] = swords[i].value[2] = 50;
        swords[i].value[3] = TYPE_SLASH;
        swords[i].spectype = SITEM_HIGHLANDER;
        swords[i].questowner = ch->name;
        swords[i].name = swords[i].short_descr = "katana";
        swords[i].wear_loc = WEAR_WEAPON;
        swords[i].carried_by = ch; ch->carrying = &swords[i];
    }
    set_fighting(&actors[0], &actors[1]);
    set_fighting(&actors[1], &actors[0]);
}

static void rejected(void)
{
    static int gate;
    ++gate;
    do_counter(&actors[0], "");
    if (actors[0].mana != 10000 || actors[0].wait != 0 ||
        actors[0].highland_guard || actors[0].highland_counter_reuse)
        fprintf(stderr, "rejected gate %d: class=%ld defense=%ld powers=%ld output=%s\n",
                gate, actors[0].class, pcs[0].highland[HIGH_DEFENSE],
                pcs[0].highland[HIGH_ABILITY], output[0]);
    assert(actors[0].mana == 10000 && actors[0].wait == 0 &&
           actors[0].highland_guard == 0 &&
           actors[0].highland_counter_reuse == 0);
}

static void gates(void)
{
    fixture(); actors[0].class = CLASS_MAGE; rejected();
    fixture(); pcs[0].highland[HIGH_DEFENSE] = 3; rejected();
    fixture(); pcs[0].highland[HIGH_ABILITY] = 3; rejected();
    fixture(); actors[0].carrying = NULL; rejected();
    fixture(); swords[0].questowner = "somebody"; rejected();
    fixture(); swords[0].value[3] = TYPE_BLUNT; rejected();
    fixture(); actors[0].combat_pause = 1; rejected();
    fixture(); stop_fighting(&actors[1], FALSE); actors[1].desc = NULL; rejected();
    fixture(); stop_fighting(&actors[1], FALSE);
    SET_BIT(actors[1].itemaffect, ITEMA_PEACE); rejected();
    fixture(); actors[0].fighting = &actors[0]; rejected();
    fixture(); actors[0].mana = 999; do_counter(&actors[0], "");
    assert(actors[0].mana == 999 && !actors[0].highland_guard &&
           !actors[0].highland_counter_reuse && !actors[0].wait);
    fixture(); do_counter(&actors[0], "nonsense");
    assert(actors[0].mana == 10000 && !actors[0].highland_guard);
}

static void lifecycle(void)
{
    fixture();
    interpret(&actors[0], "counter");
    assert(actors[0].mana == 9000 && actors[0].highland_guard == 8 &&
           actors[0].highland_counter_reuse == 24 && actors[0].wait == 4);
    do_counter(&actors[0], "");
    assert(actors[0].mana == 9000 && actors[0].highland_guard == 8);
    do_counter(&actors[0], "off");
    assert(!actors[0].highland_guard && !actors[0].highland_riposte &&
           actors[0].highland_counter_reuse == 24 && actors[0].mana == 9000);
    actors[0].desc = NULL; /* The main-loop timers do not depend on input. */
    for (int n = 0; n < 24; ++n) combat_status_update();
    assert(!actors[0].highland_counter_reuse);
    actors[0].desc = &descs[0];
    do_counter(&actors[0], "");
    for (int n = 0; n < 8; ++n) combat_status_update();
    assert(!actors[0].highland_guard && actors[0].highland_counter_reuse == 16);
    actors[0].highland_riposte = 8;
    actors[0].class = CLASS_MAGE;
    combat_status_update();
    assert(!actors[0].highland_riposte && !actors[0].highland_guard &&
           actors[0].highland_counter_reuse == 15);
    actors[0].highland_guard = actors[0].highland_riposte = 8;
    clear_char(&actors[0]);
    assert(!actors[0].highland_guard && !actors[0].highland_riposte &&
           !actors[0].highland_counter_reuse);
}

static void parry_and_riposte(void)
{
    int successes = 0, failures = 0;
    /* Exercise actual RNG without asserting a statistically precise rate. */
    for (int trial = 0; trial < 200; ++trial)
    {
        fixture();
        do_counter(&actors[1], "");
        long ah = actors[0].hit, bh = actors[1].hit;
        bool caught = highlander_counter_parry(&actors[0], &actors[1],
                                               &swords[0], TYPE_SLASH);
        assert(!actors[1].highland_guard);
        assert(actors[0].hit == ah && actors[1].hit == bh); /* No nested damage. */
        if (!caught) { ++failures; assert(!actors[1].highland_riposte); continue; }
        ++successes;
        assert(actors[1].highland_riposte == 8);
        actors[0].highland_guard = 8; /* The reply cannot start another counter. */
        one_hit(&actors[1], &actors[0], TYPE_SLASH, "1");
        assert(!actors[1].highland_riposte && actors[0].highland_guard == 8);
        assert(actors[1].hit == bh && actors[0].hit <= ah &&
               strstr(output[1], "precise katana riposte"));
    }
    assert(successes > 0 && failures > 0);
    int native_parries = 0;
    for (int trial = 0; trial < 100; ++trial)
    {
        fixture(); do_counter(&actors[1], "");
        long before = actors[1].hit;
        actors[0].beginners_luck = 1; /* Ordinary avoidance cannot mask this check. */
        one_hit(&actors[0], &actors[1], TYPE_SLASH, "1");
        assert(!actors[1].highland_guard);
        if (actors[1].highland_riposte)
        {
            ++native_parries;
            assert(actors[1].hit == before &&
                   strstr(output[1], "prepare a riposte"));
        }
    }
    assert(native_parries > 0);

    fixture(); swords[0].wear_loc = WEAR_HOLD;
    do_counter(&actors[0], "");
    assert(actors[0].mana == 9000 && actors[0].highland_guard == 8);
    actors[0].highland_riposte = 8;
    one_hit(&actors[0], &actors[1], TYPE_SLASH, "2");
    assert(!actors[0].highland_riposte &&
           strstr(output[0], "precise katana riposte"));

    fixture(); do_counter(&actors[1], "");
    assert(!highlander_counter_parry(&actors[0], &actors[1], &swords[0], TYPE_SPELL));
    assert(actors[1].highland_guard == 8); /* Spells are not melee. */
    SET_BIT(pcs[1].flag, FLAG_AFK);
    assert(!highlander_counter_parry(&actors[0], &actors[1], &swords[0], TYPE_SLASH));
    assert(actors[1].highland_guard == 8);
    REMOVE_BIT(pcs[1].flag, FLAG_AFK);
    actors[1].carrying = NULL;
    assert(!highlander_counter_parry(&actors[0], &actors[1], &swords[0], TYPE_SLASH));
    assert(!actors[1].highland_guard);

    fixture(); actors[0].highland_riposte = 8;
    assert(!highlander_counter_strike(&actors[0], &actors[1], NULL, TYPE_UNARMED));
    assert(actors[0].highland_riposte == 8);
    actors[0].carrying = NULL;
    assert(!highlander_counter_strike(&actors[0], &actors[1], &swords[0], TYPE_SLASH));
    assert(!actors[0].highland_riposte);
}

static void all_melee(void)
{
    const char *contacts[] = {"1", "attack", "kick", "fangs", "horns",
        "wing buffet", "wave fist", "holy sword", "tsume", "forked tongue",
        "tendril of shadow", "shadow arm", "chimerstral dagger",
        "spider fangs", "circle", "backstab", "repeating fist",
        "Fist of Caine", "spiked tail", "tongue", "bone crafting", "mirage strike"};
    const char *ranged[] = {"shuriken", "breath", "surge", "acid bolt",
        "flare arrow", "icicle barrage", "wind blade", "rock shower",
        "mindblast", "spit", "lightning blast", "bone quills", "bristles",
        "disintegrate"};
    for (size_t i = 0; i < sizeof(contacts)/sizeof(contacts[0]); ++i)
    {
        assert(highlander_melee_attack(TYPE_HIT, contacts[i]));
        fixture(); actors[0].carrying = NULL; actors[0].style[0] = STYLE_UNARMED;
        do_counter(&actors[1], "");
        one_hit(&actors[0], &actors[1], TYPE_UNARMED, (char *)contacts[i]);
        assert(!actors[1].highland_guard); /* Claws/fists/contact need no weapon. */
    }
    for (size_t i = 0; i < sizeof(ranged)/sizeof(ranged[0]); ++i)
    {
        assert(!highlander_melee_attack(TYPE_HIT, ranged[i]));
        fixture(); do_counter(&actors[1], "");
        one_hit(&actors[0], &actors[1], TYPE_HIT, (char *)ranged[i]);
        assert(actors[1].highland_guard == 8 && !actors[1].highland_riposte);
    }
    int kicks = 0, stuns = 0, punches = 0, bashes = 0;
    for (int trial = 0; trial < 100; ++trial)
    {
        fixture(); do_counter(&actors[1], "");
        actors[0].class = CLASS_MONK; pcs[0].monk[MONK_TECH] = 8;
        do_cripple(&actors[0], "beta");
        if (actors[1].highland_riposte)
        {
            ++kicks;
            assert(actors[1].hit == 100000 && actors[1].mana == 9000 &&
                   actors[0].mana == 8000);
        }
        fixture(); do_counter(&actors[1], "");
        actors[0].class = CLASS_MONK; pcs[0].monk[MONK_TECH] = 8;
        do_stunfist(&actors[0], "beta");
        if (actors[1].highland_riposte)
        {
            ++stuns;
            assert(!actors[1].combat_pause && !actors[1].stun_immunity &&
                   actors[0].mana == 5000 && actors[0].combat_pause == 8);
        }
        fixture(); do_counter(&actors[1], "");
        do_punch(&actors[0], "beta");
        if (actors[1].highland_riposte)
        {
            ++punches;
            assert(actors[1].hit == 100000 && actors[1].position == POS_FIGHTING);
        }
        fixture(); do_counter(&actors[1], "");
        do_bash(&actors[0], "beta");
        if (actors[1].highland_riposte)
        {
            ++bashes;
            assert(actors[1].hit == 100000 && actors[1].wait == 4 &&
                   actors[0].wait == 20);
        }
    }
    assert(kicks && stuns && punches && bashes);
}

int main(void)
{
    string_space = malloc(1024 * 1024); assert(string_space);
    top_string = string_space; *top_string = '\0'; init_mm();
    gates(); lifecycle(); parry_and_riposte(); all_melee();
    puts("Highlander Counter: gates, cost, timers, parry, native riposte and non-recursion passed");
    return 0;
}
