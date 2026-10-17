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

void do_blur( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) ) return;

    if (!IS_WEREWOLF(ch))
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if (get_auspice(ch, AUSPICE_RAGABASH) < 1)
    {
	send_to_char("You require the level 1 Ragabash power to use Blur of the Milky Eye.\n\r",ch);
	return;
    }

    if ( IS_SET(ch->act, PLR_WIZINVIS) )
    {
	REMOVE_BIT(ch->act, PLR_WIZINVIS);
	send_to_char( "You slowly fade into existance.\n\r", ch );
	act("$n slowly fades into existance.",ch,NULL,NULL,TO_ROOM);
    }
    else
    {
	send_to_char( "You slowly fade out of existance.\n\r", ch );
	act("$n slowly fades out of existance.",ch,NULL,NULL,TO_ROOM);
	SET_BIT(ch->act, PLR_WIZINVIS);
    }
    return;
}

void do_blissful( CHAR_DATA *ch, char *argument )
{
    if (IS_NPC(ch)) return;

    if (!IS_WEREWOLF(ch))
    {
	stc("Huh?\n\r",ch);
	return;
    }

    if (get_auspice(ch,AUSPICE_RAGABASH) < 2)
    {
	stc("You require Ragabash 2 to use Blissful Ignorance.\n\r",ch);
	return;
    }

    if (IS_ITEMAFF(ch,ITEMA_BLISSFUL))
    {
	stc("You allow yourself to be seen.\n\r",ch);
	REMOVE_BIT(ch->itemaffect,ITEMA_BLISSFUL);
    }
    else
    {
	stc("You force those around you to forget you are there.\n\r",ch);
	SET_BIT(ch->itemaffect,ITEMA_BLISSFUL);
    }
    return;
}

void do_whelp (CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    AFFECT_DATA af;
    long sn;

    if (IS_NPC(ch)) return;

    if (!IS_WEREWOLF(ch))
    {
	stc("Huh?\n\r",ch);
	return;
    }

    if (get_auspice(ch,AUSPICE_RAGABASH) < 4)
    {
	stc("You require Ragabash 4 to use Whelp's Body.\n\r",ch);
	return;
    }

    if (argument[0] == '\0')
    {
	stc("Use Whelp's Body on whom?\n\r",ch);
	return;
    }

    if ((victim = get_char_room(ch,argument)) == NULL)
    {
	stc("They aren't here.\n\r",ch);
	return;
    }

    if (is_safe(victim,ch)) return;

    if ((sn = skill_lookup("whelpsbody")) < 0)
    {
	stc("Bug, report this to Akuma.\n\r",ch);
	return;
    }

    if (is_affected(victim, sn))
    {
	stc("They are already affected by Whelps Body.\n\r",ch);
	return;
    }

    af.type      = sn;
    af.location  = APPLY_HITROLL;
    af.modifier  = get_auspice(ch,AUSPICE_RAGABASH) * -15;
    af.duration  = 2;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    af.location  = APPLY_DAMROLL;
    affect_to_char( victim, &af );

    af.location = APPLY_AC;
    affect_to_char( victim, &af );

    act("You use Whelp's Body on $N.",ch,NULL,victim,TO_CHAR);
    act("$n concentrates on $N.",ch,NULL,victim,TO_NOTVICT);
    act("You feel as weak as a whelp as $n concentrates on you.",ch,NULL,victim,TO_VICT);
    WAIT_STATE(ch,12);
    return;
}

void do_mother( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MIL];
    char buf[MSL];

    argument = one_argument(argument, arg);

    if (IS_NPC(ch)) return;

    if (!IS_WEREWOLF(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (get_auspice(ch, AUSPICE_THEURGE) < 1)
    {
	stc("You require the level 1 Theurge power to use Mother's Touch.\n\r", ch);
	return;
    }

    if (ch->pcdata->obeah > 0)
    {
	if (ch->pcdata->obeah > 1)
	    sprintf(buf, "You cannot call upon your healing powers for another %li seconds.\n\r", ch->pcdata->obeah);
	else
	    sprintf(buf, "You cannot call upon your healing powers for another 1 second.\n\r");
	stc(buf, ch);
	return;
    }

    if (arg[0] == '\0')
    {
	stc("Who do you want to heal with Mother's Touch?\n\r", ch);
	return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL)
    {
	stc("They are not here.\n\r", ch);
	return;
    }

    if (ch == victim && get_auspice(ch, AUSPICE_THEURGE) < 2)
    {
	stc("You require Theurge 2 to use Grandmother's Touch.\n\r", ch);
	return;
    }

    act("You lay your hands upon $N.", ch, NULL, victim, TO_CHAR);
    act("$n's lays $s hands upon $N.", ch, NULL, victim, TO_NOTVICT);
    act("$n's lays $s hands upon you.", ch, NULL, victim, TO_VICT);
    ch->pcdata->obeah = 15;
    if (victim->hit < victim->max_hit)
    {
	victim->hit += get_auspice(ch, AUSPICE_THEURGE) * victim->max_hit * .03;
	update_pos(victim);
	if (victim->hit >= victim->max_hit)
	{
	    victim->hit = victim->max_hit;
	    stc("You have been completely healed!\n\r", victim);
	    act("$n's wounds have been completely healed!", victim, NULL, NULL, TO_ROOM);
	}
	else
	{
	    stc("Your wounds begin to heal!\n\r", victim);
	    act("$n's wounds begin to heal!", victim, NULL, NULL, TO_ROOM);
	}
    }
    WAIT_STATE(ch, 12);
    return;
}

void do_pulse( CHAR_DATA *ch, char *argument )
{
    char arg [MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_WEREWOLF(ch) )
    {
	send_to_char("Huh?\n\r",ch);
	return;
    }

    if (get_auspice(ch,AUSPICE_THEURGE) < 3)
    {
	stc("You require Theurge 3 to use Pulse of the Invisible.\n\r",ch);
	return;
    }

    if (IS_VAMPAFF(ch,VAM_SPIRITEYES) )
    {
	send_to_char("You can no longer sense things in the shadowlands.\n\r",ch);
	REMOVE_BIT(ch->vampaff, VAM_SPIRITEYES);
	REMOVE_BIT(ch->affected_by, AFF_SHADOWSIGHT);
	return;
    }
    send_to_char("You can now sense things in the shadowlands.\n\r",ch);
    SET_BIT(ch->vampaff, VAM_SPIRITEYES);
    SET_BIT(ch->affected_by, AFF_SHADOWSIGHT);
    return;
}

void do_grasp( CHAR_DATA *ch, char *argument )
{
    char arg [MAX_INPUT_LENGTH];  
    CHAR_DATA *victim;

    argument = one_argument( argument, arg );

    if ( IS_NPC(ch) ) return;

    if ( !IS_WEREWOLF(ch) ) 
    {
	stc("Huh?\n\r",ch);
	return;
    }

    if (get_auspice(ch,AUSPICE_THEURGE) < 4)
    {
	send_to_char("You require Theurge 4 to Grasp the Beyond.\n\r",ch);
	return;
    }
    if(ch->fight_timer > 0)
	{
	stc("Not with a fight timer.\n\r", ch);
	return;
	}
    if ( arg[0] == '\0' )
    {
        send_to_char( "Who do you want to pull with you into Umbra?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "They are not here.\n\r", ch );
        return;
    }

    if (ch == victim)
    {
	stc("All you have to do is step sideways.\n\r", ch);
	return;
    }

    if (victim->position == POS_FIGHTING)
    {
	stc("You can't get a grip on them while they are fighting.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(ch, AFF_SHADOWPLANE))
    {
	stc("You are already in the Umbra.\n\r", ch);
	return;
    }
    if (IS_AFFECTED(victim, AFF_SHADOWPLANE))
    {
	stc("They are already in the Umbra.\n\r", ch);
	return;
    }

    act("You grasp $N and step into the Umbra",ch,NULL,victim,TO_CHAR);
    act("$n grasps $N and steps into the Umbra.",ch,NULL,victim,TO_NOTVICT);
    act("$n grasps you and pulls you into the Umbra.",ch,NULL,victim,TO_VICT);
    SET_BIT(victim->affected_by, AFF_SHADOWPLANE);
    SET_BIT(ch->affected_by, AFF_SHADOWPLANE);
    return;
}

void do_lobotomy( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_WEREWOLF(ch))
    {
	stc("Huh?\n\r",ch);
	return;
    }
    if (get_auspice(ch, AUSPICE_THEURGE) < 5)
    {
	stc("Your require Theurge 5 to use Feral Lobotomy.\n\r",ch);
	return;
    }

    if ( arg[0] == '\0' )
    {
        send_to_char( "Who do you want to use Feral Lobotomy on?\n\r", ch );
        return;
    }
     
    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "They are not here.\n\r", ch );
        return;
    }

    if (ch == victim)
    {
	stc("You give yourself a lobotomy.\n\r",ch);
	return;
    }

    if (is_safe(victim,ch)) return;

    if (victim->mana < 1)
    {
	stc("There isn't much to mess around with in there.\n\r",ch);
	return;
    }

    act("You fuck with $N's head.",ch,NULL,victim,TO_CHAR);
    act("$n smirks as he concentrates on $N.",ch,NULL,victim,TO_NOTVICT);
    act("$n smirks, and suddenly you don't feel quite right in the head.",ch,NULL,victim,TO_VICT);
    victim->mana -= victim->max_mana * .20;
    WAIT_STATE(ch, 12);
    if (victim->mana < 0) victim->mana = 0;
    if (victim->fighting == NULL) multi_hit(victim,ch);
    return;
}

void do_wallgranite( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA  *obj;
    EXIT_DATA *pexit;
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *to_room;
    char       arg [MAX_INPUT_LENGTH];
    char       buf [MAX_INPUT_LENGTH];
    long        duration = number_range(30,90);
    long       direction, rev;

    one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_WEREWOLF(ch))
    {
	send_to_char("Huh?\n\r",ch);
	return;
    }

    if (get_auspice(ch,AUSPICE_PHILODOX) < 4)
    {
	send_to_char("You require the level 4 power of Philodox create a Wall of Granite.\n\r",ch);
	return;
    }

    if ( arg[0] == '\0')
    {
	send_to_char( "Which direction do you wish to make the wall?\n\r", ch );
	return;
    }

         if (!str_cmp(arg,"north") || !str_cmp(arg,"n")) 
	{direction = DIR_NORTH; rev = DIR_SOUTH;}
    else if (!str_cmp(arg,"south") || !str_cmp(arg,"s")) 
	{direction = DIR_SOUTH; rev = DIR_NORTH;}
    else if (!str_cmp(arg,"east")  || !str_cmp(arg,"e")) 
	{direction = DIR_EAST;  rev = DIR_WEST;}
    else if (!str_cmp(arg,"west")  || !str_cmp(arg,"w")) 
	{direction = DIR_WEST;  rev = DIR_EAST;}
    else if (!str_cmp(arg,"up")    || !str_cmp(arg,"u")) 
	{direction = DIR_UP;    rev = DIR_DOWN;}
    else if (!str_cmp(arg,"down")  || !str_cmp(arg,"d")) 
	{direction = DIR_DOWN;  rev = DIR_UP;}
    else
    {
	send_to_char( "Select direction: North, South, East, West, Up or Down.\n\r", ch );
	return;
    }

    switch (direction)
    {
	default:        strcpy(arg,"north"); break;
	case DIR_NORTH: strcpy(arg,"north"); break;
	case DIR_SOUTH: strcpy(arg,"south"); break;
	case DIR_EAST:  strcpy(arg,"east");  break;
	case DIR_WEST:  strcpy(arg,"west");  break;
	case DIR_UP:    strcpy(arg,"up");    break;
	case DIR_DOWN:  strcpy(arg,"down");  break;
    }

    if ( ( location = ch->in_room ) == NULL )
    {
	send_to_char("Nothing happens.\n\r",ch);
	return;
    }

    if ( ( pexit = location->exit[direction] ) == NULL )
    {
	send_to_char("Nothing happens.\n\r",ch);
	return;
    }

    if ( ( to_room = pexit->to_room ) == NULL )
    {
	send_to_char("Nothing happens.\n\r",ch);
	return;
    }

    if (IS_SET(pexit->exit_info, EX_WALL_WATER) || 
	IS_SET(pexit->exit_info, EX_WALL_GRANITE) || 
	IS_SET(pexit->exit_info, EX_WALL_WIND) || 
	IS_SET(pexit->exit_info, EX_WALL_FLAME))
    {
	send_to_char("There is already a wall blocking that exit.\n\r",ch);
	return;
    }

    act( "A look of concentration crosses $n's face.", ch, NULL, NULL, TO_ROOM );

    obj = create_object( get_obj_index( OBJ_VNUM_PORTAL ), 0 );
    obj->item_type = ITEM_WALL;
    obj->value[0] = direction;
    obj->timer = duration;
    free_string(obj->name);
    obj->name = str_dup("wall stone granite");
    free_string(obj->short_descr);
    obj->short_descr = str_dup("a wall of granite");
    sprintf(buf,"A wall of solid granite blocks your exit %s.",arg);
    free_string(obj->description);
    obj->description = str_dup(buf);
    obj_to_room( obj, ch->in_room );

    SET_BIT(pexit->exit_info, EX_WALL_GRANITE);

    if (direction == DIR_UP)
	sprintf(buf,"Stone erupts from the ground and forms a wall above you.");
    else if (direction == DIR_DOWN)
	sprintf(buf,"Stone erupts from the ground and forms a wall below you.");
    else
	sprintf(buf,"Stone erupts from the ground and forms a wall to the %s.",arg);
    act( buf, ch, NULL, NULL, TO_CHAR );
    act( buf, ch, NULL, NULL, TO_ROOM );
    WAIT_STATE(ch,12);

    if (location == to_room) return;

    if ( ( pexit = to_room->exit[rev] ) == NULL ) return;
    if (pexit->to_room != location) return;
    if (IS_SET(pexit->exit_info, EX_WALL_WATER) || 
	IS_SET(pexit->exit_info, EX_WALL_GRANITE) || 
	IS_SET(pexit->exit_info, EX_WALL_WIND) || 
	IS_SET(pexit->exit_info, EX_WALL_FLAME)) return;

    char_from_room(ch);
    char_to_room(ch, to_room);

    switch (rev)
    {
	default:        strcpy(arg,"north"); break;
	case DIR_NORTH: strcpy(arg,"north"); break;
	case DIR_SOUTH: strcpy(arg,"south"); break;
	case DIR_EAST:  strcpy(arg,"east");  break;
	case DIR_WEST:  strcpy(arg,"west");  break;
	case DIR_UP:    strcpy(arg,"up");    break;
	case DIR_DOWN:  strcpy(arg,"down");  break;
    }

    obj = create_object( get_obj_index( OBJ_VNUM_PORTAL ), 0 );
    obj->item_type = ITEM_WALL;
    obj->value[0] = rev;
    obj->timer = duration;
    free_string(obj->name);
    obj->name = str_dup("wall stone granite");
    free_string(obj->short_descr);
    obj->short_descr = str_dup("a wall of granite");
    sprintf(buf,"A wall of solid granite blocks your exit %s.",arg);
    free_string(obj->description);
    obj->description = str_dup(buf);
    obj_to_room( obj, ch->in_room );

    SET_BIT(pexit->exit_info, EX_WALL_GRANITE);

    if (rev == DIR_UP)
	sprintf(buf,"Stone erupts from the ground and forms a wall above you.");
    else if (rev == DIR_DOWN)
	sprintf(buf,"Stone erupts from the ground and forms a wall below you.");
    else
	sprintf(buf,"Stone erupts from the ground and forms a wall to the %s.",arg);
    act( buf, ch, NULL, NULL, TO_ROOM );

    char_from_room(ch);
    char_to_room(ch, location);

    return;
}

void do_geas( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    AFFECT_DATA af;
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_WEREWOLF(ch))
    {
	stc("Huh?\n\r",ch);
	return;
    }

    if (get_auspice(ch,AUSPICE_PHILODOX) < 5)
    {
	stc("You require Philodox 5 to use Geas.\n\r",ch);
	return;
    }

    if ((victim = get_char_room(ch,arg)) == NULL)
    {
	stc("They aren't here.\n\r",ch);
	return;
    }

    if (!IS_NPC(victim))
    {
	stc("Only works on NPCs.\n\r",ch);
	return;
    }

    if (ch == victim)
    {
	stc("You force you to obey you.\n\r",ch);
	return;
    }

    if (victim->level > 750)
    {
	stc("They are too strong for you to control.\n\r",ch);
	return;
    }

    if ( victim->master )
        stop_follower( victim );
    add_follower( victim, ch );
    af.type      = gsn_charm_person;
    af.duration  = 25;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char( victim, &af );
    act( "Isn't $n just so nice?", ch, NULL, victim, TO_VICT );
    act( "You lay a Geas on $N.",ch,NULL,victim,TO_CHAR);
    return;
}

void do_wyld( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    CHAR_DATA *vch;
    CHAR_DATA *victim;
    char buf [MAX_STRING_LENGTH];
    
    if (IS_NPC(ch)) return;

    if (!IS_WEREWOLF(ch) && ch->in_room == NULL)
    {
	send_to_char("Huh?\n\r",ch);
	return;
    }

    if (get_auspice(ch,AUSPICE_GALLIARD) < 1)
    {
	send_to_char("You require the level 1 Galliard power to use the Call of the Wyld.\n\r",ch);
	return;
    }

    if ( ( victim = ch->fighting ) == NULL && ch->hit > 0 )
    {
	send_to_char("You can only call for assistance in times of great need.\n\r",ch);
	return;
    }

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected != CON_PLAYING ) continue;
	if ( (vch = d->character) == NULL ) continue;
	if ( vch->in_room == NULL) continue;
	if ( IS_NPC(vch) ) continue;
	if ( vch == ch )
	    act("You call for assistance!", ch, NULL, argument, TO_CHAR);
	if (ch != vch && (IS_WEREWOLF(vch) || IS_BUI(vch)))
	{
	    if (victim != NULL)
	    {
		sprintf(buf,"CALL OF THE WYLD:[$n is under attack by %s at %s]", 
		    IS_NPC(victim) ? victim->short_descr : victim->name,
                    ch->in_room->name);
		act(buf,ch,NULL,vch, TO_VICT);
	    }
	    else if (ch->hit < 1)
	    {
		sprintf(buf,"CALL OF THE WYLD:[$n is badly wounded at %s]", 
                    ch->in_room->name);
		act(buf,ch,NULL,vch, TO_VICT);
	    }
	    else
	    {
		sprintf(buf,"CALL OF THE WYLD:[$n needs assistance at %s]", 
                    ch->in_room->name);
		act(buf,ch,NULL,vch, TO_VICT);
	    }
	}
    }
    return;
}

void do_wyrmcall( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *ich;

    if (!IS_WEREWOLF(ch))
    {
	stc("Huh?\n\r",ch);
	return;
    }

    if (get_auspice(ch, AUSPICE_GALLIARD) < 3)
    {
	stc("You require Galliard 3 to use Call of the Wyrm.\n\r",ch);
	return;
    }

    stc("You start to howl.\n\r",ch);
    act("$n starts to howl.",ch,NULL,NULL,TO_ROOM);

    for (ich = ch->in_room->people; ich != NULL; ich = ich->next_in_room)
    {
	if (IS_NPC(ich)) continue;
        if (IS_VAMPIRE(ich) && ich->beast == 0) continue;
	if (!IS_VAMPIRE(ich) && !IS_DROW(ich)) continue;
    
	affect_strip ( ich, gsn_invis                   );
	affect_strip ( ich, gsn_mass_invis              );
	affect_strip ( ich, gsn_sneak                   );
	REMOVE_BIT   ( ich->affected_by, AFF_HIDE       );
	REMOVE_BIT   ( ich->affected_by, AFF_INVISIBLE  );
	REMOVE_BIT   ( ich->affected_by, AFF_SNEAK      );
	act("$n is baited into the open!",ich,NULL,NULL,TO_ROOM);
	stc( "You find yourself stepping into the open.\n\r",ich);
	if (IS_VAMPIRE(ich))
	{
	    if (IS_VAMPAFF(ich,VAM_OBFUSCATE))	REMOVE_BIT(ich->vampaff, VAM_OBFUSCATE);
	    if (IS_ITEMAFF(ich,ITEMA_PEACE))	do_majesty(ich, "");
	    if (IS_POLYAFF(ich,POLY_SHADOW))	do_shadowbody(ich, "");
	    if (IS_POLYAFF(ich,POLY_MIST))	do_change(ich, "human");
	}
	if (IS_DROW(ich))
	{
	    if (get_drow(ich, DROW_GLOBE)) REMOVE_BIT(ich->pcdata->drow[DROW_POWERS], DROW_GLOBE);
	}
    }
    WAIT_STATE(ch,8);
    return;
}

void do_songrage( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_WEREWOLF(ch))
    {
	stc("Huh?\n\r",ch);
	return;
    }

    if (get_auspice(ch,AUSPICE_GALLIARD) < 4)
    {
	stc("You require Galliard 4 to use Song of Rage.\n\r",ch);
	return;
    }

    if (arg[0] == '\0')
    {
	stc("Who do you wish to sing into a rage?\n\r",ch);
	return;
    }

    if ((victim = get_char_room(ch,arg)) == NULL)
    {
	stc("They aren't here.\n\r",ch);
	return;
    }

    if (is_safe(victim,ch)) return;

    stc("You begin to sing the Song of Rage.\n\r",ch);
    act("$n begins to sing a haunting melody.",ch,NULL,NULL,TO_ROOM);
    if (!IS_NPC(victim))
    {
	if (IS_WEREWOLF(victim) && !IS_SET(victim->act, PLR_WOLFMAN))
	{
	    victim->pcdata->wolf = number_range(150,200);
	    do_werewolf(victim,"");
	}
	else if (IS_VAMPIRE(victim))
	{
	    if (victim->beast > 0) 
	    {
	    do_beastlike(victim,"");
	    stc("You bare your fangs and growl as your inner beast consumes you.\n\r",victim);
	    act("$n bares $s fangs and growls as $s inner beast consumes $m.",victim,NULL,NULL,TO_ROOM);
	    victim->pcdata->wolf += number_range(10,20);
	    }
	    if(victim->beast == 0)
		return;
	}
	else do_berserk(victim,"");
    }
    else do_berserk(victim,"");
    WAIT_STATE(ch,30);
    return;
}

void do_bridgewalk (CHAR_DATA *ch, char *argument)
{
    char arg [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *pRoomIndex;
    long to_room;
     
    argument = one_argument( argument, arg );
        
    if (IS_NPC(ch)) return;
    
    if (!IS_WEREWOLF(ch))
    {
        stc( "Huh?\n\r", ch );
        return;
    }
    if (get_auspice(ch, AUSPICE_GALLIARD) < 5)
    {
        stc("You require Ragabash 5 to use Bridge Walker.\n\r",ch);
        return;
    }
    if (arg[0] == '\0')
    {
        stc( "Who do you wish to Bridge Walk to?\n\r", ch );
        return;
    }
    if (IS_ITEMAFF(ch,ITEMA_BLISSFUL))
	do_blissful(ch,"");

    if ( ( victim = get_char_world( ch, arg ) ) == NULL || victim->in_room == NULL || 
     victim->level > 1000 || number_range(1,3) == 1 || victim->in_room == ch->in_room ||
     IS_SET(victim->in_room->room_flags, ROOM_SAFE) || 
     IS_SET(victim->in_room->room_flags, ROOM_PRIVATE) || 
     IS_SET(victim->in_room->room_flags, ROOM_SOLITARY) || 
     IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL) || (!IS_NPC(victim) && 
     !IS_EXTRA(victim, EXTRA_SUMMON) ))
    {
	stc( "A random moon fluctuation disrupts your bridge!\n\r", ch );
	for ( ; ; )
	{
	    to_room = number_range( 100, 16000 );
	    pRoomIndex = get_room_index( to_room );
	    if ( pRoomIndex != NULL )
	    if ( !IS_SET(pRoomIndex->room_flags, ROOM_PRIVATE)
	     &&   !IS_SET(pRoomIndex->room_flags, ROOM_SOLITARY)
	     &&   !IS_SET(pRoomIndex->room_flags, ROOM_NO_TELEPORT))
		break;
	}

	act( "$n slowly fades out of existence.", ch, NULL, NULL, TO_ROOM );
	char_from_room( ch );
	char_to_room( ch, pRoomIndex );
	act( "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM );
	do_look( ch, "auto" );
	WAIT_STATE(ch,8);
	return;
    }

    stc("You create a moon bridge.\n\r", ch);
    act("$n steps into a shaft of moonlight.", ch, NULL, NULL, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, victim->in_room);
    do_look(ch, "");
    act("$n appears from a stray moonbeam.", ch, NULL, victim, TO_ROOM);
    return;
}

void do_helios( CHAR_DATA *ch, char *argument )
{
    if (IS_NPC(ch)) return;
    
    if (!IS_WEREWOLF(ch))
    {
	stc("Huh?\n\r",ch);
	return;
    }
         
    if (get_auspice(ch,AUSPICE_AHROUN) < 5)
    {
	stc("You require Ahroun 5 to use Kiss of Helios.\n\r",ch);
	return;
    }

    if (IS_MORE(ch,MORE_HELIOS) )  
    {
	stc("The flames around your hands are extinguished.\n\r",ch);
	act("The flames around $n's hands die away.", ch, NULL, NULL, TO_ROOM);
	REMOVE_BIT(ch->more, MORE_HELIOS);
	if ( ch->in_room != NULL && ch->in_room->light > 0 )
	    --ch->in_room->light;
	return;
    }
    else
    {
	stc("Your hands erupts into flames.\n\r",ch);
	act("$n's hands erupts into flames.",ch,NULL,NULL,TO_ROOM);
    }
    SET_BIT(ch->more, MORE_HELIOS);
    if ( ch->in_room != NULL ) ++ch->in_room->light;
    return;
}
