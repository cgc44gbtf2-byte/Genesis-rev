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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
/*
 * Local functions.
 */
void	talk_channel	args( ( CHAR_DATA *ch, char *argument, long channel, const char *verb ) );

/*
 * Generic channel function.
 */
void talk_channel( CHAR_DATA *ch, char *argument, long channel, const char *verb )
{
    char buf[MAX_STRING_LENGTH];
    char mess[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    long position;

    if (argument[0] == '\0')
    {
	sprintf(buf, "%s what?\n\r", verb);
	buf[0] = UPPER(buf[0]);
	stc(buf, ch);
	return;
    }

    if (IS_EXTRA(ch, GAGGED))
    {
	sprintf(buf, "You can't %s with a gag on!\n\r", verb);
	stc(buf, ch);
	return;
    }

    if (!IS_NPC(ch) && IS_SET(ch->act, PLR_SILENCE))
    {
	sprintf(buf, "You can't %s.\n\r", verb);
	stc(buf, ch);
	return;
    }

    if (ch->in_room != NULL && IS_SET(ch->in_room->added_flags, ROOM2_SILENCE) && !IS_BUI(ch))
    {
	sprintf(buf, "You can't %s.\n\r", verb);
	stc(buf, ch);
	return;
    }

    REMOVE_BIT(ch->deaf, channel);
    if (!str_cmp(ch->name, "Akuma")) sprintf(mess, "#r%s", argument);
    else if (!str_cmp(ch->name, "Chaos")) sprintf(mess, "#7%s", argument);
    else if (!str_cmp(ch->name, "Dude")) sprintf(mess, "#C%s", argument);
    else if (!str_cmp(ch->name, "Shane")) sprintf(mess, "#R%s", argument);
    else if (!str_cmp(ch->name, "Corruption")) sprintf(mess, "#0%s", argument);
    else sprintf(mess, "%s", argument);

    switch (channel)
    {
    default:
	sprintf(buf, "You %s '#G$t#n'.", verb);
	position = ch->position;
	ch->position = POS_STANDING;
	act(buf, ch, mess, NULL, TO_CHAR);
	ch->position = position;
	sprintf(buf, "%s %ss '#G$t#n'.", ch->name, verb);
	break;

    case CHANNEL_IMMTALK:
	sprintf(buf, "#G%s: $t", ch->name);
	position = ch->position;
	ch->position = POS_STANDING;
	act(buf, ch, mess, NULL, TO_CHAR);
	ch->position = position;
	break;

    case CHANNEL_HONOR:
	sprintf(buf, "#g[#C%s#g] #n'#C$t#n'.", ch->name);
	position = ch->position;
	ch->position = POS_STANDING;
	act(buf, ch, mess, NULL, TO_CHAR);
	ch->position = position;
	break;

    case CHANNEL_VAMPTALK:
	sprintf(buf, "#R<<#C%s#R>> #c$t#n", ch->name);
	position = ch->position;
	ch->position = POS_STANDING;
	act(buf, ch, mess, NULL, TO_CHAR);
	ch->position = position;
	break;

    case CHANNEL_HOWL:
	sprintf(buf, "You howl '#c$t#n'.");
	position = ch->position;
	ch->position = POS_STANDING;
	act(buf, ch, mess, NULL, TO_CHAR);
	ch->position = position;
	sprintf(buf, "%s howls '#c$t#n'.", ch->name);
	break;

    case CHANNEL_MIKTALK:
        sprintf(buf, "#c**#r%s#c** #n'#r$t#n'.", ch->name);
	position = ch->position;
	ch->position = POS_STANDING;
	act(buf, ch, mess, NULL, TO_CHAR);
	ch->position = position;
	break;

    case CHANNEL_ELEM:
	sprintf(buf, "#L-}#n%s#L{- #n'#r$t#n'.", ch->name);
	position = ch->position;
	ch->position = POS_STANDING;
	act(buf, ch, mess, NULL, TO_CHAR);
	ch->position = position;
	break;

    case CHANNEL_MONK:
	sprintf(buf, "#7[[#0%s#7]] #n'#7$t#n'.", ch->name);
	position = ch->position;
	ch->position = POS_STANDING;
	act(buf, ch, mess, NULL, TO_CHAR);
	ch->position = position;
	break;
    
    case CHANNEL_TELEPATHY:
	sprintf(buf, ":%s: '#c$t#n'.", ch->name);
	position = ch->position;
	ch->position = POS_STANDING;
	act(buf, ch, mess, NULL, TO_CHAR);
	ch->position = position;
	break;

    case CHANNEL_SORCTALK:
	sprintf(buf, "{%s} '#c$t#n'.", ch->name);
	position = ch->position;
	ch->position = POS_STANDING;
	act(buf, ch, mess, NULL, TO_CHAR);
	ch->position = position;
	break;

    case CHANNEL_DROW:
	sprintf(buf, "#P<(#0%s#P)> #n'#p$t#n'.", ch->name);
	position = ch->position;
	ch->position = POS_STANDING;
	act(buf, ch, mess, NULL, TO_CHAR);
	ch->position = position;
	break;

    case CHANNEL_MAGETALK:
	sprintf(buf, "#o>|#c%s#o|<#n '#0$t#n'.", ch->name);
        position = ch->position;
        ch->position = POS_STANDING;
        act(buf, ch, mess, NULL, TO_CHAR);
        ch->position = position;
        break;

    case CHANNEL_OOC:
	sprintf(buf, "#C(OOC) #r%s#n: #G$t#n", ch->name);
	position = ch->position;
	ch->position = POS_STANDING;
	act(buf, ch, mess, NULL, TO_CHAR);
	ch->position = position;
	break;

    }

    for (d = descriptor_list; d != NULL; d = d->next)
    {
	CHAR_DATA *och;
	CHAR_DATA *vch;

	och = d->original ? d->original : d->character;
	vch = d->character;

	if ((d->connected != CON_PLAYING && channel != CHANNEL_IMMTALK && channel != CHANNEL_HONOR)
	 || d->connected > CON_PLAYING) continue;

	if (vch != ch && !IS_SET(och->deaf, channel))
	{
	    if (!IS_IMM(och))
	    {
		if (channel == CHANNEL_IMMTALK) continue;
		if (channel == CHANNEL_HONOR && !IS_SET(och->act, PLR_HONOR) && !IS_BUI(och)) continue;
		if (channel == CHANNEL_VAMPTALK && !IS_VAMPIRE(och)) continue;
		if (channel == CHANNEL_HOWL && !IS_WEREWOLF(och))
		{
		    if (och->in_room == ch->in_room)
			act("$n throws back $s head and howls loudly.", ch, NULL, och, TO_VICT);
		    else if (och->in_room->area == ch->in_room->area)
			act("You hear a loud howl nearby.", ch, NULL, och, TO_VICT);
		    else
			act("You hear a loud howl in the distance.", ch, NULL, och, TO_VICT);
		    continue;
		}
		if (channel == CHANNEL_TELEPATHY && !IS_DRAGON(och)) continue;
		if (channel == CHANNEL_SORCTALK && !IS_SORCERER(och)) continue;
		if (channel == CHANNEL_MIKTALK && !IS_NINJA(och))continue;
		if (channel == CHANNEL_HIGHTALK && !IS_HIGHLANDER(och))continue;
		if (channel == CHANNEL_MAGETALK && !IS_MAGE(och)) continue;
		if (channel == CHANNEL_ELEM && !IS_ELEM(och)) continue;
		if (channel == CHANNEL_DROW && !IS_DROW(och)) continue;
                if (channel == CHANNEL_MONK && !IS_MONK(och)) continue;
	    }
	    if (channel == CHANNEL_YELL && vch->in_room->area != ch->in_room->area) continue;

	    position = vch->position;
	    vch->position = POS_STANDING;
	    act(buf, ch, mess, vch, TO_VICT);
	    vch->position = position;
	}
    }
    return;
}

void do_chat( CHAR_DATA *ch, char *argument )
{
    if (IS_NPC(ch) || IS_SET(ch->deaf, CHANNEL_CHAT))
    {
	stc("You aren't on that channel!\n\r", ch);
	return;
    }
    talk_channel(ch, argument, CHANNEL_CHAT, "chat");
    return;
}

void do_newbie( CHAR_DATA *ch, char *argument )
{
    if (IS_NPC(ch) || IS_SET(ch->deaf, CHANNEL_NEWBIE))
    {
	stc("You aren't on that channel!\n\r", ch);
	return;
    }
    talk_channel(ch, argument, CHANNEL_NEWBIE, "newbie");
    return;
}

void do_music( CHAR_DATA *ch, char *argument )
{
    if (IS_NPC(ch) || IS_SET(ch->deaf, CHANNEL_MUSIC))
    {
	stc("You aren't on that channel!\n\r", ch);
	return;
    }
    talk_channel(ch, argument, CHANNEL_MUSIC, "music");
    return;
}

void do_shout( CHAR_DATA *ch, char *argument )
{
    if (IS_SET(ch->deaf, CHANNEL_SHOUT))
    {
	stc("You aren't on that channel!\n\r", ch);
	return;
    }
    talk_channel(ch, argument, CHANNEL_SHOUT, "shout");
    WAIT_STATE(ch, 12);
    return;
}

void do_yell( CHAR_DATA *ch, char *argument )
{
    if (IS_SET(ch->deaf, CHANNEL_YELL))
    {
	stc("You aren't on that channel!\n\r", ch);
	return;
    }
    talk_channel(ch, argument, CHANNEL_YELL, "yell");
    return;
}

void do_ooc( CHAR_DATA *ch, char *argument )
{
    if (IS_NPC(ch) || IS_SET(ch->deaf, CHANNEL_OOC))
    {
	stc("You aren't on that channel!\n\r", ch);
	return;
    }
    talk_channel(ch, argument, CHANNEL_OOC, "ooc");
    return;
}

void do_miktalk( CHAR_DATA *ch, char *argument )
{
    if ((!IS_NINJA(ch) && !IS_IMM(ch)) || IS_SET(ch->deaf, CHANNEL_MIKTALK))
    {
	stc("You aren't on that channel!\n\r", ch);
	return;
    }
    talk_channel(ch, argument, CHANNEL_MIKTALK, "miktalk");
    return;
}

void do_hightalk( CHAR_DATA *ch, char *argument )
{
    if ((!IS_HIGHLANDER(ch) && !IS_IMM(ch)) || IS_SET(ch->deaf, CHANNEL_HIGHTALK))
    {
	stc("You aren't on that channel!\n\r", ch);
	return;
    }
    talk_channel(ch, argument, CHANNEL_HIGHTALK, "hightalk");
    return;
}

void do_sorctalk( CHAR_DATA *ch, char *argument )
{
    if ((!IS_SORCERER(ch) && !IS_IMM(ch)) || IS_SET(ch->deaf, CHANNEL_SORCTALK))
    {
        stc("You aren't on that channel!\n\r", ch);
        return;
    }
    talk_channel(ch, argument, CHANNEL_SORCTALK, "sorctalk");
    return;
}


void do_spiritspeak( CHAR_DATA *ch, char *argument)
{
    if ((!IS_ELEM(ch) && !IS_IMM(ch)) || IS_SET(ch->deaf, CHANNEL_ELEM))
    {
	stc("You aren't on that channel!\n\r", ch);
	return;
    }
    talk_channel(ch, argument, CHANNEL_ELEM, "spiritspeak");
    return;
}

void do_immtalk( CHAR_DATA *ch, char *argument )
{
    talk_channel(ch, argument, CHANNEL_IMMTALK, "immtalk");
    return;
}

void do_honor( CHAR_DATA *ch, char *argument )
{
    if (!IS_BUI(ch) && !IS_SET(ch->act, PLR_HONOR))
    {
	stc("Huh?\n\r", ch);
	return;
    }
    talk_channel(ch,argument, CHANNEL_HONOR, "honor");
    return;
}

void do_vamptalk( CHAR_DATA *ch, char *argument )
{
    if ((!IS_VAMPIRE(ch) && !IS_IMM(ch)) || IS_SET(ch->deaf, CHANNEL_VAMPTALK))
    {
	stc("You aren't on that channel!\n\r", ch);
	return;
    }
    talk_channel(ch, argument, CHANNEL_VAMPTALK, "vamptalk");
    return;
}

void do_howl( CHAR_DATA *ch, char *argument )
{
    if ((!IS_WEREWOLF(ch) && !IS_IMM(ch)) || IS_SET(ch->deaf, CHANNEL_HOWL))
    {
	stc("You aren't on that channel!\n\r", ch);
	return;
    }
    talk_channel(ch, argument, CHANNEL_HOWL, "howls");
    return;
}

void do_telepathy( CHAR_DATA *ch, char *argument )
{
    if ((!IS_DRAGON(ch)  && !IS_IMM(ch)) || IS_SET(ch->deaf, CHANNEL_TELEPATHY))
    {
	stc("You aren't on that channel!\n\r", ch);
	return;
    }
    talk_channel(ch, argument, CHANNEL_TELEPATHY, "telepathy");
    return;
}

void do_magetalk( CHAR_DATA *ch, char *argument )
{
    if ((!IS_MAGE(ch) && !IS_IMM(ch)) || IS_SET(ch->deaf, CHANNEL_MAGETALK))
    {
	stc("You aren't on that channel!\n\r", ch);
	return;
    }
    talk_channel(ch, argument, CHANNEL_MAGETALK, "magetalk");
    return;
}

void do_sign( CHAR_DATA *ch, char *argument )
{
    if ((!IS_DROW(ch) && !IS_IMM(ch)) || IS_SET(ch->deaf, CHANNEL_DROW))
    {
	stc("You aren't on that channel!\n\r", ch);
	return;
    }
    talk_channel(ch, argument, CHANNEL_DROW, "sign");
    return;  
}

void do_monk( CHAR_DATA *ch, char *argument )
{
    if ((!IS_MONK(ch) && !IS_IMM(ch)) || IS_SET(ch->deaf, CHANNEL_MONK) )
    {
	stc("You aren't on that channel!\n\r", ch);
	return;
    }
    talk_channel(ch, argument, CHANNEL_MONK, "monk");
    return;
}
     
void do_say( CHAR_DATA *ch, char *argument )
{
    char mess[MAX_STRING_LENGTH];
    char speak[15];
    char endbit[5];
    char secbit[5];

    if (IS_EXTRA(ch, GAGGED))
    {
	stc("You can't speak with a gag on!\n\r", ch);
	return;
    }

    if (ch->in_room != NULL && IS_SET(ch->in_room->added_flags, ROOM2_SILENCE) && !IS_BUI(ch))
    {
	stc("Your words make no sound!\n\r", ch);
	return;
    }

    if (strlen(argument) > MAX_INPUT_LENGTH)
    {
	stc("Line too long.\n\r", ch);
	return;
    }

    if (argument[0] == '\0')
    {
	stc("Say what?\n\r", ch);
	return;
    }

    endbit[0] = argument[strlen(argument)-1];
    endbit[1] = '\0';

    if (strlen(argument) > 1) secbit[0] = argument[strlen(argument)-2];
    else secbit[0] = '\0';
    secbit[1] = '\0';

    if (!str_cmp(ch->name, "Akuma")) sprintf(mess, "#r%s", argument);
    else if (!str_cmp(ch->name, "Chaos")) sprintf(mess, "#7%s", argument);
    else if (!str_cmp(ch->name, "Dude")) sprintf(mess, "#C%s", argument);
    else if (!str_cmp(ch->name, "Shane")) sprintf(mess, "#R%s", argument);
    else if (!str_cmp(ch->name, "Corruption")) sprintf(mess, "#0%s", argument);
    else sprintf(mess, "#R%s", argument);
    strcat(mess, "#n");

    if (!IS_NPC(ch) && (IS_SET(ch->act,PLR_WOLFMAN) || IS_POLYAFF(ch, POLY_WOLF) || (IS_VAMPIRE(ch) && ch->pcdata->wolf > 0)))
    {
	if (number_percent() > 50) sprintf(speak,"growl");
	else sprintf(speak,"snarl");
    }
    else if (!IS_NPC(ch) && IS_POLYAFF(ch, POLY_CHIMERA)) sprintf(speak, "roar");
    else if (!IS_NPC(ch) && IS_POLYAFF(ch, POLY_ZULO)) sprintf(speak,"roar");
    else if (!IS_NPC(ch) && IS_POLYAFF(ch, POLY_ZULO)) sprintf(speak, "screech");
    else if (!IS_NPC(ch) && IS_POLYAFF(ch, POLY_BAT)) sprintf(speak,"squeak");
    else if (!IS_NPC(ch) && IS_POLYAFF(ch, POLY_DRAGONFORM)) sprintf(speak,"rumble");
    else if (!IS_NPC(ch) && IS_POLYAFF(ch, POLY_SERPENT)) sprintf(speak,"hiss");
    else if (!IS_NPC(ch) && IS_POLYAFF(ch, POLY_FROG)) sprintf(speak,"croak");
    else if (!IS_NPC(ch) && IS_POLYAFF(ch, POLY_RAVEN)) sprintf(speak,"squark");
    else if (!str_cmp(endbit,"!")) sprintf(speak,"exclaim");
    else if (!str_cmp(endbit,"?")) sprintf(speak,"ask");
    else if (secbit[0] != '\0' && !str_cmp(secbit,".") && !str_cmp(endbit,".")) sprintf(speak,"mutter");
    else if (!str_cmp(endbit,".")) sprintf(speak,"state");
    else if (secbit[0] != '\0' && !str_cmp(secbit,":") && !str_cmp(endbit,")")) sprintf(speak,"grin");
    else if (secbit[0] != '\0' && !str_cmp(secbit,":") && !str_cmp(endbit,"(")) sprintf(speak,"frown");
    else sprintf(speak,"say");

    act("You $t '$T'.", ch, speak, mess, TO_CHAR);
    act("$n $ts '$T'.", ch, speak, mess, TO_ROOM);

    room_text(ch, strlower(mess));
    return;
}

void room_text( CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    CHAR_DATA *mob;
    OBJ_DATA *obj;
    ROOMTEXT_DATA *rt;
    char arg[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    bool mobfound;
    bool hop;

    for (rt = ch->in_room->roomtext; rt != NULL; rt = rt->next)
    {
	if (!strcmp(argument,rt->input) || is_in(argument, rt->input) || all_in(argument, rt->input))
	{
    if (rt->name != NULL && rt->name[0] != '\0' && str_cmp(rt->name,"all") && str_cmp(rt->name,"|all*")) 
	    if (!is_in(ch->name, rt->name)) continue;
	    mobfound = TRUE;
	    if (rt->mob != 0)
	    {
		mobfound = FALSE;
		for (vch = char_list; vch != NULL; vch = vch_next)
		{
		    vch_next	= vch->next;
		    if (vch->in_room == NULL) continue;
		    if (!IS_NPC(vch)) continue;
		    if (vch->in_room == ch->in_room && vch->pIndexData->vnum == rt->mob)
		    {
			mobfound = TRUE;
			break;
		    }
		}
	    }
	    if (!mobfound) continue;
	    hop = FALSE;
	    switch(rt->type % RT_RETURN)
	    {
		case RT_SAY:
		    break;
		case RT_LIGHTS:
		    do_changelight(ch,"");
		    break;
		case RT_LIGHT:
		    REMOVE_BIT(ch->in_room->room_flags, ROOM_DARK);
		    break;
		case RT_DARK:
		    SET_BIT(ch->in_room->room_flags, ROOM_DARK);
		    break;
		case RT_OBJECT:
		    if (get_obj_index(rt->power) == NULL) return;
		    obj = create_object(get_obj_index(rt->power), ch->level);
		    if (IS_SET(rt->type, RT_TIMER)) obj->timer = 40;
		    if (CAN_WEAR(obj,ITEM_TAKE)) obj_to_char(obj,ch);
		    else obj_to_room(obj,ch->in_room);
		    if (!str_cmp(rt->choutput,"copy")) act(rt->output, ch, obj, NULL, TO_CHAR);
		    else act(rt->choutput, ch, obj, NULL, TO_CHAR);
		    if (!IS_SET(rt->type, RT_PERSONAL)) act(rt->output, ch, obj, NULL, TO_ROOM);
		    hop = TRUE;
		    break;
		case RT_MOBILE:
		    if (get_mob_index(rt->power) == NULL ) return;
		    mob = create_mobile(get_mob_index(rt->power));
		    char_to_room(mob,ch->in_room);
		    if (!str_cmp(rt->choutput,"copy")) act(rt->output, ch, NULL, mob, TO_CHAR);
		    else act(rt->choutput, ch, NULL, mob, TO_CHAR);
		    if (!IS_SET(rt->type, RT_PERSONAL)) act(rt->output, ch, NULL, mob, TO_ROOM);
		    hop = TRUE;
		    break;
		case RT_SPELL:
		    (*skill_table[rt->power].spell_fun) (rt->power, number_range(20,30), ch, ch);
		    break;
		case RT_PORTAL:
		    if (get_obj_index(OBJ_VNUM_PORTAL) == NULL) return;
		    obj = create_object(get_obj_index(OBJ_VNUM_PORTAL), 0);
		    obj->timer = 5;
		    obj->value[0] = rt->power;
		    obj->value[1] = 1;
		    obj_to_room(obj,ch->in_room);
		    break;
		case RT_TELEPORT:
		    if ( get_room_index(rt->power) == NULL) return;
		    if (!str_cmp(rt->choutput,"copy")) act(rt->output, ch, NULL, NULL, TO_CHAR);
		    else act(rt->choutput, ch, NULL, NULL, TO_CHAR);
		    if (!IS_SET(rt->type, RT_PERSONAL)) act(rt->output, ch, NULL, NULL, TO_ROOM);
		    char_from_room(ch);
		    char_to_room(ch,get_room_index(rt->power));
		    act("$n appears in the room.",ch,NULL,NULL,TO_ROOM);
		    do_look(ch,"auto");
		    hop = TRUE;
		    break;
		case RT_ACTION:
		    sprintf(arg,argument);
		    argument = one_argument( arg, arg1 );
		    argument = one_argument( arg, arg2 );
		    if ( (mob = get_char_room(ch, arg2) ) == NULL ) continue;
		    if (IS_NPC(mob)) SET_BIT(mob->act, ACT_COMMANDED);
		    interpret( mob, rt->output );
		    if (IS_NPC(mob)) REMOVE_BIT(mob->act, ACT_COMMANDED);
		    break;
		case RT_OPEN_LIFT:
		    open_lift(ch);
		    break;
		case RT_CLOSE_LIFT:
		    close_lift(ch);
		    break;
		case RT_MOVE_LIFT:
		    move_lift(ch,rt->power);
		    break;
		default:
		    break;
	    }
	    if (hop && IS_SET(rt->type, RT_RETURN) ) return;
	    else if (hop) continue;
	    if (!str_cmp(rt->choutput,"copy") && !IS_SET(rt->type, RT_ACTION))
	    	act( rt->output, ch, NULL, NULL, TO_CHAR );
	    else if (!IS_SET(rt->type, RT_ACTION))
	    	act( rt->choutput, ch, NULL, NULL, TO_CHAR );
	    if (!IS_SET(rt->type, RT_PERSONAL) && !IS_SET(rt->type, RT_ACTION) )
		act( rt->output, ch, NULL, NULL, TO_ROOM );
	    if (IS_SET(rt->type, RT_RETURN) ) return;
	}
    }
    return;
}

char *strlower(char *ip)
{
    static char buffer[MAX_STRING_LENGTH];
    long pos;

    for (pos = 0; pos < (MAX_STRING_LENGTH - 1) && ip[pos] != '\0'; pos++)
	buffer[pos] = tolower(ip[pos]);
    buffer[pos] = '\0';
    return buffer;
}

bool is_in(char *arg, char *ip)
{
    char *lo_arg;
    char cmp[MAX_STRING_LENGTH];
    long fitted;

    if (ip[0] != '|') return FALSE;
    cmp[0] = '\0';
    lo_arg = strlower(arg);
    do
    {
	ip += strlen(cmp) + 1;
	fitted = sscanf(ip, "%[^*]", cmp);
	if (strstr(lo_arg, cmp) != NULL) return TRUE;
    }
    while (fitted > 0);
    return FALSE;
}

bool all_in(char *arg, char *ip)
{
    char *lo_arg;
    char cmp[MAX_STRING_LENGTH];
    long fitted;

    if (ip[0] != '&') return FALSE;
    cmp[0] = '\0';
    lo_arg = strlower(arg);
    do
    {
	ip += strlen(cmp) + 1;
	fitted = sscanf(ip, "%[^*]", cmp);
	if (strstr(lo_arg, cmp) == NULL) return FALSE;
    }
    while (fitted > 0);
    return TRUE;
}	

void do_tell( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    long position;

    if (IS_NPC(ch) || IS_SET(ch->act, PLR_SILENCE) || IS_EXTRA(ch, GAGGED)
     || (ch->in_room != NULL && IS_SET(ch->in_room->added_flags, ROOM2_SILENCE)))
    {
	stc("Your message didn't get through.\n\r", ch);
	return;
    }

    argument = one_argument(argument, arg);

    if (arg[0] == '\0' || argument[0] == '\0')
    {
	stc("Tell whom what?\n\r", ch);
	return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL || IS_NPC(victim))
    {
	stc("They aren't here.\n\r", ch);
	return;
    }

    if (victim->desc == NULL)
    {
	act("$E is currently link dead.", ch, 0, victim, TO_CHAR);
	return;
    }

    if (!IS_IMM(ch) && IS_SET(victim->deaf, CHANNEL_TELL))
    {
	if (strlen(victim->pcdata->marriage) < 2 || str_cmp(ch->name, victim->pcdata->marriage))
	{
	    act("$E can't hear you.", ch, 0, victim, TO_CHAR);
	    return;
	}
    }

    act("You tell $N '#r$t#n'.", ch, argument, victim, TO_CHAR);

    position = victim->position;
    victim->position = POS_STANDING;
    act("$n tells you '#r$t#n'.", ch, argument, victim, TO_VICT);
    victim->position = position;
    victim->reply = ch;

    return;
}

void do_whisper( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    if (IS_EXTRA(ch, GAGGED))
    {
	stc("Not with a gag on!\n\r", ch);
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Syntax: whisper <person> <message>\n\r", ch );
	return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL)
    {
	stc("They aren't here.\n\r", ch);
	return;
    }

    if (!IS_AWAKE(victim))
    {
	act("$E cannot hear you.", ch, NULL, victim, TO_CHAR);
	return;
    }

    if (!IS_NPC(victim) && victim->desc == NULL)
    {
	act("$E is currently link dead.", ch, NULL, victim, TO_CHAR);
	return;
    }

    act("You whisper to $N '#r$t#n'.", ch, argument, victim, TO_CHAR);
    act("$n whispers to you '#r$t#n'.", ch, argument, victim, TO_VICT);
    act("$n whispers something to $N.", ch, NULL, victim, TO_NOTVICT);

    return;
}

void do_reply( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    long position;

    if (IS_NPC(ch) || IS_SET(ch->act, PLR_SILENCE) || IS_EXTRA(ch, GAGGED)
     || (ch->in_room != NULL && IS_SET(ch->in_room->added_flags, ROOM2_SILENCE)))
    {
	stc( "Your message didn't get through.\n\r", ch );
	return;
    }

    if ((victim = ch->reply) == NULL)
    {
	stc("They aren't here.\n\r", ch);
	return;
    }

    if (!IS_NPC(victim) && victim->desc == NULL)
    {
	act("$E is currently link dead.", ch, NULL, victim, TO_CHAR);
	return;
    }

    act("You reply to $N '#r$t#n'.", ch, argument, victim, TO_CHAR);

    position = victim->position;
    victim->position = POS_STANDING;
    act("$n replies '#r$t#n'.", ch, argument, victim, TO_VICT);
    victim->position = position;
    victim->reply = ch;

    return;
}

void do_emote( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char *plast;

    if (IS_NPC(ch))
    {
	stc("You can't show your emotions.\n\r", ch);
	return;
    }

    if (argument[0] == '\0')
    {
	stc("Pose what?\n\r", ch);
	return;
    }

    for (plast = argument; *plast != '\0'; plast++);

    strcpy(buf, argument);
    if (isalpha(plast[-1])) strcat( buf, "." );

    act( "$n $T", ch, NULL, buf, TO_CHAR );
    act( "$n $T", ch, NULL, buf, TO_ROOM );
    return;
}

void do_qui( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to QUIT, you have to spell it out.\n\r", ch );
    return;
}

void do_quit( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    char buf[MSL];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    smash_tilde(argument);

    if (IS_NPC(ch))
    {
	act("$n has left the game.", ch, NULL, NULL, TO_ROOM);
	extract_char(ch, TRUE);
	return;
    }

    /* Idle auto-quit reaches the original, whose descriptor lives on its NPC. */
    restore_player_control(ch);

    if (ch->fight_timer > 0)
    {
	stc("Not with a fight timer.\n\r", ch);
	return;
    }

    if (ch->position == POS_FIGHTING)
    {
	stc("No way! You are fighting.\n\r", ch);
	return;
    }

    /* Commit an ordinary live snapshot before discarding a note, consuming
     * shaped equipment or releasing a character. A backup alone is not the
     * authoritative login file. Initial ineligible sleepers keep old policy. */
    if (player_save_owner(ch) != NULL &&
        !(player_save_state(ch,PLAYER_SAVE_PRIMARY) & PLAYER_SAVE_PRIMARY)) {
        stc("Quit cancelled: your player save failed. You remain in the game; a retry is scheduled.\n\r",ch);
        return;
    }

    if (IS_MORE(ch, MORE_DARKNESS) && ch->in_room != NULL)
    {
	REMOVE_BIT(ch->in_room->added_flags, ROOM2_DARKNESS); 
	REMOVE_BIT(ch->more, MORE_DARKNESS);
	ch->pcdata->darkness = 60;
	stc("The darkness in the room gradually fades away to nothing.\n\r", ch);
	act("The darkness in the room gradually fades away to nothing.", ch, NULL, NULL, TO_ROOM);
    }

    if (IS_MORE(ch, MORE_SILENCE) && ch->in_room != NULL)
    {
	REMOVE_BIT(ch->in_room->added_flags, ROOM2_SILENCE);
	REMOVE_BIT(ch->more, MORE_SILENCE);
	ch->pcdata->silence = 60;
	stc("The room is no longer silent.\n\r", ch);
	act("The room is no longer silent.", ch, NULL, NULL, TO_ROOM);
    }

    REMOVE_BIT(ch->pcdata->flag, FLAG_AFK);

    send_to_char( "\n\r     Twas brillig, and the slithy toves\n\r", ch);
    send_to_char( "        Did gyre and gimble in the wade\n\r", ch);
    send_to_char( "     All mimsy were the borogroves,\n\r", ch);
    send_to_char( "        And the mome raths outgrabe.\n\r\n\r", ch);
    send_to_char( "     Beware the Jabberwock, my son\n\r", ch);
    send_to_char( "        The jaws that bite, the claws that catch!\n\r", ch);
    send_to_char( "     Beware the Jubjub bird, and shun\n\r", ch);
    send_to_char( "        The frumious Bandersnatch\n\r\n\r", ch);
    send_to_char( "     -- Alice through the looking glass\n\r", ch);
    send_to_char( "     Lewis Carroll\n\r\n\r", ch);

    /* Free note that might be there somehow */
    if (ch->pcdata->in_progress) {
        free_note(ch->pcdata->in_progress);
        ch->pcdata->in_progress = NULL;
    }

    /*
     * After extract_char the ch is no longer valid!
     */
    d = ch->desc;
    for (obj = ch->carrying; obj != NULL; obj = obj_next)
    {
	obj_next = obj->next_content;
	if (obj->wear_loc == WEAR_NONE) continue;
	if (obj->chobj != NULL && !IS_NPC(obj->chobj) && obj->chobj->pcdata->obj_vnum != 0)
	{
	    unequip_char(ch, obj);
	    extract_obj(obj);
	}
    }
    free_string(ch->short_descr);
    ch->short_descr = str_dup("");
    do_autosave(ch,"");

    if (ch->pcdata->obj_vnum != 0) act("$n slowly fades out of existance.", ch, NULL, NULL, TO_ROOM);
    else act("$n has left the game.", ch, NULL, NULL, TO_ROOM);

    if (d != NULL)
    {
	sprintf(log_buf, "%s@%s has quit.", ch->name, d->host);
	close_socket2(d);
    }
    else sprintf(log_buf, "%s has quit.", ch->name);
    log_string(log_buf);

    if (ch->in_room != NULL) char_from_room(ch);
    char_to_room(ch, get_room_index(ROOM_VNUM_DISCONNECTION));

    if ((obj = ch->pcdata->chobj) != NULL)
    {
	obj->chobj = NULL;
	ch->pcdata->chobj = NULL;
	extract_obj(obj);
    }
    if (!IS_IMM(ch))
    {
	sprintf(buf, "#r%s#n #ohas left the #7G #0E #7N #0E #7S #0I #7S#n.", ch->name);
	do_info(ch, buf);
    }
    extract_char(ch, TRUE);
    return;
}

void do_save( CHAR_DATA *ch, char *argument )
{
    if (player_save_owner(ch) == NULL) {
        stc("This character is not eligible for a player save.\n\r",ch);
        return;
    }
    unsigned int saved = player_save_state(ch,PLAYER_SAVE_ALL);
    if (saved == PLAYER_SAVE_ALL) stc("Saved.\n\r",ch);
    else if (saved & PLAYER_SAVE_PRIMARY)
        stc("Player saved, but backup failed. Backup retry scheduled while you remain loaded.\n\r",ch);
    else if (saved & PLAYER_SAVE_BACKUP)
        stc("Primary save failed; only the backup saved. Primary retry scheduled.\n\r",ch);
    else stc("Save failed. Retries are scheduled while you remain loaded.\n\r",ch);
    return;
}

void do_autosave( CHAR_DATA *ch, char *argument )
{
    player_save_state(ch,PLAYER_SAVE_ALL);
    return;
}

void do_follow( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];
    CHAR_DATA *victim;

    one_argument(argument, arg);

    if (arg[0] == '\0')
    {
	stc("Follow whom?\n\r", ch);
	return;
    }

    if (!IS_AWA(ch) && !IS_NPC(ch))
    {
	stc("You need to get 5 mkills before you may join a group.\n\r", ch);
	return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL)
    {
	stc("They aren't here.\n\r", ch);
	return;
    }

    if (IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL)
    {
	act("But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR);
	return;
    }

    if (victim == ch)
    {
	if (ch->master == NULL) stc("You already follow yourself.\n\r", ch);
	else stop_follower(ch);
	return;
    }

    if (ch->master != NULL) stop_follower(ch);

    add_follower(ch, victim);
    return;
}

void add_follower( CHAR_DATA *ch, CHAR_DATA *master )
{
    if (ch->master != NULL)
    {
	bug( "Add_follower: non-null master.", 0 );
	return;
    }

    ch->master = master;
    ch->leader = NULL;

    if (can_see(master, ch)) act("$n now follows you.", ch, NULL, master, TO_VICT);
    act("You now follow $N.",  ch, NULL, master, TO_CHAR);
    return;
}

void stop_follower( CHAR_DATA *ch )
{
    if (ch->master == NULL)
    {
	bug("Stop_follower: null master.", 0);
	return;
    }

    if (IS_AFFECTED(ch, AFF_CHARM))
    {
	REMOVE_BIT(ch->affected_by, AFF_CHARM);
	affect_strip(ch, gsn_charm_person);
    }

    if (can_see(ch->master, ch)) act("$n stops following you.", ch, NULL, ch->master, TO_VICT);
    act("You stop following $N.", ch, NULL, ch->master, TO_CHAR);

    ch->master = NULL;
    ch->leader = NULL;
    return;
}

void die_follower( CHAR_DATA *ch )
{
    CHAR_DATA *fch;
    CHAR_DATA *fch_next;

    if (ch->master != NULL) stop_follower(ch);

    ch->leader = NULL;

    for (fch = char_list; fch != NULL; fch = fch_next)
    {
	fch_next = fch->next;
	if (fch->leader == ch) fch->leader = fch;
	if (fch->master == ch) stop_follower(fch);
	if (!IS_NPC(ch) && IS_NPC(fch) && fch->lord != NULL && strlen(fch->lord) > 1 && !str_cmp(fch->lord, ch->name))
	{
	    act("$n slowly fades out of existance.", fch, NULL, NULL, TO_ROOM);
	    extract_char(fch, TRUE);
	}
    }

    return;
}

void do_order( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *och;
    CHAR_DATA *och_next;
    bool found;
    bool fAll;
    bool is_animal = FALSE;

    argument = one_argument(argument, arg);

    if (arg[0] == '\0' || argument[0] == '\0')
    {
	stc("Order whom to do what?\n\r", ch);
	return;
    }

    if (IS_SET(ch->act, PLR_SILENCE))
    {
	stc("Not while silenced.\n\r", ch);
	return;
    }

    if (IS_AFFECTED(ch, AFF_CHARM))
    {
	stc("You feel like taking, not giving, orders.\n\r", ch);
	return;
    }

    if (!str_cmp(arg, "all"))
    {
	fAll = TRUE;
	victim = NULL;
    }
    else
    {
	fAll = FALSE;
	if ((victim = get_char_room(ch, arg)) == NULL)
	{
	    stc("They aren't here.\n\r", ch);
	    return;
	}

	if (victim == ch)
	{
	    stc("Aye aye, right away!\n\r", ch);
	    return;
	}
	if (IS_BUI(victim)) return;
	if (is_in(argument,"|quit*gift*tok*token*claim*consent*propose*accept*vampi*reca*/*ha *hav*"))
	{
	    stc("You cannot make that sort of order.\n\r", ch);
	    return;
	}

	if (IS_VAMPIRE(ch) && IS_VAMPIRE(victim) && ch->vampgen >= victim->vampgen)
	{
	    act( "$N ignores your order.", ch, NULL, victim, TO_CHAR );
	    act( "You ignore $n's order.", ch, NULL, victim, TO_VICT );
	    return;
	}
	
	if (IS_NPC(victim)) switch ( victim->pIndexData->vnum )
	{
	    default:
		is_animal = FALSE;
		break;
	    case 3062: case 3066: case 5333: case 30006: case 30007:
	    case 30008: case 30009: case 30010: case 30013: case 30014:
		is_animal = TRUE;
		break;
	}
	else is_animal = FALSE;
	if ((!IS_AFFECTED(victim, AFF_CHARM) || victim->master != ch) && !(IS_VAMPIRE(ch) && IS_VAMPIRE(victim)))
	{
	    if (IS_NPC(ch) || IS_NPC(victim) || strlen(victim->pcdata->love) < 2 || str_cmp(victim->pcdata->love, ch->name))
	    {
		if (!is_animal || !IS_MORE(ch, MORE_ANIMAL_MASTER))
		{
		    if (!IS_SPIRIT(victim) || !IS_VAMPIRE(ch) || !IS_WEREWOLF(ch) || get_disc(ch,AUSPICE_THEURGE) < 3)
		    {
			send_to_char( "Do it yourself!\n\r", ch );
			return;
		    }
		}
	    }
	}
    }

    found = FALSE;
    is_animal = FALSE;
    for ( och = ch->in_room->people; och != NULL; och = och_next )
    {
	och_next = och->next_in_room;
	if ( och == ch ) continue;
	if (IS_NPC(och)) switch ( och->pIndexData->vnum )
	{
	    default:
		is_animal = FALSE;
		break;
	    case 3062: case 3066: case 5333: case 30006: case 30007:
	    case 30008: case 30009: case 30010: case 30013: case 30014:
		is_animal = TRUE;
		break;
	}
	else is_animal = FALSE;
	if ((fAll || och == victim) && ((IS_AFFECTED(och, AFF_CHARM) && och->master == ch)
	 || (IS_VAMPIRE(ch) && IS_VAMPIRE(och) && ch->vampgen < och->vampgen && !str_cmp(ch->clan,och->clan))
	 || (!IS_NPC(ch) && !IS_NPC(och) && strlen(och->pcdata->love) > 1 &&!str_cmp(och->pcdata->love,ch->name))
	 || (is_animal && IS_MORE(ch, MORE_ANIMAL_MASTER))
	 || (IS_SPIRIT(och) && IS_WEREWOLF(ch) && get_auspice(ch, AUSPICE_THEURGE) > 3)))
	{
	    found = TRUE;
	    act("$n orders you to '$t'.", ch, argument, och, TO_VICT);
	    if (IS_NPC(och)) SET_BIT(och->act, ACT_COMMANDED);
	    interpret(och, argument);
	    if (IS_NPC(och)) REMOVE_BIT(och->act, ACT_COMMANDED);
	    WAIT_STATE(ch,12);
	}
    }

    if (found) stc("Ok.\n\r", ch);
    else stc("You have no followers here.\n\r", ch);
    return;
}

void do_command( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];
    char action[MIL];
    char buf[MSL];
    CHAR_DATA *victim;
    CHAR_DATA *vch_next;
    long level;

    argument = one_argument(argument, arg);

    if (!IS_VAMPIRE(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (get_disc(ch, DISC_DOMINATE) < 1)
    {
	stc("You require level 1 Dominate to Command people.\n\r", ch);
	return;
    }

    if (arg[0] == '\0' || argument[0] == '\0')
    {
	stc("Command whom to do what?\n\r", ch);
	return;
    }

    if (get_disc(ch, DISC_DOMINATE) < 2) one_argument(argument, action);
    else strcpy(action, argument);

    if (is_in(action, "|def*quit*sac*gift*tok*claim*propose*vampi*reca*/*ha *hav*"))
    {
	stc("I think not.\n\r", ch);
	return;
    }

    level = get_disc(ch, DISC_DOMINATE) * 125;

    if (!str_cmp(arg, "all") && get_disc(ch, DISC_DOMINATE) >= 6)
    {
	sprintf(buf, "I think everyone wants to %s", action);
	do_say(ch, buf);
	WAIT_STATE(ch, 15);
	for (victim = ch->in_room->people; victim != NULL; victim = vch_next)
	{
	    vch_next = victim->next_in_room;
	    WAIT_STATE(ch, 15);
	    if (victim == ch) continue;
	    if (IS_BUI(victim)) continue;
	    if (is_safe(ch, victim)) continue;

	    if ((IS_VAMPIRE(victim) && victim->vampgen < ch->vampgen)
	     || (IS_NPC(victim) && victim->level > level))
	    {
		act("You shake off $N's suggestion.", victim, NULL, ch, TO_CHAR);
		act("$n shakes off $N's suggestion.", victim, NULL, ch, TO_NOTVICT);
		act("$n shakes off your suggestion.", victim, NULL, ch, TO_VICT);
		act("$s mind is too strong to overcome.", victim, NULL, ch, TO_VICT);
		continue;
	    }

	    if (IS_NPC(victim) || (IS_VAMPIRE(victim) && victim->vampgen > ch->vampgen)
	     || (strlen(victim->pcdata->conding) > 2 && !str_cmp(victim->pcdata->conding, ch->name)))
	    {
		act("You blink in confusion.", victim, NULL, NULL, TO_CHAR);
		act("$n blinks in confusion.", victim, NULL, NULL, TO_ROOM);
		strcpy(buf, "Yes, you're right, I do...");
		do_say(victim, buf);
		if (IS_NPC(victim)) SET_BIT(victim->act, ACT_COMMANDED);
		interpret(victim, action);
		if (IS_NPC(victim)) REMOVE_BIT(victim->act, ACT_COMMANDED);
		WAIT_STATE(victim, 15);
		continue;
	    }

	    if ((!IS_NPC(victim) && victim->pcdata->resist[WILL_DOMINATE] > 0)
	     || get_disc(ch, DISC_DOMINATE) < 7)
	    {
		act("You shake off $N's suggestion.", victim, NULL, ch, TO_CHAR);
		act("$n shakes off $N's suggestion.", victim, NULL, ch, TO_NOTVICT);
		act("$n shakes off your suggestion.", victim, NULL, ch, TO_VICT);
		continue;
	    }

	    if (!IS_NPC(victim)) victim->pcdata->resist[WILL_DOMINATE] = 15;
	    act("You blink in confusion.", victim, NULL, NULL, TO_CHAR);
	    act("$n blinks in confusion.", victim, NULL, NULL, TO_ROOM);
	    strcpy(buf, "Yes, you're right, I do...");
	    do_say(victim, buf);
	    if (IS_NPC(victim)) SET_BIT(victim->act, ACT_COMMANDED);
	    interpret(victim, action);
	    if (IS_NPC(victim)) REMOVE_BIT(victim->act, ACT_COMMANDED);
	    WAIT_STATE(victim, 15);
	}
	return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL)
    {
	stc("They aren't here.\n\r", ch);
	return;
    }

    if (victim == ch)
    {
	stc("How can you Command yourself??\n\r", ch);
	return;
    }

    if (IS_BUI(victim))
    {
	stc("I think not...\n\r", ch);
	return;
    }

    if (is_safe(ch, victim)) return;

    if (IS_NPC(victim)) sprintf(buf, "I think %s wants to %s", victim->short_descr, action);
    else if (IS_AFFECTED(victim, AFF_POLYMORPH)) sprintf(buf, "I think %s wants to %s", victim->morph, action);
    else sprintf(buf, "I think %s wants to %s", victim->name, action);
    do_say(ch, buf);
    WAIT_STATE(ch, 12);

    if ((IS_VAMPIRE(victim) && victim->vampgen < ch->vampgen)
     || (IS_NPC(victim) && victim->level > level))
    {
	act("You shake off $N's suggestion.", victim, NULL, ch, TO_CHAR);
	act("$n shakes off $N's suggestion.", victim, NULL, ch, TO_NOTVICT);
	act("$n shakes off your suggestion.", victim, NULL, ch, TO_VICT);
	act("$s mind is too strong to overcome.", victim, NULL, ch, TO_VICT);
	return;
    }

    if (IS_NPC(victim) || (IS_VAMPIRE(victim) && victim->vampgen > ch->vampgen)
     || (strlen(victim->pcdata->conding) > 2 && !str_cmp(victim->pcdata->conding, ch->name)))
    {
	act("You blink in confusion.", victim, NULL, NULL, TO_CHAR);
	act("$n blinks in confusion.", victim, NULL, NULL, TO_ROOM);
	strcpy(buf, "Yes, you're right, I do...");
	do_say(victim, buf);
	if (IS_NPC(victim)) SET_BIT(victim->act, ACT_COMMANDED);
	interpret(victim, action);
	if (IS_NPC(victim)) REMOVE_BIT(victim->act, ACT_COMMANDED);
	WAIT_STATE(victim, 15);
	return;
    }

    if ((!IS_NPC(victim) && victim->pcdata->resist[WILL_DOMINATE] > 0)
     || get_disc(ch, DISC_DOMINATE) < 7)
    {
	act("You shake off $N's suggestion.", victim, NULL, ch, TO_CHAR);
	act("$n shakes off $N's suggestion.", victim, NULL, ch, TO_NOTVICT);
	act("$n shakes off your suggestion.", victim, NULL, ch, TO_VICT);
	return;
    }

    if (!IS_NPC(victim)) victim->pcdata->resist[WILL_DOMINATE] = 10;
    act("You blink in confusion.", victim, NULL, NULL, TO_CHAR);
    act("$n blinks in confusion.", victim, NULL, NULL, TO_ROOM);
    strcpy(buf, "Yes, you're right, I do...");
    do_say(victim, buf);
    if (IS_NPC(victim)) SET_BIT(victim->act, ACT_COMMANDED);
    interpret(victim, action);
    if (IS_NPC(victim)) REMOVE_BIT(victim->act, ACT_COMMANDED);
    WAIT_STATE(victim, 15);
    return;
}

void do_group( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    bool grouped = FALSE;

    one_argument(argument, arg);

    if (arg[0] == '\0')
    {
	CHAR_DATA *gch;
	CHAR_DATA *leader;

	leader = (ch->leader != NULL) ? ch->leader : ch;
	sprintf(buf, "%s's group:\n\r", PERS(leader, ch));
	stc(buf, ch);

	for (gch = char_list; gch != NULL; gch = gch->next)
	{
	    if (is_same_group(gch, ch))
	    {
		sprintf(buf, "[%-16s] %4li/%4li hp %4li/%4li mana %5li xp\n\r",
		 capitalize(PERS(gch, ch)), gch->hit, gch->max_hit, gch->mana, gch->max_mana, gch->exp);
		stc(buf, ch);
	    }
	}
	return;
    }

    if (ch->master != NULL || (ch->leader != NULL && ch->leader != ch))
    {
	stc("But you are following someone else!\n\r", ch);
	return;
    }

    if (!str_cmp(arg,"all"))
    {
	for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
	{
	    vch_next = vch->next_in_room;
	    if ( vch == ch ) continue;
	    if ( vch->master != ch ) continue;
	    if ( is_same_group( vch, ch ) ) continue;
	    vch->leader = ch;
	    act( "$N joins $n's group.", ch, NULL, vch, TO_NOTVICT );
	    act( "You join $n's group.", ch, NULL, vch, TO_VICT    );
	    act( "$N joins your group.", ch, NULL, vch, TO_CHAR    );
	    grouped = TRUE;
	}
	if (!grouped) send_to_char("There is nobody here following you who isn't grouped already.\n\r",ch);
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->master != ch && ch != victim )
    {
	act( "$N isn't following you.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( is_same_group( victim, ch ) && ch != victim )
    {
	victim->leader = NULL;
	act( "$n removes $N from $s group.",   ch, NULL, victim, TO_NOTVICT );
	act( "$n removes you from $s group.",  ch, NULL, victim, TO_VICT    );
	act( "You remove $N from your group.", ch, NULL, victim, TO_CHAR    );
	return;
    }

    victim->leader = ch;
    act( "$N joins $n's group.", ch, NULL, victim, TO_NOTVICT );
    act( "You join $n's group.", ch, NULL, victim, TO_VICT    );
    act( "$N joins your group.", ch, NULL, victim, TO_CHAR    );
    return;
}



void do_gtell( CHAR_DATA *ch, char *argument )
{
    char buf[MSL];
    CHAR_DATA *gch;

    if (argument[0] == '\0')
    {
	stc("Tell your group what?\n\r", ch);
	return;
    }

    if (ch->in_room != NULL && IS_SET(ch->in_room->added_flags, ROOM2_SILENCE) && !IS_BUI(ch))
    {
	sprintf(buf, "You can't grouptell.\n\r");
	stc(buf, ch);
	return;
    }

    if (IS_SET(ch->act, PLR_NO_TELL))
    {
	stc("Your message didn't get through!\n\r", ch);
	return;
    }

    /*
     * Note use of send_to_char, so gtell works on sleepers.
     */
    sprintf(buf, "%s tells the group '#y%s#n'.\n\r", ch->name, argument);
    for (gch = char_list; gch != NULL; gch = gch->next)
    {
	if (is_same_group(gch, ch))
	    stc(buf, gch);
    }
    return;
}



/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group( CHAR_DATA *ach, CHAR_DATA *bch )
{
    if ( ach->leader != NULL ) ach = ach->leader;
    if ( bch->leader != NULL ) bch = bch->leader;
    return ach == bch;
}

void do_changelight(CHAR_DATA *ch, char *argument )
{
    if (IS_SET(ch->in_room->room_flags, ROOM_DARK))
    {
	REMOVE_BIT(ch->in_room->room_flags, ROOM_DARK);
	act("The room is suddenly filled with light!",ch,NULL,NULL,TO_CHAR);
	act("The room is suddenly filled with light!",ch,NULL,NULL,TO_ROOM);
	return;
    }
    SET_BIT(ch->in_room->room_flags, ROOM_DARK);
    act("The lights in the room suddenly go out!",ch,NULL,NULL,TO_CHAR);
    act("The lights in the room suddenly go out!",ch,NULL,NULL,TO_ROOM);
    return;
}

void open_lift( CHAR_DATA *ch )
{
    ROOM_INDEX_DATA *location;
    long in_room;

    in_room  = ch->in_room->vnum;
    location = get_room_index(in_room);

    if (is_open(ch)) return;

    act("The doors open.",ch,NULL,NULL,TO_CHAR);
    act("The doors open.",ch,NULL,NULL,TO_ROOM);
    move_door(ch);
    if (is_open(ch)) act("The doors close.",ch,NULL,NULL,TO_ROOM);
    if (!same_floor(ch,in_room)) act("The lift judders suddenly.",ch,NULL,NULL,TO_ROOM);
    if (is_open(ch)) act("The doors open.",ch,NULL,NULL,TO_ROOM);
    move_door(ch);
    open_door(ch,FALSE);
    char_from_room(ch);
    char_to_room(ch,location);
    open_door(ch,TRUE);
    move_door(ch);
    open_door(ch,TRUE);
    thru_door(ch,in_room);
    char_from_room(ch);
    char_to_room(ch,location);
    return;
}

void close_lift( CHAR_DATA *ch )
{
    ROOM_INDEX_DATA *location;
    long in_room;

    in_room  = ch->in_room->vnum;
    location = get_room_index(in_room);

    if (!is_open(ch)) return;
    act("The doors close.",ch,NULL,NULL,TO_CHAR);
    act("The doors close.",ch,NULL,NULL,TO_ROOM);
    open_door(ch,FALSE);
    move_door(ch);
    open_door(ch,FALSE);
    char_from_room(ch);
    char_to_room(ch,location);
    return;
}

void move_lift( CHAR_DATA *ch, long to_room )
{
    ROOM_INDEX_DATA *location;
    long in_room;

    in_room  = ch->in_room->vnum;
    location = get_room_index(in_room);

    if (is_open(ch)) act("The doors close.",ch,NULL,NULL,TO_CHAR);
    if (is_open(ch)) act("The doors close.",ch,NULL,NULL,TO_ROOM);
    if (!same_floor(ch,to_room)) act("The lift judders suddenly.",ch,NULL,NULL,TO_CHAR);
    if (!same_floor(ch,to_room)) act("The lift judders suddenly.",ch,NULL,NULL,TO_ROOM);
    move_door(ch);
    open_door(ch,FALSE);
    char_from_room(ch);
    char_to_room(ch,location);
    open_door(ch,FALSE);
    thru_door(ch,to_room);
    return;
}

bool same_floor( CHAR_DATA *ch, long cmp_room )
{
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
    {
	obj_next = obj->next_content;
	if ( obj->item_type != ITEM_PORTAL ) continue;
	if ( obj->pIndexData->vnum == 30001 ) continue;
	if ( obj->value[0] == cmp_room ) return TRUE;
	else return FALSE;
    }
    return FALSE;
}

bool is_open( CHAR_DATA *ch )
{
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
    {
	obj_next = obj->next_content;
	if ( obj->item_type != ITEM_PORTAL ) continue;
	if ( obj->pIndexData->vnum == 30001 ) continue;
	if ( obj->value[2] == 0 ) return TRUE;
	else return FALSE;
    }
    return FALSE;
}

void move_door( CHAR_DATA *ch )
{
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    ROOM_INDEX_DATA *pRoomIndex;

    for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
    {
	obj_next = obj->next_content;
	if ( obj->item_type != ITEM_PORTAL ) continue;
	if ( obj->pIndexData->vnum == 30001 ) continue;
	pRoomIndex = get_room_index(obj->value[0]);
	char_from_room(ch);
	char_to_room(ch,pRoomIndex);
	return;
    }
    return;
}

void thru_door( CHAR_DATA *ch, long doorexit )
{
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
    {
	obj_next = obj->next_content;
	if ( obj->item_type != ITEM_PORTAL ) continue;
	if ( obj->pIndexData->vnum == 30001 ) continue;
	obj->value[0] = doorexit;
	return;
    }
    return;
}

void open_door( CHAR_DATA *ch, bool be_open )
{
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
    {
	obj_next = obj->next_content;
	if ( obj->item_type != ITEM_PORTAL ) continue;
	if ( obj->pIndexData->vnum == 30001 ) continue;
	if ( obj->value[2] == 0 && !be_open ) obj->value[2] = 3;
	else if ( obj->value[2] == 3 && be_open ) obj->value[2] = 0;
	return;
    }
    return;
}
