/***************************************************************************
 * Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer, 	   *
 * Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.    *
 * 									   *
 * Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael 	   *
 * Chastain, Michael Quan, and Mitchell Tse.  				   *
 * 									   *
 * In order to use any part of this Merc Diku Mud, you must comply with    *
 * both the original Diku license in 'license.doc' as well the Merc 	   *
 * license in 'license.txt'.  In particular, you may not remove either of  *
 * these copyright notices.  						   *
 * 									   *
 * Much time and thought has gone into this software and you are 	   *
 * benefitting.  We hope that you share your changes too.  What goes 	   *
 * around, comes around.  						   *
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


void do_element( CHAR_DATA *ch, char *argument )
{
    char arg1[MIL];
    char arg2[MIL];
    char buf[MSL];

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (!IS_ELEM(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (arg1[0] == '\0')
    {
	stc("                -=Elemental Skill Levels=-                     \n\r", ch);
	sprintf(buf, "     Fire:          (%-2li)   |     Water:         (%-2li)\n\r", ch->pcdata->powers[ELEM_FIRE], ch->pcdata->powers[ELEM_WATER]);
	stc(buf, ch);
	sprintf(buf, "     Earth:         (%-2li)   |     Wind:          (%-2li)\n\r", ch->pcdata->powers[ELEM_EARTH], ch->pcdata->powers[ELEM_WIND]);
	stc(buf, ch);
	sprintf(buf, "     Sun:           (%-2li)   |     Moon:          (%-2li)\n\r", ch->pcdata->powers[ELEM_SUN], ch->pcdata->powers[ELEM_MOON]);
	stc(buf, ch);
	sprintf(buf, "     Time:          (%-2li)   |     Infinity:      (%-2li)\n\r\n\r", ch->pcdata->powers[ELEM_TIME], ch->pcdata->powers[ELEM_INFIN]);
	stc(buf, ch);

	stc("Syntax: 'elemskill <element>' to see what skills are available. \n\r", ch);
	stc("      : 'elemskill <element> improve' to increase your skill in an element.\n\r", ch);
	stc("      : 'elemskill <element> focus' to focus in an element.\n\r", ch);
	stc("      : 'elemskill remove focus' to unfocus in an element.\n\r", ch);
	return;
    }

    if (arg2[0] == '\0')
    {
	if (!str_cmp(arg1, "fire"))
	{
	    stc("Fire - Element of Strength and Ferocity.\n\r\n\r", ch);
	    if (ch->pcdata->powers[ELEM_FIRE] == 0) stc("You have no skill with Fire.\n\r", ch);
	    if (ch->pcdata->powers[ELEM_FIRE] >= 1) stc("1 : Flare Arrow    - Launches flaming arrows.\n\r", ch);
	    if (ch->pcdata->powers[ELEM_FIRE] >= 5) stc("5 : Flame Aura     - Strength increase. <auto>\n\r", ch);
	    if (ch->pcdata->powers[ELEM_FIRE] >= 8) stc("8 : Fire Blade     - Ignites your weapons.\n\r", ch);
	    if (ch->pcdata->powers[ELEM_FIRE] >= 10) stc("10: Phoenix Form   - Turn into a Phoenix.\n\r", ch);
	}
	else if (!str_cmp(arg1, "water"))
	{
	    stc("Water - Element of Order and Conformity.\n\r\n\r", ch);
	    if (ch->pcdata->powers[ELEM_WATER] == 0) stc("You have no skill with Water.\n\r", ch);
	    if (ch->pcdata->powers[ELEM_WATER] >= 1) stc("1 : Icicle Storm  - Conjures a barrage of icicles.\n\r", ch);
	    if (ch->pcdata->powers[ELEM_WATER] >= 5) stc("5 : Water Travel  - Allows quick travel.\n\r", ch);
	    if (ch->pcdata->powers[ELEM_WATER] >= 8) stc("8 : Aqua Barrier  - Surrounds you with a barrier of water.\n\r", ch);
	    if (ch->pcdata->powers[ELEM_WATER] >= 10) stc("10: Water Control - Stuns opponents.\n\r", ch);
	}
	else if (!str_cmp(arg1, "wind"))
	{
	    stc("Wind - Element of Chaos and Speed.\n\r\n\r", ch);
	    if (ch->pcdata->powers[ELEM_WIND] == 0) stc("You have no skill with Wind.\n\r", ch);
	    if (ch->pcdata->powers[ELEM_WIND] >= 1) stc("1 : Wind Wings     - Lifts you into the air.\n\r", ch);
	    if (ch->pcdata->powers[ELEM_WIND] >= 5) stc("5 : Wind Song      - Speed increase. <auto>\n\r", ch);
	    if (ch->pcdata->powers[ELEM_WIND] >= 8) stc("8 : Wind Blades    - Slashes with blades of wind.\n\r", ch);
	    if (ch->pcdata->powers[ELEM_WIND] >= 10) stc("10: Atoning Winds  - Slows opponents.\n\r", ch);
	}
	else if (!str_cmp(arg1, "earth"))
	{
	    stc("Earth - Element of Toughness.\n\r\n\r", ch);
	    if (ch->pcdata->powers[ELEM_EARTH] == 0) stc("You have no skill with Earth.\n\r",ch);
	    if (ch->pcdata->powers[ELEM_EARTH] >= 1) stc("1 : Local Quake   - Shakes the earth.\n\r",ch);
	    if (ch->pcdata->powers[ELEM_EARTH] >= 5) stc("5 : Earthen Skin  - Defense increase. <auto>\n\r", ch);
	    if (ch->pcdata->powers[ELEM_EARTH] >= 8) stc("8 : Stone Blast   - Blasts everything.\n\r", ch);
	    if (ch->pcdata->powers[ELEM_EARTH] >= 10) stc("10: Liquify Stone - Turns the land into quicksand.\n\r", ch);
	}
	else if (!str_cmp(arg1, "sun"))
	{
	    stc("Sun - Element of Light and Protection.\n\r\n\r", ch);
	    if (ch->pcdata->powers[ELEM_SUN] == 0) stc("You have no skill with Sun.\n\r", ch);
	    if (ch->pcdata->powers[ELEM_SUN] >= 1) stc("1 : Sun Bath        - Showers target with healing light.\n\r", ch);
	    if (ch->pcdata->powers[ELEM_SUN] >= 5) stc("5 : Holy Aura       - Surrounds you with a holy aura.\n\r", ch);
	    if (ch->pcdata->powers[ELEM_SUN] >= 8) stc("8 : Solar Flash     - Blinds opponents.\n\r", ch);
	    if (ch->pcdata->powers[ELEM_SUN] >= 10) stc("10: Solar Light     - Grants you a halo of sunlight.\n\r", ch);
	}
	else if (!str_cmp(arg1, "moon"))
	{
	    stc("Moon - Element of Darkness and Illusion.\n\r\n\r", ch);
	    if (ch->pcdata->powers[ELEM_MOON] == 0) stc("You have no skill with Moon.\n\r", ch);
	    if (ch->pcdata->powers[ELEM_MOON] >= 1) stc("1 : Mirage Slice   - Summons an illusionary double to attack.\n\r", ch);
	    if (ch->pcdata->powers[ELEM_MOON] >= 5) stc("5 : Mirage Bodies  - Evade increase. <auto>\n\r", ch);
	    if (ch->pcdata->powers[ELEM_MOON] >= 8) stc("8 : Night Blade    - Life stealing double attack.\n\r", ch);
	    if (ch->pcdata->powers[ELEM_MOON] >= 10) stc("10: Spiral Moon    - Weakens with dark energy.\n\r", ch);
	}
	else if (!str_cmp(arg1,"time"))
	{
	    stc("Time - Element of Travel and Moments.\n\r\n\r", ch);
	    if (ch->pcdata->powers[ELEM_TIME] == 0) stc("You have no skill with Time.\n\r", ch);
	    if (ch->pcdata->powers[ELEM_TIME] >= 1) stc("1 : Speed Vision   - Attack increase. <auto>\n\r", ch);
	    if (ch->pcdata->powers[ELEM_TIME] >= 5) stc("5 : Hasten         - Speed increase. <auto>\n\r", ch);
	    if (ch->pcdata->powers[ELEM_TIME] >= 8) stc("8 : Quick Chant    - Spell speed increase. <auto>\n\r", ch);
	    if (ch->pcdata->powers[ELEM_TIME] >= 10) stc("10: Slow           - Slows target.\n\r", ch);
	}
	else if (!str_cmp(arg1,"infinity"))
	{
	    stc("Infinity - Element of Unity and the Void.\n\r\n\r", ch);
	    if (ch->pcdata->powers[ELEM_INFIN] == 0) stc("You have no skill with this element.\n\r", ch);
	    if (ch->pcdata->powers[ELEM_INFIN] >= 1) stc("1 : Far Sight      - Watches from afar.\n\r", ch);
	    if (ch->pcdata->powers[ELEM_INFIN] >= 3) stc("3 : Cloak of Sight - Hides you at a distance.\n\r", ch);
	    if (ch->pcdata->powers[ELEM_INFIN] >= 5) stc("5 : Infinite Sight - Opens your eyes to everything.\n\r", ch);
	    if (ch->pcdata->powers[ELEM_INFIN] >= 8) stc("8 : Focus          - Skill increase. <auto>\n\r", ch);
	    if (ch->pcdata->powers[ELEM_INFIN] >= 10) stc("10: Ultima Wave    - Creates a wave of destruction.\n\r", ch);
	}
	else
	{
	    do_element(ch, "");
	    return;
	}
    }
    else if (!str_cmp(arg2, "focus"))
    {
	long element, opposite;

	if (ch->position == POS_FIGHTING)
	{
	    stc("Not while fighting!\n\r",ch);
	    return;
	}

	if (ch->pcdata->powers[ELEM_CHOICE] != 0)
	{
	    if (!str_cmp(arg1, "remove"))
	    {
		if (IS_POLYAFF(ch, POLY_PHOENIXFORM)) do_phoenixform(ch, "");
		if (IS_ITEMAFF(ch, ITEMA_SUNSHIELD)) do_solarlight(ch, "");
		stc("You stop focusing on an element.\n\r", ch);
		for (element = ELEM_FIRE; element <= ELEM_INFIN; element++)
		{
		    if (ch->pcdata->powers[element] == 10) ch->pcdata->powers[element] = 8;
		    if (ch->pcdata->powers[element] == 6) ch->pcdata->powers[element] = 8;
		}
		ch->pcdata->powers[ELEM_CHOICE] = 0;
	    }
	    else stc("You are already focused on an element.  Type 'elemskill remove focus' to refocus.\n\r", ch);
	    return;
	}

	for (element = ELEM_FIRE; element <= ELEM_INFIN; element++)
	{
	    if (ch->pcdata->powers[element] < 8)
	    {
		stc("You may not focus until you have 8 in every element.\n\r", ch);
		return;
	    }
	}

	if (!str_cmp(arg1, "fire"))
	{
	    if (IS_ITEMAFF(ch, ITEMA_AQUABARRIER)) do_aquabarrier(ch, "");
	    element = ELEM_FIRE;
	    opposite = ELEM_WATER;
	}
	else if (!str_cmp(arg1, "water"))
	{
	    if (IS_ITEMAFF(ch, ITEMA_FLAMING)) do_fireblade(ch, "");
	    element = ELEM_WATER;
	    opposite = ELEM_FIRE;
	}
	else if (!str_cmp(arg1, "wind"))
	{
	    element = ELEM_WIND;
	    opposite = ELEM_EARTH;
	}
	else if (!str_cmp(arg1, "earth"))
	{
	    element = ELEM_EARTH;
	    opposite = ELEM_WIND;
	}
	else if (!str_cmp(arg1, "sun"))
	{
	    element = ELEM_SUN;
	    opposite = ELEM_MOON;
	}
	else if (!str_cmp(arg1, "moon"))
	{
	    element = ELEM_MOON;
	    opposite = ELEM_SUN;
	}
	else if (!str_cmp(arg1, "time"))
	{
	    element = ELEM_TIME;
	    opposite = ELEM_INFIN;
	}
	else if (!str_cmp(arg1, "infinity"))
	{
	    element = ELEM_INFIN;
	    opposite = ELEM_TIME;
	}
	else
	{
	    stc("Syntax: 'elemskill <element> focus' to select an elemental focus.\n\r", ch);
	    stc("      : 'elemskill remove focus' to unfocus.\n\r", ch);
	    return;
	}

	sprintf(buf, "You begin to focus in the %s element.\n\r", arg1);
	stc(buf, ch);

	ch->pcdata->powers[ELEM_CHOICE] = element;
	ch->pcdata->powers[element] += 2;
	ch->pcdata->powers[opposite] -= 2;

	return;
    }
    else if (!str_cmp(arg2, "improve"))
    {
	long improve, cost, max = 8;

	if (ch->pcdata->powers[ELEM_CHOICE] != 0)
	{
	    stc("You have already mastered the basics of every element.\n\r", ch);
	    return;
	}

	if      (!str_cmp(arg1, "fire"))	improve = ELEM_FIRE;
	else if (!str_cmp(arg1, "water"))	improve = ELEM_WATER;
	else if (!str_cmp(arg1, "wind"))	improve = ELEM_WIND;
	else if (!str_cmp(arg1, "earth"))	improve = ELEM_EARTH;
	else if (!str_cmp(arg1, "sun"))		improve = ELEM_SUN;
	else if (!str_cmp(arg1, "moon"))	improve = ELEM_MOON;
	else if (!str_cmp(arg1, "time"))	improve = ELEM_TIME;
	else if (!str_cmp(arg1, "infinity"))	improve = ELEM_INFIN;
	else
	{
	    stc("Syntax: 'elemskill <element> improve'.\n\r", ch);
	    return;
	}

	arg1[0] = UPPER(arg1[0]);
	if (ch->pcdata->powers[improve] >= max)
	{
	    sprintf(buf, "You may not improve your skill with the %s element any further.\n\r", arg1);
	    stc(buf, ch);
	    return;
	}

	cost = (ch->pcdata->powers[improve] + 1) * 10;
	if (cost > ch->primal)
	{
	    sprintf(buf, "You need %li primal to improve your skill with the %s element.\n\r", cost, arg1);
	    stc(buf, ch);
	    return;
	}

	ch->pcdata->powers[improve]++;
	ch->primal -= cost;
	sprintf(buf, "You improve your skill with the %s element.\n\r", arg1);
	stc(buf, ch);
    }
    else do_element(ch, "");

    return;
}


void do_flarearrow( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MIL];
    long power, hpcheck;

    one_argument(argument, arg);

    if (!IS_ELEM(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (ch->pcdata->powers[ELEM_FIRE] < 1)
    {
	stc("You need Fire 1 to use Flare Arrows.\n\r", ch);
	return;
    }

    if (arg[0] == '\0' && ch->fighting == NULL)
    {
	stc("Cast Flare Arrows on whom?\n\r", ch);
	return;
    }

    if ((victim = ch->fighting) == NULL)
    {
	if ((victim = get_char_room(ch, arg)) == NULL)
	{
	    stc("They aren't here.\n\r", ch);
	    return;
	}
    }

    if (is_safe(ch, victim)) return;

    power = 100 + 50 * ch->pcdata->powers[ELEM_FIRE];
    if (ch->mana < power)
    {
	stc("You don't have enough mana.\n\r", ch);
	return;
    }

    act("You fire a volley of flame arrows at $N.", ch, NULL, victim, TO_CHAR); 
    act("$n fires a volley of flame arrows at $N.", ch, NULL, victim, TO_NOTVICT); 
    act("$n fires a volley of flame arrows at you.", ch, NULL, victim, TO_VICT);

    ch->mana -= power;
    hpcheck = victim->hit;
    WAIT_STATE(ch, 20 - ch->pcdata->powers[ELEM_FIRE]);

    one_hit(ch, victim, TYPE_HIT, "flare arrow");
    one_hit(ch, victim, TYPE_HIT, "flare arrow");
    one_hit(ch, victim, TYPE_HIT, "flare arrow");

    if (victim == NULL || hpcheck == victim->hit) return;

    if (!IS_NPC(victim) && !IS_AFFECTED(victim, AFF_FLAMING))
    {
	if (number_percent() <= victim->pcdata->atm)
	    act("You resist $N's attempt to set you on fire.", victim, NULL, ch, TO_CHAR);
	else
	{
	    SET_BIT(victim->affected_by, AFF_FLAMING);
	    stc("The flame arrows set you on fire!\n\r", victim);
	    act("The flame arrows set $n on fire!", victim, NULL, NULL, TO_ROOM);
	}
    }

    return; 
}

void do_fireblade(CHAR_DATA *ch, char *argument)
{
    if (!IS_ELEM(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (ch->pcdata->powers[ELEM_FIRE] < 8)
    {
	stc("You need Fire 8 to use Fireblade.\n\r", ch);
	return;
    }

    if (!IS_ITEMAFF(ch, ITEMA_FLAMING))
    {
	if (ch->mana < 2000)
	{
	    stc("You do not have enough mana to surround your weapons in sorching flame.\n\r", ch);
	    return;
	}

	ch->mana -= 2000;
	SET_BIT(ch->itemaffect, ITEMA_FLAMING);
	stc("Your weapons are surrounded by a red scorching fire!\n\r", ch);
	return;
    }
    else if (IS_ITEMAFF(ch, ITEMA_FLAMING))
    {
	REMOVE_BIT(ch->itemaffect, ITEMA_FLAMING);
	stc("Your weapons are no longer surrounded by scorching fire.\n\r", ch);
	return;
    }
    return;
}

void do_phoenixform( CHAR_DATA *ch, char *argument )
{
    char buf[50];

    if (!IS_ELEM(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (ch->pcdata->powers[ELEM_FIRE] < 10)
    {
	stc("You need Fire 10 to use Phoenixform.\n\r", ch);
	return;
    }

    if (IS_AFFECTED(ch, AFF_POLYMORPH) && !IS_VAMPAFF(ch, VAM_DISGUISED))
    {
	stc("Not while polymorphed.\n\r", ch);
	return;
    }

    if (IS_POLYAFF(ch, POLY_PHOENIXFORM))
    {
	free_string(ch->morph);
	ch->morph = str_dup("");
	REMOVE_BIT(ch->affected_by, AFF_POLYMORPH);
	REMOVE_BIT(ch->vampaff, VAM_DISGUISED);
	REMOVE_BIT(ch->polyaff, POLY_PHOENIXFORM);
	stc("You dispel the fiery form around you.\n\r", ch);
	act("A fire phoenix's form fades away to reveal $n.", ch, NULL, NULL, TO_ROOM);
	return;
    }

    if (ch->mana < 5000)
    {
	stc("You do not have enough mana to use phoenix form.\n\r", ch);
	return;
    }

    ch->mana -= 5000;
    stc("You cry out as your flames consume and turn you into a phoenix!\n\r", ch);
    act("$n's body is consumed by flame as they turn into a firey phoenix!", ch, NULL, NULL, TO_ROOM);
    SET_BIT(ch->affected_by, AFF_POLYMORPH);
    SET_BIT(ch->vampaff, VAM_DISGUISED);
    SET_BIT(ch->polyaff, POLY_PHOENIXFORM);
    free_string(ch->morph);
    sprintf(buf, "#RA firey phoenix#n[#0%s#n]", ch->name);
    ch->morph = str_dup(buf);
    return;
}


void do_iciclestorm( CHAR_DATA *ch, char *argument ) 
{
    CHAR_DATA *victim;
    char arg[MIL];
    long power, wait;

    one_argument(argument, arg);

    if (!IS_ELEM(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (ch->pcdata->powers[ELEM_WATER] < 1)
    {
	send_to_char("You need Water 1 to use Icicle Storm.\n\r", ch);
	return;
    }

    power = 150 * ch->pcdata->powers[ELEM_WATER];

    if (arg[0] == '\0' && ch->fighting == NULL)
    {
	send_to_char( "Cast Icicle Storm on whom?\n\r", ch ); 
	return; 
    } 

    if ((victim = ch->fighting) == NULL)
    {
	if ((victim = get_char_room(ch, arg)) == NULL)
	{
	    stc("They aren't here.\n\r", ch);
	    return; 
	}
    }

    if (ch->mana < power)
    {
	stc("You don't have enough mana.\n\r", ch);
	return;
    }

    act("You send a swathe of sharp icicles at $N.",ch,NULL,victim,TO_CHAR);
    act("$n sends a swathe of sharp icicles at $N.",ch,NULL,victim,TO_NOTVICT);
    act("$n sends a swathe of sharp icicles at you.",ch,NULL,victim,TO_VICT);

    one_hit(ch, victim, TYPE_HIT, "icicle barrage");

    WAIT_STATE(ch, 12);
    return;
}

void do_watertravel( CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    char arg[MIL];

    argument = one_argument(argument, arg);

    if (!IS_ELEM(ch))
    {
	stc("Huh?\n\r",ch);
	return;
    }

    if (ch->pcdata->powers[ELEM_WATER] < 5)
    {
	stc("You need Water 5 to use Water Travel.\n\r", ch);
	return;
    }

    if (ch->position == POS_FIGHTING)
    {
	stc("You can't concentrate enough!\n\r", ch);
	return;
    }

    if (ch->mana < 1500)
    {
	stc("You don't have enough mana!\n\r", ch);
	return;
    }

    if (arg[0] == '\0')
    {
	stc("Travel to who?\n\r", ch);
	return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL || victim == ch || IS_BUI(victim)
     || victim->in_room == NULL || ch->in_room == NULL || victim->in_room == ch->in_room)
    {
	stc("You concentrate but fail to form yourself into a figure of water!\n\r", ch);
	return;
    }

    if (IS_SET(victim->in_room->room_flags, ROOM_PRIVATE) || IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
     || IS_SET(victim->in_room->room_flags, ROOM_NO_TELEPORT) || IS_SET(victim->in_room->room_flags, ROOM_SAFE)
     || (!IS_NPC(victim) && !IS_EXTRA(victim, EXTRA_SUMMON)))
    {
	stc("You concentrate but fail to form yourself into a figure of water!\n\r", ch);
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim,MORE_GUARDIAN))
	{
	    guardian_message(victim);
	    act("$n just failed to Water Travel to you.", ch, NULL, victim, TO_VICT);
	}
	return;
    }

    act("$n turns into a figure of water and vanishes in a spray of mist!!", ch, NULL, NULL, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, victim->in_room);
    act("You turn yourself into water and reform your watery body at $N!", ch, NULL, victim, TO_CHAR);
    act("$n reforms from the particles of water in the air!\n\r",ch,NULL,NULL,TO_ROOM);

    do_look(ch, "auto");
    ch->mana -= 1500;
    return;
}

void do_aquabarrier( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];

    argument = one_argument(argument, arg);

    if (!IS_ELEM(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (ch->pcdata->powers[ELEM_WATER] < 8)
    {
	stc("You need Water 8 to use Aqua Barrier.\n\r", ch);
	return;
    }

    if (!IS_ITEMAFF(ch, ITEMA_AQUABARRIER))
    {
	if (ch->mana < 2500)
	{
	    stc("You don't have enough mana to summon the barrier!\n", ch);
	    return;
	}
	if (IS_POLYAFF(ch,POLY_PHOENIXFORM))
	{
	    stc("The barrier of water you try to form around yourself turns into hot steam!\n", ch);
	    WAIT_STATE(ch, 12);
	    return;
	}
        SET_BIT(ch->itemaffect, ITEMA_AQUABARRIER);
    	stc("You surround yourself with a dense barrier of water!\n\r", ch);
	ch->mana -= 2500;
	WAIT_STATE(ch, 12);
    }
    else
    {
	REMOVE_BIT(ch->itemaffect, ITEMA_AQUABARRIER);
    	stc("The barrier of water surrounding you disperses in a spray of mist!\n\r", ch);
    }

    return;
}

void do_watercontrol(CHAR_DATA *ch, char *argument)
{
    char arg[MIL];
    CHAR_DATA *victim;

    argument = one_argument(argument, arg);

    if (!IS_ELEM(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (ch->pcdata->powers[ELEM_WATER] < 10)
    {
	stc("You need Water 10 to use Water Control.\n\r", ch);
	return;
    }

    if ((victim = ch->fighting) == NULL)
    {
	stc("Only during a fight.\n\r", ch);
	return;
    }

    if (ch->mana < 4000)
    {
	stc("You don't have enough mana.\n\r", ch);
	return;
    }

    if (number_range(1, char_stat(victim, STAT_INT) + char_stat(victim, STAT_WIS)) > 15)
    {
	act("You gesture towards $N, but $E resists your spell!\n\r", ch, NULL, victim, TO_CHAR);
	act("$n gesture towards $N, but $E resists $n's spell!\n\r", ch, NULL, victim, TO_NOTVICT);
	act("$n gesture towards you, but you resist $s spell!\n\r", ch, NULL, victim, TO_VICT);
    }
    else
    {
	ch->mana -= 4000;
	act("You gesture towards $N, sending $M to the ground gasping!\n\r", ch, NULL, victim, TO_CHAR);
	act("$n gestures towards $N, sending $M to the ground gasping!\n\r", ch, NULL, victim, TO_NOTVICT);
	act("$n gestures towards you, sending you to the ground gasping!\n\r", ch, NULL, victim, TO_VICT);
	WAIT_STATE(victim, 20);
    }

    WAIT_STATE(ch, 15);
    return;
}


void do_windwings( CHAR_DATA *ch, char *argument )
{
    if (!IS_ELEM(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (ch->pcdata->powers[ELEM_WIND] < 1)
    {
	stc("You need Wind 1 to use Wind Wings.\n\r", ch);
	return;
    }

    if (!IS_ITEMAFF(ch, AFF_FLYING))
    {
	stc("You start floating on a cushion of air.\n\r", ch);
	SET_BIT(ch->affected_by, AFF_FLYING);
    }
    else 
    {
	stc("You stop floating on a cushion of air.\n\r", ch);
	REMOVE_BIT(ch->affected_by, AFF_FLYING);
    }

    return;
}

void do_windblades( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MIL];
    long power;

    argument = one_argument(argument, arg);

    if (!IS_ELEM(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (ch->pcdata->powers[ELEM_WIND] < 8)
    {
	stc("You need Wind 8 to use Wind Blades.\n\r", ch);
	return;
    }
    
    if (arg[0] == '\0' && ch->fighting == NULL)
    {
	stc("Send blades of wind at who?\n\r", ch);
	return;
    }
    
    if ((victim = ch->fighting) == NULL)
    {
	if ((victim = get_char_room(ch, arg)) == NULL)
	{
	    stc("They aren't here.\n\r", ch);
	    return;
	}
    }

    if (is_safe(ch, victim)) return;

    power = ch->pcdata->powers[ELEM_WIND] * 100;

    if (ch->mana < power)
    {
	stc("You don't have enough mana.\n\r", ch);
	return;
    }
    
    if (ch == victim)
    {
	stc("Right.  Hit yourself with razor-sharp blades of wind...No!\n\r", ch);
	return;
    }

    act("You spin around, sending blades of wind at $N.", ch, NULL, victim, TO_CHAR); 
    act("$n spins around, sending blades of wind at $N.", ch, NULL, victim, TO_NOTVICT); 
    act("$n spind around, sending blades of wind at you.", ch, NULL, victim, TO_VICT);

    WAIT_STATE(ch, 12);
    ch->mana -= power;

    one_hit(ch, victim, TYPE_HIT, "wind blade");
    one_hit(ch, victim, TYPE_HIT, "wind blade");
    one_hit(ch, victim, TYPE_HIT, "wind blade");

    return; 
}

void do_stoneblast( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];
    CHAR_DATA *victim;
    CHAR_DATA *victim_next;

    one_argument(argument, arg);

    if (!IS_ELEM(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (ch->pcdata->powers[ELEM_EARTH] < 8)
    {
	stc("You lack the ability to control the earth for this attack!.\n\r", ch);
	return;
    }

    if (ch->mana < ch->pcdata->powers[ELEM_EARTH] * 350)
    {
	stc("You can't concentrate enough!\n\r", ch);
	return;
    }

    ch->mana -= ch->pcdata->powers[ELEM_EARTH] * 350;
    stc("You strike the ground, sending a blast of rocks flying everywhere!!\n\r", ch);
    act("$n strikes the ground, sending a blast of rocks flying everywhere!!!", ch, NULL, NULL, TO_ROOM);
    for (victim = ch->in_room->people; victim; victim = victim_next)
    {
	victim_next = victim->next_in_room;
	if (victim == ch) continue;
	if (is_same_group(ch, victim)) continue;
	if (is_safe(ch, victim)) continue;
	if (!CAN_PK(victim)) continue;
	if (IS_BUI(victim)) continue;
	if (!str_cmp(ch->pcdata->conding, victim->name) && IS_MORE(ch, MORE_LOYAL)) continue;
  	one_hit(ch, victim, TYPE_HIT, "rock shower");
  	one_hit(ch, victim, TYPE_HIT, "rock shower");
    }
    WAIT_STATE(ch, 13);
    return; 
}

void do_localquake( CHAR_DATA *ch, char *argument ) {
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	long dam;
	long earthlevel;
	char buf[MAX_INPUT_LENGTH];
	one_argument( argument, arg );
	if (IS_NPC(ch)) return;
	if ( !IS_ELEM(ch) )
	{
	   send_to_char("Huh?\n\r", ch);
	   return;
	}
	if (ch->pcdata->powers[ELEM_EARTH] < 1)
	{
	   send_to_char("You need Earth Mastery Level 1 to use this ability\n\r",ch);
	   return;
	}
	earthlevel = ch->pcdata->powers[ELEM_EARTH];
	if ( arg[0] == '\0' && ch->fighting == NULL)
	{
	   send_to_char( "Cast Local Quake on whom?\n\r", ch );
	   return;
	}
	if ( (victim = ch->fighting) == NULL)
	{
	   if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	   {
		send_to_char( "They aren't here.\n\r", ch );
		return;
	   }
	}

	if (ch->mana < (earthlevel*(20-earthlevel)*30 ))
	{
	   send_to_char("You don't have enough mana.\n\r", ch);
	   return;
	}
	if ( ch == victim )
	{
	   send_to_char( "Shake the ground underneath your body and risk stunning yourself? Try it on something else.\n\r", ch );
	   return;
	}

	if (!hurt_fighting(ch, victim)) return;

	ch->mana -= (earthlevel*(20-earthlevel) * 30);
	dam = number_range(1,500)+(earthlevel*(earthlevel))*10;
	if (!IS_NPC(victim) && (victim->pcdata->atm > 0))
	   dam = (dam*(100-victim->pcdata->atm)/100);
	if (!IS_ITEMAFF(ch,AFF_FLYING))
	{
	   if (dam >= 0  && dam <= 300)
	   {
		sprintf (buf,"Your local quake shakes $N. [#g%li#n]",dam);
		act(buf, ch, NULL, victim, TO_CHAR);
		sprintf(buf, "$n's local quake shakes you. [#r%li#n]",dam);
		act(buf, ch,NULL, victim, TO_VICT);
		act("$n's local quake shakes $N.", ch,NULL, victim, TO_NOTVICT);
	   }
	   if (dam >= 301 && dam <= 1200)
	   {
		sprintf (buf,"Your local quake causes the ground underneath $N to fracture! [#g%li#n]",dam);
		act(buf, ch, NULL, victim, TO_CHAR);
		sprintf(buf,"$n's local quake causes the ground underneath you to fracture! [#r%li#n]",dam);
		act(buf, ch,NULL, victim, TO_VICT);
		act("$n's local quake causes the ground underneath $N to fracture!", ch,NULL, victim, TO_NOTVICT);
	   }
	   if (dam >= 1201 && dam <= 1500)
	   {
		sprintf (buf,"Your local quake rips up the ground underneath $N!! [#g%li#n]",dam);
		act(buf, ch, NULL, victim, TO_CHAR);
		sprintf(buf,"$n's local quake rips up the ground underneath you!! [#r%li#n]", dam);
		act(buf, ch,NULL, victim, TO_VICT);
		act("$n's local quake rips up the ground underneath $N!!", ch,NULL, victim, TO_NOTVICT);
	   }
	   hurt_person(ch, victim, dam);
	   if((number_range(1,40)<(earthlevel+1))/*||(earthlevel == 10)*/)
	   {
		stc("The quake knocks you off your feet!\n\r",victim);
		act("The quake knocks $n off their feet!\n\r",victim, NULL, NULL, TO_ROOM);
		WAIT_STATE(victim, earthlevel*1.5);
	   }
	}
	else
	{
	   act("You send a tremor through the ground underneath $n's feet, but to no affect!", ch, NULL, victim, TO_CHAR);
	   act("$n sends a tremor through the ground underneath your feet, but to no affect!", ch, NULL, victim, TO_VICT);
	   act("$n sends a tremor through the ground underneath $n's feet, but to no affect!", ch, NULL, victim, TO_NOTVICT);
	}
	if ( victim->position > POS_STUNNED)
	{
	   if(victim->fighting == NULL) set_fighting (victim, ch);
	   victim->position = POS_FIGHTING;
	   if(ch->fighting == NULL) set_fighting (ch, victim);
	}
	WAIT_STATE( ch, earthlevel*1 );
	return;
}

void do_holyaura( CHAR_DATA *ch, char *argument )
{
    	char arg[MIL];
    	argument = one_argument( argument, arg );
    	if(IS_NPC(ch)) return;
    	if(!IS_ELEM(ch))
    	{
           stc("Huh?\n\r",ch);
           return;
    	}
    	if(ch->pcdata->powers[ELEM_SUN]<5)
    	{
           stc("You haven't reached a high enough level in Sun control for this!\n\r",ch);
	   return;
    	}
	if(ch->mana < ch->pcdata->powers[ELEM_SUN] *500)
	{
	    stc("You don't have enough mana.\n\r", ch);
	    return;
	}

	ch->mana -= ch->pcdata->powers[ELEM_SUN] *500;
    	if (!IS_ITEMAFF(ch, ITEMA_AURA) )
    	{
	   SET_BIT(ch->itemaffect, ITEMA_AURA);
	   send_to_char ("You surround yourself in a holy aura!\n\r",ch);
	   return;
    	}
    	else if (IS_ITEMAFF(ch, ITEMA_AURA) )
    	{
	   REMOVE_BIT(ch->itemaffect, ITEMA_AURA);
	   send_to_char("The aura surrounding you dies down and fades away\n\r",ch);
	   return;
	}
   	return; 
}

void do_solarflash(CHAR_DATA *ch, char *argument)
{
    	char arg[MAX_STRING_LENGTH];
    	long sn;
    	AFFECT_DATA af;
    	CHAR_DATA *victim;
    	argument = one_argument(argument, arg);
    	if(IS_NPC(ch)) return;
    	if(!IS_ELEM(ch))
    	{
           stc("Huh?\n\r",ch);
           return;
	}
    	if(ch->pcdata->powers[ELEM_SUN]<8)
    	{
    	   stc("You cannot control the light effectively enough yet!.\n\r", ch);
    	   return;
    	}
    	if (arg[0] == '\0')
    	{
    	   stc("Who do you wish to solarflash?.\n\r",ch);
    	   return;
    	}
    	if ((victim = get_char_room(ch, arg)) == NULL)
    	{
    	   stc("They are not here.\n\r",ch);
    	   return;
    	}
    	if (is_safe(ch,victim)) return;
	if (ch->mana < ch->pcdata->powers[ELEM_SUN] * 250)
	{
	    stc("You don't have enough mana.\n\r", ch);
	    return;
	}

    	if (!IS_NPC(victim))
    	{
           if (IS_SET(victim->act, PLR_HOLYLIGHT)) REMOVE_BIT(victim->act, PLR_HOLYLIGHT);
           else
           {
            	stc("The light breaks your concentration and your supernatural vision dims.\n\r",victim);
           }
    	}
    	if (!IS_AFFECTED(victim, AFF_BLIND))
    	{
    	   if ( ( sn = skill_lookup( "blind" ) ) < 0 ) return;
    	   af.type      = sn;
    	   af.location  = APPLY_HITROLL;
    	   af.modifier  = -50;
    	   af.duration  = 20;
    	   af.bitvector = AFF_BLIND;
    	   affect_to_char( victim, &af );
    	}
    	act("You gesture and emit a pulse of bright light! $N is blinded!",ch,NULL,victim,TO_CHAR);
    	act("$n gestures and emits a pulse of bright light! $N is blinded!",ch,NULL,victim,TO_NOTVICT);
    	act("$n gestures and emits a pulse of bright light! You are blinded!",ch,NULL,victim,TO_VICT);
	if ( victim->position > POS_STUNNED)
	{
	   if(victim->fighting == NULL) set_fighting (victim, ch);
	   victim->position = POS_FIGHTING;
	   if(ch->fighting == NULL) set_fighting (ch, victim);
	}
	ch->mana -= ch->pcdata->powers[ELEM_SUN] *250;
    	WAIT_STATE(ch,15);
    	return;
}

void do_sunbath(CHAR_DATA *ch, char *argument) {
    	char arg[MAX_INPUT_LENGTH];
    	CHAR_DATA *victim;
	long i;
    	argument = one_argument( argument, arg );
    	if (IS_NPC(ch) ) return;
    	if (!IS_ELEM(ch) ){
	   send_to_char("Huh?\n\r",ch);
	   return;
	}
	if (ch->pcdata->powers[ELEM_SUN] < 3){
   	   stc("You lack the training in light control to use this technique.\n\r",ch);
   	   return;
   	}
	if (ch->mana < 800){
	   send_to_char("You do not have enough mana to summon down healing light.\r\n", ch);
	   return;
    	}
	if ( arg[0] == '\0' )
	   victim = ch;
    	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    	{
    	   send_to_char( "They aren't here.\n\r", ch );
     	   return;
    	}
	i = victim->max_hit / 10;
	if ( ch->fighting != NULL ) i /= (11 - ch->pcdata->powers[ELEM_SUN]);
	i = number_range(1001,1500);
   	victim->hit += i;
    	ch->mana -= 800;
    	if (victim->hit > victim->max_hit)
	   victim->hit = victim->max_hit;
    	update_pos( victim );
    	if ((ch == victim) )
    	{
	   act("You shower holy light upon yourself. The pain ebbs away!", ch, NULL, victim, TO_CHAR);
	   act("$n is bathed in a holy light.", ch,NULL, victim, TO_NOTVICT);
    	}
    	else
    	{
	   act("You shower holy light on $N, healing their wounds.", ch, NULL, victim, TO_CHAR);
	   act("$n shower holy light on you, and healing your wounds.", ch,NULL, victim, TO_VICT);
	   act("$n shower holy light on $N, healing their wounds.", ch,NULL, victim, TO_NOTVICT);
    	}

	WAIT_STATE(ch, 18);
    	return; 
}

void do_solarlight( CHAR_DATA *ch, char *argument )
{
    	if (IS_NPC(ch)) return;
    	if (!IS_ELEM(ch))
    	{
           send_to_char("Huh?\n\r",ch);
           return;
    	}
	if (ch->pcdata->powers[ELEM_SUN] < 10)
    	{
    	   send_to_char("Only one who has mastered the art of light control can do this technique.\n\r",ch);
    	   return;
    	}
	
	if (ch->mana < ch->pcdata->powers[ELEM_SUN]*600)
	{
	    stc("You don't have enough mana.]\n\r", ch);
	    return;
	}
	ch->mana -= ch->pcdata->powers[ELEM_SUN]*600;
    	if (IS_ITEMAFF(ch,ITEMA_SUNSHIELD) )
    	{
    	   send_to_char("The aura of sunlight around you fades away.\n\r",ch);
    	   act("$n's aura of sunlight gradually fades away.", ch, NULL, NULL, TO_ROOM);
    	   REMOVE_BIT(ch->itemaffect, ITEMA_SUNSHIELD);
    	   return;
    	}
    	send_to_char("You concentrate and surround yourself in an aura of sunlight!\n\r",ch);
    	act("$n concentrates for a moment and surrounds themselves in an aura of sunlight!", ch, NULL, NULL, TO_ROOM);
    	SET_BIT(ch->itemaffect, ITEMA_SUNSHIELD);
    	return;
}

void do_infinitesight( CHAR_DATA *ch, char *argument ) {
    if ( IS_NPC(ch) )return;
    if ( !IS_ELEM(ch) )
    {
  	send_to_char("Huh?\n\r", ch);
  	return;
    }
    if ( ch->pcdata->powers[ELEM_INFIN] < 5 )
    {
  	send_to_char("You have not learned enough in the ways of Infinity.\n\r", ch);
  	return;
    }
    if ( IS_SET(ch->act, PLR_HOLYLIGHT) )
    {
	REMOVE_BIT(ch->act, PLR_HOLYLIGHT);
	send_to_char( "Your vision becomes limited to the finite once again.\n\r", ch );
    }
    else
    {
	send_to_char( "Your vision clears and everything is now visible.\n\r", ch );
	SET_BIT(ch->act, PLR_HOLYLIGHT);
    }
    return; 
}

void do_mirageslice( CHAR_DATA *ch, char *argument ) {
    	char arg[MAX_INPUT_LENGTH];
    	CHAR_DATA *victim;
    	int loop;
    	one_argument( argument, arg );
    	if (!IS_ELEM(ch) )
    	{
	   send_to_char ("Huh?\n\r",ch);
	   return;
    	}
    	if (ch->pcdata->powers[ELEM_MOON] < 3)
    	{
	   send_to_char("You lack the illusionary skill for Mirage Slice.\n\r",ch);
	   return;
    	}
	if (ch->mana < ch->pcdata->powers[ELEM_MOON]* 100)
	{
	   stc("You can't focus enough to use this ability!\n\r",ch);
	   return;
	}
   	if (arg[0] == '\0' && ch->fighting == NULL)
    	{
           send_to_char( "Who shall you hit?\n\r", ch );
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
    	if (is_safe(ch,victim)) return;
    	if ( ch == victim )
    	{
           send_to_char( "Use this? On Yourself?\n\r", ch );
           return;
    	}
	ch->mana -= ch->pcdata->powers[ELEM_MOON]*100;
    	act("You form an illusionary double and you both attack $N!",ch, NULL, victim, TO_CHAR);
    	act( "$n forms an illusionary double and both attack you!", ch, NULL, victim, TO_VICT );
    	act( "$n forms an illusionary double and both attack $N!", ch, NULL, victim, TO_NOTVICT);
    	for (loop = 1; loop <= 2; loop++)
    	{
	   one_hit(ch, victim, TYPE_HIT, "mirage strike");
    	}
    	WAIT_STATE(ch, 8);
	return; 
}

void do_ultimawave( CHAR_DATA *ch, char *argument ) {
    	char arg[MAX_INPUT_LENGTH];
    	CHAR_DATA *victim;
    	CHAR_DATA *victim_next;
    	int damage;
	int blah;
    	char buf[MAX_INPUT_LENGTH];
    	one_argument( argument, arg );
    	if (IS_NPC(ch)) return;
    	if (!IS_ELEM(ch))
    	{
           send_to_char("Huh?\n\r",ch);
           return;
    	}
    	if (ch->pcdata->powers[ELEM_INFIN] < 10){
    	   stc("Only a master in the arts of the infinite can use this ability!",ch);
    	   return;
    	}
    	if (ch->mana < 10000)
    	{
    	   stc("You are too mentally drained to do this ability!",ch);
           return;
    	}
	if (ch->pcdata->obeah > 0)
    	{
           send_to_char("You cannot concentrate enough to do this attack again! Try later!\n\r",ch);
           return;
    	}
    	damage = ch->mana*4/10;
	blah = ch->mana*3/1000;

    	ch->mana = 0;
	ch->pcdata->obeah = blah;

    	send_to_char("You gather all your mental energy and release it in one giant explosion!\n\r",ch);
    	act("$n glows a bright white and the room explodes in a fury of pure energy!!",ch,NULL,NULL,TO_ROOM);
    	for (victim = ch->in_room->people; victim; victim = victim_next)
    	{
	    victim_next = victim->next_in_room;
	    if (victim == ch) continue;
	    if (is_same_group(ch, victim)) continue;
	    if (IS_BUI(victim)) continue;
	    if (!CAN_PK(victim)) continue;
	    if (!hurt_fighting(ch, victim)) continue;

	    if (!IS_NPC(victim) && (victim->pcdata->atm > 0))
		damage *= (100 - victim->pcdata->atm) / 100;
	    sprintf(buf, "$n's body is battered by the mental energy! [#g%i#n]", damage);
	    act(buf,victim, NULL, NULL, TO_ROOM);
	    sprintf(buf, "You are bruised and battered by the blast of mental energy! [#r%i#n]", damage);
	    act(buf,ch,NULL,victim, TO_VICT);
	    hurt_person(ch, victim, damage);
    	}
    	WAIT_STATE( ch, 30 );
    	return;  
} 

void do_elemarmor( CHAR_DATA *ch, char *argument ) 
{
    	OBJ_INDEX_DATA *pObjIndex;
    	OBJ_DATA *obj;
    	char arg[MAX_INPUT_LENGTH];
    	int vnum = 0;
    	argument = one_argument( argument, arg );
    	if (IS_NPC(ch)) return;
    	if (!IS_ELEM(ch))
    	{
	   send_to_char("Huh?\n\r",ch);
	   return;
    	}
    	if (arg[0] == '\0')
    	{
	   send_to_char("Please specify what kind of equipment you wish to forge.\n\r", ch );
	   send_to_char("Bracer or Sword\n\r", ch );
	   return;
    	}
    	if (!str_cmp(arg,"bracer"  )) vnum = 29620;
    	else if (!str_cmp(arg,"sword"  )) vnum = 29621;
    	else
    	{
	   send_to_char("That is an invalid type.\n\r", ch );
	   return;
    	}
    	if ( ch->primal < 100 || ch->pcdata->quest < 400)
    	{
	   send_to_char("It costs 100 points of primal and 400 quest points to forge this piece of armor.\n\r",ch);
	   return;
    	}
    	if ( vnum == 0 || (pObjIndex = get_obj_index( vnum )) == NULL)
    	{
	   send_to_char("Uh-oh! It's not there! Please inform Akuma!\n\r",ch);
	   return;
    	}
    	obj = create_object(pObjIndex, 50);
    	obj_to_char(obj, ch);
    	ch->primal -= 100;
    	ch->pcdata->quest -= 400;
    	free_string( obj->questmaker );
    	obj->questmaker = str_dup( ch->name );
    	free_string( obj->questowner );
    	obj->questowner = str_dup( ch->name );
    	act("You create a piece of starsilver and forge $p out of it.",ch,obj,NULL,TO_CHAR);
    	act("$n creates a piece of starsilver and forges $p out of it.",ch,obj,NULL,TO_ROOM);
    	return;
}

void do_cloak( CHAR_DATA *ch, char *argument)
{
	if (IS_NPC(ch)) return;
	if (!IS_ELEM(ch))
	{
	   stc("Huh?\n\r",ch);
	   return;
	}
	if(ch->pcdata->powers[ELEM_INFIN] < 3)
	{
	   stc("You lack the skill and discipline in the Infinity Element for this ability.\n\r",ch);
	   return;
	}
	if (!IS_MORE(ch, MORE_SHIELDED))
	{
	   stc("You hide your presence and your strength from prying eyes.\n\r",ch);
	   SET_BIT(ch->more, MORE_SHIELDED);
	   return;
	}
	stc("You lower your guard in hiding yourself.\n\r",ch);
	REMOVE_BIT(ch->more, MORE_SHIELDED);
	return;
}

void do_atoningwinds( CHAR_DATA *ch, char *argument )
{
    	char arg[MIL];
    	CHAR_DATA *victim;
//    	AFFECT_DATA af;
    	long sn;
    	argument = one_argument( argument, arg );
  
  	if(IS_NPC(ch))  return;
    	if(!IS_ELEM(ch))
    	{
           stc("Huh?\n\r",ch);
           return;
    	}
    	if(ch->pcdata->powers[ELEM_WIND]<10)
    	{
           stc("You cannot control the winds well enough to use this ability!\n\r",ch);
           return;
    	}
    	if ((sn = skill_lookup("atoningwind")) < 0 )
    	{
           stc("We got some bugs! Report to Akuma!\n\r",ch);
           return;
    	}
	if ( arg[0] == '\0' )
    	{
	   send_to_char( "Cast Atoning Winds on who?\n\r",ch);
	   return;
    	}
	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    	{
	   send_to_char( "Who? What? Where?\n\r", ch );
	   return;
    	}
    	if (is_safe(ch,victim)) return;
    	if ( ch == victim )
    	{
           send_to_char( "It's novel for weakening yourself, but hardly wise.\n\r", ch );
           return;
    	}
//    	if(is_affected(victim,sn))
	if (IS_XTRA(victim, XTRA_ATONING))
    	{
           stc("But they are already being held down by the winds!\n\r",ch);
           return;
    	}
	if(ch->mana < ch->pcdata->powers[ELEM_WIND]* 600)
	{
	    stc("You don't have enough mana.\n\r", ch);
	    return;
	}
	ch->mana -= ch->pcdata->powers[ELEM_WIND]* 600;
    	if( number_range(1,(char_stat(victim, STAT_INT)+char_stat(victim, STAT_DEX)))> 14){
	   act("You call down a ferocious wind on $N, but they manage to escape!", ch, NULL, victim, TO_CHAR);
	   act("$n calls down a ferocious wind on you, but you manage to escape!", ch,NULL, victim, TO_VICT);
	   act("$n calls down a ferocious wind on $N, but they manage to escape!.", ch,NULL, victim, TO_NOTVICT);
	}
	else 
	{
	/*
    	   af.type      = sn;
    	   af.duration  = 20;
	   af.location  = APPLY_NONE;
     	   af.bitvector = 0;
	   af.modifier  = 0;
    	   affect_to_char( victim, &af );
	*/
           SET_BIT(victim->xtra, XTRA_ATONING);
	   victim->atoningwinds_timer = 20;
	   act("You call down a ferocious wind on $N, slowing them down.", ch, NULL, victim, TO_CHAR);
	   act("$n calls down a ferocious wind on you, slowing you down.", ch,NULL, victim, TO_VICT);
	   act("$n calls down a ferocious wind on $N, slowing them down..", ch,NULL, victim, TO_NOTVICT);
	}
   	if ( victim->position > POS_STUNNED)
   	{
	   if(victim->fighting == NULL) set_fighting (victim, ch);
	   victim->position = POS_FIGHTING;
	   if(ch->fighting == NULL) set_fighting (ch, victim);
	}
    	WAIT_STATE(ch, 14);
    	return;
}

void do_spiralmoon( CHAR_DATA *ch, char *argument )
{
    	char arg[MIL];
    	CHAR_DATA *victim;
    	AFFECT_DATA af;
    	long sn;
	long amount;
    	argument = one_argument( argument, arg );
    	if(IS_NPC(ch))  return;
    	if(!IS_ELEM(ch))
    	{
           stc("Huh?\n\r",ch);
           return;
    	}
    	if(ch->pcdata->powers[ELEM_MOON]<10)
    	{
           stc("You cannot control the shadows and characteristics of the moon  well enough to use this ability!\n\r",ch);
           return;
    	}
    	if((sn = skill_lookup("spiralmoon")) < 0 )
    	{
           stc("We got some bugs! Report to Akuma!\n\r",ch);
           return;
    	}
	if ( arg[0] == '\0' )
    	{
	   send_to_char( "Cast Spiral Moon on who?\n\r",ch);
	   return;
    	}
	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    	{
	   send_to_char( "Who? What? Where?\n\r", ch );
	   return;
    	}
    	if (is_safe(ch,victim)) return;
    	if(is_affected(victim,sn))
    	{
           stc("But they already are affected by the spirits of darkness!\n\r",ch);
	   return;
    	}
	if( ch->mana < 5500)
	{
	    stc("You don't have enough mana.\n\r", ch);
	    return;
	}
	ch->mana -= 5500;
	amount = victim->max_hit/4;
    	if( number_range(1,(char_stat(victim, STAT_INT)+char_stat(victim, STAT_WIS)))> 15){
	   if(victim != ch)
	   {
	   	act("You form a dark red moon over $N, but they resist its dark affects!", ch, NULL, victim, TO_CHAR);
	   	act("$n forms a dark red moon over you, but you resist its dark affects!", ch,NULL, victim, TO_VICT);
	   	act("$n forms a dark red moon over $N, but they resist its dark affects!", ch,NULL, victim, TO_NOTVICT);
	   }
	   else
		stc("You form a dark red moon over yourself, but you resist the affects!",ch);
	}
	else
	{
	   af.type      = sn;
	   af.duration  = 20;
	   af.location  = APPLY_HIT;
	   af.bitvector = 0;
	   af.modifier  = -amount;
	   affect_to_char( victim, &af );
	   if(victim != ch)
	   {
	   	act("You form a dark red moon over $N, sending dark energy corsing over them!", ch, NULL, victim, TO_CHAR);
	   	act("$n forms a dark red moon over you, sending dark energy corsing over you!", ch, NULL, victim, TO_VICT);
	   	act("$N forms a dark red moon over $N, sending dark energy corsing over them!", ch, NULL, victim, TO_VICT);
	   }
	   else
	   	stc("You form a dark red moon over yourself, black viscous energy corsing over your body!",ch);
	   if (victim->hit > victim->max_hit) victim->hit = victim->max_hit;
	}
   	if (( victim->position > POS_STUNNED) && ( victim != ch))
   	{
	   if(victim->fighting == NULL) set_fighting (victim, ch);
	   victim->position = POS_FIGHTING;
	   if(ch->fighting == NULL) set_fighting (ch, victim);
	}
	WAIT_STATE(ch, 14);
	return;
}

void do_liquifyearth( CHAR_DATA *ch, char *argument )
{
    	char arg[MIL];
    	CHAR_DATA *victim;
//    	AFFECT_DATA af;
    	long sn;
    	argument = one_argument( argument, arg );
  
  	if(IS_NPC(ch))  return;
    	if(!IS_ELEM(ch))
    	{
           stc("Huh?\n\r",ch);
           return;
    	}
    	if(ch->pcdata->powers[ELEM_EARTH]<10)
    	{
           stc("You cannot control the earth well enough to use this ability!\n\r",ch);
           return;
    	}
    	if((sn = skill_lookup("liquifyearth")) < 0 )
    	{
           stc("We got some bugs! Report to Akuma!\n\r",ch);
           return;
    	}
	if ( arg[0] == '\0' )
    	{
	   send_to_char( "Cast Liquify Earth on who?\n\r",ch);
	   return;
    	}
	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    	{
	   send_to_char( "Who? What? Where?\n\r", ch );
	   return;
    	}
    	if (is_safe(ch,victim)) return;
    	if ( ch == victim )
    	{
           send_to_char( "Diving into quicksand would be less messy. And not quite be so... sucky.\n\r", ch );
           return;
    	}
//    	if(is_affected(victim,sn))
	if(IS_XTRA(victim, XTRA_LIQUIFY))
    	{
           stc("But they are already being slowed down by the earth!\n\r",ch);
           return;
    	}
	if (ch->mana < ch->pcdata->powers[ELEM_EARTH] * 800)
	{
	    stc("You don't have enough mana.\n\r", ch);
	    return;
	}
	ch->mana -= ch->pcdata->powers[ELEM_EARTH] * 800;
    	if( number_range(1,(char_stat(victim, STAT_STR)+char_stat(victim, STAT_DEX)))> 14){
	   act("You liquify the ground below $N, but they break free!", ch, NULL, victim, TO_CHAR);
	   act("$n liquifies the ground below you, but you break free!", ch,NULL, victim, TO_VICT);
	   act("$n liquifies the ground below $N, but they break free!.", ch,NULL, victim, TO_NOTVICT);
	}
	else 
	{
	/*
    	   af.type      = sn;
    	   af.duration  = 20;
	   af.location  = APPLY_NONE;
     	   af.bitvector = 0;
	   af.modifier  = 0;
    	   affect_to_char( victim, &af );
	*/
	   SET_BIT(victim->xtra, XTRA_LIQUIFY);
           victim->liquify_timer = 20;


	   act("You liquify the ground below $N, slowing them down.", ch, NULL, victim, TO_CHAR);
	   act("$n liquifies the ground below you, slowing you down.", ch,NULL, victim, TO_VICT);
	   act("$n liquifies the ground below $N, slowing them down..", ch,NULL, victim, TO_NOTVICT);
	}
   	if ( victim->position > POS_STUNNED)
   	{
	   if(victim->fighting == NULL) set_fighting (victim, ch);
	   victim->position = POS_FIGHTING;
	   if(ch->fighting == NULL) set_fighting (ch, victim);
	}
    	WAIT_STATE(ch, 14);
    	return;
}

void do_slow( CHAR_DATA *ch, char *argument )
{
    	char arg[MIL];
    	CHAR_DATA *victim;
//    	AFFECT_DATA af;
    	long sn;
    	argument = one_argument( argument, arg );
  
  	if(IS_NPC(ch))  return;
    	if(!IS_ELEM(ch))
    	{
           stc("Huh?\n\r",ch);
           return;
    	}
    	if(ch->pcdata->powers[ELEM_TIME]<10)
    	{
           stc("You cannot control time well enough to use this ability!\n\r",ch);
           return;
    	}
    	if((sn = skill_lookup("slow")) < 0 )
    	{
           stc("We got some bugs! Report to Akuma!\n\r",ch);
           return;
    	}
	if ( arg[0] == '\0' )
    	{
	   send_to_char( "Cast Slow on who?\n\r",ch);
	   return;
    	}
	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    	{
	   send_to_char( "Who? What? Where?\n\r", ch );
	   return;
    	}
    	if (is_safe(ch,victim)) return;
    	if ( ch == victim )
    	{
           send_to_char( "I've heard of stupid things. This is pretty bad. ;)\n\r", ch );
           return;
    	}
//    	if(is_affected(victim,sn))
	if(IS_XTRA(victim, XTRA_SLOW))
    	{
           stc("But they are already being slowed by the forces of time!\n\r",ch);
           return;
    	}
	if (ch->mana < ch->pcdata->powers[ELEM_TIME] * 800)
        {
            stc("You don't have enough mana.\n\r", ch);
            return;
        }
        ch->mana -= ch->pcdata->powers[ELEM_TIME] * 800;


    	if( number_range(1,(char_stat(victim, STAT_INT)+char_stat(victim, STAT_WIS)))> 14){
	   act("You surround $N in a pinkish halo, but it shimmers and fades away!", ch, NULL, victim, TO_CHAR);
	   act("$n surrounds you in a pinkish halo, but it shimmers and fades away!", ch,NULL, victim, TO_VICT);
	   act("$n surrounds $N in a pinkish halo, but it shimmers and fades away!.", ch,NULL, victim, TO_NOTVICT);
	}
	else 
	{
	/*
    	   af.type      = sn;
    	   af.duration  = 20;
	   af.location  = APPLY_NONE;
     	   af.bitvector = 0;
	   af.modifier  = 0;
    	   affect_to_char( victim, &af );
	*/
	   SET_BIT(victim->xtra, XTRA_SLOW);
           victim->slow_timer = 20;

	   act("You surround $N in a pinkish halo! $N slows down!", ch, NULL, victim, TO_CHAR);
	   act("$n surrounds you in a pinkish halo! You slow down!", ch,NULL, victim, TO_VICT);
	   act("$n surround $N in a pinkish halo! $N slows down!", ch,NULL, victim, TO_NOTVICT);
	}
   	if ( victim->position > POS_STUNNED)
   	{
	   if(victim->fighting == NULL) set_fighting (victim, ch);
	   victim->position = POS_FIGHTING;
	   if(ch->fighting == NULL) set_fighting (ch, victim);
	}
    	WAIT_STATE(ch, 14);
    	return;
}

void do_farsight( CHAR_DATA *ch, char *argument ) {
    	DESCRIPTOR_DATA *d;
    	ROOM_INDEX_DATA *chroom;
    	ROOM_INDEX_DATA *victimroom;
    	CHAR_DATA *victim;
    	char arg [MAX_INPUT_LENGTH];
    	char buf [MAX_STRING_LENGTH];
    	bool found = FALSE;

    	smash_tilde( argument );
    	argument = one_argument( argument, arg );
    	if (IS_NPC(ch)) return;
    	if (!IS_ELEM(ch))
    	{
	   send_to_char("Huh?\n\r",ch);
	   return;
    	}
    	if ( ch->pcdata->powers[ELEM_INFIN] < 2 )
    	{
	   send_to_char("You have not learned how to see the infinite well enough yet!\n\r",ch);
	   return;
    	}
  	if ( arg[0] == '\0' )
  	{
    	   send_to_char("You send your inner sight out at the world.\n\r",ch);
    	   act("$n closes $s eyes briefly, then opens them again, $s eyes glowing white momentarily.",ch,NULL,NULL,TO_ROOM);
	   for ( d = descriptor_list; d != NULL; d = d->next )
	   {
		if ( d->connected != CON_PLAYING ) continue;
		if ( (victim = d->character) == NULL ) continue;
		if ( IS_NPC(victim) || victim->in_room == NULL ) continue;
		if ( victim == ch ) continue;
		if ( !can_see(ch,victim) ) continue;
		if ( IS_AFFECTED(victim, AFF_POLYMORPH)) sprintf(buf,"%s: ", victim->morph);
	    	else sprintf(buf,"%s: ", victim->name);
	    	send_to_char(buf,ch);
	    	if (!IS_MORE(victim, MORE_SHIELDED) ) sprintf(buf,"%s.\n\r", victim->in_room->name);
	    	else sprintf(buf,"(shielded).\n\r");
	    	send_to_char(buf,ch);
	    	found = TRUE;
	    }
	    if (!found) send_to_char("You don't sense anyone.\n\r",ch);
		return;
    	}
	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    	{
	    send_to_char( "You can't seem to find them.\n\r", ch );
	    return;
    	}
	if (!IS_NPC(victim) && IS_MORE(victim,MORE_SHIELDED) && !IS_ITEMAFF(ch,ITEMA_VISION))
    	    {
		send_to_char("You can't seem to find them.\n\r",ch);
		if (!IS_NPC(victim) && ch != victim && IS_MORE(victim,MORE_GUARDIAN))
		{
	    	    guardian_message( victim );
	    	    act("$n just failed to use Far Sight on you.",ch,NULL,victim,TO_VICT);
		}
	    	return;
    	    }
	chroom = ch->in_room;
    	victimroom = victim->in_room;
    	char_from_room(ch);
    	char_to_room(ch,victimroom);
    	if (IS_AFFECTED(ch, AFF_SHADOWPLANE) && (!IS_AFFECTED(victim, AFF_SHADOWPLANE)))
    	{
	    REMOVE_BIT(ch->affected_by, AFF_SHADOWPLANE);
	    do_look(ch,"auto");
	    SET_BIT(ch->affected_by, AFF_SHADOWPLANE);
    	}
    	else if (!IS_AFFECTED(ch, AFF_SHADOWPLANE) && (IS_AFFECTED(victim, AFF_SHADOWPLANE)))
    	{
	    SET_BIT(ch->affected_by, AFF_SHADOWPLANE);
	    do_look(ch,"auto");
	    REMOVE_BIT(ch->affected_by, AFF_SHADOWPLANE);
    	}
    	else
	    do_look(ch,"auto");
    	char_from_room(ch);
    	char_to_room(ch,chroom);
    	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
    	{
	    guardian_message( victim );
	    act("$n just used Far Sight on you.",ch,NULL,victim,TO_VICT);
    	}
	return;
}


void do_nightblade( CHAR_DATA *ch, char *argument)
{
	char arg[MIL];
    	char buf[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	long dam;
	long hitpoints;
	int loop;
	one_argument( argument, arg);
	if(IS_NPC(ch)) return;
	if(!IS_ELEM(ch))
	{
	   stc("Huh?",ch);
	   return;
	}
	if(ch->pcdata->powers[ELEM_MOON] < 8)
	{
	   stc("You haven't learned to control Luna's shadowy light well enough for this ability.\n\r",ch);
	   return;
	}
    	if (arg[0] == '\0' && ch->fighting == NULL)
    	{
     	   send_to_char( "Who shall you hit?\n\r", ch );
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
           send_to_char( "It'd hurt. It'd suck. It's also in no shape or form advised. =)\n\r", ch );
           return;
	}
	if (ch->mana < ch->pcdata->powers[ELEM_MOON] * 450)
        {
            stc("You don't have enough mana.\n\r", ch);
            return;
        }
	if (!hurt_fighting(ch, victim)) return;

        ch->mana -= ch->pcdata->powers[ELEM_MOON] * 450;

	dam = (mod_damcap(ch, victim)*1.5);
	if (!IS_NPC(victim) && (victim->pcdata->atm > 0)){
	    dam = (dam*(100-victim->pcdata->atm))/100;
	}
        if (IS_AFFECTED(victim, AFF_SANCTUARY)&&(dam > 1)) dam -= dam / 3;

	act("You form a pair of shadowy dark blades and strike at $N, draining $S life!",ch,NULL,victim,TO_CHAR); 
	act("$n forms a pair of shadowy dark blades and strikes at $N, draining $S life!",ch,NULL,victim,TO_NOTVICT); 
	act("$n forms a pair of shadowy dark blades and strikes at you, draining your life!",ch,NULL,victim,TO_VICT);
	for( loop = 0; loop < 2; loop++)
	{
	   if(victim->hit < dam)
	    {
		victim->hit = -10;
		continue;
	    }
	    if (victim == NULL) break;
	   if ((dam >= 0) && dam <= 400)
	   {
	   	sprintf (buf,"Your night blade barely hits $N. [#g%li#n]",dam);
	   	act(buf, ch, NULL, victim, TO_CHAR);
	   	sprintf(buf,"$n's night blade barely hits you. [#r%li#n]",dam);
           	act(buf, ch, NULL, victim, TO_VICT);
	   	act("$n's night blade barely hits $N.", ch,NULL, victim, TO_NOTVICT);
	   }
	   if (dam >=401 && dam <= 900)
	   {
		sprintf(buf, "Your night blade strikes $N. [#g%li#n]",dam);
		act(buf, ch, NULL, victim, TO_CHAR);
		sprintf(buf, "$n's nightblade strikes you. [#r%li#n]",dam);
		act(buf, ch, NULL, victim, TO_VICT);
		act("$n's night blade strikes $N.", ch, NULL, victim, TO_NOTVICT);
	   }
	   if (dam >=901 && dam <= 1800)
	   {
		sprintf(buf, "Your night blade strikes $N hard. [#g%li#n]",dam);
		act(buf, ch, NULL, victim, TO_CHAR);
		sprintf(buf, "$n's nightblade strikes you hard. [#r%li#n]",dam);
		act(buf, ch, NULL, victim, TO_VICT);
		act("$n's night blade strikes $N hard.", ch, NULL, victim, TO_NOTVICT);
	   }
	   if (dam >=1801 && dam <= 2800)
	   {
		sprintf(buf, "Your night blade strikes $N hard! [#g%li#n]",dam);
		act(buf, ch, NULL, victim, TO_CHAR);
		sprintf(buf, "$n's nightblade strikes you hard! [#r%li#n]",dam);
		act(buf, ch, NULL, victim, TO_VICT);
		act("$n's night blade strikes $N hard!", ch, NULL, victim, TO_NOTVICT);
	   }
	   if (dam >=2801)
	   {
		sprintf(buf, "Your night blade strikes $N *REALLY* hard. [#g%li#n]",dam);
		act(buf, ch, NULL, victim, TO_CHAR);
		sprintf(buf, "$n's nightblade strikes you *REALLY* hard. [#r%li#n]",dam);
		act(buf, ch, NULL, victim, TO_VICT);
		act("$n's night blade strikes $N *REALLY* hard.", ch, NULL, victim, TO_NOTVICT);
	   }
 	   hurt_person(ch, victim, dam);
	   hitpoints = (dam*ch->pcdata->powers[ELEM_MOON]*(number_range(1,5)+15))/1000;
	   ch->hit += hitpoints;
	   if (ch->hit> ch->max_hit) ch->hit = ch->max_hit;
	}
	WAIT_STATE(ch, 13);
	return;
}
