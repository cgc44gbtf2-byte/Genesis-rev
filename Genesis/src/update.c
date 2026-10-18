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

void	regen		args( ( CHAR_DATA *ch ) );
void	mobile_update	args( ( void ) );
void	weather_update	args( ( void ) );
void	char_update	args( ( void ) );
void	obj_update	args( ( void ) );
void	aggr_update	args( ( void ) );
void	trap_update	args( ( void ) );



void regen( CHAR_DATA *ch )
{
    long heal;

    if (ch->hit <= 0)
    {
	ch->hit += 3;
	update_pos(ch);
	return;
    }

    if (IS_AFFECTED(ch, AFF_FLAMING))
    {
	if (number_range(1, 20) <= char_stat(ch, STAT_DEX))
	{
	    stc("You manage to smother the fires around you!\n\r", ch);
	    REMOVE_BIT(ch->affected_by, AFF_FLAMING);
	}
	else
	{
	    stc("You cannot regen while on fire!\n\r", ch);
	    return;
	}
    }

    if (ch->hit < ch->max_hit)
    {
	if (ch->position == POS_FIGHTING && !IS_NPC(ch)) heal = 50 * char_stat(ch, STAT_CON);
	else if (ch->position == POS_RESTING) heal = 250 * char_stat(ch, STAT_CON);
	else if (ch->position == POS_MEDITATING) heal = 350 * char_stat(ch, STAT_CON);
	else heal = 150 * char_stat(ch, STAT_CON);

	if (!IS_NPC(ch) && ch->fight_timer <= 0) heal *= 1 + (ch->hit / 20000);

	if (IS_WEREWOLF(ch)) heal *= 1.5;
	if (IS_MONK(ch) && get_monk(ch, MONK_MANTRA) >= 3) heal *= 1.5;
	if (IS_NINJA(ch) && ch->pcdata->regenerate > 0)
	{
	    heal *= 1.5;
	    ch->pcdata->regenerate--;
	    if (ch->pcdata->regenerate <= 0) stc("Your improved regen fades.\n\r", ch);
	}
	if (IS_VAMPIRE(ch) && ch->pcdata->regenerate != 0)
	{
	    int cost = 4;

	    if (IS_MORE(ch, MORE_THINBLOOD))
	    {
		cost *= 5;
		stc("Your thinned blood makes it difficult to regenerate.\n\r", ch);
	    }

	    if (ch->blood[BLOOD_CURRENT] < cost)
		stc("You do not have enough blood to regenerate more.\n\r", ch);
	    else
	    {
		ch->blood[BLOOD_CURRENT] -= cost;
		if (IS_MORE(ch, MORE_THINBLOOD)) heal *= 1.5;
		else heal *= 2;
	    }
	}

	if (IS_MORE(ch, MORE_DISINT)) heal /= 4;

	ch->hit += heal;
	if (ch->hit > ch->max_hit)
	{
	    ch->hit = ch->max_hit;
	    stc("Your health has completely regenerated.\n\r", ch);
	}
    }

    if (IS_ITEMAFF(ch, ITEMA_BLISSFUL) || (IS_MAGE(ch) && IS_MORE(ch, MORE_SHIELDED)))
    {
	ch->mana -= 1000;
	if (ch->mana < 0)
	{
	    ch->mana = 0;
	    stc("You no longer have the mana to keep yourself hidden.\n\r", ch);
	    REMOVE_BIT(ch->itemaffect, ITEMA_BLISSFUL);
	    if (IS_MAGE(ch)) REMOVE_BIT(ch->more, MORE_SHIELDED);
	}
    }
    else if (ch->mana < ch->max_mana)
    {
	if (ch->position == POS_FIGHTING) heal = 50 * char_stat(ch, STAT_WIS);
	else if (ch->position == POS_RESTING) heal = 250 * char_stat(ch, STAT_WIS);
	else if (ch->position == POS_MEDITATING) heal = 350 * char_stat(ch, STAT_WIS);
	else heal = 150 * char_stat(ch, STAT_WIS);

	if (ch->fight_timer <= 0) heal *= 1 + (ch->mana / 20000);

	if (IS_MONK(ch) && get_monk(ch, MONK_MANTRA) >= 3) heal *= 1.5;
	if (IS_SORCERER(ch) || IS_MAGE(ch)) heal *= 1.5;

	ch->mana += heal;
	if (ch->mana > ch->max_mana)
	{
	    ch->mana = ch->max_mana;
	    stc("Your mana has completely regenerated.\n\r", ch);
	}
    }
    return;
}



/*
 * Mob autonomous action.
 * This function takes 25% to 35% of ALL Merc cpu time.
 * -- Furey
 */
void mobile_update( void )
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    EXIT_DATA *pexit;
    long door;

    /* Examine all mobs. */
    for (ch = char_list; ch != NULL; ch = ch_next)
    {
	ch_next = ch->next;

	if (ch->in_room == NULL) continue;

	regen(ch);
	if (!IS_NPC(ch))
	{
	    if (ch->desc == NULL || !IS_PLAYING(ch->desc)) continue;

	    if (ch->position <= POS_STUNNED && ch->pcdata->wolf > 0 && !IS_NINJA(ch))
	    {
		ch->pcdata->wolf = 0;
		if (IS_WEREWOLF(ch)) do_unwerewolf(ch, "");
		if (IS_VAMPAFF(ch, VAM_NIGHTSIGHT) && !IS_POLYAFF(ch, POLY_ZULO)) do_nightsight(ch, "");
		if (IS_VAMPAFF(ch, VAM_FANGS) && !IS_POLYAFF(ch, POLY_ZULO)) do_fangs(ch, "");
		if (IS_VAMPAFF(ch, VAM_CLAWS) && !IS_POLYAFF(ch, POLY_ZULO)) do_claws(ch, "");
		stc("You feel your beast release its hold.\n\r", ch);
	    }

	    if (ch->embraced == ARE_EMBRACING)
	    {
		CHAR_DATA *vch;
		long amount = number_range(20, 40);

		if ((vch = ch->embrace) == NULL) ch->embraced = ARE_NONE;
		else if (ch->position < POS_STANDING || vch->position == POS_FIGHTING
		 || vch->in_room != ch->in_room || is_safe(ch, vch))
		{
		    vch->embrace = NULL;
		    vch->embraced = ARE_NONE;
		    ch->embrace = NULL;
		    ch->embraced = ARE_NONE;
		}
		else if (vch->level > 500 && vch->position > POS_STUNNED)
		{
		    if (vch->position < POS_FIGHTING)
		    {
			act("You clamber to your feet.", vch, NULL, NULL, TO_CHAR);
			act("$n clambers to $s feet.", vch, NULL, NULL, TO_ROOM);
			vch->position = POS_STANDING;
		    }
		    act("$N rips your head away from $S throat and hurls you across the room.", ch, NULL, vch, TO_CHAR);
		    act("You rip $n's head away from your throat and hurl $m across the room.", ch, NULL, vch, TO_VICT);
		    act("$N rips $n's head away from $S throat and hurls $m across the room.", ch, NULL, vch, TO_NOTVICT);
		    act("You crash to the ground, stunned.", ch, NULL, NULL, TO_CHAR);
		    act("$n crashes to the ground, stunned.", ch, NULL, NULL, TO_ROOM);
		    stop_fighting(ch, TRUE);
		    ch->position = POS_STUNNED;
		    ch->embrace = NULL;
		    ch->embraced = ARE_NONE;
		    vch->embrace = NULL;
		    vch->embraced = ARE_NONE;
		}
		else
		{
		    act("You shudder with ecstacy as you drain $N's lifeblood from $S throat.", ch, NULL, vch, TO_CHAR);
		    act("The gentle motion of $n's lips on your throat sends electric shivers through your body.", ch, NULL, vch, TO_VICT);
		    act("$n drinks deeply from $N's throat.", ch, NULL, vch, TO_NOTVICT);
		    update_pos(vch);

		    if (amount > vch->blood[BLOOD_CURRENT]) amount = vch->blood[BLOOD_CURRENT];
		    ch->blood[BLOOD_CURRENT] += amount;
		    vch->blood[BLOOD_CURRENT] -= amount;
		    if (ch->blood[BLOOD_CURRENT] >= ch->blood[BLOOD_POOL]) ch->blood[BLOOD_CURRENT] = ch->blood[BLOOD_POOL];

		    if (vch->blood[BLOOD_CURRENT] <= vch->blood[BLOOD_POOL] / 3)
		    {
			if (!IS_VAMPIRE(vch))
			{
			    act("$N sways weakly in your arms.", ch, NULL, vch, TO_CHAR);
			    act("You sway weakly in $n's arms.", ch, NULL, vch, TO_VICT);
			    act("$N sways weakly in $n's arms.", ch, NULL, vch, TO_NOTVICT);
			}
			if (vch->blood[BLOOD_CURRENT] <= vch->blood[BLOOD_POOL] / 10)
			{
			    act("You can feel that $N is very low on blood.", ch, NULL, vch, TO_CHAR);
			    stc("You are very low on blood.\n\r", vch);
			}
		    }

		    if ((IS_WEREWOLF(vch) && get_tribe(vch, TRIBE_GET_OF_FENRIS) >= 3)
		     || (IS_VAMPIRE(vch) && get_disc(vch, DISC_VICISSITUDE) >= 6))
		    {
			long dam = number_range(150, 300) * amount;
			act("Your lips blister and burn as $N's blood enters your mouth!", ch, NULL, vch, TO_CHAR);
			hurt_person(ch, vch, dam);
			if (ch->position <= POS_STUNNED)
			{
			    act("$n keels over and collapses on the floor.",ch,NULL,NULL,TO_ROOM);
			    ch->embrace = NULL;
			    ch->embraced = ARE_NONE;
			    vch->embrace = NULL;
			    vch->embraced = ARE_NONE;
			}
		    }

		    if (IS_MORE(vch, MORE_BRISTLES))
		    {
			long dam = number_range(5000, 7500);
			damage(vch, ch, dam, TYPE_HIT, "bristles");
			stop_fighting(vch, TRUE);
			update_pos(ch);
			if (ch->position <= POS_STUNNED)
			{
			    act("$n keels over and collapses on the floor.",ch,NULL,NULL,TO_ROOM);
			    ch->embrace = NULL;
			    ch->embraced = ARE_NONE;
			    vch->embrace = NULL;
			    vch->embraced = ARE_NONE;
			}
		    }

		    if (vch->blood[BLOOD_CURRENT] <= 0)
		    {
			act("Your body falls limply to the ground.", vch, NULL, NULL, TO_CHAR);
			act("$n's body falls limply to the ground.", vch, NULL, NULL, TO_ROOM);
			ch->embrace = NULL;
			ch->embraced = ARE_NONE;
			vch->embrace = NULL;
			vch->embraced = ARE_NONE;
			if(IS_VAMPIRE(vch))
			{
			    ch->exp += vch->exp /2;
			    vch->exp -= vch->exp / 2;
			    ch->truebeast += 10;
			}
			if (IS_VAMPIRE(vch) && vch->vampgen < ch->vampgen)
			{
			    ch->vampgen = vch->vampgen;
			    if (vch->vampgen > 3) vch->vampgen += 2;
			}
			killperson(ch, vch);
		    }

		}
	    }
	

	    if (IS_MAGE(ch))
	    {
		if (ch->quint[BODY] < ch->arete && number_range(1, 2) == 1) ch->quint[BODY]++;
		if (ch->fighting == NULL && ch->paradox > 0 && number_range(1,20) == 1) ch->paradox--;

		if (IS_AFFECTED(ch, AFF_DEEPUMBRA))
		{
		    if (ch->quint[BODY] < ch->arete) ch->quint[BODY]++;
		    regen(ch);
		}
	    }
	    else if (IS_NINJA(ch))
	    {
		if (ch->position == POS_FIGHTING) REMOVE_BIT(ch->affected_by, AFF_KAKUSU);

		if (ch->pcdata->ninja[NINJA_KI] < 300)
		{
		    int ki;

		    if (IS_AFFECTED(ch, AFF_KAKUSU)) ki = 0;
		    else if (ch->position == POS_MEDITATING) ki = 5;
		    else ki = 2;

		    ch->pcdata->ninja[NINJA_KI] += ki;
		    if (ch->pcdata->ninja[NINJA_KI] > 300) ch->pcdata->ninja[NINJA_KI] = 300;
		}

		if (ch->pcdata->ninja[NINJA_HARAKIRI] > 0) ch->pcdata->ninja[NINJA_HARAKIRI]--;
		if (ch->pcdata->wolf > 0)
		{
		    if (ch->position != POS_FIGHTING) ch->pcdata->wolf--;
		    else if (ch->pcdata->wolf < get_ninja(ch, NINJA_SORA) * 25) ch->pcdata->wolf++;
		}
	    }
            else if (IS_VAMPIRE(ch))
            {
		if (IS_VAMPAFF(ch, VAM_OBFUSCATE))
		{
		    ch->blood[BLOOD_CURRENT]--;
		    if (ch->blood[BLOOD_CURRENT] <= 0)
		    {
			REMOVE_BIT(ch->vampaff, VAM_OBFUSCATE);
			stc("You drop out of obfuscate.\n\r", ch);
		    }
		}
		if (IS_POLYAFF(ch, POLY_SHADOW))
		{
		    ch->blood[BLOOD_CURRENT] -= 5;
		    if (ch->blood[BLOOD_CURRENT] <= 0)
		    {
			ch->blood[BLOOD_CURRENT] = 0;
			stc("You reform your human body\n\r", ch);
			REMOVE_BIT(ch->polyaff, POLY_SHADOW);
			REMOVE_BIT(ch->affected_by, AFF_ETHEREAL);
			REMOVE_BIT(ch->affected_by, AFF_POLYMORPH);
		    }
		}
		if (IS_ITEMAFF(ch, ITEMA_PEACE))
		{
		    ch->blood[BLOOD_CURRENT] -= 5;
		    if (ch->blood[BLOOD_CURRENT] <= 0)
		    {
			ch->blood[BLOOD_CURRENT] = 0;
			REMOVE_BIT(ch->itemaffect, ITEMA_PEACE);
			stc("You are no longer so imposing.\n\r", ch);
		    }
		}
		if (ch->pcdata->wolf > 0)
		{
		    if (ch->position == POS_FIGHTING && (ch->pcdata->wolf < ch->beast * 0.5)) ch->pcdata->wolf++;
		    else
		    {
			if (number_percent() < ch->beast) beast_rage(ch);
			else ch->pcdata->wolf--;
			if (ch->pcdata->wolf <= 0) stc("You feel your beast release it's hold.\n\r", ch);
		    }
		}
	    }
	    else if (IS_WEREWOLF(ch))
	    {
		if (ch->position == POS_FIGHTING)
		{
		    int rage = 300;

		    if (IS_ITEMAFF(ch, ITEMA_BLISSFUL)) REMOVE_BIT(ch->itemaffect, ITEMA_BLISSFUL);
		    if (get_tribe(ch, TRIBE_RED_TALONS) >= 3) rage += 50;

		    if (ch->pcdata->wolf < rage)
		    {
			if (get_auspice(ch, AUSPICE_AHROUN) >= 4) ch->pcdata->wolf += number_range(15, 30);
			else ch->pcdata->wolf += number_range(5, 10);
			if (ch->pcdata->wolf > rage) ch->pcdata->wolf = rage;
		    }

		    if (!IS_SET(ch->act, PLR_WOLFMAN) && ch->pcdata->wolf >= 100) do_werewolf(ch,"");
		}
		else if (ch->pcdata->wolf > 0)
		{
		    ch->pcdata->wolf--;
		    if (ch->pcdata->wolf < 100) do_unwerewolf(ch,"");
		}
	    }
	    else if (IS_DROW(ch))
	    {
		if (get_drow(ch, DROW_GLOBE))
		{
		    ch->pcdata->classpoints[CP_CURRENT] -= 150;
		    if (ch->pcdata->classpoints[CP_CURRENT] <= 0)
		    {
			ch->pcdata->classpoints[CP_CURRENT] = 0;
			REMOVE_BIT(ch->pcdata->drow[DROW_POWERS], DROW_GLOBE);
		        stc("You are unable to maintain your globe of darkness.\n\r", ch);
		        act("Darkness fades from the room as $n relaxes.", ch, NULL, NULL, TO_ROOM);
		    }
		}
	    }

	    continue;
 	}

	if (IS_AFFECTED(ch, AFF_CHARM) || IS_EXTRA(ch, EXTRA_ILLUSION) || ch->wizard != NULL) continue;

	/* Examine call for special procedure */
if (IS_COMBAT_PAUSED(ch)) continue;

if (ch->spec_fun != 0)
	{
	    if ((*ch->spec_fun) (ch)) continue;
	    if (ch == NULL)  continue;
	}

	if (ch->position == POS_FIGHTING && IS_SET(ch->act, ACT_SKILLED) && ch->stance[0] < 1)
	{
	    switch (number_range(1, 5))
	    {
		default:
		case 1: do_stance(ch, "viper"); break;
		case 2: do_stance(ch, "crane"); break;
		case 3: do_stance(ch, "mongoose"); break;
		case 4: do_stance(ch, "bull"); break;
		case 5: do_stance(ch, "crab"); break;
	    }
	}

	/* That's all for sleeping / busy monster */
	if (ch->position != POS_STANDING) continue;

	/* Scavenge */
	/*if (IS_SET(ch->act, ACT_SCAVENGER) && ch->in_room->contents != NULL && number_bits(3) == 0)
	{
	    OBJ_DATA *obj;
	    OBJ_DATA *obj_best;
	    long top_num = 0;

	    for (obj = ch->in_room->contents; obj; obj = obj->next_content)
	    {
		if (!CAN_WEAR(obj, ITEM_TAKE)) continue;
		if (obj->value <= 0) continue;
		if (obj->item_type == ITEM_CORPSE_PC || obj->item_type == ITEM_CORPSE_NPC) continue;

		top_num++;
		if (number_range(1, top_num) == 1) obj_best = obj;
	    }

	    if (top_num > 0)
	    {
		obj_from_room(obj_best);
		obj_to_char(obj_best, ch);
		act("$n picks $p up.", ch, obj_best, NULL, TO_ROOM);
		act("You pick $p up.", ch, obj_best, NULL, TO_CHAR);
	    }
	}*/

	/* Wander */
	if (!IS_SET(ch->act, ACT_SENTINEL) && (door = number_bits(5)) <= 5 && (pexit = ch->in_room->exit[door]) != NULL
	 && pexit->to_room != NULL && !IS_SET(pexit->exit_info, EX_CLOSED) && !IS_SET(pexit->to_room->room_flags, ROOM_NO_MOB)
	 && (!IS_SET(ch->act, ACT_STAY_AREA) || pexit->to_room->area == ch->in_room->area))
	    move_char(ch, door);
    }
    return;
}



/*
 * Update the weather.
 */
   void weather_update( void )
   {
      char buf[MAX_STRING_LENGTH];
      DESCRIPTOR_DATA *d;
      CHAR_DATA *ch = NULL;
      long diff, recover;
      bool char_up;
      bool mes_all = FALSE;
   
      strcpy(buf,"Buf - Please inform KaVir.\n\r");
   
      for ( d = descriptor_list; d != NULL; d = d->next )
      {
         if ( d->connected == CON_PLAYING
            && ( ch = d->character) != NULL
            &&  !IS_NPC(ch)
            &&  !IS_EXTRA(ch, EXTRA_OSWITCH)
            &&  !IS_MORE(ch, MORE_HEAD))
         {
            recover = number_range(5,10) + (ch->pcdata->willpower[1] * 0.1);
            if (ch->pcdata->willpower[0] < ch->pcdata->willpower[1])
               ch->pcdata->willpower[0] += recover;
            if (ch->pcdata->willpower[0] > ch->pcdata->willpower[1])
               ch->pcdata->willpower[0] = ch->pcdata->willpower[1];
         }
      }
      if (IS_SET(world_affects, WORLD_ECLIPSE))
      {
         strcpy(buf,"The sun emerges from behind the moon.\n\r");
         REMOVE_BIT(world_affects, WORLD_ECLIPSE);
         mes_all = TRUE;
      }
      else if (IS_SET(world_affects, WORLD_FOG) && number_range(1,5) == 1)
      {
         strcpy(buf,"The fog disperses.\n\r");
         REMOVE_BIT(world_affects, WORLD_FOG);
         mes_all = TRUE;
      }
      else if (IS_SET(world_affects, WORLD_RAIN) && number_range(1,5) == 1)
      {
         strcpy(buf,"It is no longer raining quite so heavily.\n\r");
         REMOVE_BIT(world_affects, WORLD_RAIN);
         mes_all = TRUE;
      }
   
      if (mes_all)
      {
         for ( d = descriptor_list; d != NULL; d = d->next )
         {
            if ( d->connected == CON_PLAYING
               && ( ch = d->character) != NULL
               &&  !IS_NPC(ch)
               &&  !IS_EXTRA(ch, EXTRA_OSWITCH)
               &&  !IS_MORE(ch, MORE_HEAD)
               &&  ch->in_room != NULL
               &&  ch->in_room->sector_type != SECT_INSIDE
               &&  IS_OUTSIDE(ch))
               send_to_char(buf,ch);
         }
      }
      buf[0] = '\0';
      switch ( ++time_info.hour )
      {
         case  4:
            for ( d = descriptor_list; d != NULL; d = d->next )
            {
               if ( d->connected == CON_PLAYING
                  && ( ch = d->character) != NULL
                  &&  !IS_NPC(ch)
                  &&  !IS_EXTRA(ch, EXTRA_OSWITCH)
                  &&  !IS_MORE(ch, MORE_HEAD)
                  &&  IS_VAMPIRE(ch))
                  send_to_char( "You feel weary as the night begins to come to an end.\n\r",ch);
            }
            break;
      
         case  5:
            weather_info.sunlight = SUN_LIGHT;
            strcat( buf, "The day has begun.\n\r" );
            break;
      
         case  6:
            weather_info.sunlight = SUN_RISE;
            strcat( buf, "The sun rises in the east.\n\r" );
            for ( d = descriptor_list; d != NULL; d = d->next )
            {
               if ( d->connected == CON_PLAYING
                  && ( ch = d->character) != NULL
                  &&  !IS_NPC(ch)
                  &&  !IS_EXTRA(ch, EXTRA_OSWITCH)
                  &&  !IS_MORE(ch, MORE_HEAD)
		  &&  IS_MORE(ch, MORE_FAVOUR))
	       {
		  REMOVE_BIT(ch->more, MORE_FAVOUR);
		  stc("You feel weary as the blessing of the almighty fades from you.\n\r", ch);
	       }
            }
            break;
      
         case 19:
            weather_info.sunlight = SUN_SET;
            strcat( buf, "The sun slowly disappears in the west.\n\r" );
            break;
      
         case 20:
            weather_info.sunlight = SUN_DARK;
            strcat( buf, "The night has begun.\n\r" );
            break;
      
         case 24:
            time_info.hour = 0;
            time_info.day++;
            for ( d = descriptor_list; d != NULL; d = d->next )
            {
               char_up = FALSE;
               if ( d->connected == CON_PLAYING
                  && ( ch = d->character) != NULL
                  &&  !IS_NPC(ch)
                  &&  !IS_EXTRA(ch, EXTRA_OSWITCH)
                  &&  !IS_MORE(ch, MORE_HEAD))
               {
                  send_to_char( "You hear a clock in the distance strike midnight.\n\r",ch);
                  if ( IS_VAMPIRE(ch))
                  {
                     if (ch->beast > ch->truebeast && ch->beast > 0 && 
                        ch->beast < 100 && number_range(1,25) == 1)
                     {
			ch->beast--;
                        if (ch->beast == 0)
                        {
                           send_to_char("You have attained Golconda!\n\r", ch);
                        }
                     }
                  }
               }
            }
            break;
      }
   
      if ( time_info.day   >= 35 )
      {
         time_info.day = 0;
         time_info.month++;
      }
   
      if ( time_info.month >= 17 )
      {
         time_info.month = 0;
         time_info.year++;
      }
   
    /*
     * Weather change.
     */
      if ( time_info.month >= 9 && time_info.month <= 16 )
         diff = weather_info.mmhg >  985 ? -2 : 2;
      else
         diff = weather_info.mmhg > 1015 ? -2 : 2;
   
      weather_info.change   += diff * dice(1, 4) + dice(2, 6) - dice(2, 6);
      weather_info.change    = UMAX(weather_info.change, -12);
      weather_info.change    = UMIN(weather_info.change,  12);
   
      weather_info.mmhg += weather_info.change;
      weather_info.mmhg  = UMAX(weather_info.mmhg,  960);
      weather_info.mmhg  = UMIN(weather_info.mmhg, 1040);
   
      switch ( weather_info.sky )
      {
         default: 
            bug( "Weather_update: bad sky %li.", weather_info.sky );
            weather_info.sky = SKY_CLOUDLESS;
            break;
      
         case SKY_CLOUDLESS:
            if ( weather_info.mmhg <  990
               || ( weather_info.mmhg < 1010 && number_bits( 2 ) == 0 ) )
            {
               strcat( buf, "The sky is getting cloudy.\n\r" );
               weather_info.sky = SKY_CLOUDY;
            }
            break;
      
         case SKY_CLOUDY:
            if ( weather_info.mmhg <  970
               || ( weather_info.mmhg <  990 && number_bits( 2 ) == 0 ) )
            {
               strcat( buf, "It starts to rain.\n\r" );
               weather_info.sky = SKY_RAINING;
            }
         
            if ( weather_info.mmhg > 1030 && number_bits( 2 ) == 0 )
            {
               strcat( buf, "The clouds disappear.\n\r" );
               weather_info.sky = SKY_CLOUDLESS;
            }
            break;
      
         case SKY_RAINING:
            if (IS_SET(world_affects, WORLD_RAIN))
            {
               if ( weather_info.mmhg <  970 )
                  weather_info.mmhg += 10;
               else if ( weather_info.mmhg > 1010 )
                  weather_info.mmhg -= 10;
               strcat( buf, "The rain pours down heavily from the sky.\n\r" );
               break;
            }
            if ( weather_info.mmhg <  970 && number_bits( 2 ) == 0 )
            {
               strcat( buf, "Lightning flashes in the sky.\n\r" );
               weather_info.sky = SKY_LIGHTNING;
            }
         
            if ( weather_info.mmhg > 1030
               || ( weather_info.mmhg > 1010 && number_bits( 2 ) == 0 ) )
            {
               strcat( buf, "The rain stopped.\n\r" );
               weather_info.sky = SKY_CLOUDY;
            }
            break;
      
         case SKY_LIGHTNING:
            if ( weather_info.mmhg > 1010
               || ( weather_info.mmhg >  990 && number_bits( 2 ) == 0 ) )
            {
               strcat( buf, "The lightning has stopped.\n\r" );
               weather_info.sky = SKY_RAINING;
               break;
            }
            break;
      }
   
      if ( buf[0] != '\0' )
      {
         for ( d = descriptor_list; d != NULL; d = d->next )
         {
            if ( d->connected == CON_PLAYING
               &&   IS_OUTSIDE(d->character)
               &&   IS_AWAKE(d->character) )
               send_to_char( buf, d->character );
         }
      }
   
      return;
   }



/*
 * Update all chars, including mobs.
 * This function is performance sensitive.
 */
   void char_update( void )
   {   
      CHAR_DATA *ch;
      CHAR_DATA *ch_next;
      CHAR_DATA *ch_save;
      CHAR_DATA *ch_quit;
   
      bool is_obj;
      bool drop_out = FALSE;
      time_t save_time;
   
      save_time	= current_time;
      ch_save	= NULL;
      ch_quit	= NULL;
      for ( ch = char_list; ch != NULL; ch = ch_next )
      {
         AFFECT_DATA *paf;
         AFFECT_DATA *paf_next;
      
         ch_next = ch->next;
         drop_out = FALSE;
      
         if (!IS_NPC(ch) && (IS_MORE(ch,MORE_HEAD) || IS_EXTRA(ch,EXTRA_OSWITCH)))
            is_obj = TRUE;
         else if (!IS_NPC(ch) && ch->pcdata->obj_vnum != 0)
         {is_obj = TRUE;SET_BIT(ch->extra, EXTRA_OSWITCH);}
         else
            is_obj = FALSE;
	/*
	 * Find dude with oldest save time.
	 */
	if (!IS_NPC(ch) && IS_AWA(ch) && ch->save_time < save_time
	 && descriptor_is_in_game(ch->desc))
	{
	    ch_save	= ch;
	    save_time	= ch->save_time;
	}

	if (ch->position <= POS_STUNNED) update_pos(ch);

	if (!IS_NPC(ch))
	{
	    ch->timer++;
	    if ((!IS_IMM(ch) && ch->timer > 20) || ch->timer > 90) ch_quit = ch;
	}

	if (!IS_NPC(ch) && !IS_BUI(ch) && !is_obj)
	{
            if (IS_VAMPIRE(ch))
            {
               if (ch->blood[BLOOD_CURRENT] > 0) ch->blood[BLOOD_CURRENT]--;
               if (ch->hit > 0 && ch->blood[BLOOD_CURRENT] < 1)
               {
                  send_to_char( "You are DYING from lack of blood!\n\r", ch);
                  act( "$n gets a hungry look in $s eyes.", ch, NULL, NULL, TO_ROOM);
                  if (ch->hit > 0) ch->hit -= number_range(2, 5);
                  update_pos(ch);
                  if (number_percent() <= ch->beast && ch->beast > 0) vamp_rage(ch);
                  if (!IS_VAMPAFF(ch, VAM_FANGS)) do_fangs(ch,"");
               }
               else if (ch->hit > 0 && ch->blood[BLOOD_CURRENT] <= 20)
               {
                  send_to_char( "You crave blood.\n\r", ch );
                  if (number_range(1,1000) <= ch->beast && ch->beast > 0) vamp_rage(ch);
                  if (number_percent() > (ch->blood[BLOOD_CURRENT]+75)
                     && !IS_VAMPAFF(ch, VAM_FANGS)) do_fangs(ch,"");
               }
            }
         }
      
         for ( paf = ch->affected; paf != NULL; paf = paf_next )
         {
            paf_next	= paf->next;
            if ( paf->duration > 0 )
               paf->duration--;
            else if ( paf->duration < 0 )
               ;
            else
            {
               if ( paf_next == NULL
                  ||   paf_next->type != paf->type
                  ||   paf_next->duration > 0 )
               {
                  if ( paf->type > 0 && skill_table[paf->type].msg_off && !is_obj)
                  {
                     send_to_char( skill_table[paf->type].msg_off, ch );
                     send_to_char( "\n\r", ch );
                  }
               }
            
               affect_remove( ch, paf );
            }
         }
      
      /*
      * Careful with the damages here,
      *   MUST NOT refer to ch after damage taken,
      *   as it may be lethal damage (on NPC).
      */
         if ( IS_AFFECTED(ch, AFF_FLAMING) && !is_obj && !drop_out && ch->in_room != NULL )
         {
            long dam = number_range(250,500);
         
            if (IS_NPC(ch))
            {
               act( "$n's flesh burns and crisps.", ch, NULL, NULL, TO_ROOM );
               send_to_char( "Your flesh burns and crisps.\n\r", ch );
               if (IS_VAMPIRE(ch) && get_disc(ch,DISC_FORTITUDE) > 0)
               {
                  long per_red = 0;
                  switch( get_disc(ch,DISC_FORTITUDE) )
                  {
                     default: per_red = 0;                   
                        break;
                     case 1:  per_red = number_range(5,10);  
                        break;
                     case 2:  per_red = number_range(10,20); 
                        break;
                     case 3:  per_red = number_range(15,30); 
                        break;
                     case 4:  per_red = number_range(20,40); 
                        break;
                     case 5:  per_red = number_range(25,50); 
                        break;
                     case 6:  per_red = number_range(30,60); 
                        break;
                     case 7:  per_red = number_range(35,70); 
                        break;
                     case 8:  per_red = number_range(40,80); 
                        break;
                     case 9:  per_red = number_range(45,90); 
                        break;
                     case 10: per_red = number_range(50,100);
                        break;
                  }
                  ch->hit += (per_red * dam * 0.01);
               }
               ch->hit -= dam;
               update_pos(ch);
               if (ch == NULL) 
                  continue;
               if (ch->position == POS_DEAD)
               {
                  killperson(ch, ch);
                  drop_out = TRUE;
                  continue;
               }
            }
         }
      
         if ( IS_AFFECTED(ch, AFF_POISON) && !is_obj && !drop_out )
         {
            act( "$n shivers and suffers.", ch, NULL, NULL, TO_ROOM );
            send_to_char( "You shiver and suffer.\n\r", ch );
            if (ch == NULL || ch->position == POS_DEAD) 
               continue;
         }
         else if ( ch->position == POS_INCAP && !is_obj && !drop_out )
         {
            if (IS_AWA(ch))
               ch->hit = ch->hit + number_range(2,4);
            else
               ch->hit = ch->hit - number_range(1,2);
            update_pos( ch );
            if (ch->position > POS_INCAP)
            {
               act( "$n's wounds stop bleeding and seal up.", ch, NULL, NULL, TO_ROOM );
               send_to_char( "Your wounds stop bleeding and seal up.\n\r", ch );
            }
            if (ch->position > POS_STUNNED)
            {
               act( "$n clambers back to $s feet.", ch, NULL, NULL, TO_ROOM );
               send_to_char( "You clamber back to your feet.\n\r", ch );
            }
         }
         else if ( ch->position == POS_MORTAL && !is_obj && !drop_out )
         {
            drop_out = FALSE;
            if (IS_AWA(ch))
               ch->hit = ch->hit + number_range(2,4);
            else
            {
               ch->hit = ch->hit - number_range(1,2);
               if ((!IS_NPC(ch) && ch->hit<-10) || (IS_NPC(ch) && ch->hit<0))
               {
               /* kavirp */
                  killperson(ch, ch);
                  continue;
               }
            }
            if (!drop_out)
            {
               update_pos( ch );
               if (ch->position == POS_INCAP)
               {
                  act( "$n's wounds begin to close, and $s bones pop back into place.", ch, NULL, NULL, TO_ROOM );
                  send_to_char( "Your wounds begin to close, and your bones pop back into place.\n\r", ch );
               }
            }
         }
         else if ( ch->position == POS_DEAD && !is_obj && !drop_out )
         {
         /*
         update_pos(ch);
         */
            if (!IS_NPC(ch)) 
            {
               killperson(ch, ch);
               continue;
            }
         }
         drop_out = FALSE;
      }
   
    /*
     * Autosave and autoquit.
     * Check that these chars still exist.
     */
    if (ch_save != NULL || ch_quit != NULL)
    {
	for (ch = char_list; ch != NULL; ch = ch_next)
	{
	    ch_next = ch->next;
	    if (ch == ch_save) do_autosave(ch, "");
	    if (ch == ch_quit) do_quit(ch, "");
	}
    }
    return;
}



/*
 * Update all objs.
 * This function is performance sensitive.
 */
void obj_update( void )
{   
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    for (obj = object_list; obj != NULL; obj = obj_next)
    {
	CHAR_DATA *rch;
	char *message;

	obj_next = obj->next;

	if (obj->timer <= 0 || --obj->timer > 0) continue;

	if (IS_SET(obj->extra_flags, ITEM_VANISH)) message = "$p slowly fades away to nothing.";
	else switch (obj->item_type)
	{
	    default:			message = "$p crumbles into dust."; break;
	    case ITEM_FOUNTAIN:		message = "$p dries up."; break;
	    case ITEM_WALL:		message = "$p sinks back into the ground."; break;
	    case ITEM_CORPSE_NPC:
	    case ITEM_CORPSE_PC:	message = "$p decays into dust."; break;
	}

	if (obj->carried_by != NULL)
	    act(message, obj->carried_by, obj, NULL, TO_CHAR);
	else if (obj->in_room != NULL && (rch = obj->in_room->people) != NULL)
	{
	    act(message, rch, obj, NULL, TO_ROOM);
	    act(message, rch, obj, NULL, TO_CHAR);
	}
	extract_obj(obj);
    }
    return;
}



/*
 * Aggress.
 *
 * for each mortal PC
 *     for each mob in room
 *         aggress on some random PC
 *
 * This function takes 25% to 35% of ALL Merc cpu time.
 * Unfortunately, checking on each PC move is too tricky,
 *   because we don't want the mob to just attack the first PC
 *   who leads the party into the room.
 *
 * -- Furey
 */
   void aggr_update( void )
   {
      CHAR_DATA *wch;
      CHAR_DATA *wch_next;
      CHAR_DATA *ch;
      CHAR_DATA *ch_next;
      CHAR_DATA *vch;
      CHAR_DATA *vch_next;
      CHAR_DATA *victim = NULL;
   
      OBJ_DATA *obj = NULL;
      OBJ_DATA *chobj = NULL;
      ROOM_INDEX_DATA *objroom = NULL;
      DESCRIPTOR_DATA *d;
   
      for ( d = descriptor_list; d != NULL; d = d->next )
      {
      /*
      if ( d->connected == CON_PLAYING
      && ( ch = d->character ) != NULL
      &&   !IS_NPC(ch)
      &&   ch->pcdata->stage[1] > 0
      && ( victim = ch->pcdata->partner ) != NULL
      &&   !IS_NPC(victim)
      &&   ch->in_room != NULL
      &&   victim->in_room != NULL
      &&   victim->in_room != ch->in_room )
      {
       ch->pcdata->stage[1] = 0;
       victim->pcdata->stage[1] = 0;
      }
      */
         if ( d->connected == CON_PLAYING
            && ( ch = d->character ) != NULL
            &&   !IS_NPC(ch)
            &&   ch->pcdata != NULL
            && ( obj = ch->pcdata->chobj ) != NULL )
         {
            if (obj->in_room != NULL)
               objroom = obj->in_room;
            else if (obj->in_obj != NULL)
               objroom = get_room_index(ROOM_VNUM_IN_OBJECT);
            else if (obj->carried_by != NULL)
            {
               if (obj->carried_by != ch && obj->carried_by->in_room != NULL)
                  objroom = obj->carried_by->in_room;
               else 
                  continue;
            }
            else 
               continue;
            if (ch->in_room != objroom && objroom != NULL)
            {
               char_from_room(ch);
               char_to_room(ch,objroom);
               do_look(ch,"auto");
            }
         }
         else if ( d->connected == CON_PLAYING
                 && ( ch = d->character ) != NULL
                 &&   !IS_NPC(ch)
                 &&   ch->pcdata != NULL
                 &&   (IS_MORE(ch,MORE_HEAD) || IS_EXTRA(ch,EXTRA_OSWITCH) || ch->pcdata->obj_vnum != 0) )
         {
            if (ch->pcdata->obj_vnum != 0)
            {
               bind_char(ch);
               continue;
            }
         
            if (IS_MORE(ch,MORE_HEAD))
            {
               REMOVE_BIT(ch->more, MORE_HEAD);
               send_to_char("You are able to regain a body.\n\r",ch);
               ch->position = POS_RESTING;
               ch->hit = 1;
            } 
            else {
               send_to_char("You return to your body.\n\r",ch);
               REMOVE_BIT(ch->extra,EXTRA_OSWITCH);}
            REMOVE_BIT(ch->affected_by,AFF_POLYMORPH);
            free_string(ch->morph);
            ch->morph = str_dup("");
            char_from_room(ch);
            char_to_room(ch,get_room_index(ROOM_VNUM_ALTAR));
            if ( ( chobj = ch->pcdata->chobj ) != NULL )
               chobj->chobj = NULL;
            ch->pcdata->chobj = NULL;
            do_look(ch,"auto");
         }
         continue;
      }
   
      for ( wch = char_list; wch != NULL; wch = wch_next )
      {
         wch_next = wch->next;
         if ( IS_NPC(wch)
            || ( wch->desc != NULL && wch->desc->connected != CON_PLAYING )
            ||   wch->position <= POS_STUNNED
            ||   !IS_BUI(wch)
            ||   wch->pcdata == NULL
            || ( ( chobj = wch->pcdata->chobj ) != NULL )
            ||   wch->in_room == NULL )
            continue;
      
         for ( ch = wch->in_room->people; ch != NULL; ch = ch_next )
         {
            long count;
         
            ch_next	= ch->next_in_room;
         
            if ( !IS_NPC(ch)
               ||   IS_COMBAT_PAUSED(ch)
               ||   !IS_SET(ch->act, ACT_AGGRESSIVE)
               ||   is_safe(ch, wch)
               ||   ch->in_room != wch->in_room
               ||   ch->fighting != NULL
               ||   IS_AFFECTED(ch, AFF_CHARM)
               ||   !IS_AWAKE(ch)
               ||   !can_see( ch, wch ) )
               continue;
         
         /*
         * Ok we have a 'wch' player character and a 'ch' npc aggressor.
         * Now make the aggressor fight a RANDOM pc victim in the room,
         *   giving each 'vch' an equal chance of selection.
         */
            count	= 0;
            victim	= NULL;
            for ( vch = wch->in_room->people; vch != NULL; vch = vch_next )
            {
               vch_next = vch->next_in_room;
            
               if ( !IS_NPC(vch)
                  && ( vch->desc == NULL || IS_PLAYING(vch->desc) )
                  &&   is_safe(ch, vch)
                  &&   vch->pcdata != NULL
                  && ( ( chobj = vch->pcdata->chobj ) == NULL )
                  &&   !IS_BUI(vch)
                  &&   vch->position > POS_STUNNED
                  &&   can_see( ch, vch ) )
               {
                  if ( number_range( 0, count ) == 0 )
                     victim = vch;
                  count++;
               }
            }
         
            if ( victim == NULL )
            {
            /*
            bug( "Aggr_update: null victim attempt by mob %li.", ch->pIndexData->vnum );
            */
               continue;
            }
         
            multi_hit( ch, victim );
         }
      }
   
      return;
   }


/*
 * For traps that deal continual damage, or something.  I could have added it elsewhere,
 *  but it was easy enough to put here by itself for easy access - Akuma
 */
void trap_update( void )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    TRAP_DATA *trap;

    for(vch = char_list; vch != NULL; vch = vch_next)
    {
	vch_next = vch->next;

	if (IS_BUI(vch) || IS_NPC(vch)) continue;
	if ((trap = vch->in_room->trap) == NULL) continue;
	if (!IS_SET(trap->trapbits, TRAP_REPEAT)) continue;
	if (vch == NULL || vch->position == POS_DEAD) continue;

	trigger_trap(vch, trap);
    }
}


/*
 * Handle all kinds of updates.
 * Called once per pulse from game loop.
 */
void update_handler( void )
{
    static long pulse_gain_exp;
    static long pulse_area;
    static long pulse_mobile;
    static long pulse_violence;
    static long pulse_point;

    if (--pulse_gain_exp <= 0)
    {
	char buf[MSL];
	DESCRIPTOR_DATA *d;
	CHAR_DATA *ch = NULL;
	pulse_gain_exp = PULSE_PER_SECOND;

	if (exp_info.time >= 1)
	{
	    exp_info.time--;
	    if (exp_info.time <= 0)
	    {
		if (IS_SET(exp_info.bit, EXP_DEGRADE))
		{
		    exp_info.exp -= 25;
		    if (exp_info.exp <= 100)
		    {
			exp_info.exp = 100;
			exp_info.time = 0;
			exp_info.bit = 0;
			sprintf(buf, "Degrading Expmod has run out.");
		    }
		    else
		    {
			sprintf(buf, "Expmod has degraded.");
			exp_info.time = 15 * 60;
		    }
		    do_info(NULL, buf);
		}
		else
		{
		    if (IS_SET(exp_info.bit, EXP_RANDOM)) sprintf(buf, "Random%% Expmod has run out.");
		    else sprintf(buf, "%li%% Expmod has ran out.", exp_info.exp);
		    do_info(NULL, buf);
		    exp_info.time = 0;
		    exp_info.exp = 100;
		    exp_info.bit = 0;
		}
	    }
	}

	for (d = descriptor_list; d != NULL; d = d->next)
	{
	    if (d->connected == CON_PLAYING && (ch = d->character) != NULL && !IS_NPC(ch))
	    {
		ch->save_timer++;
		if (ch->save_timer >= 300)
		{
		    ch->save_timer = 0;
		    do_autosave(ch, "");
		}

		if (ch->fight_timer > 0)
		{
		    if (IS_BUI(ch)) ch->fight_timer = 0;
		    else ch->fight_timer--;
		}

		if (ch->pcdata->pain > 0)
		{
		    ch->pcdata->pain--;
		    if (ch->pcdata->pain <= 0)
		    {
			stc("Your body no longer screams out in pain.\n\r", ch);
			return;
		    }
		} 

		if (ch->pcdata->silence > 0)
		{
		    ch->pcdata->silence--;
		    if (ch->pcdata->silence <= 0)
		    {
			if (IS_MORE(ch, MORE_SILENCE))
			{
			    act("The room is no longer silent.", ch,NULL, NULL, TO_CHAR);
			    act("The room is no longer silent.", ch, NULL, NULL, TO_ROOM);
			    REMOVE_BIT(ch->in_room->added_flags, ROOM2_SILENCE); 
			    REMOVE_BIT(ch->more, MORE_SILENCE);
			    ch->pcdata->silence = 60;
			}
			else stc("Your ability to call the Silence of Death has returned.\n\r", ch);
		    }
		}

		if (ch->pcdata->necromancy > 0)
		{
		    ch->pcdata->necromancy--;
		    if (ch->pcdata->necromancy <= 0)
			stc("Your powers of Necromancy have replenished themselves.\n\r", ch);
		}

		if (ch->pcdata->weather > 0)
		{
		    ch->pcdata->weather--;
		    if (ch->pcdata->weather <= 0)
			stc("Your powers of weather control have replenished themselves.\n\r", ch);
		}

		if (ch->pcdata->celerity > 0)
		{
		    ch->pcdata->celerity--;
		    if (ch->pcdata->celerity <= 0)
		    {
			if (IS_WILLPOWER(ch, RES_CELERITY) && ch->position == POS_FIGHTING)
			{
			    if (ch->blood[BLOOD_CURRENT] < 10)
			    {
				stc("Not enough blood to renew Celerity.\n\r", ch);
				stc("Your movements slow down.\n\r", ch);
				act("$n is no longer moving with supernatural speed.", ch, NULL, NULL, TO_ROOM);
			    }
			    else
			    {
				ch->blood[BLOOD_CURRENT] -= 10;
				ch->pcdata->celerity += 60;
				stc("You burn blood to continue using your Celerity.\n\r", ch);
			    }
			}
			else
			{
			    stc("Your movements slow down.\n\r", ch);
			    act("$n is no longer moving with supernatural speed.", ch, NULL, NULL, TO_ROOM);
			}
		    }
		}

		if (ch->pcdata->obeah > 0)
		{
		    ch->pcdata->obeah--;
		    if (ch->pcdata->obeah <= 0)
			stc("Your healing powers have returned.\n\r", ch);
		}

		if (ch->pcdata->darkness > 0)
		{
		    ch->pcdata->darkness--;
		    if (ch->pcdata->darkness <= 0)
		    {
			if (IS_MORE(ch, MORE_DARKNESS))
			{
			    act("The darkness in the room gradually fades away to nothing.", ch, NULL, NULL, TO_CHAR);
			    act("The darkness in the room gradually fades away to nothing.", ch, NULL, NULL, TO_ROOM);
			    REMOVE_BIT(ch->in_room->added_flags, ROOM2_DARKNESS); 
			    REMOVE_BIT(ch->more, MORE_DARKNESS);
			    ch->pcdata->darkness = 60;
			}
			else stc("Your ability to call the Shroud has returned.\n\r",ch);
		    }
		}

		if (ch->pcdata->majesty > 0)
		{
		    ch->pcdata->majesty--;
		    if (ch->pcdata->majesty <= 0)
		    {
			if (IS_WEREWOLF(ch)) stc("You can now use your Icy Chill of Despair again if you wish.\n\r", ch);
			if (IS_VAMPIRE(ch)) stc("You can now raise your Majesty again if you wish.\n\r", ch);
		    }
		}

		if (ch->doublecast_timer > 0)
		{
		    ch->doublecast_timer--;
		    if (ch->doublecast_timer <= 0)
			stc("You have regained your ability to use doublecast.\n\r", ch);
		}

		if (ch->polymorph_timer > 0)
 		{
		    if (!IS_POLYAFF(ch, POLY_PIG)) ch->polymorph_timer = 0;
		    else
		    {
			ch->polymorph_timer--;

			if (ch->polymorph_timer <= 0)
			{
			    stc("Your body shifts back into its natural form.\n\r", ch);
			    act("$n's body moves and shifts back into its natural form.", ch, NULL, NULL, TO_ROOM);
			    free_string(ch->morph);
			    ch->morph = str_dup("");
			    REMOVE_BIT(ch->polyaff, POLY_PIG);
			    REMOVE_BIT(ch->affected_by, AFF_POLYMORPH);
			}
		    }
		}

		if (ch->ephemera_timer > 0)
		{
		    ch->ephemera_timer--;
		    if (ch->ephemera_timer <= 0)
		    {
			stc("Fate is no longer twisted against you.\n\r", ch);
			act("Fate is no longer twisted against $n.", ch, NULL, NULL, TO_ROOM);
			REMOVE_BIT(ch->xtra, XTRA_MUTATE_EPHEMERA);
		    }
		}

		if (ch->better_body_timer > 0)
		{
		    ch->better_body_timer--;
		    if (ch->better_body_timer <= 0)
		    {
			stc("The effects of better body wear off.\n\r", ch);
			if (IS_XTRA(ch, XTRA_BETTER_STR)) REMOVE_BIT(ch->xtra, XTRA_BETTER_STR);
			if (IS_XTRA(ch, XTRA_BETTER_DEX)) REMOVE_BIT(ch->xtra, XTRA_BETTER_DEX);
			if (IS_XTRA(ch, XTRA_BETTER_CON)) REMOVE_BIT(ch->xtra, XTRA_BETTER_CON);
			if (IS_XTRA(ch, XTRA_BETTER_INT)) REMOVE_BIT(ch->xtra, XTRA_BETTER_INT);
			if (IS_XTRA(ch, XTRA_BETTER_WIS)) REMOVE_BIT(ch->xtra, XTRA_BETTER_WIS);
		    }
		}

		if (ch->slow_timer > 0)
		{
		    ch->slow_timer--;
		    if (ch->slow_timer <= 0)
		    {
			REMOVE_BIT(ch->xtra, XTRA_SLOW);
			act("You are no longer moving so slowly.", ch, NULL, NULL, TO_CHAR);
			act("$n is no longer moving so slowly.", ch, NULL, NULL, TO_ROOM);
		    }
		}

		if (ch->atoningwinds_timer > 0)
		{
		    ch->atoningwinds_timer--;
		    if (ch->atoningwinds_timer <= 0)
		    {
			REMOVE_BIT(ch->xtra, XTRA_ATONING);
			act("You are no longer affected by atoning winds.", ch, NULL, NULL, TO_CHAR);
			act("$n is no longer affected by atoning winds.", ch, NULL, NULL, TO_ROOM);
		    }
		}

		if (ch->liquify_timer > 0)
		{
		    ch->liquify_timer--;
		    if (ch->liquify_timer <= 0)
		    {
			REMOVE_BIT(ch->xtra, XTRA_LIQUIFY);
			act("You are no longer affected by liquify earth", ch, NULL, NULL, TO_CHAR);
			act("$n is no longer affected by liquify earth.", ch,NULL, NULL, TO_ROOM);
		    }
		}

		if (ch->enrage_timer > 0)
		{
		    ch->enrage_timer--;
		    if (ch->enrage_timer <= 0)
			stc("Your snarls die away as you calm yourself\n\r", ch);
		}

		if (ch->pcdata->resist[WILL_ANIMALISM] > 0)
		{
		    ch->pcdata->resist[WILL_ANIMALISM]--;
		    if (ch->pcdata->resist[WILL_ANIMALISM] <= 0)
		    {
			stc("You are no longer immune to Beast Within or Song of Serenity.\n\r", ch);
			if (!IS_WILLPOWER(ch, RES_TOGGLE)) REMOVE_BIT(ch->pcdata->resist[0], RES_ANIMALISM);
		    }
		}

		if (ch->pcdata->resist[WILL_AUSPEX] > 0)
		{
		    ch->pcdata->resist[WILL_AUSPEX]--;
		    if (ch->pcdata->resist[WILL_AUSPEX] <= 0)
		    {
			stc("You are no longer immune to having your mind Unveiled.\n\r", ch);
			if (!IS_WILLPOWER(ch, RES_TOGGLE)) REMOVE_BIT(ch->pcdata->resist[0], RES_AUSPEX);
		    }
		}

		if (ch->pcdata->resist[WILL_DOMINATE] > 0)
		{
		    ch->pcdata->resist[WILL_DOMINATE]--;
		    if (ch->pcdata->resist[WILL_DOMINATE] <= 0)
		    {
			stc("You are no longer immune to being Dominated.\n\r", ch);
			if (!IS_WILLPOWER(ch, RES_TOGGLE)) REMOVE_BIT(ch->pcdata->resist[0], RES_DOMINATE);
		    }
		}

		if (ch->pcdata->resist[WILL_PRESENCE] > 0)
		{
		    ch->pcdata->resist[WILL_PRESENCE]--;
		    if (ch->pcdata->resist[WILL_PRESENCE] <= 0)
		    {
			stc("You are no longer immune to Dread Gaze and Entrancement.\n\r", ch);
			if (!IS_WILLPOWER(ch, RES_TOGGLE)) REMOVE_BIT(ch->pcdata->resist[0], RES_PRESENCE);
		    }
		}
	    }
	}
    }

    if (--pulse_area <= 0)
    {
	pulse_area = PULSE_AREA;
	area_update();
	trap_update();
    }

    if (--pulse_mobile <= 0)
    {
	pulse_mobile = PULSE_MOBILE;
	mobile_update();
	obj_update();
    }

    if (--pulse_violence <= 0)
    {
	pulse_violence = PULSE_VIOLENCE;
	violence_update();
	aggr_update();
    }

    if (--pulse_point <= 0)
    {
	pulse_point = PULSE_TICK;
	weather_update();
	char_update();
    }

    tail_chain();
    return;
}

void paradox( CHAR_DATA *ch )
{
    char buf[MSL];

    if (IS_NPC(ch)) return;

    stc("You feel the air charge with electricity.\n\r", ch);
    stc("The paradox has come for your soul!\n\r", ch);
    sprintf(buf, "#r%s#n has been electrified by a paradox.", ch->name);
    do_info(ch, buf);

    ch->hit = -10;
    ch->pdeath++;
    update_pos(ch);
    do_escape(ch, "");
    do_visible(ch, "");
    if (IS_AFFECTED(ch, AFF_SHADOWPLANE)) REMOVE_BIT(ch->affected_by, AFF_SHADOWPLANE);
    if (IS_AFFECTED(ch, AFF_DEEPUMBRA)) REMOVE_BIT(ch->affected_by, AFF_DEEPUMBRA);
    if (get_drow(ch, DROW_GLOBE)) REMOVE_BIT(ch->pcdata->drow[DROW_POWERS], DROW_GLOBE);
    SET_BIT(ch->extra, TIED_UP);
    SET_BIT(ch->extra, GAGGED);
    SET_BIT(ch->extra, BLINDFOLDED);
    WAIT_STATE(ch, 60);

    return;
}
