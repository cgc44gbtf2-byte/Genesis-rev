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
 *  God Wars Mud originally written by KaVir aka Richard Woolcock.         *
 *  Changes done to the code done by Sage aka Walter Howard, this mud is   *
 *  for the public, however if you run this code it means you agree        *
 *  to the license.low, license.gw, and license.merc have fun. :)          *
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

void do_principles( CHAR_DATA *ch, char *argument )
{
    char arg1[MIL];
    char arg2[MIL];
    char buf[MSL];

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (!IS_NINJA(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (arg1[0] == '\0')
    {
	sprintf(buf, "Principles: Sora (%li), Chikyu (%li), Ningenno (%li).\n\r",
	 get_ninja(ch, NINJA_SORA), get_ninja(ch, NINJA_CHIKYU), get_ninja(ch, NINJA_NINGENNO));
	stc(buf, ch);
	return;
    }

    if (arg2[0] == '\0')
    {
	if (!str_cmp(arg1, "sora"))
	{
	    stc("Sora - *Heaven*.  Each level adds 25 to max Michi.\n\r\n\r", ch);
	    if (get_ninja(ch, NINJA_SORA) < 1) 
		stc("You have none of the Sora principles.\n\r", ch);
	    if (get_ninja(ch, NINJA_SORA) >= 1)
		stc("1 : Michi       - Improves combat abilities. (michi) <100 ki>\n\r", ch);
	    if (get_ninja(ch, NINJA_SORA) >= 4)
                stc("4 : Kanzuite    - Raises your awareness. (heighten/readaura/scry)\n\r", ch);
	    if (get_ninja(ch, NINJA_SORA) >= 6)
		stc("6 : Kakusu      - Cloaks you in shadow. (kakusu) <20 ki, movement drains>\n\r", ch);
	    if (get_ninja(ch, NINJA_SORA) >= 8)
		stc("8 : Bomuzite    - Renders opponents unconcious. (bomuzite) <20 ki>\n\r", ch);
            if (get_ninja(ch, NINJA_SORA) >= 10)
		stc("10: Mizotamouyo - Allows quick transportation to distant places. (mizotamouyo)\r\n", ch);
	}
	else if (!str_cmp(arg1, "chikyu"))
	{
	    stc("Chikyu - *Earth*.  Each level adds damage/parry.\n\r\n\r", ch);
	    if (get_ninja(ch, NINJA_CHIKYU) < 1) 
		stc("You have none of the Chikyu principles.\n\r", ch);
	    if (get_ninja(ch, NINJA_CHIKYU) >= 1) 
		stc("1 : Mienaku     - \"Ninja VANISH!\" - TMNT. (mienaku) <20 ki>\n\r", ch);
	    if (get_ninja(ch, NINJA_CHIKYU) >= 4)
		stc("4 : Orimasu     - Increased focus improves healing rates. (orimasu) <50 ki>\n\r", ch);
	    if (get_ninja(ch, NINJA_CHIKYU) >= 6)
		stc("6 : Gunaya      - Entangles foes in sticky strands of webbing. (web) <10 ki>\n\r", ch);
            if (get_ninja(ch, NINJA_CHIKYU) >= 8)
		stc("8 : Tsumazuku   - Leaves opponents unable to respond to your attack. (tsumazuku)\n\r", ch);
	    if (get_ninja(ch, NINJA_CHIKYU) >= 10)
		stc("10: HaraKiri    - Increases power through pain. (harakiri)\n\r", ch);
	}
	else if (!str_cmp(arg1, "ningenno"))
	{
	    stc("Ningenno - *Humanity*.  Each level adds speed/dodge.\n\r\n\r", ch);
	    if (get_ninja(ch, NINJA_NINGENNO) < 1)
		stc("You have none of the Ningenno principles.\n\r", ch);
	    if (get_ninja(ch, NINJA_NINGENNO) >= 1)
		stc("1 : Tsume       - Ninja claw. (tsume)\n\r", ch);
	    if (get_ninja(ch, NINJA_NINGENNO) >= 4)
		stc("4 : Shuurai     - Lightning quick piercing attacks. (shuurai)\n\r", ch);
	    if (get_ninja(ch, NINJA_NINGENNO) >= 6)
		stc("6 : Shuriken    - Throws small blades during combat. (automatic)\n\r", ch);
	    if (get_ninja(ch, NINJA_NINGENNO) >= 8)
		stc("8 : Tsuin Ittou - Allows the use of two weapons. (automatic)\n\r", ch);
	    if (get_ninja(ch, NINJA_NINGENNO) >= 10)
		stc("10: Hakunetsu   - Delivers a powerful piercing attack to an opponents vitals. (hakunetsu)\n\r", ch);
	}
	else do_principles(ch, "");
	return;
    }
    else if (!str_cmp(arg2, "improve"))
    {
	long improve;
	long cost;

	if (!str_cmp(arg1, "sora")) improve = NINJA_SORA;
	else if (!str_cmp(arg1, "chikyu")) improve = NINJA_CHIKYU;
	else if (!str_cmp(arg1, "ningenno")) improve = NINJA_NINGENNO;
	else
	{
	    stc("Principles: Sora, Chikyu, Ningenno.\n\r", ch);
	    return;
	}

	cost = (get_ninja(ch, improve) + 1) * 10;
	arg1[0] = UPPER(arg1[0]);

	if (get_ninja(ch, improve) >= 10)
	{
	    sprintf(buf, "You have already gained all the powers of the %s principle.\n\r", arg1);
	    stc(buf, ch);
	    return;
	}
	if (cost > ch->primal)
	{
	    sprintf(buf, "It costs you %li primal to improve your %s principle.\n\r", cost, arg1);
	    stc(buf, ch);
	    return;
	}

	ch->pcdata->ninja[improve]++;
	ch->primal -= cost;
	sprintf(buf, "You improve your ability in the %s principle.\n\r", arg1);
	stc(buf, ch);
    }
    else stc("To improve, type: prin <sora/chikyu/ningenno> improve.\n\r", ch);
    return;
}


void do_noutcast( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char buf[MSL];
    char arg[MIL];

    argument = one_argument(argument, arg);

    if (!IS_NINJA(ch) || ch->vampgen > 2)
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (arg[0] == '\0' || (victim = get_char_room(ch, arg)) == NULL)
    {
	stc("They aren't here.\n\r", ch);
	return;
    }

    if (!IS_NINJA(victim) || ch == victim || str_cmp(victim->clan, ch->clan))
    {
	stc("You may only outcast other ninja in your clan.\n\r", ch);
	return;
    }

    victim->vampgen = 4;
    free_string(victim->clan);
    victim->clan = str_dup("");
    do_save(victim, "");
    sprintf(buf, "You have outcast %s from the %s clan!\n\r", victim->name, ch->clan);
    stc(buf, ch);
    sprintf(buf, "%s has outcast you from the %s clan!\n\r", ch->name, ch->clan);
    stc(buf, victim);

    return;
}

void do_humble( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MIL];

    argument = one_argument(argument, arg);

    if (!IS_NINJA(ch) || ch->vampgen > 3)
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (arg[0] == '\0' || (victim = get_char_room(ch, arg)) == NULL)
    {
	stc("Discipline whom?\n\r", ch);
	return;
    }

    if (!IS_NINJA(victim) || ch == victim || strlen(victim->clan) != 0 || !IS_EXTRA(victim, EXTRA_VAMPIRE))
    {
	stc("You failed.\n\r", ch);
	return;
    }

    act("You allow $N the honor of being inducted into your clan.", ch, NULL, victim, TO_CHAR);
    act("$n allows $N the honor of being inducted into $s clan.", ch, NULL, victim, TO_NOTVICT);
    act("$n allows you the honor of being inducted into $s clan.", ch, NULL, victim, TO_VICT);

    free_string(victim->clan);
    victim->clan = str_dup(ch->clan);
    victim->vampgen = 4;
    do_autosave(victim, "");

    return;
}

void do_promote( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char      buf[MAX_STRING_LENGTH];
    char      arg [MAX_INPUT_LENGTH];
    argument = one_argument( argument, arg );
    if (IS_NPC(ch)) return;
    if (!IS_NINJA(ch) || ch->vampgen > 2) stc("Huh?\n\r",ch);
    else if (arg[0] == '\0') stc("Promote whom?\n\r",ch);
    else if ((victim = get_char_room(ch,arg)) == NULL) stc("They aren't here.\n\r",ch);
    else if (IS_NPC(victim)) stc("Not on NPC's.\n\r",ch);
    else if (!IS_NINJA(victim)) stc("They aren't a ninja.\n\r",ch);
    else if (ch == victim) stc( "You cannot promote yourself.\n\r",ch);
    else if (str_cmp(victim->clan, ch->clan)) stc("They are NOT in your clan!.\n\r",ch);
    else
    {
	victim->vampgen = 3;
	do_save(victim, "");
	sprintf(buf, "You have promoted %s to Chunin!\n\r",victim->name);
	stc(buf, ch);
	sprintf(buf, "You have been promoted to Chunin of %s!\n\r",ch->clan);
	stc(buf, victim);
    }
    return;
}

void do_demote( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char      buf[MAX_STRING_LENGTH];
    char      arg [MAX_INPUT_LENGTH];
    argument = one_argument( argument, arg );
    if (IS_NPC(ch)) return;
    if (!IS_NINJA(ch) || ch->vampgen > 2) send_to_char("Huh?\n\r",ch);
    else if (arg[0] == '\0') send_to_char("Demote whom?\n\r",ch);
    else if ((victim = get_char_room(ch,arg)) == NULL) send_to_char("They aren't here.\n\r",ch);
    else if (IS_NPC(victim)) send_to_char("Not on NPC's.\n\r",ch);
    else if (!IS_NINJA(victim)) send_to_char("They aren't a ninja.\n\r",ch);
    else if (ch == victim) send_to_char("You cannot demote yourself.\n\r",ch);
    else if (str_cmp(victim->clan, ch->clan)) send_to_char("They are NOT in your clan!.\n\r",ch);
    else
    {
	victim->vampgen = 4;
	do_save(victim, "");
	sprintf(buf, "You have demoted %s to Genin.\n\r",victim->name);
	send_to_char(buf, ch);
	sprintf(buf, "You have been demoted to Genin of %s.\n\r",ch->clan);
	send_to_char(buf, victim);
    }
    return;
}


void do_michi( CHAR_DATA *ch, char *argument )
{
    if (!IS_NINJA(ch) || get_ninja(ch, NINJA_SORA) < 1)
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (ch->pcdata->ninja[NINJA_KI] < 100)
    {
	stc("But you don't have enough Ki to perform the michi.\n\r", ch);
	return;
    }

    stc("You are gifted positive energy while performing the michi.\n\r", ch);
    act("$n is gifted positives energy while performing the michi.", ch, NULL, NULL, TO_ROOM);

    ch->pcdata->wolf = get_ninja(ch, NINJA_SORA) * 25;
    ch->pcdata->ninja[NINJA_KI] -= 100;

    if (ch->position == POS_FIGHTING) WAIT_STATE(ch, 12);

    return;
}

void do_kakusu( CHAR_DATA *ch, char *argument )
{
    if (!IS_NINJA(ch) || get_ninja(ch, NINJA_SORA) < 6)
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (IS_AFFECTED(ch, AFF_KAKUSU))
    {
	REMOVE_BIT(ch->affected_by, AFF_KAKUSU);
	stc("You appear from the shadows.\n\r", ch);
	act("$n appears from the shadows.", ch, NULL, NULL, TO_ROOM);
    }
    else
    {
	if (ch->pcdata->ninja[NINJA_KI] < 20)
	{
	    stc("You don't have enough Ki.\n\r", ch);
	    return;
	}
	stc("You disappear into the shadows.\n\r", ch);
	act("$n disappears into the shadows.", ch, NULL, NULL, TO_ROOM);
	SET_BIT(ch->affected_by, AFF_KAKUSU);
	ch->pcdata->ninja[NINJA_KI] -= 20;
	WAIT_STATE(ch, 12);
    }
    return;
}

void do_bomuzite( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg1[MIL];

    argument = one_argument(argument, arg1);

    if (!IS_NINJA(ch) || get_ninja(ch, NINJA_SORA) < 8)
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (arg1[0] == '\0' || (victim = get_char_room(ch, arg1)) == NULL)
    {
	stc("They aren't here.\n\r", ch);
	return;
    }

    if (is_safe(ch, victim));

    if (ch->pcdata->ninja[NINJA_KI] < 20)
    {
	stc("You need 20 Ki.\n\r", ch);
	return;
    }

    act("You toss your bomb onto the floor and put $N to sleep.", ch, NULL, victim, TO_CHAR);
    act("$n tosses a bomb onto the floor.  You feel sleepy.", ch, NULL, victim, TO_VICT);
    victim->position = POS_SLEEPING;
    ch->pcdata->ninja[NINJA_KI] -= 20; 
    WAIT_STATE(ch, 8);

    return;
}

void do_mizotamouyo( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MIL];

    one_argument(argument, arg);

    if (!IS_NINJA(ch) || get_ninja(ch, NINJA_SORA) < 10)
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (arg[0] == '\0')
    {
	stc("Who do you wish to mizotamouyo to?\r\n", ch);
	return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL || victim == ch || victim->in_room == NULL || victim->in_room == ch->in_room)
    {
	stc("You fail your mizotamouyo.\n\r", ch);
	return;
    }

    if (IS_SET(victim->in_room->room_flags, ROOM_SAFE) || IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
     || IS_SET(victim->in_room->room_flags, ROOM_SOLITARY) || IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
     || IS_SET(victim->extra, EXTRA_ILLUSION) || (!IS_NPC(victim) && !IS_EXTRA(victim, EXTRA_SUMMON))
     || victim->level > 1000)
    {
	stc("You are unable to focus on their location.\n\r", ch);
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
	{
	    guardian_message(victim);
	    act("$n just failed to travel to you.", ch, NULL, victim, TO_VICT);
	}
	return;
    }

    if (IS_AFFECTED(ch, AFF_KAKUSU))
    {
	if (ch->pcdata->ninja[NINJA_KI] < 20)
	{
	    stc("You have run out of Ki and are forced out of the shadows.\n\r", ch);
	    REMOVE_BIT(ch->affected_by, AFF_KAKUSU);
	}
	else ch->pcdata->ninja[NINJA_KI] -= 20;
    }

    if (!IS_AFFECTED(ch, AFF_KAKUSU)) act("$n dissipates into the shadows, slowly vanishing from sight.", ch, NULL, NULL, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, victim->in_room);
    do_look(ch, "");
    if (!IS_AFFECTED(ch, AFF_KAKUSU)) act("$n materializes from the shadows, slowly coming into view.", ch, NULL, NULL, TO_ROOM);
    stc("You travel the paths of shadow to arrive at your destination.\n\r", ch);
    return;
}


void do_mienaku( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *was_in;
    ROOM_INDEX_DATA *now_in;
    CHAR_DATA *victim;
    EXIT_DATA *pexit;
    long attempt,door;

    if (!IS_NINJA(ch) || get_ninja(ch, NINJA_CHIKYU) < 1)
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if ((victim = ch->fighting) == NULL)
    {
	stc("You aren't in combat.\n\r", ch);
	return;
    }

    if (ch->pcdata->ninja[NINJA_KI] < 20)
    {
	stc("You don't have enough Ki.\n\r", ch);
	return;
    }

    if (IS_AFFECTED(ch, AFF_WEBBED) || IS_AFFECTED(ch, AFF_SHADOW))
    {
	stc("You are unable to flee while restrained.\n\r", ch);
	return;
    }

    if (IS_WEREWOLF(victim) && number_percent() < get_tribe(victim, TRIBE_GET_OF_FENRIS) * 5)
    {
	stc("You find yourself moving too slow to flee!\n\r",ch);
	stc("You Halt the Coward's Flight.\n\r", victim);
	return;
    }

    was_in = ch->in_room;

    stc("You move to vanish from combat!\n\r", ch);
    ch->pcdata->ninja[NINJA_KI] -= 10;

    for (attempt =  0; attempt < 6; attempt++)
    {
	door = number_door();
	if ((pexit = was_in->exit[door]) == 0 || pexit->to_room == NULL || IS_SET(pexit->exit_info, EX_CLOSED)) continue;

	move_char(ch, door);
	if ((now_in = ch->in_room) == was_in) continue;

	ch->in_room = was_in;
	stop_fighting(ch, TRUE);
	act("$n has escaped $t!", ch, dir_name[door], NULL, TO_ROOM);
	act("You escape to the $t!", ch, dir_name[door], NULL, TO_CHAR);
	ch->in_room = now_in;
	stc("You escape from combat!\n\r", ch);

	if (victim->in_room == was_in)
	{
	    SET_BIT(victim->extra, BLINDFOLDED);
	    act("You start to move at super speeds and blind $N.", ch, NULL, victim, TO_CHAR);
	    act("$n starts to move at super speeds and blinds $N.", ch, NULL, victim, TO_ROOM);
	    act("$n starts to move at super speeds and blinds you.", ch, NULL, victim, TO_VICT);
	}
	return;
    }

    stc("You attempt to flee failed!\n\r", ch);
    return;
}

void do_orimasu( CHAR_DATA *ch, char *argument )
{
    if (!IS_NINJA(ch) || get_ninja(ch, NINJA_CHIKYU) < 4)
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (ch->pcdata->regenerate > 4)
    {
	stc("You are already regenerating.\n\r", ch);
	return;
    }

    if (ch->pcdata->ninja[NINJA_KI] < 50)
    {
	stc("You need 50 Ki.\n\r", ch);
	return;
    }

    ch->pcdata->ninja[NINJA_KI] -= 50;
    ch->pcdata->regenerate = 10;
    stc("You begin regenerating yourself.\n\r", ch);
    return;
}

void do_tsumazuku( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MSL];
    int chance = number_range(1, 8);

    argument = one_argument(argument, arg);

    if (!IS_NINJA(ch) || get_ninja(ch, NINJA_CHIKYU) < 8)
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if ((victim = ch->fighting) == NULL)
    {
	stc("You must be fighting.\n\r", ch);
	return;
    }

    if (chance <= 3)
    {
	act("$N avoids your attempt to trip $M, causing you to fall prone!", ch, NULL, victim, TO_CHAR);
	act("You avoid $n's attempt to trip you, causing $m to fall prone!", ch, NULL, victim, TO_VICT);
	act("$N avoids $n's attempt to trip $M, causing $m to fall prone!", ch, NULL, victim, TO_NOTVICT);
	WAIT_STATE(ch, 18);
    }
    else
    {
	act("You skillfully trip $N, knocking $M to the ground!", ch, NULL, victim, TO_CHAR);
	act("$n skillfully trips you, knocking you to the ground!", ch, NULL, victim, TO_VICT);
	act("$n skillfully trips $N, knocking $M to the ground!", ch, NULL, victim, TO_NOTVICT);
	WAIT_STATE(ch, 12);
	WAIT_STATE(victim, 15);
    }
    return;
}

void do_harakiri( CHAR_DATA *ch, char *argument )
{
    int hara = 25;
    if (!IS_NINJA(ch) || get_ninja(ch, NINJA_CHIKYU) < 10)
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (ch->pcdata->ninja[NINJA_HARAKIRI] > 0)
    {
	stc("You are still under the influence of the last blood ritual you performed.\n\r", ch);
	return;
    }

    if (ch->hit < ch->max_hit / 5)
    {
	stc("You are hurt too badly.\n\r", ch);
	return;
    }

    hara += ch->hit / 2000;
    hara += ch->mana / 5000;

    ch->hit = ch->max_hit / 20;
    ch->mana = ch->max_mana / 20;
    ch->pcdata->ninja[NINJA_HARAKIRI] = hara;

    stc("You invoke the strength of your blood through ritual purification!\n\r", ch);
    act("$n cuts $s finger and bleeds profusely.", ch, NULL, NULL, TO_ROOM);
    return;
}


void do_tsume( CHAR_DATA *ch, char *argument )
{
    if (!IS_NINJA(ch) || get_ninja(ch, NINJA_NINGENNO) < 1)
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (IS_VAMPAFF(ch, VAM_CLAWS))
    {
	stc("You remove the tsume from your wrists.\n\r", ch);
	REMOVE_BIT(ch->vampaff, VAM_CLAWS);
    }
    else
    {
	stc("You attach a tsume to your wrists.\n\r", ch);
	SET_BIT(ch->vampaff, VAM_CLAWS);
    }
    return;
}

void do_shuurai( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    char arg[MIL];

    one_argument(argument, arg);

    if (!IS_NINJA(ch) || get_ninja(ch, NINJA_NINGENNO) < 4)
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (((obj = get_eq_char(ch, WEAR_WEAPON)) == NULL || obj->value[3] != TYPE_PIERCE)
     && ((obj = get_eq_char(ch, WEAR_HOLD)) == NULL || obj->value[3] != TYPE_PIERCE))
    {
	stc("You may only circle with a piercing weapon.\n\r", ch);
	return;
    }

    if (arg[0] == '\0' || (victim = get_char_room(ch, arg)) == NULL)
    {
	if ((victim = ch->fighting) == NULL) 
	{
	    stc("No such person here.\n\r", ch);
	    return;
        }
    }

    if (victim == ch)
    {
	stc("You fail to circle yourself.\n\r", ch);
	return;
    }

    if (is_safe(ch, victim)) return;

    one_hit(ch, victim, TYPE_PIERCE, "circle");
    WAIT_STATE(ch, 9);

    return;
}

void do_hakunetsu( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    char arg[MIL];

    one_argument(argument, arg);

    if (!IS_NINJA(ch) || get_ninja(ch, NINJA_NINGENNO) < 10)
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (((obj = get_eq_char(ch, WEAR_WEAPON)) == NULL || obj->value[3] != TYPE_PIERCE)
     && ((obj = get_eq_char(ch, WEAR_HOLD)) == NULL || obj->value[3] != TYPE_PIERCE))
    {
	stc("You may only backstab with a piercing weapon.\n\r", ch);
	return;
    }

    if (arg[0] == '\0' || (victim = get_char_room(ch, arg)) == NULL)
    {
	stc("No such person here.\n\r", ch);
	return;
    }

    if (victim == ch || (victim->stance[0] != STANCE_NONE && victim->hit < victim->max_hit))
    {
	stc("You can't properly line up the backstab.\n\r", ch);
	return;
    }

    if (is_safe(ch, victim)) return;

    if (!IS_AFFECTED(ch, AFF_KAKUSU) && get_ninja(ch, NINJA_SORA) >= 6 && ch->pcdata->ninja[NINJA_KI] >= 20) 
	do_kakusu(ch, "");
    one_hit(ch, victim, TYPE_PIERCE, "backstab");
    WAIT_STATE(ch, 12);

    return;
}
