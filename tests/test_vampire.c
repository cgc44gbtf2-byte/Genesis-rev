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

static void test_rejections(DO_FUN *power, int rank)
{
    reset();
    power(NULL, "");
    power(&chars[3], "student");
    assert_unchanged();
    players[0].powers[DISC_THAUMATURGY] = rank - 1;
    power(chars, "student");
    assert_unchanged();
    players[0].powers[DISC_THAUMATURGY] = rank;
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
    test_teaching();
    test_save_failure();
    test_rejections(do_theftvitae, 4);
    test_rejections(do_cauldronblood, 5);
    test_blood_magic();
    test_potency();
    test_buff_save_roundtrip();
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
