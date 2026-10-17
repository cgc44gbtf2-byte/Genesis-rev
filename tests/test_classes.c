/*
 * Isolated autoclass regressions with synthetic characters only.
 * From mudfix (compile sequentially):
 * cc -std=gnu17 -O0 -ffunction-sections -fdata-sections \
 *   tests/test_classes.c -Wl,--gc-sections -o /tmp/test_classes
 * /tmp/test_classes
 * Add -fsanitize=address,undefined -fno-omit-frame-pointer for sanitizers.
 */
#include "../Genesis/src/clan.c"
#include <assert.h>
#include <ctype.h>
#include <strings.h>

static char output[8192];
static char *owned_string;
static int duplicates, releases;
long gsn_potency;
bool is_affected(CHAR_DATA *ch, long sn)
{
    (void)ch;
    (void)sn;
    return FALSE; /* This isolated class-selection fixture has no affects. */
}

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
    (void)ch;
    assert(strlen(output) + strlen(message) < sizeof(output));
    strcat(output, message);
}

/* Match ordinary, unswitched characters; no descriptor fixtures are needed. */
long get_trust(CHAR_DATA *ch)
{
    return IS_NPC(ch) ? 1 : ch->level;
}

long get_disc(CHAR_DATA *ch, long disc) { return ch->pcdata->powers[disc]; }
void do_wwpowers(CHAR_DATA *ch, char *arg)
{ (void)arg; send_to_char("Werewolf listing\n", ch); }
void do_magepowers(CHAR_DATA *ch, char *arg)
{ (void)arg; send_to_char("Mage listing\n", ch); }

/* Isolate the selection thresholds from the separate stat-cap calculation. */
int raw_stat(CHAR_DATA *ch, int stat)
{
    assert(stat >= STAT_STR && stat <= STAT_WIS);
    return ch->stats[stat];
}

/* Enforce the production string ownership contract, including replacement. */
char *str_dup(const char *text)
{
    assert(text != NULL && owned_string == NULL);
    owned_string = malloc(strlen(text) + 1);
    assert(owned_string != NULL);
    strcpy(owned_string, text);
    ++duplicates;
    return owned_string;
}

void free_string(char *text)
{
    assert(text != NULL && text == owned_string);
    free(text);
    owned_string = NULL;
    ++releases;
}

static void fixture(CHAR_DATA *ch, PC_DATA *pc)
{
    int i;
    assert(owned_string == NULL);
    memset(ch, 0, sizeof(*ch));
    memset(pc, 0, sizeof(*pc));
    ch->pcdata = pc;
    ch->class = CLASS_NONE;
    ch->level = LEV_AWA;
    ch->mkill = 5;
    for (i = 0; i < 5; ++i)
        ch->stats[i] = 4;
    for (i = 0; i < 3; ++i)
        pc->cland[i] = 42;
    duplicates = releases = 0;
    ch->clan = str_dup("Previous affiliation");
    output[0] = '\0';
}

static void reject(CHAR_DATA *ch, char *command, const char *message)
{
    CHAR_DATA before = *ch;
    PC_DATA before_pc;
    if (ch->pcdata != NULL)
        before_pc = *ch->pcdata;
    do_autoclass(ch, command);
    assert(memcmp(ch, &before, sizeof(*ch)) == 0);
    if (ch->pcdata != NULL)
        assert(memcmp(ch->pcdata, &before_pc, sizeof(before_pc)) == 0);
    assert(duplicates == 1 && releases == 0);
    assert(strstr(output, message) != NULL);
}

static void check_clan(CHAR_DATA *ch, const char *name)
{
    char first;
    assert(duplicates == 2 && releases == 1);
    assert(ch->clan == owned_string);
    assert(strcmp(ch->clan, name) == 0);
    /* Selected names must be owned, writable strings, not string literals. */
    first = ch->clan[0];
    ch->clan[0] = '?';
    ch->clan[0] = first;
}

static void test_prerequisites(void)
{
    static const struct {
        char *command;
        long class;
        int minimum[5];
    } cases[] = {
        {"vampire ventrue", CLASS_VAMPIRE, {3, 2, 4, 2, 1}},
        {"werewolf", CLASS_WEREWOLF, {4, 2, 3, 1, 2}},
        {"sorcerer", CLASS_SORCERER, {2, 2, 1, 4, 3}},
        {"monk", CLASS_MONK, {2, 3, 2, 1, 4}},
        {"dragon bronze", CLASS_DRAGON, {4, 1, 3, 2, 2}},
        {"highlander", CLASS_HIGHLANDER, {4, 2, 3, 1, 2}},
        {"ninja", CLASS_NINJA, {3, 4, 2, 2, 1}},
        {"drow", CLASS_DROW, {2, 4, 1, 3, 2}},
        {"elementalist", CLASS_ELEM, {2, 1, 2, 3, 4}},
        {"mage akashic", CLASS_MAGE, {1, 2, 2, 4, 3}}
    };
    CHAR_DATA ch;
    PC_DATA pc;
    size_t i;
    int stat, j;

    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i)
    {
        for (stat = 0; stat < 5; ++stat)
        {
            fixture(&ch, &pc);
            for (j = 0; j < 5; ++j)
                ch.stats[j] = cases[i].minimum[j];
            --ch.stats[stat];
            reject(&ch, cases[i].command, "requirements");
            free_string(ch.clan);
        }
        fixture(&ch, &pc);
        for (j = 0; j < 5; ++j)
            ch.stats[j] = cases[i].minimum[j];
        do_autoclass(&ch, cases[i].command);
        assert(ch.class == cases[i].class);
        assert(strstr(output, "You are now") != NULL);
        if (ch.class == CLASS_WEREWOLF)
        {
            assert(pc.rank == 1 && ch.vampgen == 6);
            assert(pc.tgift[0] == -1 && pc.tgift[1] == -1 && pc.tgift[2] == -1);
        }
        if (ch.class == CLASS_SORCERER)
            assert(ch.level == LEV_SOR && ch.vampgen == 4);
        if (ch.class == CLASS_MONK || ch.class == CLASS_NINJA)
            assert(ch.vampgen == 4);
        if (ch.class == CLASS_DRAGON)
            assert(ch.vampgen == 12);
        if (ch.class == CLASS_DROW)
            assert(ch.vampgen == 5);
        free_string(ch.clan);
    }
}

static void test_vampires(void)
{
    static const struct {
        char *command;
        const char *name;
        int disciplines[3];
        int extra;
    } cases[] = {
        {"VaMpIrE VeNtRuE", "Ventrue",
            {DISC_DOMINATE, DISC_FORTITUDE, DISC_PRESENCE}, -1},
        {"vampire nosferatu", "Nosferatu",
            {DISC_ANIMALISM, DISC_OBFUSCATE, DISC_POTENCE}, -1},
        {"vampire brujah", "Brujah",
            {DISC_CELERITY, DISC_POTENCE, DISC_PRESENCE}, -1},
        {"vampire assamite", "Assamite",
            {DISC_CELERITY, DISC_OBFUSCATE, DISC_QUIETUS}, DISC_QUIETUS},
        {"vampire gangrel", "Gangrel",
            {DISC_ANIMALISM, DISC_FORTITUDE, DISC_PROTEAN}, DISC_PROTEAN},
        {"vampire serpentsofthelight", "Serpents of the Light",
            {DISC_OBFUSCATE, DISC_PRESENCE, DISC_SERPENTIS}, DISC_SERPENTIS},
        {"vampire lasombra", "Lasombra",
            {DISC_DOMINATE, DISC_OBTENEBRATION, DISC_POTENCE}, DISC_OBTENEBRATION},
        {"vampire tzimisce", "Tzimisce",
            {DISC_AUSPEX, DISC_ANIMALISM, DISC_VICISSITUDE}, DISC_VICISSITUDE},
        {"vampire ravnos", "Ravnos",
            {DISC_FORTITUDE, DISC_ANIMALISM, DISC_CHIMERSTRY}, DISC_CHIMERSTRY}
    };
    static const int common[] = {
        DISC_ANIMALISM, DISC_AUSPEX, DISC_CELERITY, DISC_DOMINATE,
        DISC_FORTITUDE, DISC_OBFUSCATE, DISC_POTENCE, DISC_PRESENCE
    };
    CHAR_DATA ch;
    PC_DATA pc;
    size_t i, j;
    int disc;
    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i)
    {
        fixture(&ch, &pc);
        do_autoclass(&ch, cases[i].command);
        assert(ch.class == CLASS_VAMPIRE && ch.vampgen == 9);
        assert(pc.willpower[0] == 80 && pc.willpower[1] == 80);
        assert(ch.blood[BLOOD_POOL] == 300);
        check_clan(&ch, cases[i].name);
        for (j = 0; j < 3; ++j)
            assert(pc.cland[j] == cases[i].disciplines[j]);
        for (disc = 0; disc <= DISC_MAX; ++disc)
        {
            int expected = disc == cases[i].extra ? -1 : 0;
            for (j = 0; j < sizeof(common) / sizeof(common[0]); ++j)
                if (disc == common[j])
                    expected = -1;
            assert(pc.powers[disc] == expected);
        }
        free_string(ch.clan);
    }
}

static void test_mages_and_dragons(void)
{
    static const struct {
        char *command;
        const char *name;
        int sphere;
    } mages[] = {
        {"mage akashic", "Akashic Brotherhood", SPHERE_MIND},
        {"mage celestial", "Celestial Chorus", SPHERE_PRIME},
        {"mage ecstasy", "Cult of Ecstasy", SPHERE_TIME},
        {"mage dreamspeakers", "Dreamspeakers", SPHERE_SPIRIT},
        {"mage euthanatos", "Euthanatos", SPHERE_ENTROPY},
        {"mage hermes", "Order of Hermes", SPHERE_FORCES},
        {"mage ether", "Sons of Ether", SPHERE_MATTER},
        {"mage verbena", "Verbena", SPHERE_LIFE},
        {"mage virtual", "Virtual Adepts", SPHERE_CORRESPONDENCE},
        {"mage hollow", "Hollow Ones", -1}
    };
    static const struct {
        char *command;
        int color;
    } dragons[] = {
        {"dragon bronze", PURPLE_MAGIC}, {"dragon red", RED_MAGIC},
        {"dragon blue", BLUE_MAGIC}, {"dragon silver", GREEN_MAGIC},
        {"dragon gold", YELLOW_MAGIC}, {"dragon black", BLACK_MAGIC}
    };
    CHAR_DATA ch;
    PC_DATA pc;
    size_t i;
    for (i = 0; i < sizeof(mages) / sizeof(mages[0]); ++i)
    {
        fixture(&ch, &pc);
        do_autoclass(&ch, mages[i].command);
        assert(ch.class == CLASS_MAGE && ch.arete == 1);
        check_clan(&ch, mages[i].name);
        assert(pc.cland[0] == mages[i].sphere);
        assert(pc.cland[1] == -1 && pc.cland[2] == -1);
        free_string(ch.clan);
    }
    for (i = 0; i < sizeof(dragons) / sizeof(dragons[0]); ++i)
    {
        fixture(&ch, &pc);
        do_autoclass(&ch, dragons[i].command);
        assert(ch.class == CLASS_DRAGON && ch.vampgen == 12);
        assert(pc.dragcol == dragons[i].color);
        assert(duplicates == 1 && releases == 0);
        free_string(ch.clan);
    }
}

static void test_rejections(void)
{
    static char *invalid[] = {
        "", "unknown", "vampire", "vampire unknown", "vampire malkavian",
        "mage", "mage unknown", "dragon", "dragon unknown"
    };
    CHAR_DATA ch;
    PC_DATA pc;
    size_t i;
    for (i = 0; i < sizeof(invalid) / sizeof(invalid[0]); ++i)
    {
        fixture(&ch, &pc);
        reject(&ch, invalid[i], "Syntax:");
        assert(strstr(output, "Malkavian") == NULL);
        free_string(ch.clan);
    }
    fixture(&ch, &pc);
    ch.level = LEV_AWA - 1;
    reject(&ch, "vampire ventrue", "5 mkills");
    free_string(ch.clan);

    fixture(&ch, &pc);
    ch.class = CLASS_MAGE;
    reject(&ch, "vampire ventrue", "already have a class");
    free_string(ch.clan);

    fixture(&ch, &pc);
    ch.act = ACT_IS_NPC;
    ch.pcdata = NULL;
    reject(&ch, "vampire ventrue", "");
    free_string(ch.clan);

    fixture(&ch, &pc);
    ch.pcdata = NULL;
    reject(&ch, "vampire ventrue", "player data is unavailable");
    free_string(ch.clan);
}

static void test_power_listings(void)
{
    CHAR_DATA ch = {0};
    PC_DATA pc = {0};
    ch.pcdata = &pc;
    ch.class = CLASS_WEREWOLF;
    output[0] = '\0';
    do_powers(&ch, "");
    assert(strstr(output, "Werewolf listing") && !strstr(output, "Huh?"));
    ch.class = CLASS_VAMPIRE;
    struct { int disc, rank; char *yes, *no; } cases[] = {
        {DISC_ANIMALISM, 4, "Beastwithin", " Fleshbond"},
        {DISC_CHIMERSTRY, 7, "Horrid", "Horridreality"},
        {DISC_VICISSITUDE, 1, "Malleable", "Changeling"},
        {DISC_PRESENCE, 7, "Empathy", "MaskEmpathy"},
        {DISC_DOMINATE, 3, "Possession", "Huh?"},
        {DISC_NECROMANCY, 1, "Spiriteyes", " Preserve"},
        {DISC_NECROMANCY, 2, "Preserve", " Spiritgate"},
        {DISC_NECROMANCY, 3, "Spiritgate", " Spiritguardian"},
        {DISC_NECROMANCY, 4, "Spiritguardian", " Zombie"},
        {DISC_NECROMANCY, 5, "Zombie", " Torment"},
        {DISC_OBFUSCATE, 6, "Shield", "Huh?"},
        {DISC_QUIETUS, 6, "Leech", "Huh?"},
        {DISC_POTENCE, 7, "Potence: Fist", "All your discipline powers are automatic"}
    };
    for (size_t i = 0; i < sizeof(cases)/sizeof(cases[0]); ++i)
    {
        memset(pc.powers, 0, sizeof(pc.powers));
        pc.powers[cases[i].disc] = cases[i].rank;
        output[0] = '\0';
        do_powers(&ch, "");
        assert(strstr(output, cases[i].yes) && !strstr(output, cases[i].no));
    }
}

int main(void)
{
    test_power_listings();
    test_prerequisites();
    test_vampires();
    test_mages_and_dragons();
    test_rejections();
    assert(owned_string == NULL);
    puts("class acquisition regressions passed");
    return 0;
}
