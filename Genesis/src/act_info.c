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
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"



long     get_ratio       args( ( CHAR_DATA *ch ) );

char *	const	where_name	[] =
{
    "#l[#cOn Finger     #l]#n ",
    "#l[#cOn Finger     #l]#n ",
    "#l[#cAround Neck   #l]#n ",
    "#l[#cAround Neck   #l]#n ",
    "#l[#cOn Body       #l]#n ",
    "#l[#cOn Head       #l]#n ",
    "#l[#cOn Legs       #l]#n ",
    "#l[#cOn Feet       #l]#n ",
    "#l[#cOn Hands      #l]#n ",
    "#l[#cOn Arms       #l]#n ",
    "#l[#cAround Body   #l]#n ",
    "#l[#cAround Waist  #l]#n ",
    "#l[#cAround Wrist  #l]#n ",
    "#l[#cAround Wrist  #l]#n ",
    "#l[#cWielded       #l]#n ",
    "#l[#cHeld          #l]#n ",
    "#l[#cOn Face       #l]#n ",
};



/*
 * Local functions.
 */
char *	format_obj_to_char	args( ( OBJ_DATA *obj, CHAR_DATA *ch,
				    bool fShort ) );
void	show_char_to_char_0	args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void	show_char_to_char_1	args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void	show_char_to_char	args( ( CHAR_DATA *list, CHAR_DATA *ch ) );
bool	check_blind		args( ( CHAR_DATA *ch ) );

void	obj_score		args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );


char *format_obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch, bool fShort )
{
    static char buf[MAX_STRING_LENGTH];

    buf[0] = '\0';
    if ( IS_SET(obj->quest, QUEST_ARTIFACT)) strcat(buf, "#R(Artifact)#n " );
    else if ( IS_SET(obj->quest, QUEST_RELIC)) strcat(buf, "#r(Relic)#n " );
    else if ( obj->points > 0 )
    {
	if (IS_SET(obj->quest, QUEST_IMPROVED)) strcat(buf, "#P(Mythical)#n " );
	else strcat(buf, "#p(Legendary)#n " );
    }
    if ( IS_SET(obj->quest, QUEST_AGGRAVATED)) strcat(buf, "#r(Aggravated)#n " );
    if ( IS_OBJ_STAT(obj, ITEM_INVIS)     )   strcat( buf, "#c(Invis)#n " );
    if ( IS_OBJ_STAT(obj, ITEM_GLOW)      )   strcat( buf, "#g(Glowing)#n " );
    if ( IS_OBJ_STAT(obj, ITEM_HUM)       )   strcat( buf, "#g(Humming)#n " );
    if ( IS_OBJ_STAT(obj, ITEM_SHADOWPLANE) &&
	 obj->in_room != NULL &&
	!IS_AFFECTED(ch,AFF_SHADOWPLANE) )    strcat( buf, "#C(Umbra)#n " );
    if (!IS_OBJ_STAT(obj, ITEM_SHADOWPLANE) &&
	 obj->in_room != NULL &&
	 IS_AFFECTED(ch,AFF_SHADOWPLANE) )    strcat( buf, "#C(Normal plane)#n " );
    if ( !IS_NPC(ch) && IS_VAMPIRE(ch) && 
	IS_OBJ_STAT(obj, ITEM_VANISH) &&
	get_disc(ch,DISC_CHIMERSTRY) > 0 )    strcat( buf, "#C(Illusion)#n " );
    if ( !IS_NPC(ch) && IS_WEREWOLF(ch) && IS_SET(obj->spectype, SITEM_SILVER) &&
	get_auspice(ch,AUSPICE_AHROUN) >= 1 )  strcat( buf, "#7(Silver)#n " );

    if (fShort && obj->short_descr != NULL) strcat(buf, obj->short_descr);
    else if (obj->description != NULL) strcat(buf, obj->description);

    return buf;
}

int raw_stat( CHAR_DATA *ch, int stat )
{
    int power = ch->stats[stat], max = stat_max(ch, stat);

    if (power > max) power = max;
    return power;
}

int char_stat( CHAR_DATA *ch, int stat )
{
    int power = raw_stat(ch, stat);

    switch (stat)
    {
	case STAT_STR:
	    if (IS_XTRA(ch, XTRA_BETTER_STR)) power++;
	    if (IS_NINJA(ch) && ch->pcdata->ninja[NINJA_HARAKIRI] > 0) power += 5;
	    break;
	case STAT_DEX:
	    if (IS_XTRA(ch, XTRA_BETTER_DEX)) power++;
	    if (IS_HIGHLANDER(ch) && get_celerity(ch) > 0) power += 2;
	    break;
	case STAT_CON:
	    if (IS_XTRA(ch, XTRA_BETTER_CON)) power++;
	    break;
	case STAT_INT:
	    if (IS_XTRA(ch, XTRA_BETTER_INT)) power++;
	    break;
	case STAT_WIS:
	    if (IS_XTRA(ch, XTRA_BETTER_WIS)) power++;
	    break;
    }

    return power;
}

int stat_max( CHAR_DATA *ch, int stat )
{
    long max = 8;

    if (IS_IMM(ch)) max += 5;

    if (IS_NPC(ch)) max = 1000;
    else switch (stat)
    {
	case STAT_STR:
	    if (IS_DRAGON(ch) || IS_HIGHLANDER(ch) || IS_WEREWOLF(ch)) max += 3;
	    if (IS_NINJA(ch) || IS_VAMPIRE(ch)) max += 2;
	    if (IS_DROW(ch) || IS_ELEM(ch) || IS_SORCERER(ch) || IS_MONK(ch)) max += 1;
	    break;
	case STAT_DEX:
	    if (IS_DROW(ch) || IS_NINJA(ch)) max += 3;
	    if (IS_MONK(ch)) max += 2;
	    if (IS_HIGHLANDER(ch) || IS_SORCERER(ch) || IS_WEREWOLF(ch) || IS_VAMPIRE(ch) || IS_MAGE(ch)) max += 1;
	    break;
	case STAT_CON:
	    if (IS_VAMPIRE(ch)) max += 3;
	    if (IS_WEREWOLF(ch) || IS_DRAGON(ch) || IS_HIGHLANDER(ch)) max += 2;
	    if (IS_ELEM(ch) || IS_NINJA(ch) || IS_MONK(ch) || IS_MAGE(ch)) max += 1;
	    break;
	case STAT_INT:
	    if (IS_SORCERER(ch) || IS_MAGE(ch)) max += 3;
	    if (IS_DROW(ch) || IS_ELEM(ch)) max += 2;
	    if (IS_DRAGON(ch) || IS_NINJA(ch) || IS_VAMPIRE(ch)) max += 1;
	    break;
	case STAT_WIS:
	    if (IS_MONK(ch) || IS_ELEM(ch)) max += 3;
	    if (IS_SORCERER(ch) || IS_MAGE(ch)) max += 2;
	    if (IS_DROW(ch) || IS_DRAGON(ch) || IS_HIGHLANDER(ch) || IS_WEREWOLF(ch)) max += 1;
	    break;
    }
    return max;
}

long stat_cost( CHAR_DATA *ch, int stat )
{
    long cost = raw_stat(ch, stat);

    if (cost > 5) cost *= 2 * cost;
    else cost *= cost;
    cost *= 10000;

    return cost;
}


long get_celerity( CHAR_DATA *ch )
{
    if (IS_NPC(ch)) return 0;

    if (IS_WEREWOLF(ch))
    {
	if (ch->pcdata->rank == 5) return ch->pcdata->rank;
	else return ch->pcdata->rank+1;
    }
    else if (IS_NINJA(ch)) return get_ninja(ch, NINJA_NINGENNO);
    else if (IS_HIGHLANDER(ch) && ch->pcdata->celerity > 0) return 1;
    else if (ch->pcdata->celerity > 0 && IS_VAMPIRE(ch))
	return get_disc(ch, DISC_CELERITY);

    return 0;
}

long get_disc( CHAR_DATA *ch, long disc )
{
    long max = 7;

    if (!IS_VAMPIRE(ch)) return 0;

    if (disc < DISC_ANIMALISM || disc > DISC_MAX)
    {
	bug( "Get_disc: Out of discipline range.", 0 );
	return 0;
    }

    if (ch->pcdata->powers[disc] > max) return max;
    else return ch->pcdata->powers[disc];
}

long get_truedisc( CHAR_DATA *ch, long disc )
{
    if (!IS_VAMPIRE(ch)) return 0;

    if (disc < DISC_ANIMALISM || disc > DISC_MAX)
    {
	bug( "Get_truedisc: Out of discipline range.", 0 );
	return 0;
    }

    return ch->pcdata->powers[disc];
}

long get_breed( CHAR_DATA *ch, long gift )
{
    long max = 5;

    if (!IS_WEREWOLF(ch)) return 0;

    if (gift < BREED_HOMID || gift > BREED_METIS)
    {
	bug("Get_breed: Out of breed range.", 0);
	return 0;
    }

    if (ch->pcdata->breed[gift] > max) return max;
    return ch->pcdata->breed[gift];
}

long get_auspice( CHAR_DATA *ch, long gift )
{
    long max = 5;

    if (!IS_WEREWOLF(ch)) return 0;

    if (gift < AUSPICE_RAGABASH || gift > AUSPICE_AHROUN)
    {
	bug("Get_auspice: Out of auspice range.", 0);
	return 0;
    }

    if (ch->pcdata->auspice[gift] > max) return max;
    return ch->pcdata->auspice[gift];
}

long get_tribe( CHAR_DATA *ch, long gift )
{
    long max = 5;

    if (!IS_WEREWOLF(ch)) return 0;

    if (gift < TRIBE_BLACK_FURIES || gift > TRIBE_WENDIGOS)
    {
	bug("Get_tribe: Out of tribe range.", 0);
	return 0;
    }

    if (ch->pcdata->tribes[gift] > max) return max;
    return ch->pcdata->tribes[gift];
}

long get_color( CHAR_DATA *ch, long color )
{
    long max = 5;

    if (!IS_SORCERER(ch)) return 0;

    if (color < 0 || color > 5)
    {
	bug("Get_color: out of color range", 0);
	return 0;
    }

    if (ch->pcdata->colors[color] > max) return max;
    return ch->pcdata->colors[color];
}

long get_highland( CHAR_DATA *ch, long power )
{
    long max = 6;

    if (!IS_HIGHLANDER(ch)) return 0;

    if (power < 0 || power > 3)
    {
	bug("Get_highland: out of power range", 0);
	return 0;
    }

    if (ch->pcdata->highland[power] > max) return max;
    return ch->pcdata->highland[power];
}

long get_ninja( CHAR_DATA *ch, long power )
{
    long max = 10;

    if (!IS_NINJA(ch)) return 0;

    if (power < 0 || power > 2)
    {
	bug("Get_ninja: out of power range", 0);
	return 0;
    }

    if (ch->pcdata->ninja[power] > max) return max;
    return ch->pcdata->ninja[power];
}

long get_monk( CHAR_DATA *ch, long power )
{
    long max = 8;

    if (!IS_MONK(ch)) return 0;

    if (power < 0 || power > 2)
    {
	bug("Get_monk: out of power range", 0);
	return 0;
    }

    if (ch->pcdata->monk[power] > max) return max;
    return ch->pcdata->monk[power];
}

bool get_drow( CHAR_DATA *ch, long power )
{
    if (!IS_DROW(ch)) return FALSE;

    if (IS_SET(ch->pcdata->drow[DROW_POWERS], power)) return TRUE;
    else return FALSE;
}

long get_style( CHAR_DATA *ch, long style )
{
    if (style < STYLE_UNARMED || style > STYLE_TWOWEAPON)
    {
	bug("Get_style: out of style range", 0);
	return 0;
    }

    if (ch->style[style] < 0) ch->style[style] = 0;
    if (ch->style[style] > 200) ch->style[style] = 200;

    return ch->style[style];
}

long char_hitroll( CHAR_DATA *ch )
{
    long hr = GET_HITROLL(ch);
    long min;

    if (!IS_NPC(ch))
    {
	if (IS_NINJA(ch) && ch->pcdata->wolf > 0)
	    hr += ch->pcdata->wolf;

	if (IS_VAMPIRE(ch) && get_celerity(ch) > 0) 
	    hr += (get_celerity(ch) * 20);
	if ((ch->class == CLASS_VAMPIRE) && ch->pcdata->wolf > 0)
	    hr += ch->beast;
	else if (IS_SET(ch->act, PLR_WOLFMAN) && ch->pcdata->wolf > 0)
	    hr += ch->pcdata->wolf;
	if (IS_HIGHLANDER(ch))
	    hr += get_highland(ch, HIGH_ATTACK) * 50;
        if (IS_MONK(ch))
	{
	    if (get_monk(ch, MONK_ART) >= 2) hr += 200;
	    if (IS_POLYAFF(ch, POLY_CHIMERA)) hr += 200;
	}
	if (IS_WEREWOLF(ch) && get_tribe(ch, TRIBE_STARGAZERS) >= 2)
	    hr += get_tribe(ch, TRIBE_STARGAZERS) * 10;
	if (IS_POLYAFF(ch, POLY_WOLF)) hr += 200;
	else if (IS_POLYAFF(ch, POLY_SERPENT)) hr += 150;
	else if (IS_POLYAFF(ch, POLY_ZULO)) hr += 200;
	else if (IS_POLYAFF(ch, POLY_CHIRO)) hr += 500;
	else if (IS_POLYAFF(ch, POLY_DRAGONFORM)) hr += ((13-ch->vampgen)*50);
    }

    if (!IS_NPC(ch) && ch->pcdata->wolf > 0) return hr;

    if (ch->hit < ch->max_hit)
    {
	if (!IS_NPC(ch) && IS_WEREWOLF(ch) && get_auspice(ch, AUSPICE_PHILODOX) > 0)
	    return hr;
	min = hr * 0.75;
	if (ch->hit < 1 || ch->max_hit < 1)
	    hr *= 0.75;
	else
	{
	    hr = hr * ch->hit / ch->max_hit;
	    if (hr < min) hr = min;
	}
    }
    return hr;
}

long char_damroll( CHAR_DATA *ch )
{
    long dr = GET_DAMROLL(ch);
    long min;

    if (!IS_NPC(ch))
    {
	if (IS_MONK(ch))
	{
	    if (get_monk(ch, MONK_ART) >= 1) dr += 100;
	    if (get_monk(ch, MONK_ART) >= 7) dr += 100;
	    if (IS_POLYAFF(ch, POLY_CHIMERA)) dr += 200;
	}
	if (IS_NINJA(ch))
	{
	    if (ch->pcdata->wolf > 0) dr += ch->pcdata->wolf;
	}
	if (get_disc(ch,DISC_POTENCE) > 0) 
	    dr += get_disc(ch,DISC_POTENCE) * 20;
	if ((ch->class == CLASS_VAMPIRE && !IS_WEREWOLF(ch)) && ch->pcdata->wolf > 0)
	    dr += ch->beast;
	else if (IS_SET(ch->act,PLR_WOLFMAN) && ch->pcdata->wolf > 0)
	    dr += ch->pcdata->wolf;
	if (IS_HIGHLANDER(ch))
	    dr += get_highland(ch, HIGH_ATTACK) * 50;
	if (IS_WEREWOLF(ch) && get_tribe(ch, TRIBE_STARGAZERS) >= 2)
	    dr += get_tribe(ch, TRIBE_STARGAZERS) * 10;
        if (IS_DROW(ch))
            dr += ch->pcdata->drow[DROW_SPEED] * 50;
	if (IS_ITEMAFF(ch, ITEMA_AURA)) dr += (ch->pcdata->powers[ELEM_SUN]*ch->pcdata->powers[ELEM_SUN]*2.5);
	if (IS_POLYAFF(ch, POLY_WOLF)) dr += 200;
	else if (IS_POLYAFF(ch, POLY_SERPENT)) dr += 150;
	else if (IS_POLYAFF(ch, POLY_ZULO)) dr += 200;
        else if (IS_POLYAFF(ch, POLY_CHIRO)) dr += 500;
	else if (IS_POLYAFF(ch, POLY_DRAGONFORM)) dr += ((13-ch->vampgen)*50);
    }

    if (!IS_NPC(ch) && ch->pcdata->wolf > 0) return dr;

    if (ch->hit < ch->max_hit)
    {
	if (!IS_NPC(ch) && IS_WEREWOLF(ch) && get_auspice(ch, AUSPICE_PHILODOX) > 0)
	    return dr;
	min = dr * 0.75;
	if (ch->hit < 1 || ch->max_hit < 1)
	    dr *= 0.75;
	else
	{
	    dr = dr * ch->hit / ch->max_hit;
	    if (dr < min) dr = min;
	}
    }
    return dr;
}

long char_ac( CHAR_DATA *ch )
{
    long a_c = GET_AC(ch), min;

    if (!IS_NPC(ch))
    {
	if (get_disc(ch,DISC_FORTITUDE) > 0)
	    a_c += (get_disc(ch,DISC_FORTITUDE) * 100);
	if (IS_DRAGON(ch) && IS_POLYAFF(ch, POLY_DRAGONFORM)) a_c += 300;
	if (IS_HIGHLANDER(ch)) a_c += get_highland(ch, HIGH_DEFENSE) * 100;
	if (IS_WEREWOLF(ch) && get_tribe(ch, TRIBE_CHILDREN_OF_GAIA) >= 2) a_c += 250;
        if (IS_MONK(ch))
	{
	    if (get_monk(ch, MONK_ART) >= 4) a_c += 375;
	    if (IS_POLYAFF(ch, POLY_CHIMERA)) a_c += 375;
	}
	if (IS_POLYAFF(ch, POLY_WOLF)) a_c += 100;
	else if (IS_POLYAFF(ch, POLY_SERPENT)) a_c += 200;
	else if (IS_POLYAFF(ch, POLY_BAT)) a_c += 300;
	else if (IS_POLYAFF(ch, POLY_ZULO)) a_c += 200;
        else if (IS_POLYAFF(ch, POLY_CHIRO)) a_c += 500;
	else if (IS_POLYAFF(ch, POLY_DRAGONFORM)) a_c += ((13-ch->vampgen)*75);
    }

    if (!IS_NPC(ch) && ch->pcdata->wolf > 0) return a_c;

    if (ch->hit < ch->max_hit)
    {
	if (!IS_NPC(ch) && IS_WEREWOLF(ch) && get_auspice(ch, AUSPICE_PHILODOX) >= 1)
	    return a_c;
	min = a_c * 0.75;
	if (ch->hit < 1 || ch->max_hit < 1)
	    a_c *= 0.75;
	else
        {
            a_c = a_c * ch->hit / ch->max_hit;
            if (a_c < min) a_c = min;
        }
    }
    return a_c;
}

/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char( OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing )
{
    char buf[MAX_STRING_LENGTH];
    char **prgpstrShow;
    long *prgnShow;
    char *pstrShow;
    OBJ_DATA *obj;
    long nShow;
    long iShow;
    long count;
    bool fCombine;

    if ( ch->desc == NULL )
	return;

    /*
     * Alloc space for output lines.
     */
    count = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
	count++;
    prgpstrShow	= alloc_mem( count * sizeof(char *) );
    prgnShow    = alloc_mem( count * sizeof(long)    );
    nShow	= 0;

    /*
     * Format the list of objects.
     */
    for ( obj = list; obj != NULL; obj = obj->next_content )
    { 
	if (!IS_NPC(ch) && ch->pcdata->chobj != NULL && obj->chobj != NULL && obj->chobj == ch)
	    continue;
	if (obj->description != NULL && strlen(obj->description) < 2)
	    continue;
	if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) )
	{
	    pstrShow = format_obj_to_char( obj, ch, fShort );
	    fCombine = FALSE;

	    if ( IS_NPC(ch) || IS_SET(ch->act, PLR_COMBINE) )
	    {
		/*
		 * Look for duplicates, case sensitive.
		 * Matches tend to be near end so run loop backwords.
		 */
		for ( iShow = nShow - 1; iShow >= 0; iShow-- )
		{
		    if ( !strcmp( prgpstrShow[iShow], pstrShow ) )
		    {
			prgnShow[iShow]++;
			fCombine = TRUE;
			break;
		    }
		}
	    }

	    /*
	     * Couldn't combine, or didn't want to.
	     */
	    if ( !fCombine )
	    {
		prgpstrShow [nShow] = str_dup( pstrShow );
		prgnShow    [nShow] = 1;
		nShow++;
	    }
	}
    }

    /*
     * Output the formatted list.
     */
    for ( iShow = 0; iShow < nShow; iShow++ )
    {
	if ( IS_NPC(ch) || IS_SET(ch->act, PLR_COMBINE) )
	{
	    if ( prgnShow[iShow] != 1 )
	    {
		sprintf( buf, "(%2li) ", prgnShow[iShow] );
		send_to_char( buf, ch );
	    }
	    else
	    {
		send_to_char( "     ", ch );
	    }
	}
	send_to_char( prgpstrShow[iShow], ch );
	send_to_char( "\n\r", ch );
	free_string( prgpstrShow[iShow] );
    }

    if ( fShowNothing && nShow == 0 )
    {
	if ( IS_NPC(ch) || IS_SET(ch->act, PLR_COMBINE) )
	    send_to_char( "     ", ch );
	send_to_char( "Nothing.\n\r", ch );
    }

    /*
     * Clean up.
     */
    free_mem( prgpstrShow, count * sizeof(char *) );
    free_mem( prgnShow,    count * sizeof(long)    );

    return;
}

void show_char_to_char_0( CHAR_DATA *victim, CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    char buf4[MAX_STRING_LENGTH];
    char *vname;
    char *ename;

    buf[0] = '\0';
    buf2[0] = '\0';
    buf3[0] = '\0';
    buf4[0] = '\0';

    if (!IS_NPC(victim) && victim->pcdata->chobj != NULL )
	return;

    if ( IS_MORE(victim, MORE_HEAD) && IS_AFFECTED(victim, AFF_POLYMORPH))
	strcat( buf, "     " );
    else {
	if (!IS_NPC(victim) && victim->desc==NULL ) strcat( buf,"(Link-Dead) ");
    	if ( IS_MORE(victim, MORE_SPIRIT)         ) strcat( buf, "(Spirit World) " );
    	if ( IS_AFFECTED(victim, AFF_INVISIBLE)   ) strcat( buf, "(Invis) "   );
    	if ( IS_AFFECTED(victim, AFF_HIDE)        ) strcat( buf, "(Hide) "    );
    	if ( IS_AFFECTED(victim, AFF_CHARM)       ) strcat( buf, "(Charmed) " );
    	if ( IS_AFFECTED(victim, AFF_PASS_DOOR)  ||
	    IS_AFFECTED(victim, AFF_ETHEREAL)     ) strcat( buf, "(Translucent) ");
    	if ( IS_AFFECTED(victim, AFF_FAERIE_FIRE) ) strcat( buf, "(Pink Aura) "  );
    	if ( IS_AFFECTED(victim, AFF_SANCTUARY)   ) strcat( buf, "(White Aura) " );
	if ( !IS_NPC(ch) && IS_VAMPIRE(ch) && 
	    IS_EXTRA(victim, EXTRA_ILLUSION) &&
	    get_disc(ch,DISC_CHIMERSTRY) > 0      ) strcat( buf, "(Illusion) "   );
	if ( !IS_NPC(ch) && IS_WEREWOLF(ch) && 
	   (IS_ITEMAFF(victim, ITEMA_LEFT_SILVER) ||
	    IS_ITEMAFF(victim, ITEMA_RIGHT_SILVER)) &&
	    get_auspice(ch,AUSPICE_AHROUN) >= 1    ) strcat( buf, "(Silver) "   );
    }
    if ( IS_AFFECTED(ch, AFF_SHADOWPLANE) && !IS_AFFECTED(victim, AFF_SHADOWPLANE))
	strcat( buf, "(Normal plane) "     );
    else if ( !IS_AFFECTED(ch, AFF_DEEPUMBRA ) && IS_AFFECTED(victim, AFF_DEEPUMBRA))
	strcat( buf, "(Deep Umbra) "	);
    else if ( IS_AFFECTED(ch, AFF_DEEPUMBRA) && !IS_AFFECTED(victim, AFF_DEEPUMBRA))
	strcat( buf, "(Umbra) "	);
    else if ( !IS_AFFECTED(ch, AFF_SHADOWPLANE) && IS_AFFECTED(victim, AFF_SHADOWPLANE))
	strcat( buf, "(Umbra) "     );

    if ( !IS_NPC(ch) && !IS_NPC(victim) && IS_VAMPIRE(victim) && IS_VAMPAFF(victim, VAM_DISGUISED))
    {
	if ((IS_VAMPIRE(ch) && get_disc(ch,DISC_AUSPEX) >= get_disc(victim,DISC_OBFUSCATE))
	    || (IS_WEREWOLF(ch) && get_auspice(ch, AUSPICE_PHILODOX) > 1))
	{
	    if (!IS_CRAFT(victim, CRAFT_MALLEABLE) && !IS_SET(victim->act, PLR_WOLFMAN))
	    {
		strcat(buf, "(");
		strcat(buf, victim->name);
		strcat(buf, ")");
	    }
	}
    }

    if ( IS_AFFECTED(victim, AFF_FLAMING) ) 
    {
	if (IS_NPC(victim))
	    sprintf( buf2, "...%s is engulfed in blazing flames!", victim->short_descr );
	else if (!IS_NPC(victim) && IS_AFFECTED(victim,AFF_POLYMORPH))
	    sprintf( buf2, "...%s is engulfed in blazing flames!", victim->morph );
	else
	    sprintf( buf2, "...%s is engulfed in blazing flames!", victim->name );
    }

    if ( !IS_NPC(victim) && IS_MORE(victim, MORE_HEAD) && IS_AFFECTED(victim, AFF_POLYMORPH)) 
    {
	if (IS_EXTRA(victim,GAGGED) && IS_EXTRA(victim,BLINDFOLDED))
	    sprintf( buf3, "\n\r...%s is gagged and blindfolded!", victim->morph );
	else if (IS_EXTRA(victim,GAGGED))
	    sprintf( buf3, "\n\r...%s is gagged!", victim->morph );
	else if (IS_EXTRA(victim,BLINDFOLDED))
	    sprintf( buf3, "\n\r...%s is blindfolded!", victim->morph );
    }

    if ( IS_MORE(victim, MORE_HEAD) && IS_AFFECTED(victim, AFF_POLYMORPH))
    {
	strcat( buf, victim->morph );
	strcat( buf, " is lying here.\n\r" );
    	strcat( buf, buf2 );
    	strcat( buf, buf3 );
	strcat( buf, "\n\r" );
    	buf[5] = UPPER(buf[5]);
    	send_to_char( buf, ch );
	return;
    }

    if ( IS_EXTRA(victim, TIED_UP) ) 
    {
	if (IS_NPC(victim))
	    sprintf( buf3, "\n\r...%s is tied up", victim->short_descr );
	else if (!IS_NPC(victim) && IS_AFFECTED(victim,AFF_POLYMORPH))
	    sprintf( buf3, "\n\r...%s is tied up", victim->morph );
	else
	    sprintf( buf3, "\n\r...%s is tied up", victim->name );
	if (IS_EXTRA(victim,GAGGED) && IS_EXTRA(victim,BLINDFOLDED))
	    strcat( buf3, ", gagged and blindfolded!" );
	else if (IS_EXTRA(victim,GAGGED))
	    strcat( buf3, " and gagged!" );
	else if (IS_EXTRA(victim,BLINDFOLDED))
	    strcat( buf3, " and blindfolded!" );
	else
	    strcat( buf3, "!" );
    }

    if ( IS_AFFECTED(victim, AFF_WEBBED) ) 
    {
	if (IS_NPC(victim))
	    sprintf( buf4, "\n\r...%s is coated in a sticky web.", victim->short_descr );
	else if (!IS_NPC(victim) && IS_AFFECTED(victim,AFF_POLYMORPH))
	    sprintf( buf4, "\n\r...%s is coated in a sticky web.", victim->morph );
	else
	    sprintf( buf4, "\n\r...%s is coated in a sticky web.", victim->name );
	strcat( buf3, buf4 );
    }

    if ( IS_AFFECTED(victim, AFF_SHADOW) ) 
    {
	if (IS_NPC(victim))
	    sprintf( buf4, "\n\r...%s is entrapped by tendrils of darkness.", victim->short_descr );
	else if (!IS_NPC(victim) && IS_AFFECTED(victim,AFF_POLYMORPH))
	    sprintf( buf4, "\n\r...%s is entrapped by tendrils of darkness.", victim->morph );
	else
	    sprintf( buf4, "\n\r...%s is entrapped by tendrils of darkness.", victim->name );
	strcat( buf3, buf4 );
    }

    if ( victim->embraced == ARE_EMBRACING && victim->embrace != NULL ) 
    {
	CHAR_DATA *emb = victim->embrace;
	if (IS_NPC(victim))
	    vname = str_dup(victim->short_descr);
	else if (IS_AFFECTED(victim,AFF_POLYMORPH))
	    vname = str_dup(victim->morph);
	else vname = str_dup(victim->name);

	if (IS_NPC(emb))
	    ename = str_dup(emb->short_descr);
	else if (IS_AFFECTED(emb,AFF_POLYMORPH))
	    ename = str_dup(emb->morph);
	else ename = str_dup(emb->name);

	if (ch == victim)
	    sprintf( buf4, "\n\r...You are holding %s in a close embrace.", ename );
	else if (ch == emb)
	    sprintf( buf4, "\n\r...%s is holding you in a close embrace.", vname );
	else
	    sprintf( buf4, "\n\r...%s is holding %s in a close embrace.", vname, ename );
	strcat( buf3, buf4 );
    }
    else if ( victim->embraced == ARE_EMBRACED && victim->embrace != NULL ) 
    {
	CHAR_DATA *emb = victim->embrace;
	if (IS_NPC(victim))
	    vname = str_dup(victim->short_descr);
	else if (IS_AFFECTED(victim,AFF_POLYMORPH))
	    vname = str_dup(victim->morph);
	else vname = str_dup(victim->name);

	if (IS_NPC(emb))
	    ename = str_dup(emb->short_descr);
	else if (IS_AFFECTED(emb,AFF_POLYMORPH))
	    ename = str_dup(emb->morph);
	else ename = str_dup(emb->name);

	if (ch == victim)
	    sprintf( buf4, "\n\r...You are being held by %s in a close embrace.", ename );
	else if (ch == emb)
	    sprintf( buf4, "\n\r...%s is being held by you in a close embrace.", vname );
	else
	    sprintf( buf4, "\n\r...%s is being held by %s in a close embrace.", vname, ename );
	strcat( buf3, buf4 );
    }

    if ( victim->position == POS_STANDING && victim->long_descr[0] != '\0' && 
	strlen(victim->long_descr) > 3 && victim->stance[0] < 1)
    {
	strcpy(buf4, victim->long_descr);
	buf4[strlen(buf4)-2] = '\0';
	strcat(buf, buf4);
	strcat(buf, buf2);
	strcat(buf, buf3);
	strcat(buf, "\n\r");
	buf[0] = UPPER(buf[0]);
	stc(buf, ch);
	return;
    }

    if ( !IS_NPC(victim) && IS_AFFECTED(victim, AFF_POLYMORPH) )
	strcat( buf, victim->morph );
    else
    	strcat( buf, PERS( victim, ch ) );

    if (victim->position == POS_STANDING && (IS_AFFECTED(victim, AFF_FLYING) || (!IS_NPC(victim) && IS_VAMPAFF(victim,VAM_FLYING))))
	strcat(buf, " is hovering here.");
    else
    {
	switch (victim->position)
	{
	case POS_DEAD: strcat(buf, " is DEAD!!"); break;
	case POS_MORTAL:
	    if (IS_VAMPIRE(victim)) strcat(buf, " is lying here torpored.");
	    else strcat(buf, " is lying here mortally wounded.");
	    break;
	case POS_INCAP: strcat(buf, " is lying here incapacitated.");break;
	case POS_STUNNED: strcat(buf, " is lying here stunned."); break;
	case POS_SLEEPING: strcat(buf, " is sleeping here."); break;
    	case POS_RESTING: strcat(buf, " is resting here."); break;
    	case POS_MEDITATING: strcat(buf, " is meditating here."); break;
    	case POS_SITTING: strcat(buf, " is sitting here."); break;
    	case POS_STANDING:
	    if (victim->stance[0] == STANCE_VIPER)
		strcat( buf, " is here, crouched in a viper fighting stance." );
	    else if (victim->stance[0] == STANCE_CRANE)
		strcat( buf, " is here, crouched in a crane fighting stance." );
	    else if (victim->stance[0] == STANCE_CRAB)
		strcat( buf, " is here, crouched in a crab fighting stance." );
	    else if (victim->stance[0] == STANCE_MONGOOSE)
		strcat( buf, " is here, crouched in a mongoose fighting stance." );
	    else if (victim->stance[0] == STANCE_BULL)
		strcat( buf, " is here, crouched in a bull fighting stance." );
	    else if (victim->stance[0] == STANCE_MANTIS)
		strcat( buf, " is here, crouched in a mantis fighting stance." );
	    else if (victim->stance[0] == STANCE_DRAGON)
		strcat( buf, " is here, crouched in a dragon fighting stance." );
	    else if (victim->stance[0] == STANCE_TIGER)
		strcat( buf, " is here, crouched in a tiger fighting stance." );
	    else if (victim->stance[0] == STANCE_MONKEY)
		strcat( buf, " is here, crouched in a monkey fighting stance." );
	    else if (victim->stance[0] == STANCE_SWALLOW)
		strcat( buf, " is here, crouched in a swallow fighting stance." );
	    else
		strcat( buf, " is here." );
	    break;
    	case POS_FIGHTING:
	    strcat( buf, " is here, fighting " );
	    if (victim->fighting == NULL) 
	    {
		strcat(buf, "thin air??");
		victim->position = POS_STANDING;
	    }
	    else if (victim->fighting == ch) strcat(buf, "YOU!");
	    else if (victim->in_room == victim->fighting->in_room)
	    {
	        strcat(buf, PERS(victim->fighting, ch));
	        strcat(buf, ".");
	    }
	    else strcat(buf, "somone who left??");
	    break;
	}
    }

    if (buf2[0] != '\0') strcat( buf, "\n\r" );
    strcat( buf, buf2 );
    strcat( buf, buf3 );
    strcat( buf, "\n\r" );
    buf[0] = UPPER(buf[0]);
    send_to_char( buf, ch );

    return;
}



void show_char_to_char_1( CHAR_DATA *victim, CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    long iWear;
    long percent;
    bool found;

    if ( can_see( victim, ch ) )
    {
	act( "$n looks at you.", ch, NULL, victim, TO_VICT    );
	act( "$n looks at $N.",  ch, NULL, victim, TO_NOTVICT );
    }
    if(IS_EXTRA(victim, EXTRA_MARK))
	{
	do_kill(victim, ch->name);
	return;
	}
    if (!IS_NPC(ch) && IS_MORE(victim, MORE_HEAD))
    {
	act( "$N is lying here.", ch, NULL, victim, TO_CHAR );
	return;
    }
    if ( victim->description[0] != '\0' )
    {
	send_to_char( victim->description, ch );
    }
    else
    {
	act( "You see nothing special about $M.", ch, NULL, victim, TO_CHAR );
    }

    if ( victim->max_hit > 0 )
	percent = ( 100 * victim->hit ) / victim->max_hit;
    else
	percent = -1;

    strcpy( buf, PERS(victim, ch) );

         if ( percent >= 100 ) strcat( buf, " is in perfect health.\n\r"  );
    else if ( percent >=  90 ) strcat( buf, " is slightly scratched.\n\r" );
    else if ( percent >=  80 ) strcat( buf, " has a few bruises.\n\r"     );
    else if ( percent >=  70 ) strcat( buf, " has some cuts.\n\r"         );
    else if ( percent >=  60 ) strcat( buf, " has several wounds.\n\r"    );
    else if ( percent >=  50 ) strcat( buf, " has many nasty wounds.\n\r" );
    else if ( percent >=  40 ) strcat( buf, " is bleeding freely.\n\r"    );
    else if ( percent >=  30 ) strcat( buf, " is covered in blood.\n\r"   );
    else if ( percent >=  20 ) strcat( buf, " is leaking guts.\n\r"       );
    else if ( percent >=  10 ) strcat( buf, " is almost dead.\n\r"        );
    else                       strcat( buf, " is DYING.\n\r"              );

    buf[0] = UPPER(buf[0]);
    send_to_char( buf, ch );

    if (IS_WEREWOLF(victim))
    {
	if (IS_MORE(victim, MORE_BRISTLES) && IS_SET(victim->act, PLR_WOLFMAN))
	    act("$N's body is covered in thick fur and quill-like spines.",ch,NULL,victim,TO_CHAR);
	else if (IS_SET(victim->act, PLR_WOLFMAN))
	    act("$N's body is covered in thick fur.",ch,NULL,victim,TO_CHAR);
    }
    if (IS_DRAGON(victim))
    {
	if (IS_POLYAFF(victim, POLY_DRAGONFORM))
	    act("$N's body is covered in thick scales.", ch, NULL, victim, TO_CHAR);
	if (IS_DRAGAFF(victim, DRAG_WINGSOUT))
	    act("$N has a pair of scale-encrusted wings spread out behind $S back.", ch, NULL, victim, TO_CHAR);
	if (IS_DRAGAFF(victim, DRAG_HORNSOUT))
	    act("$N has a pair of pointed horns extending from $S head.", ch, NULL, victim, TO_CHAR);
	if (IS_POLYAFF(victim, POLY_DRAGONFORM))
	    act("$N has a long tail hanging from $S backside.", ch, NULL, victim, TO_CHAR);
    }

    if (IS_WEREWOLF(victim) && IS_VAMPAFF(victim, VAM_NIGHTSIGHT)) act("$N's eyes are glowing bright green.",ch,NULL,victim,TO_CHAR);
    else if (IS_AFFECTED(victim, AFF_INFRARED) || IS_VAMPAFF(victim, VAM_NIGHTSIGHT)) act("$N's eyes are glowing bright red.",ch,NULL,victim,TO_CHAR);

    if (IS_VAMPAFF(victim, VAM_FANGS)) act("$N has a pair of long, pointed fangs.",ch,NULL,victim,TO_CHAR);
    if (IS_AFFECTED(victim, AFF_FLYING)) 
    {
	if (IS_DRAGON(victim)) act("$N is flying in the air.",ch,NULL,victim,TO_CHAR);
	else if (!IS_DROW(victim)) act("$N is hovering in the air.",ch,NULL,victim,TO_CHAR);
    }
    if (IS_DROW(victim))
    {
	if (IS_AFFECTED(victim, AFF_FLYING))
	    act("$N is hovering on $S Skip Disc.", ch, NULL, victim, TO_CHAR);
	if (get_drow(ch, DROW_BITE))
	    act("$N has a pair of poisonous fangs.", ch, NULL, victim, TO_CHAR);
    }
    if (IS_VAMPIRE(victim) && IS_VAMPAFF(victim, VAM_CLAWS))
    {
	if (IS_POLYAFF(victim, POLY_ZULO))
	    act("$N has long black talons extending from $S fingers.",ch,NULL,victim,TO_CHAR);
	else if (IS_POLYAFF(victim, POLY_WOLF))
	    act("$N has razor sharp talons extending from $S paws.",ch,NULL,victim,TO_CHAR);
	else if (IS_POLYAFF(victim, POLY_BAT))
	    act("$N has tiny, razor sharp claws extending from $S wing tips.",ch,NULL,victim,TO_CHAR);
	else
	    act("$N has razor sharp claws protruding from under $S finger nails.",ch,NULL,victim,TO_CHAR);
    }
    else if (!IS_NPC(victim) && IS_VAMPAFF(victim, VAM_CLAWS))
    {
	if (IS_WEREWOLF(victim) && get_auspice(victim, AUSPICE_AHROUN) >= 3)
	    act("$N has gleaming silver talons extending from $S fingers.",ch,NULL,victim,TO_CHAR);
	else
	    act("$N has razor sharp talons extending from $S fingers.",ch,NULL,victim,TO_CHAR);
    }
    if (!IS_NPC(victim) && IS_MORE(victim, MORE_HELIOS))
	act("$N's hands are engulfed in blazing flames.",ch,NULL,victim,TO_CHAR);

    if (IS_VAMPIRE(victim))
    {
	if (IS_CRAFT(victim, CRAFT_FLESHSOAK))
	    act("$N's skin looks unnaturally tough.", ch, NULL, victim, TO_CHAR);
	if (IS_CRAFT(victim, CRAFT_BONETALONS))
	    act("$N has bone spikes slicing out from $s knuckles.", ch, NULL, victim, TO_CHAR);
	if (IS_CRAFT(victim, CRAFT_BONEQUILLS))
	    act("$N's body is covered in small quills of bone.", ch, NULL, victim, TO_CHAR);
    }

    if (IS_EXTRA(victim, EXTRA_MARK))
	act("$N has a horrible disfiguring mark on their forehead.",ch,NULL,victim,TO_CHAR); 
    if (IS_MAGE(victim) && IS_XTRA(victim, XTRA_HOLY_SMOTE))
	act("$N has a holy sword float around $S body.", ch, NULL, victim, TO_CHAR);
    if (IS_WEREWOLF(victim) && get_tribe(victim, TRIBE_WENDIGOS) >= 5 && IS_OUTSIDE(victim) && IS_SET(world_affects, WORLD_RAIN))
	act("$N is surrounded by a crackling aura of lightning!", ch, NULL, victim, TO_CHAR);

    found = FALSE;
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
	if ( ( obj = get_eq_char( victim, iWear ) ) != NULL
	&&   can_see_obj( ch, obj ) )
	{
	    if ( !found )
	    {
		send_to_char( "\n\r", ch );
		act( "$N is using:", ch, NULL, victim, TO_CHAR );
		found = TRUE;
	    }
	    send_to_char( where_name[iWear], ch );
	    if (IS_NPC(ch) || ch->pcdata->chobj == NULL || ch->pcdata->chobj != obj)
	    {
	    	send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
	    	send_to_char( "\n\r", ch );
	    }
	    else
		send_to_char( "you\n\r", ch);
	}
    }
    return;
}



void show_char_to_char( CHAR_DATA *list, CHAR_DATA *ch )
{
    CHAR_DATA *rch;

    for ( rch = list; rch != NULL; rch = rch->next_in_room )
    {
	if ( rch == ch )
	    continue;

	if ( IS_IMM(ch) || IS_IMM(rch) )
	{
	    if (!IS_NPC(rch) && IS_SET(rch->act, PLR_WIZINVIS) && get_trust(ch) < get_trust(rch))
		continue;
	}
	else
	{
	    if ( !IS_NPC(rch)
	    && ( IS_SET(rch->act, PLR_WIZINVIS)
	    ||   IS_ITEMAFF(rch, ITEMA_VANISH) )
	    &&   !IS_SET(ch->act, PLR_HOLYLIGHT)
	    &&   !IS_ITEMAFF(ch, ITEMA_VISION) )
		continue;
	}

	if ( !IS_NPC(rch) )
	{
	    if (IS_MORE(rch,MORE_HEAD)
		|| IS_EXTRA(rch,EXTRA_OSWITCH)
		|| IS_VAMPAFF(rch, VAM_EARTHMELDED)) continue;
	}

	if ( can_see( ch, rch ) )
	    show_char_to_char_0( rch, ch );
	else if ( room_is_dark( ch->in_room )
	&&      ( IS_AFFECTED(rch, AFF_INFRARED) || IS_VAMPAFF(rch, VAM_NIGHTSIGHT)) )
	{
	    if (IS_WEREWOLF(rch) && IS_VAMPAFF(rch, VAM_NIGHTSIGHT))
		send_to_char( "You see glowing green eyes watching YOU!\n\r", ch );
	    else
		send_to_char( "You see glowing red eyes watching YOU!\n\r", ch );
	}
    }

    return;
} 



bool check_blind( CHAR_DATA *ch )
{
    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT) 
       && !IS_EXTRA(ch, BLINDFOLDED) )
	return TRUE;

    if ( IS_ITEMAFF(ch, ITEMA_VISION) )
	return TRUE;

    if ( IS_EXTRA(ch, BLINDFOLDED) )
    {
	send_to_char( "You can't see a thing through the blindfold!\n\r", ch );
	return FALSE;
    }

    if ( IS_AFFECTED(ch, AFF_BLIND) && !IS_AFFECTED(ch, AFF_SHADOWSIGHT))
    {
	send_to_char( "You can't see a thing!\n\r", ch );
	return FALSE;
    }

    return TRUE;
}



void do_look( CHAR_DATA *ch, char *argument )
{
    char buf  [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    EXIT_DATA *pexit;
    CHAR_DATA *victim;
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    CHAR_DATA *wizard;
    OBJ_DATA *obj;
    OBJ_DATA *portal;
    OBJ_DATA *portal_next;
    ROOM_INDEX_DATA *pRoomIndex;
    ROOM_INDEX_DATA *location;
    char *pdesc;
    long door;
    bool found;

    if (ch->desc == NULL && (wizard = ch->wizard) == NULL) return;

    if (ch->in_room == NULL) return;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (IS_VAMPIRE(ch) && get_disc(ch,DISC_PROTEAN) >= 7 &&  ch->position < POS_SLEEPING)
	WAIT_STATE(ch,12);
    else if (ch->position < POS_SLEEPING)
    {
	send_to_char( "You can't see anything but stars!\n\r", ch );
	return;
    }

    if ( ch->position == POS_SLEEPING )
    {
	send_to_char( "You can't see anything, you're sleeping!\n\r", ch );
	return;
    }

    if (!check_blind(ch))
	return;

    if (!IS_NPC(ch) && ch->in_room != NULL && IS_SET(ch->in_room->added_flags, ROOM2_DARKNESS) &&
	!IS_ITEMAFF(ch, ITEMA_VISION) && !IS_BUI(ch))
    {
	if (!IS_VAMPIRE(ch) || get_disc(ch,DISC_OBTENEBRATION) == 0)
	{
	    if (!IS_WEREWOLF(ch) || get_tribe(ch, TRIBE_UKTENA) < 1)
	    {
		if (IS_NPC(ch))
		{
		    send_to_char( "It is pitch black ... \n\r", ch );
		    return;
		}
	    }
	}
    }
    
    if (!IS_BUI(ch))
    {
	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
	{
	    if (IS_DROW(vch) && get_drow(ch, DROW_GLOBE))
	    {
		stc("A pitch black darkness pervades the room.\n\r", ch);
		return;
	    }
	}
    }

    if ( !IS_NPC(ch)
    &&   !IS_SET(ch->act, PLR_HOLYLIGHT)
    &&   !IS_ITEMAFF(ch, ITEMA_VISION)
    &&   !IS_VAMPAFF(ch, VAM_NIGHTSIGHT)
    &&   !IS_AFFECTED(ch, AFF_SHADOWPLANE)
    &&   !(ch->in_room != NULL && ch->in_room->vnum == ROOM_VNUM_IN_OBJECT
    &&   !IS_NPC(ch) && ch->pcdata->chobj != NULL
    &&   ch->pcdata->chobj->in_obj != NULL)
    &&   room_is_dark( ch->in_room ) 
    &&   !IS_BUI(ch))
    {
	send_to_char( "It is pitch black ... \n\r", ch );
	show_char_to_char( ch->in_room->people, ch );
	return;
    }

    if (!IS_BUI(ch) && IS_EXTRA(ch, BLINDFOLDED))
    {
        send_to_char( "It is pitch black.....\n\r", ch);
        return;
    }

    if ( arg1[0] == '\0' || !str_cmp( arg1, "auto" ) )
    {
	/* 'look' or 'look auto' */
	if (ch->in_room != NULL && ch->in_room->vnum == ROOM_VNUM_IN_OBJECT
	&& !IS_NPC(ch) && ch->pcdata->chobj != NULL && ch->pcdata->chobj->in_obj != NULL)
	    act( "$p",ch,ch->pcdata->chobj->in_obj,NULL,TO_CHAR);
    	else if ( IS_AFFECTED(ch, AFF_SHADOWPLANE) && !IS_AFFECTED(ch,AFF_SHADOWSIGHT))
	    send_to_char( "The Umbra\n\r", ch );
        else if ( IS_AFFECTED(ch, AFF_SHADOWPLANE) && !IS_AFFECTED(ch, AFF_DEEPUMBRA) && IS_AFFECTED(ch,AFF_SHADOWSIGHT))
	{
	    if (IS_BUI(ch)) sprintf(buf, "#g%s#n (Umbra) #l[#c%li#l]#n\n\r", ch->in_room->name, ch->in_room->vnum);
	    else sprintf(buf, "#g%s#n (Umbra)\n\r", ch->in_room->name);
	    send_to_char( buf, ch );
	}
	else if ( IS_AFFECTED(ch, AFF_SHADOWPLANE) && IS_AFFECTED(ch, AFF_DEEPUMBRA) && IS_AFFECTED(ch,AFF_SHADOWSIGHT))
	{
            if (IS_BUI(ch)) sprintf(buf, "#g%s#n (Deep Umbra) #l[#c%li#l]#n\n\r", ch->in_room->name, ch->in_room->vnum);
            else sprintf(buf, "#g%s#n (Deep Umbra)\n\r", ch->in_room->name);
            send_to_char( buf, ch );
        }

	else
	{
	    if (IS_BUI(ch)) sprintf(buf, "#g%s  #l[#c%li#l]#n\n\r", ch->in_room->name, ch->in_room->vnum);
	    else sprintf(buf, "#g%s#n\n\r", ch->in_room->name);
	    send_to_char( buf, ch );
	}

	if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOEXIT) )
	    do_exits( ch, "auto" );

	if (ch->in_room != NULL && ch->in_room->vnum == ROOM_VNUM_IN_OBJECT
	&& !IS_NPC(ch) && ch->pcdata->chobj != NULL && ch->pcdata->chobj->in_obj != NULL)
	{
	    act( "You are inside $p.",ch,ch->pcdata->chobj->in_obj,NULL,TO_CHAR);
	    show_list_to_char( ch->pcdata->chobj->in_obj->contains, ch, FALSE, FALSE );
	}
	else if ( (arg1[0] == '\0' || !str_cmp( arg1, "auto" ) ) && IS_AFFECTED(ch, AFF_SHADOWPLANE) && !IS_AFFECTED(ch,AFF_SHADOWSIGHT))
	    send_to_char( "You are standing in complete darkness.\n\r", ch );
        
        else if ( ( !IS_NPC(ch) && !IS_SET(ch->act, PLR_BRIEF) ) &&
		( arg1[0] == '\0' || !str_cmp( arg1, "auto" ) ) )
	{
	    send_to_char( ch->in_room->description, ch );
	    if (ch->in_room->blood == 1000)
		sprintf(buf,"You notice that the room is completely drenched in blood.\n\r");
	    else if (ch->in_room->blood > 750)
		sprintf(buf,"You notice that there is a very large amount of blood around the room.\n\r");
	    else if (ch->in_room->blood > 500)
		sprintf(buf,"You notice that there is a large quantity of blood around the room.\n\r");
	    else if (ch->in_room->blood > 250)
		sprintf(buf,"You notice a fair amount of blood on the floor.\n\r");
	    else if (ch->in_room->blood > 100)
		sprintf(buf,"You notice several blood stains on the floor.\n\r");
	    else if (ch->in_room->blood > 50)
		sprintf(buf,"You notice a few blood stains on the floor.\n\r");
	    else if (ch->in_room->blood > 25)
		sprintf(buf,"You notice a couple of blood stains on the floor.\n\r");
	    else if (ch->in_room->blood > 0)
		sprintf(buf,"You notice a few drops of blood on the floor.\n\r");
	    else sprintf(buf,"You notice nothing special in the room.\n\r");
	    ADD_COLOUR(ch, buf, L_RED);
	    if (ch->in_room->blood > 0) send_to_char(buf,ch);
	}

	show_list_to_char( ch->in_room->contents, ch, FALSE, FALSE );
	show_char_to_char( ch->in_room->people,   ch );
	return;
    }

    if ( !str_cmp( arg1, "i" ) || !str_cmp( arg1, "in" ) )
    {
	/* 'look in' */
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "Look in what?\n\r", ch );
	    return;
	}

	if ( ( obj = get_obj_here( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "You do not see that here.\n\r", ch );
	    return;
	}

	switch ( obj->item_type )
	{
	default:
	    send_to_char( "That is not a container.\n\r", ch );
	    break;

	case ITEM_PORTAL:
        case ITEM_WGATE:
	    pRoomIndex = get_room_index(obj->value[0]);
	    location = ch->in_room;
	    if ( pRoomIndex == NULL )
	    {
		send_to_char( "It doesn't seem to lead anywhere.\n\r", ch );
		return;
	    }
	    if (obj->value[2] == 1 || obj->value[2] == 3)
	    {
		send_to_char( "It seems to be closed.\n\r", ch );
		return;
	    }
            if ( obj->item_type == ITEM_WGATE)
            {
               stc("You gaze into the rift, but can see nothing.\n\r",ch);
               return;
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
                    if ( obj->item_type == ITEM_WGATE )
                    {
                        stc("You peer through the magical rift...\n\r",ch);
                    }
		    if (IS_AFFECTED(ch, AFF_SHADOWPLANE) &&
			!IS_SET(portal->extra_flags, ITEM_SHADOWPLANE) )
		    {
			REMOVE_BIT(ch->affected_by, AFF_SHADOWPLANE);
	    		do_look(ch,"auto");
			SET_BIT(ch->affected_by, AFF_SHADOWPLANE);
			break;
		    }
		    else if (!IS_AFFECTED(ch, AFF_SHADOWPLANE) &&
			IS_SET(portal->extra_flags, ITEM_SHADOWPLANE) )
		    {
			SET_BIT(ch->affected_by, AFF_SHADOWPLANE);
	    		do_look(ch,"auto");
			REMOVE_BIT(ch->affected_by, AFF_SHADOWPLANE);
			break;
		    }
		    else
		    {
	    		do_look(ch,"auto");
			break;
		    }
	    	}
	    }
	    char_from_room(ch);
	    char_to_room(ch,location);
	    break;

	case ITEM_DRINK_CON:
	    if ( obj->value[1] <= 0 )
	    {
		send_to_char( "It is empty.\n\r", ch );
		break;
	    }

	    if (obj->value[1] < obj->value[0] / 5)
	    	sprintf( buf, "There is a little %s liquid left in it.\n\r",liq_table[obj->value[2]].liq_color);
	    else if (obj->value[1] < obj->value[0] / 4)
	    	sprintf( buf, "It contains a small about of %s liquid.\n\r",liq_table[obj->value[2]].liq_color);
	    else if (obj->value[1] < obj->value[0] / 3)
	    	sprintf( buf, "It's about a third full of %s liquid.\n\r",liq_table[obj->value[2]].liq_color);
	    else if (obj->value[1] < obj->value[0] / 2)
	    	sprintf( buf, "It's about half full of %s liquid.\n\r",liq_table[obj->value[2]].liq_color);
	    else if (obj->value[1] < obj->value[0])
	    	sprintf( buf, "It is almost full of %s liquid.\n\r",liq_table[obj->value[2]].liq_color);
	    else if (obj->value[1] == obj->value[0])
	    	sprintf( buf, "It's completely full of %s liquid.\n\r",liq_table[obj->value[2]].liq_color);
	    else
	    	sprintf( buf, "Somehow it is MORE than full of %s liquid.\n\r",liq_table[obj->value[2]].liq_color);
	    send_to_char( buf, ch );
	    break;

	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	    if ( IS_SET(obj->value[1], CONT_CLOSED) )
	    {
		send_to_char( "It is closed.\n\r", ch );
		break;
	    }

	    act( "$p contains:", ch, obj, NULL, TO_CHAR );
	    show_list_to_char( obj->contains, ch, TRUE, TRUE );
	    break;
	}
	return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) != NULL )
    {
	show_char_to_char_1( victim, ch );
	return;
    }

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
	vch_next	= vch->next;
	if ( vch->in_room == NULL )
	    continue;
	if ( vch->in_room == ch->in_room )
	{
	    if (!IS_NPC(vch) && !str_cmp(arg1,vch->morph))
	    {
		show_char_to_char_1( vch, ch );
		return;
	    }
	    continue;
	}
    }

    if (!IS_NPC(ch) && ch->pcdata->chobj != NULL && ch->pcdata->chobj->in_obj != NULL)
    {
	obj = get_obj_in_obj(ch,arg1);
	if (obj != NULL)
	{
	    send_to_char( obj->description, ch );
	    send_to_char( "\n\r", ch );
	    return;
	}
    }

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if (!IS_NPC(ch) && ch->pcdata->chobj != NULL && obj->chobj != NULL && obj->chobj == ch)
	    continue;
	if ( can_see_obj( ch, obj ) )
	{
	    pdesc = get_extra_descr( arg1, obj->extra_descr );
	    if ( pdesc != NULL )
	    {
		send_to_char( pdesc, ch );
		return;
	    }

	    pdesc = get_extra_descr( arg1, obj->pIndexData->extra_descr );
	    if ( pdesc != NULL )
	    {
		send_to_char( pdesc, ch );
		return;
	    }
	}

	if ( is_name( arg1, obj->name ) )
	{
	    send_to_char( obj->description, ch );
	    send_to_char( "\n\r", ch );
	    return;
	}
    }

    for ( obj = ch->in_room->contents; obj != NULL; obj = obj->next_content )
    {
	if (!IS_NPC(ch) && ch->pcdata->chobj != NULL && obj->chobj != NULL && obj->chobj == ch)
	    continue;
	if ( can_see_obj( ch, obj ) )
	{
	    pdesc = get_extra_descr( arg1, obj->extra_descr );
	    if ( pdesc != NULL )
	    {
		send_to_char( pdesc, ch );
		return;
	    }

	    pdesc = get_extra_descr( arg1, obj->pIndexData->extra_descr );
	    if ( pdesc != NULL )
	    {
		send_to_char( pdesc, ch );
		return;
	    }
	}

	if ( is_name( arg1, obj->name ) )
	{
	    send_to_char( obj->description, ch );
	    send_to_char( "\n\r", ch );
	    return;
	}
    }

    pdesc = get_extra_descr( arg1, ch->in_room->extra_descr );
    if ( pdesc != NULL )
    {
	send_to_char( pdesc, ch );
	return;
    }

         if ( !str_cmp( arg1, "n" ) || !str_cmp( arg1, "north" ) ) door = 0;
    else if ( !str_cmp( arg1, "e" ) || !str_cmp( arg1, "east"  ) ) door = 1;
    else if ( !str_cmp( arg1, "s" ) || !str_cmp( arg1, "south" ) ) door = 2;
    else if ( !str_cmp( arg1, "w" ) || !str_cmp( arg1, "west"  ) ) door = 3;
    else if ( !str_cmp( arg1, "u" ) || !str_cmp( arg1, "up"    ) ) door = 4;
    else if ( !str_cmp( arg1, "d" ) || !str_cmp( arg1, "down"  ) ) door = 5;
    else
    {
	send_to_char( "You do not see that here.\n\r", ch );
	return;
    }

    /* 'look direction' */
    if ( ( pexit = ch->in_room->exit[door] ) == NULL )
    {
	send_to_char( "Nothing special there.\n\r", ch );
	return;
    }
/*
    if ( pexit->description != NULL && pexit->description[0] != '\0' )
	send_to_char( pexit->description, ch );
    else
	send_to_char( "Nothing special there.\n\r", ch );
*/

    if ( pexit->keyword    != NULL
    &&   pexit->keyword[0] != '\0'
    &&   pexit->keyword[0] != ' ' )
    {
	if ( IS_SET(pexit->exit_info, EX_CLOSED) )
	{
	    act( "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
	}
	else if ( IS_SET(pexit->exit_info, EX_ISDOOR) )
	{
	    act( "The $d is open.",   ch, NULL, pexit->keyword, TO_CHAR );
	    if ((pexit = ch->in_room->exit[door]) == NULL) return;
	    if ((pRoomIndex = pexit->to_room) == NULL) return;
	    location = ch->in_room;
	    char_from_room(ch);
	    char_to_room(ch,pRoomIndex);
	    do_look(ch,"auto");
	    char_from_room(ch);
	    char_to_room(ch,location);
	}
	else
	{
	    if ((pexit = ch->in_room->exit[door]) == NULL) return;
	    if ((pRoomIndex = pexit->to_room) == NULL) return;
	    location = ch->in_room;
	    char_from_room(ch);
	    char_to_room(ch,pRoomIndex);
	    do_look(ch,"auto");
	    char_from_room(ch);
	    char_to_room(ch,location);
	}
    }
    else
    {
	if ((pexit = ch->in_room->exit[door]) == NULL) return;
	if ((pRoomIndex = pexit->to_room) == NULL) return;
	location = ch->in_room;
	char_from_room(ch);
	char_to_room(ch,pRoomIndex);
	do_look(ch,"auto");
	char_from_room(ch);
	char_to_room(ch,location);
    }

    return;
}



void do_examine( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Examine what?\n\r", ch );
	return;
    }

    do_look( ch, arg );

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	switch ( obj->item_type )
	{
	default:
	    break;

	case ITEM_DRINK_CON:
	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	    send_to_char( "When you look inside, you see:\n\r", ch );
	    sprintf( buf, "in %s", arg );
	    do_look( ch, buf );
	}
    }

    return;
}



/*
 * Thanks to Zrin for auto-exit part.
 */
void do_exits( CHAR_DATA *ch, char *argument )
{
    extern char * const dir_name[];
    char buf[MAX_STRING_LENGTH];
    EXIT_DATA *pexit;
    bool found;
    bool fAuto;
    long door;

    buf[0] = '\0';
    fAuto  = !str_cmp(argument, "auto");

    if (IS_NPC(ch) || !check_blind(ch)) return;

    strcpy( buf, fAuto ? "#r[#yExits#r:#y" : "#yObvious exits#r:#n\n\r" );

    found = FALSE;
    for ( door = 0; door <= 5; door++ )
    {
	if ( ( pexit = ch->in_room->exit[door] ) != NULL
	&&   pexit->to_room != NULL
	&&   !IS_SET(pexit->exit_info, EX_CLOSED) )
	{
	    found = TRUE;
	    if ( fAuto )
	    {
		strcat( buf, " " );
		strcat( buf, dir_name[door] );
	    }
	    else
	    {
		if (IS_BUI(ch))
		    sprintf(buf + strlen(buf), "#y%-5s #r-#n %s #0[#7%li#0]#n\n\r", capitalize(dir_name[door]), pexit->to_room->name, pexit->to_room->vnum);
		else if (IS_SET(world_affects, WORLD_FOG) && IS_OUTSIDE(ch))
		    sprintf(buf + strlen(buf), "#y%-5s #r-#n Too foggy to tell\n\r", capitalize(dir_name[door]));
		else if (room_is_dark(pexit->to_room) && !IS_VAMPAFF(ch, VAM_NIGHTSIGHT))
		    sprintf(buf + strlen(buf), "#y%-5s #r-#n Too dark to tell\n\r", capitalize(dir_name[door]));
		else
		    sprintf(buf + strlen(buf), "#y%-5s #r-#n %s\n\r", capitalize(dir_name[door]), pexit->to_room->name);
	    }
	}
    }

    if ( !found )
	strcat( buf, fAuto ? " #ynone" : "None.\n\r" );

    if ( fAuto )
	strcat( buf, "#r]#n\n\r" );

    send_to_char( buf, ch );
    return;
}

/* Score command redone - Kai */
void do_score( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char ss1[MAX_STRING_LENGTH];
    char ss2[MAX_STRING_LENGTH];

    if (IS_NPC(ch)) return;

    if (IS_EXTRA(ch,EXTRA_OSWITCH) || IS_MORE(ch, MORE_HEAD))
	{obj_score(ch,ch->pcdata->chobj);return;}
    sprintf( buf, "%s%s #nis here.\n\r", ch->name, ch->pcdata->title);
    send_to_char( buf, ch );
    send_to_char("#l-----------------------------------------------------------------------#n\n\r",ch);
    sprintf( buf, "Character Created: %s\n\r", ch->createtime );
    send_to_char( buf, ch );
    sprintf( buf, "You have been playing for %li hours.\n\r", get_hours(ch));
    send_to_char( buf, ch );
    send_to_char("#l-----------------------------------------------------------------------#n\n\r",ch);
    sprintf(buf,"Str: %2i/%-2i Dex: %2i/%-2i Con: %2i/%-2i Int: %2i/%-2i Wis: %2i/%-2i\n\r\n\r",
	char_stat(ch, STAT_STR), raw_stat(ch, STAT_STR), char_stat(ch, STAT_DEX), raw_stat(ch, STAT_DEX), 
	char_stat(ch, STAT_CON), raw_stat(ch, STAT_CON), char_stat(ch, STAT_INT), raw_stat(ch, STAT_INT), 
	char_stat(ch, STAT_WIS), raw_stat(ch, STAT_WIS));
    stc(buf,ch);
    sprintf(buf, "Hit Points:   %6li/%-13li  Experience: %li\n\r", ch->hit, ch->max_hit, ch->exp);
    send_to_char( buf, ch );
    sprintf(buf, "Mana:         %6li/%-13li  Primal: %li\n\r", ch->mana, ch->max_mana, ch->primal);
    send_to_char( buf, ch );
    sprintf(buf, "Quest Points: %-15li       Quests Completed: %li\n\r", ch->pcdata->quest, ch->pcdata->score[SCORE_NUM_QUEST]);
    send_to_char( buf, ch );
    sprintf(buf, "PK Score: %li\n\r\n\r", get_ratio(ch));
    send_to_char( buf, ch );

    sprintf(buf, "Hitroll: %-8li Damroll: %-8li Armor: %li\n\r", char_hitroll(ch), char_damroll(ch), char_ac(ch));
    stc(buf, ch);
    sprintf(buf, "Speed:   %-8li Avoid:   %-8li Break: %li\n\r", get_speed(ch) - 5, avoidblow(ch), breakavoid(ch));
    stc(buf, ch);
    sprintf(buf, "Damcap:  %-8li Soak:    %-8li\n\r", get_damcap(ch), get_soak(ch));
    stc(buf, ch);

    if (IS_VAMPIRE(ch))
    {
        sprintf( buf, "Blood: %5li/%-5li.\n\r", ch->blood[0], ch->blood[1]);
        send_to_char( buf, ch );
        sprintf( buf, "Beast: %-8li ", ch->beast );
        send_to_char( buf, ch );
        if (ch->beast <= 0 ) send_to_char("You have attained Golconda!\n\r",ch);
        else if (ch->beast <= 5 ) send_to_char("You have almost reached Golconda!\n\r",ch);
        else if (ch->beast <= 10) send_to_char("You are nearing Golconda!\n\r",ch);
        else if (ch->beast <= 15) send_to_char("You have great control over your beast.\n\r",ch);
        else if (ch->beast <= 20) send_to_char("Your beast has little influence over your actions.\n\r",ch);
        else if (ch->beast <= 30) send_to_char("You are in control of your beast.\n\r",ch);
        else if (ch->beast <= 40) send_to_char("You are able to hold back the beast.\n\r",ch);
        else if (ch->beast <= 60) send_to_char("You are constantly struggling for control of your beast.\n\r",ch);
        else if (ch->beast <= 75) send_to_char("Your beast has great control over your actions.\n\r",ch);
        else if (ch->beast <= 90) send_to_char("The power of the beast overwhelms you.\n\r",ch);
        else if (ch->beast <= 99) send_to_char("You have almost lost your battle with the beast!\n\r",ch);
        else                      send_to_char("The beast has taken over!\n\r",ch);
    }

    if (!IS_NPC(ch))
    {
        sprintf( buf, "Status:  %-8li ", ch->status );
        send_to_char( buf, ch );
        send_to_char( "You are ", ch );
        if (IS_ST(ch)) send_to_char("a Storyteller.\n\r", ch);
        else if (IS_IMP(ch)) send_to_char("an Implementor.\n\r", ch);
        else if (IS_IMM(ch)) send_to_char("an Immortal.\n\r", ch);
        else if (IS_BUI(ch)) send_to_char("a Builder.\n\r", ch);
        else if (!IS_AWA(ch)) send_to_char("a Sleeper.\n\r", ch);
        else if (ch->status <= 0) send_to_char("a Mortal.\n\r", ch);
        else if (ch->status <= 4) send_to_char("an Avatar.\n\r", ch);
        else if (ch->status <= 9) send_to_char("a Godling.\n\r", ch);
        else if (ch->status <= 14) send_to_char("a Demigod.\n\r", ch);
        else if (ch->status <= 19) send_to_char("a Lesser God.\n\r", ch);
        else if (ch->status <= 24) send_to_char("a Greater God.\n\r", ch);
        else send_to_char("a Supreme God.\n\r", ch);
    }
    send_to_char("#l-----------------------------------------------------------------------#n\n\r",ch);

    if ( !IS_NPC(ch) )
    {
        if      (ch->pkill  == 0) sprintf(ss1,"no players");
        else if (ch->pkill  == 1) sprintf(ss1,"%li player",ch->pkill);
        else                      sprintf(ss1,"%li players",ch->pkill);
        if      (ch->pdeath == 0) sprintf(ss2,"no players");
        else if (ch->pdeath == 1) sprintf(ss2,"%li player",ch->pdeath);
        else                      sprintf(ss2,"%li players",ch->pdeath);
        sprintf( buf, "You have killed %s and have been killed by %s.\n\r", ss1, ss2 );
        send_to_char( buf, ch );
        if      (ch->mkill  == 0) sprintf(ss1,"no mobs");
        else if (ch->mkill  == 1) sprintf(ss1,"%li mob",ch->mkill);
        else                      sprintf(ss1,"%li mobs",ch->mkill);
        if      (ch->mdeath == 0) sprintf(ss2,"no mobs");
        else if (ch->mdeath == 1) sprintf(ss2,"%li mob",ch->mdeath);
        else                      sprintf(ss2,"%li mobs",ch->mdeath);
        sprintf( buf, "You have killed %s and have been killed by %s.\n\r", ss1, ss2 );
        send_to_char( buf, ch );
    }

    stc("#l-----------------------------------------------------------------------#n\n\r", ch);

    if (IS_AFFECTED(ch, AFF_HIDE)) stc("You are keeping yourself hidden from those around you.\n\r", ch);

    if (!IS_NPC(ch))
    {
	if (IS_DRAGON(ch))
	{
	    sprintf(buf, "You currently have %li dragon points.\n\rYou have earned a total of %li dragon points in your lifetime.\n\r",ch->pcdata->classpoints[CP_CURRENT], ch->pcdata->classpoints[CP_TOTAL]);
	    stc(buf, ch);
	}
	if (IS_DROW(ch))
	{
	    sprintf(buf, "You currently have %li drow points.\n\rYou have earned a total of %li drow points in your lifetime.\n\r", ch->pcdata->classpoints[CP_CURRENT], ch->pcdata->classpoints[CP_TOTAL]);
	    stc(buf, ch);
	    if (get_drow(ch, DROW_GLOBE)) stc("You are currently using Globe of Darkness.\n\r", ch);
	}
	if (IS_NINJA(ch))
	{
	    sprintf(buf, "You currently have %li out of 300 Ki stored.\n\r", ch->pcdata->ninja[NINJA_KI]);
	    stc(buf, ch);
	    if (ch->pcdata->wolf > 0)
	    {
		sprintf(buf, "Michi:  Affects Hitroll and Damroll by +%li.\n\r", ch->pcdata->wolf);
		stc(buf, ch);
	    }
	    if (ch->pcdata->ninja[NINJA_HARAKIRI] > 0)
	    {
		sprintf(buf, "You receive the power of HaraKiri for %li more ticks.\n\r", ch->pcdata->ninja[NINJA_HARAKIRI]);
		stc(buf, ch);
	    }
	}
	if (IS_VAMPIRE(ch) && ch->pcdata->wolf > 0)
	{
	    sprintf(buf, "The beast is in control of your actions:  Affects Hitroll and Damroll by +%li.\n\r", ch->pcdata->wolf);
	    stc(buf, ch);
        }

	if (IS_WEREWOLF(ch) && ch->pcdata->wolf > 0 && IS_SET(ch->act, PLR_WOLFMAN))
	{
	    sprintf(buf, "You are raging:  Affects Hitroll and Damroll by +%li.\n\r", ch->pcdata->wolf);
	    stc(buf, ch);
	}

	if (ch->fight_timer > 0)
	{
	    sprintf(buf, "You have %li seconds left on your fight timer.\n\r", ch->fight_timer);
	    stc(buf, ch);
	}

	if (IS_MAGE(ch))
	{
	    sprintf(buf, "You have %li points of paradox accumulated.\n\r", ch->paradox);
	    stc(buf, ch);
	    sprintf(buf, "You have %li point of quintessence stored.\n\r", ch->quint[1]);
	    stc(buf, ch);
	    sprintf(buf, "Arete: %li.\n\r", ch->arete);
	    stc(buf, ch);
	}

	if (IS_MORE(ch, MORE_FAVOUR))
	    stc("You have been blessed by the almighty.\n\r", ch);

	if (IS_XTRA(ch, XTRA_BETTER_STR))
	{
	    sprintf(buf, "You are affected by better body[str] for : %li.\n\r", ch->better_body_timer);
	    stc(buf, ch);
	}
	if (IS_XTRA(ch, XTRA_BETTER_DEX))
	{
	    sprintf(buf, "You are affected by better body[dex] for : %li.\n\r", ch->better_body_timer);
	    stc(buf, ch);
	}
	if (IS_XTRA(ch, XTRA_BETTER_CON))
	{
	    sprintf(buf, "You are affected by better body[con] for : %li.\n\r", ch->better_body_timer);
	    stc(buf, ch);
	}
	if (IS_XTRA(ch, XTRA_BETTER_INT))
	{
	    sprintf(buf, "You are affected by better body[int] for : %li.\n\r", ch->better_body_timer);
	    stc(buf, ch);
	}
	if (IS_XTRA(ch, XTRA_BETTER_WIS))
	{
	    sprintf(buf, "You are affected by better body[wis] for : %li.\n\r", ch->better_body_timer);
	    stc(buf, ch);
	}
	if (ch->pcdata->atm > 0)
	{
	    sprintf(buf,"You have %li percent magic resistance.\n\r",ch->pcdata->atm);
	    stc(buf, ch);
	}
    }
    return;
}

char *	const	day_name	[] =
{
    "the Moon", "the Bull", "Deception", "Thunder", "Freedom",
    "the Great Gods", "the Sun"
};

char *	const	month_name	[] =
{
    "Winter", "the Winter Wolf", "the Frost Giant", "the Old Forces",
    "the Grand Struggle", "the Spring", "Nature", "Futility", "the Dragon",
    "the Sun", "the Heat", "the Battle", "the Dark Shades", "the Shadows",
    "the Long Shadows", "the Ancient Darkness", "the Great Evil"
};

void do_time( CHAR_DATA *ch, char *argument )
{
    extern char str_boot_time[];
    char buf[MAX_STRING_LENGTH];
    char *suf;
    long day;

    day     = time_info.day + 1;

    if (day > 4 && day <  20)	suf = "th";
    else if (day % 10 == 1)	suf = "st";
    else if (day % 10 == 2)	suf = "nd";
    else if (day % 10 == 3)	suf = "rd";
    else			suf = "th";

    sprintf(buf, "It is %li o'clock %s, Day of %s, %li%s the Month of %s.\n\rGW: Genesis started up at %s\rThe system time is %s\r",
	(time_info.hour % 12 == 0) ? 12 : time_info.hour % 12, time_info.hour >= 12 ? "pm" : "am", day_name[day % 7],
	day, suf, month_name[time_info.month], str_boot_time, (char *) ctime(&current_time));

    send_to_char(buf, ch);
    return;
}



void do_weather( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    static char * const sky_look[4] =
    {
	"cloudless",
	"cloudy",
	"rainy",
	"lit by flashes of lightning"
    };

    if ( !IS_OUTSIDE(ch) )
    {
	send_to_char( "You can't see the weather indoors.\n\r", ch );
	return;
    }

    if ( IS_SET(world_affects, WORLD_ECLIPSE) )
	send_to_char( "The sun is eclipsed by the moon.\n\r", ch );

    else if ( IS_SET(world_affects, WORLD_FOG) )
	send_to_char( "A thick blanket of fog covers the ground.\n\r", ch );

    if ( IS_SET(world_affects, WORLD_RAIN) )
    {
	sprintf( buf, "The sky is pouring with a heavy rain and %s.\n\r",
	weather_info.change >= 0
	? "a warm southerly breeze blows"
	: "a cold northern gust blows"
	);
	send_to_char( buf, ch );
	return;
    }

    sprintf( buf, "The sky is %s and %s.\n\r",
	sky_look[weather_info.sky],
	weather_info.change >= 0
	? "a warm southerly breeze blows"
	: "a cold northern gust blows"
	);
    send_to_char( buf, ch );
    return;
}



void do_help( CHAR_DATA *ch, char *argument )
{
    char argall[MIL];
    char argone[MIL];
    HELP_DATA *pHelp;

    if (argument[0] == '\0')
	argument = "summary";

    /*
     * Tricky argument handling so 'help a b' doesn't match a.
     */
    argall[0] = '\0';
    while (argument[0] != '\0')
    {
	argument = one_argument(argument, argone);
	if (argall[0] != '\0')
	    strcat(argall, " ");
	strcat(argall, argone);
    }

    for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
    {
	if (pHelp->level > get_trust(ch)) continue;
	if (!is_abbr(argall, pHelp->keyword)) continue;

	if (pHelp->level >= 0 && str_cmp(argall, "imotd"))
	{
	    stc(pHelp->keyword, ch);
	    stc("\n\r", ch);
	}

	/*
	 * Strip leading '.' to allow initial blanks.
	 */
	if (pHelp->text[0] == '.') stc(pHelp->text+1, ch);
	else stc(pHelp->text  , ch);
	return;
    }

    stc("No help on that word.\n\r", ch);
    return;
}



/*
 * New 'who' command originally by Alander of Rivers of Mud.
 * Redone by Kai.
 */
void do_who( CHAR_DATA *ch, char *argument )
{
    char buf[MSL];
    char mort[MSL];
    char avat[MSL];
    char immo[MSL];
    char kav[MSL];
    char arg[MIL];
    char openb[20];
    char closeb[20];
    DESCRIPTOR_DATA *d;
    long nTotal = 0, mor = 0, ava = 0, imm = 0, clas = 0;
    bool isPK = FALSE;

    smash_tilde(argument);
    one_argument(argument, arg);

    if (IS_NPC(ch)) return;

    if (arg[0] != '\0')
    {
	if (!str_cmp(arg, "pk")) isPK = TRUE;
	else if (!str_cmp(arg, "vampire")) clas = CLASS_VAMPIRE;
	else if (!str_cmp(arg, "werewolf")) clas = CLASS_WEREWOLF;
	else if (!str_cmp(arg, "sorcerer")) clas = CLASS_SORCERER;
	else if (!str_cmp(arg, "drow")) clas = CLASS_DROW;
	else if (!str_cmp(arg, "highlander")) clas = CLASS_HIGHLANDER;
	else if (!str_cmp(arg, "mage")) clas = CLASS_MAGE;
	else if (!str_cmp(arg, "ninja")) clas = CLASS_NINJA;
	else if (!str_cmp(arg, "monk")) clas = CLASS_MONK;
	else if (!str_cmp(arg, "elementalist")) clas = CLASS_ELEM;
	else if (!str_cmp(arg, "dragon")) clas = CLASS_DRAGON;
	else
	{
	    stc("Syntax: Invalid argument.\n\r", ch);
	    return;
	}
    }

    /*
     * Now show matching chars.
     */
    buf[0] = '\0';
    mort[0] = '\0';
    avat[0] = '\0';
    immo[0] = '\0';
    
    for (d = descriptor_list; d != NULL; d = d->next)
    {
	CHAR_DATA *wch;
	char const *class;

	/*
	 * Check for match against restrictions.
	 */
	if (d->connected != CON_PLAYING) continue;

	wch = (d->original != NULL) ? d->original : d->character;

	if (IS_IMM(wch) && !can_see(ch, wch)) continue;
	if (isPK && (!IS_FLAG(wch, FLAG_PK) || IS_BUI(wch))) continue;
	if (clas != 0 && wch->class != clas) continue;

	nTotal++;

	/*
	 * Figure out what to print for class.
	 */
	class = " ";
	if ((IS_MORE(wch,MORE_HEAD) || IS_EXTRA(wch,EXTRA_OSWITCH)) && wch->pcdata->chobj != NULL)
	{
	    switch (wch->pcdata->chobj->pIndexData->vnum)
	    {
		default: class = "#gAn Object#n"; break;
		case 12: class = "#gA Head#n"; break;
		case 30005: class = "#gA Brain#n"; break;
	    }
	}
	else switch ( wch->level )
	{
	    default: break;
	    case LEV_ST: class = "#cStory Teller#n"; break;
	    case LEV_IMP: class = "#cImplementor#n"; break;
	    case LEV_IMM: class = "#cImmortal#n"; break;
	    case LEV_BUI: class = "#cBuilder#n"; break;
	    case LEV_SOR: case LEV_AWA:
		if (IS_VAMPIRE(wch))
		{
		    if (wch->status <= 0) class = "#rThin-blood#n";
		    else if (wch->status <= 4) class = "#2Neonate#n";
		    else if (wch->status <= 9) class = "#3Ancilla#n";
		    else if (wch->status <= 14) class = "#4Elder#n";
		    else if (wch->status <= 19) class = "#5Methuselah#n";
		    else if (wch->status <= 24) class = "#6Antediluvian#n";
		    else class = "#7Caine#n";
		}
		else if (IS_WEREWOLF(wch))
		{
		    if (wch->status <= 0) class = "#rCub#n";
		    else if (wch->status <= 4) class = "#2Cliath#n";
		    else if (wch->status <= 9) class = "#3Fostern#n";
		    else if (wch->status <= 14) class = "#4Adren#n";
		    else if (wch->status <= 19) class = "#5Athro#n";
		    else if (wch->status <= 24) class = "#6Elder#n";
		    else class = "#7Chieftain#n";
		}
		else if (IS_MONK(wch))
        	{
		    if (wch->status <= 0) class = "#rBeliever#n";
		    else if (wch->status <= 4) class = "#2Faithful#n";
		    else if (wch->status <= 9) class = "#3Devoted#n";
		    else if (wch->status <= 14) class = "#4True Believer#n";
		    else if (wch->status <= 19) class = "#5Fanatic#n";
		    else if (wch->status <= 24) class = "#6Prophet#n";
		    else class = "#7Saint#n";
		}
		else if (IS_MAGE(wch))
		{
		    if (wch->status <= 0) class = "#rSleeper#n";
		    else if (wch->status <= 4) class = "#2Unschooled#n";
		    else if (wch->status <= 9) class = "#3Novice#n";
		    else if (wch->status <= 14) class = "#4Disciplined#n";
		    else if (wch->status <= 19) class = "#5Wise#n";
		    else if (wch->status <= 24) class = "#6Enlightened#n";
		    else class = "#7Transcendant#n";
		}
		else if (IS_HIGHLANDER(wch))
		{
		    if (wch->status <= 0) class = "#rAvatar#n";
		    else if (wch->status <= 4) class = "#2Swordsman#n";
		    else if (wch->status <= 9) class = "#3Bladesmaster#n";
		    else if (wch->status <= 14) class = "#4Ancient#n";
		    else if (wch->status <= 19) class = "#5Legendary#n";
		    else if (wch->status <= 24) class = "#6Mythical#n";
		    else class = "#7Divine Master#n";
		}
		else if (IS_NINJA(wch))
		{
		    if (wch->status <= 0) class = "#rShinobi#n";
		    else if (wch->status <= 4) class = "#2Genin#n";
		    else if (wch->status <= 9) class = "#3Chunin#n";
		    else if (wch->status <= 14) class = "#4Ninja#n";
		    else if (wch->status <= 19) class = "#5Shihai#n";
		    else if (wch->status <= 24) class = "#6Sensei#n";
		    else class = "#7Kami#n";
		}
		else if (IS_DROW(wch))
		{
		    if (wch->status <= 0) class = "#rAbbil#n";
		    else if (wch->status <= 4) class = "#2Sargtlin#n";
		    else if (wch->status <= 9) class = "#3Malla#n";
		    else if (wch->status <= 14) class = "#4Yathtallar#n";
		    else if (wch->status <= 19) class = "#5Ilharess#n";
		    else if (wch->status <= 24) class = "#6Ultrin#n";
		    else class = "#7Quarlsharess#n";
		}
		else if (IS_ELEM(wch))
		{
		    if (wch->status <= 0) class = "#rSleeper#n";
		    else if (wch->status <= 4) class = "#2Student#n";
		    else if (wch->status <= 9) class = "#3Apprentice#n";
		    else if (wch->status <= 14) class = "#4Learned#n";
		    else if (wch->status <= 19) class = "#5Master#n";
		    else if (wch->status <= 24) class = "#6Grand Master#n";
		    else class = "#7True Master#n";
		}
		else if (IS_SORCERER(wch))
		{
		    if (wch->status <= 0) class = "#rWild Talent#n";
		    else if(wch->status <= 4) class = "#2Apprentice#n";
		    else if(wch->status <= 9) class = "#3Adept#n";
		    else if(wch->status <= 14) class = "#4Mystic#n";
		    else if(wch->status <= 19) class = "#5Sorcerer#n";
		    else if(wch->status <= 24) class = "#6Conjurer#n";
		    else class = "#7GrandSorcerer#n";
		}
		else
		{
		    if (wch->status <= 0) class = "#rMortal#n";
		    else if (wch->status <= 4) class = "#2Avatar#n";
		    else if (wch->status <= 9) class = "#3Godling#n";
		    else if (wch->status <= 14) class = "#4Demigod#n";
		    else if (wch->status <= 19) class = "#5Lesser God#n";
		    else if (wch->status <= 24) class = "#6Greater God#n";
		    else class = "#7Supreme God#n";
		}
		break;
	    case LEV_SLE: class = "#oSleeper#n"; break;
	}
	if (!str_cmp(wch->name, "Akuma")) class = "#rBloody Hell#n";
	else if (!str_cmp(wch->name, "Chaos")) class = "#7Silly Bunny#n";
	else if (!str_cmp(wch->name, "Dude")) class = "#CUmm...#n";
	else if (!str_cmp(wch->name, "Shane")) class = "#RPsycho#n";
	else if (!str_cmp(wch->name, "Corruption")) class = "#0Fleeting#n";


	/*
	 * Format it up.
	 */
	if (IS_VAMPIRE(wch))
	{strcpy(openb, "<<"); strcpy(closeb, ">>#n");}
	else if (IS_WEREWOLF(wch))
	{strcpy(openb, "(("); strcpy(closeb, "))#n");}
	else if (IS_SORCERER(wch))
	{strcpy(openb, "{{"); strcpy(closeb, "}}#n");}
	else if (IS_HIGHLANDER(wch))
	{strcpy(openb, "-="); strcpy(closeb, "=-#n");}
	else if (IS_NINJA(wch))
	{strcpy(openb, "**"); strcpy(closeb, "**#n");}
	else if (IS_DROW(wch))
	{strcpy(openb, "<("); strcpy(closeb, ")>#n");}
	else if (IS_DRAGON(wch))
	{strcpy(openb, "::"); strcpy(closeb, "::#n");}
	else if (IS_ELEM(wch))
	{strcpy(openb, "#L*#n{"); strcpy(closeb, "#n}#L*#n");}
	else if (IS_MAGE(wch))
	{strcpy(openb, ">|"); strcpy(closeb, "|<");}
	else
	{strcpy(openb, "[["); strcpy(closeb, "]]#n");}

	if (IS_VAMPIRE(wch))
	{
	    char clanname[80];

	    if (strlen(wch->clan) < 2)
		strcpy( clanname, "Caitiff" );
	    else
		strcpy( clanname, wch->clan );
	    switch ( wch->vampgen )
	    {
		default:
		    sprintf( kav, ". %s%s Childe%s", openb, clanname, closeb );
		    break;
		case 13:
		case 12:
		    sprintf( kav, ". %s%s Neonate%s", openb, clanname, closeb );
		    break;
		case 11:
		case 10:
		case 9:
		    sprintf( kav, ". %s%s Ancilla%s", openb, clanname, closeb );
		    break;
		case 8:
		case 7:
		case 6:
		    sprintf( kav, ". %s%s Elder%s", openb, clanname, closeb );
		    break;
		case 5:
		case 4:
		case 3:
		    sprintf( kav, ". %s%s Methuselah%s", openb, clanname, closeb );
		    break;
		case 2:
		    sprintf( kav, ". %sAntediluvian%s", openb, closeb );
		    break;
		case 1:
		    sprintf( kav, ". %sMaster Vampire%s", openb, closeb );
		    break;
	    }
	}
	else if (IS_WEREWOLF(wch))
	{
	    if (wch->pcdata->rank == 6)
		sprintf(kav, ". %sGarou Chieftain%s", openb, closeb);
	    else if (wch->pcdata->rank == 5)
		sprintf(kav, ". %sGarou Elder%s", openb, closeb);
	    else if (wch->pcdata->rank == 4)
		sprintf(kav, ". %sGarou Athro%s", openb, closeb);
	    else if (wch->pcdata->rank == 3)
		sprintf(kav, ". %sGarou Adren%s", openb, closeb);
	    else if (wch->pcdata->rank == 2)
		sprintf(kav, ". %sGarou Fostern%s", openb, closeb);
	    else if (wch->pcdata->rank == 1)
		sprintf(kav, ". %sGarou Cliath%s", openb, closeb);
	    else 
		sprintf(kav, ". %sBugged Garou%s",openb, closeb);
	}
	else if (IS_DRAGON(wch))
	{
	    char *col;
	    switch (wch->pcdata->dragcol)
	    {
		default:
		case RED_MAGIC: col = "Red"; break;
		case PURPLE_MAGIC: col = "Bronze"; break;
		case BLUE_MAGIC: col = "Blue"; break;
		case GREEN_MAGIC: col = "Silver"; break;
		case YELLOW_MAGIC: col = "Gold"; break;
		case BLACK_MAGIC: col = "Black"; break;
	    }
	    if (wch->vampgen == 1)
		sprintf( kav, ". %s%s Dragon Lord%s", openb,col, closeb );
	    else
		sprintf( kav, ". %s%s Dragon%s", openb,col, closeb );
	}
	else if (IS_ELEM(wch))
	{
	    char *elem_type;
	    switch (wch->pcdata->powers[ELEM_CHOICE])
	    {
		case ELEM_FIRE:  elem_type = "#RFire#n"; break;
		case ELEM_WATER: elem_type = "#LWater#n"; break;
		case ELEM_WIND:  elem_type = "#yWind#n"; break;
		case ELEM_EARTH: elem_type = "#gEarth#n"; break;
		case ELEM_SUN:   elem_type = "#7Holy#n"; break;
		case ELEM_MOON:  elem_type = "#0Shadow#n"; break;
		case ELEM_TIME:  elem_type = "#pTime#n"; break;
		case ELEM_INFIN: elem_type = "#PInfinity#n"; break;
		default:         elem_type = "#CElemental#n"; break;
	    }
	    if (wch->vampgen == 1)
	    {
		if(wch->sex == SEX_MALE)
		   sprintf( kav, ". %s%s Master%s", openb, elem_type, closeb);
		else
		   sprintf( kav, ". %s%s Mistress%s", openb, elem_type, closeb);
	    }
	    else
		sprintf( kav, ". %s%s User%s", openb, elem_type, closeb);
	}
	else if (IS_MAGE(wch))
	{
                if (wch->vampgen == 1)
                   sprintf( kav, ". %sOracle of%s", openb, closeb );
                else if (wch->vampgen == 2)
                   sprintf( kav, ". %sArchmage of %s%s", openb, wch->clan, closeb );
                else if (wch->vampgen == 3)
                   sprintf( kav, ". %sMaster of %s%s", openb, wch->clan, closeb );
		 else if (wch->vampgen == 4)
                   sprintf( kav, ". %sAdept of %s%s", openb, wch->clan, closeb );
                else
                   sprintf( kav, ". %sApprentice of %s%s", openb, wch->clan, closeb );

	}
	else if (IS_SORCERER(wch))
	{
	    if (strlen(wch->clan) == 0) sprintf(kav, ". %sRogue Talent%s", openb, closeb);
	    else if (wch->vampgen == 1) sprintf(kav, ". %s%s Transcended%s", openb, wch->clan, closeb);
	    else if (wch->vampgen == 2) sprintf(kav, ". %s%s Master%s", openb, wch->clan, closeb);
	    else if (wch->vampgen == 3) sprintf(kav, ". %s%s Sorcerer%s", openb, wch->clan, closeb);
	    else sprintf(kav, ". %s%s Apprentice%s", openb, wch->clan, closeb);
	}
	else if (IS_MONK(wch))
	{
	    if (strlen(wch->clan) < 1) sprintf(kav, ". %sWandering Monk%s", openb, closeb);
	    else if (wch->vampgen == 1) sprintf(kav, ". %sSaint of %s%s", openb, wch->clan, closeb);
	    else if (wch->vampgen == 2) sprintf(kav, ". %s%s's Prophet%s", openb, wch->clan, closeb);
	    else if (wch->vampgen == 3) sprintf(kav, ". %s%s's Devoted%s", openb, wch->clan, closeb);
	    else sprintf(kav, ". %s%s's Faithful%s", openb, wch->clan, closeb);
	}
	else if (IS_HIGHLANDER(wch))
	{
	    if (wch->vampgen == 1) sprintf(kav,  ". %sMacLeod%s", openb, closeb);
	    else sprintf(kav, ". %sHighlander%s", openb, closeb);
	}
	else if (IS_NINJA(wch))
	{
	    if (wch->clan == NULL || strlen(wch->clan) < 1)
		sprintf(kav, ". %sNinja Outcast%s", openb, closeb);
	    else if (wch->vampgen == 1)
		sprintf(kav, ". %sKami of %s%s", openb, wch->clan, closeb);
	    else if (wch->vampgen == 2)
		sprintf(kav, ". %sShihai of %s%s", openb, wch->clan, closeb);
	    else if (wch->vampgen == 3)
		sprintf(kav, ". %sChunin of %s%s", openb, wch->clan, closeb);
	    else
		sprintf(kav, ". %sGenin of %s%s", openb, wch->clan, closeb);
	}
	else if (IS_DROW(wch))
	{
	    if (wch->vampgen == 1)
		sprintf(kav, ". %sWeb Master%s", openb, closeb);
	    else if (wch->clan == NULL || strlen(wch->clan) <= 1)
	    	sprintf(kav, ". %sBregan D'Aerthe%s", openb, closeb );
	    else if (wch->vampgen == 2)
	    {
		if (wch->sex == SEX_MALE) sprintf(kav, ". %sPatron of %s%s", openb, wch->clan, closeb);
		else sprintf(kav, ". %sMatron of %s%s", openb, wch->clan, closeb );
	    }
	    else if (wch->vampgen == 3)
	    {
		if (wch->sex == SEX_MALE) sprintf(kav, ". %sWeaponmaster of %s%s", openb, wch->clan, closeb);
		else sprintf(kav, ". %sHigh Priestess of %s%s", openb, wch->clan, closeb );
	    }
	    else if (wch->vampgen == 4)
		sprintf(kav, ". %sNoble of %s%s", openb, wch->clan, closeb);
            else
		sprintf(kav, ". %sSoldier of %s%s", openb, wch->clan, closeb);
	}
	else strcpy( kav, "." );

	if (IS_BUI(wch))
	{
	    sprintf(immo + strlen(immo), "%-17s", class);
	    if (IS_NINJA(wch) && IS_AFFECTED(wch, AFF_KAKUSU)) sprintf(immo + strlen(immo), " #r(#0Kak#r)#n ");
	    else if (IS_VAMPAFF(wch, VAM_EARTHMELDED)) sprintf(immo + strlen(immo), "  #L(#o^#L)#n  ");
	    else if (IS_SET(wch->act, PLR_WIZINVIS)) sprintf(immo + strlen(immo), "  #L(#g*#L)#n  ");
	    else if (IS_SET(wch->act, PLR_WIZINCOG)) sprintf(immo + strlen(immo), "  #L(#G*#L)#n  ");
	    else sprintf(immo + strlen(immo), "       ");
	    sprintf(immo + strlen(immo), " #0%s#n%s%s", wch->name, wch->pcdata->title, kav);
	    if (IS_FLAG(wch, FLAG_RP)) sprintf(immo + strlen(immo), " #l[#gRP#l]#n");
	    if (IS_FLAG(wch, FLAG_PIMP)) sprintf(immo + strlen(immo), " #p[#rPimp#p]#n");
	    if (IS_FLAG(wch, FLAG_KINKY)) sprintf(immo + strlen(immo), " #y[#PKinky#y]#n");
	    if (IS_FLAG(wch, FLAG_LAZY)) sprintf(immo + strlen(immo), " #y[#CLazy#y]#n");
	    if (IS_FLAG(wch, FLAG_AFK)) sprintf(immo + strlen(immo), " #0[#7AFK#0]#n");
	    sprintf(immo + strlen(immo), "\n\r");
	    imm++;
	}
	else if (IS_AWA(wch))
	{
	    sprintf(avat + strlen(avat), "%-17s", class);
	    if (IS_NINJA(wch) && IS_AFFECTED(wch, AFF_KAKUSU)) sprintf(avat + strlen(avat), " #r(#0Kak#r)#n ");
            else if (IS_VAMPAFF(wch, VAM_EARTHMELDED)) sprintf(avat + strlen(avat), "  #L(#o^#L)#n  ");
            else sprintf(avat + strlen(avat), "       ");
	    if (IS_SET(wch->act, PLR_FREEZE)) sprintf(avat + strlen(avat), " #C%s#n is Frozen%s", wch->name, kav);
	    else if (IS_SET(wch->act, PLR_SILENCE)) sprintf(avat + strlen(avat), " #C%s#n is Silenced%s", wch->name, kav);
	    else sprintf(avat + strlen(avat), " #C%s#n%s%s", wch->name, wch->pcdata->title, kav);
	    if (IS_AFFECTED(wch, AFF_DEEPUMBRA)) sprintf(avat + strlen(avat), " #L(#CDeep Umbra#L)#n ");
	    else if (IS_AFFECTED(wch, AFF_SHADOWPLANE)) sprintf(avat + strlen(avat), " #L(#CUmbra#L)#n ");
	    if (IS_FLAG(wch, FLAG_PK)) sprintf(avat + strlen(avat), " #r[#0PK#r]#n");
	    if (IS_FLAG(wch, FLAG_RP)) sprintf(avat + strlen(avat), " #l[#gRP#l]#n");
	    if (IS_FLAG(wch, FLAG_TEST)) sprintf(avat + strlen(avat), " #C[#LTester#C]#n");
	    if (IS_FLAG(wch, FLAG_CCC)) sprintf(avat + strlen(avat), " #G*(#7CCC#G)*#n");
	    if (IS_FLAG(wch, FLAG_AFK)) sprintf(avat + strlen(avat), " #0[#7AFK#0]#n");
	    sprintf(avat + strlen(avat), "\n\r");
	    ava++;
	}
	else
	{
	    sprintf(mort + strlen(mort), "%-17s", class);
	    sprintf(mort + strlen(mort), "       ");
	    sprintf(mort + strlen(mort), " %s%s%s", wch->name, wch->pcdata->title, kav);
	    if (IS_FLAG(wch, FLAG_AFK)) sprintf(mort + strlen(mort), " #0[#7AFK#0]#n");
	    sprintf(mort + strlen(mort), "\n\r");
	    mor++;
	}
    }

    stc("\n\r", ch);

    stc("#l-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=#n\n\r", ch);
    stc("  #0Your gaze pierces the mists of the realm and falls upon these brave souls...#n\n\r", ch);
    stc("#l-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=#n\n\r", ch);

    if (imm > 0)
    {
	if (imm > 1) sprintf(buf, "#l-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-#c %li Immortals #l-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=#n\n\r", imm);
	else sprintf(buf, "#l-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-#c One Immortal #l=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=#n\n\r");
	stc(buf, ch);
	stc(immo, ch);
    }

    if (ava > 0)
    {
	if (ava > 1) sprintf(buf, "#l-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-#r %li Awakened #l=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=#n\n\r", ava);
	else sprintf(buf, "#l-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-#r One Awakened #l=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=#n\n\r");
	stc(buf, ch);
	stc(avat, ch);
    }

    if (mor > 0)
    {
	if (mor > 1) sprintf(buf, "#l-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-#g %li Sleepers #l=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=#n\n\r", mor);
	else sprintf(buf, "#l-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-#g One Sleeper #l-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=#n\n\r");
	stc(buf, ch);
	stc(mort, ch);
    }

    stc("#l-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=#n\n\r", ch);
    if ((ava + mor) >= 10) sprintf(buf, "#cThere are a total of %li visible players connected.  Lotsa Players XP Bonus: %li%%.#n\n\r", nTotal, (ava + mor));
    else if (nTotal == 1) sprintf(buf, "#cYou are the only visible player connected!#n\n\r");
    else sprintf(buf, "#cThere are a total of %li visible players connected.#n\n\r", nTotal);
    stc(buf, ch);

    if (exp_info.exp != 100)
    {
	char exp[MSL];
	if (IS_SET(exp_info.bit, EXP_RANDOM))
	    sprintf(buf, "#7Expmod: #sR#sa#sn#sd#so#sm#c%%");
	else
	    sprintf(buf, "#7Expmod: #c%li%%", exp_info.exp);

	if (IS_SET(exp_info.bit, EXP_DEGRADE))
	    sprintf(exp, "%s degrading exp.  Next degrade, %li minutes.\n\r", buf, exp_info.time / 60);
	else if (exp_info.time > 0)
	    sprintf(exp, "%s exp for %li minutes.\n\r", buf, exp_info.time / 60);
	else
	    sprintf(exp, "%s exp forever.\n\r", buf);
	stc(exp, ch);
    }
    stc("#l-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=#n\n\r", ch);
    return;
}

void do_inventory( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *portal;
    OBJ_DATA *portal_next;
    ROOM_INDEX_DATA *pRoomIndex;
    ROOM_INDEX_DATA *location;
    bool found;

    if (!IS_NPC(ch) && IS_MORE(ch,MORE_HEAD))
        {send_to_char( "You are not a container.\n\r", ch ); return;}
    else if (!IS_NPC(ch) && IS_EXTRA(ch,EXTRA_OSWITCH))
    {
        if ( !IS_NPC(ch) && (obj = ch->pcdata->chobj) == NULL)
	    {send_to_char( "You are not a container.\n\r", ch ); return;}
	switch ( obj->item_type )
	{
	default:
	    send_to_char( "You are not a container.\n\r", ch );
	    break;

	case ITEM_PORTAL:
	    pRoomIndex = get_room_index(obj->value[0]);
	    location = ch->in_room;
	    if ( pRoomIndex == NULL )
	    {
		send_to_char( "You don't seem to lead anywhere.\n\r", ch );
		return;
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
		    if (IS_AFFECTED(ch, AFF_SHADOWPLANE) && !IS_SET(portal->extra_flags, ITEM_SHADOWPLANE))
		    {
			REMOVE_BIT(ch->affected_by, AFF_SHADOWPLANE);
	    		do_look(ch,"auto");
			SET_BIT(ch->affected_by, AFF_SHADOWPLANE);
			break;
		    }
		    else if (!IS_AFFECTED(ch, AFF_SHADOWPLANE) && IS_SET(portal->extra_flags, ITEM_SHADOWPLANE))
		    {
			SET_BIT(ch->affected_by, AFF_SHADOWPLANE);
	    		do_look(ch,"auto");
			REMOVE_BIT(ch->affected_by, AFF_SHADOWPLANE);
			break;
		    }
		    else
		    {
	    		do_look(ch,"auto");
			break;
		    }
	    	}
	    }
	    char_from_room(ch);
	    char_to_room(ch,location);
	    break;

	case ITEM_DRINK_CON:
	    if ( obj->value[1] <= 0 )
	    {
		send_to_char( "You are empty.\n\r", ch );
		break;
	    }
	    if (obj->value[1] < obj->value[0] / 5)
	    	sprintf( buf, "There is a little %s liquid left in you.\n\r",liq_table[obj->value[2]].liq_color);
	    else if (obj->value[1] < obj->value[0] / 4)
	    	sprintf( buf, "You contain a small about of %s liquid.\n\r",liq_table[obj->value[2]].liq_color);
	    else if (obj->value[1] < obj->value[0] / 3)
	    	sprintf( buf, "You're about a third full of %s liquid.\n\r",liq_table[obj->value[2]].liq_color);
	    else if (obj->value[1] < obj->value[0] / 2)
	    	sprintf( buf, "You're about half full of %s liquid.\n\r",liq_table[obj->value[2]].liq_color);
	    else if (obj->value[1] < obj->value[0])
	    	sprintf( buf, "You are almost full of %s liquid.\n\r",liq_table[obj->value[2]].liq_color);
	    else if (obj->value[1] == obj->value[0])
	    	sprintf( buf, "You're completely full of %s liquid.\n\r",liq_table[obj->value[2]].liq_color);
	    else
	    	sprintf( buf, "Somehow you are MORE than full of %s liquid.\n\r",liq_table[obj->value[2]].liq_color);
	    send_to_char( buf, ch );
	    break;

	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	    act( "$p contain:", ch, obj, NULL, TO_CHAR );
	    show_list_to_char( obj->contains, ch, TRUE, TRUE );
	    break;
	}
	return;
    }
    send_to_char( "You are carrying:\n\r", ch );
    show_list_to_char( ch->carrying, ch, TRUE, TRUE );
    return;
}

void do_equipment( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    long iWear;
    bool found;

    send_to_char( "You are using:\n\r", ch );
    found = FALSE;
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
	if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
	    continue;

	send_to_char( where_name[iWear], ch );
	if ( can_see_obj( ch, obj ) )
	{
	    send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
	    send_to_char( "\n\r", ch );
	}
	else
	{
	    send_to_char( "something.\n\r", ch );
	}
	found = TRUE;
    }

    if ( !found )
	send_to_char( "Nothing.\n\r", ch );

    return;
}

void do_compare( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj1;
    OBJ_DATA *obj2;
    long value1;
    long value2;
    char *msg;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Compare what to what?\n\r", ch );
	return;
    }

    if ( ( obj1 = get_obj_carry( ch, arg1 ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if ( arg2[0] == '\0' )
    {
	for ( obj2 = ch->carrying; obj2 != NULL; obj2 = obj2->next_content )
	{
	    if ( obj2->wear_loc != WEAR_NONE
	    &&   can_see_obj( ch, obj2 )
	    &&   obj1->item_type == obj2->item_type
	    && ( obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE) != 0 )
		break;
	}

	if ( obj2 == NULL )
	{
	    send_to_char( "You aren't wearing anything comparable.\n\r", ch );
	    return;
	}
    }
    else
    {
	if ( ( obj2 = get_obj_carry( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "You do not have that item.\n\r", ch );
	    return;
	}
    }
	    
    msg		= NULL;
    value1	= 0;
    value2	= 0;

    if ( obj1 == obj2 )
    {
	msg = "You compare $p to itself.  It looks about the same.";
    }
    else if ( obj1->item_type != obj2->item_type )
    {
	msg = "You can't compare $p and $P.";
    }
    else
    {
	switch ( obj1->item_type )
	{
	default:
	    msg = "You can't compare $p and $P.";
	    break;

	case ITEM_ARMOR:
	    value1 = obj1->value[0];
	    value2 = obj2->value[0];
	    break;

	case ITEM_WEAPON:
	    value1 = obj1->value[1] + obj1->value[2];
	    value2 = obj2->value[1] + obj2->value[2];
	    break;
	}
    }

    if ( msg == NULL )
    {
	     if ( value1 == value2 ) msg = "$p and $P look about the same.";
	else if ( value1  > value2 ) msg = "$p looks better than $P.";
	else                         msg = "$p looks worse than $P.";
    }

    act( msg, ch, obj1, obj2, TO_CHAR );
    return;
}

void do_credits( CHAR_DATA *ch, char *argument )
{
    do_help( ch, "diku" );
    return;
}

void do_wizlist( CHAR_DATA *ch, char *argument )
{
    do_help( ch, "wizlist" );
    return;
}

void do_where( CHAR_DATA *ch, char *argument )
{
    char buf[MSL];
    char arg[MIL];
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;
    bool found;

    one_argument(argument, arg);

    if (IS_SET(world_affects, WORLD_FOG) && (IS_OUTSIDE(ch)))
    {
	stc("It is too foggy to locate anyone.\n\r", ch);
	return;
    }

    if (arg[0] == '\0')
    {
	stc("Players near you:\n\r", ch);
	found = FALSE;
	for (d = descriptor_list; d != NULL; d = d->next)
	{
	    if (d->connected != CON_PLAYING || (victim = d->character) == NULL) continue;
	    if (IS_NPC(victim)) continue;
	    if (victim->in_room == NULL || victim->in_room->area != ch->in_room->area) continue;
	    if (!can_see(ch, victim) && (!IS_HIGHLANDER(ch) || IS_IMM(victim))) continue;

	    found = TRUE;
	    if (!can_see(ch, victim)) sprintf(buf, "#r%-28s#n %s\n\r", victim->name, victim->in_room->name);
	    else sprintf(buf, "%-28s %s\n\r", victim->name, victim->in_room->name);
	    stc(buf, ch);
	}
	if (!found) stc("None\n\r", ch);
    }
    else
    {
	found = FALSE;
	for ( victim = char_list; victim != NULL; victim = victim->next )
	{
	    if ( victim->in_room != NULL
	    &&   victim->in_room->area == ch->in_room->area
	    &&   !IS_AFFECTED(victim, AFF_HIDE)
	    &&   !IS_AFFECTED(victim, AFF_SNEAK)
	    &&   can_see( ch, victim )
	    &&   is_name( arg, victim->name ) )
	    {
		found = TRUE;
		sprintf( buf, "%-28s %s\n\r", PERS(victim, ch), victim->in_room->name );
		stc(buf, ch);
		break;
	    }
	}
	if (!found) act("You didn't find any $T.", ch, NULL, arg, TO_CHAR);
    }

    return;
}

void do_consider( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    char *msg;
    long diff;
    long overall;
    long con_hit;
    long con_dam;
    long con_ac;
    long con_hp;

    one_argument( argument, arg );
    overall = 0;

    if ( arg[0] == '\0' )
    {
	send_to_char( "Consider killing whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They're not here.\n\r", ch );
	return;
    }

    act( "You examine $N closely, looking for $S weaknesses.", ch, NULL, victim, TO_CHAR );
    act( "$n examine $N closely, looking for $S weaknesses.", ch, NULL, victim, TO_NOTVICT );
    act( "$n examines you closely, looking for your weaknesses.", ch, NULL, victim, TO_VICT );

    do_skill(ch,victim->name);

    if (!IS_NPC(victim) && IS_EXTRA(victim, EXTRA_FAKE_CON))
    {
	con_hit = victim->pcdata->fake_hit;
	con_dam = victim->pcdata->fake_dam;
	con_ac = victim->pcdata->fake_ac;
	con_hp = victim->pcdata->fake_hp;
    }
    else
    {
	con_hit = char_hitroll(victim);
	con_dam = char_damroll(victim);
	con_ac = char_ac(victim);
	con_hp = victim->hit;
    }
    if (con_hp < 1) con_hp = 1;

    diff = victim->level - ch->level + con_hit - char_hitroll(ch);
         if ( diff <=-100 ) {msg = "You are FAR more skilled than $M."; overall = overall + 3;}
    else if ( diff <= -50 ) {msg = "$E is not as skilled as you are."; overall = overall + 2;}
    else if ( diff <= -25 ) {msg = "$E doesn't seem quite as skilled as you."; overall = overall + 1;}
    else if ( diff <=  25 ) {msg = "You are about as skilled as $M.";}
    else if ( diff <=  50 ) {msg = "$E is slightly more skilled than you are."; overall = overall - 1;}
    else if ( diff <= 100 ) {msg = "$E seems more skilled than you are."; overall = overall -2;}
    else                    {msg = "$E is FAR more skilled than you."; overall = overall - 3;}
    act( msg, ch, NULL, victim, TO_CHAR );

    diff = victim->level - ch->level + con_dam - char_damroll(ch);
         if ( diff <=-100 ) {msg = "You are FAR more powerful than $M."; overall = overall + 3;}
    else if ( diff <= -50 ) {msg = "$E is not as powerful as you are."; overall = overall + 2;}
    else if ( diff <= -25 ) {msg = "$E doesn't seem quite as powerful as you."; overall = overall + 1;}
    else if ( diff <=  25 ) {msg = "You are about as powerful as $M.";}
    else if ( diff <=  50 ) {msg = "$E is slightly more powerful than you are."; overall = overall - 1;}
    else if ( diff <= 100 ) {msg = "$E seems more powerful than you are."; overall = overall -2;}
    else                    {msg = "$E is FAR more powerful than you."; overall = overall - 3;}
    act( msg, ch, NULL, victim, TO_CHAR );

    diff = ch->hit * 100 / con_hp;
         if ( diff <=  10 ) {msg = "$E is currently FAR healthier than you are."; overall = overall - 3;}
    else if ( diff <=  50 ) {msg = "$E is currently much healthier than you are."; overall = overall - 2;}
    else if ( diff <=  75 ) {msg = "$E is currently slightly healthier than you are."; overall = overall - 1;}
    else if ( diff <= 125 ) {msg = "$E is currently about as healthy as you are.";}
    else if ( diff <= 200 ) {msg = "You are currently slightly healthier than $M."; overall = overall + 1;}
    else if ( diff <= 500 ) {msg = "You are currently much healthier than $M."; overall = overall + 2;}
    else                    {msg = "You are currently FAR healthier than $M."; overall = overall + 3;}
    act( msg, ch, NULL, victim, TO_CHAR );

    diff = con_ac - char_ac(ch);
         if ( diff <= 100) {msg = "$E is FAR better armoured than you."; overall = overall - 3;}
    else if ( diff <= 50 ) {msg = "$E looks much better armoured than you."; overall = overall - 2;}
    else if ( diff <= 25 ) {msg = "$E looks better armoured than you."; overall = overall - 1;}
    else if ( diff <= -25 ) {msg = "$E seems about as well armoured as you.";}
    else if ( diff <= -50 ) {msg = "You are better armoured than $M."; overall = overall + 1;}
    else if ( diff <= -100) {msg = "You are much better armoured than $M."; overall = overall + 2;}
    else                    {msg = "You are FAR better armoured than $M."; overall = overall + 3;}
    act( msg, ch, NULL, victim, TO_CHAR );

    diff = overall;
         if ( diff <= -11 ) msg = "Conclusion: $E would kill you in seconds.";
    else if ( diff <=  -7 ) msg = "Conclusion: You would need a lot of luck to beat $M.";
    else if ( diff <=  -3 ) msg = "Conclusion: You would need some luck to beat $N.";
    else if ( diff <=   2 ) msg = "Conclusion: It would be a very close fight.";
    else if ( diff <=   6 ) msg = "Conclusion: You shouldn't have a lot of trouble defeating $M.";
    else if ( diff <=  10 ) msg = "Conclusion: $N is no match for you.  You can easily beat $M.";
    else                    msg = "Conclusion: $E wouldn't last more than a few seconds against you.";
    act( msg, ch, NULL, victim, TO_CHAR );

    return;
}

void set_title( CHAR_DATA *ch, char *title )
{
    char buf[MSL];

    if (IS_NPC(ch))
    {
	bug("Set_title: NPC.", 0);
	return;
    }

    if (isalpha(title[0]) || isdigit(title[0]))
    {
	buf[0] = ' ';
	strcpy(buf+1, title);
    }
    else
    {
	strcpy(buf, title);
    }

    free_string(ch->pcdata->title);
    ch->pcdata->title = str_dup(buf);
    return;
}

void do_title( CHAR_DATA *ch, char *argument )
{
    if (IS_NPC(ch)) return;

    if (argument[0] == '\0')
    {
	stc("Change your title to what?\n\r", ch);
	return;
    }

    if (strlen(argument) > 50) argument[50] = '\0';

    smash_tilde(argument);
    set_title(ch, argument);
    stc("Ok.\n\r", ch);
}

void do_email( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    if ( argument[0] == '\0' )
    {
	if (IS_EXTRA(ch, EXTRA_INVIS_EMAIL))
	{
	    send_to_char("Email invis OFF (ie it now shows via finger).\n\r",ch);
	    REMOVE_BIT(ch->extra, EXTRA_INVIS_EMAIL);
	}
	else
	{
	    send_to_char("Email invis ON (ie it no longer show via finger).\n\r",ch);
	    SET_BIT(ch->extra, EXTRA_INVIS_EMAIL);
	}
	do_autosave(ch,"");
	return;
    }

    if ( strlen(argument) > 50 )
	argument[50] = '\0';

    smash_tilde( argument );
    free_string( ch->pcdata->email );
    ch->pcdata->email = str_dup( argument );
    send_to_char( "Ok.\n\r", ch );
}

void do_description( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( argument[0] != '\0' )
    {
	buf[0] = '\0';
	smash_tilde( argument );
	if ( argument[0] == '+' )
	{
	    if ( ch->description != NULL )
		strcat( buf, ch->description );
	    argument++;
	    while ( isspace(*argument) )
		argument++;
	}

	    if ( strlen2(buf)  >= MAX_STRING_LENGTH - 2 )
        {
            send_to_char( "Description too long.\n\r", ch );
            return;
        }

	if ( strlen2(buf) + strlen2(argument) >= MAX_STRING_LENGTH - 2 )
	{
	    send_to_char( "Description too long.\n\r", ch );
	    return;
	}

	strcat( buf, argument );
	strcat( buf, "\n\r" );
	free_string( ch->description );
	ch->description = str_dup( buf );
    }

    send_to_char( "Your description is:\n\r", ch );
    send_to_char( ch->description ? ch->description : "(None).\n\r", ch );
    return;
}

void do_report( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    char buf[MAX_STRING_LENGTH];
    char hit_str[MAX_INPUT_LENGTH];
    char mana_str[MAX_INPUT_LENGTH];
    char mhit_str[MAX_INPUT_LENGTH];
    char mmana_str[MAX_INPUT_LENGTH];
    char exp_str[MAX_INPUT_LENGTH];
    sprintf(hit_str, "%li", ch->hit);
    COL_SCALE(hit_str, ch, ch->hit, ch->max_hit);
    sprintf(mana_str, "%li", ch->mana);
    COL_SCALE(mana_str, ch, ch->mana, ch->max_mana);
    sprintf(exp_str, "%li", ch->exp);
    COL_SCALE(exp_str, ch, ch->exp, 1000);
    sprintf(mhit_str, "%li", ch->max_hit);
    ADD_COLOUR(ch, mhit_str, L_CYAN);
    sprintf(mmana_str, "%li", ch->max_mana);
    ADD_COLOUR(ch, mmana_str, L_CYAN);
    sprintf( buf,
	"You report: %s/%s hp %s/%s mana %s xp.\n\r",
	hit_str,  mhit_str,
	mana_str, mmana_str,
	exp_str   );

    send_to_char( buf, ch );

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
	vch_next	= vch->next;
	if ( vch == NULL ) continue;
	if ( vch == ch ) continue;
	if ( vch->in_room == NULL ) continue;
	if ( vch->in_room != ch->in_room ) continue;
    	sprintf(hit_str, "%li", ch->hit);
    	COL_SCALE(hit_str, vch, ch->hit, ch->max_hit);
    	sprintf(mana_str, "%li", ch->mana);
    	COL_SCALE(mana_str, vch, ch->mana, ch->max_mana);
    	sprintf(exp_str, "%li", ch->exp);
    	COL_SCALE(exp_str, vch, ch->exp, 1000);
    	sprintf(mhit_str, "%li", ch->max_hit);
    	ADD_COLOUR(vch, mhit_str, L_CYAN);
    	sprintf(mmana_str, "%li", ch->max_mana);
    	ADD_COLOUR(vch, mmana_str, L_CYAN);
    	if (!IS_NPC(ch) && IS_AFFECTED(ch,AFF_POLYMORPH))
    	    sprintf( buf, "%s reports: %s/%s hp %s/%s mana %s xp.\n\r",
	    ch->morph,
	    hit_str,  mhit_str,
	    mana_str, mmana_str,
	    exp_str   );
        else
    	    sprintf( buf, "%s reports: %s/%s hp %s/%s mana %s xp.\n\r",
	    IS_NPC(ch) ? capitalize(ch->short_descr) : ch->name,
	    hit_str,  mhit_str,
	    mana_str, mmana_str,
	    exp_str   );
	buf[0] = UPPER(buf[0]);
    	send_to_char( buf, vch );
    }
    return;
}

void do_socials( CHAR_DATA *ch, char *argument )
{
    char arg[MSL];
    char buf[MIL];
    long iSocial, col = 0;
    bool found = FALSE;

    one_argument(argument, arg);

    if (arg[0] == '\0')
    {
	stc("List of all socials follows.\n\rFor information on a specific socials type 'Socials <social>'.\n\r", ch);
	for (iSocial = 0; social_table[iSocial].name[0] != '\0'; iSocial++)
	{
	    sprintf(buf, "%-12s", social_table[iSocial].name);
	    stc(buf, ch);
	    if (++col % 6 == 0) stc("\n\r", ch);
	}

	if (col % 6 != 0) stc("\n\r", ch);
    }
    else
    {
	for (iSocial = 0; social_table[iSocial].name[0] != '\0'; iSocial++)
	{
	    if (arg[0] == social_table[iSocial].name[0]
	     && !str_prefix(arg, social_table[iSocial].name))
	    {
		found = TRUE;
		break;
	    }
	}

	if (!found)
	{
	    stc("No such social.\n\r", ch);
	    return;
	}

	sprintf(buf, "Social displayed: %s\n\r", social_table[iSocial].name);
	stc(buf, ch);
	if (social_table[iSocial].char_no_arg != NULL)
	{
	    sprintf(buf, "No target, to you:      %s\n\r", social_table[iSocial].char_no_arg);
	    stc(buf, ch);
	}
	if (social_table[iSocial].others_no_arg != NULL)
	{
	    sprintf(buf, "No target, to others:   %s\n\r", social_table[iSocial].others_no_arg);
	    stc(buf, ch);
	}
	if (social_table[iSocial].char_found != NULL)
	{
	    sprintf(buf, "Targeted, to you:       %s\n\r", social_table[iSocial].char_found);
	    stc(buf, ch);
	}
	if (social_table[iSocial].others_found != NULL)
	{
	    sprintf(buf, "Targeted, to others:    %s\n\r", social_table[iSocial].others_found);
	    stc(buf, ch);
	}
	if (social_table[iSocial].vict_found != NULL)
	{
	    sprintf(buf, "Targeted, to victim:    %s\n\r", social_table[iSocial].vict_found);
	    stc(buf, ch);
	}
	if (social_table[iSocial].char_auto != NULL)
	{
	    sprintf(buf, "Self target, to you:    %s\n\r", social_table[iSocial].char_auto);
	    stc(buf, ch);
	}
	if (social_table[iSocial].others_auto != NULL)
	{
	    sprintf(buf, "Self target, to others: %s\n\r", social_table[iSocial].others_auto);
	    stc(buf, ch);
	}
    }
    return;
}



void do_spells( CHAR_DATA *ch, char *argument )
{
    char buf[30];
    long sn, col= 0;

    if (IS_NPC(ch)) return;

    for ( sn = 0; sn < MAX_SKILL && skill_table[sn].name != NULL; sn++ )
    {
	if (skill_table[sn].skill_level > ch->level) continue;
	sprintf(buf, "%-20s", skill_table[sn].name);
	stc(buf, ch);
	if (++col % 4 == 0) stc("\n\r", ch);
    }

    if (col % 4 != 0) stc("\n\r", ch);
    return;
}



/*
 * Contributed by Alander.
 */
void do_commands( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    long cmd;
    long col;
 
    col = 0;
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
        if ( cmd_table[cmd].level == 0
        &&   cmd_table[cmd].level <= get_trust( ch ) )
	{
	    sprintf( buf, "%-12s", cmd_table[cmd].name );
	    send_to_char( buf, ch );
	    if ( ++col % 6 == 0 )
		send_to_char( "\n\r", ch );
	}
    }
 
    if ( col % 6 != 0 )
	send_to_char( "\n\r", ch );
    return;
}



void do_channels( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );

    if (IS_NPC(ch)) return;
    if ( arg[0] == '\0' )
    {
	if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_SILENCE) )
	{
	    send_to_char( "You are silenced.\n\r", ch );
	    return;
	}

	send_to_char( "Channels:", ch );

	send_to_char( !IS_SET(ch->deaf, CHANNEL_CHAT)
	    ? " +CHAT"
	    : " -chat",
	    ch );

	if ( IS_IMM(ch) )
	{
	    send_to_char( !IS_SET(ch->deaf, CHANNEL_IMMTALK)
		? " +IMMTALK"
		: " -immtalk",
		ch );
	}

	if (IS_BUI(ch) || IS_SET(ch->act, PLR_HONOR))
	    stc(!IS_SET(ch->deaf, CHANNEL_HONOR) ? " +HONOR" : " -honor", ch);

	send_to_char( !IS_SET(ch->deaf, CHANNEL_MUSIC)
	    ? " +MUSIC"
	    : " -music",
	    ch );

	send_to_char( !IS_SET(ch->deaf, CHANNEL_SHOUT)
	    ? " +SHOUT"
	    : " -shout",
	    ch );

	send_to_char( !IS_SET(ch->deaf, CHANNEL_YELL)
	    ? " +YELL"
	    : " -yell",
	    ch );

	if (IS_IMM(ch))
	{
	    send_to_char( !IS_SET(ch->deaf, CHANNEL_LOG)
		? " +LOG"
		: " -log",
		ch );
	}

	if (IS_NINJA( ch) || IS_IMM(ch))
        {
            send_to_char( !IS_SET(ch->deaf, CHANNEL_MIKTALK)
                ? " +MIKT"
                : " -mikt",
                ch );
        }
        if (IS_DRAGON(ch) || IS_IMM(ch))
        {
            send_to_char( !IS_SET(ch->deaf, CHANNEL_TELEPATHY)
                ? " +TELEPATHY"
                : " -telepathy",
                ch );
        }

        if (IS_DROW( ch) || IS_IMM(ch))
        {
            send_to_char( !IS_SET(ch->deaf, CHANNEL_DROW)
                ? " +SIGN"
                : " -sign",
                ch ); 
        }

	send_to_char( !IS_SET(ch->deaf, CHANNEL_INFO)
	    ? " +INFO"
	    : " -info",
	    ch );

	send_to_char( !IS_SET(ch->deaf, CHANNEL_TELL)
	    ? " +TELL"
	    : " -tell",
	    ch );

	send_to_char( !IS_SET(ch->deaf, CHANNEL_NEWBIE)
	    ? " +NEWBIE"
	    : " -newbie",
	    ch );

        send_to_char( !IS_SET(ch->deaf, CHANNEL_OOC)
            ? " +OOC"
            : " -ooc",
            ch );
        
	/* Class Channels - Kai */
	if (IS_VAMPIRE(ch) || IS_IMM(ch))
	{
	    send_to_char( !IS_SET(ch->deaf, CHANNEL_VAMPTALK)
	    ? " +VAMP"
	    : " -vamp",
	    ch );
	}
        if (IS_MONK(ch) || IS_IMM(ch))
        {
            send_to_char( !IS_SET(ch->deaf, CHANNEL_MONK)
            ? " +MONK"
            : " -monk",
            ch );
        }
	if (IS_WEREWOLF(ch) || IS_IMM(ch))
	{
	     send_to_char( !IS_SET(ch->deaf, CHANNEL_HOWL)
	    ? " +HOWL"
	    : " -howl",
	    ch );
	}

	if (IS_SORCERER(ch) || IS_IMM(ch))
	{
	    send_to_char( !IS_SET(ch->deaf, CHANNEL_SORCTALK)
	    ? " +SORCTALK"
	    : " -sorctalk",
	    ch );
	}
	if (IS_HIGHLANDER(ch) || IS_IMM(ch))
	{
	    send_to_char( !IS_SET(ch->deaf, CHANNEL_HIGHTALK)
	    ? " +HIGH"
	    : " -high",
	    ch );
	}

	if (IS_MAGE(ch) || IS_IMM(ch))
        {
            send_to_char( !IS_SET(ch->deaf, CHANNEL_MAGETALK)
            ? " +MAGE"
            : " -mage",
            ch );
        }

	if (IS_ELEM(ch) || IS_IMM(ch))
	{
	    send_to_char( !IS_SET(ch->deaf, CHANNEL_ELEM)
		? " +SPIRITS"
		: " -spirits",
	        ch );
	}	 
	send_to_char( ".\n\r", ch );
    }
    else
    {
	bool fClear;
	long bit;

	     if ( arg[0] == '+' ) fClear = TRUE;
	else if ( arg[0] == '-' ) fClear = FALSE;
	else
	{
	    send_to_char( "Channels -channel or +channel?\n\r", ch );
	    return;
	}

	     if ( !str_cmp( arg+1, "sorcerer"     ) ) bit = CHANNEL_SORCTALK;
        else if ( !str_cmp( arg+1, "chat"     ) ) bit = CHANNEL_CHAT;
	else if ( !str_cmp( arg+1, "immtalk"  ) ) bit = CHANNEL_IMMTALK;
	else if ( !str_cmp( arg+1, "honor"  ) ) bit = CHANNEL_HONOR;
	else if ( !str_cmp( arg+1, "music"    ) ) bit = CHANNEL_MUSIC;
	else if ( !str_cmp( arg+1, "newbie"   ) ) bit = CHANNEL_NEWBIE;
	else if ( !str_cmp( arg+1, "shout"    ) ) bit = CHANNEL_SHOUT;
	else if ( !str_cmp( arg+1, "yell"     ) ) bit = CHANNEL_YELL;
	else if ( !str_cmp( arg+1, "howl"     ) ) bit = CHANNEL_HOWL;
	else if (IS_IMM(ch) && !str_cmp( arg+1, "log") ) bit = CHANNEL_LOG;
	else if ( !str_cmp( arg+1, "info"     ) ) bit = CHANNEL_INFO;
	else if ( !str_cmp( arg+1, "vamp"     ) ) bit = CHANNEL_VAMPTALK;
	else if ( !str_cmp( arg+1, "tell"     ) ) bit = CHANNEL_TELL;
        else if ( !str_cmp( arg+1, "mikt"     ) ) bit = CHANNEL_MIKTALK;
	else if ( !str_cmp( arg+1, "spirits"  ) ) bit = CHANNEL_ELEM;
        else if ( !str_cmp( arg+1, "monk"     ) ) bit = CHANNEL_MONK;
        else if ( !str_cmp( arg+1, "high"     ) ) bit = CHANNEL_HIGHTALK;
  	else if ( !str_cmp( arg+1, "mage"     ) ) bit = CHANNEL_MAGETALK;
        else if ( !str_cmp( arg+1, "sign"     ) ) bit = CHANNEL_DROW;
        else if ( !str_cmp( arg+1, "telepathy") ) bit = CHANNEL_TELEPATHY;
        else if ( !str_cmp( arg+1, "ooc"      ) ) bit = CHANNEL_OOC;
	else if ( !str_cmp( arg+1, "all"      ))
	{
	if (fClear )
	REMOVE_BIT (ch->deaf, CHANNEL_SORCTALK);
	else
	SET_BIT(ch->deaf, CHANNEL_SORCTALK);
	if (fClear )
	REMOVE_BIT (ch->deaf, CHANNEL_CHAT);
	else
	SET_BIT(ch->deaf, CHANNEL_CHAT);
	if (fClear )
	REMOVE_BIT (ch->deaf, CHANNEL_IMMTALK);
	else
	SET_BIT(ch->deaf, CHANNEL_IMMTALK);
	if (fClear )
	REMOVE_BIT (ch->deaf, CHANNEL_MUSIC);
	else
	SET_BIT(ch->deaf, CHANNEL_MUSIC);
	if (fClear )
	REMOVE_BIT (ch->deaf, CHANNEL_NEWBIE);
	else
	SET_BIT(ch->deaf, CHANNEL_NEWBIE);
	if (fClear )
	REMOVE_BIT (ch->deaf, CHANNEL_SHOUT);
	else
	SET_BIT(ch->deaf, CHANNEL_SHOUT);
	if (fClear )
	REMOVE_BIT (ch->deaf, CHANNEL_YELL);
	else
	SET_BIT(ch->deaf, CHANNEL_YELL);
	if (fClear )
	REMOVE_BIT (ch->deaf, CHANNEL_HOWL);
	else
	SET_BIT(ch->deaf, CHANNEL_HOWL);
	if (fClear )
	REMOVE_BIT (ch->deaf, CHANNEL_INFO);
	else
	SET_BIT(ch->deaf, CHANNEL_INFO);
	if (fClear )
	REMOVE_BIT (ch->deaf, CHANNEL_VAMPTALK);
	else
	SET_BIT(ch->deaf, CHANNEL_VAMPTALK);
	if (fClear )
	REMOVE_BIT (ch->deaf, CHANNEL_TELL);
	else
	SET_BIT(ch->deaf, CHANNEL_TELL);
	if (fClear )
	REMOVE_BIT (ch->deaf, CHANNEL_MIKTALK);
	else
	SET_BIT(ch->deaf, CHANNEL_MIKTALK);
	if (fClear )
	REMOVE_BIT (ch->deaf, CHANNEL_ELEM);
	else
	SET_BIT(ch->deaf, CHANNEL_ELEM);
	if (fClear )
	REMOVE_BIT (ch->deaf, CHANNEL_MONK);
	else
	SET_BIT(ch->deaf, CHANNEL_MONK);
	if (fClear )
	REMOVE_BIT (ch->deaf, CHANNEL_HIGHTALK);
	else
	SET_BIT(ch->deaf, CHANNEL_HIGHTALK);
	if (fClear )
        REMOVE_BIT (ch->deaf, CHANNEL_MAGETALK);
        else
        SET_BIT(ch->deaf, CHANNEL_MAGETALK);
	if (fClear )
	REMOVE_BIT (ch->deaf, CHANNEL_DROW);
	else
	SET_BIT(ch->deaf, CHANNEL_DROW);
	if (fClear )
	REMOVE_BIT (ch->deaf, CHANNEL_TELEPATHY);
	else
	SET_BIT(ch->deaf, CHANNEL_TELEPATHY);
	if (fClear )
	REMOVE_BIT (ch->deaf, CHANNEL_OOC);
	else
	SET_BIT(ch->deaf, CHANNEL_OOC);
send_to_char( "Ok.\n\r", ch );
return;
	}		
	else
	{
	    send_to_char( "Set or clear which channel?\n\r", ch );
	    return;
	}

	if ( fClear )
	    REMOVE_BIT (ch->deaf, bit);
	else
	    SET_BIT    (ch->deaf, bit);

	send_to_char( "Ok.\n\r", ch );
    }

    return;
}



/*
 * Contributed by Grodyn.
 */
void do_config( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    if ( IS_NPC(ch) )
	return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "[ Keyword  ] Option\n\r", ch );

	send_to_char(  IS_SET(ch->act, PLR_ANSI)
            ? "[+ANSI      ] You have ansi colour on.\n\r"
	    : "[-ansi      ] You have ansi colour off.\n\r"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_AUTOEXIT)
            ? "[+AUTOEXIT  ] You automatically see exits.\n\r"
	    : "[-autoexit  ] You don't automatically see exits.\n\r"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_AUTOSEARCH)
            ? "[+AUTOSEARCH] You automatically look in corpses after a fight.\n\r"
	    : "[-autosearch] You don't automatically look in corpses after a fight.\n\r"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_AUTOFOLLOW)
            ? "[+AUTOFOLLOW] You automatically follow opponents who flee from you.\n\r"
            : "[-autofollow] You don't automatically follow opponents who flee from you.\n\r"
            , ch );

	send_to_char(  IS_SET(ch->act, PLR_BLANK)
	    ? "[+BLANK     ] You have a blank line before your prompt.\n\r"
	    : "[-blank     ] You have no blank line before your prompt.\n\r"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_BRIEF)
	    ? "[+BRIEF     ] You see brief descriptions.\n\r"
	    : "[-brief     ] You see long descriptions.\n\r"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_COMBINE)
	    ? "[+COMBINE   ] You see object lists in combined format.\n\r"
	    : "[-combine   ] You see object lists in single format.\n\r"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_PROMPT)
	    ? "[+PROMPT    ] You have a prompt.\n\r"
	    : "[-prompt    ] You don't have a prompt.\n\r"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_TELNET_GA)
	    ? "[+TELNETGA  ] You receive a telnet GA sequence.\n\r"
	    : "[-telnetga  ] You don't receive a telnet GA sequence.\n\r"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_SILENCE)
	    ? "[+SILENCE   ] You are silenced.\n\r"
	    : ""
	    , ch );

	send_to_char( !IS_SET(ch->act, PLR_NO_TELL)
	    ? ""
	    : "[-tell      ] You can't use 'tell'.\n\r"
	    , ch );
	send_to_char( IS_SET(ch->act, PLR_SDAM)
	    ? "[+SHOWDAM   ] You see the damage everyone does.\n\r"
	    : "[-showdam   ] You do not see the damage everyone does.\n\r"
	    , ch);
    }
    else
    {
	bool fSet;
	long bit;

	     if ( arg[0] == '+' ) fSet = TRUE;
	else if ( arg[0] == '-' ) fSet = FALSE;
	else
	{
	    send_to_char( "Config -option or +option?\n\r", ch );
	    return;
	}

             if ( !str_cmp( arg+1, "ansi"     ) ) bit = PLR_ANSI;
        else if ( !str_cmp( arg+1, "autoexit" ) ) bit = PLR_AUTOEXIT;
	else if ( !str_cmp( arg+1, "autosearch")) bit = PLR_AUTOSEARCH;
	else if ( !str_cmp( arg+1, "autofollow")) bit = PLR_AUTOFOLLOW;
        else if ( !str_cmp( arg+1, "blank"    ) ) bit = PLR_BLANK;
	else if ( !str_cmp( arg+1, "brief"    ) ) bit = PLR_BRIEF;
	else if ( !str_cmp( arg+1, "combine"  ) ) bit = PLR_COMBINE;
        else if ( !str_cmp( arg+1, "prompt"   ) ) bit = PLR_PROMPT;
	else if ( !str_cmp( arg+1, "telnetga" ) ) bit = PLR_TELNET_GA;
	else if ( !str_cmp( arg+1, "showdam"  ) ) bit = PLR_SDAM;
	else
	{
	    send_to_char( "Config which option?\n\r", ch );
	    return;
	}

	if ( fSet )
	    SET_BIT    (ch->act, bit);
	else
	    REMOVE_BIT (ch->act, bit);

	send_to_char( "Ok.\n\r", ch );
    }

    return;
}

void do_ansi( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) ) return;
    if (IS_SET(ch->act, PLR_ANSI)) do_config(ch,"-ansi");
    else do_config(ch,"+ansi");
    return;
}

void do_autoexit( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) ) return;
    if (IS_SET(ch->act, PLR_AUTOEXIT)) do_config(ch,"-autoexit");
    else do_config(ch,"+autoexit");
    return;
}

void do_autosearch( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) ) return;
    if (IS_SET(ch->act, PLR_AUTOSEARCH)) do_config(ch,"-autosearch");
    else do_config(ch,"+autosearch");
    return;
}

void do_autofollow( CHAR_DATA *ch, char *argument)
{
    if ( IS_NPC(ch) ) return;
    if (IS_SET(ch->act, PLR_AUTOFOLLOW)) do_config(ch,"-autofollow");
    else do_config(ch,"+autofollow");
    return;
}

void do_blank( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) ) return;
    if (IS_SET(ch->act, PLR_BLANK)) do_config(ch,"-blank");
    else do_config(ch,"+blank");
    return;
}

void do_brief( CHAR_DATA *ch, char *argument )
{
    if (IS_NPC(ch)) return;
    if (IS_SET(ch->act, PLR_BRIEF)) do_config(ch,"-brief");
    else do_config(ch,"+brief");
    return;
}

void do_showdamage( CHAR_DATA *ch, char *argument )
{
    if (IS_NPC(ch)) return;
    if (IS_SET(ch->act, PLR_SDAM)) do_config(ch,"-showdam");
    else do_config(ch,"+showdam");
    return;
}

void obj_score( CHAR_DATA *ch, OBJ_DATA *obj )
{
    char buf[MAX_STRING_LENGTH];
    AFFECT_DATA *paf;
    long itemtype;

    sprintf( buf,"You are %s.\n\r",obj->short_descr);
    send_to_char( buf, ch );

    sprintf( buf,"Type %s, Extra flags %s.\n\r",item_type_name(obj->item_type),
	extra_bit_name(obj->extra_flags));
    send_to_char( buf, ch );

    sprintf( buf,"You weigh %li pounds and are worth %li gold coins.\n\r",obj->weight,obj->cost);
    send_to_char( buf, ch );

    if (obj->questmaker != NULL && strlen(obj->questmaker) > 1 &&
	obj->questowner != NULL && strlen(obj->questowner) > 1)
    {
	sprintf( buf, "You were created by %s, and are owned by %s.\n\r", obj->questmaker,obj->questowner );
	send_to_char( buf, ch );
    }
    else if (obj->questmaker != NULL && strlen(obj->questmaker) > 1)
    {
	sprintf( buf, "You were created by %s.\n\r", obj->questmaker );
	send_to_char( buf, ch );
    }
    else if (obj->questowner != NULL && strlen(obj->questowner) > 1)
    {
	sprintf( buf, "You are owned by %s.\n\r", obj->questowner );
	send_to_char( buf, ch );
    }

    switch ( obj->item_type )
    {
    case ITEM_SCROLL: 
    case ITEM_POTION:
	sprintf( buf, "You contain level %li spells of:", obj->value[0] );
	send_to_char( buf, ch );

	if ( obj->value[1] >= 0 && obj->value[1] < MAX_SKILL )
	{
	    send_to_char( " '", ch );
	    send_to_char( skill_table[obj->value[1]].name, ch );
	    send_to_char( "'", ch );
	}

	if ( obj->value[2] >= 0 && obj->value[2] < MAX_SKILL )
	{
	    send_to_char( " '", ch );
	    send_to_char( skill_table[obj->value[2]].name, ch );
	    send_to_char( "'", ch );
	}

	if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
	{
	    send_to_char( " '", ch );
	    send_to_char( skill_table[obj->value[3]].name, ch );
	    send_to_char( "'", ch );
	}

	send_to_char( ".\n\r", ch );
	break;

    case ITEM_QUEST:
	sprintf( buf, "Your quest point value is %li.\n\r", obj->value[0] );
	send_to_char( buf, ch );
	break;

    case ITEM_WAND: 
    case ITEM_STAFF: 
	sprintf( buf, "You have %li(%li) charges of level %li",
	    obj->value[1], obj->value[2], obj->value[0] );
	send_to_char( buf, ch );

	if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
	{
	    send_to_char( " '", ch );
	    send_to_char( skill_table[obj->value[3]].name, ch );
	    send_to_char( "'", ch );
	}

	send_to_char( ".\n\r", ch );
	break;
      
    case ITEM_WEAPON:
	sprintf( buf, "You inflict %li to %li damage in combat (average %li).\n\r",
	    obj->value[1], obj->value[2],
	    ( obj->value[1] + obj->value[2] ) / 2 );
	send_to_char( buf, ch );

	if (obj->value[0] >= 1000)
	    itemtype = obj->value[0] - ((obj->value[0] / 1000) * 1000);
	else
	    itemtype = obj->value[0];

	if (itemtype > 0)
	{
	    if (obj->level < 10)
		sprintf(buf,"You are a minor spell weapon.\n\r");
	    else if (obj->level < 20)
		sprintf(buf,"You are a lesser spell weapon.\n\r");
	    else if (obj->level < 30)
		sprintf(buf,"You are an average spell weapon.\n\r");
	    else if (obj->level < 40)
		sprintf(buf,"You are a greater spell weapon.\n\r");
	    else if (obj->level < 50)
		sprintf(buf,"You are a major spell weapon.\n\r");
	    else
		sprintf(buf,"You are a supreme spell weapon.\n\r");
	    send_to_char(buf,ch);
	}

	if (itemtype == 1)
	    sprintf (buf, "You are dripping with corrosive acid.\n\r");
	else if (itemtype == 4)
	    sprintf (buf, "You radiate an aura of darkness.\n\r");
	else if (itemtype == 30)
	    sprintf (buf, "You are the bane of all evil.\n\r");
	else if (itemtype == 34)
	    sprintf (buf, "You drink the souls of your victims.\n\r");
	else if (itemtype == 37)
	    sprintf (buf, "You have been tempered in hellfire.\n\r");
	else if (itemtype == 48)
	    sprintf (buf, "You crackle with sparks of lightning.\n\r");
	else if (itemtype == 53)
	    sprintf (buf, "You are dripping with a dark poison.\n\r");
	else if (itemtype > 0)
	    sprintf (buf, "You have been imbued with the power of %s.\n\r",skill_table[itemtype].name);
	if (itemtype > 0)
	    send_to_char( buf, ch );

	if (obj->value[0] >= 1000)
	    itemtype = obj->value[0] / 1000;
	else
	    break;

	if (itemtype == 4 || itemtype == 1)
	    sprintf (buf, "You radiate an aura of darkness.\n\r");
	else if (itemtype == 27 || itemtype == 2)
	    sprintf (buf, "You allow your wielder to see invisible things.\n\r");
	else if (itemtype == 39 || itemtype == 3)
	    sprintf (buf, "You grant your wielder the power of flight.\n\r");
	else if (itemtype == 45 || itemtype == 4)
	    sprintf (buf, "You allow your wielder to see in the dark.\n\r");
	else if (itemtype == 46 || itemtype == 5)
	    sprintf (buf, "You render your wielder invisible to the human eye.\n\r");
	else if (itemtype == 52 || itemtype == 6)
	    sprintf (buf, "You allow your wielder to walk through solid doors.\n\r");
	else if (itemtype == 54 || itemtype == 7)
	    sprintf (buf, "You protect your wielder from evil.\n\r");
	else if (itemtype == 57 || itemtype == 8)
	    sprintf (buf, "You protect your wielder in combat.\n\r");
	else if (itemtype == 9)
	    sprintf (buf, "You allow your wielder to walk in complete silence.\n\r");
	else if (itemtype == 10)
	    sprintf (buf, "You surround your wielder with a shield of lightning.\n\r");
	else if (itemtype == 11)
	    sprintf (buf, "You surround your wielder with a shield of fire.\n\r");
	else if (itemtype == 12)
	    sprintf (buf, "You surround your wielder with a shield of ice.\n\r");
	else if (itemtype == 13)
	    sprintf (buf, "You surround your wielder with a shield of acid.\n\r");
	else if (itemtype == 14)
	    sprintf (buf, "You protect your wielder from attacks from DarkBlade clan guardians.\n\r");
	else if (itemtype == 15)
	    sprintf (buf, "You surround your wielder with a shield of chaos.\n\r");
	else if (itemtype == 16)
	    sprintf (buf, "You regenerate the wounds of your wielder.\n\r");
	else if (itemtype == 17)
	    sprintf (buf, "You enable your wielder to move at supernatural speed.\n\r");
	else if (itemtype == 18)
	    sprintf (buf, "You can slice through armour without difficulty.\n\r");
	else if (itemtype == 19)
	    sprintf (buf, "You protect your wielder from player attacks.\n\r");
	else if (itemtype == 20)
	    sprintf (buf, "You surround your wielder with a shield of darkness.\n\r");
	else if (itemtype == 21)
	    sprintf (buf, "You grant your wielder superior protection.\n\r");
	else if (itemtype == 22)
	    sprintf (buf, "You grant your wielder supernatural vision.\n\r");
	else if (itemtype == 23)
	    sprintf (buf, "You make your wielder fleet-footed.\n\r");
	else if (itemtype == 24)
	    sprintf (buf, "You conceal your wielder from sight.\n\r");
	else if (itemtype == 25)
	    sprintf (buf, "You invoke the power of your wielders beast.\n\r");
	else
	    sprintf (buf, "You are bugged...please report it.\n\r");
	if (itemtype > 0)
	    send_to_char( buf, ch );
	break;

    case ITEM_ARMOR:
	sprintf( buf, "Your armor class is %li.\n\r", obj->value[0] );
	send_to_char( buf, ch );
	if (obj->value[3] < 1)
	    break;
	if (obj->value[3] == 4 || obj->value[3] == 1)
	    sprintf (buf, "You radiate an aura of darkness.\n\r");
	else if (obj->value[3] == 27 || obj->value[3] == 2)
	    sprintf (buf, "You allow your wearer to see invisible things.\n\r");
	else if (obj->value[3] == 39 || obj->value[3] == 3)
	    sprintf (buf, "You grant your wearer the power of flight.\n\r");
	else if (obj->value[3] == 45 || obj->value[3] == 4)
	    sprintf (buf, "You allow your wearer to see in the dark.\n\r");
	else if (obj->value[3] == 46 || obj->value[3] == 5)
	    sprintf (buf, "You render your wearer invisible to the human eye.\n\r");
	else if (obj->value[3] == 52 || obj->value[3] == 6)
	    sprintf (buf, "You allow your wearer to walk through solid doors.\n\r");
	else if (obj->value[3] == 54 || obj->value[3] == 7)
	    sprintf (buf, "You protect your wearer from evil.\n\r");
	else if (obj->value[3] == 57 || obj->value[3] == 8)
	    sprintf (buf, "You protect your wearer in combat.\n\r");
	else if (obj->value[3] == 9)
	    sprintf (buf, "You allow your wearer to walk in complete silence.\n\r");
	else if (obj->value[3] == 10)
	    sprintf (buf, "You surround your wearer with a shield of lightning.\n\r");
	else if (obj->value[3] == 11)
	    sprintf (buf, "You surround your wearer with a shield of fire.\n\r");
	else if (obj->value[3] == 12)
	    sprintf (buf, "You surround your wearer with a shield of ice.\n\r");
	else if (obj->value[3] == 13)
	    sprintf (buf, "You surround your wearer with a shield of acid.\n\r");
	else if (obj->value[3] == 14)
	    sprintf (buf, "You protect your wearer from attacks from DarkBlade clan guardians.\n\r");
	else if (obj->value[3] == 15)
	    sprintf (buf, "You surround your wielder with a shield of chaos.\n\r");
	else if (obj->value[3] == 16)
	    sprintf (buf, "You regenerate the wounds of your wielder.\n\r");
	else if (obj->value[3] == 17)
	    sprintf (buf, "You enable your wearer to move at supernatural speed.\n\r");
	else if (obj->value[3] == 18)
	    sprintf (buf, "You can slice through armour without difficulty.\n\r");
	else if (obj->value[3] == 19)
	    sprintf (buf, "You protect your wearer from player attacks.\n\r");
	else if (obj->value[3] == 20)
	    sprintf (buf, "You surround your wearer with a shield of darkness.\n\r");
	else if (obj->value[3] == 21)
	    sprintf (buf, "You grant your wearer superior protection.\n\r");
	else if (obj->value[3] == 22)
	    sprintf (buf, "You grant your wearer supernatural vision.\n\r");
	else if (obj->value[3] == 23)
	    sprintf (buf, "You make your wearer fleet-footed.\n\r");
	else if (obj->value[3] == 24)
	    sprintf (buf, "You conceal your wearer from sight.\n\r");
	else if (obj->value[3] == 25)
	    sprintf (buf, "You invoke the power of your wearers beast.\n\r");
	else
	    sprintf (buf, "You are bugged...please report it.\n\r");
	if (obj->value[3] > 0)
	    send_to_char( buf, ch );
	break;
    }

    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->location != APPLY_NONE && paf->modifier != 0 )
	{
	    sprintf( buf, "You %s %li.\n\r",
		affect_loc_name( paf->location ), paf->modifier );
	    send_to_char( buf, ch );
	}
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->location != APPLY_NONE && paf->modifier != 0 )
	{
	    sprintf( buf, "You %s %li.\n\r",
		affect_loc_name( paf->location ), paf->modifier );
	    send_to_char( buf, ch );
	}
    }
    return;
}

/* Do_prompt from Morgenes from Aldara Mud */
void do_prompt( CHAR_DATA *ch, char *argument )
{
   char buf [ MAX_STRING_LENGTH ];

   buf[0] = '\0';
   if (IS_NPC(ch)) return;
/*
   ch = ( ch->desc->original ? ch->desc->original : ch->desc->character );
*/
   if ( argument[0] == '\0' )
   {
       do_help( ch, "prompt" );
       return;
   }

   if( !strcmp( argument, "on" ) )
   {
      if (IS_EXTRA(ch, EXTRA_PROMPT))
         send_to_char("But you already have customised prompt on!\n\r",ch);
      else
      {
         send_to_char("Ok.\n\r",ch);
         SET_BIT(ch->extra, EXTRA_PROMPT);
      }
      return;
   }
   else if( !strcmp( argument, "off" ) )
   {
      if (!IS_EXTRA(ch, EXTRA_PROMPT))
         send_to_char("But you already have customised prompt off!\n\r",ch);
      else
      {
         send_to_char("Ok.\n\r",ch);
         REMOVE_BIT(ch->extra, EXTRA_PROMPT);
      }
      return;
   }
   else if( !strcmp( argument, "clear" ) )
   {
      free_string(ch->prompt);
      ch->prompt = str_dup( "" );
      return;
   }
   else
   {
      if ( strlen( argument ) > 50 )
	  argument[50] = '\0';
      smash_tilde( argument );
      strcat( buf, argument );
   }

   free_string( ch->prompt );
   ch->prompt = str_dup( buf );
   send_to_char( "Ok.\n\r", ch );
   return;
} 

/* Do_prompt from Morgenes from Aldara Mud */
void do_cprompt( CHAR_DATA *ch, char *argument )
{
   char buf [ MAX_STRING_LENGTH ];

   buf[0] = '\0';
   if (IS_NPC(ch)) return;
/*
   ch = ( ch->desc->original ? ch->desc->original : ch->desc->character );
*/
   if ( argument[0] == '\0' )
   {
       do_help( ch, "cprompt" );
       return;
   }

   if( !strcmp( argument, "clear" ) )
   {
      free_string(ch->cprompt);
      ch->cprompt = str_dup( "" );
      return;
   }
   else
   {
      if ( strlen( argument ) > 50 )
	  argument[50] = '\0';
      smash_tilde( argument );
      strcat( buf, argument );
   }

   free_string( ch->cprompt );
   ch->cprompt = str_dup( buf );
   send_to_char( "Ok.\n\r", ch );
   return;
} 

void do_peek( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( IS_NPC(ch) ) return;

    if ( arg[0] == '\0' )
    {
	send_to_char( "Peek at whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They are not here.\n\r", ch );
	return;
    }

    send_to_char( "You peek at their inventory:\n\r", ch );
    show_list_to_char( victim->carrying, ch, TRUE, TRUE );
    WAIT_STATE(ch, 12);
    return;
}

void do_affects( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA * obj;
    AFFECT_DATA *paf;
    int iWear, itemtype;
    bool first;

    if ( ch->affected != NULL )
    {
	send_to_char( "You are affected by:\n\r", ch );
	for ( paf = ch->affected; paf != NULL; paf = paf->next )
	{
	    sprintf( buf, "Affect: '%s'", skill_table[paf->type].name );
	    send_to_char( buf, ch );

	    sprintf( buf, " %s %li for %li hours with bits %s.\n\r", affect_loc_name( paf->location ), paf->modifier, paf->duration, affect_bit_name( paf->bitvector ) );
	    send_to_char( buf, ch );
	}
    }
    else send_to_char( "You don't have any special affects on you.\n\r", ch );

    send_to_char( "Your equipment is affecting you by:\n\r", ch );

    first = TRUE;

    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
        if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
            continue;
	if (obj->item_type == ITEM_ARMOR)
	{

	    itemtype = obj->value[3];

	    if (!first) send_to_char(", ", ch);

	    if (obj->value[3] < 1)
            	break;
            if (obj->value[3] == 4)
                sprintf (buf, "Blind ");
            else if (obj->value[3] == 27 || obj->value[3] == 2)
                sprintf (buf, "Detect Invis");
            else if (obj->value[3] == 39 || obj->value[3] == 3)
                sprintf (buf, "Fly");
            else if (obj->value[3] == 45 || obj->value[3] == 1)
                sprintf (buf, "Infravision ");
            else if (obj->value[3] == 46 || obj->value[3] == 5)
                sprintf (buf, "Invis");
            else if (obj->value[3] == 52 || obj->value[3] == 6)
                sprintf (buf, "Pass Door");
            else if (obj->value[3] == 54 || obj->value[3] == 7)
                sprintf (buf, "Protection");
            else if (obj->value[3] == 57 || obj->value[3] == 8)
                sprintf (buf, "Sanct");
            else if (obj->value[3] == 9)
                sprintf (buf, "Sneak");
            else if (obj->value[3] == 10)
                sprintf (buf, "Lightning Shield");
            else if (obj->value[3] == 11)
                sprintf (buf, "Fire Shield");
            else if (obj->value[3] == 12)
                sprintf (buf, "Ice Shield");
            else if (obj->value[3] == 13)
                sprintf (buf, "Acid Shield");
            else if (obj->value[3] == 14)
                sprintf (buf, "Haste");
            else if (obj->value[3] == 15)
                sprintf (buf, "Vorpal");
            else if (obj->value[3] == 16)
                sprintf (buf, "Regen");
            else if (obj->value[3] == 17)
                sprintf (buf, "Haste");
	    else if (obj->value[3] == 18)
                sprintf (buf, "Vision");
            else if (obj->value[3] == 19)
                sprintf (buf, "Soak1");
            else if (obj->value[3] == 20)
                sprintf (buf, "Magic Resistance");
            else if (obj->value[3] == 21)
                sprintf (buf, "Soak2");
            else if (obj->value[3] == 24)
                sprintf (buf, "Shadows");
            else if (obj->value[3] == 58)
                sprintf (buf, "Vorpal");
	    send_to_char(buf, ch);

	    first = FALSE;
	}
    }
    send_to_char("\n\r", ch);
    return;
}

