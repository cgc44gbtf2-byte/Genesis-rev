/* Full engine tests on synthetic characters and a private temporary save
 * directory. Never load historical accounts or world files. */
#define _DEFAULT_SOURCE
#define ATOMIC_SAVE_TESTING
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../Genesis/src/merc.h"

extern void init_mm(void);
#include "../Genesis/src/atomic_save.h"

void fwrite_char(CHAR_DATA *, FILE *);
void fread_char(CHAR_DATA *, FILE *);
void char_update(void);
extern char *string_space, *top_string;

static CHAR_DATA chars[4];
static PC_DATA players[4];
static DESCRIPTOR_DATA descriptors[4];
static char output[4][65536];
static ROOM_INDEX_DATA room, elsewhere;
static AREA_DATA area;
static char *names[] = {"teacher", "student", "other", "mobile"};

static void reset(void)
{
    for (int i = 0; i < 4; ++i)
        while (chars[i].affected != NULL)
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
    gsn_potency = skill_lookup("blood potency");
    assert(gsn_potency > 0 && gsn_potency < MAX_SKILL);
    for (int i = 0; i < 4; ++i)
    {
        clear_char(&chars[i]);
        chars[i].name = chars[i].short_descr = names[i];
        chars[i].contingency = "";
        chars[i].pcdata = &players[i];
        chars[i].in_room = &room;
        chars[i].class = CLASS_VAMPIRE;
        chars[i].level = LEV_AWA;
        chars[i].vampgen = 9;
        chars[i].next = i < 3 ? &chars[i + 1] : NULL;
        chars[i].next_in_room = chars[i].next;
        chars[i].style[0] = STYLE_UNARMED;
        chars[i].save_time = current_time;
        players[i].flag = FLAG_PK;
        players[i].love = players[i].conding = "";
        players[i].pwd = players[i].bamfin = players[i].bamfout = "";
        players[i].cdest = players[i].title = players[i].decap_char = "";
        players[i].marriage = players[i].email = players[i].covenant = "";
        players[i].cov_time = players[i].leader = players[i].vleader = "";
        players[i].cland[0] = players[i].cland[1] = players[i].cland[2] = -1;
        descriptors[i].character = &chars[i];
        descriptors[i].connected = CON_PLAYING;
        descriptors[i].outbuf = output[i];
        descriptors[i].outsize = sizeof(output[i]);
        chars[i].desc = &descriptors[i];
        update_gen(&chars[i]);
    }
    chars[3].act = ACT_IS_NPC;
    chars[3].desc = NULL;
    chars[3].pcdata = NULL;
    atomic_save_test_fault = SAVE_FAULT_NONE;
}

static void input(int who, const char *command)
{
    snprintf(descriptors[who].incomm, sizeof(descriptors[who].incomm), "%s", command);
    interpret(&chars[who], descriptors[who].incomm);
    descriptors[who].incomm[0] = '\0';
}

static void offer(void)
{
    players[0].powers[DISC_THAUMATURGY] = 3; /* Non-clan teaching is intentional. */
    input(0, "teachdisc student thaumaturgy");
    assert(players[1].teach_disc == DISC_THAUMATURGY);
    assert(!strcmp(players[1].teach_from, "teacher"));
    assert(players[1].powers[DISC_THAUMATURGY] == 0);
}

static void test_teaching(void)
{
    reset();
    offer();
    input(1, "teachdisc accept other thaumaturgy");
    input(1, "teachdisc accept teacher auspex");
    assert(players[1].powers[DISC_THAUMATURGY] == 0);
    /* Broad feeding consent is NOT a substitute for explicit acceptance. */
    chars[1].extra |= EXTRA_VAMPIRE;
    assert(players[1].powers[DISC_THAUMATURGY] == 0);
    players[0].powers[DISC_THAUMATURGY] = 2;
    input(1, "teachdisc accept teacher thaumaturgy");
    assert(players[1].powers[DISC_THAUMATURGY] == 0);
    players[0].powers[DISC_THAUMATURGY] = 3;
    chars[1].affected_by = AFF_ETHEREAL;
    input(1, "teachdisc accept teacher thaumaturgy");
    assert(players[1].powers[DISC_THAUMATURGY] == 0);
    chars[1].affected_by = 0;
    current_time += 120;
    input(1, "teachdisc accept teacher thaumaturgy");
    assert(players[1].powers[DISC_THAUMATURGY] == 0);
    offer();
    input(1, "teachdisc decline");
    input(1, "teachdisc accept teacher thaumaturgy");
    assert(players[1].powers[DISC_THAUMATURGY] == 0);
    offer();
    /* A forged/nested interpreter call cannot accept a lesson. */
    {
        char forced[] = "teachdisc accept teacher thaumaturgy";
        interpret(&chars[1], forced);
    }
    assert(players[1].powers[DISC_THAUMATURGY] == 0);
    /* Exercise the real Vampire Dominate path, not only the guard directly. */
    players[0].powers[DISC_DOMINATE] = 3;
    chars[1].vampgen = 10;
    do_command(&chars[0], "student teachdisc accept teacher thaumaturgy");
    assert(players[1].powers[DISC_THAUMATURGY] == 0);
    chars[0].wait = chars[1].wait = 0;
    input(1, "teachdisc accept teacher thaumaturgy");
    assert(players[1].powers[DISC_THAUMATURGY] == -1);
    assert(players[1].teach_until == 0);
    assert(chars[0].exp == 0 && chars[1].exp == 0);
    assert(chars[0].primal == 0 && chars[1].primal == 0);
    assert(access("../player/Student", F_OK) == 0);
    chars[1].primal = 15;
    do_discipline(&chars[1], "thaumaturgy");
    assert(players[1].powers[DISC_THAUMATURGY] == 1 && chars[1].primal == 0);
    players[1].powers[DISC_THAUMATURGY] = 5;
    chars[1].primal = 1000;
    do_discipline(&chars[1], "thaumaturgy");
    assert(players[1].powers[DISC_THAUMATURGY] == 5 && chars[1].primal == 1000);
    players[1].powers[DISC_PROTEAN] = -1;
    players[1].cland[0] = DISC_PROTEAN;
    chars[1].primal = 5;
    do_discipline(&chars[1], "protean");
    assert(players[1].powers[DISC_PROTEAN] == 1 && chars[1].primal == 0);

    reset();
    do_teachdisc(NULL, "");
    do_teachdisc(&chars[3], "student thaumaturgy");
    input(0, "teachdisc teacher thaumaturgy");
    input(0, "teachdisc student daimoinon");
    input(0, "teachdisc student 4");
    assert(players[1].teach_until == 0);
    players[0].powers[DISC_THAUMATURGY] = 2;
    input(0, "teachdisc student thaumaturgy");
    assert(players[1].teach_until == 0);
    players[0].powers[DISC_THAUMATURGY] = 3;
    chars[1].class = CLASS_NONE;
    input(0, "teachdisc student thaumaturgy");
    assert(players[1].teach_until == 0);
    chars[1].class = CLASS_VAMPIRE;
    chars[1].desc = NULL;
    input(0, "teachdisc student thaumaturgy");
    assert(players[1].teach_until == 0);
    chars[1].desc = &descriptors[1];
    chars[1].combat_pause = 1;
    input(0, "teachdisc student thaumaturgy");
    assert(players[1].teach_until == 0);
    chars[1].combat_pause = 0;
    chars[1].position = POS_FIGHTING;
    input(0, "teachdisc student thaumaturgy");
    assert(players[1].teach_until == 0);
}

static void test_save_failure(void)
{
    reset();
    offer();
    assert(rename("../player", "../player-off") == 0);
    input(1, "teachdisc accept teacher thaumaturgy");
    assert(players[1].powers[DISC_THAUMATURGY] == 0);
    assert(players[1].teach_until > current_time);
    assert(rename("../player-off", "../player") == 0);
    atomic_save_test_fault = SAVE_FAULT_DIRSYNC;
    input(1, "teachdisc accept teacher thaumaturgy");
    assert(players[1].powers[DISC_THAUMATURGY] == 0);
    assert(players[1].teach_until > current_time);
    assert(strstr(output[1], "not confirmed") != NULL);
    atomic_save_test_fault = SAVE_FAULT_NONE;
    input(1, "teachdisc accept teacher thaumaturgy");
    assert(players[1].powers[DISC_THAUMATURGY] == -1);
    assert(chars[0].exp == 0 && chars[1].exp == 0);
}

static void assert_unchanged(void)
{
    assert(chars[0].blood[0] == 100 && chars[1].blood[0] == 100);
    assert(chars[0].hit == 10000 && chars[1].hit == 10000);
    assert(chars[0].wait == 0 && chars[0].fighting == NULL);
}

static void test_rejections(DO_FUN *power, int rank, long discipline)
{
    reset();
    power(NULL, "");
    power(&chars[3], "student");
    assert_unchanged();
    players[0].powers[discipline] = rank - 1;
    power(chars, "student");
    assert_unchanged();
    players[0].powers[discipline] = rank;
    power(chars, "teacher");
    power(chars, "missing");
    power(chars, "student surplus");
    assert_unchanged();
    chars[0].combat_pause = 1;
    power(chars, "student");
    assert_unchanged();
    chars[0].combat_pause = 0;
    chars[0].hit = 0;
    power(chars, "student");
    assert(chars[0].blood[0] == 100 && chars[1].blood[0] == 100);
    chars[0].hit = 10000;
    chars[1].in_room = &elsewhere;
    power(chars, "student");
    assert_unchanged();
    chars[1].in_room = &room;
    chars[1].affected_by = AFF_SHADOWPLANE;
    power(chars, "student");
    assert_unchanged();
    chars[1].affected_by = 0;
    room.room_flags = ROOM_SAFE;
    power(chars, "student");
    assert_unchanged();
    room.room_flags = 0;
    players[1].flag |= FLAG_AFK;
    power(chars, "student");
    assert_unchanged();
    players[1].flag &= ~FLAG_AFK;
    chars[1].desc = NULL;
    power(chars, "student");
    assert_unchanged();
    chars[1].desc = &descriptors[1];
    chars[0].affected_by = AFF_CHARM;
    chars[0].master = &chars[1];
    power(chars, "student");
    assert_unchanged();
    chars[0].affected_by = 0;
    chars[0].master = NULL;
    chars[1].hit = 0;
    power(chars, "student");
    assert(chars[0].blood[0] == 100 && chars[1].blood[0] == 100);
    chars[1].hit = 10000;
    chars[3].act |= ACT_NOKILL;
    power(chars, "mobile");
    assert_unchanged();
    chars[3].act &= ~ACT_NOKILL;
    chars[3].level = 701;
    power(chars, "mobile");
    assert_unchanged();
}

static void test_blood_magic(void)
{
    reset();
    players[0].powers[DISC_THAUMATURGY] = 1;
    do_tasteblood(chars, "student");
    assert(strstr(output[0], "Generation: 9") != NULL);
    assert(strstr(output[0], "Blood potency: 3") != NULL);
    assert(chars[0].blood[0] == 100 && chars[0].wait == 12);
    chars[0].wait = 0;
    do_bloodrage(chars, "student regenerate");
    assert(players[1].regenerate == 0);
    players[0].powers[DISC_THAUMATURGY] = 2;
    do_bloodrage(chars, "student boost");
    do_bloodrage(chars, "student regenerate");
    assert(players[1].regenerate == 0 && chars[0].wait == 0);
    chars[1].extra |= EXTRA_VAMPIRE;
    do_bloodrage(chars, "student regenerate");
    assert(players[1].regenerate == 10 && chars[0].wait == 12);
    chars[0].wait = 0;
    do_bloodrage(chars, "student regenerate");
    assert(players[1].regenerate == 10 && chars[0].wait == 0);

    reset();
    players[0].powers[DISC_THAUMATURGY] = 4;
    /* Explicit target must win even while fighting someone else. */
    chars[0].fighting = &chars[1];
    do_theftvitae(chars, "other");
    assert(chars[1].blood[0] == 100 && chars[2].blood[0] < 100);
    assert(chars[0].blood[0] + chars[2].blood[0] == 200);
    assert(chars[2].fighting == chars);
    reset();
    players[0].powers[DISC_THAUMATURGY] = 4;
    chars[0].blood[0] = 299;
    chars[1].blood[0] = 2;
    do_theftvitae(chars, "student");
    assert(chars[0].blood[0] == 300 && chars[1].blood[0] == 1);
    reset();
    players[0].powers[DISC_THAUMATURGY] = 4;
    chars[0].blood[0] = 300;
    do_theftvitae(chars, "student");
    assert(chars[1].blood[0] == 100 && chars[0].wait == 0);
    reset();
    players[0].powers[DISC_THAUMATURGY] = 4;
    players[1].powers[DISC_VICISSITUDE] = 6;
    do_theftvitae(chars, "student");
    assert(chars[0].blood[0] >= 0 && chars[0].blood[0] < 100);
    assert(chars[0].hit < 10000);
    assert(chars[1].blood[0] < 100);

    reset();
    players[0].powers[DISC_THAUMATURGY] = 5;
    chars[0].blood[0] = 49;
    do_cauldronblood(chars, "student");
    assert(chars[0].blood[0] == 49 && chars[1].hit == 10000);
    assert(chars[0].wait == 0 && chars[0].fighting == NULL);
    chars[0].blood[0] = 50;
    do_cauldronblood(chars, "student");
    assert(chars[0].blood[0] == 0 && chars[0].wait == 20);
    assert(chars[1].hit >= 8000 && chars[1].hit <= 9000);
    assert(chars[1].blood[0] >= 80 && chars[1].blood[0] <= 90);
    assert(chars[0].fighting == &chars[1] && chars[1].fighting == chars);
    reset();
    players[0].powers[DISC_THAUMATURGY] = 5;
    players[1].powers[DISC_FORTITUDE] = 7;
    do_cauldronblood(chars, "student");
    assert(chars[1].hit >= 8700 && chars[1].hit <= 9700);
    reset();
    players[0].powers[DISC_THAUMATURGY] = 5;
    chars[0].fighting = &chars[1];
    chars[2].hit = 1;
    do_cauldronblood(chars, "other");
    assert(chars[1].hit == 10000 && chars[1].blood[0] == 100);
    assert(chars[2].hit <= 0 && chars[2].position != POS_DEAD);
}

static void test_potency(void)
{
    reset();
    players[0].powers[DISC_THAUMATURGY] = 2;
    do_potency(chars, "");
    assert(chars[0].blood[0] == 100 && chars[0].affected == NULL);
    players[0].powers[DISC_THAUMATURGY] = 3;
    chars[0].blood[0] = 49;
    do_potency(chars, "");
    assert(chars[0].blood[0] == 49 && chars[0].affected == NULL);
    chars[0].blood[0] = 50;
    do_potency(chars, "");
    assert(chars[0].blood[0] == 0 && chars[0].blood[1] == 400);
    assert(get_blood_potency(chars) == 4);
    assert(chars[0].affected->duration >= 15 && chars[0].affected->duration <= 30);
    chars[0].blood[0] = 400;
    do_potency(chars, "");
    assert(chars[0].blood[0] == 400 && chars[0].blood[1] == 400);
    update_gen(chars);
    update_gen(chars);
    assert(chars[0].blood[1] == 400); /* No reset or stacking on generation updates. */
    chars[0].vampgen = 1;
    update_gen(chars);
    assert(chars[0].blood[1] == 1100 && get_blood_potency(chars) == 11);
    affect_strip(chars, gsn_potency);
    assert(chars[0].blood[1] == 1000 && get_blood_potency(chars) == 10);
    chars[0].vampgen = 9;
    update_gen(chars);
    chars[0].blood[0] = 300;
    chars[0].wait = 0;
    do_potency(chars, "");
    chars[0].blood[0] = 400;
    chars[0].affected->duration = 0;
    /* Exercise actual world-tick expiration, not just affect_strip. */
    char_list = chars;
    chars[0].next = NULL;
    char_update();
    assert(chars[0].affected == NULL && chars[0].blood[1] == 300);
    assert(chars[0].blood[0] <= 300 && get_blood_potency(chars) == 3);
    reset();
    players[0].powers[DISC_THAUMATURGY] = 3;
    chars[0].blood[1] = LONG_MAX - 50;
    do_potency(chars, "");
    assert(chars[0].blood[0] == 100 && chars[0].affected == NULL);
    gsn_potency = 0;
    do_potency(chars, "");
    assert(chars[0].blood[0] == 100 && chars[0].affected == NULL);
    gsn_potency = skill_lookup("blood potency");
}

static void test_buff_save_roundtrip(void)
{
    FILE *fp;
    CHAR_DATA loaded;
    PC_DATA pc;
    reset();
    players[0].powers[DISC_THAUMATURGY] = 3;
    do_potency(chars, "");
    chars[0].blood[0] = 399;
    assert(save_char_obj_checked(chars));
    fp = fopen("../player/Teacher", "r");
    assert(fp != NULL && !strcmp(fread_word(fp), "#PLAYER"));
    clear_char(&loaded);
    memset(&pc, 0, sizeof(pc));
    loaded.pcdata = &pc;
    fread_char(&loaded, fp);
    fclose(fp);
    assert(loaded.blood[0] == 399 && loaded.blood[1] == 400);
    assert(is_affected(&loaded, gsn_potency));
    assert(get_blood_potency(&loaded) == 4);
    assert(pc.teach_until == 0 && pc.teach_from[0] == '\0');
    update_gen(&loaded);
    assert(loaded.blood[1] == 400);
    affect_strip(&loaded, gsn_potency);
    assert(loaded.blood[1] == 300 && loaded.blood[0] == 300);
    assert(get_blood_potency(&loaded) == 3);
}

static void test_recovery_cannot_be_bypassed(void)
{
    long blood, hp, victim_blood, third_blood;
    char forced[] = "theftvitae other";
    reset();
    players[0].powers[DISC_THAUMATURGY] = 5;
    do_cauldronblood(chars, "student");
    blood = chars[0].blood[0];
    hp = chars[1].hit;
    victim_blood = chars[1].blood[0];
    third_blood = chars[2].blood[0];
    assert(chars[0].wait == 20);
    /* An indirect interpreter call doesn't pass the descriptor input gate
     * where ordinary lag is enforced. The ability must enforce it too. */
    interpret(chars, forced);
    do_cauldronblood(chars, "student");
    do_tasteblood(chars, "student");
    do_bloodrage(chars, "teacher regenerate");
    do_potency(chars, "");
    assert(chars[0].blood[0] == blood && chars[1].hit == hp);
    assert(chars[1].blood[0] == victim_blood && chars[2].blood[0] == third_blood);
    assert(chars[0].wait == 20 && chars[0].affected == NULL);
    assert(players[0].regenerate == 0);
    for (int i = 19; i > 0; --i)
    {
        chars[0].wait = i;
        do_theftvitae(chars, "other");
        assert(chars[0].blood[0] == blood && chars[2].blood[0] == third_blood);
        assert(chars[0].wait == i); /* Rejection does not refresh recovery. */
    }
    chars[0].wait = 0;
    do_theftvitae(chars, "other");
    assert(chars[2].blood[0] < third_blood && chars[0].wait == 12);
}

static void test_blood_depletion_is_not_execution(void)
{
    reset();
    players[0].powers[DISC_THAUMATURGY] = 5;
    chars[1].class = CLASS_MONK;
    chars[1].hit = chars[1].max_hit = 1000000;
    chars[1].blood[0] = 1;
    do_theftvitae(chars, "student");
    assert(chars[1].blood[0] == 0 && chars[0].blood[0] == 101);
    assert(chars[1].hit == 1000000 && chars[1].position == POS_FIGHTING);
    chars[0].wait = 0;
    do_cauldronblood(chars, "student");
    assert(chars[0].blood[0] == 101 && chars[0].wait == 0);
    assert(chars[1].hit == 1000000);

    reset();
    players[0].powers[DISC_THAUMATURGY] = 5;
    chars[1].class = CLASS_MONK;
    chars[1].hit = chars[1].max_hit = 1000000;
    chars[1].blood[0] = 1;
    do_cauldronblood(chars, "student");
    assert(chars[1].blood[0] == 0 && chars[0].blood[0] == 50);
    assert(chars[1].hit == 990000); /* Exactly 1%, not a full-HP execution. */
    assert(chars[1].position == POS_FIGHTING && chars[0].wait == 20);

    reset();
    players[0].powers[DISC_THAUMATURGY] = 4;
    chars[3].blood[0] = 1;
    chars[3].hit = chars[3].max_hit = 1000000;
    do_theftvitae(chars, "mobile");
    assert(chars[3].blood[0] == 0 && chars[3].hit == 1000000);
    assert(chars[3].fighting == chars && chars[0].blood[0] == 101);
}

static void test_quietus_leech(void)
{
    reset();
    players[0].powers[DISC_QUIETUS] = 5;
    do_leechblood(chars, "student");
    assert_unchanged();
    players[0].powers[DISC_QUIETUS] = 6;
    chars[0].fighting = &chars[1];
    chars[2].class = CLASS_MONK;
    chars[2].blood[0] = 1;
    chars[2].hit = chars[2].max_hit = 1000000;
    do_leechblood(chars, "other");
    assert(chars[2].blood[0] == 0 && chars[2].hit == 1000000);
    assert(chars[0].blood[0] == 101 && chars[1].blood[0] == 100);
    assert(chars[0].wait == 12);
    do_leechblood(chars, "student");
    assert(chars[1].blood[0] == 100 && chars[0].wait == 12);
    reset();
    players[0].powers[DISC_QUIETUS] = 6;
    do_leechblood(chars, "mobile");
    assert_unchanged();
    chars[0].blood[0] = chars[0].blood[1];
    do_leechblood(chars, "student");
    assert(chars[1].blood[0] == 100 && chars[0].wait == 0);
    chars[0].blood[0] = 5;
    chars[0].hit = chars[0].max_hit = 20000;
    chars[1].blood[0] = 2;
    players[1].powers[DISC_VICISSITUDE] = 6;
    do_leechblood(chars, "student");
    assert(chars[0].blood[0] == 3 && chars[1].blood[0] == 0);
    assert(chars[0].hit == 10000 && chars[0].wait == 12);
    assert(chars[1].hit == 10000);
}

static void test_blood_economy(void)
{
    long generations[] = {1, 2, 4, 6, 9, 12, 13, 14, 0, -1, LONG_MAX};
    for (size_t i = 0; i < sizeof(generations) / sizeof(generations[0]); ++i)
    {
        long total;
        reset();
        players[0].powers[DISC_THAUMATURGY] = 4;
        chars[1].vampgen = generations[i];
        update_gen(&chars[1]);
        chars[1].blood[0] = chars[1].blood[1];
        total = chars[0].blood[0] + chars[1].blood[0];
        for (int cast = 0; cast < 12; ++cast)
        {
            chars[0].wait = 0;
            do_theftvitae(chars, "student");
            assert(chars[0].blood[0] + chars[1].blood[0] == total);
            assert(chars[0].blood[0] >= 0 && chars[0].blood[0] <= chars[0].blood[1]);
            assert(chars[1].blood[0] >= 0 && chars[1].blood[0] <= chars[1].blood[1]);
            assert(chars[1].hit == 10000);
        }
    }
    reset();
    players[0].powers[DISC_THAUMATURGY] = 5;
    chars[0].blood[0] = chars[0].blood[1]; /* Gen 9: six 50-blood casts. */
    chars[1].vampgen = 1;
    update_gen(&chars[1]);
    chars[1].blood[0] = 1000;
    for (int cast = 1; cast <= 6; ++cast)
    {
        chars[0].wait = 0;
        chars[1].hit = chars[1].max_hit; /* Isolate affordability, not a duel. */
        do_cauldronblood(chars, "student");
        assert(chars[0].blood[0] == 300 - cast * 50 && chars[0].wait == 20);
    }
    chars[0].wait = 0;
    {
        long before = chars[1].blood[0], hp = chars[1].hit;
        do_cauldronblood(chars, "student");
        assert(chars[0].blood[0] == 0 && chars[0].wait == 0);
        assert(chars[1].blood[0] == before && chars[1].hit == hp);
    }
}

static void test_random_drain_bounds(void)
{
    /* UMIN is a macro that may evaluate its selected argument twice.
     * Exercise limits inside each random range, not just below its minimum. */
    for (int quietus = 0; quietus <= 1; ++quietus)
        for (int trial = 0; trial < 512; ++trial)
        {
            long limit = quietus ? 15 : 7;
            reset();
            players[0].powers[DISC_THAUMATURGY] = 4;
            players[0].powers[DISC_QUIETUS] = 6;
            chars[0].blood[0] = chars[0].blood[1] - limit;
            chars[1].class = CLASS_MONK;
            chars[1].blood[0] = limit;
            if (quietus) do_leechblood(chars, "student");
            else do_theftvitae(chars, "student");
            assert(chars[0].blood[0] <= chars[0].blood[1]);
            assert(chars[1].blood[0] >= 0);
            assert(chars[0].blood[0] + chars[1].blood[0] == 300);
            assert(chars[1].hit == 10000);
        }
    for (int trial = 0; trial < 512; ++trial)
    {
        reset();
        players[0].powers[DISC_THAUMATURGY] = 5;
        chars[1].class = CLASS_MONK;
        chars[1].blood[0] = 15;
        do_cauldronblood(chars, "student");
        assert(chars[1].blood[0] >= 0 && chars[1].blood[0] <= 5);
        assert(chars[1].hit >= 8500 && chars[1].hit <= 9000);
        assert(chars[0].blood[0] == 50 && chars[0].wait == 20);
    }
}

static void test_cauldron_class_and_hp_envelopes(void)
{
    const long classes[] = {CLASS_VAMPIRE, CLASS_WEREWOLF, CLASS_SORCERER,
        CLASS_DROW, CLASS_HIGHLANDER, CLASS_MAGE, CLASS_DRAGON, CLASS_NINJA,
        CLASS_MONK, CLASS_ELEM};
    const long hp[] = {10000, 50000, 100000, 1000000, LONG_MAX};
    for (size_t c = 0; c < sizeof(classes)/sizeof(classes[0]); ++c)
        for (size_t h = 0; h < sizeof(hp)/sizeof(hp[0]); ++h)
            for (int rank = 0; rank <= (classes[c] == CLASS_VAMPIRE ? 7 : 0); ++rank)
            {
                long boiled, raw, taken, low_reduction, high_reduction;
                reset();
                players[0].powers[DISC_THAUMATURGY] = 5;
                chars[1].class = classes[c];
                chars[1].hit = chars[1].max_hit = hp[h];
                players[1].powers[DISC_FORTITUDE] = rank;
                do_cauldronblood(chars, "student");
                boiled = 100 - chars[1].blood[0];
                assert(boiled >= 10 && boiled <= 20);
                raw = hp[h] / 100 * boiled + hp[h] % 100 * boiled / 100;
                low_reduction = raw / 100 * (5 * rank)
                    + raw % 100 * (5 * rank) / 100;
                high_reduction = raw / 100 * (10 * rank)
                    + raw % 100 * (10 * rank) / 100;
                taken = hp[h] - chars[1].hit;
                assert(taken >= raw - high_reduction && taken <= raw - low_reduction);
                assert(chars[1].hit > 0 && chars[1].hit < hp[h]);
                assert(chars[0].blood[0] == 50 && chars[0].wait == 20);
            }
}

static OBJ_DATA *object_form_fixture(bool mummy)
{
    static OBJ_INDEX_DATA index;
    OBJ_DATA *body;
    reset();
    memset(&index, 0, sizeof(index));
    index.vnum = mummy ? OBJ_VNUM_CORPSE_PC : OBJ_VNUM_SPILT_BLOOD;
    index.name = str_dup("synthetic body");
    index.short_descr = str_dup("a synthetic body");
    index.description = str_dup("A synthetic body lies here.");
    index.item_type = ITEM_TRASH;
    body = create_object(&index, 0);
    obj_to_room(body, &room);
    body->chobj = chars;
    players[0].chobj = body;
    players[0].obj_vnum = index.vnum;
    players[0].powers[DISC_VICISSITUDE] = 4;
    players[0].powers[DISC_SERPENTIS] = 3;
    chars[0].more |= mummy ? MORE_MUMMIFIED : MORE_PLASMA;
    chars[0].extra |= EXTRA_OSWITCH;
    chars[0].affected_by |= AFF_POLYMORPH;
    chars[0].morph = str_dup("a synthetic body");
    room.vnum = ROOM_VNUM_IN_OBJECT;
    return body;
}

static void test_object_form_returns(void)
{
    static ROOM_INDEX_DATA temple, hell;
    for (int mummy = 0; mummy <= 1; ++mummy)
    {
        DO_FUN *fn = mummy ? do_mummify : do_plasmaform;
        long flag = mummy ? MORE_MUMMIFIED : MORE_PLASMA;
        OBJ_DATA *body = object_form_fixture(mummy);
        memset(&temple, 0, sizeof(temple));
        memset(&hell, 0, sizeof(hell));
        temple.area = hell.area = &area;
        temple.vnum = ROOM_VNUM_TEMPLE;
        hell.vnum = ROOM_VNUM_HELL;
        room_index_hash[temple.vnum % MAX_KEY_HASH] = NULL;
        room_index_hash[hell.vnum % MAX_KEY_HASH] = NULL;
        /* No exit: refuse before destroying the body or changing flags. */
        fn(chars, "");
        assert(players[0].chobj == body && body->chobj == chars);
        assert(chars[0].in_room == &room && IS_MORE(chars, flag));
        assert(IS_EXTRA(chars, EXTRA_OSWITCH) && IS_AFFECTED(chars, AFF_POLYMORPH));
        /* Wrong form must not destroy the body through the other command. */
        (mummy ? do_plasmaform : do_mummify)(chars, "");
        assert(players[0].chobj == body && IS_MORE(chars, flag));
        room_index_hash[temple.vnum % MAX_KEY_HASH] = &temple;
        body->chobj = &chars[1];
        fn(chars, "");
        assert(players[0].chobj == body && chars[0].in_room == &room);
        body->chobj = chars;
        fn(chars, "");
        assert(players[0].chobj == NULL && chars[0].in_room == &temple);
        assert(temple.people == chars && room.contents == NULL && object_list == NULL);
        assert(!IS_MORE(chars, flag) && !IS_EXTRA(chars, EXTRA_OSWITCH));
        assert(!IS_AFFECTED(chars, AFF_POLYMORPH));
        free_string(chars[0].morph);
        chars[0].morph = str_dup("");
        /* Real engine extraction can leave the form active but bodyless. */
        body = object_form_fixture(mummy);
        extract_obj(body);
        assert(players[0].chobj == NULL && IS_MORE(chars, flag));
        fn(chars, "");
        assert(chars[0].in_room == &temple && !IS_MORE(chars, flag));
        free_string(chars[0].morph);
        chars[0].morph = str_dup("");
        /* Keep the original destination when an operator supplies it. */
        object_form_fixture(mummy);
        room_index_hash[hell.vnum % MAX_KEY_HASH] = &hell;
        fn(chars, "");
        assert(chars[0].in_room == &hell && players[0].chobj == NULL);
        free_string(chars[0].morph);
        chars[0].morph = str_dup("");
        room_index_hash[temple.vnum % MAX_KEY_HASH] = NULL;
        room_index_hash[hell.vnum % MAX_KEY_HASH] = NULL;
    }
}

int main(void)
{
    char directory[] = "/tmp/anathema-vampire-XXXXXX";
    char cwd[4096];
    assert(getcwd(cwd, sizeof(cwd)) != NULL && mkdtemp(directory) != NULL);
    assert(chdir(directory) == 0 && mkdir("player", 0700) == 0);
    assert(mkdir("area", 0700) == 0 && chdir("area") == 0);
    string_space = malloc(1024 * 1024); /* Tiny synthetic files, not a world load. */
    assert(string_space != NULL);
    top_string = string_space;
    current_time = 123456; /* Reproducible, initialized engine RNG, not all-zero state. */
    init_mm();
    test_random_drain_bounds();
    test_cauldron_class_and_hp_envelopes();
    test_recovery_cannot_be_bypassed();
    test_blood_depletion_is_not_execution();
    test_quietus_leech();
    test_blood_economy();
    test_teaching();
    test_save_failure();
    test_rejections(do_theftvitae, 4, DISC_THAUMATURGY);
    test_rejections(do_cauldronblood, 5, DISC_THAUMATURGY);
    test_rejections(do_leechblood, 6, DISC_QUIETUS);
    test_blood_magic();
    test_potency();
    test_buff_save_roundtrip();
    test_object_form_returns();
    reset();
    unlink("../player/Teacher");
    unlink("../player/Student");
    assert(chdir("..") == 0);
    assert(rmdir("player") == 0 && rmdir("area") == 0);
    assert(chdir(cwd) == 0 && rmdir(directory) == 0);
    if (fpReserve != NULL) fclose(fpReserve);
    puts("vampire teaching and Thaumaturgy tests passed");
    return 0;
}
