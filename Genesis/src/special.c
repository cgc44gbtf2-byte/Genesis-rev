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



/*
 * The following special functions are available for mobiles.
 */
DECLARE_SPEC_FUN(	spec_breath_any		);
DECLARE_SPEC_FUN(	spec_breath_acid	);
DECLARE_SPEC_FUN(	spec_breath_fire	);
DECLARE_SPEC_FUN(	spec_breath_frost	);
DECLARE_SPEC_FUN(	spec_breath_gas		);
DECLARE_SPEC_FUN(	spec_breath_lightning	);
DECLARE_SPEC_FUN(	spec_cast_adept		);
DECLARE_SPEC_FUN(	spec_cast_cleric	);
DECLARE_SPEC_FUN(	spec_cast_judge		);
DECLARE_SPEC_FUN(	spec_cast_mage		);
DECLARE_SPEC_FUN(	spec_cast_undead	);
DECLARE_SPEC_FUN(	spec_fido		);
DECLARE_SPEC_FUN(	spec_guard		);
DECLARE_SPEC_FUN(       spec_guard2             );
DECLARE_SPEC_FUN(	spec_janitor		);
DECLARE_SPEC_FUN(	spec_mayor		);
DECLARE_SPEC_FUN(	spec_poison		);
DECLARE_SPEC_FUN(	spec_thief		);
DECLARE_SPEC_FUN(	spec_eater		);
DECLARE_SPEC_FUN(	spec_rogue		);
DECLARE_SPEC_FUN(       spec_isha_breg          );


/*
 * Given a name, return the appropriate spec fun.
 */
SPEC_FUN *spec_lookup( const char *name )
{
    if ( !str_cmp( name, "spec_breath_any"	  ) ) return spec_breath_any;
    if ( !str_cmp( name, "spec_breath_acid"	  ) ) return spec_breath_acid;
    if ( !str_cmp( name, "spec_breath_fire"	  ) ) return spec_breath_fire;
    if ( !str_cmp( name, "spec_breath_frost"	  ) ) return spec_breath_frost;
    if ( !str_cmp( name, "spec_breath_gas"	  ) ) return spec_breath_gas;
    if ( !str_cmp( name, "spec_breath_lightning"  ) ) return spec_breath_lightning;
    if ( !str_cmp( name, "spec_cast_adept"	  ) ) return spec_cast_adept;
    if ( !str_cmp( name, "spec_cast_cleric"	  ) ) return spec_cast_cleric;
    if ( !str_cmp( name, "spec_cast_judge"	  ) ) return spec_cast_judge;
    if ( !str_cmp( name, "spec_cast_mage"	  ) ) return spec_cast_mage;
    if ( !str_cmp( name, "spec_cast_undead"	  ) ) return spec_cast_undead;
    if ( !str_cmp( name, "spec_fido"		  ) ) return spec_fido;
    if ( !str_cmp( name, "spec_guard"		  ) ) return spec_guard;
    if ( !str_cmp( name, "spec_guard2"            ) ) return spec_guard2;
    if ( !str_cmp( name, "spec_janitor"		  ) ) return spec_janitor;
    if ( !str_cmp( name, "spec_mayor"		  ) ) return spec_mayor;
    if ( !str_cmp( name, "spec_poison"		  ) ) return spec_poison;
    if ( !str_cmp( name, "spec_thief"		  ) ) return spec_thief;
    if ( !str_cmp( name, "spec_eater"		  ) ) return spec_eater;
    if ( !str_cmp( name, "spec_rogue"		  ) ) return spec_rogue;
    if ( !str_cmp( name, "spec_isha_breg"         ) ) return spec_isha_breg; 
    return 0;
}



/*
 * Core procedure for dragons.
 */
bool dragon( CHAR_DATA *ch, char *spell_name )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    long sn;

    if ( ch->position != POS_FIGHTING )
	return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim->fighting == ch && number_bits( 2 ) == 0 )
	    break;
    }

    if ( victim == NULL )
	return FALSE;

    if ( ( sn = skill_lookup( spell_name ) ) < 0 )
	return FALSE;
    (*skill_table[sn].spell_fun) ( sn, ch->level / 25, ch, victim );
    return TRUE;
}



/*
 * Special procedures for mobiles.
 */
bool spec_breath_any( CHAR_DATA *ch )
{
    if ( ch->position != POS_FIGHTING )
	return FALSE;

    switch ( number_bits( 3 ) )
    {
    case 0: return spec_breath_fire		( ch );
    case 1:
    case 2: return spec_breath_lightning	( ch );
    case 3: return spec_breath_gas		( ch );
    case 4: return spec_breath_acid		( ch );
    case 5:
    case 6:
    case 7: return spec_breath_frost		( ch );
    }

    return FALSE;
}



bool spec_breath_acid( CHAR_DATA *ch )
{
    return dragon( ch, "acid breath" );
}



bool spec_breath_fire( CHAR_DATA *ch )
{
    return dragon( ch, "fire breath" );
}



bool spec_breath_frost( CHAR_DATA *ch )
{
    return dragon( ch, "frost breath" );
}



bool spec_breath_gas( CHAR_DATA *ch )
{
    long sn;

    if ( ch->position != POS_FIGHTING )
	return FALSE;

    if ( ( sn = skill_lookup( "gas breath" ) ) < 0 )
	return FALSE;
    (*skill_table[sn].spell_fun) ( sn, ch->level / 25, ch, NULL );
    return TRUE;
}



bool spec_breath_lightning( CHAR_DATA *ch )
{
    return dragon( ch, "lightning breath" );
}



bool spec_cast_adept( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;

    if ( !IS_AWAKE(ch) )
	return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim != ch && can_see( ch, victim ) && number_bits( 1 ) == 0 )
	    break;
    }

    if ( victim == NULL )
	return FALSE;

    switch ( number_bits( 3 ) )
    {
    case 0:
	act( "$n utters the word 'loricatus'.", ch, NULL, NULL, TO_ROOM );
	spell_armor( skill_lookup( "armor" ), ch->level / 25, ch, victim );
	return TRUE;

    case 1:
	act( "$n utters the word 'beatus'.", ch, NULL, NULL, TO_ROOM );
	spell_bless( skill_lookup( "bless" ), ch->level / 25, ch, victim );
	return TRUE;

    case 2:
	act( "$n utters the words 'rememdium caecus'.", ch, NULL, NULL, TO_ROOM );
	spell_cure_blindness( skill_lookup( "cure blindness" ), ch->level / 25, ch, victim );
	return TRUE;

    case 3:
	act( "$n utters the words 'rememdium minimus'.", ch, NULL, NULL, TO_ROOM );
	spell_cure_light( skill_lookup( "cure light" ), ch->level / 25, ch, victim );
	return TRUE;

    case 4:
	act( "$n utters the words 'rememdium venenum'.", ch, NULL, NULL, TO_ROOM );
	spell_cure_poison( skill_lookup( "cure poison" ), ch->level / 25, ch, victim );
	return TRUE;

    case 5:
	act( "$n utters the word 'novo'.", ch, NULL, NULL, TO_ROOM );
	spell_refresh( skill_lookup( "refresh" ), ch->level / 25, ch, victim );
	return TRUE;

    }

    return FALSE;
}



bool spec_cast_cleric( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell;
    long sn;

    if ( ch->position != POS_FIGHTING )
	return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim->fighting == ch && number_bits( 2 ) == 0 )
	    break;
    }

    if ( victim == NULL )
	return FALSE;


    switch (number_bits(3))
    {
	case 0: case 1:	spell = "cause serious";	break;
	case 2: case 3:	spell = "earthquake";		break;
	case 4:		spell = "cause critical";	break;
	case 5:		spell = "flamestrike";		break;
	case 6:		spell = "harm";			break;
	case 7:		spell = "dispel magic";		break;
    }

    if ( ( sn = skill_lookup( spell ) ) < 0 )
	return FALSE;
    (*skill_table[sn].spell_fun) ( sn, ch->level / 25, ch, victim );
    return TRUE;
}



bool spec_cast_judge( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell;
    long sn;

    if ( ch->position != POS_FIGHTING )
	return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim->fighting == ch && number_bits( 2 ) == 0 )
	    break;
    }

    if ( victim == NULL )
	return FALSE;

    spell = "high explosive";
    if ( ( sn = skill_lookup( spell ) ) < 0 )
	return FALSE;
    (*skill_table[sn].spell_fun) ( sn, ch->level / 25, ch, victim );
    return TRUE;
}



bool spec_cast_mage( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell;
    long sn;

    if (ch->position != POS_FIGHTING) return FALSE;

    for (victim = ch->in_room->people; victim != NULL; victim = v_next)
    {
	v_next = victim->next_in_room;
	if (victim->fighting == ch && number_bits(2) == 0) break;
    }

    if (victim == NULL) return FALSE;

    switch (number_bits(2))
    {
	case 0:	spell = "chill touch";	break;
	case 1:	spell = "colour spray";	break;
	case 2:	spell = "fireball";	break;
	case 3:	spell = "acid blast";	break;
    }

    if ((sn = skill_lookup(spell)) < 0) return FALSE;
    (*skill_table[sn].spell_fun) (sn, ch->level / 25, ch, victim);
    return TRUE;
}



bool spec_cast_undead( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell;
    long sn;

    if ( ch->position != POS_FIGHTING )
	return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim->fighting == ch && number_bits( 2 ) == 0 )
	    break;
    }

    if ( victim == NULL )
	return FALSE;

    for ( ;; )
    {
	long min_level;

	switch ( number_bits( 4 ) )
	{
	case  0: min_level =  0; spell = "curse";          break;
	case  1: min_level =  3; spell = "weaken";         break;
	case  2: min_level =  6; spell = "chill touch";    break;
	case  3: min_level =  9; spell = "blindness";      break;
	case  4: min_level = 12; spell = "poison";         break;
	case  5: min_level = 15; spell = "energy drain";   break;
	case  6: min_level = 18; spell = "harm";           break;
	case  7: min_level = 21; spell = "teleport";       break;
	default: min_level = 24; spell = "gate";           break;
	}

	if ( ch->level >= min_level )
	    break;
    }

    if ( ( sn = skill_lookup( spell ) ) < 0 )
	return FALSE;
    (*skill_table[sn].spell_fun) ( sn, ch->level / 25, ch, victim );
    return TRUE;
}



bool spec_fido( CHAR_DATA *ch )
{
    OBJ_DATA *corpse;
    OBJ_DATA *c_next;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    if ( !IS_AWAKE(ch) )
	return FALSE;

    for ( corpse = ch->in_room->contents; corpse != NULL; corpse = c_next )
    {
	c_next = corpse->next_content;
	if ( corpse->item_type != ITEM_CORPSE_NPC )
	    continue;

	act( "$n savagely devours a corpse.", ch, NULL, NULL, TO_ROOM );
	for ( obj = corpse->contains; obj; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    obj_from_obj( obj );
	    obj_to_room( obj, ch->in_room );
	}
	if (corpse != NULL) extract_obj( corpse );
	return TRUE;
    }

    return FALSE;
}



bool spec_guard( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    if (!IS_AWAKE(ch) || ch->fighting != NULL)
	return FALSE;

    for (victim = ch->in_room->people; victim != NULL; victim = victim->next_in_room)
    {
	if (IS_NPC(victim)) continue;
	if (victim->fighting == NULL) continue;
	if (number_percent() < 75) continue;
	if (!can_see(ch, victim)) continue;

	break;
    }

    if (victim != NULL)
    {
	if (IS_EXTRA(victim, EXTRA_AWE))
	{
	    sprintf(buf, "How DARE you attack %s? You shall DIE!", victim->name);
	    do_say(ch, buf);
	    do_rescue(ch, victim->name);
	}
	else
	{
	    act("$n screams 'PROTECT THE INNOCENT!!  BANZAI!!", ch, NULL, NULL, TO_ROOM);
	    multi_hit( ch, victim );
	}
	return TRUE;
    }
    return FALSE;
}


bool spec_guard2( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    CHAR_DATA *ech;

    if (!IS_AWAKE(ch) || ch->fighting != NULL)
	return FALSE;

    ech = NULL;

    for (victim = ch->in_room->people; victim != NULL; victim = v_next)
    {
	v_next = victim->next_in_room;

	if (!IS_NPC(victim) && IS_EXTRA(victim, EXTRA_AWE) && number_percent() > 95 && can_see(ch, victim))
	    break;

	if (victim->fighting != NULL && victim->fighting != ch)
	    ech = victim;
    }

    if (ech != NULL)
    {
	if (!IS_NPC(ech) && IS_EXTRA(ech, EXTRA_AWE))
	{
	    sprintf(buf, "You dare to attack %s! You shall suffer and die!", ech->name);
	    do_say(ch, buf);
	    do_rescue(ch, ech->name);
	}
	else
	{
	    act("$n shouts 'This looks like fun, ATTACK!!'", ch, NULL, NULL, TO_ROOM);
	    multi_hit(ch, ech );
	}
	return TRUE;
    }
    return FALSE;
}


bool spec_janitor( CHAR_DATA *ch )
{
    OBJ_DATA *trash;
    OBJ_DATA *trash_next;

    if ( !IS_AWAKE(ch) )
	return FALSE;

    for ( trash = ch->in_room->contents; trash != NULL; trash = trash_next )
    {
	trash_next = trash->next_content;
	if ( !IS_SET( trash->wear_flags, ITEM_TAKE ) )
	    continue;
	if (( trash->item_type == ITEM_DRINK_CON
	||   trash->item_type == ITEM_TRASH
	||   trash->cost < 10) && trash->pIndexData->vnum != OBJ_VNUM_SEVERED_HEAD )
	{
	    act( "$n picks up some trash.", ch, NULL, NULL, TO_ROOM );
	    obj_from_room( trash );
	    obj_to_char( trash, ch );
	    return TRUE;
	}
    }

    return FALSE;
}



bool spec_mayor( CHAR_DATA *ch )
{
    static const char open_path[] =
	"W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";

    static const char close_path[] =
	"W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";

    static const char *path;
    static long pos;
    static bool move;

    if ( !move )
    {
	if ( time_info.hour ==  6 )
	{
	    path = open_path;
	    move = TRUE;
	    pos  = 0;
	}

	if ( time_info.hour == 20 )
	{
	    path = close_path;
	    move = TRUE;
	    pos  = 0;
	}
    }

    if ( ch->fighting != NULL )
	return spec_cast_cleric( ch );
    if ( !move || ch->position < POS_SLEEPING )
	return FALSE;

    switch ( path[pos] )
    {
    case '0':
    case '1':
    case '2':
    case '3':
	move_char( ch, path[pos] - '0' );
	break;

    case 'W':
	ch->position = POS_STANDING;
	act( "$n awakens and groans loudly.", ch, NULL, NULL, TO_ROOM );
	break;

    case 'S':
	ch->position = POS_SLEEPING;
	act( "$n lies down and falls asleep.", ch, NULL, NULL, TO_ROOM );
	break;

    case 'a':
	act( "$n says 'Hello Honey!'", ch, NULL, NULL, TO_ROOM );
	break;

    case 'b':
	act( "$n says 'What a view!  I must do something about that dump!'",
	    ch, NULL, NULL, TO_ROOM );
	break;

    case 'c':
	act( "$n says 'Vandals!  Youngsters have no respect for anything!'",
	    ch, NULL, NULL, TO_ROOM );
	break;

    case 'd':
	act( "$n says 'Good day, citizens!'", ch, NULL, NULL, TO_ROOM );
	break;

    case 'e':
	act( "$n says 'I hereby declare the city of Midgaard open!'",
	    ch, NULL, NULL, TO_ROOM );
	break;

    case 'E':
	act( "$n says 'I hereby declare the city of Midgaard closed!'",
	    ch, NULL, NULL, TO_ROOM );
	break;

    case 'O':
	do_unlock( ch, "gate" );
	do_open( ch, "gate" );
	break;

    case 'C':
	do_close( ch, "gate" );
	do_lock( ch, "gate" );
	break;

    case '.' :
	move = FALSE;
	break;
    }

    pos++;
    return FALSE;
}



bool spec_poison( CHAR_DATA *ch )
{
    CHAR_DATA *victim;

    if ( ch->position != POS_FIGHTING
    || ( victim = ch->fighting ) == NULL
    ||   number_percent( ) > (ch->level / 25) )
	return FALSE;

    act( "You bite $N!",  ch, NULL, victim, TO_CHAR    );
    act( "$n bites $N!",  ch, NULL, victim, TO_NOTVICT );
    act( "$n bites you!", ch, NULL, victim, TO_VICT    );
    spell_poison( gsn_poison, ch->level / 25, ch, victim );
    return TRUE;
}



bool spec_thief( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    long gold;

    if ( ch->position != POS_STANDING )
	return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;

	if ( IS_NPC(victim)
	||   IS_BUI(victim)
	||   number_bits( 2 ) != 0
	||   !can_see( ch, victim ) )	/* Thx Glop */
	    continue;

	if ( IS_AWAKE(victim) && number_range( 0, ch->level ) == 0 )
	{
	    act( "You discover $n's hands in your wallet!",
		ch, NULL, victim, TO_VICT );
	    act( "$N discovers $n's hands in $S wallet!",
		ch, NULL, victim, TO_NOTVICT );
	    return TRUE;
	}
	else
	{
	    gold = victim->gold * number_range( 1, 20 ) / 100;
	    ch->gold     += 7 * gold / 8;
	    victim->gold -= gold;
	    return TRUE;
	}
    }

    return FALSE;
}

bool spec_eater( CHAR_DATA *ch )
{
  /* The spec_eater is a hungry bugger who eats players.  If they get 
   * eaten, they get transported to the room with the same vnum as the 
   * mob Example:  A spec_eater dragon with the vnum 31305 would send 
   * anybody eaten to room 31305.
   * KaVir.
   */
   CHAR_DATA       *victim;
   ROOM_INDEX_DATA *pRoomIndex;
   if ( ch->position != POS_FIGHTING )
   {
      return FALSE;
   }
   if (number_percent() > 50 ) return FALSE;
   victim=ch->fighting;
   act( "$n stares at $N hungrily and licks $s lips!",  ch, NULL, victim, TO_NOTVICT    );
   act( "$n stares at you hungrily and licks $s lips!",  ch, NULL, victim, TO_VICT    );
   if (number_percent() > 25 ) return FALSE;
   pRoomIndex = get_room_index(ch->pIndexData->vnum);
   act( "$n opens $s mouth wide and lunges at you!", ch, NULL, victim, TO_VICT    );
   act( "$n swallows you whole!", ch, NULL, victim, TO_VICT    );
   act( "$n opens $s mouth wide and lunges at $N!",  ch, NULL, victim, TO_NOTVICT );
   act( "$n swallows $N whole!",  ch, NULL, victim, TO_NOTVICT );
   char_from_room(victim);
   char_to_room(victim,pRoomIndex);
   do_emote(ch,"burps loudly.");
   do_look(victim,"auto");
   return TRUE;
}

/* Spec_rogue, coded by Malice. */
bool spec_rogue( CHAR_DATA *ch )
{
/* To add to the life of mobs... they pickup and wear equipment */
    OBJ_DATA *object;
    OBJ_DATA *obj2;
    OBJ_DATA *object_next;
    char buf [MAX_STRING_LENGTH];
    bool drop_out = FALSE;

    if ( !IS_AWAKE( ch ) )
	return FALSE;

    for ( object = ch->in_room->contents; object; object = object_next )
    {
        object_next = object->next_content;
        if ( object == NULL )
	    continue;

	if ( !can_see_obj( ch, object ))
	    continue;
	    
	if ( !IS_SET( object->wear_flags, ITEM_TAKE ) )
	    continue;

	if ( object->item_type == ITEM_CORPSE_NPC )
	    continue;
	    
	if ( object->item_type == ITEM_CORPSE_PC )
	    continue;
	    
	if ( ( object->item_type != ITEM_DRINK_CON
	    && object->item_type != ITEM_TRASH ))
	    {
		act( "$n picks up $p and examines it carefully.", ch, object, NULL, TO_ROOM );
		obj_from_room( object );
		obj_to_char( object, ch );
		/*Now compare it to what we already have*/
		for ( obj2 = ch->carrying; obj2 != NULL; obj2 = obj2->next_content )
		{
		    if ( obj2->wear_loc != WEAR_NONE
			&& can_see_obj( ch, obj2 )
			&& object->item_type == obj2->item_type
			&& ( object->wear_flags & obj2->wear_flags & ~ITEM_TAKE) != 0 )
			break;
		}
		if (!obj2)
		{
		    switch (object->item_type)
		    {
			default:
			    strcpy(buf,"Hey, what a find!");
			    do_say(ch,buf);
			    break;
                        case ITEM_PILL:
                            strcpy(buf, "Hrm,  always wanted a pill!");
                            do_say(ch,buf);
                            do_eat(ch,object->name);
                            break;
			case ITEM_FOOD:
			    strcpy(buf, "This looks like a tasty morsel!");
			    do_say(ch,buf);
			    do_eat(ch,object->name);
			    drop_out = TRUE;
			    break;
			case ITEM_WAND:
			    strcpy(buf,"Wow, a magic wand!");
			    do_say(ch,buf);
			    wear_obj(ch,object,FALSE);
			    break;
			case ITEM_STAFF:
			    strcpy(buf,"Kewl, a magic staff!");
			    do_say(ch,buf);
			    wear_obj(ch,object,FALSE);
			    break;
			case ITEM_WEAPON:
			    strcpy(buf,"Hey, this looks like a nifty weapon!");
			    do_say(ch,buf);
			    wear_obj(ch,object,FALSE);
			    break;
			case ITEM_ARMOR:
			    strcpy(buf,"Oooh...a nice piece of armor!");
			    do_say(ch,buf);
			    wear_obj(ch,object,FALSE);
			    break;
			case ITEM_POTION:
			    strcpy(buf, "Great!  I was feeling a little thirsty!");
			    do_say(ch,buf);
			    act( "You quaff $p.", ch, object, NULL ,TO_CHAR );
			    act( "$n quaffs $p.", ch, object, NULL, TO_ROOM );
			    obj_cast_spell( object->value[1], object->level, ch, ch, NULL );
 			    obj_cast_spell( object->value[2], object->level, ch, ch, NULL );
			    obj_cast_spell( object->value[3], object->level, ch, ch, NULL );
			    extract_obj( object );
			    drop_out = TRUE;
			    break;
			case ITEM_SCROLL:
			    strcpy(buf,"Hmmm I wonder what this says?");
			    do_say(ch,buf);
			    act( "You recite $p.", ch, object, NULL, TO_CHAR );
			    act( "$n recites $p.", ch, object, NULL, TO_ROOM );
			    obj_cast_spell( object->value[1], object->level, ch, NULL, object );
			    obj_cast_spell( object->value[2], object->level, ch, NULL, object );
			    obj_cast_spell( object->value[3], object->level, ch, NULL, object );
			    extract_obj( object );
			    drop_out = TRUE;
			    break;
		    }
		    return TRUE;
		}
		
	    if (drop_out) return TRUE;
	    if ((object->level > obj2->level))
	    {
	    	strcpy(buf,"Now THIS looks like an improvement!");
		do_say(ch,buf);
	    	remove_obj(ch,obj2->wear_loc,TRUE);
	    	wear_obj(ch,object,FALSE);
	    }
	    else
	    {
	    	strcpy(buf,"I don't want this piece of junk!");
		do_say(ch,buf);
	    	act("You don't like the look of $p.",ch,object,NULL,TO_CHAR);
			do_drop(ch,object->name);
			do_sacrifice(ch,object->name);
		}
	    return TRUE;
	}
    }

    return FALSE;
}

bool spec_isha_breg ( CHAR_DATA *ch )
{
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;
    char buf[MSL];

    if (ch->fighting) return FALSE;

    for (d = descriptor_list; d != NULL; d = d->next)
    {
	if (!IS_PLAYING(d) || (victim = d->character) == NULL || IS_NPC(victim) || IS_BUI(victim)
	 || victim->in_room != ch->in_room || victim->pcdata->chobj != NULL)
	    continue;

	if (victim->position <= POS_INCAP && ch->position == POS_STANDING)
	{
	    victim->mdeath++;

	    char_from_room(victim);
	    char_to_room(victim, get_room_index(ROOM_VNUM_ALTAR));

	    behead(victim, ch);
	    if (!str_cmp(ch->name,"Revenant"))
		sprintf(buf, "#r%s#n strings #0%s#n from the ceiling and tears out their intestines.",ch->name,victim->name);	
	    else
		sprintf(buf, "#r%s#n has been decapitated by #r%s#n.", victim->name, ch->name);
	    do_fatality(ch, buf);
	    if (!str_cmp(ch->name,"Revenant")) sprintf(buf, "#0Die bitch!! Die!!");
	    else sprintf(buf, "#G%s you are a worm!", victim->name);
	    do_shout(ch, buf);
	    continue;
        }
	multi_hit(ch, victim);
	return TRUE;
   }
   return FALSE;
}
