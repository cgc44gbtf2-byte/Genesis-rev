/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
 *  God Wars Mud improvements copyright (C) 1994, 1995, 1996 by Richard    *
 *  Woolcock.  This mud is NOT to be copied in whole or in part, or to be  *
 *  run without the permission of Richard Woolcock.  Nobody else has       *
 *  permission to authorise the use of this code.                          *
 ***************************************************************************/

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

/*
 * Local functions.
 */
void	dam_message	args( ( CHAR_DATA *ch, CHAR_DATA *victim, long dam, long dt, char *message ) );
void	death_cry	args( ( CHAR_DATA *ch ) );

/*
 * Control the fights going on.
 * Called periodically by update_handler.
 */
void violence_update( void )
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *victim;
    CHAR_DATA *rch;
    CHAR_DATA *rch_next;

    for (ch = first_fight; ch != NULL; ch = ch_next)
    {
	ch_next	= ch->next_fight;

        if (IS_COMBAT_PAUSED(ch)) continue;

	if ((victim = ch->fighting) == NULL || victim->in_room != ch->in_room)
	{
	    stop_fighting(ch, FALSE);
	    continue;
	}

	if (IS_AWAKE(ch) && IS_AWAKE(victim) && ch->in_room == victim->in_room)
	{
	    if (--ch->init <= 0)
	    {
		multi_hit(ch, victim);
		ch->init = get_speed(ch);
	    }
	    else continue;
	}
	else stop_fighting(ch, FALSE);

	if ((victim = ch->fighting) == NULL)
	{
	    stop_fighting(ch, FALSE);
	    continue;
	}
	if (!IS_NPC(ch) && !IS_NPC(victim)) ch->fight_timer = 30;

	/*
	 * Fun for the whole family!
	 */
	for (rch = ch->in_room->people; rch != NULL; rch = rch_next)
	{
	    rch_next = rch->next_in_room;

	    if (IS_AWAKE(rch) && rch->fighting == NULL)
	    {
		/* Auto-assist others in group. */
		if (is_same_group(ch, rch) && !is_same_group(rch, victim))
		    multi_hit(rch, victim);
		else if (IS_NPC(ch) && IS_NPC(rch))
		{
		    /* NPC's assist NPC's of same type or 1/16 chance regardless. */
		    if (rch->pIndexData == ch->pIndexData || number_bits(4) == 0)
		    {
			CHAR_DATA *vch;
			CHAR_DATA *target;
			long number;

			target = NULL;
			number = 0;
			for (vch = ch->in_room->people; vch; vch = vch->next)
			{
			    if (can_see(rch, vch) && is_same_group(vch, victim) && number_range(0, number) == 0)
			    {
				target = vch;
				number++;
			    }
			}

			if (target != NULL) multi_hit(rch, target);
		    }
		}
	    }
	}
    }

    return;
}



/*
 * Do one group of attacks.
 */
void multi_hit( CHAR_DATA *ch, CHAR_DATA *victim )
{
    OBJ_DATA *wield;
    long sn, dt, style = ch->style[0];

    if (ch == NULL || victim == NULL || IS_COMBAT_PAUSED(ch)) return;
    if (ch->position < POS_SLEEPING || is_safe(ch, victim)) return;

    if (IS_NPC(ch)) /* Temporary I swear */
    {
	if ((wield = get_eq_char(ch, WEAR_WEAPON)) && wield->item_type == ITEM_WEAPON) dt = wield->value[3];
	else dt = TYPE_UNARMED;
	one_hit(ch, victim, dt, "attack");
	if (number_range(251, 500) <= ch->level) one_hit(ch, victim, dt, "attack");
	if (number_range(501, 1000) <= ch->level) one_hit(ch, victim, dt, "attack");
	if (number_range(1001, 2500) <= ch->level) one_hit(ch, victim, dt, "attack");
	if (number_range(2501, 5000) <= ch->level) one_hit(ch, victim, dt, "attack");
	return;
    }

    if ((wield = get_eq_char(ch, WEAR_WEAPON)) != NULL)
    {
	if (style == STYLE_UNARMED || style == STYLE_NATURAL)
	{
	    stc("Erm...'Oh Shit(weapon)' bug alert.  Inform Akuma.\n\r", ch);
	    return;
	}
	dt = wield->value[3];
    }
    else
    {
	if (style == STYLE_UNARMED || style == STYLE_SHIELD) dt = TYPE_UNARMED;
	else if (style == STYLE_NATURAL) dt = TYPE_SLASH;
	else
	{
	    stc("Erm...'Oh Shit(unarmed)' bug alert.  Inform Akuma.\n\r", ch);
	    return;
	}
    }

    improve_stance(ch);
    improve_style(ch);
    improve_weapon(ch, dt);

    one_hit(ch, victim, dt, "1");
    switch(style)
    {
	default:
	    stc("Erm...'Oh Shit(style)' bug alert.  Inform Akuma.\n\r", ch);
	    return;
	case STYLE_UNARMED:
	    if (number_range(101, 200) <= get_style(ch, style)) one_hit(ch, victim, dt, "1");
	    break;
	case STYLE_NATURAL:
	    break;
	case STYLE_ONEHAND:
	    if (number_range(1, 100) <= get_style(ch, style)) one_hit(ch, victim, dt, "1");
	    if (number_range(101, 200) <= get_style(ch, style)) one_hit(ch, victim, dt, "1");
	    break;
	case STYLE_SHIELD:
	    break;
	case STYLE_TWOHAND:
	    break;
	case STYLE_TWOWEAPON:
	    if (number_range(51, 150) <= get_style(ch, style)) one_hit(ch, victim, dt, "1");
	    if (number_range(151, 250) <= get_style(ch, style)) one_hit(ch, victim, dt, "1");

	    if ((wield = get_eq_char(ch, WEAR_HOLD)) == NULL || wield->item_type != ITEM_WEAPON)
	    {
		stc("Erm...'Oh Shit(two weapon)' bug alert.  Inform Akuma.\n\r", ch);
		return;
	    }
	    dt = wield->value[3];
	    improve_weapon(ch, dt);

	    if (number_range(1, 100) <= get_style(ch, style)) one_hit(ch, victim, dt, "2");
	    if (number_range(101, 200) <= get_style(ch, style)) one_hit(ch, victim, dt, "2");
	    break;
    }

    if (victim == NULL || victim->position != POS_FIGHTING) return;
    if (wield != NULL && wield->value[0] >= 1)
    {
	sn = wield->value[0] % 1000;
	if (sn != 0 && victim->position == POS_FIGHTING) (*skill_table[sn].spell_fun) (sn, wield->level, ch, victim);
    }
    else if (style == STYLE_NATURAL && IS_WEREWOLF(ch) && get_tribe(ch, TRIBE_BLACK_FURIES) >= 3)
	spell_poison(gsn_poison, 25, ch, victim);

    if (!IS_NPC(ch) && IS_VAMPAFF(ch, VAM_FANGS) && number_range(1, 3) <= 2) one_hit(ch, victim, TYPE_PIERCE, "fangs");
    if (IS_DRAGON(ch))
    {
	if (IS_DRAGAFF(ch, DRAG_HORNSOUT) && number_range(1, 3) == 1) one_hit(ch, victim, TYPE_PIERCE, "horns");
	if (IS_DRAGAFF(ch, DRAG_WINGSOUT) && number_range(1, 3) == 1) one_hit(ch, victim, TYPE_BLUNT, "wing buffet");
    }
    if (IS_MONK(ch) && get_monk(ch, MONK_ART) >= 8 && number_range(1, 3) == 1) one_hit(ch, victim, TYPE_HIT, "wave fist");
    if (IS_MAGE(ch) && IS_XTRA(ch, XTRA_HOLY_SMOTE) && number_range(1, 2) == 1) one_hit(ch, victim, TYPE_HIT, "holy sword");
    if (IS_WEREWOLF(ch) && get_tribe(ch, TRIBE_WENDIGOS) >= 5 && IS_SET(world_affects, WORLD_RAIN) && IS_OUTSIDE(ch))
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
	{
	    vch_next = vch->next_in_room;
	    if (is_safe(ch, vch)) continue;
	    if (vch->fighting != ch) continue;
	    if (number_range(1, 2) == 1) continue;
	    one_hit(ch, vch, TYPE_HIT, "lightning blast");
	}
    }

    if (IS_NINJA(ch))
    {
	if (IS_VAMPAFF(ch, VAM_CLAWS) && number_range(1, 3) == 1) one_hit(ch, victim, TYPE_SLASH, "tsume");
	if (get_ninja(ch, NINJA_NINGENNO) >= 6 && number_range(1, 3) == 1)
	{
	    one_hit(ch, victim, TYPE_PIERCE, "shuriken");
	    if (victim == NULL || victim->position != POS_FIGHTING) return;
	    spell_poison(gsn_poison, 25, ch, victim);
	}
    }
    
    if (IS_POLYAFF(ch, POLY_SERPENT))
    {
	one_hit(ch, victim, TYPE_SLASH, "forked tongue");
	one_hit(ch, victim, TYPE_SLASH, "forked tongue");
    }

    if (IS_VAMPIRE(ch) && IS_XTRA(ch, XTRA_TENEBROUS))
    {
	one_hit(ch, victim, TYPE_SLASH, "tendril of shadow");
	one_hit(ch, victim, TYPE_BLUNT, "shadow arm");
    }

    if (IS_VAMPIRE(ch) && get_disc(ch, DISC_CHIMERSTRY) >= 2)
	one_hit(ch, victim, TYPE_SLASH, "chimerstral dagger");

    if (IS_DROW(ch) && get_drow(ch, DROW_BITE) && number_range(1, 4) == 1)
	one_hit(ch, victim, TYPE_PIERCE, "spider fangs");

    return;
}



/*
 * Determines number of pulses between Attacks.
 */
long get_speed( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    long count = 26;

    count -= char_stat(ch, STAT_DEX);

    if ((victim = ch->fighting) == NULL || !IS_STANCE(victim, STANCE_TIGER))
    {
	if (IS_STANCE(ch, STANCE_VIPER)) count -= ch->stance[STANCE_VIPER] / 100;
	else if (IS_STANCE(ch, STANCE_MANTIS)) count -= ch->stance[STANCE_MANTIS] / 100;
	else if (IS_STANCE(ch, STANCE_MONKEY)) count -= ch->stance[STANCE_MONKEY] / 100;
    }

    if (!IS_NPC(ch))
    {
	switch (ch->class)
	{
	    case CLASS_VAMPIRE:
		count -= get_celerity(ch) + 1;
		if (IS_POLYAFF(ch, POLY_SERPENT)) count -= 3;
		break;
	    case CLASS_WEREWOLF:
		if (get_tribe(ch, TRIBE_SILENT_STRIDERS) >= 1) count--;
		if (get_tribe(ch, TRIBE_SILENT_STRIDERS) >= 3) count--;
		if (get_auspice(ch, AUSPICE_AHROUN) >= 2) count--;
		if (ch->pcdata->rank >= 5) count -= 5;
		else count -= ch->pcdata->rank + 1;
		break;
	   case CLASS_MAGE:
		if (ch->spheres[MTIM] >= 1) count -= 5;
		break;
	   case CLASS_MONK:
		if (get_monk(ch, MONK_ART) >= 3) count -= 4;
		if (get_monk(ch, MONK_ART) >= 8) count -= 2;
		break;
	    case CLASS_DROW:
		if (get_drow(ch, DROW_SKIP)) count -= 3;
                count -= ch->pcdata->drow[DROW_SPEED];
		break;
	    case CLASS_HIGHLANDER:
		count -= get_highland(ch, HIGH_ATTACK);
		break;
	    case CLASS_NINJA:
		count -= get_ninja(ch, NINJA_NINGENNO);
		break;
	    case CLASS_DRAGON:
		if (IS_POLYAFF(ch, POLY_DRAGONFORM)) count -= (13 - ch->vampgen);
		break;
	    case CLASS_ELEM:
		if (ch->pcdata->powers[ELEM_WIND] >= 5) count -= ch->pcdata->powers[ELEM_WIND] / 2;
		if (ch->pcdata->powers[ELEM_TIME] >= 5) count -= ch->pcdata->powers[ELEM_TIME] / 2;
		if (ch->pcdata->powers[ELEM_EARTH] >= 5) count += ch->pcdata->powers[ELEM_EARTH] - 5;
		break;
	}
	if (ch->pcdata->pain > 0) count += 3;
    }


    if (IS_XTRA(ch, XTRA_ATONING)) count += 2;
    if (IS_XTRA(ch, XTRA_LIQUIFY)) count += 4;
    if (IS_XTRA(ch, XTRA_SLOW)) count += 1;

    if (is_affected(ch, gsn_friction_curse)) count += 2;
    if (is_affected(ch, gsn_alter_weight)) count += 3;
    if (IS_ITEMAFF(ch, ITEMA_SPEED)) count--;
    if (count < 6) count = 6;

    return count;
}



/*
 * Hit one guy once.
 * Makes a single attack, check avoid, damage limited by damcap.
 */
void one_hit( CHAR_DATA *ch, CHAR_DATA *victim, long dt, char *message )
{
    OBJ_DATA *wield;
    long dam, max_dam;
    bool weap = TRUE;

    if (ch == NULL || IS_COMBAT_PAUSED(ch)) return;

    /*
     * Can't beat a dead char!
     * Guard against weird room-leavings.
     */
    if (victim == NULL || victim->position == POS_DEAD || ch->in_room != victim->in_room) return;

    /*
     * Check if it is a weapon attack.
     */
    if (IS_NPC(ch)) wield = get_eq_char(ch, WEAR_WEAPON);
    else if (message[0] == '1')
    {
	wield = get_eq_char(ch, WEAR_WEAPON);
	if (wield == NULL || wield->item_type != ITEM_WEAPON)
	{
	    wield = NULL;
	    message = str_dup("");
	}
	else message = str_dup(wield->short_descr);
    }
    else if (message[0] == '2')
    {
	wield = get_eq_char(ch, WEAR_HOLD);
	if (wield == NULL || wield->item_type != ITEM_WEAPON)
	{
	    wield = NULL;
	    message = str_dup("");
	}
	else message = str_dup(wield->short_descr);
    }
    else
    {
	wield = NULL;
	weap = FALSE;
    }

    if (message[0] == '\0')
    {
	if (IS_NPC(ch)) message = str_dup("fist");
	else if (IS_VAMPAFF(ch, VAM_CLAWS) && IS_MORE(ch, MORE_HELIOS)) message = str_dup("flaming claws");
	else if (IS_CRAFT(ch, CRAFT_BONETALONS)) message = str_dup("bone talons");
	else if (IS_VAMPAFF(ch, VAM_CLAWS)) message = str_dup("claws");
	else if (IS_MORE(ch, MORE_HELIOS)) message = str_dup("flaming fist");
	else if (IS_MONK(ch) && get_monk(ch, MONK_ART) >= 6) message = str_dup("adamantium fist");
	else  message = str_dup("fist");
    }
    if (!str_cmp(message, "backstab") || !str_cmp(message, "circle")) weap = TRUE;

    if (check_avoid(ch, victim, weap))
    {
	if (IS_NPC(ch) || !IS_SET(ch->act, PLR_BRIEF)) act("$N avoids your blow.", ch, NULL, victim, TO_CHAR);
	if (IS_NPC(victim) || !IS_SET(victim->act, PLR_BRIEF)) act("You avoid $n's blow.", ch, NULL, victim, TO_VICT);
	hurt_fighting(ch, victim);
	return;
    }

    /*
     * Calc damage.
     */
    if (IS_NPC(ch))
    {
	dam = mod_damcap(ch, victim);
	dam = number_range(dam / 2, 3 * dam / 2);
	if (wield != NULL) dam *= 1.5;
    }
    else
    {
	if (wield != NULL)
	{
	    dam = number_range(wield->value[1] * 5, wield->value[2] * 5);
	}
	else if (dt == TYPE_UNARMED)
	{
	    if (IS_MONK(ch) && get_monk(ch, MONK_ART) >= 6) dam = number_range(50, 100);
	    else if (IS_MORE(ch, MORE_HELIOS)) dam = number_range(50, 100);
	    else dam = number_range(15, 25);
	}
	else
	{
	    dam = number_range(25, 50);
	}

	dam += char_damroll(ch);

	if (IS_MAGE(ch) && ch->spheres[MENT] >= 1) dam *= 2;
	if (IS_MONK(ch))
	{
	    if (get_monk(ch, MONK_ART) >= 7) dam *= 2.5;
	    else if (get_monk(ch, MONK_ART) >= 1) dam *= 1.5;
	}
	if (IS_VAMPIRE(ch))     if (get_disc(ch, DISC_POTENCE) > 0) dam *= 1 + (get_disc(ch, DISC_POTENCE) * .4);
	if (IS_ELEM(ch))	if (ch->pcdata->powers[ELEM_FIRE] >= 5) dam *= ch->pcdata->powers[ELEM_FIRE] * .3;
	if (IS_DRAGON(ch))      if (IS_POLYAFF(ch, POLY_DRAGONFORM)) dam *= 1 + ((13 - ch->vampgen) * .2);
	if (IS_DROW(ch))	if (get_drow(ch, DROW_MIGHT)) dam *= 2.5;
	if (IS_NINJA(ch))       dam *= 1 + (get_ninja(ch, NINJA_CHIKYU) * .2);
	if (IS_HIGHLANDER(ch))	dam *= 1 + (get_highland(ch, HIGH_WEAPON) * .3);
	dam *= 1 + (.5 * char_stat(ch, STAT_STR));
    }

    dam = dambonus(ch, victim, dam);

    max_dam = mod_damcap(ch, victim);
    if (!IS_NPC(ch) && dam > max_dam) dam = max_dam;

    if (!IS_NPC(ch))
    {
	if (ch->style[0] == STYLE_NATURAL && weap) dam *= 1 + (ch->style[STYLE_NATURAL] * .0025);
	if (ch->style[0] == STYLE_TWOHAND && weap) dam *= 1 + (ch->style[STYLE_TWOHAND] * .01);
    }
    if (!IS_NPC(victim))
    {
	if (victim->style[0] == STYLE_SHIELD) dam /= 1 + (victim->style[STYLE_SHIELD] * .005);
    }

    if (!str_cmp(message, "backstab")) dam *= 2;

    if (dt >= TYPE_UNARMED && dt <= TYPE_BLUNT && IS_AFFECTED(victim, AFF_SANCTUARY)) dam *= .7;

    if (is_affected(ch, gsn_friction_curse)) //friction curse turns some of your movement into damage
 	hurt_person(ch, ch, 100);


    damage(ch, victim, dam, dt, message);
    tail_chain();
    return;
}



/*
 * Inflict damage from a hit.
 */
void damage( CHAR_DATA *ch, CHAR_DATA *victim, long dam, long dt, char *message )
{
    ROOM_INDEX_DATA *combat_room = ch->in_room;
    if (victim == NULL || victim->position == POS_DEAD) return;

    if (victim != ch)
    {
	/*
	 * Certain attacks are forbidden.
	 * Most other attacks are returned.
	 */
	if (is_safe(ch, victim)) return;

	if (victim->position > POS_STUNNED)
	{
	    if (victim->fighting == NULL) set_fighting(victim, ch);
	    victim->position = POS_FIGHTING;
	    if (ch->fighting == NULL) set_fighting(ch, victim);
	}

	/*
	 * More charm stuff.
	 */
	if ( victim->master == ch )
	    stop_follower( victim );

	/*
	 * Damage modifiers.
	 */
	if ( IS_AFFECTED(ch, AFF_HIDE) )
	{
	    if (!can_see(victim,ch)) dam *= 1.5;
	    send_to_char("You leap from your concealment.\n\r",ch);
	    REMOVE_BIT( ch->affected_by, AFF_HIDE );
	    act( "$n leaps from $s concealment.", ch, NULL, NULL, TO_ROOM );
	}
        if (!IS_NPC(ch) && IS_NINJA(ch) && IS_AFFECTED(ch, AFF_KAKUSU))
        {
            if (!can_see(victim, ch)) dam *= 1.5;
	    stc("You leap from the shadows.\n\r",ch);
	    REMOVE_BIT(ch->affected_by, AFF_KAKUSU);
	    act("$n leaps from the shadows.", ch, NULL, NULL, TO_ROOM);
        }

	if (is_affected(ch, gsn_mage_invis))
	{
	    affect_strip(ch, gsn_mage_invis);
	    act("$n slowly fades into your awareness.",ch,NULL,NULL,TO_ROOM);
	}
	if (IS_ITEMAFF(ch,ITEMA_BLISSFUL))
	{
	    REMOVE_BIT(ch->itemaffect,ITEMA_BLISSFUL);
	    act("$n slowly fades into your awareness.",ch,NULL,NULL,TO_ROOM);
	}
	if (!IS_NPC(ch) && IS_VAMPAFF(ch, VAM_OBFUSCATE))
	{
	    send_to_char("You drop out of Obfuscate!\n\r",ch);
	    if (!can_see(victim,ch)) dam *= 1.5;
	    REMOVE_BIT( ch->vampaff, VAM_OBFUSCATE );
	    act( "$n fades into existance.", ch, NULL, NULL, TO_ROOM );
	}

	if (IS_WEREWOLF(victim) && get_tribe(victim, TRIBE_STARGAZERS) >= 5 && number_range(1, 8) == 1)
	{
	    CHAR_DATA *temp;
	    stc("Your attack is redirected back at you!\n\r", ch);
	    act("You redirect $n's attack back at $m!", ch, NULL, victim, TO_VICT);
	    temp = ch;
	    ch = victim;
	    victim = temp;
	}
	if (IS_XTRA(ch, XTRA_MUTATE_EPHEMERA) && number_range(1, 6) == 1)
        {
            CHAR_DATA *temp;
            stc("Fate twists your attack back against you!\n\r", ch);
            act("Fate twists $ns attack back at $m!", ch, NULL, victim, TO_VICT);
            temp = ch;
            ch = victim;
            victim = temp;
        }

	dam_message(ch, victim, dam, dt, message);
	if (victim == NULL || victim->position == POS_DEAD) return;
    }
#ifdef COMBAT_CALLBACK_TESTING
    /* The dedicated regression build can free an NPC at this callback.
     * Production and the other full-engine suites call hurt_person directly. */
    {
        extern void combat_test_hurt_person(CHAR_DATA *, CHAR_DATA *, long);
        combat_test_hurt_person(ch, victim, dam);
    }
#else
    hurt_person(ch, victim, dam);
#endif
    if (!combat_pair_in_room(combat_room, ch, victim)) return;
    if (IS_ELEM(victim) && IS_POLYAFF(victim, POLY_PHOENIXFORM) && victim != ch)
    {
	act("Your burning form singes $n!", ch, NULL, victim, TO_VICT);
	act("$N's burning form singes you!", ch, NULL, victim, TO_CHAR);
	act("$N's burning form singes $n!", ch, NULL, victim, TO_NOTVICT);
	hurt_person(victim, ch, number_range(200, 250));
    }
    if (IS_VAMPIRE(victim) && IS_CRAFT(victim, CRAFT_BONEQUILLS) && victim != ch && number_range(1, 6) == 1)
    {
	act("Your bone quills slice into $n!", ch, NULL, victim, TO_VICT);
	act("$N's bone quills slice into you!", ch, NULL, victim, TO_CHAR);
	one_hit(victim, ch, TYPE_PIERCE, "bone quills");
    }
    return;
}

bool hurt_fighting( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if (victim->position == POS_DEAD) return FALSE;

    if (victim != ch)
    {
	if (auto_safe(ch, victim)) return FALSE;

	if (victim->position > POS_STUNNED)
	{
	    if (victim->fighting == NULL) set_fighting(victim, ch);
	    victim->position = POS_FIGHTING;
	    if (ch->fighting == NULL) set_fighting(ch, victim);

	    /*
	     * If victim is charmed, ch might attack victim's master.
	     */
	    if (IS_NPC(ch) && IS_NPC(victim) && IS_AFFECTED(victim, AFF_CHARM)
	    && victim->master != NULL && victim->master->in_room == ch->in_room && number_bits(3) == 0)
	    {
		stop_fighting(ch, FALSE);
		multi_hit(ch, victim->master);
		return TRUE;
	    }
	}
	if (victim->master == ch) stop_follower(victim);
    }
    return TRUE;
}

void hurt_person( CHAR_DATA *ch, CHAR_DATA *victim, long dam )
{
    victim->hit -= dam;

    if (IS_IMM(victim) && victim->hit < 1) victim->hit = victim->max_hit;

    update_pos(victim);

    switch(victim->position)
    {
	case POS_MORTAL:
	    if (IS_VAMPIRE(victim))
	    {
		act("$n is mortally wounded, and spraying blood everywhere.",victim, NULL, NULL, TO_ROOM);
		stc("You have been torpored.\n\r",victim);
	    }
	    else
	    {
		act("$n is mortally wounded, and spraying blood everywhere.",victim, NULL, NULL, TO_ROOM);
		stc("You have been mortally wounded.\n\r",victim);
	    }
	    break;
	case POS_INCAP:
	    act("$n is incapacitated, and bleeding badly.",victim, NULL, NULL, TO_ROOM);
	    stc("You are incapacitated, and bleeding badly.\n\r",victim);
	    break;
	case POS_STUNNED:
	    act( "$n is stunned, but will soon recover.",victim, NULL, NULL, TO_ROOM );
	    stc("You are stunned, but will soon recover.\n\r",victim);
	    break;
	case POS_DEAD:
	    act("$n is DEAD!!", victim, 0, 0, TO_ROOM);
	    stc("You have been KILLED!!\n\r",victim);
	    break;
	default:
	    if (dam > victim->max_hit * 0.25) stc("That really did HURT!\n\r", victim);
	    if (victim->hit < victim->max_hit * 0.25 && dam > 0)
	    {
		if (!IS_NPC(victim) && IS_VAMPIRE(victim) && number_percent() < victim->beast) vamp_rage(victim);
		else stc("You sure are BLEEDING!\n\r", victim);
	    }
	    break;
    }

    /*
     * Payoff for killing things.
     */
    if (victim->position == POS_DEAD)
    {
	group_gain(ch, victim);

	if (!IS_NPC(victim))
	{
	    sprintf(log_buf, "%s killed by %s at %li",victim->name,(IS_NPC(ch) ? ch->short_descr : ch->name),victim->in_room->vnum);
	    log_string(log_buf);
	    if (!CAN_PK(victim))
	    {
		sprintf(log_buf, "#G%s doesn't have a PK flag!#n", victim->name);
		log_string(log_buf);
	    }
	    if (!CAN_PK(ch))
	    {
		sprintf(log_buf, "#G%s doesn't have a PK flag!#n", ch->name);
		log_string(log_buf);
	    }
	}
        dp_gain(ch, victim);
    	if (!IS_NPC(victim) && IS_NPC(ch)) victim->mdeath++;
	raw_kill(victim, ch);

	if (IS_SET(ch->act, PLR_AUTOSEARCH)) do_look(ch, "in corpse");
	return;
    }

    /*
     * Sleep spells and extremely wounded folks.
     */
    if (!IS_AWAKE(victim)) stop_fighting(victim, TRUE);

    if (victim == ch)
	return;

    /*
     * Take care of link dead people.
     */
    if (!IS_NPC(victim) && victim->desc == NULL)
    {
	do_recall(victim, "");
	return;
    }

    if (IS_AFFECTED(victim, AFF_FEAR)) do_flee(victim, "");

    tail_chain( );
    return;
}



bool is_safe( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if (ch->in_room != victim->in_room) return TRUE;
    if (IS_NPC(victim) && IS_SET(victim->act, ACT_NOKILL))
    {
	stc("That mobile cannot be attacked.\n\r", ch);
	return TRUE;
    }

    if (IS_AFFECTED(ch, AFF_ETHEREAL) && !IS_AFFECTED(victim, AFF_ETHEREAL))
    {
	stc("You cannot while ethereal.\n\r", ch);
	return TRUE;
    }
    if (!IS_AFFECTED(ch, AFF_ETHEREAL) && IS_AFFECTED(victim, AFF_ETHEREAL))
    {
	stc("You cannot fight an ethereal person.\n\r", ch);
	return TRUE;
    }
    if (IS_POLYAFF(ch, POLY_MIST) || IS_POLYAFF(victim, POLY_MIST))
    {
	stc("People in mist form may not enter combat.\n\r", ch);
	return TRUE;
    }
    if (IS_POLYAFF(ch, POLY_SHADOW) || IS_POLYAFF(victim, POLY_SHADOW))
    {
	stc("People in smoke form may not enter combat.\n\r", ch);
	return TRUE;
    }
    if (IS_AFFECTED(ch, AFF_DEEPUMBRA) && !IS_AFFECTED(victim, AFF_DEEPUMBRA))
    {
	act("You are too insubstantial!", ch, NULL, victim, TO_CHAR );
	return TRUE;
    }
    if (!IS_AFFECTED(ch, AFF_DEEPUMBRA) && IS_AFFECTED(victim, AFF_DEEPUMBRA))
    {
	act( "$E is too insubstantial!", ch, NULL, victim, TO_CHAR );
	return TRUE;
    }
    if (IS_AFFECTED(ch, AFF_SHADOWPLANE) && !IS_AFFECTED(victim, AFF_SHADOWPLANE))
    {
	act("You are too insubstantial!", ch, NULL, victim, TO_CHAR);
	return TRUE;
    }
    if (!IS_AFFECTED(ch, AFF_SHADOWPLANE) && IS_AFFECTED(victim, AFF_SHADOWPLANE))
    {
	if (!IS_MAGE(ch) || !IS_SET(ch->xtra, XTRA_MAGE_CARESS))
	{
	    act("$E is too insubstantial!", ch, NULL, victim, TO_CHAR);
	    return TRUE;
	}
    }
    if (!IS_NPC(victim) && IS_FLAG(victim, FLAG_AFK))
    {
        stc("They are AFK!\n\r", ch);
        return TRUE;
    }
    if (!IS_NPC(ch) && IS_FLAG(ch, FLAG_AFK))
    {
	stc("Not while afk!\n\r", ch);
	return TRUE;
    }
    if (!IS_NPC(victim) && victim->desc == NULL && victim->position != POS_FIGHTING)
    {   
	stc("They are currently LINK-DEAD!\n\r", ch);
	return TRUE;
    }
    if (IS_SET(ch->in_room->room_flags, ROOM_SAFE) && ch->status < 5)
    {
        stc("Something in the air prevents you from attacking.\n\r", ch);
        return TRUE;
    }
    if (victim == NULL || victim->in_room == NULL) return TRUE;
    if (IS_SET(victim->in_room->room_flags, ROOM_SAFE) && victim->status < 5)
    {
	stc("Something in the air prevents you from attacking.\n\r", ch);
	return TRUE;
    }
    if (IS_MORE(ch, MORE_HEAD) || IS_EXTRA(ch, EXTRA_OSWITCH))
    {
	stc("Objects cannot fight!\n\r", ch);
	return TRUE;
    }
    if (IS_MORE(victim, MORE_HEAD) || IS_EXTRA(victim, EXTRA_OSWITCH))
    {
	stc("You cannot attack an object.\n\r", ch);
	return TRUE;
    }
    if (IS_VAMPAFF(ch, VAM_EARTHMELDED) || IS_VAMPAFF(victim, VAM_EARTHMELDED))
    {
	stc("Earthmelded players can't enter combat.\n\r", ch);
	return TRUE;
    }

    if (victim->fighting == ch) return FALSE;
    if (victim->fight_timer > 0) return FALSE;

    if (IS_NPC(ch) && (!IS_NPC(victim) && !CAN_PK(victim)) && IS_SET(ch->act, ACT_COMMANDED))
    {
	stc("You may not attack people without a PK flag.\n\r", ch);
	return TRUE;
    }
    if (IS_NPC(victim) && !CAN_PK(ch) && IS_SET(victim->act, ACT_COMMANDED))
    {
	stc("You need a PK flag to attack player controlled mobs.\n\r", ch);
	return TRUE;
    }
    if (IS_NPC(ch) && ch->level < 700 && IS_EXTRA(victim, EXTRA_AWE) && (ch->position != POS_FIGHTING))
    {
	stc("You like them too much to attack them.\n\r", ch);
	return TRUE;
    }
    if (IS_ITEMAFF(ch, ITEMA_PEACE))
    {
	send_to_char( "You are unable to attack them.\n\r", ch );
	return TRUE;
    }
    if (IS_ITEMAFF(victim, ITEMA_PEACE))
    {
	stc("They look FAR too imposing to attack!\n\r", ch);
	return TRUE;
    }

    if (IS_NPC(ch) || IS_NPC(victim)) return FALSE;

    if (ch->desc == NULL || victim->desc == NULL)
    {
	if (victim->position != POS_FIGHTING && victim->position > POS_STUNNED)
	{
	    stc("They are currently link dead.\n\r", ch);
	    return TRUE;
	}
    }
    if (IS_MORE(ch, MORE_LOYAL) && strlen(ch->pcdata->conding) > 1 && !str_cmp(ch->pcdata->conding, victim->name))
    {
	stc("You refuse to attack your master!\n\r", ch);
	return TRUE;
    }
    else if (strlen(ch->pcdata->love) > 1 && !str_cmp(ch->pcdata->love, victim->name))
    {
	stc("How could you attack someone you love?!\n\r", ch);
	return TRUE;
    }

    return FALSE;
}

bool auto_safe( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if (ch->in_room != victim->in_room) return TRUE;
    if (IS_NPC(victim) && IS_SET(victim->act, ACT_NOKILL)) return TRUE;
    if (IS_AFFECTED(ch, AFF_ETHEREAL) && !IS_AFFECTED(victim, AFF_ETHEREAL)) return TRUE;
    if (!IS_AFFECTED(ch, AFF_ETHEREAL) && IS_AFFECTED(victim, AFF_ETHEREAL)) return TRUE;
    if (IS_POLYAFF(ch, POLY_MIST) || IS_POLYAFF(victim, POLY_MIST)) return TRUE;
    if (IS_POLYAFF(ch, POLY_SHADOW) || IS_POLYAFF(victim, POLY_SHADOW)) return TRUE;
    if (IS_AFFECTED(ch, AFF_DEEPUMBRA) && !IS_AFFECTED(victim, AFF_DEEPUMBRA)) return TRUE;
    if (!IS_AFFECTED(ch, AFF_DEEPUMBRA) && IS_AFFECTED(victim, AFF_DEEPUMBRA)) return TRUE;
    if (IS_AFFECTED(ch, AFF_SHADOWPLANE) && !IS_AFFECTED(victim, AFF_SHADOWPLANE)) return TRUE;
    if (!IS_AFFECTED(ch, AFF_SHADOWPLANE) && IS_AFFECTED(victim, AFF_SHADOWPLANE)) return TRUE;
    if (ch->in_room == NULL || IS_SET(ch->in_room->room_flags, ROOM_SAFE)) return TRUE;
    if (IS_MORE(ch, MORE_HEAD) || IS_EXTRA(ch, EXTRA_OSWITCH)) return TRUE;
    if (IS_MORE(victim, MORE_HEAD) || IS_EXTRA(victim, EXTRA_OSWITCH)) return TRUE;
    if (IS_VAMPAFF(ch, VAM_EARTHMELDED) || IS_VAMPAFF(victim, VAM_EARTHMELDED)) return TRUE;

    if (victim->fighting == ch) return FALSE;

    if (IS_NPC(ch) && !CAN_PK(victim) && IS_SET(ch->act, ACT_COMMANDED)) return TRUE;
    if (IS_NPC(victim) && !CAN_PK(ch) && IS_SET(victim->act, ACT_COMMANDED)) return TRUE;
    if (IS_NPC(ch) && ch->level < 700 && IS_EXTRA(victim, EXTRA_AWE)) return TRUE;
    if (IS_ITEMAFF(ch, ITEMA_PEACE)) return TRUE;
    if (IS_ITEMAFF(victim, ITEMA_PEACE)) return TRUE;

    if (IS_NPC(ch) || IS_NPC(victim)) return FALSE;

    if ((ch->desc == NULL || victim->desc == NULL) && victim->position != POS_FIGHTING && victim->position > POS_STUNNED) return TRUE;

    if (IS_MORE(ch, MORE_LOYAL) && strlen(ch->pcdata->conding) > 1 && !str_cmp(ch->pcdata->conding, victim->name)) return TRUE;
    if (strlen(ch->pcdata->love) > 1 && !str_cmp(ch->pcdata->love, victim->name)) return TRUE;

    return FALSE;
}


long mod_damcap( CHAR_DATA *ch, CHAR_DATA *victim )
{
    long max_dam, soak;

    if (victim == NULL) return 0;

    max_dam = get_damcap(ch);
    soak = get_soak(victim);

    if (IS_VAMPIRE(victim) && IS_ITEMAFF(ch, ITEMA_SUNSHIELD)) max_dam += 500;
    if (IS_VAMPIRE(victim) && get_disc(victim, DISC_FORTITUDE) > 6) soak += max_dam / 4;
    if (IS_WEREWOLF(victim) && get_auspice(victim, AUSPICE_RAGABASH) < 5)
    {
	if (IS_WEREWOLF(ch) && get_auspice(ch, AUSPICE_AHROUN) >= 3 && IS_VAMPAFF(ch, VAM_CLAWS)) max_dam += 500;
	else
	{
	    if (IS_ITEMAFF(ch, ITEMA_RIGHT_SILVER)) max_dam += 250;
	    if (IS_ITEMAFF(ch, ITEMA_LEFT_SILVER)) max_dam += 250;
	}
    }

    if (IS_VAMPIRE(ch) && IS_VAMPAFF(ch, VAM_CLAWS) && get_disc(ch, DISC_PROTEAN) >= 1) soak -= 400;

    if (IS_NPC(victim)) soak /= 2;

    max_dam -= soak;
    if (max_dam < 500) max_dam = 500;
    return max_dam;
}


long get_damcap( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    long max_dam = 500 + (char_stat(ch, STAT_STR) * 100);

    if (IS_ITEMAFF(ch, ITEMA_ARTIFACT)) max_dam += 100;
    if (IS_ITEMAFF(ch, ITEMA_VORPAL)) max_dam += 500;

    if ((victim = ch->fighting) == NULL || !IS_STANCE(victim, STANCE_SWALLOW))
    {
	if (IS_STANCE(ch, STANCE_BULL)) max_dam += ch->stance[STANCE_BULL];
	else if (IS_STANCE(ch, STANCE_DRAGON)) max_dam += ch->stance[STANCE_DRAGON];
	else if (IS_STANCE(ch, STANCE_TIGER)) max_dam += ch->stance[STANCE_TIGER];
    }

    if (!IS_NPC(ch))
    {
	if (IS_FLAG(ch, FLAG_PK) && ch->status > 0) max_dam += 10 * ch->status;
	if (IS_MORE(ch, MORE_FAVOUR)) max_dam += 100;

	switch (ch->class)
	{
	    default: break;
	    case CLASS_VAMPIRE:
		max_dam += 200;
		if (ch->pcdata->wolf > 0) max_dam += ch->beast;
		if (get_disc(ch, DISC_POTENCE) > 0) max_dam += get_disc(ch,DISC_POTENCE) * 145;
		if (get_disc(ch, DISC_POTENCE) >= 7) max_dam += 50;
		break;
	    case CLASS_WEREWOLF:
		if (get_tribe(ch, TRIBE_SHADOW_LORDS) >= 1) max_dam += 100;
		if (get_tribe(ch, TRIBE_GET_OF_FENRIS) >= 1) max_dam += 100;
		if (get_tribe(ch, AUSPICE_PHILODOX) >= 3) max_dam += 100;
		if (IS_SET(ch->act, PLR_WOLFMAN)) max_dam += ch->pcdata->wolf * 1.5;
		if (ch->pcdata->rank >= 5) max_dam += 500;
		else max_dam += (ch->pcdata->rank + 1) * 100;
		break;
	    case CLASS_MAGE:
		if (ch->spheres[MENT] >= 1) max_dam += 400;
		break;
	    case CLASS_HIGHLANDER:
		max_dam += get_highland(ch, HIGH_ATTACK) * 300;
		break;
	    case CLASS_MONK:
		if (get_monk(ch, MONK_ART) >= 1) max_dam += 100;
		if (get_monk(ch, MONK_ART) >= 7) max_dam += 100;
		if (IS_POLYAFF(ch, POLY_CHIMERA)) max_dam += 200;
		break;
	    case CLASS_DROW:
		if (get_drow(ch, DROW_MIGHT)) max_dam += 600;
		if (get_drow(ch, DROW_BITE)) max_dam += 200;
		max_dam += ch->pcdata->drow[DROW_SPEED] * 100;
		break;
	    case CLASS_NINJA:
		max_dam += ch->pcdata->wolf * 4;
		max_dam += get_ninja(ch, NINJA_CHIKYU) * 50;
		break;
	    case CLASS_DRAGON:
		if (IS_POLYAFF(ch, POLY_DRAGONFORM)) max_dam += (13 - ch->vampgen) * 100;
		break;
	    case CLASS_ELEM:
		if (ch->pcdata->powers[ELEM_FIRE] >= 5) max_dam += ch->pcdata->powers[ELEM_FIRE] * 60;
		if (ch->pcdata->powers[ELEM_INFIN] >= 8) max_dam += ch->pcdata->powers[ELEM_INFIN] * 30;
		break;
	}
    }

    return max_dam;
}


long get_soak( CHAR_DATA *victim )
{
    CHAR_DATA *ch;
    long soak = char_stat(victim, STAT_CON) * 50;

    if (IS_ITEMAFF(victim, ITEMA_RESISTANCE)) soak += 500;

    if (!IS_NPC(victim))
    {
	switch (victim->class)
	{
	    default: break;
	    case CLASS_VAMPIRE:
		if (get_disc(victim, DISC_FORTITUDE) > 0) soak += get_disc(victim, DISC_FORTITUDE) * 75;
		if (IS_POLYAFF(victim, POLY_CHIRO)) soak += 150;
		if (IS_POLYAFF(victim, POLY_SERPENT)) soak += 500;
		if (IS_CRAFT(victim, CRAFT_FLESHSOAK)) soak += 100;
		break;
	    case CLASS_WEREWOLF:
		if (get_tribe(victim, TRIBE_BONE_GNAWERS) >= 2) soak += 30 * get_tribe(victim, TRIBE_BONE_GNAWERS);
		break;
	    case CLASS_HIGHLANDER:
		soak += get_highland(victim, HIGH_DEFENSE) * 50;
		break;
	    case CLASS_DROW:
		if (get_drow(victim, DROW_TOUGH)) soak += 300;
		break;
	    case CLASS_DRAGON:
		if (IS_POLYAFF(victim, POLY_DRAGONFORM)) soak += (13 - victim->vampgen) * 25;
		break;
	    case CLASS_ELEM:
		if (victim->pcdata->powers[ELEM_EARTH] >= 5) soak += victim->pcdata->powers[ELEM_EARTH] * 35;
		if (IS_ITEMAFF(victim, ITEMA_SUNSHIELD)) soak += 100;
		break;
	    case CLASS_MONK:
		if (get_monk(victim, MONK_ART) >= 5) soak += 200;
		break;
	}
	if (IS_MORE(victim, MORE_FAVOUR)) soak += 100;
	if ((ch = victim->fighting) == NULL || !IS_STANCE(ch, STANCE_MANTIS))
	{
	    if (IS_STANCE(victim, STANCE_CRAB)) soak += victim->stance[STANCE_CRAB];
	    else if (IS_STANCE(victim, STANCE_DRAGON)) soak += victim->stance[STANCE_DRAGON];
	    else if (IS_STANCE(victim, STANCE_SWALLOW)) soak += victim->stance[STANCE_SWALLOW];
	}
    }
    if (is_affected(victim, gsn_brittle_bones)) soak = soak / 2;

    if (victim->affected != NULL)
    {
	AFFECT_DATA *paf;
	for (paf = victim->affected; paf != NULL; paf = paf->next)
	{
	    if (paf->type == gsn_brittle_bones)
	    {
		paf->modifier--;
		if (paf->modifier == 0) affect_strip(victim, gsn_brittle_bones);
	    }
	}
    }

    return soak;
}


bool check_avoid( CHAR_DATA *ch, CHAR_DATA *victim, bool weap )
{
    long av, br;

    if (victim->position != POS_FIGHTING) return FALSE;

    if (ch->beginners_luck > 0)
    {
	ch->beginners_luck--;
	return FALSE;
    }

    av = number_range(1, avoidblow(victim));
    br = number_range(1, breakavoid(ch));

    if (!weap) av *= .5;
    if (!get_drow(victim, DROW_BFIGHT) && !can_see(victim, ch)) av *= .75;
    if (!get_drow(ch, DROW_BFIGHT) && !can_see(ch, victim)) br *= .75;

    if (av > br) return TRUE;
    else return FALSE;
}

long avoidblow( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    long chance = 0, style = ch->style[0];

    if (IS_NPC(ch))
    {
	if (ch->level < 500) return ch->level/10;
	else return 50 + (ch->level - 750)/100;
    }

    chance += char_ac(ch)/250;
    chance += char_stat(ch, STAT_DEX);/*10*/

    chance += ch->style[STYLE_UNARMED] * .05;

    if (style == STYLE_UNARMED) chance += ch->style[style] * .2;
    else if (style == STYLE_ONEHAND || style == STYLE_SHIELD) chance += ch->style[style] * .1;
    else if (style == STYLE_TWOHAND || style == STYLE_NATURAL) chance += ch->style[style] * .05;

    if ((victim = ch->fighting) == NULL || !IS_STANCE(victim, STANCE_MONKEY))
    {
	if (IS_STANCE(ch, STANCE_MONGOOSE)) chance += ch->stance[STANCE_MONGOOSE] * 0.05;
	else if (IS_STANCE(ch, STANCE_SWALLOW)) chance += ch->stance[STANCE_SWALLOW] * 0.05;
	else if (IS_STANCE(ch, STANCE_MANTIS)) chance += ch->stance[STANCE_MANTIS] * 0.05;
    }

    switch(ch->class)
    {
	case CLASS_VAMPIRE:/*55/60*/
	    chance += get_celerity(ch) * 5;
	    if(IS_XTRA(ch, XTRA_TENEBROUS)) chance += 10;
	    break;
	case CLASS_WEREWOLF:/*50*/
	    if (get_breed(ch,BREED_LUPUS) >= 3) chance += 3;
	    if (get_tribe(ch,TRIBE_CHILDREN_OF_GAIA) >= 2) chance += 3;
	    if (get_tribe(ch,TRIBE_STARGAZERS) >= 4) chance += 3;
	    chance += get_celerity(ch) * 3;
	    break;
	case CLASS_MONK:/*70*/
            if (get_monk(ch, MONK_ART) >= 4) chance += 15;
            if (IS_SET(ch->more, MORE_FAVOUR)) chance += 15;
	    if (IS_POLYAFF(ch, POLY_CHIMERA)) chance += 10;
            break;
	case CLASS_MAGE:/*50*/
	    if (ch->spheres[MTIM] >= 2) chance += 15;
	    break;
	case CLASS_DROW:/*50*/
	    if (get_drow(ch, DROW_SKIP)) chance += 10;
	    chance += ch->pcdata->drow[DROW_SPEED] * 2.5;
	    break;
	case CLASS_HIGHLANDER:/*65*/
	    if (ch->pcdata->celerity > 0) chance += 5;
	    chance += get_highland(ch, HIGH_WEAPON) * 5;
	    break;
	case CLASS_NINJA:/*40*/
	    chance += get_ninja(ch, NINJA_CHIKYU);
	    break;
	case CLASS_DRAGON:/*55*/
	    if (IS_POLYAFF(ch, POLY_DRAGONFORM)) chance += (13 - ch->vampgen) * 2;
	    break;
	case CLASS_ELEM:
	    if (ch->pcdata->powers[ELEM_WIND]>4) chance += (ch->pcdata->powers[ELEM_WIND]);
	    if (ch->pcdata->powers[ELEM_TIME]>4) chance += (ch->pcdata->powers[ELEM_TIME]);
	    if (ch->pcdata->powers[ELEM_MOON]>4) chance += (ch->pcdata->powers[ELEM_MOON]); 
	    if (ch->pcdata->powers[ELEM_EARTH]>5) chance -=(ch->pcdata->powers[ELEM_EARTH]);
	    if (IS_ITEMAFF(ch, ITEMA_AQUABARRIER)) chance += (ch->pcdata->powers[ELEM_WATER]);
	    break;
    }
    return chance;
}

long breakavoid( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    long chance = 0, style = ch->style[0];

    if (IS_NPC(ch))
    {
	if (ch->level < 750) return ch->level/10;
	else return 75 + (ch->level - 750)/100;
    }

    chance += char_hitroll(ch)/150;
    chance += char_stat(ch, STAT_DEX);/*10*/

    if (style == STYLE_TWOHAND) chance += ch->style[style] * .2;
    else if (style != STYLE_UNARMED) chance += ch->style[style] * .1;
    else chance += ch->style[style] * .05;

    if ((victim = ch->fighting) == NULL || !IS_STANCE(victim, STANCE_DRAGON))
    {
	if (IS_STANCE(ch, STANCE_CRANE)) chance += ch->stance[STANCE_CRANE] * 0.05;
	else if (IS_STANCE(ch, STANCE_TIGER)) chance += ch->stance[STANCE_TIGER] * 0.05;
	else if (IS_STANCE(ch, STANCE_MONKEY)) chance += ch->stance[STANCE_MONKEY] * 0.05;
    }

    if (ch->pcdata->pain > 0) chance -= 10;

    switch(ch->class)
    {
	case CLASS_VAMPIRE:/*50/65*/
	    chance += get_celerity(ch) * 5;
	    if (get_disc(ch, DISC_FORTITUDE) > 5)
	    chance += get_celerity(ch) * 2;
	    break;
	case CLASS_WEREWOLF:/*60*/
	    if (get_auspice(ch,AUSPICE_PHILODOX) >= 3) chance += 15;
	    chance += get_celerity(ch) * 3;
	    break;
	case CLASS_MONK:/*45*/
            if (get_monk(ch, MONK_ART) >= 2) chance += 10;
	    if (IS_POLYAFF(ch, POLY_CHIMERA)) chance += 5;
            break;
	case CLASS_MAGE: /*30*/
	    if (ch->spheres[MENT] >= 1) chance += 8;
	    break;
	case CLASS_DROW:/*65*/
	    if (get_drow(ch, DROW_EYES)) chance += 10;
	    chance += ch->pcdata->drow[DROW_SPEED] * 5;
	    break;
	case CLASS_HIGHLANDER:/*60*/
	    chance += get_highland(ch, HIGH_WEAPON) * 5;
	    break;
	case CLASS_NINJA:/*70*/
	    chance += get_ninja(ch, NINJA_NINGENNO) * 4;
	    break;
	case CLASS_DRAGON:/*55*/
	    if (IS_POLYAFF(ch,POLY_DRAGONFORM)) chance += (13 - ch->vampgen) * 2;
	    break;
	case CLASS_ELEM:
	    if (ch->pcdata->powers[ELEM_TIME]>2) chance += (ch->pcdata->powers[ELEM_TIME]);
	    if (ch->pcdata->powers[ELEM_WIND]>4) chance += (ch->pcdata->powers[ELEM_WIND]);
	   // if (ch->pcdata->powers[ELEM_EARTH]>4) chance -= (ch->pcdata->powers[ELEM_EARTH]);
	    if (IS_ITEMAFF(ch, ITEMA_AURA)) chance += (ch->pcdata->powers[ELEM_SUN]);
	    break;
    }
    return chance;
}

/*
 * Set position of a victim.
 */
void update_pos( CHAR_DATA *victim )
{
    if ( victim->hit > 0 )
    {
    	if ( victim->position <= POS_STUNNED )
	{
	    victim->position = POS_STANDING;
	    act( "$n flips back up to $s feet.", victim, NULL, NULL, TO_ROOM);
	    act( "You flip back up to your feet.", victim, NULL, NULL, TO_CHAR);
	}
	return;
    }

    if (victim->hit <= 0 && IS_MONK(victim) && IS_MORE(victim, MORE_CLOAK))
    {
	stc("Your Cloak of Life saves you from death!\n\r", victim);
	act("A flash of light bursts outward from $n, saving him from certain death!", victim, NULL, NULL, TO_ROOM);
	REMOVE_BIT(victim->more, MORE_CLOAK);
	victim->hit = victim->max_hit * .2;
    }

    if (victim->hit <= 0 && IS_DROW(victim) && get_drow(victim, DROW_GLOBE))
    {
	act("Darkness fades from the room.", victim, NULL, NULL, TO_ROOM);
	REMOVE_BIT(victim->pcdata->drow[DROW_POWERS], DROW_GLOBE);
    }

    if (!IS_NPC(victim) && victim->hit <= -10)
    {
	victim->hit = -10;
	if (victim->position == POS_FIGHTING) stop_fighting(victim,TRUE);
    }
    else if (IS_NPC(victim) && victim->hit <= 0)
    {
	victim->position = POS_DEAD;
	return;
    }

    if (victim->hit <= -6) victim->position = POS_MORTAL;
    else if (victim->hit <= -3) victim->position = POS_INCAP;
    else victim->position = POS_STUNNED;

    return;
}



/*
 * Make a corpse out of a character.
 */
void make_corpse( CHAR_DATA *ch, CHAR_DATA *maker )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *corpse;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    char *name;

    if ( IS_NPC(ch) )
    {
	name		= ch->short_descr;
	corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_NPC), 0);
	corpse->timer	= number_range( 60, 120 );
	corpse->value[2]= ch->pIndexData->vnum;
	if ( ch->gold > 0 )
	{
	    obj_to_obj( create_money( ch->gold ), corpse );
	    ch->gold = 0;
	}
    }
    else
    {
	name		= ch->name;
	corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_PC), 0);
	corpse->timer	= number_range( 600, 1200 );
	/* Why should players keep their gold? */
	if ( ch->gold > 0 )
	{
	    obj = create_money( ch->gold );
	    if (IS_AFFECTED(ch,AFF_SHADOWPLANE) &&
	        (!IS_SET(obj->extra_flags, ITEM_SHADOWPLANE) ) )
	        SET_BIT(obj->extra_flags, ITEM_SHADOWPLANE);
	    obj_to_obj( obj, corpse );
	    ch->gold = 0;
	}
    }

    free_string(corpse->questmaker);
    if (IS_NPC(maker)) corpse->questmaker = str_dup(maker->short_descr);
    else corpse->questmaker = str_dup(maker->name);

    free_string(corpse->questowner);
    if (IS_NPC(ch)) corpse->questowner = str_dup(ch->short_descr);
    else corpse->questowner = str_dup(ch->name);

    if (ch->in_room != NULL) corpse->specpower = ch->in_room->vnum;

    sprintf( buf, corpse->short_descr, name );
    free_string( corpse->short_descr );
    corpse->short_descr = str_dup( buf );

    sprintf( buf, corpse->description, name );
    free_string( corpse->description );
    corpse->description = str_dup( buf );

    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
	obj_next = obj->next_content;

	obj_from_char( obj );
	if ( IS_SET( obj->extra_flags, ITEM_VANISH ) )
	    extract_obj( obj );
	else
	{
	    if (IS_AFFECTED(ch,AFF_SHADOWPLANE) &&
	        (!IS_SET(obj->extra_flags, ITEM_SHADOWPLANE) ) )
	        SET_BIT(obj->extra_flags, ITEM_SHADOWPLANE);
	    obj_to_obj( obj, corpse );
	}
    }

    /* If you die in the shadowplane, your corpse should stay there */
    if (IS_AFFECTED(ch, AFF_SHADOWPLANE))
	SET_BIT(corpse->extra_flags, ITEM_SHADOWPLANE);

    obj_to_room( corpse, ch->in_room );
    return;
}

void death_cry( CHAR_DATA *ch )
{
    ROOM_INDEX_DATA *was_in_room;
    char *msg;
    long door;

    if ( IS_NPC(ch) )
	msg = "You hear something's death cry.";
    else
	msg = "You hear someone's death cry.";

    was_in_room = ch->in_room;
    for ( door = 0; door <= 5; door++ )
    {
	EXIT_DATA *pexit;

	if ( ( pexit = was_in_room->exit[door] ) != NULL
	&&   pexit->to_room != NULL
	&&   pexit->to_room != was_in_room )
	{
	    ch->in_room = pexit->to_room;
	    act( msg, ch, NULL, NULL, TO_ROOM );
	}
    }
    ch->in_room = was_in_room;
    return;
}

void make_head( CHAR_DATA *ch )
{
    OBJ_DATA *obj;

    if (IS_NPC(ch)) return;

    obj = create_object(get_obj_index(OBJ_VNUM_SEVERED_HEAD), 0);
    ch->pcdata->chobj = obj;
    obj->chobj = ch;
    obj->timer = 5;

    return;
}

void raw_kill( CHAR_DATA *victim, CHAR_DATA *ch )
{
    if (!IS_NPC(victim)) death_message(ch, victim);
    stop_fighting(victim, TRUE);

    if (ch == victim) ch->mdeath++;
    else if (IS_NPC(victim) && !IS_NPC(ch))
    {
	ch->mkill++;
	if (ch->mkill >= 5 && ch->level <= LEV_SLE)
	{
	    char buf[MSL];
	    sprintf(buf,"%s has awakened!",ch->name);
	    do_info(ch, buf);
	    ch->level = LEV_AWA;
	    do_help(ch, "newbie");
	}
    }
    else if (!IS_NPC(victim) && IS_NPC(ch)) victim->mdeath++;
    else if (!IS_NPC(victim) && !IS_NPC(ch) && ch != victim)
    {
	ch->pkill++;
	victim->pdeath++;
    }

    if (IS_NPC(victim) && (IS_EXTRA(victim, EXTRA_ILLUSION) || IS_SPIRIT(victim)))
	act("$n slowly fades away to nothing.", victim, NULL, NULL, TO_ROOM);
    else if (!IS_NPC(victim) || !IS_EXTRA(victim, EXTRA_ILLUSION))
    {
	death_cry(victim);
	make_corpse(victim, ch);
    }

    victim->position = POS_DEAD;

    if (IS_NPC(victim))
    {
	char_from_room(victim);
	char_to_room(victim, get_room_index(ROOM_VNUM_DISCONNECTION));
	extract_char(victim, TRUE);
	return;
    }
    	   
    victim->hit = 1;
    extract_char(victim, FALSE);
    while (victim->affected)
	affect_remove(victim, victim->affected);

    REMOVE_BIT(victim->extra, TIED_UP);
    REMOVE_BIT(victim->extra, GAGGED);
    REMOVE_BIT(victim->extra, BLINDFOLDED);
    REMOVE_BIT(victim->more, MORE_THINBLOOD);

    victim->itemaffect	 = 0;
    victim->blood[0]	 = victim->blood[1];
    victim->affected_by	 = 0;
    victim->armor	 = 0;
    victim->position	 = POS_RESTING;
    victim->hit		 = UMAX(1, victim->hit);
    victim->mana	 = UMAX(1, victim->mana);
    victim->hitroll	 = 0;
    victim->damroll	 = 0;
    victim->saving_throw = 0;
    victim->carry_weight = 0;
    victim->carry_number = 0;
    victim->pcdata->atm  = 0;
    do_call(victim, "all");
    do_autosave(victim, "");
    return;
}


void behead( CHAR_DATA *victim, CHAR_DATA *ch )
{
    ROOM_INDEX_DATA *location;
    char buf[MSL];

    if (IS_NPC(victim)) return;

    location = victim->in_room;

    stop_fighting(victim, TRUE);
    make_corpse(victim, ch);
    extract_char(victim, FALSE);

    char_from_room(victim);
    char_to_room(victim, location);

    if (victim == NULL)
    {
	bug("Behead: Victim no longer exists.", 0);
	return;
    }

    make_head(victim);

    while (victim->affected)
	affect_remove(victim, victim->affected);

    REMOVE_BIT(victim->extra, TIED_UP);
    REMOVE_BIT(victim->extra, GAGGED);
    REMOVE_BIT(victim->extra, BLINDFOLDED);
    victim->itemaffect	 = 0;
    victim->blood[0]     = victim->blood[1];
    victim->affected_by	 = 0;
    victim->armor	 = 0;
    victim->position	 = POS_STANDING;
    victim->hit		 = 1;
    victim->mana	 = 1;
    victim->hitroll	 = 0;
    victim->damroll	 = 0;
    victim->saving_throw = 0;
    victim->carry_weight = 0;
    victim->carry_number = 0;

    SET_BIT(victim->more, MORE_HEAD);
    SET_BIT(victim->affected_by, AFF_POLYMORPH);
    sprintf(buf, "the severed head of %s", victim->name);
    free_string(victim->morph);
    victim->morph = str_dup(buf);
    do_call(victim, "all");
    do_autosave(victim, "");
    return;
}

void group_gain( CHAR_DATA *ch, CHAR_DATA *victim )
{
    DESCRIPTOR_DATA *d;
    CHAR_DATA *gch;
    char buf[MSL];
    long xp;
    int count = 0, connected = 0;

    for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
	if (is_same_group(gch, ch)) count++;

    for (d = descriptor_list; d != NULL; d = d->next)
    {
	CHAR_DATA *wch;

	if (d->connected != CON_PLAYING && (d->connected < 10 || d->connected > 14)) continue;
	wch = (d->original != NULL) ? d->original : d->character;
	if (IS_BUI(wch)) continue;

	connected++;
    }

    if (victim == ch) return;

    for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
    {
	if (!is_same_group(gch, ch) || IS_NPC(gch)) continue;

	xp = xp_compute(victim);
	if (IS_FLAG(gch, FLAG_PK)) xp *= 1.25;

	if (count > 1)
	{
	    xp *= 2.5;
	    xp /= count;
	}

	if (connected >= 10) xp = (xp * (100 + connected)) / 100;

	if (xp == 1) strcpy(buf, "[You receive an experience point]\n\r");
	else sprintf(buf, "[You receive %li experience points]\n\r", xp);
	stc(buf, gch);

	if (IS_SET(exp_info.bit, EXP_RANDOM)) xp = (xp * number_range(0, 400)) / 100;
	else xp = (xp * exp_info.exp) / 100;

	if (IS_SET(exp_info.bit, EXP_RANDOM))
	{
	    sprintf(buf, "You recieve %li experience points due to random xp.\n\r", xp);
	    stc(buf, gch);
	}
	else if (exp_info.exp > 100)
	{
	    sprintf(buf, "You receive %li experience points due to bonus xp.\n\r", xp);
	    stc(buf, gch);
	}
	else if (exp_info.exp < 100)
	{
	    sprintf(buf, "You receive %li experience points due to reduced xp.\n\r", xp );
	    stc(buf, gch);
	}

        gch->pcdata->score[SCORE_TOTAL_LEVEL] += victim->level;
        if (victim->level > gch->pcdata->score[SCORE_HIGH_LEVEL]) gch->pcdata->score[SCORE_HIGH_LEVEL] += victim->level;
        gch->pcdata->score[SCORE_TOTAL_XP] += xp;
        if (xp > gch->pcdata->score[SCORE_HIGH_XP]) gch->pcdata->score[SCORE_HIGH_XP] += xp;
	gch->exp += xp;
    }

    return;
}

/*
 * Compute xp for a kill.
 * Edit this function to change xp computations.
 */
long xp_compute( CHAR_DATA *victim )
{
    long xp = victim->level;
    bool noexp = FALSE;

    if (IS_NPC(victim))
    {
	switch (victim->pIndexData->vnum)
	{
	    case 29600: case 30000: case 30001: case 30006: case 30007: case 30008: case 30009:
	    case 30010: case 30012: case 30013: case 30014: case 30015: case 30016:
		noexp = TRUE;
		break;
	}
	if (IS_SET(victim->act, ACT_NOEXP)) noexp = TRUE;
    }
    else noexp = TRUE;

    if (noexp) return 0;

    if (victim->level > 100) xp *= victim->level;
    else xp *= 100;
    xp *= 1.5;
    xp += victim->max_hit / 10;

    if (IS_AFFECTED(victim, AFF_SANCTUARY)) xp *= 1.25;

    return xp;
}

void dam_message( CHAR_DATA *ch, CHAR_DATA *victim, long dam, long type, char *mess )
{
    CHAR_DATA *rch;
    char buf1a[512], buf1b[512], buf2[512], buf3[512];
    const char *verb;
    const char *vs;
    char dammess[MIL];
    long vrand = number_range(0, 2);

    if (dam <= 50) vs = " lightly.";
    else if (dam <= 250) vs = ".";
    else if (dam <= 500) vs = " hard.";
    else if (dam <= 1000) vs = " very hard!";
    else if (dam <= 2500) vs = " extremely hard!";
    else if (dam <= 5000) vs = " incredibly hard!";
    else vs = " impossibly hard!!";

    switch(type)
    {
	default:		verb = "BUGGED";
				break;
	case TYPE_HIT:		if (vrand == 0) verb = "sucks";
				else if (vrand == 1) verb = "blows";
				else verb = "swallows";
				break;
	case TYPE_UNARMED:	if (vrand == 0) verb = "hits";
				else if (vrand == 1) verb = "strikes";
				else verb = "punishes";
				break;
	case TYPE_SLASH:	if (vrand == 0) verb = "cuts";
				else if (vrand == 1) verb = "slashes";
				else verb = "slices";
				break;
	case TYPE_PIERCE:	if (vrand == 0) verb = "stabs";
				else if (vrand == 1) verb = "pierces";
				else verb = "impales";
				break;
	case TYPE_BLUNT:	if (vrand == 0) verb = "pounds";
				else if (vrand == 1) verb = "crushes";
				else verb = "smashes";
				break;
	case TYPE_SPELL:	if (vrand == 0) verb = "blasts";
				else if (vrand == 1) verb = "incinerates";
				else verb = "explodes";
				break;
    }

    strcpy(dammess, mess);

    sprintf(buf1a, "$n's %s %s $t%s", dammess, verb, vs);
    sprintf(buf1b, "$n's %s %s $t%s [#0%li#n]", dammess, verb, vs, dam);
    sprintf(buf2, "Your %s %s $N%s [#g%li#n]", dammess, verb, vs, dam);
    sprintf(buf3, "$n's %s %s you%s [#r%li#n]", dammess, verb, vs, dam);

    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
	if(rch == ch || rch == victim) continue;
	if (IS_NPC(rch) || !IS_SET(rch->act, PLR_SDAM))
	{
	    if (!can_see(rch, victim)) act(buf1a, ch, "someone", rch, TO_VICT);
	    else if (!IS_NPC(victim))
	    {
		if (IS_AFFECTED(victim, AFF_POLYMORPH))
		    act(buf1a, ch, victim->morph, rch, TO_VICT);
		else act(buf1a, ch, victim->name, rch, TO_VICT);
	    }
	    else act(buf1a, ch, victim->short_descr, rch, TO_VICT);
	}
	else
	{
	    if (!can_see(rch, victim)) act(buf1b, ch, "someone", rch, TO_VICT);
	    else if (!IS_NPC(victim))
	    {
		if (IS_AFFECTED(victim, AFF_POLYMORPH))
		    act(buf1b, ch, victim->morph, rch, TO_VICT);
		else act(buf1b, ch, victim->name, rch, TO_VICT);
	    }
	    else act(buf1b, ch, victim->short_descr, rch, TO_VICT);
	}
    }
    act(buf2, ch, NULL, victim, TO_CHAR);
    act(buf3, ch, NULL, victim, TO_VICT);
    return;
}


/*
 * Trip a creature.
 * Caller must check for successful attack.
 */
void trip( CHAR_DATA *ch, CHAR_DATA *victim )
{
    char buf [MAX_STRING_LENGTH];

    if (IS_AFFECTED(victim,AFF_FLYING) || (IS_VAMPIRE(victim) && IS_VAMPAFF(victim,VAM_FLYING))) return;

    if (IS_NPC(ch) && victim->level > 2 && number_percent() > 5) return;

    if ( victim->wait == 0)
    {
    	sprintf(buf,"$n trips you and you go down!");
    	ADD_COLOUR(ch,buf,WHITE);
    	act( buf, ch, NULL, victim, TO_VICT    );
    	sprintf(buf,"You trip $N and $E goes down!");
    	ADD_COLOUR(ch,buf,WHITE);
    	act( buf,  ch, NULL, victim, TO_CHAR    );
    	sprintf(buf,"$n trips $N and $E goes down!");
    	ADD_COLOUR(ch,buf,WHITE);
    	act( buf,  ch, NULL, victim, TO_NOTVICT );

	WAIT_STATE( ch,     2 * PULSE_VIOLENCE );
	WAIT_STATE( victim, 2 * PULSE_VIOLENCE );
	victim->position = POS_RESTING;
    }

    return;
}

void do_kill( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if (IS_ITEMAFF(ch, ITEMA_PEACE)) do_majesty(ch,"");
    if ( arg[0] == '\0' )
    {
	send_to_char( "Kill whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "You cannot kill yourself!\n\r", ch );
	return;
    }

    if (IS_AFFECTED(ch, AFF_FEAR))
    {
	send_to_char( "No way, you're too scared!\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim ) )
	return;

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
	act( "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "You do the best you can!\n\r", ch );
	return;
    }

    autostance(ch);
    autostance(victim);

    WAIT_STATE(ch, PULSE_VIOLENCE);

    if (!IS_NPC(ch) && IS_WEREWOLF(ch) && get_tribe(ch, TRIBE_BLACK_FURIES) >= 4 && victim->position == POS_STANDING)
    {
	act("You charge into $N, knocking $M from $S feet.",ch,NULL,victim,TO_CHAR);
	act("$n charge into $N, knocking $M from $S feet.",ch,NULL,victim,TO_NOTVICT);
	act("$n charge into you, knocking you from your feet.",ch,NULL,victim,TO_VICT);
	multi_hit(ch, victim);
	multi_hit(ch, victim);
	return;
    }
    else multi_hit(ch, victim);
    return;
}

void do_redirect( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];
    CHAR_DATA *victim;

    one_argument(argument, arg);

    if (ch->position != POS_FIGHTING)
    {
	stc("You must be fighting redirect your attacks\n\r", ch);
	return;
    }

    if (arg[0] == '\0')
    {
	stc("Redirect your attacks toward whom?\n\r", ch);
	return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL)
    {
	stc("That person is not here.\n\r", ch);
	return;
    }

    if (ch->fighting == victim)
    {
	act("You are already fighting $m.\n\r", ch, NULL, victim, TO_CHAR);
	return;
    }

    if (is_safe(ch, victim)) return;

    if (victim == ch)
    {
	stc("Killing yourself is ill advised.\n\r",ch);
 	return;
    }

    ch->fighting = victim;
    act("You redirect your attacks toward $N!", ch, NULL, victim, TO_CHAR);
    act("$n redirects $s attacks toward $N!", ch, NULL, victim, TO_NOTVICT);
    act("$n redirects $s attacks toward you!", ch, NULL, victim, TO_VICT);

    WAIT_STATE(ch, 12);
    return;
}

void do_flee( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *was_in;
    ROOM_INDEX_DATA *now_in;
    CHAR_DATA *victim;
    CHAR_DATA *wch;
    CHAR_DATA *wch_next;
    long attempt;

    if ((victim = ch->fighting) == NULL)
    {
	if (ch->position == POS_FIGHTING)
	    ch->position = POS_STANDING;
	stc("You aren't fighting anyone.\n\r", ch);
	return;
    }

    if (ch->enrage_timer > 0)
    {
	stc("You snarl with rage and continue fighting.\n\r", ch);
	return;
    }

    if (IS_AFFECTED(ch, AFF_WEBBED))
    {
	send_to_char("You are unable to move with all this sticky webbing on.\n\r",ch);
	return;
    }
    else if (IS_AFFECTED(ch, AFF_SHADOW))
    {
	send_to_char("You are unable to move with these tendrils of darkness entrapping you.\n\r",ch);
	return;
    }
    if (!IS_NPC(ch) && ch->pcdata->wolf >= 0)
    {
	if (IS_VAMPIRE(ch) && number_percent() <= ch->beast && (number_range(1, 20) > 10))
	{
	    send_to_char("Your inner beast refuses to let you run!\n\r",ch);
	    WAIT_STATE(ch, 18);
	    return;
	}
	else if (IS_WEREWOLF(ch) && number_percent() <= ch->pcdata->wolf / 4)
	{
	    send_to_char("Your rage is too great!\n\r",ch);
	    WAIT_STATE(ch, 18);
	    return;
	}
    }
    if (IS_WEREWOLF(victim) && number_percent() < get_tribe(victim, TRIBE_GET_OF_FENRIS) * 10)
    {
	stc("You find yourself moving too slow to flee!.\n\r",ch);
	stc("You Halt the Cowards Flight.\n\r",victim);
	WAIT_STATE(ch, 18);
	return;
    }

    was_in = ch->in_room;
    for (attempt = 0; attempt < 6; attempt++)
    {
	EXIT_DATA *pexit;
	long door;

	door = number_door( );
	if ((pexit = was_in->exit[door]) == 0
	 || pexit->to_room == NULL || IS_SET(pexit->exit_info, EX_CLOSED)
	 || IS_SET(pexit->to_room->room_flags, ROOM_SAFE)
	 || (IS_NPC(ch) && IS_SET(pexit->to_room->room_flags, ROOM_NO_MOB)))
	    continue;

	move_char(ch, door);
	if ((now_in = ch->in_room) == was_in) continue;

	ch->in_room = was_in;
	act("$n has fled $t!", ch, dir_name[door], NULL, TO_ROOM);
	act("You flee to the $t!", ch, dir_name[door], NULL, TO_CHAR);
	ch->in_room = now_in;

	stc("You flee from combat!\n\r", ch);

	for (wch = was_in->people; wch != NULL; wch = wch_next)
	{
	    wch_next = wch->next_in_room;

	    if (IS_NPC(wch)) continue;
	    if (wch->fighting != ch) continue;
	    if (!IS_SET(wch->act, PLR_AUTOFOLLOW)) continue;
	    if (number_percent() < 50) {stop_fighting(wch, TRUE); continue;}

	    stc("You flee, but your enemy follows you!\n\r", ch);
	    stc("Your opponent tries to flee but you follow them!\n\r", wch);
	    move_char(wch, door);
	}

	WAIT_STATE(ch, 12);
	return;
    }

    stc("You were unable to escape!\n\r", ch);
    WAIT_STATE(ch, 18);
    return;
}

void do_rescue( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    CHAR_DATA *fch;
    CHAR_DATA *fch_next;
    char arg[MAX_INPUT_LENGTH];
    bool found = FALSE;

    one_argument( argument, arg );

    if (IS_ITEMAFF(ch, ITEMA_PEACE)) do_majesty(ch,"");

    if ( arg[0] == '\0' )
    {
	send_to_char( "Rescue whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "What about fleeing instead?\n\r", ch );
	return;
    }

    if ( !IS_NPC(ch) && IS_NPC(victim) )
    {
	send_to_char( "They don't need your help!\n\r", ch );
	return;
    }

    if ( ch->fighting == victim )
    {
	send_to_char( "Too late.\n\r", ch );
	return;
    }

    if (victim->fighting == NULL)
    {
	send_to_char( "That person is not fighting right now.\n\r", ch );
	return;
    }

    for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
    {
	fch_next = fch->next_in_room;

	if (fch == ch || fch == victim) continue;
	if (fch->fighting != victim) continue;
	if (auto_safe(ch, fch)) continue;

	found = TRUE;

	act( "You rescue $N!",  ch, NULL, victim, TO_CHAR    );
	act( "$n rescues you!", ch, NULL, victim, TO_VICT    );
	act( "$n rescues $N!",  ch, NULL, victim, TO_NOTVICT );

	stop_fighting( ch, FALSE );
	stop_fighting( fch, FALSE );
	stop_fighting( victim, FALSE );

	set_fighting( ch, fch );
	set_fighting( fch, ch );
	break;
    }

    if (!found) stc("They aren't being attacked.\n\r", ch);
    WAIT_STATE(ch, 8);

    return;
}

void do_kick( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];
    CHAR_DATA *victim;

    one_argument(argument, arg);

    if (IS_ITEMAFF(ch, ITEMA_PEACE)) do_majesty(ch,"");

    if (arg[0] == '\0' && ch->fighting == NULL)
    {
	send_to_char( "Who do you wish to kick?\n\r", ch );
	return;
    }

    if ( ( victim = ch->fighting ) == NULL )
    {
	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}
    }
    if ( ch == victim )
    {
	send_to_char( "Kick YOURSELF?\n\r", ch );
	return;
    }

    WAIT_STATE( ch, 12);
    one_hit(ch, victim, TYPE_UNARMED, "kick");
    return;
}

long dambonus( CHAR_DATA *ch, CHAR_DATA *victim, long dam )
{
    if (dam < 1) return 1;
    if (ch->stance[0] < 1) return dam;

    if (IS_STANCE(ch, STANCE_BULL)) dam *= 1 + (ch->stance[STANCE_BULL] * 0.01);
    else if (IS_STANCE(ch, STANCE_DRAGON)) dam *= 1 + (ch->stance[STANCE_DRAGON] * 0.01);
    else if (IS_STANCE(ch, STANCE_TIGER)) dam *= 1 + (ch->stance[STANCE_TIGER] * 0.01);

    if (IS_STANCE(victim, STANCE_CRAB)) dam /= 1 + (victim->stance[STANCE_CRAB] * 0.01);
    else if (IS_STANCE(victim, STANCE_DRAGON)) dam /= 1 + (victim->stance[STANCE_DRAGON] * 0.01);
    else if (IS_STANCE(victim, STANCE_SWALLOW)) dam /= 1 + (victim->stance[STANCE_SWALLOW] * 0.01);
    return dam;
}

void do_punch( CHAR_DATA *ch, char *argument )
{
    char arg [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    long dam;

    one_argument(argument,arg);

    if ( IS_NPC(ch) ) return;

    if (IS_ITEMAFF(ch, ITEMA_PEACE)) do_majesty(ch,"");

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "Nobody here by that name.\n\r", ch );
	return;
    }

    if ( ch == victim )
    {
	send_to_char( "You cannot punch yourself!\n\r", ch );
	return;
    }

    if ( victim->hit < victim->max_hit )
    {
	send_to_char( "They are hurt and suspicious.\n\r", ch );
	return;
    }

    if ( victim->position < POS_FIGHTING )
    {
	send_to_char( "You can only punch someone who is standing.\n\r", ch );
	return;
    }

    if (IS_ITEMAFF(ch, ITEMA_PEACE)) do_majesty(ch,"");

    if (is_safe(ch, victim)) return;

    act("You draw your fist back and aim a punch at $N.",ch,NULL,victim,TO_CHAR);
    act("$n draws $s fist back and aims a punch at you.",ch,NULL,victim,TO_VICT);
    act("$n draws $s fist back and aims a punch at $N.",ch,NULL,victim,TO_NOTVICT);
    WAIT_STATE(ch, 12);

    dam = number_range(1, 4);
    dam += char_damroll(ch);
    if (!IS_AWAKE(victim)) dam *= 2;
    if (dam <= 0) dam = 1;

    if ((IS_WEREWOLF(victim) && get_tribe(victim, TRIBE_GET_OF_FENRIS) >= 5) 
     || (IS_VAMPIRE(victim) && get_disc(victim, DISC_FORTITUDE) >= 7))
	dam = 0;

    damage(ch, victim, dam, TYPE_BLUNT, "punch");
    if (victim == NULL || victim->position == POS_DEAD || dam < 1) return;
    if (victim->position == POS_FIGHTING) stop_fighting(victim, TRUE);
    act("You fall to the ground stunned!", victim, NULL, NULL, TO_CHAR);
    act("$n falls to the ground stunned!", victim, NULL, NULL, TO_ROOM);
    victim->position = POS_STUNNED;
    return;
}

void do_berserk( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    OBJ_DATA *wield;
    char arg[MIL];
    long number_hit = 0, style = ch->style[0], dt = TYPE_HIT;

    argument = one_argument(argument, arg);

    if (IS_ITEMAFF(ch, ITEMA_PEACE)) do_majesty(ch, "");

    WAIT_STATE(ch, 18);

    act("You go BERSERK!", ch, NULL, NULL, TO_CHAR);
    act("$n goes BERSERK!", ch, NULL, NULL, TO_ROOM);

    if ((wield = get_eq_char(ch, WEAR_WEAPON)) == NULL || wield->item_type != ITEM_WEAPON)
    {
	if (style == STYLE_UNARMED || style == STYLE_SHIELD) dt = TYPE_UNARMED;
	if (style == STYLE_NATURAL) dt = TYPE_SLASH;
    }
    else dt = wield->value[3];

    if (dt == TYPE_HIT)
    {
	stc("Style Bug(berserk).  Inform Akuma.\n\r", ch);
	return;
    }

    for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
    {
	vch_next = vch->next_in_room;
	if (number_hit >= 5) continue;
	if (!IS_NPC(vch) && vch->pcdata->chobj != NULL) continue;
	if (ch == vch) continue;
	if ((IS_NPC(ch) && IS_NPC(vch)) || (!IS_NPC(ch) && !IS_NPC(vch))) continue;
	if (!can_see(ch, vch)) continue;

	one_hit(ch, vch, dt, "1");	/* Multi_hit is too much */
	number_hit++;
    }
    return;
}



/* Hurl skill by KaVir */
void do_hurl( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA       *victim;
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA       *pexit;
    EXIT_DATA       *pexit_rev;
    char            buf       [MAX_INPUT_LENGTH];
    char            direction [MAX_INPUT_LENGTH];
    char            arg1      [MAX_INPUT_LENGTH];
    char            arg2      [MAX_INPUT_LENGTH];
    long             door;
    long             rev_dir;
    long             dam;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (IS_ITEMAFF(ch, ITEMA_PEACE)) do_majesty(ch,"");

    if ( arg1[0] == '\0' )
    {
	send_to_char("Who do you wish to hurl?\n\r", ch);
	return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
	send_to_char("They are not here.\n\r", ch);
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "How can you hurl yourself?\n\r", ch );
	return;
    }

    if (victim->hit < victim->max_hit || (victim->position == POS_FIGHTING && victim->fighting != ch))
    {
	act( "$N is hurt and suspicious, and you are unable to approach $M.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( is_safe( ch, victim ) )
      return;

    if (!IS_NPC(victim) && IS_WEREWOLF(victim) && get_tribe(victim, TRIBE_GET_OF_FENRIS) >= 5)
    {
	send_to_char("You are unable to get their feet of the ground.\n\r",ch);
	return;
    }

    if ( IS_NPC(victim) && victim->level > 900 )
    {
	send_to_char("You are unable to get their feet of the ground.\n\r",ch);
	return;
    }

    WAIT_STATE( ch, 9);

    rev_dir = 0;

    if ( arg2[0] == '\0' )
	door = number_range(0,3);
    else
    {
    	if      ( !str_cmp( arg2, "n" ) || !str_cmp( arg2, "north" ) )
	    door = 0;
    	else if ( !str_cmp( arg2, "e" ) || !str_cmp( arg2, "east" ) )
	    door = 1;
    	else if ( !str_cmp( arg2, "s" ) || !str_cmp( arg2, "south" ) )
	    door = 2;
    	else if ( !str_cmp( arg2, "w" ) || !str_cmp( arg2, "west" ) )
	    door = 3;
    	else
    	{
	    send_to_char("You can only hurl people north, south, east or west.\n\r", ch);
	    return;
    	}
    }

    if (door == 0) {sprintf(direction,"north");rev_dir = 2;}
    if (door == 1) {sprintf(direction,"east");rev_dir = 3;}
    if (door == 2) {sprintf(direction,"south");rev_dir = 0;}
    if (door == 3) {sprintf(direction,"west");rev_dir = 1;}

    if (( pexit = ch->in_room->exit[door]) == NULL || (to_room = pexit->to_room) == NULL)
    {
	sprintf(buf,"$n hurls $N into the %s wall.", direction);
	act(buf,ch,NULL,victim,TO_NOTVICT);
	sprintf(buf,"You hurl $N into the %s wall.", direction);
	act(buf,ch,NULL,victim,TO_CHAR);
	sprintf(buf,"$n hurls you into the %s wall.", direction);
	act(buf,ch,NULL,victim,TO_VICT);
	dam = number_range(ch->level, (ch->level * 4));
	victim->hit = victim->hit - dam;
	update_pos(victim);
    	if (IS_NPC(victim) && !IS_NPC(ch)) ch->mkill = ch->mkill + 1;
    	if (!IS_NPC(victim) && IS_NPC(ch)) victim->mdeath = victim->mdeath + 1;
	if (victim->position == POS_DEAD) 
	{
	    raw_kill(victim, ch);
	    return;
	}
	return;
    }

    pexit = victim->in_room->exit[door];
    if (IS_SET(pexit->exit_info, EX_CLOSED) && 
	!IS_AFFECTED(victim,AFF_PASS_DOOR ) &&
	!IS_AFFECTED(victim,AFF_ETHEREAL  ) )
    {
	if (IS_SET(pexit->exit_info, EX_LOCKED))
	    REMOVE_BIT(pexit->exit_info, EX_LOCKED);
	if (IS_SET(pexit->exit_info, EX_CLOSED))
	    REMOVE_BIT(pexit->exit_info, EX_CLOSED);
	sprintf(buf,"$n hoists $N in the air and hurls $M %s.", direction);
	act(buf,ch,NULL,victim,TO_NOTVICT);
	sprintf(buf,"You hoist $N in the air and hurl $M %s.", direction);
	act(buf,ch,NULL,victim,TO_CHAR);
	sprintf(buf,"$n hurls you %s, smashing you through the %s.", direction,pexit->keyword);
	act(buf,ch,NULL,victim,TO_VICT);
	sprintf(buf,"There is a loud crash as $n smashes through the $d.");
	act(buf,victim,NULL,pexit->keyword,TO_ROOM);

	if ( ( to_room   = pexit->to_room               ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir] ) != NULL
	&&   pexit_rev->to_room == ch->in_room
	&&   pexit_rev->keyword != NULL )
	{
	    if (IS_SET(pexit_rev->exit_info, EX_LOCKED))
		REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
	    if (IS_SET(pexit_rev->exit_info, EX_CLOSED))
		REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
	    if (door == 0) sprintf(direction,"south");
	    if (door == 1) sprintf(direction,"west");
	    if (door == 2) sprintf(direction,"north");
	    if (door == 3) sprintf(direction,"east");
	    char_from_room(victim);
	    char_to_room(victim,to_room);
	    sprintf(buf,"$n comes smashing in through the %s $d.", direction);
	    act(buf,victim,NULL,pexit->keyword,TO_ROOM);
	    dam = number_range(ch->level, (ch->level * 6));
	    victim->hit = victim->hit - dam;
	    update_pos(victim);
    	    if (IS_NPC(victim) && !IS_NPC(ch)) ch->mkill = ch->mkill + 1;
    	    if (!IS_NPC(victim) && IS_NPC(ch)) victim->mdeath = victim->mdeath + 1;
	    if (victim->position == POS_DEAD) 
	    {
		raw_kill(victim, ch);
		return;
	    }
	}
    }
    else
    {
	sprintf(buf,"$n hurls $N %s.", direction);
	act(buf,ch,NULL,victim,TO_NOTVICT);
	sprintf(buf,"You hurl $N %s.", direction);
	act(buf,ch,NULL,victim,TO_CHAR);
	sprintf(buf,"$n hurls you %s.", direction);
	act(buf,ch,NULL,victim,TO_VICT);
	if (door == 0) sprintf(direction,"south");
	if (door == 1) sprintf(direction,"west");
	if (door == 2) sprintf(direction,"north");
	if (door == 3) sprintf(direction,"east");
	char_from_room(victim);
	char_to_room(victim,to_room);
	sprintf(buf,"$n comes flying in from the %s.", direction);
	act(buf,victim,NULL,NULL,TO_ROOM);
	dam = number_range(ch->level, (ch->level * 2));
	victim->hit = victim->hit - dam;
	update_pos(victim);
    	if (IS_NPC(victim) && !IS_NPC(ch)) ch->mkill = ch->mkill + 1;
    	if (!IS_NPC(victim) && IS_NPC(ch)) victim->mdeath = victim->mdeath + 1;
	if (victim->position == POS_DEAD) 
	{
	    raw_kill(victim, ch);
	    return;
	}
    }
    return;
}



void do_disarm( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    long dex = number_range(1, 20);

    if ((victim = ch->fighting) == NULL)
    {
	stc("You aren't fighting anyone.\n\r", ch);
	return;
    }

    if ((obj = get_eq_char(victim, WEAR_WEAPON)) == NULL)
    {
	stc("Your opponent is not wielding a weapon.\n\r", ch);
	return;
    }

    dex += char_stat(ch, STAT_DEX);
    dex -= char_stat(victim, STAT_DEX);

    if (dex > 10)
    {
	obj_from_char(obj);
	obj_to_char(obj, victim);

	act("You disarm $N!", ch, NULL, victim, TO_CHAR);
	act("$n disarms you!", ch, NULL, victim, TO_VICT);
	act("$n disarms $N!", ch, NULL, victim, TO_NOTVICT);
    }
    else
    {
	act("You fail on your attempt to disarm $N!", ch, NULL, victim, TO_CHAR);
	act("$n fails on $s attempt to disarm you!", ch, NULL, victim, TO_VICT);
	act("$n fails on $s attempt to disarm $N!", ch, NULL, victim, TO_NOTVICT);
    }

    WAIT_STATE(ch, 15);
    return;
}


void do_slay( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MIL];
    char buf[MSL];

    one_argument(argument, arg);

    if (arg[0] == '\0')
    {
	stc("Slay whom?\n\r", ch);
	return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL)
    {
        stc("They are not here.\n\r", ch);
        return;
    }

    if (!IS_NPC(victim) && !IS_BUI(victim) && !IS_ST(ch))
    {
	stc("You may not slay mortals.\n\r", ch);
	return;
    }

    if (!IS_NPC(victim) && victim->desc->connected != CON_PLAYING)
    {
	stc("They are busy right now.\n\r", ch);
	return;
    }

    if (!IS_NPC(victim) && victim->level > ch->level)
    {
	sprintf(buf, "%s just failed to slay %s!", ch->name, victim->name);
	do_info(ch, buf);
	victim = ch;
    }

    if (!IS_NPC(victim) && IS_FLAG(victim, FLAG_AFK))  REMOVE_BIT(ch->pcdata->flag, FLAG_AFK);

    act("You slay $M in cold blood!", ch, NULL, victim, TO_CHAR);
    act("$n slays you in cold blood!", ch, NULL, victim, TO_VICT);
    act("$n slays $N in cold blood!", ch, NULL, victim, TO_NOTVICT);
    raw_kill(victim, ch);
    return;
}

/* Had problems with people not dying when POS_DEAD...KaVir */
void killperson( CHAR_DATA *ch, CHAR_DATA *victim )
{
    stc("You have been KILLED!!\n\r", victim);
    raw_kill(victim, ch);
    return;
}

/* For decapitating players - KaVir */
void do_decapitate( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    CHAR_DATA *vch;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    bool dox = FALSE;

    one_argument(argument, arg);

    if (IS_NPC(ch)) return;

    if (IS_ITEMAFF(ch, ITEMA_PEACE)) do_majesty(ch, "");

    if (arg[0] == '\0' || (victim = get_char_room(ch, arg)) == NULL)
    {
	stc("Decapitate whom?\n\r", ch);
	return;
    }

    if (IS_NPC(victim) || ch == victim)
    {
	stc("You may only decapitate other players.\n\r", ch);
	return;
    }

    if (!CAN_PK(ch))
    {
	stc("You must have PK flag to decapitate.\n\r", ch);
	return;
    }
    if (!CAN_PK(victim))
    {
	stc("They are not PK, so you cannot decapitate them,\n\r",ch);
	return;
    }
    if (victim->position > POS_STUNNED || victim->hit > 0)
    {
	stc("You can only do this to mortally wounded players.\n\r", ch);
	return;
    }

    if (is_safe(ch,victim))
    {
	stc("They are safe from you.\n\r", ch);
	return;
    }

    act("You bring your weapon down upon $N's neck!", ch, NULL, victim, TO_CHAR);
    stc("Your head is sliced from your shoulders!\n\r", victim);
    act("$n swings $s weapon down towards $N's neck!", ch, NULL, victim, TO_NOTVICT);
    act("$n's head is sliced from $s shoulders!", victim, NULL, NULL, TO_ROOM);
    ch->fight_timer = 20;
    if (strlen(victim->pcdata->love) > 1 && !str_cmp(ch->name, victim->pcdata->love))
    {
	free_string(victim->pcdata->love);
	victim->pcdata->love = str_dup("");
    }
    if ((vch = victim->embrace ) != NULL)
    {
	vch->embrace = NULL;
	vch->embraced = ARE_NONE;
	victim->embrace = NULL;
	victim->embraced = ARE_NONE;
    }

    act("A misty white vapour pours from $N's corpse into your body.", ch, NULL, victim, TO_CHAR);
    act("A misty white vapour pours from $N's corpse into $n's body.", ch, NULL, victim, TO_NOTVICT);
    act("You double over in agony as raw energy pulses through your veins!", ch, NULL, NULL, TO_CHAR);
    act("$n doubles over in agony as sparks of energy crackle around $m!", ch, NULL, NULL, TO_NOTVICT);

    if (IS_WEREWOLF(victim)) if (IS_SET(victim->act, PLR_WOLFMAN)) do_unwerewolf(victim,"");
    if (IS_VAMPAFF(victim, VAM_NIGHTSIGHT)) do_nightsight(victim,"");
    if (IS_VAMPAFF(victim, VAM_FANGS)) do_fangs(victim,"");
    if (IS_VAMPAFF(victim, VAM_CLAWS)) do_claws(victim,"");
    mortal_vamp(victim);

    behead(victim, ch);
    ch->pkill++;
    victim->pdeath++;
    victim->pcdata->wolf = 0;

    if (((victim->status <= 0 && ch->status > 0) || (victim->status + 5) < ch->status) && (victim->name != ch->pcdata->decap_char))
    {
	dox = TRUE;
	sprintf(buf, "#r%s#n has been decapitated by #r%s#n for no status.", victim->name, ch->name);
	sprintf(log_buf, "%s decapitated by %s at %li for no status.", victim->name, ch->name, victim->in_room->vnum);
    }
    else
    {
	ch->status++;
	if (victim->status > 0) victim->status--;
	sprintf(buf, "#r%s#n has been decapitated by #r%s#n.", victim->name, ch->name);
	sprintf(log_buf, "%s decapitated by %s at %li.", victim->name, ch->name, victim->in_room->vnum);
    }
    if ((victim->status - ch->status) > 5)
    {
	victim->pcdata->decap_char = ch->name;
	stc("You now have an opportunity to retaliate.\n\r", victim);
    }
    if (victim->name == ch->pcdata->decap_char)
    {
	ch->pcdata->decap_char = "";
    }
    do_fatality(ch, buf);
    log_string(log_buf);
    if (dox) paradox(ch);
    return;
}

void do_destroy( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MIL];
    char logbuf[MIL];

    one_argument(argument, arg);

    if (arg[0] == '\0' || (victim = get_char_room(ch, arg)) == NULL)
    {
	stc("Destroy whom?\n\r", ch);
	return;
    }

    if (!CAN_PK(ch) || !CAN_PK(victim))
    {
	stc("Both players need a PK flag.\n\r", ch);
	return;
    }

    if (is_safe(ch, victim)) return;

    if (victim->position > POS_STUNNED || victim->hit > 0)
    {
	stc("You may only destroy the mortally wounded.\n\r",ch);
	return;
    }

    act("You summon forth your hatred to destroy $N",  ch, NULL, victim, TO_CHAR);
    act("$n summons forth $s hatred to destroy you", ch, NULL, victim, TO_VICT);
    act("$n summons forth $s hatred to destroy $N",  ch, NULL, victim, TO_NOTVICT);

    sprintf(logbuf, "%s destroyed %s at %li", ch->name, victim->name, victim->in_room->vnum);

    log_string(logbuf);
    raw_kill(victim, ch);
    return;
}

void death_message( CHAR_DATA *ch, CHAR_DATA *victim )
{
    char buf[MSL];
    char *chsex;
    if (ch == NULL || victim == NULL) return;
    chsex = ch->sex == SEX_FEMALE ? "her" : "his";

    if (!IS_BUI(ch) && ch == victim) sprintf(buf, "#r%s#n has just died!", ch->name);
    else if (strlen(ch->pcdata->cdest) >= 2) sprintf(buf, ch->pcdata->cdest, ch->name, victim->name);
    else if (IS_IMM(ch)) sprintf(buf, "#r%s#n has slain #r%s#n in cold blood!", ch->name, victim->name);
    else sprintf(buf, "#r%s#n has summoned forth %s hatred to destroy #r%s#n!", ch->name, chsex, victim->name);

    buf[0] = UPPER(buf[0]);
    do_fatality(ch, buf);
    return;
}

void dp_gain( CHAR_DATA *ch, CHAR_DATA *victim )
{	
    long vnum;
    long dp = 0;
    char buf[MSL];

    if (IS_NPC(victim) && (IS_DROW(ch) || IS_DRAGON(ch)))
    {
	if ((vnum = victim->pIndexData->vnum) > 29000)
	{
	    switch (vnum)
	    {
		case 29600: case 30000: case 30001: case 30006: 
	        case 30007: case 30008: case 30009: case 30010: 
		case 30013: case 30014: case 30015: case 30016:
		    return;
	    }
	}
	if (IS_SET(victim->act, ACT_NOEXP)) return;

	dp = victim->level;
	if (IS_SET(exp_info.bit, EXP_RANDOM)) dp = (dp * number_range(0, 400)) / 100;
	else dp = (dp * exp_info.exp) / 100;

	if (dp == 1)
	{
	    strcpy(buf, "[You receive a class point]\n\r");
	    stc(buf, ch);
	}
	else if (dp > 1)
	{
	    sprintf(buf, "[You receive %li class points]\n\r", dp);
	    stc(buf, ch);
	}

	ch->pcdata->classpoints[CP_CURRENT] += dp;
	ch->pcdata->classpoints[CP_TOTAL] += dp;
	 }
    return;
}

void do_statcheck(CHAR_DATA* ch, char* argument)
{
    CHAR_DATA *victim;
    char arg[MSL], buf[MSL];
    long av, br;

    one_argument(argument, arg);

    if ((victim = get_char_world(ch, arg)) == NULL)
    {
	stc("They aren't here.\n\r", ch);
	return;
    }

    av = avoidblow(victim);
    br = breakavoid(victim);

    sprintf(buf, "Statcheck %s.\n\r", victim->name);
    stc(buf, ch);
    sprintf(buf, "Avoid: %li.  Break: %li.  Speed: %li.\n\r", av, br, get_speed(victim));
    stc(buf, ch);

    return;
}
