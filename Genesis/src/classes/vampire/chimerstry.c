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

/* Existing illusion creation/control; mixed-class illusion helpers stay shared. */
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"

void do_mirrorimage( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    OBJ_DATA *obj2;

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch))
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if (get_disc(ch,DISC_CHIMERSTRY) < 1)
    {
	send_to_char("You require level 1 Chimerstry to create a Mirror Image.\n\r",ch);
	return;
    }

    if (ch->pcdata->followers > 4)
    {
	send_to_char("Nothing happens.\n\r",ch);
	return;
    }

    ch->pcdata->followers++;
    victim=create_mobile( get_mob_index( MOB_VNUM_CLONE ) );
    char_to_room(victim,get_room_index(ROOM_VNUM_DISCONNECTION));
    free_string(victim->lord);
    victim->lord = str_dup(ch->name);
    free_string(victim->name);
    victim->name = str_dup(ch->name);
    free_string(victim->short_descr);
    victim->short_descr = str_dup(ch->name);
    free_string(victim->long_descr);
    victim->long_descr = str_dup("");

    if (strlen(ch->description) > 1)
    {
	free_string(victim->description);
	victim->description = str_dup(ch->description);
    }
    SET_BIT(victim->extra, EXTRA_ILLUSION);

    {
	OBJ_DATA *obj_next;
	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if ( obj->wear_loc != WEAR_NONE )
	    {
		if (( obj2 = create_object( obj->pIndexData, 0 )) != NULL )
		{
		    SET_BIT(obj2->extra_flags, ITEM_VANISH);
    obj2->cost = 0;
    obj2->points = 0;
		    obj_to_char(obj2, victim);
		    wear_obj( victim, obj2, obj->wear_loc );
		}
	    }
	}
    }
    victim->level = 50;
    victim->hit = ch->hit;
    victim->max_hit = ch->max_hit;
    victim->mana = ch->mana;
    victim->max_mana = ch->max_mana;
    victim->hitroll = char_hitroll(ch);
    victim->damroll = char_damroll(ch);
    victim->armor = char_ac(ch);
    char_from_room(victim);
    char_to_room(victim,ch->in_room);

    send_to_char("A mirror image of you shimmers into existance.\n\r",ch);
    act("A mirror image of $n appears in the room.",ch,NULL,NULL,TO_ROOM);
    return;
}

void do_control( CHAR_DATA *ch, char *argument )
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

    if (get_disc(ch,DISC_CHIMERSTRY) < 5)
    {
	send_to_char("You require level 5 Chimerstry to Control an Illusion.\n\r",ch);
	return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char( "What illusion do you want to control?\n\r", ch );
	return;
    }

    if ( ( familiar = ch->pcdata->familiar ) != NULL ) 
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

    if ( ( familiar = victim->wizard ) != NULL ) 
    {
	send_to_char( "Nothing happens.\n\r", ch );
	return;
    }

    if (!IS_EXTRA(victim, EXTRA_ILLUSION))
    {
	send_to_char( "Nothing happens.\n\r", ch );
	return;
    }

    act("You use your power over illusions to control $N.",ch,NULL,victim,TO_CHAR);
    ch->pcdata->familiar = victim;
    victim->wizard = ch;
    return;
}
