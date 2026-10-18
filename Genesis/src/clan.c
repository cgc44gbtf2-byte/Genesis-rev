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
#include <malloc.h>
#include "merc.h"

/*
 * Local functions.
 */


/* Need to get rid of those flames somehow - KaVir */
void do_smother( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MIL];

    one_argument(argument, arg);

    if (arg[0] == '\0' || (victim = get_char_room(ch, arg)) == NULL) victim = ch;

    if (!IS_AFFECTED(victim, AFF_FLAMING))
    {
	send_to_char( "But they are not on fire!\n\r", ch );
	return;
    }

    act( "You manage to smother the flames around $M!",  ch, NULL, victim, TO_CHAR    );
    act( "$n manages to smother the flames around you!", ch, NULL, victim, TO_VICT    );
    act( "$n manages to smother the flames around $N!",  ch, NULL, victim, TO_NOTVICT );
    REMOVE_BIT(victim->affected_by, AFF_FLAMING);

    if (ch->position == POS_FIGHTING) WAIT_STATE(ch, 18);
    return;
}

/* Loads of Vampire powers follow.  KaVir */
void do_fangs( CHAR_DATA *ch, char *argument )
{
    char      arg [MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch) && !IS_WEREWOLF(ch) && !IS_DRAGON(ch))
    {
	send_to_char("Huh?\n\r",ch);
	return;
    }

    if (IS_DRAGON(ch) && (!IS_DRAGAFF(ch,DRAG_FANGS) || !IS_POLYAFF(ch,POLY_DRAGONFORM)))
    {
	stc("You must have Fangs and be in Dragonform.\n\r",ch);
	return;
    }
    if (IS_VAMPIRE(ch) && ch->pcdata->wolf > 0)
    {
	send_to_char("Your beast won't let you retract your fangs.\n\r",ch);
	return;
    }

    if (IS_VAMPAFF(ch,VAM_FANGS) )
    {
	send_to_char("Your fangs slide back into your gums.\n\r",ch);
	act("$n's fangs slide back into $s gums.", ch, NULL, NULL, TO_ROOM);
	REMOVE_BIT(ch->vampaff, VAM_FANGS);
	return;
    }
    send_to_char("Your fangs extend out of your gums.\n\r",ch);
    act("A pair of razor sharp fangs extend from $n's mouth.", ch, NULL, NULL, TO_ROOM);
    SET_BIT(ch->vampaff, VAM_FANGS);
    return;
}


void do_claws( CHAR_DATA *ch, char *argument )
{
    char arg [MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch) && !IS_WEREWOLF(ch) && !IS_DRAGON(ch))
    {
	send_to_char("Huh?\n\r",ch);
	return;
    }
    if (IS_VAMPIRE(ch) && get_disc(ch,DISC_PROTEAN) < 2)
    {
	send_to_char("You require level 2 Protean to use Claws.\n\r",ch);
	return;
    }
    if (IS_DRAGON(ch) && (!IS_DRAGAFF(ch,DRAG_CLAWS) || !IS_POLYAFF(ch,POLY_DRAGONFORM)))
    {
	stc("You need to be in Dragonform with the Claws skill to use claws.\n\r",ch);
	return;
    }

    if (IS_VAMPAFF(ch,VAM_CLAWS) )
    {
	if (IS_VAMPIRE(ch)) 
	{
	    if (ch->pcdata->wolf > 0)
	    {
		stc("Your beast won't let you retract your claws.\n\r",ch);
		return;
	    }
	    send_to_char("Your claws slide back under your nails.\n\r",ch);
	    act("$n's claws slide back under $s nails.", ch, NULL, NULL, TO_ROOM);
	}
	else if (IS_WEREWOLF(ch) && get_auspice(ch, AUSPICE_AHROUN) >= 3)
	{
	    send_to_char("Your gleaming silver talons slide back into your fingers.\n\r",ch);
	    act("$n's gleaming silver talons slide back into $s fingers.",ch,NULL,NULL,TO_ROOM);
	}
	else
	{
	    send_to_char("Your talons slide back into your fingers.\n\r",ch);
	    act("$n's talons slide back into $s fingers.",ch,NULL,NULL,TO_ROOM);
	}
	REMOVE_BIT(ch->vampaff, VAM_CLAWS);
	update_style(ch);
	return;
    }

    if (IS_VAMPIRE(ch)) 
    {
	send_to_char("Sharp claws extend from under your finger nails.\n\r",ch);
	act("Sharp claws extend from under $n's finger nails.", ch, NULL, NULL, TO_ROOM);
    }
    else if (IS_WEREWOLF(ch) && get_auspice(ch, AUSPICE_AHROUN) >= 3) 
    {
	send_to_char("Gleaming silver talons extend from your fingers.\n\r",ch);
	act("Gleaming silver talons extend from $n's fingers.",ch,NULL,NULL,TO_ROOM);
    }
    else
    {
	send_to_char("Razor sharp talons extend from your fingers.\n\r",ch);
	act("Razor sharp talons extend from $n's fingers.",ch,NULL,NULL,TO_ROOM);
    }
    SET_BIT(ch->vampaff, VAM_CLAWS);
    update_style(ch);
    return;
}

void do_nightsight( CHAR_DATA *ch, char *argument )
{
    char      arg [MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch) && !IS_WEREWOLF(ch) && !IS_DROW(ch) && !IS_DRAGON(ch) && !IS_MAGE(ch) )
    {
	send_to_char("Huh?\n\r",ch);
	return;
    }

    if (IS_WEREWOLF(ch) && get_breed(ch, BREED_METIS) < 3)
    {
	send_to_char("You require the level 1 Metis power to use Eyes of the Cat.\n\r",ch);
	return;
    }
    else if (IS_VAMPIRE(ch) && get_disc(ch,DISC_PROTEAN) < 1)
    {
	send_to_char("You require level 1 Protean to use Gleam of Red Eyes.\n\r",ch);
	return;
    }
    else if (IS_DROW(ch) && !get_drow(ch, DROW_NIGHT))
    {
        stc("You need to obtain darkvision first.\n\r",ch);
        return;
    }
    else if (IS_DRAGON(ch) && !IS_DRAGAFF(ch, DRAG_NIGHT))
    {
	stc("You need to learn Nightsight first.\n\r",ch);
	return;
    }
    else if (IS_MAGE(ch) && ch->spheres[MFOR] < 1)
    {
	stc("You do not have enough knowledge of the forces sphere.\n\r",ch);
	return;
    }

    if (IS_VAMPAFF(ch, VAM_NIGHTSIGHT) )
    {
	if (IS_VAMPIRE(ch) && ch->pcdata->wolf > 0)
	{
	    stc("Not while your beast is in control.\n\r",ch);
	    return;
	}
	if (IS_WEREWOLF(ch) && get_disc(ch, DISC_PROTEAN) < 1)
	{
	    send_to_char("The green glow in your eyes fades.\n\r",ch);
	    act("The green glow in $n's eyes fades.", ch, NULL, NULL, TO_ROOM);
	}
	else
	{
	    send_to_char("The red glow in your eyes fades.\n\r",ch);
	    act("The red glow in $n's eyes fades.", ch, NULL, NULL, TO_ROOM);
	}
	REMOVE_BIT(ch->vampaff, VAM_NIGHTSIGHT);
	return;
    }
    if (IS_WEREWOLF(ch) && get_disc(ch, DISC_PROTEAN) < 1)
    {
	send_to_char("Your eyes start glowing green.\n\r",ch);
	act("$n's eyes start glowing green.", ch, NULL, NULL, TO_ROOM);
    }
    else
    {
	send_to_char("Your eyes start glowing red.\n\r",ch);
	act("$n's eyes start glowing red.", ch, NULL, NULL, TO_ROOM);
    }
    SET_BIT(ch->vampaff, VAM_NIGHTSIGHT);
    return;
}

void do_outcast( CHAR_DATA *ch, char *argument )
{
    clan_legacy(ch, "kick", argument);
}

void do_clanname( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MIL];

    argument = one_argument(argument, arg);

    if (IS_NPC(ch)) return;

    if (arg[0] == '\0')
    {
	stc("Whose clan do you wish to change or name?\n\r", ch);
	return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL)
    {
	stc("They aren't here.\n\r", ch);
	return;
    }

    if (IS_NPC(victim))
    {
	stc("Only on players.\n\r", ch);
	return;
    }

    smash_tilde(argument);
    if (strlen(argument) < 0 || strlen(argument) > 25)
    {
	stc("Clan name should be between 0 and 25 letters long.\n\r", ch);
	stc("Leave a blank to remove clan.\n\r", ch);
	return;
    }

    if (clan_block_name_change(victim, argument))
    {
        stc("Use managed clan commands for registered clan identities.\n\r", ch);
        return;
    }
    free_string(victim->clan);
    victim->clan = str_dup(argument);
    stc("Clan set.\n\r", ch);
    if (ch != victim) stc("Clan set.\n\r", victim);
    return;
}

void do_clandisc( CHAR_DATA *ch, char *argument )
{
    char      arg [MAX_INPUT_LENGTH];
    char      buf [MAX_INPUT_LENGTH];
    long       improve, sn, col;

    argument = one_argument( argument, arg );

    if ( IS_NPC(ch) ) return;

    if ( !IS_VAMPIRE(ch) || !str_cmp(ch->clan, "Panders") || !str_cmp(ch->clan, "Caitiff") )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char("--------------------------------------------------------------------------------\n\r",ch);
	send_to_char("                               -= Disciplines =-\n\r",ch);
	send_to_char("--------------------------------------------------------------------------------\n\r",ch);
	if ( ch->vampgen <= 2 )
	{
	    /* Clan discipline */
	    improve = ch->pcdata->cland[0];
	    sprintf( buf, "Clan Discipline: %s.\n\r", disc_name(improve) );
	    send_to_char( buf, ch );

	    /* Personal discipline 1 */
	    improve = ch->pcdata->cland[1];
	    sprintf( buf, "Personal Discipline: %s.\n\r", disc_name(improve) );
	    send_to_char( buf, ch );

	    /* Personal discipline 2 */
	    improve = ch->pcdata->cland[2];
	    sprintf( buf, "Personal Discipline: %s.\n\r", disc_name(improve) );
	    send_to_char( buf, ch );
	}
	else
	{
	    /* Clan discipline */
	    improve = ch->pcdata->cland[0];
	    sprintf( buf, "Clan Discipline: %s.\n\r", disc_name(improve) );
	    send_to_char( buf, ch );

	    /* Sires discipline */
	    improve = ch->pcdata->cland[1];
	    sprintf( buf, "Sires Discipline: %s.\n\r", disc_name(improve) );
	    send_to_char( buf, ch );

	    /* Personal discipline */
	    improve = ch->pcdata->cland[2];
	    sprintf( buf, "Personal Discipline: %s.\n\r", disc_name(improve) );
	    send_to_char( buf, ch );
	}
	send_to_char("--------------------------------------------------------------------------------\n\r",ch);
	if ( ch->pcdata->cland[2] >= 0 ) return;
	send_to_char("Please select another discipline from the list below.\n\r",ch);
	send_to_char("--------------------------------------------------------------------------------\n\r",ch);
	col    = 0;
	for ( sn = 0; sn <= DISC_MAX; sn++ )
	{
	    if ( ch->pcdata->powers[sn] != 0 ) continue;
	    sprintf( buf, "%-15s           ", disc_name( sn ) );
	    send_to_char( buf, ch );
	    if ( ++col % 3 == 0 ) send_to_char( "\n\r", ch );
	}
	if ( col % 3 != 0 ) send_to_char( "\n\r", ch );
	send_to_char("--------------------------------------------------------------------------------\n\r",ch);
	return;
    }

    if (!str_cmp(arg,"animalism")) 
	improve = DISC_ANIMALISM;
    else if (!str_cmp(arg,"auspex")) 
	improve = DISC_AUSPEX;
    else if (!str_cmp(arg,"celerity")) 
	improve = DISC_CELERITY;
    else if (!str_cmp(arg,"chimerstry")) 
	improve = DISC_CHIMERSTRY;
    else if (!str_cmp(arg,"dominate")) 
	improve = DISC_DOMINATE;
    else if (!str_cmp(arg,"fortitude")) 
	improve = DISC_FORTITUDE;
    else if (!str_cmp(arg,"necromancy")) 
	improve = DISC_NECROMANCY;
    else if (!str_cmp(arg,"obfuscate")) 
	improve = DISC_OBFUSCATE;
    else if (!str_cmp(arg,"obtenebration")) 
	improve = DISC_OBTENEBRATION;
    else if (!str_cmp(arg,"potence")) 
	improve = DISC_POTENCE;
    else if (!str_cmp(arg,"presence")) 
	improve = DISC_PRESENCE;
    else if (!str_cmp(arg,"protean")) 
	improve = DISC_PROTEAN;
    else if (!str_cmp(arg,"quietus")) 
	improve = DISC_QUIETUS;
    else if (!str_cmp(arg,"serpentis")) 
	improve = DISC_SERPENTIS;
    else if (!str_cmp(arg,"thaumaturgy")) 
	improve = DISC_THAUMATURGY;
    else if (!str_cmp(arg,"vicissitude")) 
	improve = DISC_VICISSITUDE;
    else
    {
	send_to_char("You know of no such discipline.\n\r",ch);
	return;
    }

    if (ch->pcdata->cland[0] == improve ||
	ch->pcdata->cland[1] == improve ||
	ch->pcdata->cland[2] == improve)
    {
	send_to_char("You already know that discipline.\n\r",ch);
	return;
    }

    if ( ch->vampgen < 3 || ch->class != CLASS_VAMPIRE )
    {
	if ( ch->pcdata->cland[0] < 0 )
	    ch->pcdata->cland[0] = improve;
	else if ( ch->pcdata->cland[1] < 0 )
	    ch->pcdata->cland[1] = improve;
	else if ( ch->pcdata->cland[2] < 0 )
	    ch->pcdata->cland[2] = improve;
	else
	{
	    send_to_char("All of your discipline slots are full.\n\r",ch);
	    return;
	}
	ch->pcdata->powers[improve] = -1;
	send_to_char("Ok.\n\r",ch);
    }
    else
    {
	if ( ch->pcdata->cland[2] < 0 )
	    ch->pcdata->cland[2] = improve;
	else
	{
	    send_to_char("Your discipline slot is full.\n\r",ch);
	    return;
	}
	ch->pcdata->powers[improve] = -1;
	send_to_char("Ok.\n\r",ch);
    }

    return;
}

void do_mask( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char      arg [MAX_INPUT_LENGTH];
    char      buf [MAX_STRING_LENGTH];

    argument = one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch))
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if (IS_VAMPIRE(ch) && get_disc(ch,DISC_OBFUSCATE) < 2)
    {
	send_to_char("You require level 2 Obfuscate to use Mask of a Thousand Faces.\n\r",ch);
	return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char( "Change to look like whom?\n\r", ch );
	return;
    }

    if (IS_AFFECTED(ch,AFF_POLYMORPH) && !IS_VAMPAFF(ch,VAM_DISGUISED))
    {
	send_to_char( "Not while polymorphed.\n\r", ch );
	return;
    }

    if (IS_CRAFT(ch, CRAFT_MALLEABLE))
    {
	send_to_char( "You are unable to change your appearance.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( IS_BUI(victim) && victim != ch )
    {
	send_to_char( "You can only mask avatars or lower.\n\r", ch );
	return;
    }
    if ( ch == victim )
    {
	if (!IS_AFFECTED(ch,AFF_POLYMORPH) && !IS_VAMPAFF(ch,VAM_DISGUISED))
	{
	    send_to_char( "You already look like yourself!\n\r", ch );
	    return;
	}
	if (!IS_POLYAFF(ch, POLY_MASK))
	{
	    send_to_char("Nothing happens.\n\r",ch);
	    return;
	}
	sprintf(buf,"$n's form shimmers and transforms into a clone of %s.",ch->name);
	act(buf,ch,NULL,NULL,TO_ROOM);
	REMOVE_BIT(ch->affected_by, AFF_POLYMORPH);
	REMOVE_BIT(ch->vampaff, VAM_DISGUISED);
	REMOVE_BIT(ch->polyaff, POLY_MASK);
    	free_string( ch->morph );
    	ch->morph = str_dup( "" );
	act("Your form shimmers and transforms into a clone of $n.",ch,NULL,NULL,TO_CHAR);
	return;
    }
    if (IS_VAMPAFF(ch,VAM_DISGUISED))
    {
	if (!IS_POLYAFF(ch, POLY_MASK))
	{
	    send_to_char("Nothing happens.\n\r",ch);
	    return;
	}
	act("Your form shimmers and transforms into a clone of $N.",ch,NULL,victim,TO_CHAR);
	act("$n's form shimmers and transforms into a clone of you.",ch,NULL,victim,TO_VICT);
	act("$n's form shimmers and transforms into a clone of $N.",ch,NULL,victim,TO_NOTVICT);
    	free_string( ch->morph );
    	ch->morph = str_dup( victim->name );
	SET_BIT(ch->polyaff, POLY_MASK);
	return;
    }
    act("Your form shimmers and transforms into a clone of $N.",ch,NULL,victim,TO_CHAR);
    act("$n's form shimmers and transforms into a clone of you.",ch,NULL,victim,TO_VICT);
    act("$n's form shimmers and transforms into a clone of $N.",ch,NULL,victim,TO_NOTVICT);
    SET_BIT(ch->affected_by, AFF_POLYMORPH);
    SET_BIT(ch->vampaff, VAM_DISGUISED);
    SET_BIT(ch->polyaff, POLY_MASK);
    free_string( ch->morph );
    ch->morph = str_dup( victim->name );
    return;
}

void do_change( CHAR_DATA *ch, char *argument )
{
    char arg [MAX_INPUT_LENGTH]; 
    char buf [MAX_STRING_LENGTH]; 
    argument = one_argument( argument, arg ); 

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch))
    {
	send_to_char("Huh?\n\r",ch);
	return;
    }

    if (get_disc(ch,DISC_PROTEAN) < 4)
    {
	send_to_char("You require level 4 Protean to Change Shape.\n\r",ch);
	return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char( "You can change between 'human', 'bat', 'wolf' and 'mist' forms.\n\r", ch );
	return;
    }

    if ( !str_cmp(arg,"bat") )
    {
	if (IS_AFFECTED(ch, AFF_POLYMORPH))
	{
	    send_to_char( "You can only polymorph from human form.\n\r", ch );
	    return;
	}
	if (ch->stance[0] != -1) do_stance(ch,"");
	SET_BIT(ch->polyaff, POLY_BAT);
	reset_form_stats(ch);
	act( "You transform into bat form.", ch, NULL, NULL, TO_CHAR );
	act( "$n transforms into a bat.", ch, NULL, NULL, TO_ROOM );
	SET_BIT(ch->vampaff, VAM_FLYING);
	SET_BIT(ch->vampaff, VAM_SONIC);
	SET_BIT(ch->vampaff, VAM_CHANGED);
	SET_BIT(ch->affected_by, AFF_POLYMORPH);
	sprintf(buf, "A black vampire bat");
    	free_string( ch->morph );
    	ch->morph = str_dup( buf );
	do_autosave(ch, "");
	return;
    }
    else if ( !str_cmp(arg,"wolf") )
    {
	if (IS_AFFECTED(ch, AFF_POLYMORPH))
	{
	    send_to_char( "You can only polymorph from human form.\n\r", ch );
	    return;
	}
	if (ch->stance[0] != -1) do_stance(ch,"");
	act( "You transform into wolf form.", ch, NULL, NULL, TO_CHAR );
	act( "$n transforms into a dire wolf.", ch, NULL, NULL, TO_ROOM );
	SET_BIT(ch->polyaff, POLY_WOLF);
	reset_form_stats(ch);
	SET_BIT(ch->affected_by, AFF_POLYMORPH);
	SET_BIT(ch->vampaff, VAM_CHANGED);
	sprintf(buf, "A dire wolf");
    	free_string( ch->morph );
    	ch->morph = str_dup( buf );
	do_autosave(ch, "");
	return;
    }
    else if ( !str_cmp(arg,"mist") )
    {
	if (IS_AFFECTED(ch, AFF_POLYMORPH))
	{
	    send_to_char( "You can only polymorph from human form.\n\r", ch );
	    return;
	}
	if (ch->stance[0] != -1) do_stance(ch,"");
	act( "You transform into mist form.", ch, NULL, NULL, TO_CHAR );
	act( "$n transforms into a white mist.", ch, NULL, NULL, TO_ROOM );
	if (IS_EXTRA(ch, TIED_UP))
	{
	    act("The ropes binding you fall through your ethereal form.",ch,NULL,NULL,TO_CHAR);
	    act("The ropes binding $n fall through $s ethereal form.",ch,NULL,NULL,TO_ROOM);
	    REMOVE_BIT(ch->extra, TIED_UP);
	    REMOVE_BIT(ch->extra, GAGGED);
	    REMOVE_BIT(ch->extra, BLINDFOLDED);
	}
	if (is_affected(ch, gsn_web))
	{
	    act("The webbing entrapping $n falls through $s ethereal form.",ch,NULL,NULL,TO_ROOM);
	    send_to_char("The webbing entrapping you falls through your ethereal form.\n\r",ch);
	    affect_strip(ch, gsn_web);
	}
	if (IS_AFFECTED(ch, AFF_WEBBED))
	{
	    act("The webbing entrapping $n falls through $s ethereal form.",ch,NULL,NULL,TO_ROOM);
	    send_to_char("The webbing entrapping you falls through your ethereal form.\n\r",ch);
	    REMOVE_BIT(ch->affected_by, AFF_WEBBED);
	}
	if (is_affected(ch, gsn_tendrils))
	{
	    act("The tendrils of darkness entrapping $n fall through $s ethereal form.",ch,NULL,NULL,TO_ROOM);
	    send_to_char("The tendrils of darkness entrapping you fall through your ethereal form.\n\r",ch);
	    affect_strip(ch, gsn_tendrils);
	}
	if (IS_AFFECTED(ch, AFF_SHADOW))
	{
	    act("The tendrils of darkness entrapping $n fall through $s ethereal form.",ch,NULL,NULL,TO_ROOM);
	    send_to_char("The tendrils of darkness entrapping you fall through your ethereal form.\n\r",ch);
	    REMOVE_BIT(ch->affected_by, AFF_SHADOW);
	}
	SET_BIT(ch->polyaff, POLY_MIST);
	reset_form_stats(ch);
	SET_BIT(ch->vampaff, VAM_CHANGED);
	SET_BIT(ch->affected_by, AFF_POLYMORPH);
	SET_BIT(ch->affected_by, AFF_ETHEREAL);
	sprintf(buf, "A white mist");
    	free_string( ch->morph );
    	ch->morph = str_dup( buf );
	do_autosave(ch, "");
	return;
    }
    else if ( !str_cmp(arg,"human") )
    {
	if (!IS_AFFECTED(ch, AFF_POLYMORPH))
	{
	    send_to_char( "You are already in human form.\n\r", ch );
	    return;
	}
	if (IS_VAMPAFF(ch, VAM_CHANGED) && IS_POLYAFF(ch, POLY_BAT))
	{
	    REMOVE_BIT(ch->vampaff, VAM_FLYING);
	    REMOVE_BIT(ch->vampaff, VAM_SONIC);
	    REMOVE_BIT(ch->polyaff, POLY_BAT);
	}
	else if (IS_VAMPAFF(ch, VAM_CHANGED) && IS_POLYAFF(ch, POLY_WOLF))
	    REMOVE_BIT(ch->polyaff, POLY_WOLF);
	else if (IS_VAMPAFF(ch, VAM_CHANGED) && IS_POLYAFF(ch, POLY_MIST))
	{
	    REMOVE_BIT(ch->polyaff, POLY_MIST);
	    REMOVE_BIT(ch->affected_by, AFF_ETHEREAL);
	}
	else
	{
	    /* In case they try to change to human from a non-vamp form */
	    send_to_char( "You seem to be stuck in this form.\n\r", ch );
	    return;
	}
	act( "You transform into human form.", ch, NULL, NULL, TO_CHAR );
	act( "$n transforms into human form.", ch, NULL, NULL, TO_ROOM );
	REMOVE_BIT(ch->affected_by, AFF_POLYMORPH);
	REMOVE_BIT(ch->vampaff, VAM_CHANGED);
	reset_form_stats(ch);
  	free_string( ch->morph );
    	ch->morph = str_dup( "" );
	do_autosave(ch, "");
	return;
    }
    else
	send_to_char( "You can change between 'human', 'bat', 'wolf' and 'mist' forms.\n\r", ch );
    return;
}

void do_vampire( CHAR_DATA *ch, char *argument )
{
    char      arg [MAX_INPUT_LENGTH];
    argument = one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_EXTRA(ch, EXTRA_VAMPIRE))
    {
	send_to_char("You lower your defences.\n\r",ch);
	SET_BIT(ch->extra, EXTRA_VAMPIRE);
	return;
    }
    send_to_char("You raise your defences.\n\r",ch);
    REMOVE_BIT(ch->extra, EXTRA_VAMPIRE);
    return;
}

void do_shadowplane( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];

    argument = one_argument(argument, arg);

    if (IS_NPC(ch)) return;
    if (!IS_SORCERER(ch) && !IS_WEREWOLF(ch) && ch->in_room->vnum != ROOM_VNUM_TEMPLE)
    {
	stc("You may only transfer to the Umbra at Temple.\n\r", ch);
	return;
    }

    if (ch->fight_timer > 0 && ch->in_room->vnum != ROOM_VNUM_TEMPLE)
    {
	stc("Not with a fight timer.\n\r", ch);
	return;
    }

    if (IS_AFFECTED(ch, AFF_DEEPUMBRA))
    {
	stc("Not while in the deep umbra.\n\r", ch);
	return;
    }

    if (!IS_AFFECTED(ch, AFF_SHADOWPLANE))
    {
	stc("You fade into the Umbra.\n\r", ch);
	act("$n slowly fades out of existance.", ch, NULL, NULL, TO_ROOM);
	SET_BIT(ch->affected_by, AFF_SHADOWPLANE);
	do_look(ch, "auto");
	return;
    }
    REMOVE_BIT(ch->affected_by, AFF_SHADOWPLANE);
    stc("You fade back into the real world.\n\r", ch);
    act("$n slowly fades into existance.", ch, NULL, NULL, TO_ROOM);
    do_look(ch, "auto");
    return;
}


void do_truesight( CHAR_DATA *ch, char *argument )
{
    if (IS_NPC(ch))
	return;

    if (!IS_DRAGON(ch) && !IS_DROW(ch) && !IS_HIGHLANDER(ch) && !IS_MONK(ch)
     && !IS_NINJA(ch) && !IS_VAMPIRE(ch) && !IS_WEREWOLF(ch) && !IS_MAGE(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (IS_MAGE(ch) && ch->spheres[MMIN] < 1)
    {
	stc("You do not have enough knowedlge of the mind sphere to use Heightened Senses.\n\r", ch);
	return;
    }

    if (IS_WEREWOLF(ch) && get_breed(ch, BREED_LUPUS) < 1)
    {
	stc("You require the level 1 Lupis power to use Heightened Senses.\n\r", ch);
	return;
    }
    if (IS_NINJA(ch) && get_ninja(ch, NINJA_SORA) < 4)
    {
	stc("You have not learned the Sora principle to 4.\n\r", ch);
	return;
    }
    if (IS_HIGHLANDER(ch) && get_highland(ch, HIGH_ABILITY) < 1)
    {
	stc("You must have level 1 powers to use Truesight.\n\r", ch);
	return;
    }
    if (IS_VAMPIRE(ch) && get_disc(ch, DISC_AUSPEX) < 1)
    {
        stc("You require level 1 Auspex to use Heightened Senses.\n\r", ch);
        return;
    }
    if (IS_DROW(ch) && !get_drow(ch, DROW_SIGHT))
    {
	stc("You need to obtain heighten first.\n\r", ch);
	return;
    }
    if (IS_DRAGON(ch) && !IS_DRAGAFF(ch, DRAG_TRUESIGHT))
    {
	stc("You do not yet have the truesight ability\n\r", ch);
	return;
    }
    if (IS_MONK(ch) && get_monk(ch, MONK_MANTRA) < 1)
    {
	stc("You need Mantra 1 for Eyes of God.\n\r", ch);
	return;
    }
  
    if (IS_SET(ch->act, PLR_HOLYLIGHT))
    {
	REMOVE_BIT(ch->act, PLR_HOLYLIGHT);
	stc("Your senses return to normal.\n\r", ch);
    }
    else
    {
	SET_BIT(ch->act, PLR_HOLYLIGHT);
	stc("Your senses increase to incredible proportions.\n\r", ch);
	WAIT_STATE(ch, 6);
    }
    return;
}

void do_soulmask( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    long value, max_stat;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    strcpy( arg2, argument );

    if ( IS_NPC(ch) )
	return;

    if (!IS_VAMPIRE(ch))
    {
	send_to_char("Huh?\n\r",ch);
	return;
    }

    if (get_disc(ch,DISC_OBFUSCATE) < 5)
    {
	send_to_char("You require level 5 Obfuscate to use Soul Mask.\n\r",ch);
	return;
    }

    if (arg1[0] != '\0' && !str_cmp(arg1,"on"))
    {
	if (IS_EXTRA(ch, EXTRA_FAKE_CON))
	{send_to_char("You already have Soul Mask on.\n\r",ch); return;}
	SET_BIT(ch->extra, EXTRA_FAKE_CON);
	send_to_char("Your Soul Mask is now ON.\n\r",ch);
	return;
    }
    if (arg1[0] != '\0' && !str_cmp(arg1,"off"))
    {
	if (!IS_EXTRA(ch, EXTRA_FAKE_CON))
	{send_to_char("You already have Soul Mask off.\n\r",ch); return;}
	REMOVE_BIT(ch->extra, EXTRA_FAKE_CON);
	send_to_char("Your Soul Mask is now OFF.\n\r",ch);
	return;
    }
    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char("--------------------------------------------------------------------------------\n\r",ch);
	send_to_char("                                -= Soul Mask =-\n\r",ch);
	send_to_char("--------------------------------------------------------------------------------\n\r",ch);
	send_to_char("You have the following stats:\n\r",ch);
	sprintf(buf,"Weapon skill: %li, Stance: %li.\n\r",
		ch->pcdata->fake_skill, ch->pcdata->fake_stance);
	send_to_char(buf, ch);
	sprintf(buf,"Hitroll: %li, Actual: %li.\n\r",
		ch->pcdata->fake_hit, char_hitroll(ch));
	send_to_char(buf, ch);
	sprintf(buf,"Damroll: %li, Actual: %li.\n\r",
		ch->pcdata->fake_dam, char_damroll(ch));
	send_to_char(buf, ch);
	sprintf(buf,"Armour: %li, Actual: %li.\n\r",
		ch->pcdata->fake_ac, char_ac(ch));
	send_to_char(buf, ch);
	sprintf(buf,"Hp: %li/%li, Actual: %li/%li.\n\r",
		ch->pcdata->fake_hp, ch->pcdata->fake_max_hp, 
		ch->hit, ch->max_hit);
	send_to_char(buf, ch);
	send_to_char(buf, ch);
	send_to_char("--------------------------------------------------------------------------------\n\r",ch);
	send_to_char("You can set: Skill, Stance, Hit, Dam, Ac,\n\r",ch);
	send_to_char("Hp, Max_Hp.\n\r",ch);
	send_to_char("--------------------------------------------------------------------------------\n\r",ch);
	return;
    }
    value = is_number( arg2 ) ? atoi( arg2 ) : -10000;

    switch (ch->vampgen)
    {
	default: max_stat = 5;  break;
	case 1:  max_stat = 10; break;
	case 2:
	case 3:
	case 4:  max_stat = 9;  break;
	case 5:  max_stat = 8;  break;
	case 6:  max_stat = 7;  break;
	case 7:  max_stat = 6;  break;
    }

    if (!str_cmp(arg1,"skill"))
    {
	if (value < 0 || value > 200)
	{
	    send_to_char("Please enter a value between 0 and 200.\n\r",ch);
	    return;
	}
	ch->pcdata->fake_skill = value;
	send_to_char("Ok.\n\r",ch);
	return;
    }
    if (!str_cmp(arg1,"stance"))
    {
	if (value < 0 || value > 200)
	{
	    send_to_char("Please enter a value between 0 and 200.\n\r",ch);
	    return;
	}
	ch->pcdata->fake_stance = value;
	send_to_char("Ok.\n\r",ch);
	return;
    }
    if (!str_cmp(arg1,"hit") || !str_cmp(arg1,"hitroll"))
    {
	if (value < 0 || value > 1000)
	{
	    send_to_char("Please enter a value between 0 and 1000.\n\r",ch);
	    return;
	}
	ch->pcdata->fake_hit = value;
	send_to_char("Ok.\n\r",ch);
	return;
    }
    if (!str_cmp(arg1,"dam") || !str_cmp(arg1,"damroll"))
    {
	if (value < 0 || value > 1000)
	{
	    send_to_char("Please enter a value between 0 and 1000.\n\r",ch);
	    return;
	}
	ch->pcdata->fake_dam = value;
	send_to_char("Ok.\n\r",ch);
	return;
    }
    if (!str_cmp(arg1,"ac") || !str_cmp(arg1,"armour") || !str_cmp(arg1,"armor"))
    {
	send_to_char("Please use ac+ or ac- to choose a positive or negative value.\n\r",ch);
	return;
    }
    if (!str_cmp(arg1,"ac+") || !str_cmp(arg1,"armour+") || !str_cmp(arg1,"armor+"))
    {
	if (value < 0 || value > 100)
	{
	    send_to_char("Please enter a value between 0 and 100.\n\r",ch);
	    return;
	}
	ch->pcdata->fake_ac = value;
	send_to_char("Ok.\n\r",ch);
	return;
    }
    if (!str_cmp(arg1,"ac-") || !str_cmp(arg1,"armour-") || !str_cmp(arg1,"armor-"))
    {
	if (value < 1 || value > 2000)
	{
	    send_to_char("Please enter a value between 1 and 2000.\n\r",ch);
	    return;
	}
	ch->pcdata->fake_ac = 0 - value;
	send_to_char("Ok.\n\r",ch);
	return;
    }
    if (!str_cmp(arg1,"hp") || !str_cmp(arg1,"hits"))
    {
	if (value < 1 || value > 30000)
	{
	    send_to_char("Please enter a value between 1 and 30000.\n\r",ch);
	    return;
	}
	if (value > ch->pcdata->fake_max_hp)
	{
	    send_to_char("You cannot have more hp than your max_hp.\n\r",ch);
	    return;
	}
	ch->pcdata->fake_hp = value;
	send_to_char("Ok.\n\r",ch);
	return;
    }
    if (!str_cmp(arg1,"max_hp") || !str_cmp(arg1,"max_hits"))
    {
	if (value < 1 || value > 30000)
	{
	    send_to_char("Please enter a value between 1 and 30000.\n\r",ch);
	    return;
	}
	if (value < ch->pcdata->fake_hp)
	{
	    send_to_char("You cannot have less max_hp than your hp.\n\r",ch);
	    return;
	}
	ch->pcdata->fake_max_hp = value;
	send_to_char("Ok.\n\r",ch);
	return;
    }
    send_to_char("You can set: Skill, Stance, Hit, Dam, Ac,\n\r",ch);
    send_to_char("Hp, Max_Hp.\n\r",ch);
    return;
}

void do_scry( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *chroom;
    DESCRIPTOR_DATA *d;
    char      arg [MIL];
    char      buf [MSL];
    bool      found = FALSE;

    argument = one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch) && !IS_NINJA(ch) && !IS_ITEMAFF(ch, ITEMA_VISION) && !IS_WEREWOLF(ch)
     && !IS_DROW(ch) && !IS_DRAGON(ch) && !IS_MONK(ch) && !IS_MAGE(ch) && !IS_BUI(ch))
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if (!IS_ITEMAFF(ch,ITEMA_VISION))
    {
	if (IS_NINJA(ch) && get_ninja(ch, NINJA_SORA) < 4)
	{
	    stc("You need Sora 4.\n\r", ch);
	    return;
	}
	else if (IS_VAMPIRE(ch) && get_disc(ch, DISC_AUSPEX) < 3)
	{
	    stc("You require level 3 Auspex to use Scry.\n\r", ch);
	    return;
	}
	else if (IS_WEREWOLF(ch) && get_breed(ch, BREED_LUPUS) < 2)
	{
	    stc("You require Lupus 2 to use Sense Prey.\n\r", ch);
	    return;
	}
	else if (IS_DROW(ch) && !get_drow(ch, DROW_EYES))
	{
	    stc("You need to obtain spider eyes first.\n\r", ch);
	    return;
	}
	else if (IS_DRAGON(ch) && !IS_DRAGAFF(ch, DRAG_SCRY))
	{
	    stc("You dont have the scry ability yet.\n\r", ch);
	    return;
	}
	else if (IS_MONK(ch) && get_monk(ch, MONK_MANTRA) < 2)
	{
	    stc("You need Mantra 2 for Trio Mantra.\n\r", ch);
	    return;
	}
	else if (IS_MAGE(ch) && ch->spheres[MMIN] < 2)
	{
	    stc("You do not have enough knowledge of the mind sphere to use scry.\n\r", ch);
	    return;
	}
    }

    if (IS_MAGE(ch))
    {
        if (ch->mana < 500)
        {
            stc("You do not have enough mana to use scry.\n\r", ch);
            return;
        }

        ch->mana -= 500;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char("You close your eyes and mentally survey the surrounding area.\n\r",ch);
	act("$n closes $s eyes and a look of concentration crosses $s face.",ch,NULL,NULL,TO_ROOM);
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected != CON_PLAYING ) continue;
	    if ( (victim = d->character) == NULL ) continue;
	    if ( IS_NPC(victim) || victim->in_room == NULL ) continue;
	    if ( victim == ch ) continue;
	    if ( !can_see(ch,victim) ) continue;
	    if ( IS_BUI(victim) && !IS_BUI(ch) ) continue;
	    if ( IS_AFFECTED(victim, AFF_POLYMORPH) )
		sprintf(buf,"%s: ", victim->morph);
	    else
		sprintf(buf,"%s: ", victim->name);
	    send_to_char(buf,ch);
	    if (!IS_MORE(victim, MORE_SHIELDED) || IS_IMM(ch) || (IS_VAMPIRE(ch) && get_disc(ch, DISC_AUSPEX) >= 6)
	     || (IS_WEREWOLF(ch) && get_breed(ch, BREED_LUPUS) == 5))
		sprintf(buf,"%s.\n\r", victim->in_room->name);
	    else
		sprintf(buf,"(shielded).\n\r");
	    send_to_char(buf,ch);
	    found = TRUE;
	}
	if (!found) send_to_char("You cannot seem to locate anyone.\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "You are unable to locate them.\n\r", ch );
	return;
    }

    if (IS_BUI(victim) && !IS_BUI(ch))
    {
	stc("You may not scry the Gods.  Shame on you.\n\r",ch);
	return;
    }

    if ((!IS_NPC(victim) && IS_MORE(victim,MORE_SHIELDED) && !IS_ITEMAFF(ch, ITEMA_VISION)) && !IS_BUI(ch))
    {
	send_to_char("You are unable to locate them.\n\r",ch);
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN) 
         && (!IS_WEREWOLF(ch) || get_breed(ch,BREED_HOMID) < 5))
	{
	    guardian_message( victim );
	    act("$n just failed to use Scry on you.",ch,NULL,victim,TO_VICT);
	}
	return;
    }

    chroom = ch->in_room;

    ch->in_room = victim->in_room;
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
    else if (get_drow(ch, DROW_GLOBE))
    {
	REMOVE_BIT(ch->pcdata->drow[DROW_POWERS], DROW_GLOBE);
	do_look(ch, "auto");
	SET_BIT(ch->pcdata->drow[DROW_POWERS], DROW_GLOBE);
    }
    else
	do_look(ch,"auto");
    ch->in_room = chroom;

    if ((!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN) 
     && (!IS_WEREWOLF(ch) || get_breed(ch,BREED_HOMID) < 5)) && !IS_BUI(ch))
    {
	guardian_message( victim );
	act("$n just used Scry on you.",ch,NULL,victim,TO_VICT);
    }

    return;
}

void do_readaura( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA  *obj;
    char      arg [MAX_INPUT_LENGTH];
    char      buf [MAX_INPUT_LENGTH];
    long       con_hit, con_dam, con_ac, con_hp, con_mana;
    long       con_max_hp, con_max_mana;

    argument = one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch) && !IS_ITEMAFF(ch, ITEMA_VISION) && !IS_WEREWOLF(ch)
     && !IS_NINJA(ch) && !IS_DROW(ch) && !IS_DRAGON(ch) && !IS_MONK(ch) && !IS_MAGE(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (IS_NINJA(ch) && get_ninja(ch, NINJA_SORA) < 4)
    {
	send_to_char("You need Sora 4.\n\r", ch);
	return;
    }
    else if (IS_VAMPIRE(ch) && get_disc(ch, DISC_AUSPEX) < 2)
    {
	send_to_char("You require level 2 Auspex to Read somethings Aura.\n\r",ch);
	return;
    }
    else if (IS_WEREWOLF(ch) && get_tribe(ch,TRIBE_GLASS_WALKERS) < 2)
    {
	stc("You require Glass Walkers 2 to use Cybersenses.\n\r",ch);
	return;
    }
    else if (IS_DROW(ch) && !get_drow(ch, DROW_READ))
    {
        stc("You need to obtain readaura first.\n\r",ch);
        return;
    }
    else if(IS_DRAGON(ch) &&!IS_DRAGAFF(ch, DRAG_READAURA))
    {
	stc("You do not yet have the ability to read someone's aura.\n\r",ch);
	return;
    }
    else if (IS_MONK(ch) && get_monk(ch, MONK_MANTRA) < 2)
    {
	stc("You need Mantra 2 for Trio Mantra.\n\r", ch);
	return;
    }
    else if (IS_MAGE(ch) && ch->spheres[MLIF] < 1)
    {
	stc("You do not have enough knowledge of the life sphere to use read aura.\n\r", ch);
	return;
    }

    if (IS_MAGE(ch))
    {
	if (ch->mana < 500)
	{
	    stc("You do not have enough mana to use readaura.\n\r", ch);
	    return;
	}

	ch->mana -= 500;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char( "Read the aura on what?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        if ( ( obj = get_obj_carry( ch, arg ) ) == NULL || IS_WEREWOLF(ch))
	{
	    send_to_char( "Read the aura on what?\n\r", ch );
	    return;
	}
	spell_identify( skill_lookup( "identify" ), ch->level, ch, obj );
	return;
    }

    if (IS_WEREWOLF(ch) && get_breed(ch, BREED_HOMID) >= 5)
    {
	send_to_char("You are unable to read their aura.\n\r",ch);
	return;
    }

    act("$n examines $N intently.",ch,NULL,victim,TO_NOTVICT);
    act("$n examines you intently.",ch,NULL,victim,TO_VICT);

    if (!IS_NPC(victim) && IS_EXTRA(victim, EXTRA_FAKE_CON))
    {
	con_hit = victim->pcdata->fake_hit;
	con_dam = victim->pcdata->fake_dam;
	con_ac = victim->pcdata->fake_ac;
	con_hp = victim->pcdata->fake_hp;
	con_mana = victim->pcdata->fake_mana;
	con_max_hp = victim->pcdata->fake_max_hp;
	con_max_mana = victim->pcdata->fake_max_mana;
    }
    else
    {
	con_hit = char_hitroll(victim);
	con_dam = char_damroll(victim);
	con_ac = char_ac(victim);
	con_hp = victim->hit;
	con_mana = victim->mana;
	con_max_hp = victim->max_hit;
	con_max_mana = victim->max_mana;
    }

    if (IS_NPC(victim)) sprintf(buf, "%s is an NPC.\n\r",victim->short_descr);
    else 
    {
	if (IS_ST(victim)) sprintf(buf, "%s is a Storyteller.\n\r", victim->name);
	else if (IS_IMP(victim)) sprintf(buf, "%s is an Implementor.\n\r", victim->name);
	else if (IS_IMM(victim)) sprintf(buf, "%s is an Immortal.\n\r", victim->name);
	else if (IS_BUI(victim)) sprintf(buf, "%s is a Builder.\n\r", victim->name);
	else if (IS_AWA(victim)) sprintf(buf, "%s is an Awakened.\n\r", victim->name);
	else sprintf(buf, "%s is a Sleeper.\n\r", victim->name);
    }
    stc(buf, ch);

    sprintf(buf, "Str: %i.  Dex: %i.  Con: %i.  Int: %i.  Wis: %i.\n\r",
     raw_stat(victim, STAT_STR), raw_stat(victim, STAT_DEX), raw_stat(victim, STAT_CON),
     raw_stat(victim, STAT_INT), raw_stat(victim, STAT_WIS));
    stc(buf, ch);

    sprintf(buf,"Hp: %li/%li, Mana: %li/%li.\n\r", con_hp, con_max_hp, con_mana, con_max_mana);
    stc(buf, ch);
    sprintf(buf,"Hitroll: %li, Damroll: %li, AC: %li.\n\r", con_hit, con_dam, con_ac);
    stc(buf,ch);

    if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN)
     && (!IS_WEREWOLF(ch) || get_breed(ch, BREED_HOMID) < 5))
    {
	guardian_message( victim );
	act("$n just used Read Aura on you.",ch,NULL,victim,TO_VICT);
    }
    return;
}

void do_shield( CHAR_DATA *ch, char *argument )
{
    char      arg [MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if (IS_NPC(ch)) return;
    if (!IS_VAMPIRE(ch) && !IS_DRAGON(ch) && !IS_WEREWOLF(ch) && !IS_MONK(ch) && !IS_MAGE(ch))
    {
	stc("Huh?\n\r",ch);
	return;
    }
    else if (IS_VAMPIRE(ch) && get_disc(ch,DISC_OBFUSCATE) < 6)
    {
	stc("You require level 6 Obfuscate to Shield your aura.\n\r",ch);
	return;
    }
    else if (IS_DRAGON(ch) && !IS_DRAGAFF(ch, DRAG_SHIELD))
    {
	stc("You dont have the shield ability yet.\n\r",ch);
	return;
    }
    else if (IS_WEREWOLF(ch) && (get_tribe(ch, TRIBE_SILVER_FANGS) < 4))
    {
	stc("You require lvl 4 in Silver Fangs for this ability.",ch);
	return;
    }
    else if (IS_MONK(ch) && get_monk(ch, MONK_MANTRA) < 2)
    {
	stc("You need Mantra 2 for Trio Mantra.\n\r", ch);
	return;
    }
    else if (IS_MAGE(ch) && ch->spheres[MMIN] < 3)
    {
	stc("You do not have enough knowledge of the mind sphere to use shield.\n\r", ch);
	return;
    }
    if (!IS_MORE(ch, MORE_SHIELDED) )
    {
    	send_to_char("You shield your aura from those around you.\n\r",ch);
    	SET_BIT(ch->more, MORE_SHIELDED);
	return;
    }
    send_to_char("You stop shielding your aura.\n\r",ch);
    REMOVE_BIT(ch->more, MORE_SHIELDED);
    return;
}

void do_serpent( CHAR_DATA *ch, char *argument )
{
    char buf [MAX_STRING_LENGTH]; 

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch))
    {
	send_to_char("Huh?\n\r",ch);
	return;
    }

    if (get_disc(ch,DISC_SERPENTIS) < 4)
    {
	send_to_char("You require level 4 Serpentis to transform into Serpent Form.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(ch, AFF_POLYMORPH))
    {
	if (!IS_POLYAFF(ch,POLY_SERPENT))
	{
	    send_to_char( "You cannot polymorph from this form.\n\r", ch );
	    return;
	}
	act( "You transform back into human.", ch, NULL, NULL, TO_CHAR );
	act( "$n transforms into human form.", ch, NULL, NULL, TO_ROOM );
	REMOVE_BIT(ch->polyaff, POLY_SERPENT);
	REMOVE_BIT(ch->affected_by, AFF_POLYMORPH);
	reset_form_stats(ch);
	free_string( ch->morph );
	ch->morph = str_dup( "" );
	do_autosave(ch, "");
	return;
    }
    SET_BIT(ch->polyaff, POLY_SERPENT);
    reset_form_stats(ch);
    if (ch->stance[0] != -1) do_stance(ch,"");
    act( "You transform into a huge serpent.", ch, NULL, NULL, TO_CHAR );
    act( "$n transforms into a huge serpent.", ch, NULL, NULL, TO_ROOM );
    SET_BIT(ch->affected_by, AFF_POLYMORPH);
    sprintf(buf, "A huge serpent[#0%s#n]", ch->name);
    free_string( ch->morph );
    ch->morph = str_dup( buf );
    do_autosave(ch, "");
    return;
}

void poison_weapon( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( obj->item_type != ITEM_WEAPON )
    {
	send_to_char( "You can only poison weapons.\n\r", ch );
	return;
    }
    if ( obj->value[0] != 0 )
    {
	send_to_char( "This weapon cannot be poisoned.\n\r", ch );
	return;
    }
    act("You run your long serpentine tongue along $p, poisoning it.",ch,obj,NULL,TO_CHAR);
    act("$n runs $s long serpentine tongue along $p, poisoning it.",ch,obj,NULL,TO_ROOM);
    obj->value[0] = 53;
    return;
}


void beast_rage( CHAR_DATA *ch )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;

    if (IS_NPC(ch) || !IS_VAMPIRE(ch)) return;

    if (ch->pcdata->wolf < 1) return;
    if (ch->position < POS_SLEEPING) return;
    if (ch->position == POS_FIGHTING) return;
    if (ch->position < POS_FIGHTING) do_stand(ch,"");
    if (IS_ITEMAFF(ch, ITEMA_PEACE)) do_majesty(ch,"");
    if (number_range(1, 15) == 15)
    { 
    	for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    	{
	    vch_next	= vch->next_in_room;
	    if ( !IS_NPC(vch) && vch->pcdata->chobj != NULL ) continue;
	    if ( ch == vch ) continue;
	    if ( !can_see(ch, vch) ) continue;
	    if ( auto_safe(ch, vch) ) continue;
	    if ( number_percent() <= 50 ) continue;
	    if ( !CAN_PK(ch) || !CAN_PK(vch) ) continue;

	    send_to_char("Your beast demands BLOOD!\n\r",ch);
	    act("$n roars with rage as $s inner beast demands BLOOD!",ch,NULL,NULL,TO_ROOM);

	    if (!IS_NPC(vch) && vch->position == POS_MORTAL) do_destroy(ch, vch->name);
	    else multi_hit(ch, vch);
	    return;
    	}
    }
    if (ch->pcdata->wolf > 0) ch->pcdata->wolf--;
    if (ch->pcdata->wolf > 0 && number_percent() <= 25)
    {
	switch (number_range(1, 8))
	{
	    default:
	    case 1:
		send_to_char("You snarl angrilly.\n\r",ch);
		act("$n snarls angrilly.",ch,NULL,NULL,TO_ROOM);
		break;
	    case 2:
		send_to_char("You roar with rage!\n\r",ch);
		act("$n roars with rage!",ch,NULL,NULL,TO_ROOM);
		break;
	    case 3:
		send_to_char("You grit your teeth and fume with rage.\n\r",ch);
		act("$n grits $s teeth and fumes with rage.",ch,NULL,NULL,TO_ROOM);
		break;
	    case 4:
		send_to_char("You glare around icily.\n\r",ch);
		act("$n glares icily around $m.",ch,NULL,NULL,TO_ROOM);
		break;
	    case 5:
		send_to_char("You growl with unbridled hatred.\n\r",ch);
		act("$n growls with unbridled hatred.",ch,NULL,NULL,TO_ROOM);
		break;
	    case 6:
		send_to_char("You laugh diabolically.  MUHAHAHAHAHAHA!\n\r",ch);
		act("$n laughs diabolically.  MUHAHAHAHAHAHA!",ch,NULL,NULL,TO_ROOM);
		break;
	    case 7:
		send_to_char("You sneer in utter contempt.\n\r",ch);
		act("$n sneers in utter contempt.",ch,NULL,NULL,TO_ROOM);
		break;
	    case 8:
		send_to_char("You throw back your head and cackle with insane glee!\n\r",ch);
		act("$n throws back $s head and cackles with insane glee!",ch,NULL,NULL,TO_ROOM);
		break;
	}
    }
    return;
}

void vamp_rage( CHAR_DATA *ch )
{
    if (IS_NPC(ch) || !IS_VAMPIRE(ch)) return;
    send_to_char("You scream with rage as the beast within consumes you!\n\r",ch);
    act("$n screams with rage as $s inner beast consumes $m!.", ch, NULL, NULL, TO_ROOM);
    if (ch->beast > 0) do_beastlike(ch,"");
    if ( ch->pcdata->wolf >= (ch->beast * 0.5) ) return;
    if (!IS_VAMPAFF(ch, VAM_NIGHTSIGHT) && get_disc(ch,DISC_PROTEAN) > 0) 
	do_nightsight(ch,"");
    if (!IS_VAMPAFF(ch, VAM_FANGS)) do_fangs(ch,"");
    if (!IS_VAMPAFF(ch, VAM_CLAWS) && get_disc(ch,DISC_PROTEAN) > 1) 
	do_claws(ch,"");
    send_to_char("You bare your fangs and growl as your inner beast consumes you.\n\r",ch);
    act("$n bares $s fangs and growls as $s inner beast consumes $m.",ch,NULL,NULL,TO_ROOM);
    ch->pcdata->wolf += number_range(10,20);
    WAIT_STATE(ch,12);
    return;
}

void do_feed( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char      arg [MAX_INPUT_LENGTH];
    long       blood;

    argument = one_argument( argument, arg );

    if (IS_NPC(ch)) return;
    if (!IS_VAMPIRE(ch))
    {
	send_to_char("Huh?\n\r",ch);
	return;
    }

    if (arg[0] == '\0')
    {
	send_to_char("Who do you wish to feed?\n\r",ch);
	return;
    }
    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }
    if (IS_NPC(victim))
    {
	send_to_char("Not on NPC's.\n\r",ch);
	return;
    }
    if (!IS_VAMPIRE(victim))
    {
	send_to_char("Blood does them no good at all.\n\r",ch);
	return;
    }
    if (ch == victim)
    {
	send_to_char("There is no point in feeding yourself blood!\n\r",ch);
	return;
    }
    if (!IS_EXTRA(victim, EXTRA_VAMPIRE))
    {
	send_to_char("They refuse to drink your blood.\n\r",ch);
	return;
    }
    if ( ch->blood[BLOOD_CURRENT] < 10)
    {
	send_to_char("You don't have enough blood.\n\r",ch);
	return;
    }

    act("You feed $N some blood from your open wrist.",ch,NULL,victim,TO_CHAR);
    act("$n feeds $N some blood from $s open wrist.",ch,NULL,victim,TO_NOTVICT);

    if (victim->position < POS_RESTING)
	send_to_char("You feel some blood poured down your throat.\n\r",victim);
    else
	act("$n feeds you some blood from $s open wrist.", ch, NULL, victim, TO_VICT);

    blood = number_range(5, 10);
    ch->blood[BLOOD_CURRENT] -= blood;
    victim->blood[BLOOD_CURRENT] += blood;
    if (victim->blood[BLOOD_CURRENT] >= victim->blood[BLOOD_POOL])
    {
	victim->blood[BLOOD_CURRENT] = victim->blood[BLOOD_POOL];
	if (IS_VAMPIRE(victim)) 
	    send_to_char("Your blood lust is sated.\n\r",victim);
    }
    return;
}

void do_upkeep( CHAR_DATA *ch, char *argument )
{
    char      arg [MAX_INPUT_LENGTH];
    char      buf [MAX_INPUT_LENGTH];
    char      buf2 [MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (IS_MAGE(ch))
    {
	mageupkeep(ch);
	return;
    }

    if (IS_VAMPIRE(ch))
    {
	send_to_char("--------------------------------------------------------------------------------\n\r",ch);
	send_to_char("                              -= Vampire upkeep =-\n\r",ch);
	send_to_char("--------------------------------------------------------------------------------\n\r",ch);
	send_to_char("You must spend 1 blood per tick or you may well enter Frenzy.\n\r",ch);

    	if (ch->pcdata->regenerate > 0 )
	{
	    sprintf(buf,"You will attempt to regenerate %li more time%s\n\r", 
		ch->pcdata->regenerate, 
		ch->pcdata->regenerate > 1 ? "s." : ".");
	    send_to_char(buf,ch);
	}

    	if (ch->pcdata->celerity > 0 )
	{
	    sprintf(buf,"Your are infused with %li %s of Celerity.\n\r", 
		ch->pcdata->celerity, 
		ch->pcdata->celerity == 1 ? "second" : "seconds");
	    send_to_char(buf,ch);
	}

    	if (strlen(ch->pcdata->conding) > 1 )
	{
	    if (IS_MORE(ch, MORE_LOYAL))
		sprintf(buf,"You have been conditioned by %s and are loyal.\n\r", ch->pcdata->conding);
	    else
		sprintf(buf,"You have been conditioned by %s.\n\r", ch->pcdata->conding);
	    send_to_char(buf,ch);
	}
    	if (strlen(ch->pcdata->love) > 1 )
	{
	    sprintf(buf,"You are in love with %s.\n\r", ch->pcdata->love);
	    send_to_char(buf,ch);
	}
	if (get_disc(ch,DISC_PROTEAN) > 5)
	    send_to_char("Your Flesh is as hard as Marble.\n\r",ch);
	if (get_disc(ch,DISC_VICISSITUDE) > 5)
	    send_to_char("Your blood is a deadly acid.\n\r",ch);
	if (IS_MORE(ch, MORE_ANIMAL_MASTER))
	    send_to_char("You appear very attractive to animals.\n\r",ch);
    	if (IS_VAMPAFF(ch, VAM_OBFUSCATE))
	    send_to_char("You are concealed with Obfuscate.\n\r",ch);
	if (IS_EXTRA(ch, EXTRA_FAKE_CON))
	    send_to_char("Your Soul Mask is up.\n\r",ch);
    	if (IS_ITEMAFF(ch,ITEMA_PEACE) )
	    send_to_char("Your Majesty is up.\n\r",ch);
    	if (IS_EXTRA(ch,EXTRA_AWE) )
	    send_to_char("You appear very awe-inspiring.\n\r",ch);
    	if (IS_MORE(ch,MORE_GUARDIAN) )
	    send_to_char("Your Spirit Guardian is with you.\n\r",ch);
    	if (IS_VAMPAFF(ch,VAM_DISGUISED) )
	{
	    if (IS_POLYAFF(ch, POLY_ZULO))
		sprintf(buf,"You are in Zulo Form.\n\r");
	    else if (IS_POLYAFF(ch, POLY_SPIRIT))
		sprintf(buf,"You are in Spiritform.\n\r");
	    else if (IS_CRAFT(ch, CRAFT_MALLEABLE))
		sprintf(buf,"Your features look like %s.\n\r",ch->morph);
	    else if (IS_POLYAFF(ch, POLY_TRUE))
		sprintf(buf,"You are True Disguised as %s.\n\r",ch->morph);
	    else
		sprintf(buf,"You are Masked as %s.\n\r",ch->morph);
	    send_to_char(buf,ch);
	}
    	if (IS_MORE(ch, MORE_SHIELDED) )
	    send_to_char("You aura is Shielded.\n\r",ch);
    	if (IS_AFFECTED(ch,AFF_SHADOWPLANE) )
	    send_to_char("You are in the Umbra.\n\r",ch);
    	if (IS_VAMPAFF(ch,VAM_FANGS) )
	    send_to_char("You have your Fangs extended.\n\r",ch);
    	if (IS_VAMPAFF(ch,VAM_CLAWS) )
	    send_to_char("You have your Claws extended.\n\r",ch);
    	if (IS_VAMPAFF(ch,VAM_NIGHTSIGHT) )
	    send_to_char("You are using Gleam of Red Eyes.\n\r",ch);
    	if (IS_VAMPAFF(ch,VAM_SPIRITEYES) )
	    send_to_char("You are using Spirit Eyes.\n\r",ch);
    	if (IS_AFFECTED(ch,AFF_SHADOWSIGHT) )
	    send_to_char("You are using Umbral Vision.\n\r",ch);
    	if (IS_SET(ch->act,PLR_HOLYLIGHT) )
	    send_to_char("You are using Heightened Senses.\n\r",ch);
	if (IS_MORE(ch,MORE_HELIOS) )
	    send_to_char("Your hands are engulfed in burning flames.\n\r",ch);
    	if (IS_VAMPAFF(ch,VAM_CHANGED) )
	{
	    if      (IS_POLYAFF(ch,POLY_BAT))  sprintf(buf2,"Bat" );
	    else if (IS_POLYAFF(ch,POLY_WOLF)) sprintf(buf2,"Wolf");
	    else                               sprintf(buf2,"Mist");
	    sprintf(buf,"You are in %s form.\n\r",buf2);
	    send_to_char(buf,ch);
	}
	if      ( IS_POLYAFF(ch,POLY_SHADOW) )
	    send_to_char("You are in Shadow form.\n\r",ch);
	else if ( IS_POLYAFF(ch,POLY_SERPENT) )
	    send_to_char("You are in Serpent form.\n\r",ch);
	else if ( IS_MORE(ch,MORE_PLASMA) )
	    send_to_char("You are in Plasma form.\n\r",ch);
	else if ( IS_MORE(ch,MORE_MUMMIFIED) )
	    send_to_char("Your body is mummified.\n\r",ch);
	    send_to_char("--------------------------------------------------------------------------------\n\r",ch);
	    return;
    }

    else if (IS_WEREWOLF(ch))
    {
	send_to_char("--------------------------------------------------------------------------------\n\r",ch);
	send_to_char("                              -= Werewolf upkeep =-\n\r",ch);
	send_to_char("--------------------------------------------------------------------------------\n\r",ch);
    	if (strlen(ch->pcdata->conding) > 1 )
	{
	    if (IS_MORE(ch, MORE_LOYAL))
		sprintf(buf,"You have been conditioned by %s and are loyal.\n\r", ch->pcdata->conding);
	    else
		sprintf(buf,"You have been conditioned by %s.\n\r", ch->pcdata->conding);
	    send_to_char(buf,ch);
	}
    	if (strlen(ch->pcdata->love) > 1 )
	{
	    sprintf(buf,"You are in love with %s.\n\r", ch->pcdata->love);
	    send_to_char(buf,ch);
	}
	if (get_tribe(ch,TRIBE_GET_OF_FENRIS) >= 3)
	    send_to_char("Your blood is a deadly acid.\n\r",ch);
	if (IS_MORE(ch, MORE_ANIMAL_MASTER))
	    send_to_char("You appear very attractive to animals.\n\r",ch);
    	if (IS_ITEMAFF(ch,ITEMA_PEACE) )
	    send_to_char("You are using the Icy Chill of Despair.\n\r",ch);
    	if (IS_EXTRA(ch,EXTRA_AWE) )
	    send_to_char("You seem very persuasive.\n\r",ch);
    	if (IS_MORE(ch,MORE_GUARDIAN) )
	    send_to_char("Your Spirit Guardian is with you.\n\r",ch);
    	if (IS_VAMPAFF(ch, VAM_OBFUSCATE))
	    send_to_char("You are concealed with Obfuscate.\n\r",ch);
    	if (IS_MORE(ch, MORE_SHIELDED) )
	    send_to_char("You aura is Shielded.\n\r",ch);
    	if (IS_AFFECTED(ch,AFF_SHADOWPLANE) )
	    send_to_char("You are in the Umbra.\n\r",ch);
    	if (IS_VAMPAFF(ch,VAM_FANGS) )
	    send_to_char("You have your Fangs extended.\n\r",ch);
    	if (IS_VAMPAFF(ch,VAM_CLAWS) )
	    send_to_char("You have your Claws extended.\n\r",ch);
    	if (IS_VAMPAFF(ch,VAM_NIGHTSIGHT) )
	    send_to_char("You are using Eyes of the Cat.\n\r",ch);
    	if (IS_VAMPAFF(ch,VAM_SPIRITEYES) )
	    send_to_char("You are using Scent from Beyond.\n\r",ch);
    	if (IS_AFFECTED(ch,AFF_SHADOWSIGHT) )
	    send_to_char("You are using Umbral Vision.\n\r",ch);
    	if (IS_SET(ch->act,PLR_HOLYLIGHT) )
	    send_to_char("You are using Heightened Senses.\n\r",ch);
	if (IS_MORE(ch,MORE_HELIOS))
	    send_to_char("Your hands are engulfed in burning flames.\n\rYour body is immune to fire.\n\r",ch);
	if (IS_POLYAFF(ch,POLY_BAT))
	    send_to_char("You are in Lupus form.\n\r",ch);
	if (IS_MORE(ch,MORE_BRISTLES) )
	    send_to_char("Your body is covered in long quill-like spines.\n\r",ch);
	if (IS_ITEMAFF(ch,ITEMA_SUNSHIELD) )
	    send_to_char("You are surrounded with a halo of sunlight.\n\r",ch);
	if (get_tribe(ch, TRIBE_SILVER_FANGS) > 4)
	    send_to_char("Luna has blessed you with immunity to silver.\n\r",ch);
	if (get_auspice(ch, AUSPICE_PHILODOX) > 0)
	    send_to_char("You are able to shrug off the effects of pain.\n\r",ch);
	if (get_auspice(ch, AUSPICE_AHROUN) >= 3 && IS_VAMPAFF(ch, VAM_CLAWS) )
	    send_to_char("Your claws are pure silver.\n\r",ch);
	if (get_tribe(ch, TRIBE_BLACK_FURIES) > 1 && IS_VAMPAFF(ch, VAM_CLAWS))
	    send_to_char("Your claws are dripping with black venom.\n\r",ch);
	if (get_tribe(ch, TRIBE_GET_OF_FENRIS) >= 5)
	    send_to_char("Punches bounce off your body with no affect.\n\r",ch);
	if (get_tribe(ch, TRIBE_RED_TALONS) > 2)
	{
	    if (get_tribe(ch, TRIBE_RED_TALONS) > 3)
		send_to_char("Your claws can parry weapons in combat as well as any other weapon can.\n\r",ch);
	    else
		send_to_char("Your claws can parry weapons in combat.\n\r",ch);
	}
	if (get_tribe(ch, TRIBE_SILENT_STRIDERS) > 0)
	{
	    if (get_tribe(ch, TRIBE_SILENT_STRIDERS) > 2)
		send_to_char("Your move at a Speed Beyond Thought.\n\r",ch);
	    else
		send_to_char("Your move at the Speed of Thought.\n\r",ch);
	}
	if (get_tribe(ch, TRIBE_SILENT_STRIDERS) > 1)
	    send_to_char("Your body has the resistance of Messanger's Fortitude.\n\r",ch);
    }
    else if (IS_ELEM(ch))
    {
	send_to_char("-------------------------------------------------------------------------------\n\r",ch);
   	send_to_char("                          -= Elementalist upkeep =-\n\r",ch);
	send_to_char("-------------------------------------------------------------------------------\n\r",ch);
	if (ch->pcdata->powers[ELEM_FIRE] > 4)
	   stc("You feel Fire's Strength flowing through you!\n\r",ch);
	if (IS_ITEMAFF(ch, ITEMA_FLAMING) )
	   stc("Searing Flames surround your weapons.\n\r",ch);
   	if (IS_ITEMAFF(ch, ITEMA_AQUABARRIER))
   	   stc("A dense barrier of water protects you from harm.\n\r",ch);
   	if (ch->pcdata->powers[ELEM_WIND] > 4)
	   stc("You hear and respond to the Song of the Wind!\n\r",ch);
	if (ch->pcdata->powers[ELEM_EARTH] > 4)
	   stc("Your skin is as tough as stone!\n\r",ch);
   	if (IS_ITEMAFF(ch, ITEMA_AURA))
   	   stc("You are surrounded in a holy aura!\n\r",ch);
   	if (IS_ITEMAFF(ch, ITEMA_SUNSHIELD))
   	   stc("An aura of sunlight surrounds your body.\n\r",ch);
   	if (ch->pcdata->powers[ELEM_MOON] > 4)
   	   stc("You fool enemies with illusions of your body.\n\r",ch);
   	if (ch->pcdata->powers[ELEM_TIME] > 2)
   	   stc("Enemies seem slower and easier to strike.\n\r",ch);
   	if (ch->pcdata->powers[ELEM_TIME] > 4)
   	   stc("You feel quicker and more able to dodge.\n\r",ch);
   	if ( IS_SET(ch->act, PLR_HOLYLIGHT) )
	   stc("You are able to see what mortal eyes cannot.\n\r",ch);
   	if (IS_MORE(ch, MORE_SHIELDED) )
	   stc("You are keeping your strength and your location hidden from prying eyes.\n\r",ch);
   	if (ch->pcdata->powers[ELEM_INFIN] > 7)
   	   stc("You focus your skill to become your inner strength!\n\r",ch);
    }
    else
    {
	send_to_char("Huh?\n\r",ch);
	return;
    }
    send_to_char("--------------------------------------------------------------------------------\n\r",ch);
    return;
}


void do_werewolf( CHAR_DATA *ch, char *argument )
{
    char       buf[MAX_INPUT_LENGTH];
    char       arg[MAX_INPUT_LENGTH];
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    argument = one_argument( argument, arg );

    if (!IS_WEREWOLF(ch)) return;
    if (IS_SET(ch->act, PLR_WOLFMAN)) return;

    if (IS_POLYAFF(ch, POLY_MASK)) do_doppleganger(ch, "self");

    SET_BIT(ch->act, PLR_WOLFMAN);

    stc("You throw back your head and howl with rage!\n\r",ch);
    act("$n throws back $s head and howls with rage!.", ch, NULL, NULL, TO_ROOM);
    send_to_char("Coarse dark hair sprouts from your body.\n\r",ch);
    act("Coarse dark hair sprouts from $n's body.",ch,NULL,NULL,TO_ROOM);

    if (!IS_VAMPAFF(ch,VAM_NIGHTSIGHT) && get_breed(ch, BREED_METIS) >= 3)
	do_nightsight(ch, "");

    if (!IS_VAMPAFF(ch,VAM_FANGS))
    {
	send_to_char("A pair of long fangs extend from your mouth.\n\r",ch);
	act("A pair of long fangs extend from $n's mouth.",ch,NULL,NULL,TO_ROOM);
	SET_BIT(ch->vampaff, VAM_FANGS);
    }
    if (!IS_VAMPAFF(ch,VAM_CLAWS))
    {
	if (get_auspice(ch, AUSPICE_AHROUN) >= 3)
	{
	    send_to_char("Gleaming silver talons extend from your fingers.\n\r",ch);
	    act("Gleaming silver talons extend from $n's fingers.",ch,NULL,NULL,TO_ROOM);
	}
	else
	{
	    send_to_char("Razor sharp talons extend from your fingers.\n\r",ch);
	    act("Razor sharp talons extend from $n's fingers.",ch,NULL,NULL,TO_ROOM);
	}
	SET_BIT(ch->vampaff, VAM_CLAWS);
    }
    SET_BIT(ch->affected_by, AFF_POLYMORPH);
    SET_BIT(ch->vampaff, VAM_DISGUISED);

    sprintf(buf,"#oA huge werewolf#n[#0%s#n]", ch->name);
    free_string(ch->morph);
    ch->morph = str_dup(buf);
    ch->pcdata->wolf += 25;

    if (get_auspice(ch, AUSPICE_AHROUN) >= 4) ch->pcdata->wolf = 300;
    else if (ch->pcdata->wolf > 300) ch->pcdata->wolf = 300;

    update_style(ch);

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
	vch_next	= vch->next;
	if ( vch->in_room == NULL )
	    continue;
	if ( ch == vch )
	{
	    act("You throw back your head and howl with rage!", ch, NULL, NULL, TO_CHAR);
	    continue;
	}
	if ( !IS_NPC(vch) && vch->pcdata->chobj != NULL )
	    continue;
	if (!IS_NPC(vch))
	{
	    if (vch->in_room == ch->in_room)
		act("$n throws back $s head and howls with rage!", ch, NULL, vch, TO_VICT);
	    else if (vch->in_room->area == ch->in_room->area)
		send_to_char("You hear a fearsome howl close by!\n\r", vch);
	    else
		send_to_char("You hear a fearsome howl far off in the distance!\n\r", vch);
	    if (!CAN_PK(vch)) continue;
	}
	if ( vch->in_room == ch->in_room && can_see( ch, vch) )
	{
	    multi_hit(ch, vch);
	    multi_hit(ch, vch);
	    multi_hit(ch, vch);
	}
    }
    return;
}

void do_unwerewolf( CHAR_DATA *ch, char *argument )
{
    char       arg[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if (IS_NPC(ch)) return;
    if (!IS_WEREWOLF(ch)) return;
    ch->pcdata->wolf = 0;
    if (!IS_SET(ch->act, PLR_WOLFMAN)) return;
    REMOVE_BIT(ch->act, PLR_WOLFMAN);
    free_string(ch->morph);
    {
	ch->morph = str_dup("");
	REMOVE_BIT(ch->affected_by, AFF_POLYMORPH);
	REMOVE_BIT(ch->vampaff, VAM_DISGUISED);
    }

    if (IS_VAMPAFF(ch, VAM_NIGHTSIGHT)) do_nightsight(ch, "");

    if (IS_VAMPAFF(ch,VAM_CLAWS) && !IS_POLYAFF(ch, POLY_ZULO))
    {
	if (get_auspice(ch, AUSPICE_AHROUN) >= 3)
	{
	    send_to_char("Your gleaming silver talons slide back into your fingers.\n\r",ch);
	    act("$n's gleaming silver talons slide back into $s fingers.",ch,NULL,NULL,TO_ROOM);
	}
	else
	{
	    send_to_char("Your talons slide back into your fingers.\n\r",ch);
	    act("$n's talons slide back into $s fingers.",ch,NULL,NULL,TO_ROOM);
	}
	REMOVE_BIT(ch->vampaff, VAM_CLAWS);
    }
    if (IS_VAMPAFF(ch,VAM_FANGS))
    {
	send_to_char("Your fangs slide back into your mouth.\n\r",ch);
	act("$n's fangs slide back into $s mouth.",ch,NULL,NULL,TO_ROOM);
	REMOVE_BIT(ch->vampaff, VAM_FANGS);
    }
    send_to_char("Your coarse hair shrinks back into your body.\n\r",ch);
    act("$n's coarse hair shrinks back into $s body.",ch,NULL,NULL,TO_ROOM);
    update_style(ch);
    return;
}

void do_favour( CHAR_DATA *ch, char *argument )
{
    char      arg1 [MAX_INPUT_LENGTH];
    char      arg2 [MAX_INPUT_LENGTH];
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if (!IS_VAMPIRE(ch) && !IS_WEREWOLF(ch))
        { stc("Huh?\n\r", ch); return; }
    if (!arg1[0] || argument[0])
        { stc("Syntax: favour <player> induct|outcast|elder.\n\r", ch); return; }
    if (!str_cmp(arg2, "induct")) clan_legacy(ch, "invite", arg1);
    else if (!str_cmp(arg2, "outcast")) clan_legacy(ch, "kick", arg1);
    else if (!str_cmp(arg2, "elder")) clan_legacy(ch, "promote", arg1);
    else stc("Use induct, outcast or elder (officer); clan demote removes office.\n\r", ch);
}

void do_familiar( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    CHAR_DATA *familiar;
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );

    return;

    if ( arg[0] == '\0' )
    {
	send_to_char( "What do you wish to make your familiar?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch == victim )
    {
	send_to_char( "Become your own familiar?\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) )
    {
	send_to_char( "Not on players.\n\r", ch );
	return;
    }

    if ( ( familiar = ch->pcdata->familiar ) != NULL ) 
	familiar->wizard = NULL;
    ch->pcdata->familiar = victim;
    victim->wizard = ch;
    send_to_char("Ok.\n\r",ch);

    return;
}

void do_fcommand( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if ( ( victim = ch->pcdata->familiar ) == NULL )
    {
	send_to_char( "But you don't have a familiar!\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "What do you wish to make your familiar do?\n\r", ch );
	return;
    }
    if(is_in(argument,"|eat*brandish*zap*fu*quit*mount|"))
    {
        send_to_char( "Illegal command.\n\r", ch );
        return;
    }
    if ( IS_SET(ch->act, PLR_SILENCE) )
    { 
	send_to_char("Not while silenced.\n\r", ch);
	return;
    }

    WAIT_STATE(ch, 6);
    if (IS_NPC(victim)) SET_BIT(victim->act, ACT_COMMANDED);
    interpret(victim,argument);
    if (IS_NPC(victim)) REMOVE_BIT(victim->act, ACT_COMMANDED);

    return;
}

void do_flex( CHAR_DATA *ch, char *argument )
{
    act("You flex your bulging muscles.",ch,NULL,NULL,TO_CHAR);
    act("$n flexes $s bulging muscles.",ch,NULL,NULL,TO_ROOM);

    if ( IS_EXTRA(ch, TIED_UP) )
    {
	act("The ropes restraining you snap.",ch,NULL,NULL,TO_CHAR);
	act("The ropes restraining $n snap.",ch,NULL,NULL,TO_ROOM);
	REMOVE_BIT(ch->extra, TIED_UP);
    }
    if (is_affected(ch, gsn_web))
    {
	act("The webbing entrapping $n breaks away.",ch,NULL,NULL,TO_ROOM);
	send_to_char("The webbing entrapping you breaks away.\n\r",ch);
	affect_strip(ch, gsn_web);
    }
    if (IS_AFFECTED(ch, AFF_WEBBED))
    {
	act("The webbing entrapping $n breaks away.",ch,NULL,NULL,TO_ROOM);
	send_to_char("The webbing entrapping you breaks away.\n\r",ch);
	REMOVE_BIT(ch->affected_by, AFF_WEBBED);
    }
    if (is_affected(ch, gsn_tendrils))
    {
	act("The tendrils of darkness entrapping $n break away.",ch,NULL,NULL,TO_ROOM);
	send_to_char("The tendrils of darkness entrapping you break away.\n\r",ch);
	affect_strip(ch, gsn_tendrils);
    }
    if (IS_AFFECTED(ch, AFF_SHADOW))
    {
	act("The tendrils of darkness entrapping $n break away.",ch,NULL,NULL,TO_ROOM);
	send_to_char("The tendrils of darkness entrapping you break away.\n\r",ch);
	REMOVE_BIT(ch->affected_by, AFF_SHADOW);
    }
    WAIT_STATE(ch,12);
    return;
}

void do_calm( CHAR_DATA *ch, char *argument )
{
    if (IS_NPC(ch)) return;

    if (!IS_WEREWOLF(ch) && !IS_NINJA(ch))
    {
	stc("Huh?\n\r",ch);
	return;
    }

    if (IS_NINJA(ch) && ch->pcdata->wolf > 0)
    {
	stc("You control you actions and calm down.\n\r", ch);
	ch->pcdata->wolf = 0;
	return;
    }
    else if (IS_NINJA(ch))
    {
	send_to_char("You are not in the state of michi.\n\r", ch);
	return;
    }
    else if (IS_WEREWOLF(ch) && get_tribe(ch,TRIBE_CHILDREN_OF_GAIA) < 1)
    {
	stc("You require Children of Gaia 1 to use Calm.\n\r",ch);
	return;
    }

    if (IS_SET(ch->act, PLR_WOLFMAN) || ch->pcdata->wolf > 0)
    {
	send_to_char("You take a deep breath and calm yourself.\n\r",ch);
	act("$n takes a deep breath and tries to calm $mself.",ch,NULL,NULL,TO_ROOM);
	ch->pcdata->wolf -= number_range(25,50);
	if (ch->pcdata->wolf < 100) do_unwerewolf(ch,"");
	WAIT_STATE(ch,12);
	return;
    }
    send_to_char("But you are not in crinos form!\n\r",ch);
    return;
}

void do_web( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MIL];
    long sn;

    argument = one_argument(argument, arg);

    if (IS_NPC(ch)) return;

    if (!IS_NINJA(ch) && !IS_DROW(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (IS_DROW(ch) && !get_drow(ch, DROW_WEB))
    {
        stc("You need to obtain Lloth's web first.\n\r",ch);
        return;
    }

    if (IS_NINJA(ch) && get_ninja(ch, NINJA_CHIKYU) < 8)
    {
	stc("You require 8 Chikyu.\n\r",ch);
	return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL)
    {
	if ((victim = ch->fighting) == NULL)
	{
	    stc("They aren't here.\n\r", ch);
	    return;
	}
    }

    if (is_safe(ch, victim)) return;
    if (IS_NINJA(ch) && ch->pcdata->ninja[NINJA_KI] < 10)
    {
	stc("You don't have enough Ki to use web.\n\r", ch);
	return;
    }

    if ((sn = skill_lookup("web")) < 0) return;
    (*skill_table[sn].spell_fun) (sn, 50, ch, victim);
    if (IS_NINJA(ch)) ch->pcdata->ninja[NINJA_KI] -= 10;
    WAIT_STATE(ch, 12);
    return;
}

void do_entrancement( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg [MIL];
    long sn, level;

    argument = one_argument(argument, arg);

    if (!IS_VAMPIRE(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (get_disc(ch, DISC_PRESENCE) < 3)
    {
	stc("You require level 3 Presence to use Entrancement.\n\r", ch);
	return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL)
    {
	stc("They aren't here.\n\r", ch);
	return;
    }

    if (is_safe(ch, victim)) return;

    if (IS_NPC(victim) && ch->pcdata->followers > 4)
    {
	stc("Nothing happens.\n\r", ch);
	return;
    }

    if (IS_AFFECTED(victim, AFF_CHARM))
    {
	stc("Nothing happens.\n\r", ch);
	return;
    }

    act("You stare deeply into $N's eyes.", ch, NULL, victim, TO_CHAR);
    act("$n stares deeply into $N's eyes.", ch, NULL, victim, TO_NOTVICT);
    act("$n stares deeply into your eyes.", ch, NULL, victim, TO_VICT);

    if ((sn = skill_lookup("charm person")) < 0) return;
    level = get_disc(ch,DISC_PRESENCE) * 60;

    if (IS_NPC(victim) && victim->level > level)
    {
	act("$N shrugs off your attempt to entrance $M and attacks you!", ch, NULL, victim, TO_CHAR);
	act("$N shrugs off $n's attempt to entrance $M and attacks $m.", ch, NULL, victim, TO_NOTVICT);
	multi_hit(victim, ch);
	return;
    }

    if (IS_VAMPIRE(victim) && get_disc(victim, DISC_PRESENCE) >= get_disc(ch, DISC_PRESENCE))
    {
	act("$N shrugs off your attempt to entrance $M.", ch, NULL, victim, TO_CHAR);
	act("$N shrugs off $n's attempt to entrance $M.", ch, NULL, victim, TO_NOTVICT);
	act("You shrug off $n's attempt to entrance you.", ch, NULL, victim, TO_VICT);
	return;
    }

    if (!IS_NPC(victim) && victim->pcdata->resist[WILL_PRESENCE] > 0)
    {
	act("$N shrugs off your attempt to entrance $M.",ch,NULL,victim,TO_CHAR);
	act("$N shrugs off $n's attempt to entrance $M.",ch,NULL,victim,TO_NOTVICT);
	act("You shrug off $n's attempt to entrance you.",ch,NULL,victim,TO_VICT);
	return;
    }

    if (!IS_NPC(victim) && IS_WILLPOWER(victim, RES_PRESENCE) && victim->pcdata->willpower[0] >= 10)
    {
	victim->pcdata->willpower[0] -= 10;
	victim->pcdata->resist[WILL_PRESENCE] = 60;
	if (!IS_WILLPOWER(victim, RES_TOGGLE))
	    REMOVE_BIT(victim->pcdata->resist[0], RES_PRESENCE);
	send_to_char("You burn some willpower to resist Entrancement.\n\r",victim);
	act("$N shrugs off your attempt to entrance $M.",ch,NULL,victim,TO_CHAR);
	act("$N shrugs off $n's attempt to entrance $M.",ch,NULL,victim,TO_NOTVICT);
	act("You shrug off $n's attempt to entrance you.",ch,NULL,victim,TO_VICT);
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
	{
	    guardian_message( victim );
	    act("$n just failed to use Entrancement on you.",ch,NULL,victim,TO_VICT);
	}
	return;
    }

    (*skill_table[sn].spell_fun) ( sn, level, ch, victim );
    WAIT_STATE( ch, 12 );

    if ( !IS_AFFECTED(victim, AFF_CHARM) )
    {
	act("$N shrugs off your attempt to entrance $M.",ch,NULL,victim,TO_CHAR);
	act("$N shrugs off $n's attempt to entrance $M.",ch,NULL,victim,TO_NOTVICT);
	act("You shrug off $n's attempt to entrance you.",ch,NULL,victim,TO_VICT);
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
	{
	    guardian_message( victim );
	    act("$n just failed to use Entrancement on you.",ch,NULL,victim,TO_VICT);
	}
	return;
    }

    act("$N gazes lovingly back at $n.",ch,NULL,victim,TO_NOTVICT);
    if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
    {
	guardian_message( victim );
	act("$n just used Entrancement on you.",ch,NULL,victim,TO_VICT);
    }

    if (IS_NPC(victim))
    {
	free_string(victim->lord);
	victim->lord = str_dup(ch->name);
	ch->pcdata->followers++;
    }
    return;
}

bool char_exists( char *name )
{
        bool valid;
        char path[MAX_STRING_LENGTH];
        FILE *renamefp;

        valid = FALSE;

        fclose(fpReserve);
        sprintf(path, "%s%s", PLAYER_DIR, capitalize( name ) );

        if ( (renamefp = fopen(path, "r")) == NULL) 
                {
			valid = FALSE;
                }
        else
                {
                valid = TRUE;
                fclose(renamefp);
                }

   fpReserve = fopen( NULL_FILE, "r" );
   return valid;
}

void do_powers( CHAR_DATA *ch, char *argument )
{
    char buf [MAX_STRING_LENGTH];
    long sn;
    bool any_powers = FALSE;
    bool any_commands = FALSE;

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch))
    {
	if (IS_WEREWOLF(ch))
	    do_wwpowers(ch, argument);
else if (IS_MAGE(ch))
	    do_magepowers(ch, argument);
	else
	    send_to_char("Huh?\n\r",ch);
	return;
    }

    for ( sn = 0; sn <= DISC_MAX; sn++ )
    {
	if ( get_disc(ch,sn) > 0 ) 
	{
	    any_powers = TRUE;
	    switch (sn)
	    {
		default: break;
case DISC_POTENCE:
    if (get_disc(ch, sn) >= 7) any_commands = TRUE;
    break;
		case DISC_ANIMALISM:
		case DISC_AUSPEX:
		case DISC_CHIMERSTRY:
		case DISC_CELERITY:
		case DISC_DOMINATE:
		case DISC_NECROMANCY:
		case DISC_OBFUSCATE:
		case DISC_OBTENEBRATION:
		case DISC_PRESENCE:
		case DISC_PROTEAN:
		case DISC_QUIETUS:
		case DISC_SERPENTIS:
		case DISC_THAUMATURGY:
		case DISC_VICISSITUDE:
		    any_commands = TRUE;
		    break;
	    }
	}
    }
    send_to_char( "--------------------------------------------------------------------------------\n\r", ch );
    send_to_char("                            -= Discipline Commands =-\n\r",ch);
    send_to_char( "--------------------------------------------------------------------------------\n\r", ch );
    if (get_disc(ch,DISC_ANIMALISM) > 0)
    {
	strcpy(buf,"Animalism: Pigeon");
	if (get_disc(ch,DISC_ANIMALISM) >= 2) strcat(buf," Beckon");
	if (get_disc(ch,DISC_ANIMALISM) >= 3) strcat(buf," Songserenity");
	if (get_disc(ch,DISC_ANIMALISM) >= 4) strcat(buf," Beastwithin");
	if (get_disc(ch,DISC_ANIMALISM) >= 5) strcat(buf," Succulence");
	if (get_disc(ch,DISC_ANIMALISM) >= 6) strcat(buf," Frenzytrigger");
	if (get_disc(ch,DISC_ANIMALISM) == 7) strcat(buf," Fleshbond");
	strcat(buf,".\n\r");
	send_to_char(buf,ch);
    }
    if (get_disc(ch,DISC_AUSPEX) > 0)
    {
	strcpy(buf,"Auspex: Heighten");
	if (get_disc(ch,DISC_AUSPEX) >= 2) strcat(buf," Readaura");
	if (get_disc(ch,DISC_AUSPEX) >= 3) strcat(buf," Scry");
	if (get_disc(ch,DISC_AUSPEX) >= 4) strcat(buf," Unveil");
	if (get_disc(ch,DISC_AUSPEX) >= 6) strcat(buf," Omnipotence");
	if (get_disc(ch,DISC_AUSPEX) >= 7) strcat(buf," Mindblast");
	strcat(buf,".\n\r");
	send_to_char(buf,ch);
    }
    if (get_disc(ch,DISC_POTENCE) >= 7)
        send_to_char("Potence: Fist.\n\r", ch);
    if (get_disc(ch,DISC_CELERITY) > 0)
	send_to_char("Celerity: Celerity.\n\r", ch);
    if (get_disc(ch,DISC_CHIMERSTRY) > 0)
    {
	strcpy(buf,"Chimerstry: Mirror");
	if (get_disc(ch,DISC_CHIMERSTRY) >= 2) strcat(buf," Formillusion");
	if (get_disc(ch,DISC_CHIMERSTRY) >= 3) strcat(buf," Clone");
	if (get_disc(ch,DISC_CHIMERSTRY) >= 4) strcat(buf," Falseappearance");
	if (get_disc(ch,DISC_CHIMERSTRY) >= 5) strcat(buf," Control");
	if (get_disc(ch,DISC_CHIMERSTRY) >= 6) strcat(buf," Truedisguise");
	if (get_disc(ch,DISC_CHIMERSTRY) >= 7) strcat(buf," Horrid");
	strcat(buf,".\n\r");
	send_to_char(buf,ch);
    }
    if (get_disc(ch,DISC_DOMINATE) > 0)
    {
	strcpy(buf,"Dominate: Command");
	if (get_disc(ch,DISC_DOMINATE) >= 3) strcat(buf," Condition");
	if (get_disc(ch,DISC_DOMINATE) >= 3) strcat(buf," Possession");
	strcat(buf,".\n\r");
	send_to_char(buf,ch);
    }
    if (get_disc(ch,DISC_NECROMANCY) > 0)
    {
	strcpy(buf,"Necromancy: Thanatopsis");
	strcat(buf," Spiriteyes");
        if (get_disc(ch,DISC_NECROMANCY) >= 2) strcat(buf," Preserve");
	if (get_disc(ch,DISC_NECROMANCY) >= 3) strcat(buf," Spiritgate");
	if (get_disc(ch,DISC_NECROMANCY) >= 4) strcat(buf," Spiritguardian");
	if (get_disc(ch,DISC_NECROMANCY) >= 5) strcat(buf," Zombie");
	if (get_disc(ch,DISC_NECROMANCY) > 6) strcat(buf," Torment");
	strcat(buf,".\n\r");
	send_to_char(buf,ch);
    }

    if (get_disc(ch,DISC_OBFUSCATE) > 0)
    {
	strcpy(buf,"Obfuscate: Vanish");
	if (get_disc(ch,DISC_OBFUSCATE) > 1) strcat(buf," Mask");
	if (get_disc(ch,DISC_OBFUSCATE) > 4) strcat(buf," Soulmask");
        if (get_disc(ch,DISC_OBFUSCATE) >= 6) strcat(buf," Shield");
	strcat(buf,".\n\r");
	send_to_char(buf,ch);
    }
    if (get_disc(ch,DISC_OBTENEBRATION) > 0)
    {
	strcpy(buf,"Obtenebration: Shroud");
	if (get_disc(ch,DISC_OBTENEBRATION) > 1) strcat(buf," Arms");
	if (get_disc(ch,DISC_OBTENEBRATION) > 2) strcat(buf," Shadowbody");
	if (get_disc(ch,DISC_OBTENEBRATION) > 3) strcat(buf," Lamprey");
	if (get_disc(ch,DISC_OBTENEBRATION) > 4) strcat(buf," Shadowstep");
        if (get_disc(ch,DISC_OBTENEBRATION) > 5) strcat(buf," Shadowforge");
	if (get_disc(ch,DISC_OBTENEBRATION) > 6) strcat(buf," Tenebrous");
	strcat(buf,".\n\r");
	send_to_char(buf,ch);
    }
    if (get_disc(ch,DISC_PRESENCE) > 0)
    {
	strcpy(buf,"Presence: Awe");
	if (get_disc(ch,DISC_PRESENCE) > 1) strcat(buf," Dreadgaze");
	if (get_disc(ch,DISC_PRESENCE) > 2) strcat(buf," Entrance");
	if (get_disc(ch,DISC_PRESENCE) > 3) strcat(buf," Summoning");
	if (get_disc(ch,DISC_PRESENCE) > 4) strcat(buf," Majesty");
	if (get_disc(ch,DISC_PRESENCE) > 5) strcat(buf," Enrage");
	if (get_disc(ch,DISC_PRESENCE) > 6) strcat(buf," Empathy");
	strcat(buf,".\n\r");
	send_to_char(buf,ch);
    }
    if (get_disc(ch,DISC_PROTEAN) > 0)
    {
	strcpy(buf,"Protean: Nightsight");
	if (get_disc(ch,DISC_PROTEAN) > 1) strcat(buf," Claws");
	if (get_disc(ch,DISC_PROTEAN) > 2) strcat(buf," Earthmeld");
	if (get_disc(ch,DISC_PROTEAN) > 3) strcat(buf," Change");
	if (get_disc(ch,DISC_PROTEAN) > 4) strcat(buf," Spiritform");
        if (get_disc(ch,DISC_PROTEAN) > 5) strcat(buf," Earthshift");
        if (get_disc(ch,DISC_PROTEAN) > 6) strcat(buf," Beastform");
	strcat(buf,".\n\r");
	send_to_char(buf,ch);
    }
    if (get_disc(ch,DISC_QUIETUS) > 0)
    {
	strcpy(buf,"Quietus: Silence");
	if (get_disc(ch,DISC_QUIETUS) > 1) strcat(buf," Weakness");
	if (get_disc(ch,DISC_QUIETUS) > 2) strcat(buf," Infirmity");
	if (get_disc(ch,DISC_QUIETUS) > 3) strcat(buf," Bloodagony");
	if (get_disc(ch,DISC_QUIETUS) > 4) strcat(buf," Spit");
	if (get_disc(ch,DISC_QUIETUS) > 5) strcat(buf," Thinblood");
	if (get_disc(ch,DISC_QUIETUS) >= 6) strcat(buf," Leech");
	strcat(buf,".\n\r");
	send_to_char(buf,ch);
    }
    if (get_disc(ch,DISC_SERPENTIS) > 0)
    {
	strcpy(buf,"Serpentis: Eyesoftheserpent");
	if (get_disc(ch,DISC_SERPENTIS) > 1) strcat(buf," Tongue");
	if (get_disc(ch,DISC_SERPENTIS) > 2) strcat(buf," Mummify");
	if (get_disc(ch,DISC_SERPENTIS) > 3) strcat(buf," Serpent");
	if (get_disc(ch,DISC_SERPENTIS) > 6) strcat(buf," Mark");
	strcat(buf,".\n\r");
	send_to_char(buf,ch);
    }

    if (get_disc(ch,DISC_THAUMATURGY) > 0)
    {
strcpy(buf,"Thaumaturgy: Tasteblood");
if (get_disc(ch, DISC_THAUMATURGY) >= 2) strcat(buf," Bloodrage");
if (get_disc(ch, DISC_THAUMATURGY) >= 3) strcat(buf," Potency");
if (get_disc(ch, DISC_THAUMATURGY) >= 4) strcat(buf," Theftvitae");
if (get_disc(ch, DISC_THAUMATURGY) >= 5) strcat(buf," Cauldronblood");
	strcat(buf,".\n\r");
	send_to_char(buf,ch);
    }
    if (get_disc(ch,DISC_VICISSITUDE) > 0)
    {
	strcpy(buf,"Vicissitude: Malleable");
	if (get_disc(ch,DISC_VICISSITUDE) > 1) strcat(buf," Fleshcraft");
	if (get_disc(ch,DISC_VICISSITUDE) > 2) strcat(buf," Bonecraft");
	if (get_disc(ch,DISC_VICISSITUDE) > 3) strcat(buf," Plasmaform");
        if (get_disc(ch,DISC_VICISSITUDE) > 4) strcat(buf," Zuloform");
        if (get_disc(ch,DISC_VICISSITUDE) > 5) strcat(buf," Chiroptean");
	if (get_disc(ch,DISC_VICISSITUDE) > 6) strcat(buf," Disintegrate");
	strcat(buf,".\n\r");
	send_to_char(buf,ch);
    }
    if (!any_powers)
	send_to_char("You have no disciplines.\n\r",ch);
    else if (!any_commands)
	send_to_char("All your discipline powers are automatic.\n\r",ch);
    send_to_char( "--------------------------------------------------------------------------------\n\r", ch );
    return;
}

void do_dreadgaze( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg [MAX_INPUT_LENGTH];
    long sn;
    long level;

    argument = one_argument( argument, arg );

    if (IS_NPC(ch)) return;
    if (!IS_VAMPIRE(ch))
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if (get_disc(ch,DISC_PRESENCE) < 2)
    {
	send_to_char("You require level 2 Presence to use Dread Gaze.\n\r",ch);
	return;
    }

    if (IS_ITEMAFF(ch, ITEMA_PEACE)) do_majesty(ch,"");

    if ( arg[0] == '\0' )
    {
	send_to_char( "Who do you wish to use Dread Gaze on?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if (ch == victim)
    {
	send_to_char( "You're pretty scared of yourself!\n\r", ch );
	return;
    }

    if (is_safe(ch,victim)) return;

    if ( IS_AFFECTED(victim, AFF_FEAR) )
    {
	send_to_char( "They are already pretty scared.\n\r", ch );
	return;
    }

    act("You bare your fangs and snarl at $N.",ch,NULL,victim,TO_CHAR);
    act("$n bares $s fangs and snarls at $N.",ch,NULL,victim,TO_NOTVICT);
    act("$n bares $s fangs and snarls at you.",ch,NULL,victim,TO_VICT);


    if (IS_VAMPIRE(victim) && get_disc(victim,DISC_PRESENCE) >= get_disc(ch,DISC_PRESENCE))
    {
	send_to_char( "Your Dread Gaze has no effect on them.\n\r", ch );
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
	{
	    guardian_message( victim );
	    act("$n just failed to use Dread Gaze on you.",ch,NULL,victim,TO_VICT);
	}
	return;
    }

    if (!IS_NPC(victim) && victim->pcdata->resist[WILL_PRESENCE] > 0)
    {
	send_to_char( "Your Dread Gaze has no effect on them.\n\r", ch );
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
	{
	    guardian_message( victim );
	    act("$n just failed to use Dread Gaze on you.",ch,NULL,victim,TO_VICT);
	}
	return;
    }

    if (!IS_NPC(victim) && IS_WILLPOWER(victim, RES_PRESENCE) && 
	victim->pcdata->willpower[0] >= 10)
    {
	victim->pcdata->willpower[0] -= 10;
	victim->pcdata->resist[WILL_PRESENCE] = 60;
	if (!IS_WILLPOWER(victim, RES_TOGGLE))
	    REMOVE_BIT(victim->pcdata->resist[0], RES_PRESENCE);
	send_to_char( "Your Dread Gaze has no effect on them.\n\r", ch );
	send_to_char("You burn some willpower to resist Dread Gaze.\n\r",victim);
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
	{
	    guardian_message( victim );
	    act("$n just failed to use Dread Gaze on you.",ch,NULL,victim,TO_VICT);
	}
	return;
    }

    if ( ( sn = skill_lookup( "fear" ) ) < 0 ) return;
    level = get_disc(ch,DISC_PRESENCE) * 5;
    (*skill_table[sn].spell_fun) ( sn, level, ch, victim );
    WAIT_STATE( ch, 12 );
    if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
    {
	guardian_message( victim );
	act("$n just used Dread Gaze on you.",ch,NULL,victim,TO_VICT);
    }
    return;
}

void do_discipline( CHAR_DATA *ch, char *argument )
{
    char arg  [MAX_INPUT_LENGTH];
    char buf  [MAX_INPUT_LENGTH];
    char disc [25];
    long  improve;
    long  cost;
    long  max_d = 5;
    long  sn, col;
    bool got_disc = FALSE;

    one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch) )
    {
	send_to_char("Huh?\n\r",ch);
	return;
    }
    if(!str_cmp(ch->clan, "Caitiff") || !str_cmp(ch->clan, "Panders"))
	max_d = 6;
    if ( arg[0] == '\0' )
    {
	send_to_char( "--------------------------------------------------------------------------------\n\r", ch );
	send_to_char("                               -= Disciplines =-\n\r",ch);
	send_to_char( "--------------------------------------------------------------------------------\n\r", ch );
	col    = 0;
	for ( sn = 0; sn <= DISC_MAX; sn++ )
	{
	    if ( ch->pcdata->powers[sn] == 0 ) continue;
	    switch (sn)
	    {
		default: 
		    strcpy(disc,"None"); 
		    break;
		case DISC_ANIMALISM: 
		    strcpy(disc,"Animalism"); 
		    break;
		case DISC_AUSPEX: 
		    strcpy(disc,"Auspex"); 
		    break;
		case DISC_CELERITY: 
		    strcpy(disc,"Celerity"); 
		    break;
		case DISC_CHIMERSTRY: 
		    strcpy(disc,"Chimerstry"); 
		    break;
		case DISC_DOMINATE: 
		    strcpy(disc,"Dominate"); 
		    break;
		case DISC_FORTITUDE: 
		    strcpy(disc,"Fortitude"); 
		    break;
		case DISC_NECROMANCY: 
		    strcpy(disc,"Necromancy"); 
		    break;
		case DISC_OBFUSCATE: 
		    strcpy(disc,"Obfuscate"); 
		    break;
		case DISC_OBTENEBRATION: 
		    strcpy(disc,"Obtenebration"); 
		    break;
		case DISC_POTENCE: 
		    strcpy(disc,"Potence"); 
		    break;
		case DISC_PRESENCE: 
		    strcpy(disc,"Presence"); 
		    break;
		case DISC_PROTEAN: 
		    strcpy(disc,"Protean"); 
		    break;
		case DISC_QUIETUS: 
		    strcpy(disc,"Quietus"); 
		    break;
		case DISC_SERPENTIS: 
		    strcpy(disc,"Serpentis"); 
		    break;
		case DISC_THAUMATURGY: 
		    strcpy(disc,"Thaumaturgy"); 
		    break;
		case DISC_VICISSITUDE: 
		    strcpy(disc,"Vicissitude"); 
		    break;
	    }
	    if (ch->class == CLASS_VAMPIRE && (ch->pcdata->cland[0] == sn || ch->pcdata->cland[1] == sn || ch->pcdata->cland[2] == sn))
		capitalize( disc );

	    sprintf( buf, "%-15s: %-2li       ",
		disc, get_disc(ch, sn) );
	    send_to_char( buf, ch );
	    if ( ++col % 3 == 0 ) send_to_char( "\n\r", ch );
	    got_disc = TRUE;
	}
	if ( col % 3 != 0 ) send_to_char( "\n\r", ch );
	if (!got_disc) send_to_char("You currently have no disciplines.\n\r",ch);
	send_to_char( "--------------------------------------------------------------------------------\n\r", ch );
	send_to_char("To improve a discipline rating, type: discipline <name>\n\r",ch);
	send_to_char( "--------------------------------------------------------------------------------\n\r", ch );
	return;
    }
    if (!str_cmp(arg,"animalism")) improve = DISC_ANIMALISM;
    else if (!str_cmp(arg,"auspex")) improve = DISC_AUSPEX;
    else if (!str_cmp(arg,"celerity")) improve = DISC_CELERITY;
    else if (!str_cmp(arg,"chimerstry")) improve = DISC_CHIMERSTRY;
    else if (!str_cmp(arg,"dominate")) improve = DISC_DOMINATE;
    else if (!str_cmp(arg,"fortitude")) improve = DISC_FORTITUDE;
    else if (!str_cmp(arg,"necromancy")) improve = DISC_NECROMANCY;
    else if (!str_cmp(arg,"obfuscate")) improve = DISC_OBFUSCATE;
    else if (!str_cmp(arg,"obtenebration")) improve = DISC_OBTENEBRATION;
    else if (!str_cmp(arg,"potence")) improve = DISC_POTENCE;
    else if (!str_cmp(arg,"presence")) improve = DISC_PRESENCE;
    else if (!str_cmp(arg,"protean")) improve = DISC_PROTEAN;
    else if (!str_cmp(arg,"quietus")) improve = DISC_QUIETUS;
    else if (!str_cmp(arg,"serpentis")) improve = DISC_SERPENTIS;
    else if (!str_cmp(arg,"thaumaturgy")) improve = DISC_THAUMATURGY;
    else if (!str_cmp(arg,"vicissitude")) improve = DISC_VICISSITUDE;
    else
    {	send_to_char("You know of no such discipline.\n\r",ch);
	return;
    }

    if (ch->pcdata->powers[improve] == 0)
    {
	send_to_char("You know of no such discipline.\n\r",ch);
	return;
    }
    if (ch->pcdata->cland[0] == improve || ch->pcdata->cland[1] == improve || ch->pcdata->cland[2] == improve)
	{	
	max_d = 7;
	if(!str_cmp(ch->clan, "Caitiff") || !str_cmp(ch->clan, "Panders"))
	max_d = 6;
	}
    if (improve == DISC_THAUMATURGY) max_d = 5;

    if (ch->pcdata->powers[improve] >= max_d)
    {
	send_to_char("You are unable to improve your ability in that discipline any further.\n\r",ch);
	return;
    }

    else if (ch->pcdata->cland[0] == improve || 
	ch->pcdata->cland[1] == improve || 
	ch->pcdata->cland[2] == improve || !str_cmp(ch->clan, "Caitiff") 
|| !str_cmp(ch->clan, "Panders"))
    {
	if (ch->pcdata->powers[improve] == -1) 
	    cost = 5;
	else
	    cost = ( ch->pcdata->powers[improve] + 1 ) * 5;
    }
    else
    {
	if (ch->pcdata->powers[improve] == -1) 
	    cost = 15;
	else
	    cost = ((ch->pcdata->powers[improve] + 1) * 15);
    }

    if (ch->primal < cost)
    {
	sprintf(buf,"You need another %li primal to increase that discipline.\n\r", 
	    cost - ch->primal );
	send_to_char(buf,ch);
	return;
    }

    if (ch->pcdata->powers[improve] < 1)
	ch->pcdata->powers[improve] = 1;
    else
	ch->pcdata->powers[improve]++;

    ch->primal -= cost;
    send_to_char("You improve your discipline.\n\r",ch);

    return;
}

void do_pigeon( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch))
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if (get_disc(ch,DISC_ANIMALISM) < 1)
    {
	send_to_char("You require level 1 Animalism to call a Carrier Pigeon.\n\r",ch);
	return;
    }

    if ( arg1[0] == '\0' )
    {
        send_to_char( "Transport which object?\n\r", ch );
        return;
    }

    if ( arg2[0] == '\0' )
    {
        send_to_char( "Transport who whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg2 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if (victim == ch)
    {
	send_to_char( "You cannot send things to yourself!\n\r", ch );
	return;
    }

    if (victim->in_room == NULL || victim->in_room == ch->in_room)
    {
	send_to_char( "But they are already here!\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
    {
	send_to_char( "You are not carrying that item.\n\r", ch );
	return;
    }

    if (obj->weight > 100)
    {
	send_to_char( "That item weighs too much for the pigeon to carry.\n\r", ch );
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
	{
	    guardian_message( victim );
	    act("$n just failed to send $p to you via a pigeon.",ch,obj,victim,TO_VICT);
	}
	return;
    }

    if (!IS_NPC(victim) && !IS_EXTRA(victim, EXTRA_TRANSPORT))
    {
	send_to_char( "You are unable to transport anything to them.\n\r", ch );
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
	{
	    guardian_message( victim );
	    act("$n just failed to send $p to you via a pigeon.",ch,obj,victim,TO_VICT);
	}
	return;
    }

    if ( IS_AFFECTED(ch,AFF_ETHEREAL) || IS_AFFECTED(victim,AFF_ETHEREAL) )
    {
	send_to_char( "Nothing happens.\n\r", ch );
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
	{
	    guardian_message( victim );
	    act("$n just failed to send $p to you via a pigeon.",ch,obj,victim,TO_VICT);
	}
	return;
    }

    if ( !can_see_obj( victim, obj ) )
    {
	act( "$N can't see it.", ch, NULL, victim, TO_CHAR );
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
	{
	    guardian_message( victim );
	    act("$n just failed to send $p to you via a pigeon.",ch,obj,victim,TO_VICT);
	}
	return;
    }

    send_to_char( "A pigeon flies down from the sky and lands on your shoulder.\n\r", ch );
    act("A pigeon flies down from the sky and lands on $n's shoulder.",ch,NULL,NULL,TO_ROOM);
    act("You place $p in the pigeon's beak.",ch,obj,NULL,TO_CHAR);
    act("$n places $p in the pigeon's beak.",ch,obj,NULL,TO_ROOM);
    send_to_char( "The pigeon takes off from your shoulder and flies away.\n\r", ch );
    act("The pigeon takes off from $n's shoulder and flies away.",ch,NULL,NULL,TO_ROOM);

    obj_from_char(obj);
    obj_to_char(obj,victim);

    send_to_char( "A pigeon flies down from the sky and lands on your shoulder.\n\r", victim );
    act("A pigeon flies down from the sky and lands on $n's shoulder.",victim,NULL,NULL,TO_ROOM);
    act("You take $p from the pigeon's beak.",victim,obj,NULL,TO_CHAR);
    act("$n takes $p from the pigeon's beak.",victim,obj,NULL,TO_ROOM);
    send_to_char( "The pigeon takes off from your shoulder and flies away.\n\r", victim );
    act("The pigeon takes off from $n's shoulder and flies away.",victim,NULL,NULL,TO_ROOM);

    do_autosave(ch,"");
    do_autosave(victim,"");
    if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
    {
	guardian_message( victim );
	act("$n just sent $p to you via a pigeon.",ch,obj,victim,TO_VICT);
    }
    return;
}

void do_beckon( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch) && !IS_WEREWOLF(ch))
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if (IS_WEREWOLF(ch) && get_breed(ch,BREED_LUPUS) < 4)
    {
	stc("You require Lupus 4 to use Beast Life.\n\r",ch);
	return;
    }
    else if (IS_VAMPIRE(ch) && get_disc(ch,DISC_ANIMALISM) < 2)
    {
	stc("You require level 2 Animalism to Beckon an animal.\n\r",ch);
	return;
    }

    if (arg[0] == '\0')
    {
	send_to_char("Please select the type of animal you which to summon.\n\r",ch);
	send_to_char("Choose from: Horse, Frog, Raven, Cat, Dog, Yak.\n\r",ch);
	return;
    }

    if (ch->pcdata->followers > 4)
    {
	send_to_char("Nothing happens.\n\r",ch);
	return;
    }

    strcpy(buf, "$N trots into the room." );
    if (!str_cmp(arg,"stallion") || !str_cmp(arg,"horse") || !str_cmp(arg,"mount"))
    {
	ch->pcdata->followers++;
	victim=create_mobile( get_mob_index( MOB_VNUM_MOUNT ) );
	victim->level = 50;
	victim->armor = -500;
	victim->hitroll = 50;
	victim->damroll = 50;
	victim->hit = 5000;
	victim->max_hit = 5000;
	free_string(victim->name);
	victim->name = str_dup("mount black horse stallion");
	sprintf(buf,"%s's black stallion",ch->name);
	free_string(victim->short_descr);
	victim->short_descr = str_dup(buf);
	free_string(victim->long_descr);
	victim->long_descr = str_dup("A beautiful black stallion stands here.\n\r");
	strcpy(buf, "$N trots into the room." );
    }
    else if (!str_cmp(arg,"frog"))
    {
	ch->pcdata->followers++;
	victim=create_mobile( get_mob_index( MOB_VNUM_FROG ) );
	strcpy(buf, "$N hops into the room." );
    }
    else if (!str_cmp(arg,"raven"))
    {
	ch->pcdata->followers++;
	victim=create_mobile( get_mob_index( MOB_VNUM_RAVEN ) );
	strcpy(buf, "$N flies into the room." );
    }
    else if (!str_cmp(arg,"dog"))
    {
	ch->pcdata->followers++;
	victim=create_mobile( get_mob_index( MOB_VNUM_DOG ) );
    }
    else if (!str_cmp(arg,"cat"))
    {
	ch->pcdata->followers++;
	victim=create_mobile( get_mob_index( MOB_VNUM_CAT ) );
    }
    else if (!str_cmp(arg,"molerat"))
    {
	ch->pcdata->followers++;
	victim=create_mobile( get_mob_index( MOB_VNUM_MOLERAT ) );
    }
    else if (!str_cmp(arg,"yak"))
    {
	ch->pcdata->followers++;
	victim=create_mobile( get_mob_index( MOB_VNUM_YAK ) );
    }
    else
    {
	send_to_char("Please select the type of animal you which to summon.\n\r",ch);
	send_to_char("Choose from: Horse, Frog, Raven, Cat, Dog, Yak.\n\r",ch);
	return;
    }
    free_string(victim->lord);
    victim->lord = str_dup(ch->name);
    victim->blood[BLOOD_CURRENT] *= 2;
    victim->blood[BLOOD_POOL] *= 2;
    char_to_room( victim, ch->in_room );
    send_to_char("You whistle loudly.\n\r",ch);
    act( "$n whistles loudly.", ch, NULL, victim, TO_ROOM );
    act( buf, ch, NULL, victim, TO_CHAR );
    act( buf, ch, NULL, victim, TO_ROOM );
    return;
}

void do_beastwithin( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );

    if ( IS_NPC(ch) )
	return;

    if (!IS_VAMPIRE(ch))
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if (get_disc(ch,DISC_ANIMALISM) < 4)
    {
	send_to_char("You require level 4 Animalism to use Beast Within.\n\r",ch);
	return;
    }

    if ( arg[0] == '\0' ) victim = ch;
    else if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if (!IS_VAMPIRE(victim) && !IS_WEREWOLF(victim))
    {
	send_to_char("But they have no beast to call forth!\n\r",ch);
	return;
    }

    if (IS_WEREWOLF(victim) && IS_SET(victim->act, PLR_WOLFMAN))
    {
	send_to_char("Their beast is already controlling them.\n\r",ch);
	return;
    }

    if ( !IS_WEREWOLF(victim) && victim->pcdata->wolf > 0 )
    {
	if (ch == victim)
	    send_to_char("Your beast is already controlling you.\n\r",ch);
	else
	    send_to_char("Their beast is already controlling them.\n\r",ch);
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
	{
	    guardian_message( victim );
	    act("$n just failed to use Beast Within on you.",ch,NULL,victim,TO_VICT);
	}
	return;
    }

    if (!IS_WEREWOLF(victim) && (victim->beast < 20 ))
    {
	if (ch == victim)
	    send_to_char("You have too much control over your beast to use Beast Within.\n\r",ch);
	else
	    send_to_char("They have too much control over their beast to use Beast Within on.\n\r",ch);
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
	{
	    guardian_message( victim );
	    act("$n just failed to use Beast Within on you.",ch,NULL,victim,TO_VICT);
	}
	return;
    }

    if (victim->pcdata->resist[WILL_ANIMALISM] > 0 && ch!=victim)
    {
	send_to_char("You failed.\n\r",ch);
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
	{
	    guardian_message( victim );
	    act("$n just failed to use Beast Within on you.",ch,NULL,victim,TO_VICT);
	}
	return;
    }

    if (IS_WILLPOWER(victim, RES_ANIMALISM) && ch!= victim&& 
	victim->pcdata->willpower[0] >= 10)
    {
	victim->pcdata->willpower[0] -= 10;
	victim->pcdata->resist[WILL_ANIMALISM] = 60;
	if (!IS_WILLPOWER(victim, RES_TOGGLE))
	    REMOVE_BIT(victim->pcdata->resist[0], RES_ANIMALISM);
	send_to_char("You failed.\n\r",ch);
	send_to_char("You burn some willpower to resist Beast Within.\n\r",victim);
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
	{
	    guardian_message( victim );
	    act("$n just failed to use Beast Within on you.",ch,NULL,victim,TO_VICT);
	}
	return;
    }

    if (IS_VAMPIRE(victim) && get_disc(victim,DISC_PROTEAN) > 0 && 
	!IS_VAMPAFF(victim, VAM_NIGHTSIGHT) ) do_nightsight(victim,"");
    if (IS_VAMPIRE(victim) && !IS_VAMPAFF(victim, VAM_FANGS)) do_fangs(victim,"");
    if (IS_VAMPIRE(victim) && get_disc(victim,DISC_PROTEAN) > 1 && 
	!IS_VAMPAFF(victim, VAM_CLAWS) ) do_claws(victim,"");
    if (IS_WEREWOLF(victim))
    {
	victim->pcdata->wolf = number_range(150,200);
	do_werewolf(victim,"");
    }
    else
    {
	if (victim->beast > 0) do_beastlike(victim,"");
	send_to_char("You bare your fangs and growl as your inner beast consumes you.\n\r",victim);
	act("$n bares $s fangs and growls as $s inner beast consumes $m.",victim,NULL,NULL,TO_ROOM);
 	victim->pcdata->wolf += number_range(10,20);
    }
    if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
    {
	guardian_message( victim );
	act("$n just used Beast Within on you.",ch,NULL,victim,TO_VICT);
    }
    WAIT_STATE(ch,12);
    return;
}

void do_serenity( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );

    if ( IS_NPC(ch) )
	return;

    if (!IS_VAMPIRE(ch) && !IS_WEREWOLF(ch))
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if (get_disc(ch,DISC_ANIMALISM) < 3)
    {
	if (get_tribe(ch, TRIBE_CHILDREN_OF_GAIA) < 4)
	{
	    if (IS_WEREWOLF(ch))
		send_to_char("You require the level 4 Children of Gaia power to use Serenity.\n\r",ch);
	    else
		send_to_char("You require level 3 Animalism to use Song of Serenity.\n\r",ch);
	    return;
	}
    }

    if ( arg[0] == '\0' ) victim = ch;
    else if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if (IS_NPC(victim) || (!IS_VAMPIRE(victim) && !IS_SET(victim->act, PLR_WOLFMAN)))
    {
	send_to_char("They have no beast to suppress!\n\r",ch);
	return;
    }

    if ( victim->pcdata->wolf < 1 )
    {
	if (ch == victim)
	    send_to_char("But your beast is not controlling you.\n\r",ch);
	else
	    send_to_char("But their beast is not controlling them.\n\r",ch);

	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
	{
	    guardian_message( victim );
	    act("$n just failed to use Song of Serenity on you.",ch,NULL,victim,TO_VICT);
	}
	return;
    }

    if ((victim->pcdata->resist[WILL_ANIMALISM] > 0 && ch != victim))
    {
	send_to_char("You failed.\n\r",ch);
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
	{
	    guardian_message( victim );
	    act("$n just failed to use Song of Serenity on you.",ch,NULL,victim,TO_VICT);
	}
	return;
    }

    if (IS_WILLPOWER(victim, RES_ANIMALISM) && 	victim->pcdata->willpower[0] >= 10 && ch != victim)
    {
	victim->pcdata->willpower[0] -= 10;
	victim->pcdata->resist[WILL_ANIMALISM] = 60;
	if (!IS_WILLPOWER(victim, RES_TOGGLE))
	    REMOVE_BIT(victim->pcdata->resist[0], RES_ANIMALISM);
	send_to_char("You failed.\n\r",ch);
	send_to_char("You burn some willpower to resist Song of Serenity.\n\r",victim);
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
	{
	    guardian_message( victim );
	    act("$n just failed to use Song of Serenity on you.",ch,NULL,victim,TO_VICT);
	}
	return;
    }

    send_to_char("You take a deep breath and force back your inner beast.\n\r",victim);
    act("$n takes a deep breath and forces back $s inner beast.",victim,NULL,NULL,TO_ROOM);
    victim->pcdata->wolf = 0;
    if (IS_WEREWOLF(victim)) do_unwerewolf(victim,"");
    if (IS_VAMPAFF(victim, VAM_NIGHTSIGHT) && !IS_POLYAFF(victim, POLY_ZULO)) 
	do_nightsight(victim,"");
    if (IS_VAMPAFF(victim, VAM_FANGS) && !IS_POLYAFF(victim, POLY_ZULO)) 
	do_fangs(victim,"");
    if (IS_VAMPAFF(victim, VAM_CLAWS) && !IS_POLYAFF(victim, POLY_ZULO)) 
	do_claws(victim,"");
    if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
    {
	guardian_message( victim );
	act("$n just used Song of Serenity on you.",ch,NULL,victim,TO_VICT);
    }
    WAIT_STATE(ch,12);
    return;
}

void do_unveil( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;
    bool stop_unv = FALSE;

    one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch))
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if (IS_VAMPIRE(ch) && get_disc(ch,DISC_AUSPEX) < 4)
    {
	send_to_char("You require level 4 Auspex to Unveil the Shrouded Mind.\n\r",ch);
	return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char( "Unveil the Shrouded Mind of whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if (IS_NPC(victim))
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if (IS_BUI(victim))
    {
	if (ch != victim) send_to_char( "Nothing happens.\n\r", ch );
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
	{
	    guardian_message( victim );
	    act("$n just failed to use Unveil the Shrouded Mind on you.",ch,NULL,victim,TO_VICT);
	}
	return;
    }

    if ( victim->desc == NULL )
    {
	send_to_char( "Nothing happens.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->snoop_by == ch->desc )
	    {
		d->snoop_by = NULL;
		stop_unv = TRUE;
	    }
	}
	if (stop_unv) send_to_char( "You cease trying to Unveil any minds.\n\r", ch );
	else send_to_char( "How can you Unveil your own shrouded mind?\n\r", ch );
	return;
    }

    if ( victim->desc->snoop_by != NULL )
    {
	send_to_char( "You failed.\n\r", ch );
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
	{
	    guardian_message( victim );
	    act("$n just failed to use Unveil the Shrouded Mind on you.",ch,NULL,victim,TO_VICT);
	}
	return;
    }

    if (IS_MORE(ch, MORE_SHIELDED))
    {
	send_to_char( "You failed.\n\r", ch );
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
	{
	    guardian_message( victim );
	    act("$n just failed to use Unveil the Shrouded Mind on you.",ch,NULL,victim,TO_VICT);
	}
	return;
    }

    if ( ch->desc != NULL )
    {
	for ( d = ch->desc->snoop_by; d != NULL; d = d->snoop_by )
	{
	    if ( d->character == victim || d->original == victim )
	    {
		send_to_char( "Nothing happens.\n\r", ch );
		if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
		{
		    guardian_message( victim );
		    act("$n just failed to use Unveil the Shrouded Mind on you.",ch,NULL,victim,TO_VICT);
		}
		return;
	    }
	}
    }

    if (victim->pcdata->resist[WILL_AUSPEX] > 0)
    {
	send_to_char("You failed.\n\r",ch);
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
	{
	    guardian_message( victim );
	    act("$n just failed to use Unveil the Shrouded Mind on you.",ch,NULL,victim,TO_VICT);
	}
	return;
    }

    if (IS_WILLPOWER(victim, RES_AUSPEX) && 
	victim->pcdata->willpower[0] >= 10)
    {
	victim->pcdata->willpower[0] -= 10;
	victim->pcdata->resist[WILL_AUSPEX] = 60;
	if (!IS_WILLPOWER(victim, RES_TOGGLE))
	    REMOVE_BIT(victim->pcdata->resist[0], RES_AUSPEX);
	send_to_char("You failed.\n\r",ch);
	send_to_char("You burn some willpower to resist Unveil the Shrouded Mind.\n\r",victim);
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
	{
	    guardian_message( victim );
	    act("$n just failed to use Unveil the Shrouded Mind on you.",ch,NULL,victim,TO_VICT);
	}
	return;
    }

    victim->desc->snoop_by = ch->desc;
    send_to_char( "You attempt to Unveil their Shrouded Mind.\n\r", ch );
    if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
    {
	guardian_message( victim );
	act("$n just used Unveil the Shrouded Mind on you.",ch,NULL,victim,TO_VICT);
    }
    return;
}

void do_embrace( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char      arg [MAX_INPUT_LENGTH];

    one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch))
    {
	send_to_char("Huh?\n\r",ch);
	return;
    }

    if ((victim = ch->embrace) != NULL && victim->embrace != NULL &&
	victim->embrace == ch)
    {
	if (ch->embraced == ARE_EMBRACED)
	{
	    send_to_char("You are already being embraced.\n\r",ch);
	    return;
	}
	if (number_percent() < ch->beast && ch->blood[0] < ch->blood[1])
	{
	    send_to_char("You are unable to stop drinking.\n\r",ch);
	    WAIT_STATE(ch, 12);
	    return;
	}
	act("You retract your fangs from $N's neck.",ch,NULL,victim,TO_CHAR);
	act("$n retracts $s fangs from your neck.",ch,NULL,victim,TO_VICT);
	act("$n retracts $s fangs from $N's neck.",ch,NULL,victim,TO_NOTVICT);

	act("You release your hold on $N.",ch,NULL,victim,TO_CHAR);
	act("$n releases $s hold on you.",ch,NULL,victim,TO_VICT);
	act("$n releases $s hold on $N.",ch,NULL,victim,TO_NOTVICT);

	ch->embrace = NULL;
	ch->embraced = ARE_NONE;
	victim->embrace = NULL;
	victim->embraced = ARE_NONE;
	return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char( "Who do you wish to embrace?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch == victim )
    {
	send_to_char( "You cannot embrace yourself, fool!\n\r", ch );
	return;
    }

    if (is_safe(ch,victim)) return;

    if (victim->position == POS_FIGHTING)
    {
	send_to_char( "They are too busy fighting at the moment.\n\r", ch );
	return;
    }

    if (!IS_NPC(victim) && (victim->hit > 0 || victim->position > POS_STUNNED))
    {
	send_to_char( "Not while they are stunned.\n\r", ch );
	return;
    }

    if (victim->embrace != NULL)
    {
	send_to_char( "They are already being embraced.\n\r", ch );
	return;
    }

    if (IS_SET(ch->vampaff, VAM_OBFUSCATE))
    {
 	REMOVE_BIT( ch->vampaff, VAM_OBFUSCATE );

	act("You step out from the shadows.",ch,NULL,victim,TO_CHAR);
	act("$n steps out from the shadows.",ch,NULL,victim,TO_ROOM);
    }    

    act("You step towards $N, with your arms outstretched.",ch,NULL,victim,TO_CHAR);
    act("$n step towards you, with $s arms outstretched.",ch,NULL,victim,TO_VICT);
    act("$n step towards $N, with $s arms outstretched.",ch,NULL,victim,TO_NOTVICT);
    if (IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL &&
	victim->master == ch)
	send_to_char( "You stand helpless before your master.\n\r", victim );
    else if (IS_EXTRA(victim, EXTRA_VAMPIRE))
	act("You stand defenceless before $m",ch,NULL,victim,TO_VICT);
    else if (victim->position < POS_MEDITATING)
	act("You leap onto $S defenseless body.",ch,NULL,victim,TO_CHAR);
    else
    {
	act("$N leaps out of your reach!",ch,NULL,victim,TO_CHAR);
	act("You leap out of $s reach!",ch,NULL,victim,TO_VICT);
	act("$N leap out of $n's reach!",ch,NULL,victim,TO_NOTVICT);
	return;
    }

    act("You grab hold of $N in a close embrace.",ch,NULL,victim,TO_CHAR);
    act("$n grabs hold of you in a close embrace.",ch,NULL,victim,TO_VICT);
    act("$n grabs hold of $N in a close embrace.",ch,NULL,victim,TO_NOTVICT);

    if (IS_MORE(victim, MORE_BRISTLES))
    {
	one_hit( victim, ch, TYPE_HIT, "bristles");
	stop_fighting(victim, TRUE);
    }
    if (ch->position < POS_STANDING) return;

    if (!IS_VAMPAFF(ch, VAM_FANGS)) do_fangs (ch,"");

    act("You sink your fangs into $N's neck.",ch,NULL,victim,TO_CHAR);
    act("$n sinks $s fangs into your neck.",ch,NULL,victim,TO_VICT);
    act("$n sinks $s fangs into $N's neck.",ch,NULL,victim,TO_NOTVICT);

    ch->embrace = victim;
    ch->embraced = ARE_EMBRACING;
    victim->embrace = ch;
    victim->embraced = ARE_EMBRACED;

    return;
}


void update_gen( CHAR_DATA *ch )
{
    if (IS_NPC(ch) || !IS_VAMPIRE(ch)) return;
    switch (ch->vampgen)
    {
	default:
	    ch->pcdata->willpower[0] = 50;
	    ch->pcdata->willpower[1] = 50;
	    ch->blood[BLOOD_POOL] = 100;
	    break;
	case 1:
	    ch->pcdata->willpower[0] = 200;
	    ch->pcdata->willpower[1] = 200;
	    ch->blood[BLOOD_POOL] = 1000;
	    break;
        case 2:
	case 3:
            ch->pcdata->willpower[0] = 100;
            ch->pcdata->willpower[1] = 100;
            ch->blood[BLOOD_POOL] = 800;
	    break;
	case 4:
	case 5:
	    ch->pcdata->willpower[0] = 100;
	    ch->pcdata->willpower[1] = 100;
	    ch->blood[BLOOD_POOL] = 500;
	    break;
	case 6:
	case 7:
	case 8:
	    ch->pcdata->willpower[0] = 90;
	    ch->pcdata->willpower[1] = 90;
	    ch->blood[BLOOD_POOL] = 400;
	    break;
	case 9:
	case 10:
	case 11:
	    ch->pcdata->willpower[0] = 80;
	    ch->pcdata->willpower[1] = 80;
	    ch->blood[BLOOD_POOL] = 300;
	    break;
	case 12:
	case 13:
	    ch->pcdata->willpower[0] = 70;
	    ch->pcdata->willpower[1] = 70;
	    ch->blood[BLOOD_POOL] = 200;
	    break;
    }
    /* Generation changes must not lose an active, serialized capacity buff. */
    if (gsn_potency > 0 && is_affected(ch, gsn_potency))
        ch->blood[BLOOD_POOL] += 100;
    ch->blood[BLOOD_CURRENT] = URANGE(0, ch->blood[BLOOD_CURRENT], ch->blood[BLOOD_POOL]);
    return;
}

void do_formillusion( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    CHAR_DATA *original;
    OBJ_DATA *obj;
    OBJ_DATA *obj2;
    char      arg [MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch) && !IS_WEREWOLF(ch))
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if (get_disc(ch,DISC_CHIMERSTRY) < 2)
    {
	if (get_tribe(ch, TRIBE_FIANNA) < 3)
	{
	    if (IS_WEREWOLF(ch))
		send_to_char("You require the level 3 Fianna power to create an Illusionary Form.\n\r",ch);
	    else
		send_to_char("You require level 2 Chimerstry to create an Illusionary Form.\n\r",ch);
	    return;
	}
    }

    if (ch->pcdata->followers > 4)
    {
	send_to_char("Nothing happens.\n\r",ch);
	return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char( "Who do you wish to Form an Illusion of?\n\r", ch );
	return;
    }

    if ( ( original = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( original->level > 200 )
    {
	send_to_char( "Nothing happens.\n\r", ch );
	return;
    }

    ch->pcdata->followers++;
    victim=create_mobile( get_mob_index( MOB_VNUM_CLONE ) );
    char_to_room(victim,get_room_index(ROOM_VNUM_DISCONNECTION));
    free_string(victim->lord);
    victim->lord = str_dup(ch->name);
    free_string(victim->name);
    victim->name = str_dup(original->name);
    free_string(victim->short_descr);
    if (IS_NPC(original))
	victim->short_descr = str_dup(original->short_descr);
    else
	victim->short_descr = str_dup(original->name);
    free_string(victim->long_descr);
    if (IS_NPC(original))
	victim->long_descr = str_dup(original->long_descr);
    else
	victim->long_descr = str_dup("");

    if (strlen(original->description) > 1)
    {
	free_string(victim->description);
	victim->description = str_dup(original->description);
    }
    SET_BIT(victim->extra, EXTRA_ILLUSION);

    {
	OBJ_DATA *obj_next;
	for ( obj = original->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if (strlen(obj->questowner) != 0 && obj->wear_loc != WEAR_NONE )
	    {
		if (( obj2 = create_object( obj->pIndexData, 0 )) != NULL )
		{
		    SET_BIT(obj2->extra_flags, ITEM_VANISH);
		    obj_to_char(obj2, victim);
		    wear_obj( victim, obj2, obj->wear_loc );
		}
	    }
	}
    }
    if (IS_NPC(original)) victim->level = original->level;
    else victim->level = 50;
    victim->hit = original->hit;
    victim->max_hit = original->max_hit;
    victim->mana = original->mana;
    victim->max_mana = original->max_mana;
    victim->hitroll = char_hitroll(original);
    victim->damroll = char_damroll(original);
    victim->armor = char_ac(original);
    char_from_room(victim);
    char_to_room(victim,original->in_room);

    send_to_char("A mirror image of you shimmers into existance.\n\r",original);
    act("A mirror image of $n appears in the room.",original,NULL,NULL,TO_ROOM);
    return;
}

void do_cloneobject( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    OBJ_DATA *obj2;
    char      arg [MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch) && !IS_WEREWOLF(ch))
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if (get_disc(ch,DISC_CHIMERSTRY) < 3)
    {
	if (get_tribe(ch, TRIBE_FIANNA) < 4)
	{
	    if (IS_WEREWOLF(ch))
		send_to_char("You require the level 4 Fianna power to use Clone Object.\n\r",ch);
	    else
		send_to_char("You require level 3 Chimerstry to use Clone Object.\n\r",ch);
	    return;
	}
    }

    if ( arg[0] == '\0' )
    {
	send_to_char( "What object do you wish to make an illusionary clone of?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
	send_to_char( "You are not carrying that object.\n\r", ch );
	return;
    }

    if (strlen(obj->questmaker) > 1)
    {
	send_to_char( "You are unable to clone that object.\n\r", ch );
	return;
    }

    if (IS_SET(obj->extra_flags, ITEM_VANISH))
    {
	send_to_char( "You are unable to make illusions of illusions!\n\r", ch );
	return;
    }

    if (IS_SET(obj->quest, QUEST_RELIC) || IS_SET(obj->quest, QUEST_ARTIFACT))
    {
	send_to_char( "You are unable to clone that type of object.\n\r", ch );
	return;
    }

    if (strlen(obj->questowner) > 1)
    {
	send_to_char( "You are unable to clone a claimed item.\n\r", ch );
	return;
    }

    if (( obj2 = create_object( obj->pIndexData, obj->level )) != NULL )
    {
	SET_BIT(obj2->extra_flags, ITEM_VANISH);
	obj2->timer = (get_disc(ch,DISC_CHIMERSTRY) * 5);
	obj2->cost = 0;
	switch (obj2->item_type)
	{
	    default:
		obj2->value[0] = obj->value[0];
		obj2->value[1] = obj->value[1];
		obj2->value[2] = obj->value[2];
		obj2->value[3] = obj->value[3];
		break;
	    case ITEM_WAND:
	    case ITEM_STAFF:
	    case ITEM_SCROLL:
	    case ITEM_POTION:
	    case ITEM_PILL:
		obj2->value[0] = 0;
		obj2->value[1] = 0;
		obj2->value[2] = 0;
		obj2->value[3] = 0;
		break;
	}
	obj_to_char(obj2, ch);
	act("An exact duplicate of $p appears in your hands.",ch,obj,NULL,TO_CHAR);
	act("An exact duplicate of $p appears in $n's hands.",ch,obj,NULL,TO_ROOM);
    }
    else
	send_to_char( "Bug - please inform KaVir.\n\r", ch );
    return;
}

void do_rename( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    char      arg1[MAX_INPUT_LENGTH];
    char      arg2[MAX_INPUT_LENGTH];
    char      buf [MAX_STRING_LENGTH];
    char      endchar[15];

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch))
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if (get_disc(ch,DISC_CHIMERSTRY) < 4)
    {
	send_to_char("You require level 4 Chimerstry to give an object a False Appearance.\n\r",ch);
	return;
    }

    if ( arg1[0] == '\0' || arg2[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Syntax: False <object> rename  <short description>\n\r", ch );
	send_to_char( "Syntax: False <object> keyword <extra keyword/s>\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
    {
	send_to_char( "You are not carrying that object.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "rename" ) )
    {
	if (IS_SET(obj->quest, QUEST_RELIC))
	{
	    send_to_char("Not on a relic.\n\r",ch);
	    return;
	}
	if (strlen(argument) < 5)
	{
	    send_to_char("Name should be at least 5 characters long.\n\r",ch);
	    return;
	}
	strcpy(argument,capitalize(argument));
	argument[0] = LOWER(argument[0]);
	endchar[0] = (argument[strlen(argument)-1]);
	endchar[1] = '\0';
	if (!str_cmp(endchar,".")) argument[strlen(argument)-1] = '\0';
	free_string( obj->name );
	obj->name = str_dup( argument );
	free_string( obj->short_descr );
	obj->short_descr = str_dup( argument );
	if (is_in(argument,"|pair of*"))
	    sprintf(buf, "%s are lying here.",argument);
	else
	    sprintf(buf, "%s is lying here.",argument);
	buf[0] = UPPER(buf[0]);
	free_string( obj->description );
	obj->description = str_dup( buf );
	send_to_char("Ok.\n\r",ch);
    	if (obj->questmaker != NULL) free_string(obj->questmaker);
    	obj->questmaker = str_dup(ch->name);
	return;
    }

    if ( !str_cmp( arg2, "keyword" ) || !str_cmp( arg2, "keywords" ) )
    {
	if (IS_SET(obj->quest, QUEST_RELIC))
	{
	    send_to_char("Not on a relic.\n\r",ch);
	    return;
	}
	if (strlen(argument) < 3)
	{
	    send_to_char("Keywords should be at least 3 characters long.\n\r",ch);
	    return;
	}
	strcpy(argument,capitalize(argument));
	argument[0] = LOWER(argument[0]);
	endchar[0] = (argument[strlen(argument)-1]);
	endchar[1] = '\0';
	if (!str_cmp(endchar,".")) argument[strlen(argument)-1] = '\0';
	strcpy(buf,obj->name);
	if (strlen(buf) > 80)
	{
	    send_to_char("You cannot fit any more keywords on this object.\n\r",ch);
	    return;
	}
	strcat(buf," ");
	strcat(buf,argument);
	free_string( obj->name );
	obj->name = str_dup( buf );
	send_to_char("Ok.\n\r",ch);
    	if (obj->questmaker != NULL) free_string(obj->questmaker);
    	obj->questmaker = str_dup(ch->name);
	return;
    }
    send_to_char( "Syntax: False <object> rename  <short description>\n\r", ch );
    send_to_char( "Syntax: False <object> keyword <extra keyword/s>\n\r", ch );
    return;
}

void do_truedisguise( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char      arg [MAX_INPUT_LENGTH];
    char      buf [MAX_STRING_LENGTH];

    argument = one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch))
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if (get_disc(ch,DISC_CHIMERSTRY) < 6)
    {
	send_to_char("You require level 6 Chimerstry to use True Disguise.\n\r",ch);
	return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char( "Change to look like whom?\n\r", ch );
	return;
    }

    if (IS_AFFECTED(ch,AFF_POLYMORPH) && !IS_VAMPAFF(ch,VAM_DISGUISED))
    {
	send_to_char( "Not while polymorphed.\n\r", ch );
	return;
    }

    if (IS_CRAFT(ch, CRAFT_MALLEABLE))
    {
	send_to_char( "You are unable to change your appearance.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_BUI(victim) && victim != ch )
    {
	send_to_char( "You cannot disguise yourself as them.\n\r", ch );
	return;
    }
    if ( ch == victim )
    {
	if (!IS_AFFECTED(ch,AFF_POLYMORPH) && !IS_VAMPAFF(ch,VAM_DISGUISED))
	{
	    send_to_char( "You already look like yourself!\n\r", ch );
	    return;
	}
	if (!IS_POLYAFF(ch, POLY_TRUE))
	{
	    send_to_char("Nothing happens.\n\r",ch);
	    return;
	}
	sprintf(buf,"$n's form shifts into an image of %s.",ch->name);
	act(buf,ch,NULL,NULL,TO_ROOM);
	REMOVE_BIT(ch->affected_by, AFF_POLYMORPH);
	REMOVE_BIT(ch->vampaff, VAM_DISGUISED);
	REMOVE_BIT(ch->polyaff, POLY_TRUE);
    	free_string( ch->morph );
    	ch->morph = str_dup( "" );
	act("Your form shifts into an image of $n.",ch,NULL,NULL,TO_CHAR);
    	free_string( ch->long_descr );
    	ch->long_descr = str_dup( "" );
	return;
    }
    if (IS_VAMPAFF(ch,VAM_DISGUISED))
    {
	if (!IS_POLYAFF(ch, POLY_TRUE))
	{
	    send_to_char("Nothing happens.\n\r",ch);
	    return;
	}
	act("Your form shifts into an image of $N.",ch,NULL,victim,TO_CHAR);
	act("$n's form shifts into an image of you.",ch,NULL,victim,TO_VICT);
	act("$n's form shifts into an image of $N.",ch,NULL,victim,TO_NOTVICT);
    	free_string( ch->morph );
	if (IS_NPC(victim))
	{
	    ch->morph = str_dup( victim->short_descr );
	    free_string( ch->long_descr );
	    ch->long_descr = str_dup( victim->long_descr );
	}
	else
	{
	    ch->morph = str_dup( victim->name );
	    free_string( ch->long_descr );
	    ch->long_descr = str_dup( "" );
	}
	return;
    }
    act("Your form shifts into an image of $N.",ch,NULL,victim,TO_CHAR);
    act("$n's form shifts into an image of you.",ch,NULL,victim,TO_VICT);
    act("$n's form shifts into an image of $N.",ch,NULL,victim,TO_NOTVICT);
    SET_BIT(ch->affected_by, AFF_POLYMORPH);
    SET_BIT(ch->vampaff, VAM_DISGUISED);
    SET_BIT(ch->polyaff, POLY_TRUE);
    free_string( ch->morph );
    if (IS_NPC(victim))
    {
	ch->morph = str_dup( victim->short_descr );
	free_string( ch->long_descr );
	ch->long_descr = str_dup( victim->long_descr );
    }
    else
    {
	ch->morph = str_dup( victim->name );
	free_string( ch->long_descr );
	ch->long_descr = str_dup( "" );
    }
    return;
}

void do_omnipotence( CHAR_DATA *ch, char *argument )
{
    char arg [MAX_INPUT_LENGTH];
    CHAR_DATA *eye;
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch))
    {
	send_to_char("Huh?\n\r",ch);
	return;
    }

    if (IS_VAMPIRE(ch) && get_disc(ch,DISC_AUSPEX) < 6)
    {
	send_to_char("You require level 6 Auspex to use Omnipotence.\n\r",ch);
	return;
    }
    if ( arg[0] == '\0' )
    {
	if ( ( eye = ch->pcdata->familiar ) != NULL && 
	    eye->pIndexData->vnum == MOB_VNUM_EYE) 
	{
	if(IS_VAMPIRE(ch))
	    send_to_char( "You are no longer using your Omnipotence.\n\r",ch );
	else
	    send_to_char( "You are no longer using Send Vision.\n\r", ch);
	    ch->pcdata->familiar = NULL;
	    eye->wizard = NULL;
	    extract_char(eye, TRUE);
	    return;
	}
	send_to_char( "Who do you wish to use Omnipotence on?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch == victim )
    {
	send_to_char( "Now that would be completely pointless.\n\r", ch );
	return;
    }

    if (IS_BUI(victim) || IS_MORE(victim,MORE_SHIELDED))
    {
	send_to_char( "You are unable to locate them.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY) )
    {
	send_to_char( "Nothing happens.\n\r", ch );
	return;
    }

    if ( ( eye = ch->pcdata->familiar ) != NULL )
    {
	if ( eye->pIndexData->vnum == MOB_VNUM_EYE ) 
	{
	    if(IS_VAMPIRE(ch))
	    send_to_char( "You have switched the target of your Omnipotence.\n\r",ch );
	    else
	    send_to_char("You send your vision to another.\n\r", ch);
	    char_from_room( eye );
	    char_to_room( eye, victim->in_room );
	    if ( eye->master != NULL ) stop_follower( eye );
	    eye->master        = victim;
	    eye->leader        = NULL;
	}
	else
	    send_to_char("Nothing happens.\n\r",ch);
	return;
    }

    if (ch->pcdata->followers > 4 || victim->in_room == NULL)
    {
	send_to_char("Nothing happens.\n\r",ch);
	return;
    }

    ch->pcdata->followers++;

    eye=create_mobile( get_mob_index( MOB_VNUM_EYE ) );
    eye->level = 1;
    eye->hit = 1;
    eye->max_hit = 1;
    eye->hitroll = 0;
    eye->damroll = 0;
    eye->armor = 100;

    free_string(eye->name);
    eye->name = str_dup("eye eyeball");
    free_string(eye->short_descr);
    eye->short_descr = str_dup("a small spectral eye");
    free_string(eye->long_descr);
    eye->long_descr = str_dup("A small spectral eye floats here.\n\r");
    free_string(eye->lord);
    eye->lord = str_dup(ch->name);
    send_to_char( "You are now using your Omnipotence.\n\r",ch );
    char_to_room( eye, victim->in_room );
    ch->pcdata->familiar = eye;
    eye->wizard = ch;

    if ( eye->master != NULL )
    {
	send_to_char( "Bug - please inform KaVir.\n\r", ch );
	return;
    }

    eye->master        = victim;
    eye->leader        = NULL;
    SET_BIT(eye->more, MORE_SPIRIT);
    SET_BIT(eye->vampaff, VAM_SPIRITEYES);
    SET_BIT(eye->act, ACT_NOEXP);
    SET_BIT(eye->affected_by, AFF_ETHEREAL);
    SET_BIT(eye->affected_by, AFF_FLYING);
    return;
}


char *disc_name( long disc_no )
{
    static char disc[20];

    switch ( disc_no )
    {
	default: 
	    strcpy(disc,"None"); 
	    break;
	case DISC_ANIMALISM: 
	    strcpy(disc,"Animalism"); 
	    break;
	case DISC_AUSPEX: 
	    strcpy(disc,"Auspex"); 
	    break;
	case DISC_CELERITY: 
	    strcpy(disc,"Celerity"); 
	    break;
	case DISC_CHIMERSTRY: 
	    strcpy(disc,"Chimerstry"); 
	    break;
	case DISC_DOMINATE: 
	    strcpy(disc,"Dominate"); 
	    break;
	case DISC_FORTITUDE: 
	    strcpy(disc,"Fortitude"); 
	    break;
	case DISC_NECROMANCY: 
	    strcpy(disc,"Necromancy"); 
	    break;
	case DISC_OBFUSCATE: 
	    strcpy(disc,"Obfuscate"); 
	    break;
	case DISC_OBTENEBRATION: 
	    strcpy(disc,"Obtenebration"); 
	    break;
	case DISC_POTENCE: 
	    strcpy(disc,"Potence"); 
	    break;
	case DISC_PRESENCE: 
	    strcpy(disc,"Presence"); 
	    break;
	case DISC_PROTEAN: 
	    strcpy(disc,"Protean"); 
	    break;
	case DISC_QUIETUS: 
	    strcpy(disc,"Quietus"); 
	    break;
	case DISC_SERPENTIS: 
	    strcpy(disc,"Serpentis"); 
	    break;
	case DISC_THAUMATURGY: 
	    strcpy(disc,"Thaumaturgy"); 
	    break;
	case DISC_VICISSITUDE: 
	    strcpy(disc,"Vicissitude"); 
	    break;
    }
    return &disc[0];
}

void do_heal(CHAR_DATA *ch, char *argument )
{
    if (IS_NPC(ch)) return;

    if (ch->class != CLASS_NONE)
    {
	stc("Only the unclassed may use Heal.\n\r", ch);
	return;
    }
    if (ch->in_room != NULL && ch->in_room->vnum == ROOM_VNUM_TEMPLE && ch->class == CLASS_NONE)
    {
	stc("The gods have restored you.\n\r", ch);
	ch->hit = ch->max_hit;
	ch->mana = ch->max_mana;
	update_pos(ch);
	return;
    }
    stc("You must go to the temple for the gods to heal you.\n\r", ch);
    return;
}


void do_autoclass( CHAR_DATA *ch, char *argument )
{
    char arg1[MSL];
    char arg2[MSL];
    const char *clan = NULL;

    if (IS_NPC(ch)) return;
    if (ch->pcdata == NULL)
    {
stc("Your player data is unavailable; please contact an immortal.\n\r", ch);
return;
    }

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (ch->class != CLASS_NONE)
    {
	stc("You already have a class.\n\r", ch);
	return;
    }

    if (!IS_AWA(ch))
    {
	stc("You must obtain at least 5 mkills first.\n\r", ch);
	return;
    }

    if (arg1[0] == '\0')
    {
	stc("Syntax: autoclass <class> <clan(if applicable)> \n\r", ch);
	stc("Classes:\n\r", ch);
	stc("Dragon, #PDrow#n, #cElementalist#n, #CHighlander#n, #pMage#n, #7Monk#n, #0Ninja#n, #LSorcerer#n, #rVampire#n, #oWerewolf#n.\n\r", ch);
	stc("Clans:\n\r", ch);
	stc("Dragon:  #oBronze#n, #RRed#n, #LBlue#n, #7Silver#n, #yGold#n, #0Black#n.\n\r", ch);
	stc("#rVampire#n:\n\r", ch);
stc("   Camarilla:   Ventrue, Nosferatu, Brujah\n\r", ch);
	stc("   Sabbat:      Serpentsofthelight, Lasombra, Tzimisce \n\r", ch);
stc("   Independent: Assamite, Gangrel, Ravnos\n\r", ch);
	stc("#pMage#n: Akashic, Celestial, Ecstasy, Dreamspeakers, Euthanatos, Hermes, Ether, Verbena, Virtual, Hollow\n\r", ch);

	return;
    }

    if (!str_cmp(arg1, "vampire"))
    {
	if (raw_stat(ch, STAT_STR) < 3 || raw_stat(ch, STAT_DEX) < 2 || raw_stat(ch, STAT_CON) < 4
	 || raw_stat(ch, STAT_INT) < 2 || raw_stat(ch, STAT_WIS) < 1)
	{
	    stc("You do not meet the requirements for that class.\n\r", ch);
	    return;
	}

	if (!str_cmp(arg2, "ventrue"))
	{
    clan = "Ventrue";
	    ch->pcdata->cland[0] = DISC_DOMINATE;
	    ch->pcdata->cland[1] = DISC_FORTITUDE;
	    ch->pcdata->cland[2] = DISC_PRESENCE;
	}
	else if (!str_cmp(arg2, "nosferatu"))
	{
    clan = "Nosferatu";
	    ch->pcdata->cland[0] = DISC_ANIMALISM;
	    ch->pcdata->cland[1] = DISC_OBFUSCATE;
	    ch->pcdata->cland[2] = DISC_POTENCE;
	}
	else if (!str_cmp(arg2, "brujah"))
	{
    clan = "Brujah";
	    ch->pcdata->cland[0] = DISC_CELERITY;
	    ch->pcdata->cland[1] = DISC_POTENCE;
	    ch->pcdata->cland[2] = DISC_PRESENCE;
	}
	else if (!str_cmp(arg2, "assamite"))
	{
    clan = "Assamite";
	    ch->pcdata->cland[0] = DISC_CELERITY;
	    ch->pcdata->cland[1] = DISC_OBFUSCATE;
	    ch->pcdata->cland[2] = DISC_QUIETUS;
	    ch->pcdata->powers[DISC_QUIETUS] = -1;
	}
	else if (!str_cmp(arg2, "gangrel"))
	{
    clan = "Gangrel";
	    ch->pcdata->cland[0] = DISC_ANIMALISM;
	    ch->pcdata->cland[1] = DISC_FORTITUDE;
	    ch->pcdata->cland[2] = DISC_PROTEAN;
	    ch->pcdata->powers[DISC_PROTEAN] = -1;
	}
	else if (!str_cmp(arg2, "serpentsofthelight"))
	{
    clan = "Serpents of the Light";
	    ch->pcdata->cland[0] = DISC_OBFUSCATE;
	    ch->pcdata->cland[1] = DISC_PRESENCE;
	    ch->pcdata->cland[2] = DISC_SERPENTIS;
	    ch->pcdata->powers[DISC_SERPENTIS] = -1;
	}
	else if (!str_cmp(arg2, "lasombra"))
	{
    clan = "Lasombra";
	    ch->pcdata->cland[0] = DISC_DOMINATE;
	    ch->pcdata->cland[1] = DISC_OBTENEBRATION;
	    ch->pcdata->cland[2] = DISC_POTENCE;
	    ch->pcdata->powers[DISC_OBTENEBRATION] = -1;
	}
	else if (!str_cmp(arg2, "tzimisce"))
	{
    clan = "Tzimisce";
	    ch->pcdata->cland[0] = DISC_AUSPEX;
	    ch->pcdata->cland[1] = DISC_ANIMALISM;
	    ch->pcdata->cland[2] = DISC_VICISSITUDE;
	    ch->pcdata->powers[DISC_VICISSITUDE] = -1;
	}
	else if (!str_cmp(arg2, "ravnos"))
	{
    clan = "Ravnos";
	    ch->pcdata->cland[0] = DISC_FORTITUDE;
            ch->pcdata->cland[1] = DISC_ANIMALISM;
            ch->pcdata->cland[2] = DISC_CHIMERSTRY;
            ch->pcdata->powers[DISC_CHIMERSTRY] = -1;
	}
	else
	{
	    do_autoclass(ch, "");
	    return;
	}

free_string(ch->clan);
ch->clan = str_dup(clan);
ch->class = CLASS_VAMPIRE;
	ch->vampgen = 9;
	update_gen(ch);
	ch->pcdata->powers[DISC_ANIMALISM] = -1;
	ch->pcdata->powers[DISC_AUSPEX] = -1;
	ch->pcdata->powers[DISC_CELERITY] = -1;
	ch->pcdata->powers[DISC_DOMINATE] = -1;
	ch->pcdata->powers[DISC_FORTITUDE] = -1;
	ch->pcdata->powers[DISC_OBFUSCATE] = -1;
	ch->pcdata->powers[DISC_POTENCE] = -1;
	ch->pcdata->powers[DISC_PRESENCE] = -1;
	stc("You are now a #rVampire#n.\n\r", ch);
    }
    else if (!str_cmp(arg1, "werewolf"))
    {
	if (raw_stat(ch, STAT_STR) < 4 || raw_stat(ch, STAT_DEX) < 2 || raw_stat(ch, STAT_CON) < 3
	 || raw_stat(ch, STAT_INT) < 1 || raw_stat(ch, STAT_WIS) < 2)
	{
	    stc("You do not meet the requirements for this class.\n\r", ch);
	    return;
	}
	ch->class = CLASS_WEREWOLF;
	ch->pcdata->rank = 1;
	ch->vampgen = 6;
	ch->pcdata->tgift[0] = -1;
	ch->pcdata->tgift[1] = -1;
	ch->pcdata->tgift[2] = -1;
	stc("You are now a #oWerewolf#n.\n\r", ch);
	stc("Use the #otribegift#n command to select your primary gifts.\n\r", ch);
    }
    else if (!str_cmp(arg1, "sorcerer"))
    {
	if (raw_stat(ch, STAT_STR) < 2 || raw_stat(ch, STAT_DEX) < 2 || raw_stat(ch, STAT_CON) < 1
	 || raw_stat(ch, STAT_INT) < 4 || raw_stat(ch, STAT_WIS) < 3)
	{
	    stc("You do not meet the requirements for this class.\n\r", ch);
	    return;
	}
	ch->class = CLASS_SORCERER;
	ch->level = LEV_SOR;
	ch->vampgen = 4;
	stc("You are now a #LSorcerer#n.\n\r", ch);	
    }
    else if (!str_cmp(arg1, "monk"))
    {
	if (raw_stat(ch, STAT_STR) < 2 || raw_stat(ch, STAT_DEX) < 3 || raw_stat(ch, STAT_CON) < 2
	 || raw_stat(ch, STAT_INT) < 1 || raw_stat(ch, STAT_WIS) < 4)
	{
	    stc("You do not meet the requirements for that class.\n\r", ch);
	    return;
	}

	ch->class = CLASS_MONK;
	ch->vampgen = 4;
	stc("You are now a #7Monk#n.\nr\r", ch);
    }
    else if (!str_cmp(arg1, "dragon"))
    {
	if (raw_stat(ch, STAT_STR) < 4 || raw_stat(ch, STAT_DEX) < 1 || raw_stat(ch, STAT_CON) < 3
	 || raw_stat(ch, STAT_INT) < 2 || raw_stat(ch, STAT_WIS) < 2)
	{
	    stc("You do not meet the requirements for that class.\n\r", ch);
	    return;
	}

	if (!str_cmp(arg2, "bronze")) ch->pcdata->dragcol = PURPLE_MAGIC;
	else if (!str_cmp(arg2, "red")) ch->pcdata->dragcol = RED_MAGIC;
	else if (!str_cmp(arg2, "blue")) ch->pcdata->dragcol = BLUE_MAGIC;
	else if (!str_cmp(arg2, "silver")) ch->pcdata->dragcol = GREEN_MAGIC;
	else if (!str_cmp(arg2, "gold")) ch->pcdata->dragcol = YELLOW_MAGIC;
	else if (!str_cmp(arg2, "black")) ch->pcdata->dragcol = BLACK_MAGIC;
	else
	{
	    do_autoclass(ch, "");
	    return;
	}

	ch->class = CLASS_DRAGON;
	ch->vampgen = 12;
	stc("You are now a Dragon.\n\r", ch);
    }
    else if (!str_cmp(arg1, "highlander"))
    {
	if (raw_stat(ch, STAT_STR) < 4 || raw_stat(ch, STAT_DEX) < 2 || raw_stat(ch, STAT_CON) < 3
	 || raw_stat(ch, STAT_INT) < 1 || raw_stat(ch, STAT_WIS) < 2)
	{
	    stc("You do not meet the requirements for that class.\n\r", ch);
	    return;
	}

	ch->class = CLASS_HIGHLANDER;
	stc("You are now a #CHighlander#n.\n\r", ch);
    }
    else if (!str_cmp(arg1, "ninja"))
    {
	if (raw_stat(ch, STAT_STR) < 3 || raw_stat(ch, STAT_DEX) < 4 || raw_stat(ch, STAT_CON) < 2
	 || raw_stat(ch, STAT_INT) < 2 || raw_stat(ch, STAT_WIS) < 1)
	{
	    stc("You do not meet the requirements for that class.\n\r", ch);
	    return;
	}

	ch->class = CLASS_NINJA;
	ch->vampgen = 4;
	stc("You are now a #0Ninja#n.\n\r", ch);
    }
    else if (!str_cmp(arg1, "drow"))
    {
	if (raw_stat(ch, STAT_STR) < 2 || raw_stat(ch, STAT_DEX) < 4 || raw_stat(ch, STAT_CON) < 1
	 || raw_stat(ch, STAT_INT) < 3 || raw_stat(ch, STAT_WIS) < 2)
	{
	    stc("You do not meet the requirements for that class.\n\r", ch);
	    return;
	}

	ch->class = CLASS_DROW;
	ch->vampgen = 5;
	stc("You are now a #PDrow#n.\n\r", ch);
    }
    else if (!str_cmp(arg1, "elementalist"))
    {
	if (raw_stat(ch, STAT_STR) < 2 || raw_stat(ch, STAT_DEX) < 1 || raw_stat(ch, STAT_CON) < 2
	 || raw_stat(ch, STAT_INT) < 3 || raw_stat(ch, STAT_WIS) < 4)
	{
	    stc("You do not meet the requirements for that class.\n\r", ch);
	    return;
	}

	ch->class = CLASS_ELEM;
	stc("You are now an #cElementalist#n.\n\r", ch);
    }
    else if (!str_cmp(arg1, "mage"))
    {
	if (raw_stat(ch, STAT_STR) < 1 || raw_stat(ch, STAT_DEX) < 2 || raw_stat(ch, STAT_CON) < 2
	 || raw_stat(ch, STAT_INT) < 4 || raw_stat(ch, STAT_WIS) < 3)
	{
	    stc("You do not meet the requirements for that class.\n\r", ch);
	    return;
	}

 	if (!str_cmp(arg2, "akashic"))
        {
            clan = "Akashic Brotherhood";
            ch->pcdata->cland[0] = SPHERE_MIND;
        }
	else if (!str_cmp(arg2, "celestial"))
        {
            clan = "Celestial Chorus";
            ch->pcdata->cland[0] = SPHERE_PRIME;

        }
	else if (!str_cmp(arg2, "ecstasy"))
        {
            clan = "Cult of Ecstasy";
            ch->pcdata->cland[0] = SPHERE_TIME;

        }
	else if (!str_cmp(arg2, "dreamspeakers"))
        {
            clan = "Dreamspeakers";
            ch->pcdata->cland[0] = SPHERE_SPIRIT;

        }
	else if (!str_cmp(arg2, "euthanatos"))
        {
            clan = "Euthanatos";
            ch->pcdata->cland[0] = SPHERE_ENTROPY;

        }
	else if (!str_cmp(arg2, "hermes"))
        {
            clan = "Order of Hermes";
            ch->pcdata->cland[0] = SPHERE_FORCES;

        }
	else if (!str_cmp(arg2, "ether"))
        {
            clan = "Sons of Ether";
            ch->pcdata->cland[0] = SPHERE_MATTER;
        }
	else if (!str_cmp(arg2, "verbena"))
        {
            clan = "Verbena";
            ch->pcdata->cland[0] = SPHERE_LIFE;
        }
	else if (!str_cmp(arg2, "virtual"))
        {
            clan = "Virtual Adepts";
            ch->pcdata->cland[0] = SPHERE_CORRESPONDENCE;
        }
	else if (!str_cmp(arg2, "hollow"))
        {
            clan = "Hollow Ones";
            ch->pcdata->cland[0] = -1;
        }
	else 
	{
	    do_autoclass(ch, "");
	    return;
	}
free_string(ch->clan);
ch->clan = str_dup(clan);
ch->class = CLASS_MAGE;
	ch->pcdata->cland[1] = -1;
        ch->pcdata->cland[2] = -1;
	ch->arete = 1;

	stc("You are now a #pMage#n.\n\r", ch);
        stc("Use the #pselectspheres#n command to select your clan spheres.\n\r", ch);
    }
    else do_autoclass(ch, "");

    return;
}

void do_thinblood( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg [MAX_INPUT_LENGTH];
    
    argument = one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch))
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if (get_disc(ch,DISC_QUIETUS) < 6)
    {
	send_to_char("You require level 6 Quietus to use Thinned Blood.\n\r",ch);
	return;
    }

    if (IS_ITEMAFF(ch, ITEMA_PEACE)) do_majesty(ch,"");

    if ( arg[0] == '\0' )
    {
	send_to_char( "Who do you wish to use Thinned Blood on?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if (ch == victim)
    {
	send_to_char( "Why thin your own blood?\n\r", ch );
	return;
    }

    

    if ( IS_MORE(victim, MORE_THINBLOOD) )
    {
	send_to_char( "Their Blood can become no thinner.\n\r", ch );
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
	{
	    guardian_message( victim );
	    act("$n just failed to use Thinned Blood on you.",ch,NULL,victim,TO_VICT);
	}
	return;
    }

    if ( !IS_VAMPIRE(victim) )
    {
	send_to_char( "You can only thin the blood of vampires.\n\r", ch );
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
	{
	    guardian_message( victim );
	    act("$n just failed to use Consuming Fear on you.",ch,NULL,victim,TO_VICT);
	}
	return;
    }

    stc("You thin their blood.\n\r",ch);
    send_to_char("Your blood has thinned drastically!\n\r",victim);

    if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
    {
	guardian_message( victim );
	act("$n just used Thinned Blood on you.",ch,NULL,victim,TO_VICT);
    }

    SET_BIT(victim->more, MORE_THINBLOOD);

    WAIT_STATE( ch, 12 );
    return;
}

