/* Real-engine characterization before/after the second clan extraction.
 * Only synthetic rooms/prototypes/actors. No world or account loading/saves. */
#include <sys/types.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Genesis/src/merc.h"

extern void init_mm(void);
extern char *string_space, *top_string;
static CHAR_DATA actor, visitor;
static PC_DATA player, guest;
static DESCRIPTOR_DATA descriptor;
static ROOM_INDEX_DATA home, away, staging;
static AREA_DATA area;
static MOB_INDEX_DATA prototype;
static char output[65536];

static void cleanup(void)
{
    for (CHAR_DATA *ch = char_list, *next; ch != NULL; ch = next)
    {
        next = ch->next;
        if (IS_NPC(ch)) extract_char(ch, TRUE);
    }
}

static void fresh(long class)
{
    cleanup();
    clear_char(&actor); clear_char(&visitor);
    memset(&player, 0, sizeof(player)); memset(&guest, 0, sizeof(guest));
    memset(&descriptor, 0, sizeof(descriptor)); memset(output, 0, sizeof(output));
    memset(&home, 0, sizeof(home)); memset(&away, 0, sizeof(away));
    memset(&staging, 0, sizeof(staging)); memset(&area, 0, sizeof(area));
    memset(&prototype, 0, sizeof(prototype));
    memset(room_index_hash, 0, sizeof(room_index_hash));
    memset(mob_index_hash, 0, sizeof(mob_index_hash));
    home.vnum = 1; away.vnum = 2; staging.vnum = ROOM_VNUM_DISCONNECTION;
    ROOM_INDEX_DATA *rooms[] = {&home, &away, &staging};
    for (size_t i = 0; i < 3; ++i)
    {
        rooms[i]->area = &area; rooms[i]->light = 1;
        rooms[i]->name = "Synthetic room"; rooms[i]->description = "";
        unsigned bucket = rooms[i]->vnum % MAX_KEY_HASH;
        rooms[i]->next = room_index_hash[bucket]; room_index_hash[bucket] = rooms[i];
    }
    prototype.vnum = MOB_VNUM_CLONE;
    prototype.act = ACT_IS_NPC; prototype.level = 10; prototype.area = &area;
    prototype.player_name = prototype.short_descr = str_empty;
    prototype.long_descr = prototype.description = str_empty;
    mob_index_hash[prototype.vnum % MAX_KEY_HASH] = &prototype;
    actor.name = actor.short_descr = "caster"; actor.pcdata = &player;
    actor.class = class; actor.level = LEV_AWA; actor.vampgen = 9;
    actor.mana = 1000; actor.primal = 123; actor.style[0] = STYLE_UNARMED;
    actor.blood[0] = 100; actor.blood[1] = 50;
    for (int i = STAT_STR; i <= STAT_WIS; ++i) actor.stats[i] = 20;
    player.cland[0] = player.cland[1] = player.cland[2] = -1;
    player.love = player.conding = "";
    visitor.name = visitor.short_descr = "visitor"; visitor.pcdata = &guest;
    visitor.level = LEV_AWA; guest.love = guest.conding = "";
    actor.next = &visitor; char_list = &actor;
    char_to_room(&actor, &home); char_to_room(&visitor, &away);
    actor.desc = &descriptor;
    descriptor.character = &actor; descriptor.connected = CON_PLAYING;
    descriptor.outbuf = output; descriptor.outsize = sizeof(output);
    descriptor_list = NULL; first_fight = last_fight = NULL;
}

static CHAR_DATA *mobile(const char *name, ROOM_INDEX_DATA *room)
{
    CHAR_DATA *mob = create_mobile(&prototype);
    mob->name = str_dup(name); mob->short_descr = str_dup(name);
    char_to_room(mob, room);
    return mob;
}

static void command(const char *input)
{
    actor.wait = 0; /* Simulate recovery elapsed between player commands. */
    snprintf(descriptor.incomm, sizeof(descriptor.incomm), "%s", input);
    interpret(&actor, descriptor.incomm);
    descriptor.incomm[0] = '\0';
}

static void resources(void)
{
    assert(actor.mana == 1000 && actor.primal == 123 && actor.hit == 10000);
    assert(actor.blood[0] == 100 && actor.blood[1] == 50);
}

static void test_states(void)
{
    fresh(CLASS_VAMPIRE);
    command("regenerate"); assert(player.regenerate == 10 && actor.wait == 0);
    command("regenerate"); assert(player.regenerate == 0);
    player.powers[DISC_PROTEAN] = 2;
    command("earthmeld"); assert(!IS_VAMPAFF(&actor, VAM_EARTHMELDED));
    player.powers[DISC_PROTEAN] = 3;
    SET_BIT(actor.affected_by, AFF_FLAMING);
    command("earthmeld");
    assert(IS_VAMPAFF(&actor, VAM_EARTHMELDED));
    assert(!IS_AFFECTED(&actor, AFF_FLAMING));
    command("earthmeld"); assert(!IS_VAMPAFF(&actor, VAM_EARTHMELDED));
    assert(actor.wait == 0); resources();
}

static void test_obfuscate(void)
{
    fresh(CLASS_VAMPIRE);
    CHAR_DATA *following = mobile("following", &home);
    CHAR_DATA *other = mobile("other", &home);
    following->leader = &actor;
    command("vanish"); assert(!IS_VAMPAFF(&actor, VAM_OBFUSCATE));
    player.powers[DISC_OBFUSCATE] = 1;
    command("vanish all");
    assert(IS_VAMPAFF(&actor, VAM_OBFUSCATE));
    assert(!IS_VAMPAFF(following, VAM_OBFUSCATE));
    assert(!IS_VAMPAFF(other, VAM_OBFUSCATE) && actor.wait == 12);
    REMOVE_BIT(actor.vampaff, VAM_OBFUSCATE);
    player.powers[DISC_OBFUSCATE] = 3;
    command("vanish all");
    assert(IS_VAMPAFF(&actor, VAM_OBFUSCATE));
    assert(IS_VAMPAFF(following, VAM_OBFUSCATE));
    assert(!IS_VAMPAFF(other, VAM_OBFUSCATE) && actor.wait == 12);
    resources();
}

static void test_summoning(void)
{
    fresh(CLASS_VAMPIRE);
    player.powers[DISC_PRESENCE] = 3;
    SET_BIT(visitor.extra, EXTRA_SUMMON);
    command("summoning visitor"); assert(visitor.in_room == &away);
    player.powers[DISC_PRESENCE] = 4;
    REMOVE_BIT(visitor.extra, EXTRA_SUMMON);
    command("summoning visitor"); assert(visitor.in_room == &away);
    SET_BIT(visitor.extra, EXTRA_SUMMON);
    const long blocked[] = {ROOM_SAFE, ROOM_PRIVATE, ROOM_SOLITARY, ROOM_NO_RECALL};
    for (size_t i = 0; i < sizeof(blocked) / sizeof(blocked[0]); ++i)
    {
        away.room_flags = blocked[i];
        command("summoning visitor"); assert(visitor.in_room == &away && actor.wait == 0);
    }
    away.room_flags = 0; visitor.fighting = &actor;
    command("summoning visitor"); assert(visitor.in_room == &away);
    visitor.fighting = NULL; visitor.level = 561;
    command("summoning visitor"); assert(visitor.in_room == &away);
    visitor.level = LEV_AWA;
    command("summoning caster"); assert(actor.wait == 0);
    command("summoning absent"); assert(actor.wait == 0);
    command("summoning visitor");
    assert(visitor.in_room == &home && actor.wait == 12);
    CHAR_DATA *mob = mobile("summonable", &away);
    command("summoning summonable");
    assert(mob->in_room == &home && IS_ACT(mob, ACT_NOEXP));
    assert(actor.wait == 12); resources();
}

static void test_possession(void)
{
    fresh(CLASS_VAMPIRE);
    CHAR_DATA *mob = mobile("mobile", &home);
    player.powers[DISC_DOMINATE] = 2;
    command("possession mobile"); assert(player.familiar == NULL && mob->wizard == NULL);
    player.powers[DISC_DOMINATE] = 3;
    command("possession"); command("possession absent"); command("possession caster");
    char_from_room(&visitor); char_to_room(&visitor, &home);
    command("possession visitor");
    assert(player.familiar == NULL && visitor.wizard == NULL);
    mob->wizard = &visitor;
    command("possession mobile"); assert(player.familiar == NULL);
    mob->wizard = NULL; mob->level = 226;
    command("possession mobile"); assert(player.familiar == NULL);
    mob->level = 225;
    command("possession mobile");
    assert(player.familiar == mob && mob->wizard == &actor);
    CHAR_DATA *other = mobile("other", &home);
    for (int rank = 3; rank <= 7; ++rank)
    {
        player.powers[DISC_DOMINATE] = rank;
        command("possession other");
        assert(player.familiar == mob && mob->wizard == &actor && other->wizard == NULL);
    }
    SET_BIT(other->extra, EXTRA_ILLUSION);
    player.powers[DISC_CHIMERSTRY] = 5;
    command("controlillusion other");
    assert(player.familiar == mob && other->wizard == NULL);
    extract_char(mob, TRUE);
    assert(player.familiar == NULL);
    command("possession other");
    assert(player.familiar == other && other->wizard == &actor);
    assert(actor.wait == 0); resources();
}

static void test_illusions(void)
{
    fresh(CLASS_VAMPIRE);
    command("mirrorimage"); assert(prototype.count == 0 && player.followers == 0);
    player.powers[DISC_CHIMERSTRY] = 1;
    for (int i = 1; i <= 5; ++i)
    {
        command("mirrorimage");
        assert(player.followers == i && prototype.count == i);
        CHAR_DATA *copy = char_list;
        assert(IS_NPC(copy) && copy->in_room == &home);
        assert(IS_EXTRA(copy, EXTRA_ILLUSION) && !strcmp(copy->lord, actor.name));
        assert(copy->level == 50 && copy->hit == actor.hit && copy->max_hit == actor.max_hit);
        assert(copy->mana == actor.mana && copy->max_mana == actor.max_mana);
    }
    command("mirrorimage"); assert(player.followers == 5 && prototype.count == 5);
    resources();
    fresh(CLASS_VAMPIRE);
    CHAR_DATA *mob = mobile("illusion", &home);
    SET_BIT(mob->extra, EXTRA_ILLUSION);
    player.powers[DISC_CHIMERSTRY] = 4;
    command("controlillusion illusion"); assert(player.familiar == NULL);
    player.powers[DISC_CHIMERSTRY] = 5;
    char_from_room(&visitor); char_to_room(&visitor, &home);
    command("controlillusion"); command("controlillusion absent");
    command("controlillusion caster"); command("controlillusion visitor");
    assert(player.familiar == NULL);
    REMOVE_BIT(mob->extra, EXTRA_ILLUSION);
    command("controlillusion illusion"); assert(player.familiar == NULL);
    SET_BIT(mob->extra, EXTRA_ILLUSION); mob->wizard = &visitor;
    command("controlillusion illusion"); assert(player.familiar == NULL);
    mob->wizard = NULL;
    command("controlillusion illusion");
    assert(player.familiar == mob && mob->wizard == &actor);
    CHAR_DATA *other = mobile("other", &home); SET_BIT(other->extra, EXTRA_ILLUSION);
    command("controlillusion other");
    assert(player.familiar == mob && other->wizard == NULL);
    player.powers[DISC_DOMINATE] = 3;
    command("possession other");
    assert(player.familiar == mob && mob->wizard == &actor && other->wizard == NULL);
    assert(actor.wait == 0); resources();
}

static void test_class_guards(void)
{
    const long classes[] = {CLASS_NONE, CLASS_WEREWOLF, CLASS_SORCERER, CLASS_DROW,
        CLASS_HIGHLANDER, CLASS_MAGE, CLASS_DRAGON, CLASS_NINJA, CLASS_MONK, CLASS_ELEM};
    DO_FUN *powers[] = {do_regenerate, do_earthmeld, do_vanish, do_summoning,
        do_possession, do_mirrorimage, do_control};
    for (size_t c = 0; c <= sizeof(classes) / sizeof(classes[0]); ++c)
    {
        fresh(c == sizeof(classes) / sizeof(classes[0]) ? CLASS_VAMPIRE : classes[c]);
        for (int disc = DISC_ANIMALISM; disc <= DISC_MAX; ++disc)
            player.powers[disc] = 7;
        if (IS_VAMPIRE(&actor)) { SET_BIT(actor.act, ACT_IS_NPC); actor.pcdata = NULL; }
        for (size_t p = 0; p < sizeof(powers) / sizeof(powers[0]); ++p)
            (*powers[p])(&actor, "visitor");
        assert(actor.vampaff == 0 && player.regenerate == 0 && player.followers == 0);
        assert(player.familiar == NULL && visitor.in_room == &away && actor.wait == 0);
        resources();
    }
    REMOVE_BIT(actor.act, ACT_IS_NPC); actor.pcdata = &player;
}

int main(void)
{
    string_space = calloc(1, 1024 * 1024);
    assert(string_space); top_string = string_space;
    current_time = 123456; init_mm();
    test_states(); test_obfuscate(); test_summoning();
    test_possession(); test_illusions(); test_class_guards();
    cleanup();
    puts("vampire module extraction full-engine regressions passed");
    return 0;
}
