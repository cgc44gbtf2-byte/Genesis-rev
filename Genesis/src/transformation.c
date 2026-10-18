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

#include <stdio.h>
#include <time.h>
#include "merc.h"

/* Shared cleanup only: callers finish flags, appearance and class bonuses
 * before saving. Never serialize a half-completed transformation here. */
void reset_form_stats(CHAR_DATA *ch)
{
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    if (ch == NULL || IS_NPC(ch) || ch->pcdata == NULL) return;

    for (obj = ch->carrying; obj != NULL; obj = obj_next)
    {
        obj_next = obj->next_content;
        if (obj->wear_loc != WEAR_NONE)
        {
            obj_from_char(obj);
            obj_to_char(obj, ch);
        }
    }
    while (ch->affected)
        affect_remove(ch, ch->affected);
    ch->armor = 100;
    ch->hitroll = 0;
    ch->damroll = 0;
    ch->saving_throw = 0;
    ch->pcdata->atm = 0;
    ch->pcdata->followers = 0;
}
