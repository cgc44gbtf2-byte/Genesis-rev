#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"

/* Counter is original Anathema code. Its reply modifies the next ordinary
 * melee strike: it never recursively invokes damage/one_hit or keeps a target
 * pointer across pulses. These timers are transient, not player-save fields. */
static bool counter_katana(CHAR_DATA *ch, OBJ_DATA *weapon)
{
    return weapon != NULL && weapon->item_type == ITEM_WEAPON
        && weapon->value[3] == TYPE_SLASH
        && IS_SET(weapon->spectype, SITEM_HIGHLANDER)
        && weapon->questowner != NULL
        && !str_cmp(weapon->questowner, ch->name);
}

static bool counter_ready(CHAR_DATA *ch)
{
    return ch != NULL && !IS_NPC(ch) && IS_HIGHLANDER(ch)
        && get_highland(ch, HIGH_DEFENSE) >= 4
        && get_highland(ch, HIGH_ABILITY) >= 4;
}

static bool counter_equipped(CHAR_DATA *ch)
{
    return counter_katana(ch, get_eq_char(ch, WEAR_WEAPON))
        || counter_katana(ch, get_eq_char(ch, WEAR_HOLD));
}

void do_counter(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    one_argument(argument, arg);
    if (!counter_ready(ch))
    {
        stc("Counter requires Highlander Defense 4 and Powers 4.\n\r", ch);
        return;
    }
    if (!str_cmp(arg, "off"))
    {
        ch->highland_guard = ch->highland_riposte = 0;
        stc("You lower your counter guard. Recovery time is unchanged.\n\r", ch);
        return;
    }
    if (arg[0] != '\0')
    {
        stc("Syntax: counter [off]\n\r", ch);
        return;
    }
    if (ch->highland_counter_reuse > 0)
    {
        stc("Your counter technique is still recovering.\n\r", ch);
        return;
    }
    if (ch->position < POS_FIGHTING || IS_COMBAT_PAUSED(ch)
        || ch->fighting == NULL || ch->fighting == ch
        || !combat_target_live(ch, ch->fighting)
        || is_safe(ch, ch->fighting))
    {
        stc("You need a legal, active opponent to prepare a counter.\n\r", ch);
        return;
    }
    if (!counter_equipped(ch))
    {
        stc("You must wield your own forged katana to counter.\n\r", ch);
        return;
    }
    if (ch->mana < 1000)
    {
        stc("Counter costs 1000 mana.\n\r", ch);
        return;
    }
    if (!hurt_fighting(ch, ch->fighting)) return;
    ch->mana -= 1000;
    ch->highland_guard = 2 * PULSE_PER_SECOND;
    ch->highland_riposte = 0;
    ch->highland_counter_reuse = 6 * PULSE_PER_SECOND;
    WAIT_STATE(ch, PULSE_PER_SECOND);
    stc("You ready your katana to catch and counter a melee strike.\n\r", ch);
    act("$n readies a katana counter guard.", ch, NULL, NULL, TO_ROOM);
}

bool highlander_counter_strike(CHAR_DATA *ch, CHAR_DATA *victim,
    OBJ_DATA *weapon, long dt)
{
    if (!counter_ready(ch) || ch->highland_riposte <= 0)
        return FALSE;
    if (!counter_equipped(ch))
    {
        ch->highland_guard = ch->highland_riposte = 0;
        return FALSE;
    }
    if (dt < TYPE_SLASH || dt > TYPE_BLUNT || !counter_katana(ch, weapon)
        || IS_COMBAT_PAUSED(ch) || ch->fighting != victim
        || !combat_target_live(ch, victim) || !can_see(ch, victim)
        || is_safe(ch, victim) || !hurt_fighting(ch, victim))
        return FALSE;
    ch->highland_riposte = 0;
    act("You answer $N with a precise katana riposte.", ch, NULL, victim, TO_CHAR);
    act("$n answers you with a precise katana riposte.", ch, NULL, victim, TO_VICT);
    return TRUE;
}

bool highlander_counter_parry(CHAR_DATA *ch, CHAR_DATA *victim,
    OBJ_DATA *weapon, long dt)
{
    (void)weapon; /* The defender needs a katana, not the incoming attacker. */
    if (!counter_ready(victim) || victim->highland_guard <= 0)
        return FALSE;
    if (!counter_equipped(victim))
    {
        victim->highland_guard = victim->highland_riposte = 0;
        return FALSE;
    }
    if (dt < TYPE_HIT || dt > TYPE_BLUNT || ch == victim
        || victim->position < POS_FIGHTING || IS_COMBAT_PAUSED(victim)
        || !combat_target_live(ch, victim) || !can_see(victim, ch)
        || is_safe(ch, victim) || !hurt_fighting(ch, victim))
        return FALSE;
    victim->highland_guard = 0; /* One eligible swing, even when the roll fails. */
    if (number_percent() > UMIN(6, get_highland(victim, HIGH_DEFENSE)) * 5)
    {
        stc("Your katana counter guard fails to catch the strike.\n\r", victim);
        return FALSE;
    }
    victim->highland_riposte = 2 * PULSE_PER_SECOND;
    act("You catch $n's melee strike with your katana and prepare a riposte.",
        ch, NULL, victim, TO_VICT);
    act("$N catches your melee strike with a katana counter guard.",
        ch, NULL, victim, TO_CHAR);
    return TRUE;
}

/* Legacy TYPE_HIT also labels spells and projectiles. Register contact
 * attacks explicitly instead of treating every physical damage type as melee.
 * The original one_hit label must be checked before "1"/"2" expand to an
 * arbitrary weapon description. New melee entry points must use this route
 * or call the parry preflight before applying their contact effects. */
bool highlander_melee_attack(long dt, const char *message)
{
    static const char *const contacts[] = {
        "attack", "kick", "fangs", "horns", "wing buffet", "wave fist",
        "holy sword", "tsume", "forked tongue", "tendril of shadow",
        "shadow arm", "chimerstral dagger", "spider fangs", "circle",
        "backstab", "repeating fist", "Fist of Caine", "spiked tail",
        "tongue", "bone crafting", "mirage strike"
    };
    if (message == NULL || dt < TYPE_HIT || dt > TYPE_BLUNT) return FALSE;
    if (message[0] == '1' || message[0] == '2') return TRUE;
    for (size_t i = 0; i < sizeof(contacts) / sizeof(contacts[0]); ++i)
        if (!str_cmp(message, contacts[i])) return TRUE;
    return FALSE;
}

void do_increase( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];
    char buf[MIL];
    long cost, power;

    argument = one_argument(argument, arg);

    if (!IS_HIGHLANDER(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (arg[0] == '\0')
    {
snprintf(buf, sizeof(buf), "Increases: Attack (%li), Defense (%li), Weaponry (%li), Powers (%li).\n\r",
	 get_highland(ch, HIGH_ATTACK), get_highland(ch, HIGH_DEFENSE), get_highland(ch, HIGH_WEAPON), get_highland(ch, HIGH_ABILITY));
	stc(buf, ch);
	stc("Syntax: increase <attack/defense/weaponry/powers>",ch);
	return;
    }

    if (!str_cmp(arg, "attack")) power = HIGH_ATTACK;
    else if (!str_cmp(arg, "defense")) power = HIGH_DEFENSE;
    else if (!str_cmp(arg, "weaponry")) power = HIGH_WEAPON;
    else if (!str_cmp(arg, "powers")) power = HIGH_ABILITY;
    else
    {
	do_increase(ch, "");
	return;
    }

    if (get_highland(ch, power) >= 6)
    {
snprintf(buf, sizeof(buf), "You have already achieved maximum proficiency in highlander %s.\n\r", arg);
	stc(buf, ch);
	return;
    }

    cost = (get_highland(ch, power) + 1) * 15;
    if (ch->primal < cost)
    {
snprintf(buf, sizeof(buf), "It costs %li primal to improve your skill with highlander %s.\n\r", cost, arg);
	stc(buf, ch);
	return;
    }

    ch->primal -= cost;
    ch->pcdata->highland[power]++;
    snprintf(buf, sizeof(buf), "You have improved your skill with highlander %s.\n\r", arg);
    stc(buf, ch);
    return;
}

void do_bladeflash(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_STRING_LENGTH];
    long sn;
    AFFECT_DATA af;
    CHAR_DATA *victim;

    argument = one_argument(argument, arg);

    if (!IS_HIGHLANDER(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }
    if (get_highland(ch, HIGH_ABILITY) < 2)
    {
	stc("You have not learned the Powers ability to level 2 yet.\n\r", ch);
	return;
    }
    if (arg[0] == '\0')
    {
	stc("Who do you wish to blade flash.\n\r",ch);
	return;
    }
    if ((victim = get_char_room(ch, arg)) == NULL)
    {
	stc("They are not here.\n\r",ch);
	return;
    }
    /* Blindness is hostile even though it does no immediate HP damage.
     * Apply the same combat-entry/peace/link-dead checks as damaging powers
     * before removing Truesight or changing the target's affects. */
    if (victim == ch || victim->position <= POS_STUNNED)
    {
stc("You cannot blade flash them.\n\r", ch);
return;
    }
    if ((sn = skill_lookup("blind")) < 0) return;
    if (is_safe(ch,victim) || !hurt_fighting(ch,victim)) return;

    if (!IS_NPC(victim))
    {
	if (IS_SET(victim->act, PLR_HOLYLIGHT)) REMOVE_BIT(victim->act, PLR_HOLYLIGHT);
	else
	{
/*	    stc("Your blade flash has no affect.\n\r",ch);
	    WAIT_STATE(ch,24);
	    return; */
	}
    }

    if (!IS_AFFECTED(victim, AFF_BLIND))
    {
    	af.type      = sn;
    	af.location  = APPLY_HITROLL;
    	af.modifier  = -50;
    	af.duration  = 20;
    	af.bitvector = AFF_BLIND;
    	affect_to_char( victim, &af );
    }

    act("You catches a bit of light and shines it in $N's eyes.",ch,NULL,victim,TO_CHAR);
    act("$n catches a bit of light and shines it in $N's eyes.",ch,NULL,victim,TO_NOTVICT);
    act("$n catches a bit of light and shines it in your eyes.",ch,NULL,victim,TO_VICT);
    WAIT_STATE(ch,24);
    return;
}

void do_bash(CHAR_DATA *ch, char *argument) 
{
    char arg[MAX_STRING_LENGTH];
    char dam[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    long damage = (850 + number_range(1,100));
    argument = one_argument(argument, arg);

    if (!IS_HIGHLANDER(ch))
    {
	stc("Huh?\n\r",ch);
	return;
    }
    if (get_highland(ch, HIGH_ABILITY) < 3)
    {
	stc("You have not learned the Powers ability to level 3 yet.\n\r", ch);
	return;
    }
    if ((victim = arg[0] == '\0' ? ch->fighting : NULL) == NULL)
    {
	if( arg[0] == '\0' )
	{
	    stc("Bash whom?\r\n", ch);
	    return;
	}
	if( (victim = get_char_room(ch, arg)) == NULL )
	{
	    stc("How can you bash someone who is not here?\r\n", ch);
	    return;
	}
    }

    if (victim == ch || is_safe(ch, victim) || !hurt_fighting(ch, victim)) return;

    if (number_range(1,4) == 1 || (IS_WEREWOLF(victim) && get_breed(victim,BREED_LUPUS) >= 3 && number_range(1,3) == 1))
    {
	act("You rush forward, missing $N completely!",ch,NULL,victim,TO_CHAR);
	act("$n rushes forward, missing $N completely!",ch,NULL,victim,TO_NOTVICT);
	act("$n rushes forward, missing you completely!",ch,NULL,victim,TO_VICT);
    }
    else
    {
        WAIT_STATE(ch, 20);
        if (highlander_counter_parry(ch, victim, NULL, TYPE_BLUNT)) return;
snprintf(dam, sizeof(dam), "%li", damage);
	act("You rush forward, slamming into $N! [#g$t#n]", ch, dam, victim, TO_CHAR);
	act("$n rushes forward, slamming into $N!", ch, dam, victim, TO_NOTVICT);
	act("$n rushes forward, slamming into you! [#r$t#n]", ch, dam, victim, TO_VICT);
	victim->wait += 10;
        WAIT_STATE(ch, 20);
	hurt_person(ch, victim, damage);
        return;
    }
    WAIT_STATE(ch, 20);
    return;
}

void do_htwist( CHAR_DATA *ch, char *argument)
{
    if (!IS_HIGHLANDER(ch))
    {
	stc("Huh?\n\r",ch);
	return;
    }
    if (get_highland(ch, HIGH_ABILITY) < 4)
    {
	stc("You have not learned the Powers ability to level 4 yet.\n\r",ch);
	return;
    }
    if (ch->pcdata->celerity > 1)
    {
	stc("You are already using increased speed!\n\r",ch);
	return;
    }
    send_to_char("You start moving with supernatural speed!\n\r",ch);
    act("$n starts moving with supernatural speed!",ch,NULL,NULL,TO_ROOM);
    ch->pcdata->celerity += 5 * get_highland(ch, HIGH_ABILITY);
    WAIT_STATE(ch,4);
    return;
}

void do_judgement(CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg [MAX_INPUT_LENGTH];
    char buf [MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_HIGHLANDER(ch))
    {
	stc("Huh?\n\r",ch);
	return;
    }
    if (get_highland(ch, HIGH_ABILITY) < 5)
    {
	stc("You have not learned the Powers ability to level 5 yet.\n\r",ch);
	return;
    }
    if (arg[0] == '\0')
    {
	stc("Judge whom?\n\r",ch);
	return;
    }
    if ((victim = get_char_room(ch,arg)) == NULL)
    {
	stc("They aren't here.\n\r",ch);
	return;
    }

    act("You examine $N intently.",ch,NULL,victim,TO_CHAR);
    act("$n examines $N intently.",ch,NULL,victim,TO_NOTVICT);
    act("$n examines you intently.",ch,NULL,victim,TO_VICT);

    if (IS_NPC(victim)) snprintf(buf, sizeof(buf), "%s is an NPC.\n\r",victim->short_descr);
    else if (IS_ST(victim)) snprintf(buf, sizeof(buf), "%s is a Story Teller.\n\r", victim->name);
    else if (IS_IMP(victim)) snprintf(buf, sizeof(buf), "%s is an Implementor.\n\r", victim->name);
    else if (IS_IMM(victim)) snprintf(buf, sizeof(buf), "%s is an Immortal.\n\r", victim->name);
    else if (IS_BUI(victim)) snprintf(buf, sizeof(buf), "%s is a Builder.\n\r", victim->name);
    else if (IS_AWA(victim)) snprintf(buf, sizeof(buf), "%s is Awakened.\n\r", victim->name);
    else snprintf(buf, sizeof(buf), "%s is a Sleeper.\n\r", victim->name);
    stc(buf,ch);

    snprintf(buf, sizeof(buf), "Health: %li/%li\n\r", (victim->hit/2000) * 2000, (victim->max_hit/2000) * 2000);
    stc(buf,ch);
    snprintf(buf, sizeof(buf), "Magic:  %li/%li\n\r", (victim->mana/2000) * 2000, (victim->max_mana/2000) * 2000);
    stc(buf,ch);
    snprintf(buf, sizeof(buf), "Armor:  %li.\n\r",char_ac(victim));
    stc(buf,ch);
    return;
}

void do_lunge( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument(argument, arg);

    if (IS_NPC(ch)) return;

    if (!IS_HIGHLANDER(ch))
    {
	stc("Huh?\n\r",ch );
	return;
    }
    if (get_highland(ch, HIGH_ABILITY) < 6)
    {
	stc("You have not learned this attack.\n\r",ch);
	return;
    }
    if ((victim = get_char_room(ch,arg)) == NULL)
    {
	stc("They aren't here.\n\r",ch);
	return;
    }
    if (victim == ch)
    {
	stc("You can't lunge yourself.\n\r", ch);
	return;
    } 
    if (is_safe(ch,victim)) return;

    act("You lunge at $N, catching $M off guard!", ch, NULL, victim, TO_CHAR);
    act("$n lunges at you, catching you off guard!", ch, NULL, victim, TO_VICT);
    act("$n lunges at $N, catching $M off guard!", ch, NULL, victim, TO_NOTVICT);

    if (victim->position != POS_FIGHTING) set_fighting(victim,ch);
    if (ch->position != POS_FIGHTING) set_fighting(ch,victim);
    do_stance(victim,"");
    WAIT_STATE(ch, 8);
    return;   
}

void do_forge( CHAR_DATA *ch, char *argument )
{
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    OBJ_DATA *invobj;
    char arg[MAX_STRING_LENGTH];
    long vnum = 29650, value;
    const long transferable = QUEST_HITROLL | QUEST_DAMROLL | QUEST_HIT
        | QUEST_MANA | QUEST_AC | QUEST_ENCHANTED | QUEST_IMPROVED
        | QUEST_AGGRAVATED;

    one_argument(argument,arg);

    if (IS_NPC(ch)) return;

    if (!IS_HIGHLANDER(ch))
    {
	stc("Huh?\n\r",ch);
	return;
    }

    if (ch->primal < 50 || ch->pcdata->quest < 100)
    {
        stc("It costs 50 points of primal and 100 quest points to create a katana.\n\r",ch);
        return;
    }

    if ((pObjIndex = get_obj_index(vnum)) == NULL)
    {
	stc("Missing object, inform Akuma.\n\r",ch);
	return;
    }

    if (arg[0] == '\0')
    {
	obj = create_object(pObjIndex, 50);
	obj_to_char(obj, ch);
	ch->primal -= 50;
	ch->pcdata->quest -= 100;
	SET_BIT(obj->quest, QUEST_AGGRAVATED);
	free_string( obj->questmaker );
	obj->questmaker = str_dup( ch->name );
	free_string( obj->questowner );
	obj->questowner = str_dup( ch->name );
	act("$p appears in your hands.",ch,obj,NULL,TO_CHAR);
	act("$p appears in $n's hands.",ch,obj,NULL,TO_ROOM);
    }
    else
    {
	if ((invobj = get_obj_carry(ch,arg)) == NULL)
	{
	    stc("You don't have that object.\n\r",ch);
	    return;
	}
	if (invobj->item_type != ITEM_WEAPON)
	{
	    stc("That isn't a weapon!\n\r",ch);
	    return;
	}
	if (IS_SET(invobj->quest, QUEST_ARTIFACT) || IS_SET(invobj->quest, QUEST_RELIC))
	{
	    stc("You can't!.\n\r",ch);
	    return;
	}
	if (invobj->questowner == NULL)
	{
	    stc("Claim it first.\n\r",ch);
	    return;
	}
	if (str_cmp(ch->name,invobj->questowner))
	{
	    stc("It's not yours!\n\r",ch);
	    return;
	}
/* Reject non-transferable state before charging or destroying the source.
 * A class-bound, temporary or custom-action weapon cannot safely become a
 * permanent Highlander relic. Ordinary quest/enchantment affects can move
 * exactly once along with their already-spent quest-point accounting. */
if (invobj->chobj != NULL || invobj->contains != NULL
    || invobj->timer != 0 || invobj->temp_timer != 0
    || invobj->spectype != 0 || invobj->specpower != 0
    || invobj->extra_descr != NULL
    || IS_SET(invobj->extra_flags, ITEM_VANISH)
    || (invobj->extra_flags & ~pObjIndex->extra_flags) != 0
    || (invobj->quest & ~transferable) != 0
    || invobj->points < 0
    || (invobj->actcommands != NULL && invobj->actcommands[0] != '\0')
    || (invobj->chpoweron != NULL && str_cmp(invobj->chpoweron, "(null)")
        && invobj->chpoweron[0] != '\0')
    || (invobj->chpoweroff != NULL && str_cmp(invobj->chpoweroff, "(null)")
        && invobj->chpoweroff[0] != '\0')
    || (invobj->chpoweruse != NULL && str_cmp(invobj->chpoweruse, "(null)")
        && invobj->chpoweruse[0] != '\0')
    || (invobj->victpoweron != NULL && str_cmp(invobj->victpoweron, "(null)")
        && invobj->victpoweron[0] != '\0')
    || (invobj->victpoweroff != NULL && str_cmp(invobj->victpoweroff, "(null)")
        && invobj->victpoweroff[0] != '\0')
    || (invobj->victpoweruse != NULL && str_cmp(invobj->victpoweruse, "(null)")
        && invobj->victpoweruse[0] != '\0'))
{
    stc("This weapon has properties the forge cannot preserve. It was not consumed.\n\r", ch);
    return;
}
	obj = create_object(pObjIndex, 50);
	free_string( obj->questmaker );
obj->questmaker = str_dup(invobj->questmaker != NULL
    && invobj->questmaker[0] != '\0' ? invobj->questmaker : ch->name);
	free_string( obj->questowner );
	obj->questowner = str_dup( ch->name );
obj->quest |= (invobj->quest & transferable) | QUEST_AGGRAVATED;
obj->points = invobj->points;
for (value = 0;value < 4;value++)
	    obj->value[value] = invobj->value[value];
obj->affected = invobj->affected;
invobj->affected = NULL; /* Source extraction must not free transferred affects. */
obj_to_char(obj, ch);
ch->primal -= 50;
ch->pcdata->quest -= 100;
	act("You reforge $p.",ch,invobj,NULL,TO_CHAR);
	act("$n reforges $p.",ch,invobj,NULL,TO_ROOM);
	extract_obj(invobj);
    }
    return;
}
