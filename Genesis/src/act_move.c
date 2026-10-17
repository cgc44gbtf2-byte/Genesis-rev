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



char *	const	dir_name	[]		=
{
    "north", "east", "south", "west", "up", "down"
};

const	long	rev_dir		[]		=
{
    2, 3, 0, 1, 5, 4
};


/*
 * Local functions.
 */
long	find_door	args( ( CHAR_DATA *ch, char *arg ) );
bool	has_key		args( ( CHAR_DATA *ch, long key ) );
void	show_page	args( ( CHAR_DATA *ch, OBJ_DATA *book, long pnum, bool pagefalse ) );


void move_char( CHAR_DATA *ch, long door )
{
    CHAR_DATA *fch;
    CHAR_DATA *fch_next;
    ROOM_INDEX_DATA *in_room;
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;
    DESCRIPTOR_DATA *d;
    char buf  [MAX_STRING_LENGTH];
    char poly [MAX_STRING_LENGTH];
    char dispname [MAX_INPUT_LENGTH];
    char leave [20];
    long revdoor;

    if (IS_NPC(ch)) strcpy(dispname,ch->short_descr);
    else if (ch->morph[0] == '\0') strcpy(dispname,ch->name);
    else strcpy(dispname,ch->morph);
    if ( door < 0 || door > 5 )
    {
	bug( "Do_move: bad door %li.", door );
	return;
    }

    if (ch->embraced != ARE_NONE)
    {
	send_to_char( "You are unable to break your embrace.\n\r", ch );
	return;
    }

    in_room = ch->in_room;
    if ( ( pexit   = in_room->exit[door] ) == NULL
    ||   ( to_room = pexit->to_room      ) == NULL )
    {
	send_to_char( "Alas, you cannot go that way.\n\r", ch );
	return;
    }

    if ( IS_SET(pexit->exit_info, EX_WALL_WATER) )
    {
	send_to_char("You are unable to pass the wall of water.\n\r",ch);
	return;
    }

    if ( IS_SET(pexit->exit_info, EX_WALL_GRANITE) )
    {
	send_to_char("You are unable to pass the wall of granite.\n\r",ch);
	return;
    }

    if ( IS_SET(pexit->exit_info, EX_WALL_WIND))
    {
	send_to_char("You struggle through the wall of buffering winds and fall to your knees.\n\r",ch);
	WAIT_STATE(ch, 9);
        ch->position = POS_SITTING;
    }

    if ( IS_SET(pexit->exit_info, EX_WALL_FLAME) )
    {
	long sn, spelltype;
	long ch_hp = ch->hit;
	SET_BIT(ch->affected_by, AFF_FLAMING);
	send_to_char("You step through the wall of searing flames!\n\r",ch);
	act("$n gingerly steps through the wall of searing flames!",ch,NULL,NULL,TO_ROOM);
	if ( ( sn = skill_lookup("fire breath") ) >= 0 )
	{
	    spelltype = skill_table[sn].target;
	    (*skill_table[sn].spell_fun) ( sn, 25, ch, ch );
	}
	if (ch == NULL || ch->position == POS_DEAD) return;
	if (ch->hit < ch_hp)
	{
	    act("$n's flesh smolders as $e passes through the wall of searing flames!",ch,NULL,NULL,TO_ROOM);
	    send_to_char("Your flesh smolders as you pass through the wall of searing flames!\n\r",ch);
	    if (number_percent() <= 5) SET_BIT(ch->affected_by, AFF_FLAMING);
	}
    }

    if ( IS_SET(pexit->exit_info, EX_CLOSED)
    &&   pexit->keyword != NULL
    &&   !IS_AFFECTED(ch, AFF_PASS_DOOR)
    &&   !IS_AFFECTED(ch, AFF_ETHEREAL)
    &&   !IS_AFFECTED(ch, AFF_SHADOWPLANE) )
    {
	    act( "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
	    return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM)
    &&   ch->master != NULL
    &&   in_room == ch->master->in_room )
    {
	send_to_char( "What?  And leave your beloved master?\n\r", ch );
	return;
    }

    if ( room_is_private( to_room ) )
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }

    if ( !IS_NPC(ch) )
    {
	if (in_room->sector_type == SECT_AIR || to_room->sector_type == SECT_AIR )
	{
	    if (!IS_AFFECTED(ch, AFF_FLYING) && !IS_NPC(ch) && !IS_VAMPAFF(ch, VAM_FLYING) && !IS_DRAGAFF(ch, DRAG_WINGSOUT))
	    {
		send_to_char( "You can't fly.\n\r", ch );
		return;
	    }
	}

	if ( in_room->sector_type == SECT_WATER_NOSWIM
	||   to_room->sector_type == SECT_WATER_NOSWIM )
	{
	    OBJ_DATA *obj;
	    bool found;

	    /*
	     * Look for a boat.
	     */
	    found = FALSE;
	    if ( !IS_NPC(ch))
	    {
                if (IS_BUI(ch))
                    found = TRUE;
		else if (IS_WEREWOLF(ch) && get_tribe(ch, TRIBE_UKTENA) > 1)
		    found = TRUE;
		else if ( IS_VAMPAFF(ch, VAM_FLYING) )
		    found = TRUE;
                else if ( IS_POLYAFF(ch, POLY_DRAGONFORM) )
                    found = TRUE;
		else if ( IS_POLYAFF(ch, POLY_SERPENT) )
		    found = TRUE;
	    }
	    if ( IS_AFFECTED(ch, AFF_FLYING) )
		found = TRUE;
	    else if ( IS_AFFECTED(ch, AFF_SHADOWPLANE) )
		found = TRUE;
            else if ( IS_AFFECTED(ch, AFF_ETHEREAL) )
                found = TRUE;
	    else if ( IS_POLYAFF(ch, POLY_FISH) )
		found = TRUE;
	    if ( !found )
	    {
	    	for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
	    	{
		    if ( obj->item_type == ITEM_BOAT )
		    {
		    	found = TRUE;
		    	break;
		    }
	    	}
	    	if ( !found )
	    	{
		    send_to_char( "You need a boat to go there.\n\r", ch );
		    return;
	    	}
	    }
	}
        if ( in_room->sector_type == SECT_UNDERWATER
        ||   to_room->sector_type == SECT_UNDERWATER )
        {
            bool found;

            found = FALSE;
            if ( !IS_NPC(ch))
            {
		if (IS_BUI(ch))
		    found = TRUE;
                else if (IS_WEREWOLF(ch) && get_tribe(ch, TRIBE_UKTENA) > 1)
                    found = TRUE;
                else if ( IS_VAMPAFF(ch, VAM_FLYING) )
                    found = TRUE;
                else if ( IS_POLYAFF(ch, POLY_DRAGONFORM) )
                    found = TRUE;
                else if ( IS_POLYAFF(ch, POLY_SERPENT) )
                    found = TRUE;
            }
            if ( IS_AFFECTED(ch, AFF_FLYING) )
                found = TRUE;
            else if ( IS_AFFECTED(ch, AFF_SHADOWPLANE) )
                found = TRUE;
            else if ( IS_AFFECTED(ch, AFF_ETHEREAL) )
                found = TRUE;
            else if ( IS_POLYAFF(ch, POLY_FISH) )
                found = TRUE;
            if ( !found )
            {
               send_to_char( "You cannot go underwater.\n\r", ch );
                    return;
            }
        }

	else if ( !IS_AFFECTED(ch, AFF_FLYING) && IS_POLYAFF(ch, POLY_FISH) )
	{
	    bool from_ok = FALSE;
	    bool to_ok = FALSE;
	    if ( in_room->sector_type == SECT_WATER_NOSWIM ) from_ok = TRUE;
	    if ( in_room->sector_type == SECT_WATER_SWIM   ) from_ok = TRUE;
            if ( in_room->sector_type == SECT_UNDERWATER   ) from_ok = TRUE;
            if ( in_room->sector_type == SECT_UNDERWATER   ) to_ok =   TRUE;
	    if ( to_room->sector_type == SECT_WATER_NOSWIM ) to_ok   = TRUE;
	    if ( to_room->sector_type == SECT_WATER_SWIM   ) to_ok   = TRUE;
	    if ( !from_ok || !to_ok )
	    {
		send_to_char("You cannot cross land.\n\r",ch);
		return;
	    }
	}
    }

    if (IS_NPC(ch) && ch->pIndexData->vnum == MOB_VNUM_EYE)
	strcpy(leave, "rolls");
    else if (!IS_NPC(ch) && IS_EXTRA(ch,EXTRA_OSWITCH) && ch->pcdata->obj_vnum == OBJ_VNUM_SPILT_BLOOD)
	strcpy(leave, "pours");
    else if (IS_MORE(ch, MORE_HEAD) || IS_EXTRA(ch, EXTRA_OSWITCH))
	strcpy(leave, "rolls");
    else if (IS_AFFECTED(ch, AFF_ETHEREAL))
	strcpy(leave, "floats");
    else if (in_room->sector_type == SECT_WATER_NOSWIM || to_room->sector_type == SECT_WATER_NOSWIM)
	strcpy(leave, "swims");
    else if (IS_SET(ch->polyaff, POLY_CHIMERA))
	strcpy(leave,  "storms");
    else if (IS_SET(ch->polyaff, POLY_SERPENT))
	strcpy(leave, "slithers");
    else if (IS_SET(ch->polyaff, POLY_WOLF))
	strcpy(leave, "stalks");
    else if (IS_SET(ch->polyaff, POLY_FROG))
	strcpy(leave, "hops");
    else if (IS_SET(ch->polyaff, POLY_FISH))
	strcpy(leave, "swims");
    else if (in_room->sector_type == SECT_WATER_SWIM || to_room->sector_type == SECT_WATER_SWIM)
	strcpy(leave, "wades");
    else if (IS_SET(ch->polyaff, POLY_ZULO))
	strcpy(leave, "strides");
    else if (IS_SET(ch->polyaff, POLY_CHIRO))
	strcpy(leave, "flies");
    else if (ch->hit < (ch->max_hit/4))
	strcpy(leave, "crawls");
    else if (ch->hit < (ch->max_hit/3))
	strcpy(leave, "limps");
    else if (ch->hit < (ch->max_hit/2))
	strcpy(leave, "staggers");
    else
	strcpy(leave, "walks");

    if (IS_WEREWOLF(ch) && IS_ITEMAFF(ch,ITEMA_BLISSFUL)) do_blissful(ch, "");
    if (ch->stance[0] != 0) do_stance(ch,"");
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	CHAR_DATA *victim;

	if ( ( victim = d->character ) == NULL )
	    continue;

        if ( ch->in_room == NULL || victim->in_room == NULL ) continue;

        if ( ch == victim || ch->in_room != victim->in_room) continue;

	if ( d->connected != CON_PLAYING || !can_see( ch, victim ) )
	    continue;
	if (IS_AFFECTED(ch, AFF_SNEAK)) continue;
	else if (!IS_NPC(ch) && IS_NINJA(ch) && get_ninja(ch, NINJA_SORA) >= 6) continue;
	else if (!IS_NPC(ch) && IS_WEREWOLF(ch) && get_auspice(ch,AUSPICE_RAGABASH) >= 3) continue;
	else if (can_see(victim,ch))
	{
	    if (IS_AFFECTED(ch,AFF_FLYING) || (!IS_NPC(ch) && IS_VAMPAFF(ch,VAM_FLYING)))
		sprintf(poly,"%s flies $T.",dispname);
	    else
		sprintf(poly,"%s %s $T.",dispname,leave);
	    act( poly, victim, NULL, dir_name[door], TO_CHAR );
	}
    }
    char_from_room( ch );
    char_to_room( ch, to_room );
    if      ( door == 0 ) {revdoor = 2;sprintf(buf,"the south");}
    else if ( door == 1 ) {revdoor = 3;sprintf(buf,"the west");}
    else if ( door == 2 ) {revdoor = 0;sprintf(buf,"the north");}
    else if ( door == 3 ) {revdoor = 1;sprintf(buf,"the east");}
    else if ( door == 4 ) {revdoor = 5;sprintf(buf,"below");}
    else                  {revdoor = 4;sprintf(buf,"above");}

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	CHAR_DATA *victim;

	if ( ( victim = d->character ) == NULL )
	    continue;

        if ( ch->in_room == NULL || victim->in_room == NULL ) continue;

        if ( ch == victim || ch->in_room != victim->in_room) continue;

	if ( d->connected != CON_PLAYING || !can_see( ch, victim ) )
	    continue;

        if (IS_AFFECTED(ch, AFF_SNEAK)) continue;
        else if (!IS_NPC(ch) && IS_NINJA(ch) && get_ninja(ch, NINJA_SORA) >= 6) continue;
        else if (!IS_NPC(ch) && IS_WEREWOLF(ch) && get_auspice(ch,AUSPICE_RAGABASH) >= 3) continue;
	else if (can_see(victim,ch))
	{
	    if (IS_AFFECTED(ch,AFF_FLYING) || (!IS_NPC(ch) && IS_VAMPAFF(ch,VAM_FLYING)))
		sprintf(poly,"%s flies in from %s.",dispname,buf);
	    else
		sprintf(poly,"%s %s in from %s.",dispname,leave,buf);
	    act( poly, ch, NULL, victim, TO_VICT );
	}
    }

    do_look( ch, "auto" );

    trigger_trap(ch,ch->in_room->trap);

    for ( fch = in_room->people; fch != NULL; fch = fch_next )
    {
	fch_next = fch->next_in_room;

	if ( fch->master == ch && fch->position == POS_STANDING 
		&& fch->in_room != ch->in_room
		&& !IS_VAMPAFF(fch, VAM_EARTHMELDED)
		&& !IS_EXTRA(fch, EXTRA_OSWITCH)
		&& !IS_MORE(fch, MORE_HEAD))
	{
	    act( "You follow $N.", fch, NULL, ch, TO_CHAR );
	    move_char( fch, door );
	}
    }
    room_text(ch,">ENTER<");
    return;
}

void do_north( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *in_room;

    if (IS_AFFECTED(ch, AFF_WEBBED))
    {
	send_to_char("You are unable to move with all this sticky webbing on.\n\r",ch);
	return;
    }
    else if (IS_AFFECTED(ch, AFF_SHADOW))
    {
	send_to_char("You are unable to move with these tendrils of darkness entrapping you.\n\r",ch);
	return;
    }
    in_room = ch->in_room;
    move_char( ch, DIR_NORTH );
    return;
}



void do_east( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *in_room;

    if (IS_AFFECTED(ch, AFF_WEBBED))
    {
	send_to_char("You are unable to move with all this sticky webbing on.\n\r",ch);
	return;
    }
    else if (IS_AFFECTED(ch, AFF_SHADOW))
    {
	send_to_char("You are unable to move with these tendrils of darkness entrapping you.\n\r",ch);
	return;
    }
    in_room = ch->in_room;
    move_char( ch, DIR_EAST );
    return;
}



void do_south( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *in_room;

    if (IS_AFFECTED(ch, AFF_WEBBED))
    {
	send_to_char("You are unable to move with all this sticky webbing on.\n\r",ch);
	return;
    }
    else if (IS_AFFECTED(ch, AFF_SHADOW))
    {
	send_to_char("You are unable to move with these tendrils of darkness entrapping you.\n\r",ch);
	return;
    }
    in_room = ch->in_room;
    move_char( ch, DIR_SOUTH );
    return;
}



void do_west( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *in_room;

    if (IS_AFFECTED(ch, AFF_WEBBED))
    {
	send_to_char("You are unable to move with all this sticky webbing on.\n\r",ch);
	return;
    }
    else if (IS_AFFECTED(ch, AFF_SHADOW))
    {
	send_to_char("You are unable to move with these tendrils of darkness entrapping you.\n\r",ch);
	return;
    }
    in_room = ch->in_room;
    move_char( ch, DIR_WEST );
    return;
}



void do_up( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *in_room;

    if (IS_AFFECTED(ch, AFF_WEBBED))
    {
	send_to_char("You are unable to move with all this sticky webbing on.\n\r",ch);
	return;
    }
    else if (IS_AFFECTED(ch, AFF_SHADOW))
    {
	send_to_char("You are unable to move with these tendrils of darkness entrapping you.\n\r",ch);
	return;
    }
    in_room = ch->in_room;
    move_char( ch, DIR_UP );
    return;
}



void do_down( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *in_room;

    if (IS_AFFECTED(ch, AFF_WEBBED))
    {
	send_to_char("You are unable to move with all this sticky webbing on.\n\r",ch);
	return;
    }
    else if (IS_AFFECTED(ch, AFF_SHADOW))
    {
	send_to_char("You are unable to move with these tendrils of darkness entrapping you.\n\r",ch);
	return;
    }
/*    else if (IS_AFFECTED(ch, AFF_JAIL))
    {
	send_to_char("You are unable to move with these bands of water entrapping you.\n\r",ch);
	return;
    }*/
    in_room = ch->in_room;
    move_char( ch, DIR_DOWN );
    return;
}

void do_travel( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument(argument,arg);

    if (IS_NPC(ch))

    if (arg[0] == '\0')
    {
	stc("Who do you wish to travel to?\n\r",ch);
	return;
    }
    if ((victim = get_char_world(ch,arg)) == NULL || !IS_NPC(victim) || victim->level > 250 || victim->in_room == ch->in_room
     || IS_SET(victim->in_room->room_flags, ROOM_SAFE) || IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
     || IS_SET(victim->in_room->room_flags, ROOM_SOLITARY) || IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL))
    {
	stc("You are unable to travel to your target.\n\r",ch);
	return;
    }
    if (ch->fight_timer > 0)
    {
     stc("Not with a fight timer!\n\r", ch);
     return;
    }

    stc("You travel.\n\r",ch);
    act("$n travels to $N.",ch,NULL,victim,TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, victim->in_room);
    do_look(ch,"");
    act("$n travels here.",ch,NULL,NULL,TO_ROOM);
    WAIT_STATE(ch, 8);
    return;
}

long find_door( CHAR_DATA *ch, char *arg )
{
    EXIT_DATA *pexit;
    long door;

	 if ( !str_cmp( arg, "n" ) || !str_cmp( arg, "north" ) ) door = 0;
    else if ( !str_cmp( arg, "e" ) || !str_cmp( arg, "east"  ) ) door = 1;
    else if ( !str_cmp( arg, "s" ) || !str_cmp( arg, "south" ) ) door = 2;
    else if ( !str_cmp( arg, "w" ) || !str_cmp( arg, "west"  ) ) door = 3;
    else if ( !str_cmp( arg, "u" ) || !str_cmp( arg, "up"    ) ) door = 4;
    else if ( !str_cmp( arg, "d" ) || !str_cmp( arg, "down"  ) ) door = 5;
    else
    {
	for ( door = 0; door <= 5; door++ )
	{
	    if ( ( pexit = ch->in_room->exit[door] ) != NULL
	    &&   IS_SET(pexit->exit_info, EX_ISDOOR)
	    &&   pexit->keyword != NULL
	    &&   is_name( arg, pexit->keyword ) )
		return door;
	}
	act( "I see no $T here.", ch, NULL, arg, TO_CHAR );
	return -1;
    }

    if ( ( pexit = ch->in_room->exit[door] ) == NULL )
    {
	act( "I see no door $T here.", ch, NULL, arg, TO_CHAR );
	return -1;
    }

    if ( !IS_SET(pexit->exit_info, EX_ISDOOR) )
    {
	send_to_char( "You can't do that.\n\r", ch );
	return -1;
    }

    return door;
}

/* Designed for the portal spell, but can also have other uses...KaVir
 * V0 = Where the portal will take you.
 * V1 = Number of uses (0 is infinate).
 * V2 = if 2, cannot be entered.
 * V3 = The room the portal is currently in.
 */
void do_enter( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *pRoomIndex;
    ROOM_INDEX_DATA *location;
    char arg[MAX_INPUT_LENGTH];
    char poly [MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *portal;
    OBJ_DATA *portal_next;
    bool found;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Enter what?\n\r", ch );
	return;
    }

    obj = get_obj_list( ch, arg, ch->in_room->contents );
    if ( obj == NULL )
    {
	act( "I see no $T here.", ch, NULL, arg, TO_CHAR );
	return;
    }
    if ( obj->item_type != ITEM_PORTAL && obj->item_type != ITEM_WGATE )
    {
	act( "You cannot enter that.", ch, NULL, arg, TO_CHAR );
	return;
    }

    if (IS_AFFECTED(ch, AFF_SHADOWPLANE) &&
	!IS_SET(obj->extra_flags, ITEM_SHADOWPLANE) )
    {
	send_to_char( "You are too insubstantual.\n\r", ch );
	return;
    }
    else if (!IS_AFFECTED(ch, AFF_SHADOWPLANE) &&
	IS_SET(obj->extra_flags, ITEM_SHADOWPLANE) )
    {
	send_to_char( "It is too insubstantual.\n\r", ch );
	return;
    }
     if ( obj->item_type != ITEM_WGATE )
    {
     if ( obj->value[2] == 2 || obj->value[2] == 3 )
     {
        act( "It seems to be closed.", ch, NULL, arg, TO_CHAR );
        return;
     }
    }

/*
    if ( obj->value[2] == 2 || obj->value[2] == 3 )
    {
	act( "It seems to be closed.", ch, NULL, arg, TO_CHAR );
	return;
    }
*/
    pRoomIndex = get_room_index(obj->value[0]);
    location = ch->in_room;

    if ( pRoomIndex == NULL )
    {
	act( "You are unable to enter.", ch, NULL, arg, TO_CHAR );
	return;
    }

    act( "You step into $p.",  ch, obj, NULL, TO_CHAR );
    if ( !IS_NPC(ch) && IS_AFFECTED(ch,AFF_POLYMORPH) )
	sprintf(poly,"%s steps into $p.",ch->morph);
    else
	sprintf(poly,"$n steps into $p.");
    act( poly,  ch, obj, NULL, TO_ROOM );
    char_from_room(ch);
    char_to_room(ch,pRoomIndex);
    if ( !IS_NPC(ch) && IS_AFFECTED(ch,AFF_POLYMORPH) )
	sprintf(poly,"%s steps out of $p.",ch->morph);
    else
	sprintf(poly,"$n steps out of $p.");
    act( poly,  ch, obj, NULL, TO_ROOM );
    char_from_room(ch);
    char_to_room(ch,location);
/* Leave this out for now, as it doesn't seem to work properly. KaVir
    if ((obj->value[2] == 1) && (CAN_WEAR(obj,ITEM_TAKE)))
    {
	obj->value[2] = 2;
	REMOVE_BIT(obj->wear_flags, ITEM_TAKE);
    }
*/

    if ( obj->item_type != ITEM_WGATE )
    {
    if (obj->value[1] != 0)
    {
	obj->value[1] = obj->value[1] - 1;
	if (obj->value[1] < 1)
	{
	    act( "$p vanishes.",  ch, obj, NULL, TO_CHAR );
	    act( "$p vanishes.",  ch, obj, NULL, TO_ROOM );
/*
	    extract_obj(obj);
*/
	    obj->timer = 1;
	}
    }
    }
    char_from_room(ch);
    char_to_room(ch,pRoomIndex);
    found = FALSE;
    for ( portal = ch->in_room->contents; portal != NULL; portal = portal_next )
    {
	portal_next = portal->next_content;
	if ( ( obj->value[0] == portal->value[3]  )
	    && (obj->value[3] == portal->value[0]) )
	{
	    found = TRUE;
/* Leave this out for now, as it doesn't seem to work properly. KaVir
	    if ((portal->value[2] == 2) && (!CAN_WEAR(obj,ITEM_TAKE)))
	    {
		portal->value[2] = 1;
		SET_BIT(obj->wear_flags, ITEM_TAKE);
	    }
*/
	    if (IS_AFFECTED(ch, AFF_SHADOWPLANE) &&
		!IS_SET(portal->extra_flags, ITEM_SHADOWPLANE) )
	    {
		REMOVE_BIT(ch->affected_by, AFF_SHADOWPLANE);
		break;
	    }
	    else if (!IS_AFFECTED(ch, AFF_SHADOWPLANE) &&
		IS_SET(portal->extra_flags, ITEM_SHADOWPLANE) )
	    {
		SET_BIT(ch->affected_by, AFF_SHADOWPLANE);
		break;
	    }
    	    if (portal->value[1] != 0 && portal->item_type != ITEM_WGATE)
    	    {
		portal->value[1] = portal->value[1] - 1;
		if (portal->value[1] < 1)
		{
		    act( "$p vanishes.",  ch, portal, NULL, TO_CHAR );
		    act( "$p vanishes.",  ch, portal, NULL, TO_ROOM );
		    portal->timer = 1;
		}
	    }
    	}
    }
    do_look(ch,"auto");
    return;
}


void do_open( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    long door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Open what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'open object' */
	if ( obj->item_type != ITEM_CONTAINER && obj->item_type != ITEM_BOOK)
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's already open.\n\r",      ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) && obj->item_type != ITEM_BOOK)
	    { send_to_char( "You can't do that.\n\r",      ch ); return; }
	if ( IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's locked.\n\r",            ch ); return; }

	REMOVE_BIT(obj->value[1], CONT_CLOSED);
	send_to_char( "Ok.\n\r", ch );
	act( "$n opens $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'open door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's already open.\n\r",      ch ); return; }
	if (  IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's locked.\n\r",            ch ); return; }

	REMOVE_BIT(pexit->exit_info, EX_CLOSED);
	act( "$n opens the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	send_to_char( "Ok.\n\r", ch );

	/* open the other side */
	if ( ( to_room   = pexit->to_room               ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	&&   pexit_rev->to_room == ch->in_room )
	{
	    CHAR_DATA *rch;

	    REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
	    for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room )
		act( "The $d opens.", rch, NULL, pexit_rev->keyword, TO_CHAR );
	}
    }

    return;
}



void do_close( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    long door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Close what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'close object' */
	if ( obj->item_type != ITEM_CONTAINER && obj->item_type != ITEM_BOOK)
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's already closed.\n\r",    ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) && obj->item_type != ITEM_BOOK )
	    { send_to_char( "You can't do that.\n\r",      ch ); return; }

	SET_BIT(obj->value[1], CONT_CLOSED);
	if (obj->item_type == ITEM_BOOK) obj->value[2] = 0;
	send_to_char( "Ok.\n\r", ch );
	act( "$n closes $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'close door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit	= ch->in_room->exit[door];
	if ( IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's already closed.\n\r",    ch ); return; }

	SET_BIT(pexit->exit_info, EX_CLOSED);
	act( "$n closes the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	send_to_char( "Ok.\n\r", ch );

	/* close the other side */
	if ( ( to_room   = pexit->to_room               ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != 0
	&&   pexit_rev->to_room == ch->in_room )
	{
	    CHAR_DATA *rch;

	    SET_BIT( pexit_rev->exit_info, EX_CLOSED );
	    for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room )
		act( "The $d closes.", rch, NULL, pexit_rev->keyword, TO_CHAR );
	}
    }

    return;
}



void do_turn( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    long value = 0;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Syntax: Turn <book> <forward/back>.\n\r", ch );
	return;
    }

    if (is_number(arg2)) value = atoi(arg2);

    if ( ( obj = get_obj_here( ch, arg1 ) ) == NULL )
    {
	send_to_char("You don't have that book.\n\r",ch);
	return;
    }

    if ( obj->item_type != ITEM_BOOK)
    { send_to_char( "That's not a book.\n\r", ch ); return; }
    if ( IS_SET(obj->value[1], CONT_CLOSED) )
    { send_to_char( "First you should open it.\n\r",    ch ); return; }

    if (!str_cmp(arg2,"f") || !str_cmp(arg2,"forward"))
    {
	if (obj->value[2] >= obj->value[3])
	{
	    send_to_char("But you are already at the end of the book.\n\r",ch);
	    return;
	}
	obj->value[2] += 1;
	act( "You flip forward a page in $p.", ch, obj, NULL, TO_CHAR );
	act( "$n flips forward a page in $p.", ch, obj, NULL, TO_ROOM );
    }
    else if (!str_cmp(arg2,"b") || !str_cmp(arg2,"backward"))
    {
	if (obj->value[2] <= 0)
	{
	    send_to_char("But you are already at the beginning of the book.\n\r",ch);
	    return;
	}
	obj->value[2] -= 1;
	act( "You flip backward a page in $p.", ch, obj, NULL, TO_CHAR );
	act( "$n flips backward a page in $p.", ch, obj, NULL, TO_ROOM );
    }
    else if (is_number(arg2) && value >= 0 && value <= obj->value[3])
    {
	if (value == obj->value[2])
	{
	    act("$p is already open at that page.",ch,obj,NULL,TO_CHAR);
	    return;
	}
	else if (value < obj->value[2])
	{
	    act( "You flip backwards through $p.", ch, obj, NULL, TO_CHAR );
	    act( "$n flips backwards through $p.", ch, obj, NULL, TO_ROOM );
	}
	else
	{
	    act( "You flip forwards through $p.", ch, obj, NULL, TO_CHAR );
	    act( "$n flips forwards through $p.", ch, obj, NULL, TO_ROOM );
	}
	obj->value[2] = value;
    }
    else
	send_to_char("Do you wish to turn forward or backward a page?\n\r",ch);
    return;
}

void do_read( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "What do you wish to read?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) == NULL )
    {
	send_to_char("You don't have that book.\n\r",ch);
	return;
    }

    if ( obj->item_type == ITEM_PAGE)
    {
	if (!str_cmp(obj->victpoweruse,"(null)"))
	    sprintf(buf,"Untitled page.\n\r");
	else
	    sprintf(buf,"%s.\n\r",obj->victpoweruse);
	buf[0] = UPPER(buf[0]);
	send_to_char(buf,ch);
	send_to_char("--------------------------------------------------------------------------------\n\r",ch);
	send_to_char(obj->chpoweruse,ch);
	send_to_char("\n\r--------------------------------------------------------------------------------\n\r",ch);
	return;
    }

    if ( obj->item_type != ITEM_BOOK)
    { send_to_char( "That's not a book.\n\r", ch ); return; }
    if ( IS_SET(obj->value[1], CONT_CLOSED) )
    {
	if (!str_cmp(obj->victpoweruse,"(null)"))
	    sprintf(buf,"The book is untitled.\n\r");
	else
	    sprintf(buf,"The book is titled '%s'.\n\r",obj->victpoweruse);
	buf[0] = UPPER(buf[0]);
	send_to_char(buf,ch);
	return;
    }

    if (obj->value[2] == 0)
    {
	long page;

	send_to_char("Index page.\n\r",ch);
	if (obj->value[3] <= 0)
	{
	    send_to_char("<No pages>\n\r",ch);
	    return;
	}
	for ( page = 1; page <= obj->value[3]; page ++ )
	{
	    sprintf(buf,"Page %li:",page);
	    send_to_char(buf,ch);
	    show_page(ch,obj,page,TRUE);
	}
    }
    else
    {
	sprintf(buf,"Page %li:",obj->value[2]);
	send_to_char(buf,ch);
	show_page(ch,obj,obj->value[2],FALSE);
    }
    return;
}


bool has_key( CHAR_DATA *ch, long key )
{
    OBJ_DATA *obj;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->pIndexData->vnum == key )
	    return TRUE;
    }

    return FALSE;
}



void do_lock( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    long door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Lock what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'lock object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be locked.\n\r",     ch ); return; }
	if ( !has_key( ch, obj->value[2] ) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already locked.\n\r",    ch ); return; }

	SET_BIT(obj->value[1], CONT_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n locks $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'lock door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit	= ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 )
	    { send_to_char( "It can't be locked.\n\r",     ch ); return; }
	if ( !has_key( ch, pexit->key) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already locked.\n\r",    ch ); return; }

	SET_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n locks the $d.", ch, NULL, pexit->keyword, TO_ROOM );

	/* lock the other side */
	if ( ( to_room   = pexit->to_room               ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != 0
	&&   pexit_rev->to_room == ch->in_room )
	{
	    SET_BIT( pexit_rev->exit_info, EX_LOCKED );
	}
    }

    return;
}



void do_unlock( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    long door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Unlock what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'unlock object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( !has_key( ch, obj->value[2] ) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }

	REMOVE_BIT(obj->value[1], CONT_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n unlocks $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'unlock door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( !has_key( ch, pexit->key) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }

	REMOVE_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n unlocks the $d.", ch, NULL, pexit->keyword, TO_ROOM );

	/* unlock the other side */
	if ( ( to_room   = pexit->to_room               ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	&&   pexit_rev->to_room == ch->in_room )
	{
	    REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
	}
    }

    return;
}



void do_pick( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *gch;
    OBJ_DATA *obj;
    long door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Pick what?\n\r", ch );
	return;
    }

    /* look for guards */
    for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
    {
	if ( IS_NPC(gch) && IS_AWAKE(gch) )
	{
	    if (IS_EXTRA(ch, EXTRA_AWE))
	    {
		act( "$N steps aside to let you reach the lock.",ch, NULL, gch, TO_CHAR );
		act( "$N steps aside to let $n reach the lock.",ch, NULL, gch, TO_NOTVICT );
		act( "You step aside to let $n reach the lock.  Isn't $e awesome?",ch, NULL, gch, TO_VICT );
	    }
	    else
	    {
		act( "$N is standing too close to the lock.",ch, NULL, gch, TO_CHAR );
		return;
	    }
	}
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'pick object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }
	if ( IS_SET(obj->value[1], CONT_PICKPROOF) )
	    { send_to_char( "You failed.\n\r",             ch ); return; }

	REMOVE_BIT(obj->value[1], CONT_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n picks $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'pick door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 )
	    { send_to_char( "It can't be picked.\n\r",     ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_PICKPROOF) )
	    { send_to_char( "You failed.\n\r",             ch ); return; }

	REMOVE_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n picks the $d.", ch, NULL, pexit->keyword, TO_ROOM );

	/* pick the other side */
	if ( ( to_room   = pexit->to_room               ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	&&   pexit_rev->to_room == ch->in_room )
	{
	    REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
	}
    }

    return;
}




void do_stand( CHAR_DATA *ch, char *argument )
{
    switch ( ch->position )
    {
    case POS_SLEEPING:
	if ( IS_AFFECTED(ch, AFF_SLEEP) )
	    { send_to_char( "You can't wake up!\n\r", ch ); return; }

	send_to_char( "You wake and stand up.\n\r", ch );
	act( "$n wakes and stands up.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_STANDING;
	break;

    case POS_RESTING:
    case POS_SITTING:
	send_to_char( "You stand up.\n\r", ch );
	act( "$n stands up.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_STANDING;
	break;

    case POS_MEDITATING:
	send_to_char( "You uncross your legs and stand up.\n\r", ch );
	act( "$n uncrosses $s legs and stands up.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_STANDING;
	break;

    case POS_STANDING:
	send_to_char( "You are already standing.\n\r", ch );
	break;

    case POS_FIGHTING:
	send_to_char( "You are already fighting!\n\r", ch );
	break;
    }

    return;
}



void do_rest( CHAR_DATA *ch, char *argument )
{
    switch ( ch->position )
    {
    case POS_SLEEPING:
	send_to_char( "You are already sleeping.\n\r", ch );
	break;

    case POS_RESTING:
	send_to_char( "You are already resting.\n\r", ch );
	break;

    case POS_MEDITATING:
    case POS_SITTING:
    case POS_STANDING:
	send_to_char( "You rest.\n\r", ch );
	act( "$n rests.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_RESTING;
	break;

    case POS_FIGHTING:
	send_to_char( "You are already fighting!\n\r", ch );
	break;
    }
    return;
}



void do_sit( CHAR_DATA *ch, char *argument )
{
    switch ( ch->position )
    {
	case POS_SLEEPING:
	    send_to_char( "You are already sleeping.\n\r", ch );
	    break;
	case POS_RESTING:
	    send_to_char( "You sit up.\n\r", ch );
	    act( "$n sits up.", ch, NULL, NULL, TO_ROOM );
	    ch->position = POS_SITTING;
	    break;
	case POS_MEDITATING:
	    send_to_char( "You are already meditating.\n\r", ch );
	    break;
	case POS_SITTING:
	    send_to_char( "You are already sitting.\n\r", ch );
	    break;
	case POS_STANDING:
	    send_to_char( "You sit down.\n\r", ch );
	    act( "$n sits down.", ch, NULL, NULL, TO_ROOM );
	    ch->position = POS_SITTING;
	    break;
	case POS_FIGHTING:
	    send_to_char( "You are already fighting!\n\r", ch );
	    break;
    }

    return;
}



void do_meditate( CHAR_DATA *ch, char *argument )
{
    if (IS_NPC(ch) || (!IS_NINJA(ch) && !IS_MONK(ch) && !IS_DROW(ch) && !IS_MAGE(ch)))
    {
	stc("You are unable to attain the correct state of mind.\n\r", ch);
	return;
    }

    switch ( ch->position )
    {
    case POS_SLEEPING:
	send_to_char( "You are already sleeping.\n\r", ch );
	break;

    case POS_RESTING:
	send_to_char( "You are already resting.\n\r", ch );
	break;

    case POS_MEDITATING:
	send_to_char( "You are already meditating.\n\r", ch );
	break;

    case POS_SITTING:
	send_to_char( "You cross your legs and start meditating.\n\r", ch );
	act( "$n crosses $s legs and starts meditating.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_MEDITATING;
	break;

    case POS_STANDING:
	send_to_char( "You sit down, cross your legs and start meditating.\n\r", ch );
	act( "$n sits down, crosses $s legs and starts meditating.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_MEDITATING;
	break;

    case POS_FIGHTING:
	send_to_char( "You are already fighting!\n\r", ch );
	break;
    }
    return;
}



void do_sleep( CHAR_DATA *ch, char *argument )
{
    switch ( ch->position )
    {
    case POS_SLEEPING:
	send_to_char( "You are already sleeping.\n\r", ch );
	break;

    case POS_SITTING:
    case POS_MEDITATING:
    case POS_RESTING:
    case POS_STANDING: 
	send_to_char( "You sleep.\n\r", ch );
	act( "$n sleeps.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_SLEEPING;
	break;

    case POS_FIGHTING:
	send_to_char( "You are already fighting!\n\r", ch );
	break;
    }
    return;
}



void do_wake( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
	{ do_stand( ch, argument ); return; }

    if ( !IS_AWAKE(ch) )
	{ send_to_char( "You are asleep yourself!\n\r",       ch ); return; }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{ send_to_char( "They aren't here.\n\r",              ch ); return; }

    if ( IS_AWAKE(victim) )
	{ act( "$N is already awake.", ch, NULL, victim, TO_CHAR ); return; }

    if ( IS_AFFECTED(victim, AFF_SLEEP) )
	{ act( "You can't wake $M!",   ch, NULL, victim, TO_CHAR );  return; }

    if (victim->position < POS_SLEEPING)
	{ act( "$E doesn't respond!",   ch, NULL, victim, TO_CHAR );  return; }

    act( "You wake $M.", ch, NULL, victim, TO_CHAR );
    act( "$n wakes you.", ch, NULL, victim, TO_VICT );
    victim->position = POS_RESTING;
    return;
}



void do_sneak( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA af;

    send_to_char( "You attempt to move silently.\n\r", ch );
    affect_strip( ch, gsn_sneak );

    af.type      = gsn_sneak;
    af.duration  = ch->level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_SNEAK;
    affect_to_char( ch, &af );
    WAIT_STATE(ch, 12);
    return;
}


/*
 * Contributed by Alander.
 */
void do_visible( CHAR_DATA *ch, char *argument )
{
    affect_strip(ch, gsn_invis);
    affect_strip(ch, gsn_mass_invis);
    affect_strip(ch, gsn_sneak);

    REMOVE_BIT(ch->affected_by, AFF_HIDE);
    REMOVE_BIT(ch->affected_by, AFF_INVISIBLE);
    REMOVE_BIT(ch->affected_by, AFF_SNEAK);

    affect_strip(ch, gsn_mage_invis);
    REMOVE_BIT(ch->affected_by, AFF_KAKUSU);
    REMOVE_BIT(ch->itemaffect, ITEMA_BLISSFUL);
    REMOVE_BIT(ch->vampaff, VAM_OBFUSCATE);

    if (str_cmp(argument, "fogged")) stc("You are no longer hidden.\n\r", ch);
    return;
}



void do_unpolymorph( CHAR_DATA *ch, char *argument )
{
    if (!is_affected(ch, gsn_polymorph) )
    {
	send_to_char("But you are not polymorphed!\n\r",ch);
	return;
    }
    act("$n's body begins to distort.",ch,NULL,NULL,TO_ROOM);
    affect_strip ( ch, gsn_polymorph );
    act("$n resumes $s normal form.",ch,NULL,NULL,TO_ROOM);
    send_to_char( "You resume your normal form.\n\r", ch );
    return;
}



void do_recall( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *location;

    if (IS_NPC(ch) && strlen(ch->lord) < 2 && ch->level < 1000)
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    act( "A look of concentration crosses $n's face.", ch, NULL, NULL, TO_ROOM );

    if ( ( location = get_room_index( ch->home ) ) == NULL )
    {
	send_to_char( "You are completely lost.\n\r", ch );
	return;
    }
    if (ch->fight_timer > 0)
    {
        send_to_char("You may not recall with a fight timer.\n\r", ch);
        return;
    }

    if ( ch->in_room == location )
	return;

    if ( IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
    ||   IS_AFFECTED(ch, AFF_CURSE) )
    {
	send_to_char( "You are unable to recall to your haven.\n\r", ch );
	return;
    }
    if ( ( victim = ch->fighting ) != NULL )
    {
	sprintf( buf, "You recall from combat!\n\r");
	send_to_char( buf, ch );
	stop_fighting( ch, TRUE );
    }

    act( "$n disappears.", ch, NULL, NULL, TO_ROOM );
    char_from_room( ch );
    char_to_room( ch, location );
    act( "$n appears in the room.", ch, NULL, NULL, TO_ROOM );
    do_look( ch, "auto" );
    return;
}

void do_crecall( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *location;

    if (IS_NPC(ch)) return;

    act("A look of concentration crosses $n's face.", ch, NULL, NULL, TO_ROOM);

    if (strlen(ch->pcdata->covenant) < 2 || strlen(ch->pcdata->cov_time) < 2 || strlen(ch->pcdata->vleader) < 2)
    {
	stc("But you are not a member of any covenant!\n\r", ch);
	return;
    }

    if ((location = get_room_index(ch->pcdata->homecv)) == NULL)
    {
	stc("You are completely lost.\n\r", ch);
	return;
    }

    if (ch->in_room == location) return;

    if (IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL) || IS_AFFECTED(ch, AFF_CURSE) || ch->fight_timer > 0)
    {
	stc("You are unable to recall to your coven.\n\r", ch);
	return;
    }

    if ((victim = ch->fighting) != NULL)
    {
        sprintf(buf, "You recall from combat!\n\r");
        stc(buf, ch);
        stop_fighting(ch, TRUE);
    }
    act("$n disappears.", ch, NULL, NULL, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, location);
    act("$n appears in the room.", ch, NULL, NULL, TO_ROOM);
    do_look(ch, "auto");
    return;
}



void do_home( CHAR_DATA *ch, char *argument )
{
    char arg [MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if ( IS_NPC(ch) ) return;

    if ( arg[0] == '\0' || str_cmp(arg,"here") )
    {
	send_to_char( "If you wish this to be your home, you must type 'home here'.\n\r", ch );
	return;
    }

    if ( ch->in_room->vnum == ch->home )
    {
	send_to_char( "But this is already your home!\n\r", ch );
	return;
    }

    if ( (IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL) ||
	 IS_SET(ch->in_room->room_flags, ROOM_SAFE)) && ch->level < 4 )
    {
	send_to_char( "You are unable to make this room your home.\n\r", ch );
	return;
    }
    ch->home = ch->in_room->vnum;
    send_to_char( "This room is now your home.\n\r", ch );
    return;
}

void do_chome( CHAR_DATA *ch, char *argument )
{
    char arg [MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if ( IS_NPC(ch) ) return;

    if ( arg[0] == '\0' || str_cmp(arg,"here") )
    {
        send_to_char( "If you wish this to be your coven home, you must type 'chome here'.\n\r", ch );
        return;
    }
    if (strlen(ch->pcdata->covenant) < 2 || strlen(ch->pcdata->cov_time) < 2 || strlen(ch->pcdata->vleader) < 2)
    {
	send_to_char( "But you are not a member of any covenant!\n\r", ch );
	return;
    }

    if ( ch->in_room->vnum == ch->pcdata->homecv )
    {
        send_to_char( "But this is already your coven home!\n\r", ch );
        return;
    }

    if ( IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL) ||
         IS_SET(ch->in_room->room_flags, ROOM_SAFE) )
    {
        send_to_char( "You are unable to make this room your coven home.\n\r", ch );
        return;
    }
    ch->pcdata->homecv = ch->in_room->vnum;
    send_to_char( "This room is now your coven home.\n\r", ch );
    return;
}

void do_escape( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA *location;

    if (IS_NPC(ch))
	return;
  
    if ( ch->position >= POS_SLEEPING )
    {
	send_to_char( "You can only do this if you are dying.\n\r", ch );
	return;
    }

    if ( ( location = get_room_index( ROOM_VNUM_TEMPLE ) ) == NULL )
    {
	send_to_char( "You are completely lost.\n\r", ch );
	return;
    }

    if ( ch->in_room == location )
	return;

    ch->mana = 0;
    act( "$n fades out of existance.", ch, NULL, NULL, TO_ROOM );
    char_from_room( ch );
    char_to_room( ch, location );
    act( "$n fades into existance.", ch, NULL, NULL, TO_ROOM );
    do_look( ch, "auto" );
    sprintf(buf,"#r%s#n has escaped defenceless from a fight.",ch->name);
    do_info(ch,buf);

    return;
}

void do_train( CHAR_DATA *ch, char *argument )
{
    char arg1[MSL];
    char arg2[MSL];
    char arg3[MSL];
    char buf[MSL];
    long amount, cost, max;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    argument = one_argument(argument, arg3);

    if (IS_NPC(ch)) return;

    if (arg1[0] == '\0')
    {
	sprintf(buf, "You have %li experience points.\n\r", ch->exp);
	stc(buf, ch);
	stc("You can train the following:\n\r", ch);
	if (raw_stat(ch, STAT_STR) < stat_max(ch, STAT_STR))
	{
	    cost = stat_cost(ch, STAT_STR);
	    sprintf(buf, "Str    - %li.\n\r", cost);
	    stc(buf, ch);
	}
	if (raw_stat(ch, STAT_DEX) < stat_max(ch, STAT_DEX))
	{
	    cost = stat_cost(ch, STAT_DEX);
	    sprintf(buf, "Dex    - %li.\n\r", cost);
	    stc(buf, ch);
	}
	if (raw_stat(ch, STAT_CON) < stat_max(ch, STAT_CON))
	{
	    cost = stat_cost(ch, STAT_CON);
	    sprintf(buf, "Con    - %li.\n\r", cost);
	    stc(buf, ch);
	}
	if (raw_stat(ch, STAT_INT) < stat_max(ch, STAT_INT))
	{
	    cost = stat_cost(ch, STAT_INT);
	    sprintf(buf, "Int    - %li.\n\r", cost);
	    stc(buf, ch);
	}
	if (raw_stat(ch, STAT_WIS) < stat_max(ch, STAT_WIS))
	{
	    cost = stat_cost(ch, STAT_WIS);
	    sprintf(buf, "Wis    - %li.\n\r", cost);
	    stc(buf, ch);
	}
	if (ch->max_hit < 100000)
	{
	    sprintf(buf, "hp     - %li exp per point.\n\r", ch->max_hit / 10);
	    stc(buf, ch);
	}
	if (IS_ITEMAFF(ch, ITEMA_REFLECT)) ch->max_mana += 10000;
	if (ch->max_mana < 100000)
	{
	    sprintf(buf, "mana   - %li exp per point.\n\r", ch->max_mana / 10);
	    stc(buf, ch);
	}
	if (IS_ITEMAFF(ch, ITEMA_REFLECT)) ch->max_mana -= 10000;
	if (ch->primal < 100)
	{
	    sprintf(buf, "primal - %li exp per point of primal energy.\n\r", (ch->primal + 1) * 50);
	    stc(buf, ch);
	}
	return;
    }

    if (!str_cmp(arg1, "hp") || !str_cmp(arg1, "mana") || !str_cmp(arg1, "primal"))
    {
	amount = atoi(arg2);
	if (amount < 1 || amount > 10000)
	{
	    stc("Please enter a value between 1 and 10000.\n\r", ch);
	    return;
	}
	
	cost = amount;
    }

    if (!str_cmp(arg1, "str"))
    {
	max = stat_max(ch, STAT_STR);
	cost = stat_cost(ch, STAT_STR);
	if (raw_stat(ch, STAT_STR) >= max)
	{
	    sprintf(buf, "You cannot train more than %li strength.\n\r", max);
	    cost = 0;
	}
	else if (cost > ch->exp)
	{
	    sprintf(buf, "You need %li experience points to train strength.\n\r", cost);
	    cost = 0;
	}
	else
	{
	    ch->stats[STAT_STR]++;
	    sprintf(buf, "You raise your strength.\n\r");
	}
    }
    else if (!str_cmp(arg1, "dex"))
    {
	max = stat_max(ch, STAT_DEX);
	cost = stat_cost(ch, STAT_DEX);
	if (raw_stat(ch, STAT_DEX) >= max)
	{
	    sprintf(buf, "You cannot train more than %li dexterity.\n\r", max);
	    cost = 0;
	}
	else if (cost > ch->exp)
	{
	    sprintf(buf, "You need %li experience points to train dexterity.\n\r", cost);
	    cost = 0;
	}
	else
	{
	    ch->stats[STAT_DEX]++;
	    sprintf(buf, "You raise your dexterity.\n\r");
	}
    }
    else if (!str_cmp(arg1, "con"))
    {
	max = stat_max(ch, STAT_CON);
	cost = stat_cost(ch, STAT_CON);
	if (raw_stat(ch, STAT_CON) >= max)
	{
	    sprintf(buf, "You cannot train more than %li constitution.\n\r", max);
	    cost = 0;
	}
	else if (cost > ch->exp)
	{
	    sprintf(buf, "You need %li experience points to train constitution.\n\r", cost);
	    cost = 0;
	}
	else
	{
	    ch->stats[STAT_CON]++;
	    sprintf(buf, "You improve your constitution.\n\r");
	}
    }
    else if (!str_cmp(arg1, "int"))
    {
	max = stat_max(ch, STAT_INT);
	cost = stat_cost(ch, STAT_INT);
	if (raw_stat(ch, STAT_INT) >= max)
	{
	    sprintf(buf, "You cannot train more than %li intellect.\n\r", max);
	    cost = 0;
	}
	else if (cost > ch->exp)
	{
	    sprintf(buf, "You need %li experience points to train intellect.\n\r", cost);
	    cost = 0;
	}
	else
	{
	    ch->stats[STAT_INT]++;
	    sprintf(buf, "You increase your intellect.\n\r");
	}
    }
    else if (!str_cmp(arg1, "wis"))
    {
	max = stat_max(ch, STAT_WIS);
	cost = stat_cost(ch, STAT_WIS);
	if (raw_stat(ch, STAT_WIS) >= max)
	{
	    sprintf(buf, "You cannot train more than %li wisdom.\n\r", max);
	    cost = 0;
	}
	else if (cost > ch->exp)
	{
	    sprintf(buf, "You need %li experience points to train wisdom.\n\r", cost);
	    cost = 0;
	}
	else
	{
	    ch->stats[STAT_WIS]++;
	    sprintf(buf, "You increase your wisdom.\n\r");
	}
    }
    else if (!str_cmp(arg1, "hp"))
    {
	max = 100000;
	cost *= (ch->max_hit + amount / 2) / 10;
	if (cost < 0) cost = 0;
	if (is_affected(ch, skill_lookup("spiralmoon")))
	{
	    stc("You cannot train your hit points while Spiral Moon is in affect!\n\r",ch);
	    return;
	}
	if (ch->max_hit + amount > max)
	{
	    sprintf(buf,"You can't train more than %li hp.\n\r",max);
	    cost = 0;
	}
	else if (cost > ch->exp)
	{
	    sprintf(buf,"You need %li experience points to train %li hitpoints.\n\r",cost,amount);
	    cost = 0;
        }
	else
	{
	    ch->max_hit += amount;
	    sprintf(buf,"You gain %li hp.\n\r",amount);
	}
    }
    else if (!str_cmp(arg1, "mana"))
    {
	max = 100000;
	if (IS_ITEMAFF(ch, ITEMA_REFLECT)) ch->max_mana += 10000;
	cost *= (ch->max_mana + amount / 2) / 10;
	if (cost < 1) cost = 1;
	if (ch->max_mana + amount > max)
	{
	    sprintf(buf,"You can't train more than %li mana.\n\r",max);
	    cost = 0;
	}
	else if (cost > ch->exp)
	{
	    sprintf(buf,"You need %li experience points to train %li mana.\n\r",cost,amount);
	    cost = 0;
	}
	else
	{
	    ch->max_mana += amount;
	    sprintf(buf,"You gain %li mana.\n\r",amount);
	}
	if (IS_ITEMAFF(ch, ITEMA_REFLECT)) ch->max_mana -= 10000;
    }
    else if (!str_cmp(arg1, "primal"))
    {
	max = 100;
	cost *= 50 * (ch->primal + amount / 2);
	if (cost > ch->exp)
	{
	    sprintf(buf,"You need %li experience points to train %li primal.\n\r",cost,amount);
	    cost = 0;
	}
        else if (ch->primal + amount > max)
	{
	    sprintf(buf,"You can't train more than %li primal.\n\r",max);
	    cost = 0;
        }
	else
	{
	    ch->primal += amount;
	    sprintf(buf,"You gain %li primal.\n\r",amount);
	}
    }
    else
    {
	do_train(ch, "");
	return;
    }
    ch->exp -= cost;
    stc(buf, ch);
    return;
}

void do_tie( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg [MAX_INPUT_LENGTH];
    char buf [MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );
    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if (IS_NPC(victim)) return;

    if (victim == ch)
    {
	send_to_char( "You cannot tie yourself up!\n\r", ch );
	return;
    }
    if (IS_EXTRA(victim,TIED_UP))
    {
	send_to_char( "But they are already tied up!\n\r", ch );
	return;
    }
    if (is_safe(ch,victim)) return;
    if (victim->position > POS_STUNNED || victim->hit > 0)
    {
	send_to_char( "You can only tie up a defenceless person.\n\r", ch );
	return;
    }
    act("You quickly tie up $N.",ch,NULL,victim,TO_CHAR);
    act("$n quickly ties up $N.",ch,NULL,victim,TO_ROOM);
    send_to_char("You have been tied up!\n\r",victim);
    SET_BIT(victim->extra,TIED_UP);
    sprintf(buf,"#r%s#n has been tied up by #r%s#n.",victim->name,ch->name);
    do_info(ch,buf);
    return;
}

void do_untie( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg [MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );
    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }
    if (!IS_EXTRA(victim,TIED_UP))
    {
	send_to_char( "But they are not tied up!\n\r", ch );
	return;
    }
    if (victim == ch)
    {
	send_to_char( "You cannot untie yourself!\n\r", ch );
	return;
    }
    act("You quickly untie $N.",ch,NULL,victim,TO_CHAR);
    act("$n quickly unties $N.",ch,NULL,victim,TO_NOTVICT);
    act("$n quickly unties you.",ch,NULL,victim,TO_VICT);
    REMOVE_BIT(victim->extra,TIED_UP);
    return;
}

void do_gag( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg [MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );
    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }
    if (victim == ch && !IS_EXTRA(victim,GAGGED) && IS_EXTRA(victim,TIED_UP))
    {
	send_to_char( "You cannot gag yourself!\n\r", ch );
	return;
    }
    if (!IS_EXTRA(victim,TIED_UP) && !IS_EXTRA(victim,GAGGED))
    {
	send_to_char( "You can only gag someone who is tied up!\n\r", ch );
	return;
    }
    if (!IS_EXTRA(victim,GAGGED))
    {
    	act("You place a gag over $N's mouth.",ch,NULL,victim,TO_CHAR);
    	act("$n places a gag over $N's mouth.",ch,NULL,victim,TO_NOTVICT);
    	act("$n places a gag over your mouth.",ch,NULL,victim,TO_VICT);
    	SET_BIT(victim->extra,GAGGED);
	return;
    }
    if (ch == victim)
    {
    	act("You remove the gag from your mouth.",ch,NULL,victim,TO_CHAR);
    	act("$n removes the gag from $s mouth.",ch,NULL,victim,TO_ROOM);
    	REMOVE_BIT(victim->extra,GAGGED);
	return;
    }
    act("You remove the gag from $N's mouth.",ch,NULL,victim,TO_CHAR);
    act("$n removes the gag from $N's mouth.",ch,NULL,victim,TO_NOTVICT);
    act("$n removes the gag from your mouth.",ch,NULL,victim,TO_VICT);
    REMOVE_BIT(victim->extra,GAGGED);
    return;
}

void do_blindfold( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg [MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );
    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }
    if (victim == ch && !IS_EXTRA(victim,BLINDFOLDED) && IS_EXTRA(victim,TIED_UP))
    {
	send_to_char( "You cannot blindfold yourself!\n\r", ch );
	return;
    }
    if (!IS_EXTRA(victim,TIED_UP) && !IS_EXTRA(victim,BLINDFOLDED))
    {
	send_to_char( "You can only blindfold someone who is tied up!\n\r", ch );
	return;
    }
    if (!IS_EXTRA(victim,BLINDFOLDED))
    {
    	act("You place a blindfold over $N's eyes.",ch,NULL,victim,TO_CHAR);
    	act("$n places a blindfold over $N's eyes.",ch,NULL,victim,TO_NOTVICT);
    	act("$n places a blindfold over your eyes.",ch,NULL,victim,TO_VICT);
    	SET_BIT(victim->extra,BLINDFOLDED);
	return;
    }
    if (ch == victim)
    {
    	act("You remove the blindfold from your eyes.",ch,NULL,victim,TO_CHAR);
    	act("$n removes the blindfold from $s eyes.",ch,NULL,victim,TO_ROOM);
    	REMOVE_BIT(victim->extra,BLINDFOLDED);
	return;
    }
    act("You remove the blindfold from $N's eyes.",ch,NULL,victim,TO_CHAR);
    act("$n removes the blindfold from $N's eyes.",ch,NULL,victim,TO_NOTVICT);
    act("$n removes the blindfold from your eyes.",ch,NULL,victim,TO_VICT);
    REMOVE_BIT(victim->extra,BLINDFOLDED);
    return;
}

void do_pour( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch) || !IS_AFFECTED(ch, AFF_POLYMORPH) || 
	!IS_EXTRA(ch, EXTRA_OSWITCH) || ch->pcdata->obj_vnum != 
	OBJ_VNUM_SPILT_BLOOD)
    {
	send_to_char("Huh?\n\r",ch);
	return;
    }

    if ( ( obj = ch->pcdata->chobj ) == NULL )
    {
	send_to_char("Huh?\n\r",ch);
	return;
    }

    if ( obj->chobj == NULL || obj->chobj != ch )
    {
	send_to_char("Huh?\n\r",ch);
	return;
    }

    if ( obj->in_room == NULL )
    {
	send_to_char("You are unable to move.\n\r",ch);
	return;
    }

    if      (!str_cmp(arg,"n") || !str_cmp(arg,"north")) do_north(ch,"");
    else if (!str_cmp(arg,"s") || !str_cmp(arg,"south")) do_south(ch,"");
    else if (!str_cmp(arg,"e") || !str_cmp(arg,"east" )) do_east(ch,"");
    else if (!str_cmp(arg,"w") || !str_cmp(arg,"west" )) do_west(ch,"");
    else if (!str_cmp(arg,"u") || !str_cmp(arg,"up"   ))
    {
	send_to_char("You cannot pour up.\n\r",ch);
	return;
    }
    else if (!str_cmp(arg,"d") || !str_cmp(arg,"down" )) do_down(ch,"");
    else
    {
	send_to_char("Do you wish to roll north, south, east, west, down?\n\r",ch);
	return;
    }
    obj_from_room(obj);
    obj_to_room(obj,ch->in_room);
    return;
}
