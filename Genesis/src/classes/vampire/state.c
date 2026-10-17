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

/* Existing Vampire state toggles; no new rank or resource rules. */
#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include "merc.h"

void do_regenerate( CHAR_DATA *ch, char *argument )
{
    if (IS_NPC(ch)) return;
    if (!IS_VAMPIRE(ch))
    {
	stc("Huh?\n\r",ch);
	return;
    }
    if (ch->pcdata->regenerate == 0)
    {
	ch->pcdata->regenerate = 10;
	stc("You begin regenerating yourself.\n\r",ch);
    }
    else
    {
	ch->pcdata->regenerate = 0;
	stc("You stop regenerating yourself.\n\r",ch);
    }
    return;
}

void do_earthmeld( CHAR_DATA *ch, char *argument )
{
    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch) && get_disc(ch, DISC_PROTEAN) < 3)
    {
	send_to_char("Huh?\n\r",ch);
	return;
    }
    if (get_disc(ch,DISC_PROTEAN) < 3)
    {
	send_to_char("You require level 3 Protean to use Earthmeld.\n\r",ch);
	return;
    }

    if (IS_VAMPAFF(ch,VAM_EARTHMELDED) )
    {
	REMOVE_BIT(ch->vampaff, VAM_EARTHMELDED);
	send_to_char("You rise up from the ground.\n\r",ch);
	act("$n rises up from the ground.", ch, NULL, NULL, TO_ROOM);
	do_look(ch,"auto");
	return;
    }
    send_to_char("You sink into the ground.\n\r",ch);
    act("$n sinks into the ground.", ch, NULL, NULL, TO_ROOM);
    SET_BIT(ch->vampaff, VAM_EARTHMELDED);
    REMOVE_BIT(ch->affected_by, AFF_FLAMING);
    return;
}
