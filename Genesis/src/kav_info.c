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



void do_scan( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *location;

    location = ch->in_room;

    send_to_char("#r[#yNorth#r]#n\n\r",ch);
    do_spydirection(ch,"n");

    send_to_char("#r[#yEast#r]#n\n\r",ch);
    do_spydirection(ch,"e");

    send_to_char("#r[#ySouth#r]#n\n\r",ch);
    do_spydirection(ch,"s");

    send_to_char("#r[#yWest#r]#n\n\r",ch);
    do_spydirection(ch,"w");

    send_to_char("#r[#yUp#r]#n\n\r",ch);
    do_spydirection(ch,"u");

    send_to_char("#r[#yDown#r]#n\n\r",ch);
    do_spydirection(ch,"d");
    return;
}

void do_spydirection( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA       *pexit;
    CHAR_DATA       *vch;
    CHAR_DATA       *vch_next;
    char            arg      [MAX_INPUT_LENGTH];
    char            buf      [MAX_INPUT_LENGTH];
    long             door;
    bool            is_empty;

    argument = one_argument( argument, arg );

    is_empty = TRUE;

    if ( arg[0] == '\0' )
	return;

    if      ( !str_cmp( arg, "n" ) || !str_cmp( arg, "north" ) ) door = 0;
    else if ( !str_cmp( arg, "e" ) || !str_cmp( arg, "east"  ) ) door = 1;
    else if ( !str_cmp( arg, "s" ) || !str_cmp( arg, "south" ) ) door = 2;
    else if ( !str_cmp( arg, "w" ) || !str_cmp( arg, "west"  ) ) door = 3;
    else if ( !str_cmp( arg, "u" ) || !str_cmp( arg, "up"    ) ) door = 4;
    else if ( !str_cmp( arg, "d" ) || !str_cmp( arg, "down"  ) ) door = 5;
    else return;

    if (( pexit = ch->in_room->exit[door]) == NULL || (to_room = pexit->to_room) == NULL)
    {
	send_to_char("   No exit.\n\r",ch);
	return;
    }
    pexit = ch->in_room->exit[door];
    if (IS_SET(pexit->exit_info, EX_CLOSED) )
    {
	send_to_char("   Closed door.\n\r",ch);
	return;
    }

    if (IS_SET(world_affects, WORLD_FOG) && (IS_OUTSIDE(ch)))
    {
	send_to_char("   Too foggy to tell.\n\r",ch);
	return;
    }
    if (room_is_dark(to_room) && !IS_VAMPAFF(ch, VAM_NIGHTSIGHT))
    {
	send_to_char("   Too dark to tell.\n\r",ch);
	return;
    }
    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
	vch_next	= vch->next;
	if ( vch->in_room == NULL )
	    continue;
	if ( vch->in_room != to_room )
	    continue;
	if ( !can_see(ch,vch) )
	    continue;
	if (!IS_NPC(vch)) strcpy(buf,"   $N (Player)");
	else strcpy(buf,"   $N");
	act(buf,ch,NULL,vch,TO_CHAR);
	is_empty = FALSE;
    }
    if (is_empty)
    {
	send_to_char("   Nobody here.\n\r",ch);
	return;
    }
    return;
}

void do_huh( CHAR_DATA *ch, char *argument )
{
    send_to_char("Huh?\n\r",ch);
    return;
}

void do_totalscore( CHAR_DATA *ch, char *argument )
{
    char buf[MSL];
    char arg [MIL];
    long p_score = (ch->played + (int) (current_time - ch->logon) );
    long k_score = 0;
    long t_score = 0;
    long lowest = score_1;

    if (IS_NPC(ch)) return;

    argument = one_argument(argument, arg);

    if (arg[0] != '\0')
    {
	if (str_cmp(arg, "all"))
	{
	    stc("Try 'totalscore all' to list top 3 scores.\n\r", ch);
	    return;
	}
	stc("Highest scores as follows:\n\r", ch);
	if (score_1 >= score_2 && score_1 >= score_3)
	{
	    sprintf(buf,"First place is %s with %li points.\n\r", first_place, score_1);
	    stc(buf, ch);
	    if (score_2 > score_3)
	    {
		sprintf(buf, "Second place is %s with %li points.\n\r", second_place, score_2);
		stc(buf, ch);
		sprintf(buf, "Third place is %s with %li points.\n\r", third_place, score_3);
		stc(buf, ch);
	    }
	    else
	    {
		sprintf(buf, "Second place is %s with %li points.\n\r", third_place, score_3);
		stc(buf, ch);
		sprintf(buf, "Third place is %s with %li points.\n\r", second_place, score_2);
		stc(buf, ch);
	    }
	}
	else if (score_2 >= score_3)
	{
	    sprintf(buf, "First place is %s with %li points.\n\r", second_place, score_2);
	    stc(buf, ch);
	    if (score_1 > score_3)
	    {
		sprintf(buf, "Second place is %s with %li points.\n\r", first_place, score_1);
		stc(buf, ch);
		sprintf(buf, "Third place is %s with %li points.\n\r", third_place, score_3);
		stc(buf, ch);
	    }
	    else
	    {
		sprintf(buf, "Second place is %s with %li points.\n\r", third_place, score_3);
		stc(buf, ch);
		sprintf(buf, "Third place is %s with %li points.\n\r", first_place, score_1);
		stc(buf, ch);
	    }
	}
	else
	{
	    sprintf(buf, "First place is %s with %li points.\n\r", third_place, score_3);
	    stc(buf, ch);
	    if (score_1 > score_2)
	    {
		sprintf(buf,"Second place is %s with %li points.\n\r", first_place, score_1);
		stc(buf, ch);
		sprintf(buf,"Third place is %s with %li points.\n\r", second_place, score_2);
		stc(buf, ch);
	    }
	    else
	    {
		sprintf(buf, "Second place is %s with %li points.\n\r", second_place, score_2);
		stc(buf, ch);
		sprintf(buf, "Third place is %s with %li points.\n\r", first_place, score_1);
		stc(buf, ch);
	    }
	}
	return;
    }

    if (p_score < 1) p_score = 1;

    k_score = ch->pcdata->score[SCORE_TOTAL_XP];

    if (k_score < 1) sprintf(buf, "Exp from kills / time played: +0.\n\r");
    else 
    {
	t_score = k_score/p_score;
	if (t_score < 0) t_score = 0;
	sprintf(buf, "Exp from kills / time played: +%li.\n\r", t_score);
    }
    stc(buf, ch);

    p_score = ch->stance[1];
    p_score += ch->stance[2];
    p_score += ch->stance[3];
    p_score += ch->stance[4];
    p_score += ch->stance[5];
    p_score += ch->stance[6];
    p_score += ch->stance[7];
    p_score += ch->stance[8];
    p_score += ch->stance[9];
    p_score += ch->stance[10];
    p_score *= 0.25;
    sprintf(buf, "Stance Total (out of 500): +%li.\n\r", p_score);
    stc(buf, ch);
    t_score += p_score;

    p_score = ch->pcdata->score[SCORE_HIGH_LEVEL];
    sprintf(buf, "Toughest mob killed: +%li.\n\r", p_score);
    stc(buf, ch);
    t_score += p_score;

    k_score = ch->pcdata->score[SCORE_NUM_QUEST];
    if (k_score > 100) k_score = 100;
    sprintf(buf, "Quests completed (10 times %li, 1000 max): %li.\n\r", k_score, (k_score * 10));
    stc(buf, ch);
    t_score += k_score * 10;

    p_score = ch->pcdata->score[SCORE_QUEST];
    if (p_score > 0 && ch->pcdata->score[SCORE_NUM_QUEST] > 0)
    {
	p_score /= ch->pcdata->score[SCORE_NUM_QUEST];
	sprintf(buf, "Average qp earned per completed quest: +%li.\n\r", p_score);
    }
    else sprintf(buf, "Average qp earned per completed quest: +0.\n\r");
    stc(buf, ch);
    t_score += p_score;

    sprintf(buf, "Total score: %li.\n\r", t_score);
    stc(buf, ch);

    if (score_2 < lowest) lowest = score_2;
    if (score_3 < lowest) lowest = score_3;
    if (!str_cmp(first_place, ch->name)) lowest = score_1;
    else if (!str_cmp(second_place, ch->name)) lowest = score_2;
    else if (!str_cmp(third_place, ch->name)) lowest = score_3;
    else if (t_score < lowest) return;
    if (lowest == score_1)
    {
	strcpy(first_place, ch->name);
	score_1 = t_score;
    }
    else if (lowest == score_2)
    {
	strcpy(second_place, ch->name);
	score_2 = t_score;
    }
    else if (lowest == score_3)
    {
	strcpy(third_place, ch->name);
	score_3 = t_score;
    }
    return;
}

long get_totalscore( CHAR_DATA *ch )
{
    long p_score = (ch->played + (int) (current_time - ch->logon) );
    long k_score = 0;
    long t_score = 0;
    long lowest = score_1;

    if (IS_NPC(ch)) return 0;

    if (p_score < 1) p_score = 1;

    k_score = ch->pcdata->score[SCORE_TOTAL_XP];

    if (k_score >= 1)
    {
	t_score = k_score/p_score;
	if (t_score < 0) t_score = 0;
    }

    p_score = ch->stance[1];
    p_score += ch->stance[2];
    p_score += ch->stance[3];
    p_score += ch->stance[4];
    p_score += ch->stance[5];
    p_score += ch->stance[6];
    p_score += ch->stance[7];
    p_score += ch->stance[8];
    p_score += ch->stance[9];
    p_score += ch->stance[10];
    p_score *= 0.25;
    t_score += p_score;

    p_score = ch->pcdata->score[SCORE_HIGH_LEVEL];
    t_score += p_score;

    k_score = ch->pcdata->score[SCORE_NUM_QUEST];
    if (k_score > 100) k_score = 100;
    t_score += k_score * 10;

    p_score = ch->pcdata->score[SCORE_QUEST];
    if (p_score > 0 && ch->pcdata->score[SCORE_NUM_QUEST] > 0)
    {
	p_score /= ch->pcdata->score[SCORE_NUM_QUEST];
    }
    t_score += p_score;

    if (score_2 < lowest) lowest = score_2;
    if (score_3 < lowest) lowest = score_3;
    if (!str_cmp(first_place, ch->name)) lowest = score_1;
    else if (!str_cmp(second_place, ch->name)) lowest = score_2;
    else if (!str_cmp(third_place, ch->name)) lowest = score_3;
    else if (t_score < lowest) return t_score;
    if (lowest == score_1)
    {
	strcpy(first_place,ch->name);
	score_1 = t_score;
    }
    else if (lowest == score_2)
    {
	strcpy(second_place,ch->name);
	score_2 = t_score;
    }
    else if (lowest == score_3)
    {
	strcpy(third_place,ch->name);
	score_3 = t_score;
    }
    return t_score;
}

/* Exp/level command redone - Kai */
void do_level( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];
    char skill[50];

    one_argument(argument, arg);

    stc("#l---------------------------------------------#n\n\r", ch);
    stc("Stances       #l|#n Styles        #l|#n Weapons\n\r", ch);
    stc("#l---------------------------------------------#n\n\r", ch);
    sprintf(skill, "Viper   : %3li #l|#n Unarmed : %3li #l|#n Unarmed: %4li\n\r", ch->stance[1], ch->style[1], ch->weapon[1]);
    stc(skill, ch);
    sprintf(skill, "Crane   : %3li #l|#n Natural : %3li #l|#n Slash  : %4li\n\r", ch->stance[2], ch->style[2], ch->weapon[2]);
    stc(skill, ch);
    sprintf(skill, "Crab    : %3li #l|#n One Hand: %3li #l|#n Pierce : %4li\n\r", ch->stance[3], ch->style[3], ch->weapon[3]);
    stc(skill, ch);
    sprintf(skill, "Mongoose: %3li #l|#n Shield  : %3li #l|#n Blunt  : %4li\n\r", ch->stance[4], ch->style[4], ch->weapon[4]);
    stc(skill, ch);
    sprintf(skill, "Bull    : %3li #l|#n Two Hand: %3li #l|#n\n\r", ch->stance[5], ch->style[5]);
    stc(skill, ch);
    sprintf(skill, "Mantis  : %3li #l|#n Two Weap: %3li #l|#n\n\r", ch->stance[6], ch->style[6]);
    stc(skill, ch);
    sprintf(skill, "Dragon  : %3li #l|#n               #l|#n\n\r", ch->stance[7]);
    stc(skill, ch);
    sprintf(skill, "Tiger   : %3li #l|#n               #l|#n\n\r", ch->stance[8]);
    stc(skill, ch);
    sprintf(skill, "Monkey  : %3li #l|#n               #l|#n\n\r", ch->stance[9]);
    stc(skill, ch);
    sprintf(skill, "Swallow : %3li #l|#n               #l|#n\n\r", ch->stance[10]);
    stc(skill, ch);
    stc("#l---------------------------------------------#n\n\r", ch);
    switch(ch->stance[11])
    {
	default: strcpy(skill, "Autostance: None\n\r"); break;
	case 1: strcpy(skill, "Autostance: Viper\n\r"); break;
	case 2: strcpy(skill, "Autostance: Crane\n\r"); break;
	case 3: strcpy(skill, "Autostance: Crab\n\r"); break;
	case 4: strcpy(skill, "Autostance: Mongoose\n\r"); break;
	case 5: strcpy(skill, "Autostance: Bull\n\r"); break;
	case 6: strcpy(skill, "Autostance: Mantis\n\r"); break;
	case 7: strcpy(skill, "Autostance: Dragon\n\r"); break;
	case 8: strcpy(skill, "Autostance: Tiger\n\r"); break;
	case 9: strcpy(skill, "Autostance: Monkey\n\r"); break;
	case 10: strcpy(skill, "Autostance: Swallow\n\r"); break;
    }
    stc(skill, ch);
    switch(ch->stance[0])
    {
	default: strcpy(skill, "Stance: None\n\r"); break;
	case 1: strcpy(skill, "Stance: Viper\n\r"); break;
	case 2: strcpy(skill, "Stance: Crane\n\r"); break;
	case 3: strcpy(skill, "Stance: Crab\n\r"); break;
	case 4: strcpy(skill, "Stance: Mongoose\n\r"); break;
	case 5: strcpy(skill, "Stance: Bull\n\r"); break;
	case 6: strcpy(skill, "Stance: Mantis\n\r"); break;
	case 7: strcpy(skill, "Stance: Dragon\n\r"); break;
	case 8: strcpy(skill, "Stance: Tiger\n\r"); break;
	case 9: strcpy(skill, "Stance: Monkey\n\r"); break;
	case 10: strcpy(skill, "Stance: Swallow\n\r"); break;
    }
    stc(skill, ch);
    switch(ch->style[0])
    {
	default: strcpy(skill, "Style: BUGGED\n\r"); break;
	case 1: strcpy(skill, "Style: Unarmed\n\r"); break;
	case 2: strcpy(skill, "Style: Natural\n\r"); break;
	case 3: strcpy(skill, "Style: One-Handed Weapon\n\r"); break;
	case 4: strcpy(skill, "Style: Shield\n\r"); break;
	case 5: strcpy(skill, "Style: Two-Handed Weapon\n\r"); break;
	case 6: strcpy(skill, "Style: Two Weapon\n\r"); break;
    }
    stc(skill, ch);
    stc("#l---------------------------------------------#n\n\r", ch);
    if (IS_VAMPIRE(ch)) stc("You are a Vampire.\n\r", ch);
    if (IS_WEREWOLF(ch)) stc("You are a Werewolf.\n\r", ch);
    if (IS_SORCERER(ch)) stc("You are a Sorcerer.\n\r", ch);
    if (IS_HIGHLANDER(ch)) stc("You are a Highlander.\n\r", ch);
    if (IS_MONK(ch)) stc("You are a Monk.\n\r", ch);
    if (IS_NINJA(ch)) stc("You are a Ninja.\n\r", ch);
    if (IS_DROW(ch)) stc("You are a Drow.\n\r", ch);
    if (IS_DRAGON(ch)) stc("You are a Dragon.\n\r", ch);
    if (IS_ELEM(ch)) stc("You are an Elementalist.\n\r", ch);
    if (IS_MAGE(ch)) stc("You are a Mage.\n\r", ch);

    if (!IS_WEREWOLF(ch)) sprintf(skill, "Generation: %li\n\r", ch->vampgen);
    else sprintf(skill, "Rank: %li\n\r", ch->pcdata->rank);
    stc(skill, ch);

    return;
}

bool play_yet( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    char *strtime;
    long current_date;

    strtime = ctime( &current_time );
    strtime[strlen(strtime)-1] = '\0';

    if (IS_NPC(ch)) return TRUE;

    current_date = date_to_int(strtime);
    if (current_date >= ch->pcdata->denydate) return TRUE;
    else
    {
	sprintf(buf,"You are denied for %li more days.\n\r",ch->pcdata->denydate - current_date);
	send_to_char(buf,ch);
    }
    return FALSE;
}

long date_to_int( char *argument )
{
    char buf[MAX_STRING_LENGTH];
    long day_count = 0;

    if (argument[0] == '\0') return 0;

    argument = one_argument( argument, buf );

    if (buf[0] == '\0') return 0;

    argument = one_argument( argument, buf );

    if (buf[0] == '\0') return 0;

    if      (!str_cmp(buf,"Jan")) day_count = 0;
    else if (!str_cmp(buf,"Feb")) day_count = 31;
    else if (!str_cmp(buf,"Mar")) day_count = 59;
    else if (!str_cmp(buf,"Apr")) day_count = 90;
    else if (!str_cmp(buf,"May")) day_count = 120;
    else if (!str_cmp(buf,"Jun")) day_count = 151;
    else if (!str_cmp(buf,"Jul")) day_count = 181;
    else if (!str_cmp(buf,"Aug")) day_count = 212;
    else if (!str_cmp(buf,"Sep")) day_count = 242;
    else if (!str_cmp(buf,"Oct")) day_count = 273;
    else if (!str_cmp(buf,"Nov")) day_count = 304;
    else if (!str_cmp(buf,"Dec")) day_count = 334;

    argument = one_argument( argument, buf );

    if (buf[0] == '\0') return 0;
    if (!is_number(buf) || atoi(buf) < 1) return 0;

    day_count += atoi(buf);

    argument = one_argument( argument, buf );
    argument = one_argument( argument, buf );
    if (buf[0] == '\0') return 0;
    if (!is_number(buf) || atoi(buf) < 1) return 0;

    day_count += (365 * atoi(buf));

    return day_count;
}

