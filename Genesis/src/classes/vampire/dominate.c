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

/* Legacy Dominate binding rules are preserved, not redesigned here. */
#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include "merc.h"

void do_possession( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    CHAR_DATA *familiar;
    char arg [MAX_INPUT_LENGTH];

    one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch))
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if (get_disc(ch,DISC_DOMINATE) < 3)
    {
	send_to_char("You require level 3 Dominate to use Possession.\n\r",ch);
	return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char( "What do you wish to possess?\n\r", ch );
	return;
    }

    /* The reciprocal wizard/familiar link must have a single owner. */
    if (ch->pcdata->familiar != NULL)
    {
	send_to_char( "Nothing happens.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch == victim )
    {
	send_to_char( "I think not...\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) )
    {
	send_to_char( "Not on players.\n\r", ch );
	return;
    }

    if (!can_see(victim,ch))
    {
	if (get_disc(ch,DISC_DOMINATE) < 6)
	{
	    send_to_char( "You are unable to make eye contact with them.\n\r", ch );
	    return;
	}
    }

    act("You stare deeply into $N's eyes.",ch,NULL,victim,TO_CHAR);
    act("$n stares deeply into $N's eyes.",ch,NULL,victim,TO_NOTVICT);
    act("$n stares deeply into your eyes.",ch,NULL,victim,TO_VICT);

    if ( ( familiar = victim->wizard ) != NULL ) 
    {
	send_to_char( "Nothing happens.\n\r", ch );
	return;
    }

    if (victim->level > (get_disc(ch,DISC_DOMINATE) * 75))
    {
	send_to_char( "Nothing happens.\n\r", ch );
	return;
    }

    act("You possess $N's body.",ch,NULL,victim,TO_CHAR);
    ch->pcdata->familiar = victim;
    victim->wizard = ch;
    return;
}
