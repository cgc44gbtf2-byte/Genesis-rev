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

/* Vampire Beast callbacks retain their shared command-helper calls. */
#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include "merc.h"

void do_humanity( CHAR_DATA *ch, char *argument )
{
    char       arg [MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if (!IS_NPC(ch) && IS_VAMPIRE(ch) && ch->beast > 0
	&& ch->beast < 100 && number_range(1, 500) <= ch->beast)
    {
	if (ch->beast == 1)
	{
	    send_to_char("You have attained Golconda!\n\r", ch);
	}
	else
	    send_to_char("You feel slightly more in control of your beast.\n\r", ch);
	ch->beast -= 1;
    }
    return;
}

void do_beastlike( CHAR_DATA *ch, char *argument )
{
    char       arg [MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if (IS_VAMPIRE(ch) && ch->beast < 100 && number_range(1, 200) <= ch->beast)
    {
	if (ch->beast < 99)
	    send_to_char("You feel your beast take more control over your actions.\n\r", ch);
	else
	    send_to_char("Your beast has fully taken over control of your actions!\n\r", ch);
	ch->beast += 1;
	if (get_disc(ch,DISC_PROTEAN) > 0 && 
	    !IS_VAMPAFF(ch, VAM_NIGHTSIGHT) ) do_nightsight(ch,"");
	if (!IS_VAMPAFF(ch, VAM_FANGS)) do_fangs(ch,"");
	if (get_disc(ch,DISC_PROTEAN) > 1 && 
	    !IS_VAMPAFF(ch, VAM_CLAWS) ) do_claws(ch,"");
    }
    return;
}
