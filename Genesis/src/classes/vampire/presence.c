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

/* Vampire-only Presence toggles extracted from clan.c without rule changes. */
#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include "merc.h"

void do_majesty( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch))
    {
	send_to_char("Huh?\n\r",ch);
	return;
    }
    if (get_disc(ch,DISC_PRESENCE) < 5)
    {
	send_to_char("You require level 5 Presence to use Majesty.\n\r",ch);
	return;
    }

    if (IS_ITEMAFF(ch,ITEMA_PEACE) )
    {
	send_to_char("You lower your Majesty.\n\r",ch);
	act("$n no longer seems so imposing.", ch, NULL, NULL, TO_ROOM);
	REMOVE_BIT(ch->itemaffect, ITEMA_PEACE);
	ch->pcdata->majesty = 120;
	return;
    }
    if (ch->pcdata->majesty > 0)
    {
	if (ch->pcdata->majesty > 1)
	    sprintf(buf,"You cannot raise your Majesty for another %li seconds.\n\r",ch->pcdata->majesty);
	else
	    sprintf(buf,"You cannot raise your Majesty for another 1 second.\n\r");
	send_to_char(buf,ch);
	return;
    }
    send_to_char("Your raise your Majesty.\n\r",ch);
    act("$n suddenly appears very imposing.", ch, NULL, NULL, TO_ROOM);
    SET_BIT(ch->itemaffect, ITEMA_PEACE);
    return;
}

void do_awe( CHAR_DATA *ch, char *argument )
{
    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch))
    {
	send_to_char("Huh?\n\r",ch);
	return;
    }
    if (get_disc(ch,DISC_PRESENCE) < 1)
    {
	send_to_char("You require level 1 Presence to use Awe.\n\r",ch);
	return;
    }

    if (IS_EXTRA(ch,EXTRA_AWE) )
    {
	send_to_char("You are no longer awe-inspiring.\n\r",ch);
	act("$n no longer appears so awe-inspiring.", ch, NULL, NULL, TO_ROOM);
	REMOVE_BIT(ch->extra, EXTRA_AWE);
	return;
    }
    send_to_char("You are now awe-inspiring.\n\r",ch);
    act("$n suddenly appears very awe-inspiring.", ch, NULL, NULL, TO_ROOM);
    SET_BIT(ch->extra, EXTRA_AWE);
    return;
}
