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



void do_pset( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg1[MIL];
    char arg2[MIL];
    char arg3[MIL];
    char arg4[MIL];
    long value, value2;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    argument = one_argument( argument, arg4 );

    if ( arg1[0] == '\0' || arg2[0] == '\0')
    {
	stc("Syntax: pset <victim> <area> <value1> <value2>\n\r", ch);
	stc("Area being one of:\n\r", ch);
	stc("  quest stat beast cps status rank gen stance class honor\n\r", ch);
	return;
    }

    if ((victim = get_char_world(ch, arg1)) == NULL)
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if (IS_NPC(victim))
    {
	stc("You may not set NPCs.\n\r",ch);
	return;
    }

    /*
     * Snarf the value (which need not be numeric).
     */
    value = is_number( arg3 ) ? atoi( arg3 ) : -1;
    value2 = is_number( arg4 ) ? atoi( arg4 ) : -1;

    /*
     * Set something.
     */
    if (!str_cmp(arg2, "class"))
    {
	if ( victim->class != CLASS_NONE )
	{
	    send_to_char( "They already have a class.\n\r", ch );
	    return;
	}

	if (!IS_AWA(victim))
	{
	    send_to_char( "You can only do this to awakened.\n\r", ch );
	    return;
	}

	if ( !str_cmp( arg3, "vampire" ) )
	{
	    victim->class = CLASS_VAMPIRE;
	    victim->vampgen = 9;
	    update_gen(victim);
            victim->pcdata->powers[DISC_ANIMALISM] = -1;
            victim->pcdata->powers[DISC_AUSPEX] = -1;
            victim->pcdata->powers[DISC_CELERITY] = -1;
            victim->pcdata->powers[DISC_DOMINATE] = -1;
            victim->pcdata->powers[DISC_FORTITUDE] = -1;
            victim->pcdata->powers[DISC_OBFUSCATE] = -1;
            victim->pcdata->powers[DISC_POTENCE] = -1;
            victim->pcdata->powers[DISC_PRESENCE] = -1;
	    victim->pcdata->cland[0] = -1;
	    victim->pcdata->cland[1] = -1;
	    victim->pcdata->cland[2] = -1;
	    send_to_char("You are now a Vampire.\n\r",victim);
	}
	else if ( !str_cmp( arg3, "werewolf" ) )
	{
	    victim->class = CLASS_WEREWOLF;
	    victim->pcdata->rank = 1;
	    victim->vampgen = 6;
	    victim->pcdata->tgift[0] = -1;
	    victim->pcdata->tgift[1] = -1;
	    victim->pcdata->tgift[2] = -1;
	    send_to_char("You are now a Werewolf.\n\r",victim);
	}	
	else if ( !str_cmp( arg3, "sorcerer" ) )
	{
	    if ( victim->level < LEV_SOR ) victim->level = LEV_SOR;
	    victim->class = CLASS_SORCERER;
	    send_to_char("You are now an Sorcerer.\n\r",victim);
	}
	else if (!str_cmp(arg3, "mage"))
	{
	    victim->class = CLASS_MAGE;
	    send_to_char("You are now a Mage.\n\r",victim);
	}
	else if ( !str_cmp( arg3, "monk" ))
	{
	    victim->class = CLASS_MONK;
	    victim->vampgen = 4;
	    stc("You are now a Monk.\nr\r", victim);
        }
	else if ( !str_cmp( arg3, "dragon" ) )
	{
	    victim->class = CLASS_DRAGON;
	    victim->vampgen = 12;
	    send_to_char("You are now a Dragon.\n\r",victim);
	}
	else if ( !str_cmp( arg3, "highlander" ) )
	{
	    victim->class = CLASS_HIGHLANDER;
            send_to_char("You are now a Highlander.\n\r",victim);
        }
	else if ( !str_cmp( arg3, "ninja" ) )
	{
	    victim->class = CLASS_NINJA;
	    victim->vampgen = 4;
	    send_to_char("You are now a Ninja.\n\r",victim);
	}
	else if ( !str_cmp( arg3, "drow" ) )
	{
	    victim->class = CLASS_DROW;
	    victim->vampgen = 5;
	    send_to_char("You are now a Drow.\n\r",victim);
	}
	else if ( !str_cmp( arg3, "elementalist" ) )
	{
	    victim->class = CLASS_ELEM;
	    stc("You are now an Elementalist.\n\r",victim);
	}
	else
	{
	    do_pset( ch, "" );
	    return;
	}
    }
    else if (!str_cmp(arg2,"color"))
    {
	if (!IS_DRAGON(victim))
	{
	    stc("Only for Dragons.\n\r",ch);
	    return;
	}

	if (!str_cmp(arg3, "bronze")) value = PURPLE_MAGIC;
	else if (!str_cmp(arg3, "red")) value = RED_MAGIC;
	else if (!str_cmp(arg3, "blue")) value = BLUE_MAGIC;
	else if (!str_cmp(arg3, "silver")) value = GREEN_MAGIC;
	else if (!str_cmp(arg3, "gold")) value = YELLOW_MAGIC;
	else if (!str_cmp(arg3, "black")) value = BLACK_MAGIC;
	else
	{
	    stc("Pick bronze, red, blue, silver, gold, or black please.\n\r", ch);
	    return;
	}
	victim->pcdata->dragcol = value;
    }
    else if (!str_cmp(arg2, "stat"))
    {
	if (value2 < 1 || value2 > 16)
	{
	    stc("Stat range: 1 to 16.\n\r", ch);
	    return;
	}

	if (!str_cmp(arg3, "str")) value = STAT_STR;
	else if (!str_cmp(arg3, "dex")) value = STAT_DEX;
	else if (!str_cmp(arg3, "con")) value = STAT_CON;
	else if (!str_cmp(arg3, "int")) value = STAT_INT;
	else if (!str_cmp(arg3, "wis")) value = STAT_WIS;
	else if (!str_cmp(arg3, "all"))
	{
	    victim->stats[0] = value2;
	    victim->stats[1] = value2;
	    victim->stats[2] = value2;
	    victim->stats[3] = value2;
	    victim->stats[4] = value2;
	    stc("Ok.\n\r", ch);
	    return;
	}
	else
	{
	    do_pset(ch, "");
	    return;
	}

	victim->stats[value] = value2;
    }
    else if ( !str_cmp( arg2, "beast" ) )
    {
	if ( value < 0 || value > 100 )
	{
	    send_to_char( "Beast range is 0 to 100.\n\r", ch );
	    return;
	}

	victim->beast = value;
    }
    else if ( !str_cmp( arg2, "cps" ) )
    {
        if ( value < 0 || value > 10000000 ) 
        {
            send_to_char( "CP range is 0 to 10000000.\n\r", ch );
            return;
        }
            
	victim->pcdata->classpoints[CP_CURRENT] = value;  
    }
    else if ( !str_cmp( arg2, "rank" ) )
    {
	if ( value < 1 || value > 6 )
	{
	    stc( "Rank range is 1 to 6", ch);
	    return;
	}
	victim->pcdata->rank = value;
    }
    else if ( !str_cmp( arg2, "gen" ) )
    {
        if ( value < 1 || value > 13 )
        {
            stc( "Gen range is 1 to 13", ch);
            return;
        }
	victim->vampgen = value;
    }
    else if ( !str_cmp( arg2, "status" ) )
    {
        if ( value < 0 || value > 25 )
        {
            send_to_char( "Status range is 0 to 25.\n\r", ch );
            return;
        }
	victim->status = value;
    }
    else if ( !str_cmp( arg2, "quest" ) )
    {
	if ( value < 0 || value > 1000000 )
	{
	    send_to_char( "Quest range is 0 to 1000000.\n\r", ch );
	    return;
	}
	victim->pcdata->quest = value;
    }
    else if ( !str_cmp( arg2, "stance"))
    {
	if (value2 < 0 || value2 > 200)
	{
	    stc("Stance range is 0 to 200.\n\r", ch);
	    return;
	}
	if (!str_cmp(arg3, "viper")) value = STANCE_VIPER;
	else if (!str_cmp(arg3, "crane")) value = STANCE_CRANE;
	else if (!str_cmp(arg3, "crab")) value = STANCE_CRAB;
	else if (!str_cmp(arg3, "mongoose")) value = STANCE_MONGOOSE;
	else if (!str_cmp(arg3, "bull")) value = STANCE_BULL;
	else if (!str_cmp(arg3, "mantis")) value = STANCE_MANTIS;
	else if (!str_cmp(arg3, "dragon")) value = STANCE_DRAGON;
	else if (!str_cmp(arg3, "tiger")) value = STANCE_TIGER;
	else if (!str_cmp(arg3, "monkey")) value = STANCE_MONKEY;
	else if (!str_cmp(arg3, "swallow")) value = STANCE_SWALLOW;
	else if (!str_cmp(arg3, "all"))
	{
	    victim->stance[1] = value2;
	    victim->stance[2] = value2;
	    victim->stance[3] = value2;
	    victim->stance[4] = value2;
	    victim->stance[5] = value2;
	    victim->stance[6] = value2;
	    victim->stance[7] = value2;
	    victim->stance[8] = value2;
	    victim->stance[9] = value2;
	    victim->stance[10] = value2;
	    send_to_char("Ok.\n\r",ch);
	    return;
	}
	else
	{
	    do_pset(ch, "");
	    return;
	}

	victim->stance[value] = value2;
    }
    else if (!str_cmp(arg2, "honor"))
    {
	if (!IS_SET(victim->act, PLR_HONOR))
	{
	    SET_BIT(victim->act, PLR_HONOR);
	    stc("Honor set.\n\r", ch);
	    stc("You may now use the honor channel.\n\r", victim);
	}
	else
	{
	    REMOVE_BIT(victim->act, PLR_HONOR);
	    stc("Honor removed.\n\r", ch);
	    stc("You may no longer use the honor channel.\n\r", victim);
	}
    }
    else
    {
	do_pset( ch, "" );
	return;
    }
    stc("Ok.\n\r", ch);
    return;
}

