/* Full-engine characterization of the first shared-clan handler extraction.
 * Synthetic actor only: no accounts, area archive, network or disk saves. */
#include <sys/types.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Genesis/src/merc.h"

extern void init_mm(void);
extern char *string_space, *top_string;
static CHAR_DATA actor;
static PC_DATA player;
static DESCRIPTOR_DATA descriptor;
static ROOM_INDEX_DATA room;
static AREA_DATA area;
static char output[65536];

static void fresh(long class)
{
    free_string(actor.morph);
    clear_char(&actor);
    memset(&player, 0, sizeof(player));
    memset(&descriptor, 0, sizeof(descriptor));
    memset(&room, 0, sizeof(room));
    memset(&area, 0, sizeof(area));
    memset(output, 0, sizeof(output));
    actor.name = actor.short_descr = "tester";
    actor.pcdata = &player;
    actor.in_room = &room;
    actor.desc = &descriptor;
    actor.class = class;
    actor.level = LEV_AWA;
    actor.vampgen = 9;
    actor.style[0] = STYLE_UNARMED;
    actor.mana = 1000;
    actor.primal = 123;
    actor.blood[0] = 100;
    actor.blood[1] = 50;
    player.cland[0] = player.cland[1] = player.cland[2] = -1;
    player.love = player.conding = "";
    room.area = &area;
    room.light = 1;
    room.people = &actor;
    descriptor.character = &actor;
    descriptor.connected = CON_PLAYING;
    descriptor.outbuf = output;
    descriptor.outsize = sizeof(output);
    char_list = &actor;
    descriptor_list = NULL;
    first_fight = last_fight = NULL;
}

static void costless(void)
{
    assert(actor.mana == 1000 && actor.primal == 123 && actor.hit == 10000);
    assert(actor.blood[0] == 100 && actor.blood[1] == 50);
}

static void command(const char *name);

static void test_highlander_flash(void)
{
    CHAR_DATA victim;
    PC_DATA victim_pc;
    DESCRIPTOR_DATA victim_desc;
    char victim_output[4096] = {0};

    fresh(CLASS_HIGHLANDER);
    memset(&victim_pc, 0, sizeof(victim_pc));
    memset(&victim_desc, 0, sizeof(victim_desc));
    clear_char(&victim);
    victim.name = victim.short_descr = "opponent";
    victim.pcdata = &victim_pc;
    victim_pc.love = victim_pc.conding = "";
    victim.level = LEV_AWA;
    victim.status = actor.status = 100;
    victim.in_room = &room;
    victim.desc = &victim_desc;
    victim_desc.character = &victim;
    victim_desc.connected = CON_PLAYING;
    victim_desc.outbuf = victim_output;
    victim_desc.outsize = sizeof(victim_output);
    victim.next = NULL;
    actor.next = &victim;
    actor.next_in_room = &victim;
    room.people = &actor;
    player.highland[HIGH_ABILITY] = 2;
    SET_BIT(victim.act, PLR_HOLYLIGHT);

    command("bflash tester"); /* No hostile action against oneself. */
    assert(!actor.fighting && !victim.fighting && actor.wait == 0);
    SET_BIT(victim.itemaffect, ITEMA_PEACE);
    command("bflash opponent");
    assert(!actor.fighting && !victim.fighting && actor.wait == 0 &&
           !victim.affected && IS_SET(victim.act, PLR_HOLYLIGHT));
    REMOVE_BIT(victim.itemaffect, ITEMA_PEACE);
    victim.desc = NULL;
    command("bflash opponent"); /* Link-dead target, not yet fighting. */
    assert(!actor.fighting && !victim.fighting && actor.wait == 0 &&
           !victim.affected && IS_SET(victim.act, PLR_HOLYLIGHT));
    victim.desc = &victim_desc;
    command("bflash opponent");
    assert(actor.fighting == &victim && victim.fighting == &actor);
    assert(actor.wait == 24 && victim.affected != NULL);
    assert(IS_AFFECTED(&victim, AFF_BLIND));
    assert(!IS_SET(victim.act, PLR_HOLYLIGHT));
    assert(victim.affected->duration == 20 &&
           victim.affected->modifier == -50);
    stop_fighting(&actor, TRUE);
    affect_remove(&victim, victim.affected);
    char description[MAX_INPUT_LENGTH * 2];
    memset(description, 'X', sizeof(description) - 1);
    description[sizeof(description) - 1] = '\0';
    victim.short_descr = description;
    SET_BIT(victim.act, ACT_IS_NPC);
    player.highland[HIGH_ABILITY] = 5;
    descriptor.outtop = 0; output[0] = '\0';
    command("judge opponent");
    assert(strstr(output, "Health:") && strstr(output, "Magic:") &&
           strstr(output, "Armor:"));
    actor.next = actor.next_in_room = NULL;
    char_list = room.people = &actor;
}

static void command(const char *name)
{
    snprintf(descriptor.incomm, sizeof(descriptor.incomm), "%s", name);
    interpret(&actor, descriptor.incomm);
    descriptor.incomm[0] = '\0';
}

static void test_presence(void)
{
    fresh(CLASS_VAMPIRE);
    do_awe(&actor, "");
    do_majesty(&actor, "");
    assert(!IS_EXTRA(&actor, EXTRA_AWE) && !IS_ITEMAFF(&actor, ITEMA_PEACE));
    assert(strstr(output, "level 1 Presence") && strstr(output, "level 5 Presence"));
    player.powers[DISC_PRESENCE] = 1;
    command("awe");
    assert(IS_EXTRA(&actor, EXTRA_AWE));
    command("awe");
    assert(!IS_EXTRA(&actor, EXTRA_AWE));
    player.powers[DISC_PRESENCE] = 4;
    command("majesty");
    assert(!IS_ITEMAFF(&actor, ITEMA_PEACE) && player.majesty == 0);
    player.powers[DISC_PRESENCE] = 5;
    command("majesty");
    assert(IS_ITEMAFF(&actor, ITEMA_PEACE) && player.majesty == 0);
    command("majesty");
    assert(!IS_ITEMAFF(&actor, ITEMA_PEACE) && player.majesty == 120);
    command("majesty");
    assert(!IS_ITEMAFF(&actor, ITEMA_PEACE) && player.majesty == 120);
    assert(strstr(output, "another 120 seconds"));
    player.majesty = 1;
    command("majesty");
    assert(!IS_ITEMAFF(&actor, ITEMA_PEACE));
    assert(strstr(output, "another 1 second"));
    player.majesty = 0;
    command("majesty");
    assert(IS_ITEMAFF(&actor, ITEMA_PEACE));
    assert(actor.wait == 0);
    costless();
}

static void test_beast(void)
{
    fresh(CLASS_VAMPIRE);
    actor.beast = 99;
    for (int i = 0; i < 128 && actor.beast == 99; ++i)
        do_beastlike(&actor, "");
    assert(actor.beast == 100 && IS_VAMPAFF(&actor, VAM_FANGS));
    assert(!IS_VAMPAFF(&actor, VAM_CLAWS));
    do_beastlike(&actor, "");
    do_humanity(&actor, "");
    assert(actor.beast == 100);
    actor.beast = 99;
    for (int i = 0; i < 128 && actor.beast == 99; ++i)
        do_humanity(&actor, "");
    assert(actor.beast == 98);
    actor.beast = 1;
    for (int i = 0; i < 16384 && actor.beast == 1; ++i)
        do_humanity(&actor, "");
    assert(actor.beast == 0 && strstr(output, "attained Golconda"));
    do_humanity(&actor, "");
    assert(actor.beast == 0 && actor.wait == 0);
    costless();
    fresh(CLASS_VAMPIRE);
    actor.beast = 99;
    player.powers[DISC_PROTEAN] = 2;
    for (int i = 0; i < 128 && actor.beast == 99; ++i)
        do_beastlike(&actor, "");
    assert(actor.beast == 100 && IS_VAMPAFF(&actor, VAM_NIGHTSIGHT));
    assert(IS_VAMPAFF(&actor, VAM_FANGS) && IS_VAMPAFF(&actor, VAM_CLAWS));
    costless();
}

static void test_werewolf(void)
{
    fresh(CLASS_WEREWOLF);
    command("umbralvision");
    assert(IS_AFFECTED(&actor, AFF_SHADOWSIGHT));
    command("umbralvision");
    assert(!IS_AFFECTED(&actor, AFF_SHADOWSIGHT) && actor.wait == 0);
    command("rage");
    assert(player.wolf >= 10 && player.wolf <= 20);
    assert(!IS_ACT(&actor, PLR_WOLFMAN) && actor.wait == 12);
    costless();
    fresh(CLASS_WEREWOLF);
    player.wolf = 99;
    command("rage");
    assert(IS_ACT(&actor, PLR_WOLFMAN));
    assert(player.wolf >= 134 && player.wolf <= 144);
    assert(IS_VAMPAFF(&actor, VAM_FANGS) && IS_VAMPAFF(&actor, VAM_CLAWS));
    assert(IS_AFFECTED(&actor, AFF_POLYMORPH) && actor.wait == 12);
    costless();
    fresh(CLASS_WEREWOLF);
    player.wolf = 99;
    player.auspice[AUSPICE_AHROUN] = 4;
    do_rage(&actor, "");
    assert(IS_ACT(&actor, PLR_WOLFMAN) && player.wolf == 300);
    actor.wait = 0;
    do_rage(&actor, "");
    assert(player.wolf == 300 && actor.wait == 0);
    assert(strstr(output, "already in a rage"));
    costless();
}

static void test_shared(long class)
{
    fresh(class);
    player.powers[DISC_PROTEAN] = 2;
    player.breed[BREED_METIS] = 3;
    player.dragskills = DRAG_FANGS | DRAG_CLAWS | DRAG_NIGHT;
    player.drow[DROW_POWERS] = DROW_NIGHT;
    actor.spheres[MFOR] = 1;
    if (class == CLASS_DRAGON) actor.polyaff = POLY_DRAGONFORM;
    bool physical = class == CLASS_VAMPIRE || class == CLASS_WEREWOLF
        || class == CLASS_DRAGON;
    bool sight = physical || class == CLASS_DROW || class == CLASS_MAGE;
    do_fangs(&actor, "");
    do_claws(&actor, "");
    do_nightsight(&actor, "");
    assert(!!IS_VAMPAFF(&actor, VAM_FANGS) == physical);
    assert(!!IS_VAMPAFF(&actor, VAM_CLAWS) == physical);
    assert(!!IS_VAMPAFF(&actor, VAM_NIGHTSIGHT) == sight);
    do_fangs(&actor, "");
    do_claws(&actor, "");
    do_nightsight(&actor, "");
    assert(!IS_VAMPAFF(&actor, VAM_FANGS | VAM_CLAWS | VAM_NIGHTSIGHT));
    SET_BIT(actor.extra, TIED_UP);
    SET_BIT(actor.affected_by, AFF_WEBBED | AFF_SHADOW);
    do_flex(&actor, "");
    assert(!IS_EXTRA(&actor, TIED_UP));
    assert(!IS_AFFECTED(&actor, AFF_WEBBED | AFF_SHADOW));
    assert(actor.wait == 12);
    player.wolf = 75;
    actor.wait = 0;
    do_calm(&actor, "");
    assert(player.wolf == (class == CLASS_NINJA ? 0 : 75));
    if (class == CLASS_WEREWOLF)
    {
        player.tribes[TRIBE_CHILDREN_OF_GAIA] = 1;
        do_calm(&actor, "");
        assert(player.wolf == 0 && actor.wait == 12);
    }
    costless();
}

static void test_rejections(long class)
{
    fresh(class);
    player.powers[DISC_PRESENCE] = 7;
    player.powers[DISC_PROTEAN] = 7;
    if (class != CLASS_VAMPIRE)
    {
        actor.beast = 99;
        do_majesty(&actor, ""); do_awe(&actor, "");
        do_humanity(&actor, ""); do_beastlike(&actor, "");
        assert(!IS_ITEMAFF(&actor, ITEMA_PEACE) && !IS_EXTRA(&actor, EXTRA_AWE));
        assert(actor.beast == 99 && actor.vampaff == 0);
    }
    if (class != CLASS_WEREWOLF)
    {
        do_shadowsight(&actor, ""); do_rage(&actor, "");
        assert(!IS_AFFECTED(&actor, AFF_SHADOWSIGHT) && player.wolf == 0);
    }
    assert(actor.wait == 0);
    costless();
    fresh(class);
    SET_BIT(actor.act, ACT_IS_NPC);
    actor.pcdata = NULL;
    do_majesty(&actor, ""); do_awe(&actor, "");
    do_humanity(&actor, ""); do_beastlike(&actor, "");
    do_shadowsight(&actor, ""); do_rage(&actor, "");
    assert(actor.extra == 0 && actor.itemaffect == 0 && actor.vampaff == 0);
    assert(actor.wait == 0 && actor.affected_by == 0);
}

int main(void)
{
    string_space = calloc(1, 1024 * 1024);
    assert(string_space); top_string = string_space;
    current_time = 123456;
    init_mm();
    test_highlander_flash();
    test_presence();
    test_beast();
    test_werewolf();
    const long classes[] = {CLASS_NONE, CLASS_VAMPIRE, CLASS_WEREWOLF,
        CLASS_SORCERER, CLASS_DROW, CLASS_HIGHLANDER, CLASS_MAGE,
        CLASS_DRAGON, CLASS_NINJA, CLASS_MONK, CLASS_ELEM};
    for (size_t i = 0; i < sizeof(classes) / sizeof(classes[0]); ++i)
    {
        test_shared(classes[i]);
        test_rejections(classes[i]);
    }
    fresh(CLASS_NONE);
    puts("class handler extraction full-engine regressions passed");
    return 0;
}
