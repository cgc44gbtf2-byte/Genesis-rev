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

/* Existing Vanish/Cloak the Gathering behavior extracted from clan.c. */
#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include "merc.h"

void do_vanish( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    char arg [MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch))
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if (get_disc(ch,DISC_OBFUSCATE) < 1)
    {
	send_to_char("You require level 1 Obfuscate to Vanish from sight.\n\r",ch);
	return;
    }

    act( "A look of concentration crosses $n's face.", ch, NULL, NULL, TO_ROOM );

    if (get_disc(ch,DISC_OBFUSCATE) >= 3 && !str_cmp(arg,"all"))
    {
	send_to_char("You Cloak the Gathering.\n\r",ch);
	for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
	{
	    vch_next = vch->next_in_room;
	    if ( vch == ch || vch->leader == ch )
	    {
		SET_BIT( vch->vampaff, VAM_OBFUSCATE );
		act( "You fade away into the shadows.", vch, NULL, NULL, TO_CHAR );
		act( "$n fades away into the shadows.", vch, NULL, NULL, TO_ROOM );
	    }
	}
    }
    else
    {
	SET_BIT( ch->vampaff, VAM_OBFUSCATE );
	act( "You fade away into the shadows.", ch, NULL, NULL, TO_CHAR );
	act( "$n fades away into the shadows.", ch, NULL, NULL, TO_ROOM );
    }
    WAIT_STATE( ch, 12 );
    return;
}
