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
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"



/*
 * Local functions.
 */
void	get_obj		args( ( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container ) );




void get_obj( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container )
{
    OBJ_DATA *obj2;
    OBJ_DATA *obj_next;
    ROOM_INDEX_DATA *objroom;
    bool move_ch = FALSE;

    if (objects_in_hand(ch)) return;

    if (IS_AFFECTED(ch, AFF_SHADOWPLANE) && obj->in_room != NULL && !IS_SET(obj->extra_flags, ITEM_SHADOWPLANE))
    {
	send_to_char( "Your hand passes right through it!\n\r", ch );
	return;
    }
    if (!IS_AFFECTED(ch, AFF_SHADOWPLANE) && obj->in_room != NULL && IS_SET(obj->extra_flags, ITEM_SHADOWPLANE))
    {
	send_to_char( "Your hand passes right through it!\n\r", ch );
	return;
    }
    if (!CAN_WEAR(obj, ITEM_TAKE))
    {
	send_to_char( "You can't take that.\n\r", ch );
	return;
    }

    if ( container != NULL )
    {
    	if  ( IS_AFFECTED(ch,AFF_SHADOWPLANE) &&
	     !IS_SET(container->extra_flags, ITEM_SHADOWPLANE) &&
	     (container->carried_by == NULL || container->carried_by != ch) )
    	{
	    send_to_char( "Your hand passes right through it!\n\r", ch );
	    return;
    	}
    	if  (!IS_AFFECTED(ch,AFF_SHADOWPLANE) &&
	      IS_SET(container->extra_flags, ITEM_SHADOWPLANE) &&
	     (container->carried_by == NULL || container->carried_by != ch) )
    	{
	    send_to_char( "Your hand passes right through it!\n\r", ch );
	    return;
        }
	act( "You get $p from $P.", ch, obj, container, TO_CHAR );
	act( "$n gets $p from $P.", ch, obj, container, TO_ROOM );
	for ( obj2 = container->contains; obj2 != NULL; obj2 = obj_next )
	{
	    obj_next = obj2->next_content;
	    if ( obj2->chobj != NULL )
		{act( "A hand reaches inside $P and takes $p out.", obj2->chobj, obj, container, TO_CHAR );
		move_ch = TRUE;}
	}
	obj_from_obj( obj );
    }
    else
    {
	act( "You pick up $p.", ch, obj, container, TO_CHAR );
	act( "$n picks $p up.", ch, obj, container, TO_ROOM );
	if (obj != NULL) obj_from_room( obj );
    }

    if ( obj->item_type == ITEM_MONEY )
    {
	ch->gold += obj->value[0];
	extract_obj( obj );
    }
    else
    {
	obj_to_char( obj, ch );
	if ( move_ch && obj->chobj != NULL )
	{
	    if (obj->carried_by != NULL && obj->carried_by != obj->chobj)
	    	objroom = get_room_index(obj->carried_by->in_room->vnum);
	    else objroom = NULL;
	    if (objroom != NULL && get_room_index(obj->chobj->in_room->vnum) != objroom)
	    {
	    	char_from_room(obj->chobj);
	    	char_to_room(obj->chobj,objroom);
		do_look(obj->chobj,"auto");
	    }
	}
    	if (IS_AFFECTED(ch,AFF_SHADOWPLANE) &&
	    (IS_SET(obj->extra_flags, ITEM_SHADOWPLANE) ) )
	    REMOVE_BIT(obj->extra_flags, ITEM_SHADOWPLANE);
    }

    return;
}



void do_get( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    OBJ_DATA *container;
    bool found;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (IS_AFFECTED(ch,AFF_ETHEREAL) || IS_EXTRA(ch,EXTRA_ILLUSION))
    {
	send_to_char( "You cannot pick things up while ethereal.\n\r", ch );
	return;
    }


    /* Get type. */
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Get what?\n\r", ch );
	return;
    }

    if ( arg2[0] == '\0' )
    {
	if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
	{
	    /* 'get obj' */
	    obj = get_obj_list( ch, arg1, ch->in_room->contents );
	    if ( obj == NULL )
	    {
		act( "I see no $T here.", ch, NULL, arg1, TO_CHAR );
		return;
	    }

	    get_obj( ch, obj, NULL );
	}
	else
	{
	    /* 'get all' or 'get all.obj' */
	    found = FALSE;
	    for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	    {
		obj_next = obj->next_content;
		if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
		&&   can_see_obj( ch, obj ) )
		{
		    found = TRUE;
		    get_obj( ch, obj, NULL );
		}
	    }

	    if ( !found ) 
	    {
		if ( arg1[3] == '\0' )
		    send_to_char( "I see nothing here.\n\r", ch );
		else
		    act( "I see no $T here.", ch, NULL, &arg1[4], TO_CHAR );
	    }
	}
    }
    else
    {
	/* 'get ... container' */
	if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
	{
	    send_to_char( "You can't do that.\n\r", ch );
	    return;
	}

	if ( ( container = get_obj_here( ch, arg2 ) ) == NULL )
	{
	    act( "I see no $T here.", ch, NULL, arg2, TO_CHAR );
	    return;
	}

	switch ( container->item_type )
	{
	default:
	    send_to_char( "That's not a container.\n\r", ch );
	    return;

	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	    break;

	case ITEM_CORPSE_PC:
	    {
		if ( IS_NPC(ch) )
		{
		    send_to_char( "You can't do that.\n\r", ch );
		    return;
		}
		if (!CAN_PK(ch) && strlen(container->questowner) > 1)
		{
		    if ( str_cmp( container->questowner, ch->name ) )
		    {
			send_to_char( "You cannot loot the corpses of other players.\n\r", ch );
			return;
		    }
		}
	    }
	}

	if ( IS_SET(container->value[1], CONT_CLOSED) )
	{
	    act( "The $d is closed.", ch, NULL, container->name, TO_CHAR );
	    return;
	}

	if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
	{
	    /* 'get obj container' */
	    obj = get_obj_list( ch, arg1, container->contains );
	    if ( obj == NULL)
	    {
		act( "I see nothing like that in the $T.",
		    ch, NULL, arg2, TO_CHAR );
		return;
	    }
	    get_obj( ch, obj, container );
	}
	else
	{
	    /* 'get all container' or 'get all.obj container' */
	    found = FALSE;
	    for ( obj = container->contains; obj != NULL; obj = obj_next )
	    {
		obj_next = obj->next_content;
		if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
		&&   can_see_obj( ch, obj ) )
		{
		    found = TRUE;
		    get_obj( ch, obj, container );
		}
	    }

	    if ( !found )
	    {
		if ( arg1[3] == '\0' )
		    act( "I see nothing in the $T.", ch, NULL, arg2, TO_CHAR );
		else
		    act( "I see nothing like that in the $T.", ch, NULL, arg2, TO_CHAR );
	    }
	}
    }
    do_autosave(ch, "");
    return;
}



void do_put( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *container;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Put what in what?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
    {
	send_to_char( "You can't do that.\n\r", ch );
	return;
    }

    if ( ( container = get_obj_here( ch, arg2 ) ) == NULL )
    {
	act( "I see no $T here.", ch, NULL, arg2, TO_CHAR );
	return;
    }

    if ( container->item_type != ITEM_CONTAINER )
    {
	send_to_char( "That's not a container.\n\r", ch );
	return;
    }

    if ( IS_SET(container->value[1], CONT_CLOSED) )
    {
	act( "The $d is closed.", ch, NULL, container->name, TO_CHAR );
	return;
    }

    if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
    {
	/* 'put obj container' */
	if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
	{
	    send_to_char( "You do not have that item.\n\r", ch );
	    return;
	}

	if ( obj == container )
	{
	    send_to_char( "You can't fold it into itself.\n\r", ch );
	    return;
	}

        if (IS_SET(obj->quest, QUEST_ARTIFACT))
        {
	    send_to_char( "It won't go in.\n\r", ch );
	    return;
        }

	if (container->questowner == NULL || container->questowner[0] == '\0')
	{
	    stc("Containers must be claimed to be used.\n\r", ch);
	    return;
	}

	if (obj->questowner != NULL && str_cmp(container->questowner, obj->questowner))
	{
	    stc("You can only put claimed objects in containers claimed by the same person.\n\r", ch);
	    return;
	}

	if (objects_in_container(ch, container)) return;

	obj_from_char( obj );
	obj_to_obj( obj, container );
	act( "$n puts $p in $P.", ch, obj, container, TO_ROOM );
	act( "You put $p in $P.", ch, obj, container, TO_CHAR );
    }
    else
    {
	/* 'put all container' or 'put all.obj container' */
	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    if (IS_SET(obj->quest, QUEST_ARTIFACT)) continue;
	    if (objects_in_container(ch, container)) return;
	    if (container->questowner == NULL || container->questowner[0] == '\0') continue;
	    if (obj->questowner != NULL && str_cmp(container->questowner, obj->questowner)) continue;
	    if ((arg1[3] == '\0' || is_name(&arg1[4], obj->name))
	    &&   can_see_obj(ch, obj)
	    &&   obj->wear_loc == WEAR_NONE
	    &&   obj != container)
	    {
		obj_from_char( obj );
		obj_to_obj( obj, container );
		act( "$n puts $p in $P.", ch, obj, container, TO_ROOM );
		act( "You put $p in $P.", ch, obj, container, TO_CHAR );
	    }
	}
    }
    do_autosave(ch,"");
    return;
}



void do_drop( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    bool found;

    argument = one_argument( argument, arg );

    if (IS_AFFECTED(ch,AFF_ETHEREAL) || IS_EXTRA(ch,EXTRA_ILLUSION))
    {
	send_to_char( "You cannot pick things up while ethereal.\n\r", ch );
	return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char( "Drop what?\n\r", ch );
	return;
    }

    if ( is_number( arg ) )
    {
	long amount   = atoi(arg);
	argument = one_argument( argument, arg );
	if ( amount <= 0 || ( str_cmp( arg, "coins" ) && str_cmp( arg, "coin" ) ) )
	{
	    send_to_char( "Sorry, you can't do that.\n\r", ch );
	    return;
	}

	/* Otherwise causes complications if there's a pile on each plane */
	if (IS_AFFECTED(ch,AFF_SHADOWPLANE) )
	{
	    send_to_char( "You cannot drop coins in the Umbra.\n\r", ch );
	    return;
	}

	if ( ch->gold < amount )
	{
	    send_to_char( "You haven't got that many coins.\n\r", ch );
	    return;
	}

	ch->gold -= amount;

	for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    switch ( obj->pIndexData->vnum )
	    {
	    case OBJ_VNUM_MONEY_ONE:
		amount += 1;
		extract_obj( obj );
		break;

	    case OBJ_VNUM_MONEY_SOME:
		amount += obj->value[0];
		extract_obj( obj );
		break;
	    }
	}
	obj_to_room( create_money( amount ), ch->in_room );
	act( "$n drops some gold.", ch, NULL, NULL, TO_ROOM );
	send_to_char( "OK.\n\r", ch );
	do_autosave(ch,"");
	return;
    }

    if ( str_cmp( arg, "all" ) && str_prefix( "all.", arg ) )
    {
	/* 'drop obj' */
	if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
	{
	    send_to_char( "You do not have that item.\n\r", ch );
	    return;
	}

	if (objects_in_room(ch)) return;

	obj_from_char( obj );
	obj_to_room( obj, ch->in_room );
	/* Objects should only have a shadowplane flag when on the floor */
	if (IS_AFFECTED(ch,AFF_SHADOWPLANE) &&
	    (!IS_SET(obj->extra_flags, ITEM_SHADOWPLANE) ) )
	    SET_BIT(obj->extra_flags, ITEM_SHADOWPLANE);
	act( "$n drops $p.", ch, obj, NULL, TO_ROOM );
	act( "You drop $p.", ch, obj, NULL, TO_CHAR );
    }
    else
    {
	/* 'drop all' or 'drop all.obj' */
	found = FALSE;
	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    if (objects_in_room(ch)) return;
	    if ( ( arg[3] == '\0' || is_name( &arg[4], obj->name ) )
	    &&   can_see_obj( ch, obj )
	    &&   obj->wear_loc == WEAR_NONE)
	    {
		found = TRUE;
		obj_from_char( obj );
		obj_to_room( obj, ch->in_room );
		/* Objects should only have a shadowplane flag when on the floor */
		if (IS_AFFECTED(ch,AFF_SHADOWPLANE) &&
		    (!IS_SET(obj->extra_flags, ITEM_SHADOWPLANE) ) )
		    SET_BIT(obj->extra_flags, ITEM_SHADOWPLANE);
		act( "$n drops $p.", ch, obj, NULL, TO_ROOM );
		act( "You drop $p.", ch, obj, NULL, TO_CHAR );
	    }
	}

	if ( !found )
	{
	    if ( arg[3] == '\0' )
		act( "You are not carrying anything.",
		    ch, NULL, arg, TO_CHAR );
	    else
		act( "You are not carrying any $T.",
		    ch, NULL, &arg[4], TO_CHAR );
	}
    }

    do_autosave(ch,"");
    return;
}



void do_give( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char buf [MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA  *obj;
    OBJ_DATA  * obj_next;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Give what to whom?\n\r", ch );
	return;
    }

    if ( is_number( arg1 ) )
    {
	/* 'give NNNN coins victim' */
	long amount;

	amount   = atoi(arg1);
	if ( amount <= 0
	|| ( str_cmp( arg2, "coins" ) && str_cmp( arg2, "coin" ) ) )
	{
	    send_to_char( "Sorry, you can't do that.\n\r", ch );
	    return;
	}

	argument = one_argument( argument, arg2 );
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "Give what to whom?\n\r", ch );
	    return;
	}

	if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

    	if (IS_AFFECTED(victim,AFF_ETHEREAL) || IS_EXTRA(victim,EXTRA_ILLUSION))
    	{
	    send_to_char( "You cannot give things to ethereal people.\n\r", ch );
	    return;
    	}

	if ( ch->gold < amount )
	{
	    send_to_char( "You haven't got that much gold.\n\r", ch );
	    return;
	}

	ch->gold     -= amount;
	victim->gold += amount;
	act( "$n gives you some gold.", ch, NULL, victim, TO_VICT    );
	act( "$n gives $N some gold.",  ch, NULL, victim, TO_NOTVICT );
	act( "You give $N some gold.",  ch, NULL, victim, TO_CHAR    );
	send_to_char( "OK.\n\r", ch );
	do_autosave(ch,"");
	do_autosave(victim,"");
	return;
    }

    if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
    {


    if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if ( obj->wear_loc != WEAR_NONE )
    {
	send_to_char( "You must remove it first.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if (objects_in_hand(victim))
    {
	stc("Their hands are full.\n\r", ch);
	return;
    }

    if (IS_AFFECTED(victim,AFF_ETHEREAL) )
    {
	send_to_char( "You cannot give things to ethereal people.\n\r", ch );
	return;
    }

    if ( !can_see_obj( victim, obj ) )
    {
	act( "$N can't see it.", ch, NULL, victim, TO_CHAR );
	return;
    }

    obj_from_char( obj );
    obj_to_char( obj, victim );
    act( "$n gives $p to $N.", ch, obj, victim, TO_NOTVICT );
    act( "$n gives you $p.",   ch, obj, victim, TO_VICT    );
    act( "You give $p to $N.", ch, obj, victim, TO_CHAR    );
    do_autosave(ch,"");
    do_autosave(victim,"");
    }
    else
    {
	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
        {
            obj_next = obj->next_content;


  	    if ( ( arg2[3] != '\0' && !is_name( &arg2[4], obj->name ) ))
  	    {
	        continue;
	    }
	    else
	    {
	        sprintf(buf, "%s %s", arg1, obj->name);
	        do_give(ch, buf);
	    }

	}
    }
    return;
}




void do_eat( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];
    OBJ_DATA *obj;

    one_argument(argument, arg);

    if (IS_NPC(ch)) return;

    if (arg[0] == '\0')
    {
	stc("Eat what?\n\r", ch);
	return;
    }

    if ((obj = get_obj_carry(ch, arg)) == NULL)
    {
	stc("You do not have that item.\n\r", ch);
	return;
    }

    if (obj->item_type != ITEM_QUEST)
    {
	if (!IS_IMM(ch))
	{
	    stc("That's not edible.\n\r", ch);
	    return;
	}
    }
    else ch->pcdata->quest += obj->value[0];

    act("$n eats $p.", ch, obj, NULL, TO_ROOM);
    act("You eat $p.", ch, obj, NULL, TO_CHAR);
    if (obj != NULL) extract_obj(obj);

    return;
}



/*
 * Remove an object.
 */
bool remove_obj( CHAR_DATA *ch, long iWear, bool fReplace )
{
    OBJ_DATA *obj;

    if ((obj = get_eq_char(ch, iWear)) == NULL) return TRUE;

    if (!fReplace) return FALSE;

    unequip_char(ch, obj);
    act("$n stops using $p.", ch, obj, NULL, TO_ROOM);
    act("You stop using $p.", ch, obj, NULL, TO_CHAR);
    return TRUE;
}



/*
 * Wear one object.
 * Optional replacement of existing objects.
 * Big repetitive code, ick.
 */
void wear_obj( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace )
{
    if (obj->item_type == ITEM_WEAPON)
    {
	if ((IS_DROW(ch) && get_drow(ch, DROW_DUALWIELD))
	 || (IS_NINJA(ch) && get_ninja(ch, NINJA_NINGENNO) >= 8))
	{
	    if (get_eq_char(ch, WEAR_WEAPON) != NULL && get_eq_char(ch, WEAR_HOLD) == NULL && ch->style[0] != STYLE_TWOHAND)
	    {
		act("$n wields $p in $s off hand.", ch, obj, NULL, TO_ROOM);
		act("You wield $p in your off hand.", ch, obj, NULL, TO_CHAR);

		equip_char(ch, obj, WEAR_HOLD);
		update_style(ch);
		return;
	    }
	}

	if (!remove_obj(ch, WEAR_WEAPON, fReplace)) return;

	act("$n wields $p in $s main hand.", ch, obj, NULL, TO_ROOM);
	act("You wield $p in your main hand.", ch, obj, NULL, TO_CHAR);

	equip_char(ch, obj, WEAR_WEAPON);

	update_style(ch);
	return;
    }

    if (CAN_WEAR(obj, ITEM_WEAR_HOLD))
    {
	if (ch->style[0] == STYLE_TWOHAND || !remove_obj(ch, WEAR_HOLD, fReplace)) return;

	if (obj->item_type == ITEM_LIGHT)
	{
	    act("$n lights $p and clutches it in $s off hand.", ch, obj, NULL, TO_ROOM);
	    act("You light $p and clutch it in your off hand.", ch, obj, NULL, TO_CHAR);
	}
	else if (obj->item_type == ITEM_ARMOR)
	{
	    act("$n readies $p in $s off hand.", ch, obj, NULL, TO_ROOM);
	    act("You ready $p in your off hand.", ch, obj, NULL, TO_CHAR);
	}
	else
	{
	    act("$n clutches $p in $s off hand.", ch, obj, NULL, TO_ROOM);
	    act("You clutch $p in your off hand.", ch, obj, NULL, TO_CHAR);
	}

	equip_char(ch, obj, WEAR_HOLD);
	update_style(ch);
	return;
    }
    if ( CAN_WEAR( obj, ITEM_WEAR_FINGER ) )
    {
	if ( get_eq_char( ch, WEAR_FINGER_L ) != NULL
	&&   get_eq_char( ch, WEAR_FINGER_R ) != NULL
	&&   !remove_obj( ch, WEAR_FINGER_L, fReplace )
	&&   !remove_obj( ch, WEAR_FINGER_R, fReplace ) )
	    return;

	if ( get_eq_char( ch, WEAR_FINGER_L ) == NULL)
	{
	    act( "$n wears $p on $s left finger.",    ch, obj, NULL, TO_ROOM );
	    act( "You wear $p on your left finger.",  ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_FINGER_L );
	    return;
	}
	else if ( get_eq_char( ch, WEAR_FINGER_R ) == NULL)
	{
	    act( "$n wears $p on $s right finger.",   ch, obj, NULL, TO_ROOM );
	    act( "You wear $p on your right finger.", ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_FINGER_R );
	    return;
	}
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_NECK ) )
    {
	if ( get_eq_char( ch, WEAR_NECK_1 ) != NULL
	&&   get_eq_char( ch, WEAR_NECK_2 ) != NULL
	&&   !remove_obj( ch, WEAR_NECK_1, fReplace )
	&&   !remove_obj( ch, WEAR_NECK_2, fReplace ) )
	    return;

	if ( get_eq_char( ch, WEAR_NECK_1 ) == NULL )
	{
	    act( "$n slips $p around $s neck.",   ch, obj, NULL, TO_ROOM );
	    act( "You slip $p around your neck.", ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_NECK_1 );
	    return;
	}

	if ( get_eq_char( ch, WEAR_NECK_2 ) == NULL )
	{
	    act( "$n slips $p around $s neck.",   ch, obj, NULL, TO_ROOM );
	    act( "You slip $p around your neck.", ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_NECK_2 );
	    return;
	}
	bug( "Wear_obj: no free neck.", 0 );
	send_to_char( "You are already wearing two things around your neck.\n\r", ch );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_BODY ) )
    {
	if ( !remove_obj( ch, WEAR_BODY, fReplace ) )
	    return;

	act( "$n fits $p on $s body.",   ch, obj, NULL, TO_ROOM );
	act( "You fit $p on your body.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_BODY );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_HEAD ) )
    {
	if ( !remove_obj( ch, WEAR_HEAD, fReplace ) )
	    return;
	act( "$n places $p on $s head.",   ch, obj, NULL, TO_ROOM );
	act( "You place $p on your head.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_HEAD );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_FACE ) )
    {
	if ( !remove_obj( ch, WEAR_FACE, fReplace ) )
	    return;
	act( "$n places $p on $s face.",   ch, obj, NULL, TO_ROOM );
	act( "You place $p on your face.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_FACE );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_LEGS ) )
    {
	if ( !remove_obj( ch, WEAR_LEGS, fReplace ) )
	    return;
	act( "$n slips $s legs into $p.",   ch, obj, NULL, TO_ROOM );
	act( "You slip your legs into $p.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_LEGS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_FEET ) )
    {
	if ( !remove_obj( ch, WEAR_FEET, fReplace ) )
	    return;
	act( "$n slips $s feet into $p.",   ch, obj, NULL, TO_ROOM );
	act( "You slip your feet into $p.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_FEET );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_HANDS ) )
    {
	if ( !remove_obj( ch, WEAR_HANDS, fReplace ) )
	    return;
	act( "$n pulls $p onto $s hands.",   ch, obj, NULL, TO_ROOM );
	act( "You pull $p onto your hands.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_HANDS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_ARMS ) )
    {
	if ( !remove_obj( ch, WEAR_ARMS, fReplace ) )
	    return;
	act( "$n slides $s arms into $p.",   ch, obj, NULL, TO_ROOM );
	act( "You slide your arms into $p.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_ARMS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_ABOUT ) )
    {
	if ( !remove_obj( ch, WEAR_ABOUT, fReplace ) )
	    return;
	act( "$n pulls $p about $s body.",   ch, obj, NULL, TO_ROOM );
	act( "You pull $p about your body.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_ABOUT );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_WAIST ) )
    {
	if ( !remove_obj( ch, WEAR_WAIST, fReplace ) )
	    return;
	act( "$n ties $p around $s waist.",   ch, obj, NULL, TO_ROOM );
	act( "You tie $p around your waist.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_WAIST );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_WRIST ) )
    {
	if ( get_eq_char( ch, WEAR_WRIST_L ) != NULL
	&&   get_eq_char( ch, WEAR_WRIST_R ) != NULL
	&&   !remove_obj( ch, WEAR_WRIST_L, fReplace )
	&&   !remove_obj( ch, WEAR_WRIST_R, fReplace ) )
	    return;

	if ( get_eq_char( ch, WEAR_WRIST_L ) == NULL)
	{
	    act( "$n slides $s left wrist into $p.",
		ch, obj, NULL, TO_ROOM );
	    act( "You slide your left wrist into $p.",
		ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_WRIST_L );
	    return;
	}
	else if ( get_eq_char( ch, WEAR_WRIST_R ) == NULL)
	{
	    act( "$n slides $s right wrist into $p.",
		ch, obj, NULL, TO_ROOM );
	    act( "You slide your right wrist into $p.",
		ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_WRIST_R );
	    return;
	}
	else
	    send_to_char( "You cannot wear any more on your wrists.\n\r", ch );
	return;
    }
    if ( fReplace )
	send_to_char( "You can't wear, wield or hold that.\n\r", ch );

    return;
}



void do_wear( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Wear, wield, or hold what?\n\r", ch );
	return;
    }

    if (IS_SET(ch->polyaff, POLY_PIG))
    {
	send_to_char( "You can't wear anything while you're a pig!\n\r", ch);
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	OBJ_DATA *obj_next;

	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if (IS_DRAGON(ch) && IS_POLYAFF(ch,POLY_DRAGONFORM) && !IS_SET(obj->quest, QUEST_ARTIFACT)) continue;
	    if (obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) )
		wear_obj( ch, obj, FALSE );
	}
	return;
    }
    else
    {
	if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
	{
	    send_to_char( "You do not have that item.\n\r", ch );
	    return;
	}
	if (IS_DRAGON(ch) && IS_POLYAFF(ch,POLY_DRAGONFORM) && !IS_SET(obj->quest, QUEST_ARTIFACT))
	{
	    stc("You cannot wear anything in this form.\n\r", ch);
	    return;
	}
	wear_obj( ch, obj, TRUE );
    }

    return;
}



void do_remove( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    char arg[MIL];

    one_argument(argument, arg);

    if (IS_EXTRA(ch, EXTRA_ILLUSION))
    {
	stc("You cannot remove your equipment.\n\r", ch);
	return;
    }

    if (arg[0] == '\0')
    {
	stc("Remove what?\n\r", ch);
	return;
    }

    if (!str_cmp(arg, "all"))
    {
	OBJ_DATA *obj_next;

	for (obj = ch->carrying; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;
	    if (obj->wear_loc != WEAR_NONE) remove_obj(ch, obj->wear_loc, TRUE);
	}
	update_style(ch);
	return;
    }
    if ((obj = get_obj_wear(ch, arg)) == NULL)
    {
	stc("You do not have that item.\n\r", ch);
	return;
    }

    remove_obj(ch, obj->wear_loc, TRUE);
    update_style(ch);
    return;
}


void do_grip( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;

    if ((obj = get_eq_char(ch, WEAR_WEAPON)) == NULL)
    {
	stc("You aren't wearing a weapon to grip in both hands.\n\r", ch);
	return;
    }

    if (ch->style[0] == STYLE_TWOHAND)
    {
	ch->style[0] = STYLE_ONEHAND;
	act("You shift $p into one hand.", ch, obj, NULL, TO_CHAR);
	act("$n shifts $p into one hand.", ch, obj, NULL, TO_ROOM);
	return;
    }

    if (get_eq_char(ch, WEAR_HOLD) != NULL)
    {
	stc("You've already got something in your off hand.\n\r", ch);
	return;
    }

    ch->style[0] = STYLE_TWOHAND;
    act("You grip $p in both hands.", ch, obj, NULL, TO_CHAR);
    act("$n grips $p in both hands.", ch, obj, NULL, TO_ROOM);
    return;
}


void do_sacrifice( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    OBJ_DATA *cont;
    char buf[MAX_INPUT_LENGTH];

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char("Sacrifice what?\n\r",ch);
	return;
    }
    if (!str_cmp(arg,"all"))
    {
	if (ch->in_room->contents == NULL)
	{
	    stc("Sacrifice what?\n\r",ch);
	    return;
	}
	for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if (!CAN_WEAR(obj, ITEM_TAKE) || obj->item_type == ITEM_QUEST || obj->item_type == ITEM_MONEY
	     || obj->item_type == ITEM_TREASURE || obj->item_type == ITEM_QUESTCARD || IS_SET(obj->quest, QUEST_ARTIFACT)
	     || IS_SET(obj->extra_flags, ITEM_VANISH) || (obj->questowner != NULL && strlen(obj->questowner) > 1
	     && str_cmp(ch->name,obj->questowner))) continue;
	    if (obj->chobj != NULL && !IS_NPC(obj->chobj) && obj->chobj->pcdata->obj_vnum != 0) continue;
	    if (obj->points > 0 && !IS_NPC(ch) && obj->item_type != ITEM_PAGE)
	    {
		sprintf( buf, "You receive a refund of %li quest points from $p.", obj->points);
		act( buf, ch, obj, NULL, TO_CHAR );
		ch->pcdata->quest += obj->points;
		do_autosave(ch,"");
	    }
	    extract_obj( obj );
	}
	stc("You sacrifice everything you can.\n\r",ch);
	act("$n sacrifices everything $e can.",ch,NULL,NULL,TO_ROOM);
	return;
    }
    obj = get_obj_list( ch, arg, ch->in_room->contents );
    if ( obj == NULL )
    {
	send_to_char( "You can't find it.\n\r", ch );
	return;
    }
    if ( !CAN_WEAR(obj, ITEM_TAKE) || obj->item_type == ITEM_QUEST ||
	 obj->item_type == ITEM_MONEY || obj->item_type == ITEM_TREASURE ||
	 obj->item_type == ITEM_QUESTCARD || IS_SET(obj->quest, QUEST_ARTIFACT) ||
         IS_SET(obj->extra_flags, ITEM_VANISH) || 
	( obj->questowner != NULL && strlen(obj->questowner) > 1 && 
	str_cmp(ch->name,obj->questowner) && obj->item_type != ITEM_CORPSE_NPC))
    {
	act( "You are unable to drain any energy from $p.", ch, obj, 0, TO_CHAR );
	return;
    }
    else if (obj->chobj != NULL && !IS_NPC(obj->chobj) && obj->chobj->pcdata->obj_vnum != 0)
    {
	act( "You are unable to drain any energy from $p.", ch, obj, 0, TO_CHAR );
	return;
    }

    act( "$p disintegrates into a fine powder.", ch, obj, NULL, TO_CHAR );
    act( "$n drains the energy from $p.", ch, obj, NULL, TO_ROOM );
    act( "$p disintegrates into a fine powder.", ch, obj, NULL, TO_ROOM );

    if (obj->item_type == ITEM_CORPSE_NPC || obj->item_type == ITEM_CONTAINER )
    {
	for (cont = obj->contains; cont != NULL; cont = obj_next )
	{
	    obj_next = cont->next_content;
	    if (strlen(cont->questowner) != 0)
	    {
		obj_from_obj(cont);
		obj_to_room(cont, ch->in_room);
	    }
	}
    }

    if (obj->points > 0 && !IS_NPC(ch) && obj->item_type != ITEM_PAGE)
    {
	sprintf( buf, "You receive a refund of %li quest points from $p.", obj->points);
	act( buf, ch, obj, NULL, TO_CHAR );
	ch->pcdata->quest += obj->points;
	do_autosave(ch,"");
    }
    extract_obj( obj );
    do_autosave(ch,"");
    return;
}



void do_quaff( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    long level;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Quaff what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
	send_to_char( "You do not have that potion.\n\r", ch );
	return;
    }

    if ( obj->item_type != ITEM_POTION )
    {
	send_to_char( "You can quaff only potions.\n\r", ch );
	return;
    }

    act( "$n quaffs $p.", ch, obj, NULL, TO_ROOM );
    act( "You quaff $p.", ch, obj, NULL ,TO_CHAR );

    level = obj->value[0];
    if (level < 1) level = 1;
    if (level > MAX_SPELL) level = MAX_SPELL;

    obj_cast_spell( obj->value[1], level, ch, ch, NULL );
    obj_cast_spell( obj->value[2], level, ch, ch, NULL );
    obj_cast_spell( obj->value[3], level, ch, ch, NULL );

    extract_obj( obj );
    if (ch->position == POS_FIGHTING) WAIT_STATE(ch, 6 );
    return;
}



void do_recite( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *scroll = NULL;
    OBJ_DATA *obj = NULL;
    long level;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( ( scroll = get_obj_carry( ch, arg1 ) ) == NULL )
    {
	send_to_char( "You do not have that scroll.\n\r", ch );
	return;
    }

    if ( scroll->item_type != ITEM_SCROLL )
    {
	send_to_char( "You can recite only scrolls.\n\r", ch );
	return;
    }

    if ( arg2[0] == '\0' )
    {
	victim = ch;
    }
    else
    {
	if ( ( victim = get_char_room ( ch, arg2 ) ) == NULL
	&&   ( obj    = get_obj_here  ( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "You can't find it.\n\r", ch );
	    return;
	}
    }

    act( "$n recites $p.", ch, scroll, NULL, TO_ROOM );
    act( "You recite $p.", ch, scroll, NULL, TO_CHAR );

    level = scroll->value[0];
    if (level < 1) level = 1;
    if (level > MAX_SPELL) level = MAX_SPELL;

    obj_cast_spell( scroll->value[1], level, ch, victim, obj );
    obj_cast_spell( scroll->value[2], level, ch, victim, obj );
    obj_cast_spell( scroll->value[3], level, ch, victim, obj );

    extract_obj( scroll );
    if (ch->position == POS_FIGHTING) WAIT_STATE(ch, 6 );
    return;
}



void do_brandish( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    OBJ_DATA *staff;
    long sn;
    long level;

    staff = get_eq_char( ch, WEAR_HOLD );

    if ( staff == NULL )
    {
	send_to_char( "You hold nothing in your hand.\n\r", ch );
	return;
    }

    if ( staff->item_type != ITEM_STAFF )
    {
	send_to_char( "You can brandish only with a staff.\n\r", ch );
	return;
    }

    if ( ( sn = staff->value[3] ) < 0
    ||   sn >= MAX_SKILL
    ||   skill_table[sn].spell_fun == 0 )
    {
	bug( "Do_brandish: bad sn %li.", sn );
	return;
    }

    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );

    if ( staff->value[2] > 0 )
    {
	act( "$n brandishes $p.", ch, staff, NULL, TO_ROOM );
	act( "You brandish $p.",  ch, staff, NULL, TO_CHAR );
	for ( vch = ch->in_room->people; vch; vch = vch_next )
	{
	    vch_next	= vch->next_in_room;

	    switch ( skill_table[sn].target )
	    {
	    default:
		bug( "Do_brandish: bad target for sn %li.", sn );
		return;

	    case TAR_IGNORE:
		if ( vch != ch )
		    continue;
		break;

	    case TAR_CHAR_OFFENSIVE:
		if ( IS_NPC(ch) ? IS_NPC(vch) : !IS_NPC(vch) )
		    continue;
		break;
		
	    case TAR_CHAR_DEFENSIVE:
		if ( IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch) )
		    continue;
		break;

	    case TAR_CHAR_SELF:
		if ( vch != ch )
		    continue;
		break;

            case TAR_DEATH:
                if ( IS_NPC(ch) ? IS_NPC(vch) : !IS_NPC(vch) )
                    continue;
                break; 
	    }

	    level = staff->value[0];
	    if (level < 1) level = 1;
	    if (level > MAX_SPELL) level = MAX_SPELL;

	    obj_cast_spell( staff->value[3], level, ch, vch, NULL );
	}
    }

    if ( --staff->value[2] <= 0 )
    {
	act( "$n's $p blazes bright and is gone.", ch, staff, NULL, TO_ROOM );
	act( "Your $p blazes bright and is gone.", ch, staff, NULL, TO_CHAR );
	extract_obj( staff );
    }

    return;
}



void do_zap( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *wand;
    OBJ_DATA *obj = NULL;
    long level;

    one_argument( argument, arg );
    if ( arg[0] == '\0' && ch->fighting == NULL )
    {
	send_to_char( "Zap whom or what?\n\r", ch );
	return;
    }

    wand = get_eq_char( ch, WEAR_HOLD );

    if ( wand == NULL )
    {
	send_to_char( "You hold nothing in your hand.\n\r", ch );
	return;
    }

    if ( wand->item_type != ITEM_WAND )
    {
	send_to_char( "You can zap only with a wand.\n\r", ch );
	return;
    }

    if ( arg[0] == '\0' )
    {
	if ( ch->fighting != NULL )
	{
	    victim = ch->fighting;
	}
	else
	{
	    send_to_char( "Zap whom or what?\n\r", ch );
	    return;
	}
    }
    else
    {
	if ( ( victim = get_char_room ( ch, arg ) ) == NULL
	&&   ( obj    = get_obj_here  ( ch, arg ) ) == NULL )
	{
	    send_to_char( "You can't find it.\n\r", ch );
	    return;
	}
    }

    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );

    if ( wand->value[2] > 0 )
    {
	if ( victim != NULL )
	{
	    act( "$n zaps $N with $p.", ch, wand, victim, TO_ROOM );
	    act( "You zap $N with $p.", ch, wand, victim, TO_CHAR );
	}
	else
	{
	    act( "$n zaps $P with $p.", ch, wand, obj, TO_ROOM );
	    act( "You zap $P with $p.", ch, wand, obj, TO_CHAR );
	}

	level = wand->value[0];
	if (level < 1) level = 1;
	if (level > MAX_SPELL) level = MAX_SPELL;

	obj_cast_spell( wand->value[3], level, ch, victim, obj );
    }

    if ( --wand->value[2] <= 0 )
    {
	act( "$n's $p explodes into fragments.", ch, wand, NULL, TO_ROOM );
	act( "Your $p explodes into fragments.", ch, wand, NULL, TO_CHAR );
	extract_obj( wand );
    }

    return;
}



/*
 * Shopping commands.
 */
CHAR_DATA *find_keeper( CHAR_DATA *ch )
{
    CHAR_DATA *keeper;
    SHOP_DATA *pShop;
    char buf [MAX_STRING_LENGTH];

    pShop = NULL;
    for ( keeper = ch->in_room->people; keeper; keeper = keeper->next_in_room )
    {
	if ( IS_NPC(keeper) && (pShop = keeper->pIndexData->pShop) != NULL )
	    break;
    }

    if ( pShop == NULL )
    {
	send_to_char( "You can't do that here.\n\r", ch );
	return NULL;
    }

    /*
     * Shop hours.
     */
    if ( time_info.hour < pShop->open_hour )
    {
	strcpy( buf, "Sorry, The shop is closed, please come back later." );
	do_say( keeper, buf );
	return NULL;
    }
    
    if ( time_info.hour > pShop->close_hour )
    {
	strcpy( buf, "Sorry, The shop is closed. Come back tomorrow." );
	do_say( keeper, buf );
	return NULL;
    }

    /*
     * Invisible or hidden people.
     */
    if ( !can_see( keeper, ch ) )
    {
	strcpy( buf, "I don't trade with folks I can't see." );
	do_say( keeper, buf );
	return NULL;
    }

    return keeper;
}



long get_cost( CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy )
{
    SHOP_DATA *pShop;
    long cost;

    if (keeper == NULL || keeper->pIndexData == NULL || obj == NULL ||
        obj->pIndexData == NULL || obj->cost <= 0 || obj->chobj != NULL ||
        IS_SET(obj->extra_flags, ITEM_VANISH) ||
        (pShop = keeper->pIndexData->pShop) == NULL)
	return 0;

    /* These dedicated templates identify old saved conjurations as well as
     * new ones. Ordinary world stock keeps its existing prices. */
    if (obj->pIndexData->vnum == OBJ_VNUM_SHADOW_WEAPON ||
        (obj->pIndexData->vnum == OBJ_VNUM_PROTOPLASM &&
         (obj->item_type == ITEM_PAGE || obj->item_type == ITEM_BOOK ||
          obj->item_type == ITEM_PEN))) return 0;
    long profit = fBuy ? pShop->profit_buy : pShop->profit_sell;
    if (profit <= 0 || obj->cost > LONG_MAX / profit) return 0;

    if ( fBuy )
    {
	cost = obj->cost * pShop->profit_buy  / 100;
    }
    else
    {
	OBJ_DATA *obj2;
	long itype;

	cost = 0;
	for ( itype = 0; itype < MAX_TRADE; itype++ )
	{
	    if ( obj->item_type == pShop->buy_type[itype] )
	    {
		cost = obj->cost * pShop->profit_sell / 100;
		break;
	    }
	}

	for ( obj2 = keeper->carrying; obj2; obj2 = obj2->next_content )
	{
	    if ( obj->pIndexData == obj2->pIndexData )
	    {
		cost = 0;
		break;
	    }
	}
    }

    if (obj->item_type == ITEM_STAFF || obj->item_type == ITEM_WAND) {
        long maximum = obj->value[1], remaining = obj->value[2];
        if (maximum <= 0 || remaining < 0 || remaining > maximum ||
            (remaining > 0 && cost > LONG_MAX / remaining)) return 0;
        cost = cost * remaining / maximum;
    }

    return cost;
}



void do_buy( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Buy what?\n\r", ch );
	return;
    }

    {
	CHAR_DATA *keeper;
	OBJ_DATA *obj;
	long cost;

	if ( ( keeper = find_keeper( ch ) ) == NULL )
	    return;

	obj  = get_obj_carry( keeper, arg );
	cost = get_cost( keeper, obj, TRUE );

	if ( cost <= 0 || !can_see_obj( ch, obj ) )
	{
	    act( "$n tells you 'I don't sell that -- try 'list''.",
		keeper, NULL, ch, TO_VICT );
	    ch->reply = keeper;
	    return;
	}

	if ( ch->gold < cost )
	{
	    act( "$n tells you 'You can't afford to buy $p'.",
		keeper, obj, ch, TO_VICT );
	    ch->reply = keeper;
	    return;
	}
	
if (keeper->gold < 0 || keeper->gold > LONG_MAX - cost) {
    stc("The shop cannot safely accept that payment.\n\r", ch);
    return;
}
        act( "$n buys $p.", ch, obj, NULL, TO_ROOM );
	act( "You buy $p.", ch, obj, NULL, TO_CHAR );
	ch->gold     -= cost;
	keeper->gold += cost;

	if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) )
	    obj = create_object( obj->pIndexData, obj->level );
	else
	    obj_from_char( obj );
	obj_to_char( obj, ch );
	return;
    }
}

void do_list( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    {
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *keeper;
	OBJ_DATA *obj;
	long cost;
	bool found;

	one_argument( argument, arg );

	if ( ( keeper = find_keeper( ch ) ) == NULL )
	    return;

	found = FALSE;
	for ( obj = keeper->carrying; obj; obj = obj->next_content )
	{
	    if ( obj->wear_loc == WEAR_NONE
	    &&   can_see_obj( ch, obj )
	    && ( cost = get_cost( keeper, obj, TRUE ) ) > 0
	    && ( arg[0] == '\0' || is_name( arg, obj->name ) ) )
	    {
		if ( !found )
		{
		    found = TRUE;
		    send_to_char( "[Lv Price] Item\n\r", ch );
		}

		sprintf( buf, "[%2li %5li] %s.\n\r",
		    obj->level, cost, capitalize( obj->short_descr ) );
		send_to_char( buf, ch );
	    }
	}

	if ( !found )
	{
	    if ( arg[0] == '\0' )
		send_to_char( "You can't buy anything here.\n\r", ch );
	    else
		send_to_char( "You can't buy that here.\n\r", ch );
	}
	return;
    }
}



void do_sell( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *keeper;
    OBJ_DATA *obj;
    long cost;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Sell what?\n\r", ch );
	return;
    }
    if ( ( keeper = find_keeper( ch ) ) == NULL )
	return;
    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
	act( "$n tells you 'You don't have that item'.", keeper, NULL, ch, TO_VICT );
	ch->reply = keeper;
	return;
    }
    if ( obj->item_type == ITEM_PILL )
    {
	act( "$n tells you 'I do not want your homemade pills!", keeper, obj, ch, TO_VICT );
        return;
    }
    if ( obj->item_type == ITEM_POTION )
    {
        act( "$n tells you 'I do not want your homemade potions!", keeper, obj, ch, TO_VICT );
        return;
    }
    if ( obj->item_type == ITEM_WAND )
    {
        act( "$n tells you 'I do not want your homemade wands!", keeper, obj, ch, TO_VICT );
        return;
    }
    if ( ( cost = get_cost( keeper, obj, FALSE ) ) <= 0 )
    {
	act( "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT );
	return;
    }


    if (ch->gold < 0 || ch->gold > LONG_MAX - cost) {
        stc("You cannot safely carry that much gold.\n\r", ch);
        return;
    }
    free_string(obj->questowner);
    obj->questowner = str_dup("");
    act( "$n sells $p.", ch, obj, NULL, TO_ROOM );
    sprintf( buf, "You sell $p for %li gold piece%s.", cost, cost == 1 ? "" : "s" );
    act( buf, ch, obj, NULL, TO_CHAR );
    ch->gold     += cost;
    keeper->gold = keeper->gold > cost ? keeper->gold - cost : 0;

    if ( obj->item_type == ITEM_TRASH )
	extract_obj( obj );
    else
    {
	obj_from_char( obj );
	obj_to_char( obj, keeper );
    }

    return;
}



void do_value( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *keeper;
    OBJ_DATA *obj;
    long cost;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Value what?\n\r", ch );
	return;
    }

    if ( ( keeper = find_keeper( ch ) ) == NULL )
	return;

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
	act( "$n tells you 'You don't have that item'.",
	    keeper, NULL, ch, TO_VICT );
	ch->reply = keeper;
	return;
    }

    if ( ( cost = get_cost( keeper, obj, FALSE ) ) <= 0 )
    {
	act( "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT );
	return;
    }

    sprintf( buf, "$n tells you 'I'll give you %li gold coins for $p'.", cost );
    act( buf, keeper, obj, ch, TO_VICT );
    ch->reply = keeper;

    return;
}

void do_activate( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj2;
    CHAR_DATA *victim;
    CHAR_DATA *victim_next;
    CHAR_DATA *mob;
    ROOM_INDEX_DATA *pRoomIndex;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Which item do you wish to use?\n\r", ch );
	return;
    }
    if ( (obj = get_obj_wear(ch,arg1)) == NULL ) 
    {
	if ( ( obj = get_obj_here( ch, arg1 ) ) == NULL )
	{
	    send_to_char( "You can't find that item.\n\r", ch );
	    return;
	}
	/* You should only be able to use nontake items on floor */
	if (CAN_WEAR(obj, ITEM_TAKE) && !IS_DRAGON(ch))
	{
	    send_to_char( "But you are not wearing it!\n\r", ch );
	    return;
	}
    }
    if ( obj == NULL || 
	(!IS_SET(obj->spectype, SITEM_ACTIVATE) && 
	!IS_SET(obj->spectype, SITEM_PRESS) && 
	!IS_SET(obj->spectype, SITEM_PULL) && 
	!IS_SET(obj->spectype, SITEM_TWIST) ) )
    {
	send_to_char( "This item cannot be used in that way.\n\r", ch );
	return;
    }
    if ( IS_SET(obj->spectype, SITEM_TARGET) && arg2[0] == '\0' )
    {
	send_to_char( "Who do you wish to use it on?\n\r", ch );
	return;
    }
    if ( IS_SET(obj->spectype, SITEM_TARGET) )
    {
	if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "Nobody here by that name.\n\r", ch );
	    return;
	}
    }
    else victim = ch;
    if (obj->chpoweruse != NULL && obj->chpoweruse[0] != '\0'
	&& str_cmp(obj->chpoweruse,"(null)") )
	kavitem(obj->chpoweruse,ch,obj,NULL,TO_CHAR);
    if (obj->victpoweruse != NULL && obj->victpoweruse[0] != '\0'
	&& str_cmp(obj->victpoweruse,"(null)") )
	kavitem(obj->victpoweruse,ch,obj,NULL,TO_ROOM);
    if ( IS_SET(obj->spectype, SITEM_SPELL))
    {
	long castlevel = obj->level;
	if (castlevel < 1) castlevel = 1;
	else if (castlevel > 60) castlevel = 60;
	obj_cast_spell( obj->specpower, castlevel, ch, victim, NULL );
	WAIT_STATE(ch,6);
	if ( IS_SET(obj->spectype, SITEM_DELAY1)) WAIT_STATE(ch,6);
	if ( IS_SET(obj->spectype, SITEM_DELAY2)) WAIT_STATE(ch,12);
	return;
    }
    else if ( IS_SET(obj->spectype, SITEM_TRANSPORTER))
    {
    if (obj->chpoweron != NULL && obj->chpoweron[0] != '\0' && str_cmp(obj->chpoweron,"(null)") )
	    kavitem(obj->chpoweron,ch,obj,NULL,TO_CHAR);
    if (obj->victpoweron != NULL && obj->victpoweron[0] != '\0' && str_cmp(obj->victpoweron,"(null)") )
	    kavitem(obj->victpoweron,ch,obj,NULL,TO_ROOM);
	if ( (pRoomIndex = get_room_index(obj->specpower)) == NULL ) return;
	obj->specpower = ch->in_room->vnum;
	char_from_room(ch);
	char_to_room(ch,pRoomIndex);
	do_look(ch,"auto");
    if (obj->chpoweroff != NULL && obj->chpoweroff[0] != '\0' && str_cmp(obj->chpoweroff,"(null)") )
	    kavitem(obj->chpoweroff,ch,obj,NULL,TO_CHAR);
    if (obj->victpoweroff != NULL && obj->victpoweroff[0] != '\0' && str_cmp(obj->victpoweroff,"(null)") )
	    kavitem(obj->victpoweroff,ch,obj,NULL,TO_ROOM);
	if (!IS_SET(obj->quest, QUEST_ARTIFACT) && 
	    (IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL) || IS_SET(ch->in_room->room_flags, ROOM_SAFE))
	    && CAN_WEAR(obj,ITEM_TAKE))
	{
	    send_to_char("A powerful force hurls you from the room.\n\r",ch);
	    act("$n is hurled from the room by a powerful force.",ch,NULL,NULL,TO_ROOM);
	    ch->position = POS_STUNNED;
	    char_from_room(ch);
	    char_to_room(ch, get_room_index(3014));
	    act("$n appears in the room, and falls to the ground stunned.",ch,NULL,NULL,TO_ROOM);
	}
	return;
    }
    else if ( IS_SET(obj->spectype, SITEM_TELEPORTER))
    {
    if (obj->chpoweron != NULL && obj->chpoweron[0] != '\0'
	    && str_cmp(obj->chpoweron,"(null)") )
	    kavitem(obj->chpoweron,ch,obj,NULL,TO_CHAR);
    if (obj->victpoweron != NULL && obj->victpoweron[0] != '\0'
	    && str_cmp(obj->victpoweron,"(null)") )
	    kavitem(obj->victpoweron,ch,obj,NULL,TO_ROOM);
	pRoomIndex     = get_room_index(obj->specpower);
	if ( pRoomIndex == NULL ) return;
	char_from_room(ch);
	char_to_room(ch,pRoomIndex);
	do_look(ch,"auto");
    if (obj->chpoweroff != NULL && obj->chpoweroff[0] != '\0'
	    && str_cmp(obj->chpoweroff,"(null)") )
	    kavitem(obj->chpoweroff,ch,obj,NULL,TO_CHAR);
    if (obj->victpoweroff != NULL && obj->victpoweroff[0] != '\0'
	    && str_cmp(obj->victpoweroff,"(null)") )
	    kavitem(obj->victpoweroff,ch,obj,NULL,TO_ROOM);
	if (!IS_SET(obj->quest, QUEST_ARTIFACT) && 
	    (IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL) || IS_SET(ch->in_room->room_flags, ROOM_SAFE)) &&
	    CAN_WEAR(obj,ITEM_TAKE))
	{
	    send_to_char("A powerful force hurls you from the room.\n\r",ch);
	    act("$n is hurled from the room by a powerful force.",ch,NULL,NULL,TO_ROOM);
	    ch->position = POS_STUNNED;
	    char_from_room(ch);
	    char_to_room(ch,get_room_index(3014));
	    act("$n appears in the room, and falls to the ground stunned.",ch,NULL,NULL,TO_ROOM);
	}
	return;
    }
    else if ( IS_SET(obj->spectype, SITEM_OBJECT))
    {
	if ( get_obj_index(obj->specpower) == NULL ) return;
	obj2 = create_object(get_obj_index(obj->specpower), ch->level);
	if ( CAN_WEAR(obj2,ITEM_TAKE) ) obj_to_char(obj2,ch);
	else                           obj_to_room(obj2,ch->in_room);
    }
    else if ( IS_SET(obj->spectype, SITEM_MOBILE))
    {
	if ( get_mob_index(obj->specpower) == NULL ) return;
	mob = create_mobile(get_mob_index(obj->specpower));
	char_to_room(mob,ch->in_room);
    }
    else if ( IS_SET(obj->spectype, SITEM_ACTION))
    {
	if (IS_NPC(ch)) SET_BIT(ch->act, ACT_COMMANDED);
	interpret(ch,obj->victpoweron);
	if (IS_NPC(ch)) REMOVE_BIT(ch->act, ACT_COMMANDED);
	if (obj->victpoweroff != NULL && str_cmp(obj->victpoweroff,"(null)") &&
    obj->victpoweroff[0] != '\0')
	{
	    for ( victim = char_list; victim != NULL; victim = victim_next )
	    {
		victim_next	= victim->next;
		if ( victim->in_room == NULL ) continue;
		if ( victim == ch ) continue;
		if ( victim->in_room == ch->in_room )
		{
		    if (IS_NPC(victim)) SET_BIT(victim->act, ACT_COMMANDED);
		    interpret(victim,obj->victpoweroff);
		    if (IS_NPC(victim)) REMOVE_BIT(victim->act, ACT_COMMANDED);
		    continue;
		}
	    }
	}
    }
    return;
}


void do_qmake( CHAR_DATA *ch, char *argument )
{
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if (arg[0] == '\0')
    {
	send_to_char("Do you wish to qmake a MACHINE or a CARD?\n\r",ch);
	return;
    }
    if (!str_cmp(arg,"card"))
    {
    	if ( (pObjIndex = get_obj_index( OBJ_VNUM_QUESTCARD )) == NULL)
    	{
	    send_to_char("Missing object, please inform KaVir.\n\r",ch);
	    return;
    	}
        if (ch->in_room == NULL) return;
        obj = create_object(pObjIndex, 0);
        obj_to_char(obj, ch);
	quest_object(ch,obj);
    }
    else if (!str_cmp(arg,"machine"))
    {
    	if ( (pObjIndex = get_obj_index( OBJ_VNUM_QUESTMACHINE )) == NULL)
    	{
	    send_to_char("Missing object, please inform KaVir.\n\r",ch);
	    return;
    	}
        if (ch->in_room == NULL) return;
        obj = create_object(pObjIndex, 0);
        obj_to_room(obj, ch->in_room);
    }
    else
    {
	send_to_char("You can only qmake a MACHINE or a CARD.\n\r",ch);
	return;
    }
    send_to_char("Ok.\n\r",ch);
    return;
}

void do_recharge( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *qobj;
    long count = 0;
    long value = 1;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (IS_NPC(ch)) return;
    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Syntax: recharge <quest card> <quest machine>\n\r", ch );
	return;
    }
    if ( (obj = get_obj_carry(ch,arg1)) == NULL )
    {
	send_to_char( "You are not carrying that object.\n\r", ch );
	return;
    }
    if (obj->item_type != ITEM_QUESTCARD)
    {
	send_to_char( "That is not a quest card.\n\r", ch );
	return;
    }
    if ( (qobj = get_obj_here(ch,arg2)) == NULL )
    {
	send_to_char( "There is nothing for you to recharge it with.\n\r", ch );
	return;
    }
    if (qobj->item_type != ITEM_QUESTMACHINE)
    {
	send_to_char( "That is not a quest machine.\n\r", ch );
	return;
    }
    if (obj->value[0] == -1) count += 1;
    if (obj->value[1] == -1) count += 1;
    if (obj->value[2] == -1) count += 1;
    if (obj->value[3] == -1) count += 1;
    if (count == 4) quest_object( ch, obj );
    else
    {
	send_to_char( "You have not yet completed the current quest.\n\r", ch );
	return;
    }
    act("You place $p into a small slot in $P.",ch,obj,qobj,TO_CHAR);
    act("$n places $p into a small slot in $P.",ch,obj,qobj,TO_ROOM);
    act("$P makes a few clicks and returns $p.",ch,obj,qobj,TO_CHAR);
    act("$P makes a few clicks and returns $p.",ch,obj,qobj,TO_ROOM);
    value = obj->level;
    if (value < 1) value = 1; else if (value > 100) value = 100;

    obj = create_object(get_obj_index( OBJ_VNUM_PROTOPLASM ), 0);
    obj->value[0] = value;
    obj->level = value;
    obj->cost = value*1000;
    obj->item_type = ITEM_QUEST;
    obj_to_char(obj,ch);
    if (obj->questmaker != NULL) free_string(obj->questmaker);
    obj->questmaker = str_dup(ch->name);
    free_string( obj->name );
    obj->name = str_dup( "quest token" );
    free_string( obj->short_descr );
    sprintf(buf,"a %li point quest token",value);
    obj->short_descr = str_dup( buf );
    free_string( obj->description );
    sprintf(buf,"A %li point quest token lies on the floor.",value);
    obj->description = str_dup( buf );
    act("You take $p from $P.",ch,obj,qobj,TO_CHAR);
    act("$n takes $p from $P.",ch,obj,qobj,TO_ROOM);
    ch->pcdata->score[SCORE_NUM_QUEST]++;
    ch->pcdata->score[SCORE_QUEST] += value;
    do_autosave(ch,"");
    return;
}

void quest_object( CHAR_DATA *ch, OBJ_DATA *obj )
{
    static const long quest_selection[] = 
    {
	15103, 5003, 1328,  606, 9201, 9225,  605, 1329,11011, 7221,
	 2276, 5112, 6513, 6517, 6519, 5001, 5005, 5011, 5012, 5013,
	 2902, 1352, 2348, 2361, 3005, 5011, 5012, 5013, 2902, 1352,
	 2348, 2361, 3005,  300,  303,  307, 7216, 1100,  100,30315,
	 5110, 6001, 3050,  301, 5230,30302,  663, 7303, 2915, 2275,
	 8600, 8601, 8602, 8603, 5030, 6010, 1304, 1307, 1332, 1333,
	 1342, 1356, 1361, 2304, 2322, 2331, 2382, 8003, 8005, 5300,
	 5302, 5309, 5310, 5311, 4000,  601,  664,  311, 7203, 7206,
	 1101, 5214, 5223, 5228, 2804, 1612, 5207, 9302, 5301, 5224,
	 7801, 9313, 6304, 2003, 3425, 3423, 608,  1109,30319, 8903,
	 9317, 9307, 4050, 2204, 4100, 3428,  310, 3402, 5319, 2448,
	 6512, 5114,30316, 2106, 8007, 6601, 2333, 3610, 2015, 5022,
	 1394, 2202, 1401, 6005, 1614,  647, 1388, 9311, 3604, 4701,
	30325, 6106, 2003, 7190, 9322, 1384, 3412, 2342, 1374, 2210,
	 2332, 2901, 7200, 7824, 3410, 2013, 1510, 8306, 3414, 2005
    };
    long object;
    if (obj == NULL || obj->item_type != ITEM_QUESTCARD) return;

    object = number_range(0, 149);
    obj->value[0] = quest_selection[object];
    do
    {
	object = number_range(0, 149);
	obj->value[1] = quest_selection[object];
    }
    while (obj->value[1] == obj->value[0]);
    do
    {
	object = number_range(0, 149);
	obj->value[2] = quest_selection[object];
    }
    while ((obj->value[2] == obj->value[1]) || (obj->value[2] == obj->value[0]));
    do
    {
	object = number_range(0, 149);
	obj->value[3] = quest_selection[object];
    }
    while ((obj->value[3] == obj->value[2]) || (obj->value[3] == obj->value[1]) || (obj->value[3] == obj->value[0]));
    return;
}

void do_complete( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *qobj;
    OBJ_DATA *obj;
    OBJ_INDEX_DATA *pObjIndex;
    long count = 0;
    long count2 = 0;
    long val;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Syntax: complete <quest card> <object>\n\r", ch );
	return;
    }

    if ( (qobj = get_obj_carry(ch,arg1)) == NULL )
    {
	send_to_char( "You are not carrying that object.\n\r", ch );
	return;
    }
    else if (qobj->item_type != ITEM_QUESTCARD)
    {
	send_to_char( "That is not a quest card.\n\r", ch );
	return;
    }
    if (qobj->value[0] == -1) count++;
    if (qobj->value[1] == -1) count++;
    if (qobj->value[2] == -1) count++;
    if (qobj->value[3] == -1) count++;

    if (count == 4)
    {
	send_to_char("This quest card has been completed.\n\r",ch);
	return;
    }
    if ( arg2[0] == '\0' )
    {
	send_to_char("You still need to find the following:\n\r",ch);
	for (val = 0; val < 4; val++)
	{
	    if (qobj->value[val] != -1)
	    {
		pObjIndex = get_obj_index( qobj->value[val] );
		if ( pObjIndex != NULL )
		{
		    sprintf(buf,"     %s.\n\r",pObjIndex->short_descr);
		    stc(buf,ch);
		}
		else qobj->value[val] = -1;
	    }
	}
	return;
    }
    if ( (obj = get_obj_carry(ch,arg2)) == NULL )
    {
	send_to_char( "You are not carrying that object.\n\r", ch );
	return;
    }
    if (obj->questmaker != NULL && strlen(obj->questmaker) > 1)
    {
	send_to_char( "You cannot use that item.\n\r", ch );
	return;
    }
    if (obj->pIndexData->vnum == 30037 || obj->pIndexData->vnum == 30041)
    {
	send_to_char( "That item has lost its quest value, you must collect a new one.\n\r", ch );
	return;
    }
    if (IS_SET(obj->extra_flags, ITEM_VANISH))
    {
        send_to_char( "That is not a real item.\n\r", ch );
        return;
    }
    for (val = 0; val < 4; val++)
    {
	if (qobj->value[val] != -1)
	{
	    pObjIndex = get_obj_index( qobj->value[val] );
	    if ( pObjIndex != NULL && !str_cmp(obj->short_descr, pObjIndex->short_descr))
		qobj->value[val] = -1;
	}
	if (qobj->value[val] == -1) count2++;
    }
    if (count == count2)
    {
	send_to_char( "That item is not required.\n\r", ch );
	return;
    }

    act("You touch $p to $P, and $p vanishes!",ch,obj,qobj,TO_CHAR);
    act("$n touches $p to $P, and $p vanishes!",ch,obj,qobj,TO_ROOM);
    obj_from_char(obj);
    extract_obj(obj);
    if (count2 == 4) act("$p has been completed!",ch,qobj,NULL,TO_CHAR);
    else if (count2 == 3) act("$p now requires one more object!",ch,qobj,NULL,TO_CHAR);
    else if (count2 == 2) act("$p now requires two more objects!",ch,qobj,NULL,TO_CHAR);
    else if (count2 == 1) act("$p now requires three more objects!",ch,qobj,NULL,TO_CHAR);
    return;
}

bool objects_in_room( CHAR_DATA *ch )
{
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    long obj_count = 0;

    if (ch->in_room == NULL) return TRUE;

    for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
    {
	obj_next = obj->next_content;
	obj_count++;
	if (obj_count > 25)
	{
	    send_to_char("You are unable to drop anything else here.\n\r",ch);
	    return TRUE;
	}
    }
    return FALSE;
}

bool objects_in_container( CHAR_DATA *ch, OBJ_DATA *container )
{
    OBJ_DATA *obj;
    long obj_count = 0;

    for ( obj = container->contains; obj != NULL; obj = obj->next_content )
    {
	obj_count++;
	if (obj_count > 40)
	{
	    act("You cannot fit anything else into $p.",ch,container,NULL,TO_CHAR);
	    return TRUE;
	}
    }
    return FALSE;
}

bool objects_in_hand( CHAR_DATA *ch )
{
    OBJ_DATA *obj;
    long obj_count = 0;

    for (obj = ch->carrying; obj != NULL; obj= obj->next_content)
    {
	if (obj->wear_loc == WEAR_NONE) continue;
	obj_count++;
	if (obj_count > 30)
	{
	    stc("You can't carry any more.\n\r", ch);
	    return TRUE;
	}
    }
    return FALSE;
}

void mortal_vamp( CHAR_DATA *ch )
{
    long       disc;

    if (!IS_VAMPIRE(ch)) return;
	for ( disc = DISC_ANIMALISM; disc <= DISC_MAX; disc++ )
	{
	    ch->pcdata->powers[disc] += 20;
	}
    	if (IS_VAMPAFF(ch,VAM_EARTHMELDED)) do_earthmeld(ch,"");
	if (IS_MORE(ch, MORE_MUMMIFIED)) do_mummify(ch,"");
	if (IS_MORE(ch, MORE_PLASMA)) do_plasmaform(ch,"");
	if (IS_EXTRA(ch, EXTRA_AWE)) do_awe(ch,"");
	if (IS_MORE(ch, MORE_GUARDIAN)) do_spiritguardian(ch,"");
	if (IS_ITEMAFF(ch, ITEMA_PEACE)) do_majesty(ch,"");
	do_unveil(ch,"self");
    	if (IS_POLYAFF(ch,POLY_ZULO)) do_zuloform(ch,"");
    	if (IS_POLYAFF(ch,POLY_CHIRO)) do_chiroptean(ch,"");
	if (IS_VAMPAFF(ch,VAM_CHANGED)) do_change(ch,"human");
    	if (IS_POLYAFF(ch,POLY_SERPENT)) do_serpent(ch,"");
    	if (IS_POLYAFF(ch,POLY_SHADOW)) do_shadowbody(ch,"");
    	if (IS_POLYAFF(ch,POLY_MASK)) do_mask(ch,"self");
    	if (IS_POLYAFF(ch,POLY_TRUE)) do_truedisguise(ch,"self");
	if (IS_CRAFT(ch, CRAFT_MALLEABLE)) do_malleable(ch, "self");
    	if (IS_MORE(ch,MORE_SHIELDED)) do_shield(ch,"");
    	if (IS_VAMPAFF(ch,VAM_CLAWS)) do_claws(ch,"");
    	if (IS_VAMPAFF(ch,VAM_SPIRITEYES)) do_spiriteyes(ch,"");
    	if (IS_VAMPAFF(ch,VAM_NIGHTSIGHT)) do_nightsight(ch,"");
    	if (IS_SET(ch->act,PLR_HOLYLIGHT)) do_truesight(ch,"");
    	if (IS_SET(ch->act,PLR_WIZINVIS)) do_vanish(ch,"");
	if (IS_MORE(ch, MORE_SILENCE)) do_silencedeath(ch,"");
	if (IS_MORE(ch, MORE_DARKNESS)) do_shroudnight(ch,"");
	for ( disc = DISC_ANIMALISM; disc <= DISC_MAX; disc++ )
	{
	    ch->pcdata->powers[disc] -= 20;
	}
    return;
}

