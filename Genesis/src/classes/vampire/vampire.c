/*
 * Teaching and Path of Blood adapted from Ethereal-Mists EM/src/clan.c
 * (archive commit 885604dbca035103ea8bea96e9e40993e2eb00a4).
 * The Diku/Merc/God Wars copyright and permission notices in clan.c apply.
 *
 * Genesis discipline numbers, generations, training costs and elder powers
 * are deliberately retained. No EM player-data layout is imported.
 */
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include "merc.h"

static const struct {
    const char *name;
    long id;
} disciplines[] = {
    {"animalism", DISC_ANIMALISM}, {"auspex", DISC_AUSPEX},
    {"celerity", DISC_CELERITY}, {"chimerstry", DISC_CHIMERSTRY},
    {"dominate", DISC_DOMINATE}, {"fortitude", DISC_FORTITUDE},
    {"necromancy", DISC_NECROMANCY}, {"obfuscate", DISC_OBFUSCATE},
    {"obtenebration", DISC_OBTENEBRATION}, {"potence", DISC_POTENCE},
    {"presence", DISC_PRESENCE}, {"protean", DISC_PROTEAN},
    {"quietus", DISC_QUIETUS}, {"serpentis", DISC_SERPENTIS},
    {"thaumaturgy", DISC_THAUMATURGY}, {"vicissitude", DISC_VICISSITUDE}
};

static long discipline_id(const char *name)
{
    size_t i;
    for (i = 0; i < sizeof(disciplines) / sizeof(disciplines[0]); ++i)
        if (!str_cmp(name, disciplines[i].name)) return disciplines[i].id;
    return -1;
}

static bool same_plane(CHAR_DATA *ch, CHAR_DATA *victim)
{
    long planes = AFF_ETHEREAL | AFF_DEEPUMBRA | AFF_SHADOWPLANE;
    return ch->in_room == victim->in_room
        && (ch->affected_by & planes) == (victim->affected_by & planes);
}

static bool vampire_ready(CHAR_DATA *ch)
{
    return ch != NULL && !IS_NPC(ch) && ch->pcdata != NULL
        && IS_VAMPIRE(ch) && ch->in_room != NULL && ch->hit > 0
        && ch->level >= LEV_AWA && ch->position >= POS_FIGHTING
        && !IS_MORE(ch, MORE_NPC) && !IS_MORE(ch, MORE_HEAD)
        && !IS_EXTRA(ch, EXTRA_OSWITCH) && !IS_COMBAT_PAUSED(ch);
}

static bool teaching_ready(CHAR_DATA *ch)
{
    return vampire_ready(ch) && ch->position == POS_STANDING
        && ch->fighting == NULL && ch->desc != NULL
        && ch->desc->connected == CON_PLAYING && ch->desc->original == NULL;
}

void vampire_clear_offer(CHAR_DATA *ch)
{
    if (ch == NULL || ch->pcdata == NULL) return;
    ch->pcdata->teach_from[0] = '\0';
    ch->pcdata->teach_disc = -1;
    ch->pcdata->teach_until = 0;
}

void do_teachdisc(CHAR_DATA *ch, char *argument)
{
    char first[MAX_INPUT_LENGTH], second[MAX_INPUT_LENGTH];
    char third[MAX_INPUT_LENGTH], extra[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *teacher, *student;
    long disc;
    bool accepting;
    size_t i;

    if (!teaching_ready(ch))
    {
        if (ch != NULL) stc("Teaching requires two connected, standing vampires outside combat.\n\r", ch);
        return;
    }
    argument = one_argument(argument, first);
    argument = one_argument(argument, second);
    argument = one_argument(argument, third);
    one_argument(argument, extra);
    if (!str_cmp(first, "decline") && second[0] == '\0')
    {
        vampire_clear_offer(ch);
        stc("You decline any pending discipline lesson.\n\r", ch);
        return;
    }
    if (first[0] == '\0')
    {
        stc("teachdisc <student> <discipline> -- offer a free lesson (rank 3 required).\n\r"
            "teachdisc accept <teacher> <discipline> -- accept within 120 seconds.\n\r"
            "teachdisc decline -- discard your pending lesson.\n\r"
            "Teachable disciplines:\n\r", ch);
        for (i = 0; i < sizeof(disciplines) / sizeof(disciplines[0]); ++i)
            if (ch->pcdata->powers[disciplines[i].id] >= 3)
            {
                snprintf(buf, sizeof(buf), "  %s\n\r", disciplines[i].name);
                stc(buf, ch);
            }
        return;
    }
    accepting = !str_cmp(first, "accept");
    if (second[0] == '\0' || (accepting ? third[0] == '\0'
                                      : third[0] != '\0') || extra[0] != '\0')
    {
        stc("Syntax: teachdisc <student> <discipline>, or teachdisc accept <teacher> <discipline>.\n\r", ch);
        return;
    }
    disc = discipline_id(accepting ? third : second);
    if (disc < 0)
    {
        stc("That is not a supported Genesis discipline.\n\r", ch);
        return;
    }
    teacher = accepting ? get_char_room(ch, second) : ch;
    student = accepting ? ch : get_char_room(ch, first);
    if (teacher == student || !teaching_ready(teacher) || !teaching_ready(student)
        || !same_plane(teacher, student) || !can_see(teacher, student)
        || !can_see(student, teacher))
    {
        stc("Both vampires must be here, visible, standing and outside combat.\n\r", ch);
        return;
    }
    if (teacher->pcdata->powers[disc] < 3)
    {
        stc("The teacher must have trained that discipline to rank 3.\n\r", ch);
        return;
    }
    if (student->pcdata->powers[disc] != 0)
    {
        stc("The student has already unlocked that discipline.\n\r", ch);
        return;
    }
    if (!accepting)
    {
        if (strlen(teacher->name) >= sizeof(student->pcdata->teach_from))
        {
            stc("This teacher's name cannot be represented safely in an offer.\n\r", ch);
            return;
        }
        /* Fixed names, not character pointers: logout/extraction cannot leave
         * dangling offers. Acceptance always resolves and revalidates a live
         * teacher. Offers are never written into player files. */
        snprintf(student->pcdata->teach_from, sizeof(student->pcdata->teach_from),
                 "%s", teacher->name);
        student->pcdata->teach_disc = disc;
        student->pcdata->teach_until = current_time + 120;
        /* Use the parsed discipline, not a mutable/shared disc_name buffer. */
        snprintf(buf, sizeof(buf),
                 "%s offers a free %s lesson. To agree: teachdisc accept %s %s\n\r",
                 teacher->name, second, teacher->name, second);
        stc(buf, student);
        stc("Lesson offered; nothing changes until the student accepts.\n\r", teacher);
        return;
    }
    if (student->pcdata->teach_until <= current_time
        || str_cmp(student->pcdata->teach_from, teacher->name)
        || student->pcdata->teach_disc != disc)
    {
        stc("There is no matching, unexpired lesson offer.\n\r", student);
        return;
    }
    student->pcdata->powers[disc] = -1;
    if (!save_char_obj_checked(student))
    {
        /* Free, single-player, idempotent unlock. A post-rename fsync failure
         * may already have written -1; retrying never charges either player. */
        student->pcdata->powers[disc] = 0;
        stc("The lesson save was not confirmed. No success is claimed; retry accepting.\n\r", student);
        stc("The student's save was not confirmed; the lesson can be retried.\n\r", teacher);
        return;
    }
    vampire_clear_offer(student);
    act("You teach $N a new discipline. Training still costs primal.", teacher, NULL, student, TO_CHAR);
    act("$n teaches you a new discipline. Use 'discipline' to train it.", teacher, NULL, student, TO_VICT);
}

/* Match Genesis's existing generation-to-pool progression. This is a derived
 * stat, not EM's incompatible third blood[] value or generation table. */
long get_blood_potency(CHAR_DATA *ch)
{
    long potency;
    if (ch == NULL || IS_NPC(ch) || ch->pcdata == NULL || !IS_VAMPIRE(ch)) return 1;
    switch (ch->vampgen)
    {
    case 1: potency = 10; break;
    case 2: case 3: potency = 8; break;
    case 4: case 5: potency = 5; break;
    case 6: case 7: case 8: potency = 4; break;
    case 9: case 10: case 11: potency = 3; break;
    case 12: case 13: potency = 2; break;
    default: potency = 1; break;
    }
    if (gsn_potency > 0 && is_affected(ch, gsn_potency)) ++potency;
    return potency;
}

static bool blood_power_ready(CHAR_DATA *ch, long discipline, long rank)
{
    /* Descriptor input normally enforces lag, but Dominate, force, aliases
     * and nested interpreter calls can bypass that outer gate. Never let
     * these blood powers be recast during an existing action's recovery. */
    if (!vampire_ready(ch) || ch->wait > 0)
    {
        if (ch != NULL) stc("You cannot use blood magic in your current state.\n\r", ch);
        return FALSE;
    }
    if (get_disc(ch, discipline) < rank)
    {
        stc("Your discipline is not trained high enough for that blood power.\n\r", ch);
        return FALSE;
    }
    return TRUE;
}

static bool thaumaturgy_ready(CHAR_DATA *ch, long rank)
{
    return blood_power_ready(ch, DISC_THAUMATURGY, rank);
}

static CHAR_DATA *blood_target(CHAR_DATA *ch, char *argument, bool hostile)
{
    char arg[MAX_INPUT_LENGTH], extra[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    argument = one_argument(argument, arg);
    one_argument(argument, extra);
    if (extra[0] != '\0')
    {
        stc("Specify just one target.\n\r", ch);
        return NULL;
    }
    /* A named target always wins over the current opponent. */
    victim = arg[0] == '\0' && hostile ? ch->fighting : get_char_room(ch, arg);
    if (victim == NULL || victim->in_room == NULL || victim->position == POS_DEAD
        || !same_plane(ch, victim) || !can_see(ch, victim)
        || (!IS_NPC(victim) && victim->pcdata == NULL))
    {
        stc("That target is not here and visible on your plane.\n\r", ch);
        return NULL;
    }
    if (!hostile) return victim;
    if (victim == ch || victim->hit <= 0
        || (IS_NPC(victim) && victim->level > 700)
        || (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim)
        || is_safe(ch, victim))
    {
        stc("You cannot use that blood attack on this target.\n\r", ch);
        return NULL;
    }
    return victim;
}

void do_tasteblood(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];
    size_t i;
    if (!thaumaturgy_ready(ch, 1)) return;
    victim = blood_target(ch, argument, FALSE);
    if (victim == NULL) return;
    snprintf(buf, sizeof(buf), "Blood: %li/%li. Blood potency: %li.\n\r",
             victim->blood[BLOOD_CURRENT], victim->blood[BLOOD_POOL],
             get_blood_potency(victim));
    stc(buf, ch);
    if (!IS_NPC(victim) && IS_VAMPIRE(victim))
    {
        snprintf(buf, sizeof(buf), "Clan: %s. Generation: %li.\n\r",
                 victim->clan == NULL || victim->clan[0] == '\0' ? "Caitiff" : victim->clan,
                 victim->vampgen);
        stc(buf, ch);
        for (i = 0; i < sizeof(disciplines) / sizeof(disciplines[0]); ++i)
            if (get_disc(victim, disciplines[i].id) > 0)
            {
                snprintf(buf, sizeof(buf), "%s: %li\n\r", disciplines[i].name,
                         get_disc(victim, disciplines[i].id));
                stc(buf, ch);
            }
    }
    act("$n studies your blood.", ch, NULL, victim, TO_VICT);
    WAIT_STATE(ch, 12);
}

void do_bloodrage(CHAR_DATA *ch, char *argument)
{
    char target[MAX_INPUT_LENGTH], action[MAX_INPUT_LENGTH], extra[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    if (!thaumaturgy_ready(ch, 2)) return;
    argument = one_argument(argument, target);
    argument = one_argument(argument, action);
    one_argument(argument, extra);
    if (target[0] == '\0' || str_cmp(action, "regenerate") || extra[0] != '\0')
    {
        stc("Syntax: bloodrage <vampire> regenerate. Stat boosting is not implemented.\n\r", ch);
        return;
    }
    victim = blood_target(ch, target, FALSE);
    if (victim == NULL) return;
    if (!vampire_ready(victim) || victim->pcdata->regenerate != 0
        || (victim != ch && !IS_EXTRA(victim, EXTRA_VAMPIRE)))
    {
        stc("They must be an able vampire, not already regenerating, with defences lowered to consent.\n\r", ch);
        return;
    }
    /* EM toggled regenerate, accidentally cancelling an existing effect.
     * Only enable it; normal Genesis regeneration pays its own blood costs. */
    victim->pcdata->regenerate = 10;
    act("You awaken $N's regenerative blood.", ch, NULL, victim, TO_CHAR);
    act("$n awakens your regenerative blood.", ch, NULL, victim, TO_VICT);
    WAIT_STATE(ch, 12);
}

void do_potency(CHAR_DATA *ch, char *argument)
{
    AFFECT_DATA af;
    long cost, gen;
    (void)argument;
    if (!thaumaturgy_ready(ch, 3)) return;
    if (gsn_potency <= 0 || gsn_potency >= MAX_SKILL
        || str_cmp(skill_table[gsn_potency].name, "blood potency"))
    {
        stc("Blood Potency is not configured; no blood was spent.\n\r", ch);
        return;
    }
    if (is_affected(ch, gsn_potency))
    {
        stc("Your blood is already concentrated.\n\r", ch);
        return;
    }
    gen = URANGE(1, ch->vampgen, 13);
    cost = (14 - gen) * 10;
    if (ch->blood[BLOOD_CURRENT] < cost || ch->blood[BLOOD_POOL] < 0
        || ch->blood[BLOOD_POOL] > LONG_MAX - 100)
    {
        stc("You cannot afford or safely expand your blood pool.\n\r", ch);
        return;
    }
    memset(&af, 0, sizeof(af));
    af.type = gsn_potency;
    af.duration = UMIN(5, get_disc(ch, DISC_THAUMATURGY)) * number_range(5, 10);
    af.location = APPLY_BLOOD_MAX;
    af.modifier = 100;
    ch->blood[BLOOD_CURRENT] -= cost;
    affect_to_char(ch, &af);
    stc("You concentrate your blood: +100 capacity and +1 blood potency until it fades.\n\r", ch);
    WAIT_STATE(ch, 12);
}

static void drain_blood(CHAR_DATA *ch, char *argument, bool quietus)
{
    CHAR_DATA *victim;
    long amount, capacity;
    bool acid;
    if (!blood_power_ready(ch, quietus ? DISC_QUIETUS : DISC_THAUMATURGY,
                           quietus ? 6 : 4)) return;
    victim = blood_target(ch, argument, TRUE);
    if (victim == NULL) return;
    if (quietus && IS_NPC(victim))
    {
        stc("Quietus Leech cannot target NPCs.\n\r", ch);
        return;
    }
    if (ch->blood[BLOOD_CURRENT] < 0
        || ch->blood[BLOOD_POOL] <= ch->blood[BLOOD_CURRENT]
        || victim->blood[BLOOD_CURRENT] <= 0)
    {
        stc("There is no blood to take or no room in your blood pool.\n\r", ch);
        return;
    }
    capacity = ch->blood[BLOOD_POOL] - ch->blood[BLOOD_CURRENT];
    /* UMIN can evaluate an argument twice. Roll once before clamping so a
     * second roll cannot exceed the capacity or available-blood bound. */
    amount = quietus ? number_range(10, 20)
                     : number_range(5, 10) * get_blood_potency(victim);
    amount = UMIN(amount, UMIN(capacity, victim->blood[BLOOD_CURRENT]));
    if (!hurt_fighting(ch, victim)) return;
    acid = (!quietus && IS_WEREWOLF(victim) && get_tribe(victim, TRIBE_GET_OF_FENRIS) >= 3)
        || (IS_VAMPIRE(victim) && get_disc(victim, DISC_VICISSITUDE) >= 6);
    REMOVE_BIT(ch->vampaff, VAM_OBFUSCATE);
    victim->blood[BLOOD_CURRENT] -= amount;
    WAIT_STATE(ch, 12);
    if (!IS_NPC(victim) && IS_MORE(victim, MORE_GUARDIAN))
        guardian_message(victim);
    if (acid)
    {
        ch->blood[BLOOD_CURRENT] -= UMIN(amount, ch->blood[BLOOD_CURRENT]);
        stc("The stolen blood burns inside you!\n\r", ch);
        /* Final operation: damage/death handling may change character state. */
        hurt_person(victim, ch, quietus ? 10000 : number_range(150, 300) * amount);
        return;
    }
    ch->blood[BLOOD_CURRENT] += amount;
    act("A stream of blood flows from $N into you.", ch, NULL, victim, TO_CHAR);
    act("A stream of blood flows from you into $n.", ch, NULL, victim, TO_VICT);
    /* This is resource theft, not damage. In particular, the final blood
     * point must not turn a free cast into an arbitrary-HP execution. */
}

void do_theftvitae(CHAR_DATA *ch, char *argument)
{
    drain_blood(ch, argument, FALSE);
}

void do_leechblood(CHAR_DATA *ch, char *argument)
{
    drain_blood(ch, argument, TRUE);
}

void do_cauldronblood(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    long amount, dam, fortitude, reduction;
    if (!thaumaturgy_ready(ch, 5)) return;
    victim = blood_target(ch, argument, TRUE);
    if (victim == NULL) return;
    if (ch->blood[BLOOD_CURRENT] < 50 || victim->blood[BLOOD_CURRENT] <= 0
        || victim->max_hit <= 0)
    {
        stc("You need 50 blood and a victim with blood to boil.\n\r", ch);
        return;
    }
    amount = number_range(10, 20);
    amount = UMIN(amount, victim->blood[BLOOD_CURRENT]);
    /* Integer percentage without overflowing max_hit * amount. */
    dam = (victim->max_hit / 100) * amount
        + ((victim->max_hit % 100) * amount) / 100;
    fortitude = IS_NPC(victim) ? 0 : URANGE(0, get_disc(victim, DISC_FORTITUDE), 7);
    reduction = number_range(5 * fortitude, 10 * fortitude);
    dam -= (dam / 100) * reduction + ((dam % 100) * reduction) / 100;
    if (!hurt_fighting(ch, victim)) return;
    ch->blood[BLOOD_CURRENT] -= 50;
    victim->blood[BLOOD_CURRENT] -= amount;
    REMOVE_BIT(ch->vampaff, VAM_OBFUSCATE);
    act("You boil $N's blood!", ch, NULL, victim, TO_CHAR);
    act("Your blood begins to boil!", ch, NULL, victim, TO_VICT);
    WAIT_STATE(ch, 20);
    /* Use the established combat/death helpers, never raw hit subtraction or
     * blood-depletion executions for any class. No access afterward. */
    hurt_person(ch, victim, dam);
}
