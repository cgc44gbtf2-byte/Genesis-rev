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
 * Start fights.
 */
void set_fighting( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if (auto_safe(ch, victim)) return;

    if (ch->fighting != NULL)
    {
	bug( "Set_fighting: already fighting", 0 );
	return;
    }

    if (!IS_NPC(ch) && !IS_NPC(victim))
    {
	ch->fight_timer = 30;
	victim->fight_timer = 30;
    }
    
    if ( ch->next_fight != NULL || ch->prev_fight != NULL )
    {
	bug( "Set_fighting: new fight code", 0 );
	return;
    }

    autostance(ch);
    autostance(victim);
    if (IS_AFFECTED(ch, AFF_SLEEP))
	affect_strip(ch, gsn_sleep);

    ch->fighting = victim;
    ch->position = POS_FIGHTING;

    if (first_fight == NULL)
	first_fight = ch;

    if (last_fight != NULL)
    {
	last_fight->next_fight = ch;
	ch->prev_fight = last_fight;
    }
    last_fight = ch;

    if (IS_MAGE(victim) && str_cmp(victim->contingency, "")) do_contingency(victim, "");
    if (IS_MAGE(ch) && str_cmp(ch->contingency, "")) do_contingency(ch, "");

    return;
}

/*
 * Stop fights.
 */
void stop_fighting( CHAR_DATA *ch, bool fBoth )
{
    CHAR_DATA *fch;
    CHAR_DATA *fch_next;

    for (fch = first_fight; fch != NULL; fch = fch_next)
    {
	fch_next = fch->next_fight;

	if (fch == ch || (fBoth && fch->fighting == ch))
	{
	    fch->fighting	= NULL;
	    fch->position	= POS_STANDING;

	    if (fch->prev_fight != NULL && fch->next_fight != NULL)
	    {
		fch->prev_fight->next_fight = fch->next_fight;
		fch->next_fight->prev_fight = fch->prev_fight;
	    }
	    else if (fch->prev_fight != NULL)
	    {
		fch->prev_fight->next_fight = NULL;
		last_fight = fch->prev_fight;
	    }
	    else if (fch->next_fight != NULL)
	    {
		fch->next_fight->prev_fight = NULL;
		first_fight = fch->next_fight;
	    }
	    else
	    {
		first_fight = NULL;
		last_fight = NULL;
	    }

	    fch->prev_fight = NULL;
	    fch->next_fight = NULL;

	    update_pos( fch );
	}
    }

    return;
}


/*
 * The primary output interface for formatted output.
 */
void act( const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2, long type )
{
    static char * const he_she	[] = { "it",  "he",  "she" };
    static char * const him_her	[] = { "it",  "him", "her" };
    static char * const his_her	[] = { "its", "his", "her" };

    CHAR_DATA *to;
    CHAR_DATA *to_old;
    CHAR_DATA *vch = (CHAR_DATA *) arg2;
    CHAR_DATA *familiar = NULL;
    CHAR_DATA *wizard = NULL;
    OBJ_DATA *obj1 = (OBJ_DATA  *) arg1;
    OBJ_DATA *obj2 = (OBJ_DATA  *) arg2;
    const char *str;
    const char *i;
    char buf[MSL];
    char fname[MIL];
    char *point;
    bool is_fam;
    bool is_ok;

    /*
     * Discard null and zero-length messages.
     */
    if (format == NULL || format[0] == '\0') return;

    to = ch->in_room->people;
    if (type == TO_VICT)
    {
	if (vch == NULL || vch->in_room == NULL)
	{
	    bug("Act: null vch with TO_VICT.", 0);
	    return;
	}
	to = vch->in_room->people;
    }

    for ( ; to != NULL; to = to->next_in_room)
    {
	is_fam = FALSE;
	to_old = to;

	if (type == TO_CHAR && to != ch) continue;
	if (type == TO_VICT && (to != vch || to == ch)) continue;
	if (type == TO_ROOM && to == ch) continue;
	if (type == TO_NOTVICT && (to == ch || to == vch)) continue;

	if (to->desc == NULL && IS_NPC(to) && (wizard = to->wizard) != NULL)
	{
	    if (!IS_NPC(wizard) && (familiar=wizard->pcdata->familiar) != NULL && familiar == to)
	    {
		if (to->in_room == ch->in_room && wizard->in_room != to->in_room)
		{
		    to = wizard;
		    SET_BIT(to->extra, EXTRA_CAN_SEE);
		    is_fam = TRUE;
		}
	    }
	}

	if (to->desc == NULL || !IS_AWAKE(to))
	{
	    if (is_fam) 
	    {
		REMOVE_BIT(to->extra, EXTRA_CAN_SEE);
		to = to_old;
	    }
	    continue;
	}

    	if (ch->in_room->vnum == ROOM_VNUM_IN_OBJECT)
    	{
	    is_ok = FALSE;

	    if (!IS_NPC(ch) && ch->pcdata->chobj != NULL && ch->pcdata->chobj->in_obj != NULL
	     && !IS_NPC(to) && to->pcdata->chobj != NULL && to->pcdata->chobj->in_obj != NULL
	     && ch->pcdata->chobj->in_obj == to->pcdata->chobj->in_obj)
		is_ok = TRUE; else is_ok = FALSE;

	    if (!is_ok)
	    {
		if (is_fam) 
		{
		    REMOVE_BIT(to->extra, EXTRA_CAN_SEE);
		    to = to_old;
		}
		continue;
	    }
    	}

	point = buf;
	str = format;
	while (*str != '\0')
	{
	    if (*str != '$')
	    {
		*point++ = *str++;
		continue;
	    }
	    str++;

	    if (arg2 == NULL && *str >= 'A' && *str <= 'Z')
	    {
		bug("Act: missing arg2 for code %li.", *str);
		bug(format, 0);
		i = " <@@@> ";
	    }
	    else
	    {
		switch ( *str )
		{
		    default:	bug("Act: bad code %li.", *str);
				i = " <@@@> ";				break;
		/* Thx alex for 't' idea */
		    case 't':	i = (char *) arg1;			break;
		    case 'T':	i = (char *) arg2;          		break;
		    case 'n':	i = PERS(ch, to);			break;
		    case 'N':	i = PERS(vch, to);			break;
		    case 'e':	i = he_she  [URANGE(0, ch  ->sex, 2)];	break;
		    case 'E':	i = he_she  [URANGE(0, vch ->sex, 2)];	break;
		    case 'm':	i = him_her [URANGE(0, ch  ->sex, 2)];	break;
		    case 'M':	i = him_her [URANGE(0, vch ->sex, 2)];	break;
		    case 's':	i = his_her [URANGE(0, ch  ->sex, 2)];	break;
		    case 'S':	i = his_her [URANGE(0, vch ->sex, 2)];	break;
		    case 'p':	i = can_see_obj(to, obj1) ?
				 ((obj1->chobj != NULL && obj1->chobj == to) ?
				 "you" : obj1->short_descr) : "something";
				break;
		    case 'P':	i = can_see_obj(to, obj2) ?
				 ((obj2->chobj != NULL && obj2->chobj == to) ?
				 "you" : obj2->short_descr) : "something";
				break;
		    case 'd':	if (arg2 == NULL || ((char *) arg2)[0] == '\0')
				    i = "door";
				else
				{
				    one_argument( (char *) arg2, fname );
				    i = fname;
				}
				break;
		}
	    }

	    str++;
	    while ((*point = *i ) != '\0')
		++point, ++i;
	}

	*point++ = '\n';
	*point++ = '\r';

	if (is_fam)
	{
	    if (to->in_room != ch->in_room && familiar != NULL && familiar->in_room == ch->in_room)
		stc("[ ", to);
	    else
	    {
		REMOVE_BIT(to->extra, EXTRA_CAN_SEE);
		to = to_old;
		continue;
	    }
	}

	buf[0] = UPPER(buf[0]);
	write_to_buffer(to->desc, buf, point - buf);

	if (is_fam)
	{
	    REMOVE_BIT(to->extra, EXTRA_CAN_SEE);
	    to = to_old;
	}
    }
    return;
}

void act2( const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2, long type )
{
    static char * const he_she	[] = { "it",  "he",  "she" };
    static char * const him_her	[] = { "it",  "him", "her" };
    static char * const his_her	[] = { "its", "his", "her" };

    CHAR_DATA *to;
    CHAR_DATA *vch = (CHAR_DATA *) arg2;
    OBJ_DATA *obj1 = (OBJ_DATA  *) arg1;
    OBJ_DATA *obj2 = (OBJ_DATA  *) arg2;
    const char *str;
    const char *i;
    char *point;
    char buf[MSL];
    bool is_ok;

    /*
     * Discard null and zero-length messages.
     */
    if (format == NULL || format[0] == '\0') return;

    to = ch->in_room->people;
    if (type == TO_VICT)
    {
	if (vch == NULL || vch->in_room == NULL)
	{
	    bug("Act: null vch with TO_VICT.", 0);
	    return;
	}
	to = vch->in_room->people;
    }

    for ( ; to != NULL; to = to->next_in_room)
    {
	if (type == TO_CHAR && to != ch) continue;
	if (type == TO_VICT && (to != vch || to == ch)) continue;
	if (type == TO_ROOM && to == ch) continue;
	if (type == TO_NOTVICT && (to == ch || to == vch)) continue;

	if (to->desc == NULL || !IS_AWAKE(to)) continue;

    	if (ch->in_room->vnum == ROOM_VNUM_IN_OBJECT)
    	{
	    is_ok = FALSE;

	    if (!IS_NPC(ch) && ch->pcdata->chobj != NULL && ch->pcdata->chobj->in_obj != NULL
	     && !IS_NPC(to) && to->pcdata->chobj != NULL && to->pcdata->chobj->in_obj != NULL
	     && ch->pcdata->chobj->in_obj == to->pcdata->chobj->in_obj)
		is_ok = TRUE;
	    else is_ok = FALSE;

	    if (!is_ok) continue;
    	}

	point = buf;
	str = format;
	while (*str != '\0')
	{
	    if (*str != '$')
	    {
		*point++ = *str++;
		continue;
	    }
	    str++;

	    if (arg2 == NULL && *str >= 'A' && *str <= 'Z') i = " <@@@> ";
	    else
	    {
		switch (*str)
		{
		    default:	i = " "; break;
		    case 'n':	if (ch != NULL) i = PERS(ch,  to);
				else i = " ";
				break;
		    case 'N':	if (vch != NULL) i = PERS(vch,  to);
				else i = " ";
				break;
		    case 'e':	if (ch != NULL) i = he_she[URANGE(0, ch->sex, 2)];
				else i = " ";
				break;
		    case 'E':	if (vch != NULL) i = he_she[URANGE(0, vch->sex,2)];
				else i = " ";
				break;
		    case 'm':	if (ch != NULL) i = him_her[URANGE(0, ch->sex, 2)];
				else i = " ";
				break;
		    case 'M':	if (vch != NULL) i = him_her[URANGE(0, vch->sex,2)];
				else i = " ";
				break;
		    case 's':	if (ch != NULL) i = his_her[URANGE(0, ch->sex, 2)];
				else i = " ";
				break;
		    case 'S':	if (vch != NULL) i = his_her[URANGE(0, vch->sex,2)];
				else i = " ";
				break;
		    case 'p':	if (obj1 != NULL)
				{
				    i = can_see_obj(to, obj1) ?
				     ((obj1->chobj != NULL && obj1->chobj == to) ?
				      "you" : obj1->short_descr) : "something";
				}
				else i = " ";
				break;
		    case 'P':	if (obj2 != NULL)
				{
				    i = can_see_obj(to, obj2) ?
				     ((obj2->chobj != NULL && obj2->chobj == to) ?
				     "you" : obj2->short_descr) : "something";
				}
				else i = " ";
				break;
		}
	    }
		
	    str++;
	    while ((*point = *i) != '\0')
		++point, ++i;
	}

	*point++ = '\n';
	*point++ = '\r';
	buf[0] = UPPER(buf[0]);
	write_to_buffer(to->desc, buf, point - buf);
    }
    return;
}



void kavitem( const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2, long type )
{
    static char * const he_she	[] = { "it",  "he",  "she" };
    static char * const him_her	[] = { "it",  "him", "her" };
    static char * const his_her	[] = { "its", "his", "her" };

    char buf[MAX_STRING_LENGTH];
    char kav[MAX_INPUT_LENGTH];
    CHAR_DATA *to;
    CHAR_DATA *vch = (CHAR_DATA *) arg2;
    OBJ_DATA *obj1 = (OBJ_DATA  *) arg1;
    const char *str;
    const char *i;
    char *point;
    bool is_ok;

    /*
     * Discard null and zero-length messages.
     */
    if ( format == NULL || format[0] == '\0' )
	return;

    to = ch->in_room->people;
    if ( type == TO_VICT )
    {
	if ( vch == NULL )
	{
	    bug( "Act: null vch with TO_VICT.", 0 );
	    return;
	}
	to = vch->in_room->people;
    }
    
    for ( ; to != NULL; to = to->next_in_room )
    {
	if ( to->desc == NULL || !IS_AWAKE(to) )
	    continue;

    	if (ch->in_room->vnum == ROOM_VNUM_IN_OBJECT)
    	{
	    is_ok = FALSE;

	    if (!IS_NPC(ch) && ch->pcdata->chobj != NULL && 
	    	ch->pcdata->chobj->in_room != NULL &&
	    	!IS_NPC(to) && to->pcdata->chobj != NULL && 
	    	to->pcdata->chobj->in_room != NULL &&
	    	ch->in_room == to->in_room)
		    is_ok = TRUE; else is_ok = FALSE;

	    if (!IS_NPC(ch) && ch->pcdata->chobj != NULL && 
	    	ch->pcdata->chobj->in_obj != NULL &&
	    	!IS_NPC(to) && to->pcdata->chobj != NULL && 
	    	to->pcdata->chobj->in_obj != NULL &&
	    	ch->pcdata->chobj->in_obj == to->pcdata->chobj->in_obj)
		    is_ok = TRUE; else is_ok = FALSE;

	    if (!is_ok) continue;
    	}
	if ( type == TO_CHAR && to != ch )
	    continue;
	if ( type == TO_VICT && ( to != vch || to == ch ) )
	    continue;
	if ( type == TO_ROOM && to == ch )
	    continue;
	if ( type == TO_NOTVICT && (to == ch || to == vch) )
	    continue;

	point	= buf;
	str	= format;
	while ( *str != '\0' )
	{
	    if ( *str != '$' )
	    {
		*point++ = *str++;
		continue;
	    }
	    ++str;

	    if ( arg2 == NULL && *str >= 'A' && *str <= 'Z' )
		i = "";
	    else
	    {
		switch ( *str )
		{
		default:  i = "";					break;
		case 'n': i = PERS( ch,  to  );				break;
		case 'e': i = he_she  [URANGE(0, ch  ->sex, 2)];	break;
		case 'm': i = him_her [URANGE(0, ch  ->sex, 2)];	break;
		case 's': i = his_her [URANGE(0, ch  ->sex, 2)];	break;
		case 'p':
		    i = can_see_obj( to, obj1 )
			    ? ( (obj1->chobj != NULL && obj1->chobj == to)
			    ? "you" : obj1->short_descr)
			    : "something";
		    break;

		case 'o':
		    if (obj1 != NULL) sprintf(kav,"%s's",obj1->short_descr);
		    i = can_see_obj( to, obj1 )
			    ? ( (obj1->chobj != NULL && obj1->chobj == to)
			    ? "your" : kav)
			    : "something's";
		    break;

		}
	    }
		
	    ++str;
	    while ( ( *point = *i ) != '\0' )
		++point, ++i;
	}

	*point++ = '\n';
	*point++ = '\r';
	buf[0]   = UPPER(buf[0]);
        if (to->desc && (to->desc->connected == CON_PLAYING))
	    write_to_buffer( to->desc, buf, point - buf );
    }

    return;
}
