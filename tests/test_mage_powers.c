/* Synthetic fixtures only; compile this TU with -ffunction-sections
 * -fdata-sections and link with -Wl,--gc-sections. It includes real spells. */
#include <assert.h>
#include "../Genesis/src/classes/mage/mage.c"

static CHAR_DATA caster, target;
static PC_DATA caster_pc, target_pc;
static ROOM_INDEX_DATA home, remote;
static AREA_DATA area;
static bool protected_room;
static int safe_calls, die = 10;
static long damage_total;
static int spiritward_calls;
static bool extract_npc;
static char messages[16384];

void send_to_char(const char *text, CHAR_DATA *ch)
{
    (void)ch;
    assert(strlen(messages) + strlen(text) < sizeof(messages));
    strcat(messages, text);
}
void do_spiritward(CHAR_DATA *ch, char *arg)
{ assert(IS_WEREWOLF(ch)); (void)arg; ++spiritward_calls; }
void act(const char *s, CHAR_DATA *ch, const void *a, const void *b, long t)
{ (void)s; (void)ch; (void)a; (void)b; (void)t; }
char *one_argument(char *s, char *out)
{
    while (*s == ' ') ++s;
    while (*s && *s != ' ') *out++ = *s++;
    *out = '\0';
    while (*s == ' ') ++s;
    return s;
}
bool str_cmp(const char *a, const char *b) { return strcmp(a, b) != 0; }
CHAR_DATA *get_char_world(CHAR_DATA *ch, char *name)
{ (void)ch; return !strcmp(name, "target") ? &target : NULL; }
CHAR_DATA *get_char_room(CHAR_DATA *ch, char *name)
{ return ch->in_room == target.in_room ? get_char_world(ch, name) : NULL; }
void char_from_room(CHAR_DATA *ch)
{
    CHAR_DATA **p = &ch->in_room->people;
    while (*p && *p != ch) p = &(*p)->next_in_room;
    assert(*p == ch);
    *p = ch->next_in_room;
    ch->next_in_room = NULL;
    ch->in_room = NULL;
}
void char_to_room(CHAR_DATA *ch, ROOM_INDEX_DATA *room)
{
    assert(!ch->in_room);
    ch->in_room = room;
    ch->next_in_room = room->people;
    room->people = ch;
}
bool is_safe(CHAR_DATA *ch, CHAR_DATA *victim)
{
    assert(ch->in_room != NULL);
    assert(ch->in_room == victim->in_room);
    ++safe_calls;
    return protected_room;
}
int number_range(int a, int b) { return a == 1 && b == 10 ? die : a; }
int number_percent(void) { return 100; }
void hurt_person(CHAR_DATA *ch, CHAR_DATA *victim, long dam)
{
    (void)ch;
    damage_total += dam;
    if (extract_npc && IS_NPC(victim))
    {
        char_from_room(victim);
        free(victim);
        return;
    }
    victim->hit -= dam;
}
void update_pos(CHAR_DATA *ch) { (void)ch; }
void set_fighting(CHAR_DATA *ch, CHAR_DATA *v) { ch->fighting = v; }
bool is_same_group(CHAR_DATA *ch, CHAR_DATA *v) { return ch == v; }
long skill_lookup(const char *s) { (void)s; return 1; }
bool is_affected(CHAR_DATA *ch, long sn) { (void)ch; (void)sn; return FALSE; }
void affect_to_char(CHAR_DATA *ch, AFFECT_DATA *af) { (void)ch; (void)af; }
OBJ_DATA *get_eq_char(CHAR_DATA *ch, long wear) { (void)ch; (void)wear; return NULL; }
void obj_from_char(OBJ_DATA *o) { (void)o; }
void obj_to_char(OBJ_DATA *o, CHAR_DATA *ch) { (void)o; (void)ch; }
void free_string(char *s) { free(s); }
char *str_dup(const char *s) { return strdup(s); }
void update_style(CHAR_DATA *ch) { (void)ch; }
#define COMMAND_STUB(name) void name(CHAR_DATA *ch, char *arg) { (void)ch; (void)arg; }
COMMAND_STUB(do_stance)
COMMAND_STUB(do_doppleganger)
COMMAND_STUB(do_mask)
COMMAND_STUB(do_truedisguise)
COMMAND_STUB(do_malleable)
COMMAND_STUB(do_change)
COMMAND_STUB(do_serpent)
COMMAND_STUB(do_shadowbody)
COMMAND_STUB(do_unwerewolf)
COMMAND_STUB(do_horns)
COMMAND_STUB(do_wings)
COMMAND_STUB(do_claws)
COMMAND_STUB(do_nightsight)
COMMAND_STUB(do_fangs)

static void reset(void)
{
    memset(&caster, 0, sizeof(caster));
    memset(&target, 0, sizeof(target));
    memset(&caster_pc, 0, sizeof(caster_pc));
    memset(&target_pc, 0, sizeof(target_pc));
    memset(&home, 0, sizeof(home));
    memset(&remote, 0, sizeof(remote));
    home.area = remote.area = &area;
    caster.pcdata = &caster_pc;
    target.pcdata = &target_pc;
    caster.class = target.class = CLASS_MAGE;
    caster.name = "caster";
    target.name = "target";
    caster.mana = 100000;
    caster.hit = target.hit = 10000;
    target.max_hit = 20000;
    caster.quint[BODY] = 10;
    caster.arete = 10;
    for (int i = 0; i < 9; ++i) caster.spheres[i] = 6;
    char_to_room(&caster, &home);
    char_to_room(&target, &remote);
    protected_room = FALSE;
    safe_calls = 0;
    damage_total = 0;
    extract_npc = FALSE;
    die = 10;
    messages[0] = '\0';
}

static void unchanged(long mana)
{
    assert(caster.in_room == &home && home.people == &caster);
    assert(caster.next_in_room == NULL);
    assert(remote.people == &target && target.next_in_room == NULL);
    assert(caster.mana == mana && caster.quint[BODY] == 10);
    assert(caster.hit == 10000 && damage_total == 0);
    assert(caster.paradox == 0);
}

int main(void)
{
    for (long available = 0; available <= 2; ++available)
    {
        reset();
        caster.quint[BODY] = available;
        SET_BIT(caster.xtra, XTRA_HEARTS_BLOOD);
        char args[64];
        snprintf(args, sizeof(args), "target %ld", available + 1);
        do_mage_heal(&caster, args);
        assert(caster.quint[BODY] == 0);
        assert(caster.hit == 9800 && damage_total == 200);
        long correspondence = correspondence_level(&caster, &target);
        assert(caster.mana == 100000 - MANA_MAGE_HEAL
            - correspondence * correspondence * 100 - (available + 1) * MANA_QUINT);
        assert(caster.in_room == &home && remote.people == &target);
    }
    reset();
    assert(!mage_target_safe(&caster, &caster));
    unchanged(100000);
    assert(safe_calls == 1);
    protected_room = TRUE;
    assert(mage_target_safe(&caster, &caster));
    unchanged(100000);
    reset();
    char_from_room(&target);
    char_to_room(&target, &home);
    assert(!mage_target_safe(&caster, &target));
    assert(home.people == &target && target.next_in_room == &caster);
    assert(caster.next_in_room == NULL && caster.in_room == &home);
    assert(remote.people == NULL && safe_calls == 1);
    reset();
    {
        CHAR_DATA *mob = calloc(1, sizeof(*mob));
        assert(mob);
        mob->act = ACT_IS_NPC;
        mob->name = "synthetic mob";
        mob->hit = mob->max_hit = 10000;
        char_to_room(mob, &home);
        extract_npc = TRUE;
        do_inferno(&caster, "0");
        assert(damage_total > 0 && home.people == &caster);
        assert(caster.mana == 95000 && caster.wait == 12);
    }
    reset();
    caster.class = CLASS_WEREWOLF;
    do_paradox_ward(&caster, "");
    assert(spiritward_calls == 1 && caster.mana == 100000);
    reset();
    for (int i = 0; i < 9; ++i) caster.spheres[i] = 3;
    do_magepowers(&caster, "");
    const char *third_rank[] = {"brittlebones", "frictioncurse", "betterbody",
        "sculpture", "shield", "mageenchant", "stepsideways", "contingency"};
    for (unsigned i = 0; i < sizeof(third_rank)/sizeof(third_rank[0]); ++i)
        assert(strstr(messages, third_rank[i]));
    void (*spells[])(CHAR_DATA *, char *) = {
        do_brittle_bones, do_mutate_ephemera, do_alterweight,
        do_mage_heal, do_better_body, do_polymorph_other, do_electrical_chaos
    };
    const char *bad[] = {"-1", "oops", "1junk", "99999999999999999999999999999"};
    char args[128];
    for (unsigned i = 0; i < sizeof(spells)/sizeof(spells[0]); ++i)
    {
        reset();
        protected_room = TRUE;
        /* Electrical chaos forbids global PC targets: use a local target
         * for its protected-room test, and test global rejection below. */
        if (i == 6) target.act |= ACT_IS_NPC;
        snprintf(args, sizeof(args), "target %s2", i == 4 ? "str " : "");
        spells[i](&caster, args);
        unchanged(100000);
        assert(safe_calls == 1);
        reset();
        caster.mana = 0;
        caster.xtra |= XTRA_HEARTS_BLOOD;
        caster.quint[BODY] = 10;
        snprintf(args, sizeof(args), "target %s12", i == 4 ? "str " : "");
        spells[i](&caster, args);
        unchanged(0);
        reset();
        snprintf(args, sizeof(args), "missing %s2", i == 4 ? "str " : "");
        spells[i](&caster, args);
        unchanged(100000);
        for (unsigned j = 0; j < sizeof(bad)/sizeof(bad[0]); ++j)
        {
            reset();
            snprintf(args, sizeof(args), "target %s%s", i == 4 ? "str " : "", bad[j]);
            spells[i](&caster, args);
            unchanged(100000);
        }
    }
    reset();
    do_better_body(&caster, "target invalid 2");
    unchanged(100000);
    reset();
    target.xtra |= XTRA_BETTER_STR;
    do_better_body(&caster, "target dex 2");
    unchanged(100000);
    reset();
    do_electrical_chaos(&caster, "target 2");
    unchanged(100000);
    reset();
    target.act |= ACT_IS_NPC;
    do_polymorph_other(&caster, "target 2");
    unchanged(100000);
    reset();
    caster.spheres[MCOR] = 1;
    do_brittle_bones(&caster, "target 2");
    unchanged(100000);
    reset();
    do_mage_heal(&caster, "target 2");
    assert(caster.in_room == &home && remote.people == &target);
    assert(caster.mana == 94400 && caster.quint[BODY] == 8);
    reset();
    target.pcdata->atm = 100;
    do_brittle_bones(&caster, "target 2");
    assert(caster.mana == 94900 && caster.quint[BODY] == 8);
    /* Exercise inferno with zero/nonpositive and positive rolled power. */
    for (int roll = 1; roll <= 10; roll += 9)
    {
        reset();
        die = roll;
        do_inferno(&caster, "");
        assert(caster.mana == 95000);
    }
    puts("mage powers: passed");
    return 0;
}
