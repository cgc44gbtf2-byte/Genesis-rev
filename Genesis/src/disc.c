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


void do_earthshift( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MIL];

    argument = one_argument( argument, arg );

    if (!IS_VAMPIRE(ch))
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    if (arg[0] == '\0')
    {
        send_to_char( "Who do you wish to travel to?\n\r", ch );
        return;
    }


    if (get_disc(ch,DISC_PROTEAN) < 6)
    {
        send_to_char("You require level 6 Protean to use Earth Control.\n\r",ch);
        return;
    }
    if (!IS_VAMPAFF(ch,VAM_EARTHMELDED))
    {
	stc("You must be melded with the earth to use this power.\n\r",ch);
	return;
    }
    if ( ( victim = get_char_world( ch, arg ) ) == NULL
    ||   victim == ch
    ||   victim->in_room == NULL
    ||   ch->in_room == NULL
    ||   (IS_NPC(victim) && victim->pIndexData->vnum == MOB_VNUM_EYE)
    ||   victim->level > (get_disc(ch,DISC_PROTEAN) * 200))
    {
        send_to_char( "Nothing happens.\n\r", ch );
        return;
    }

    if ( victim->in_room == ch->in_room )
    {
        send_to_char( "But you are already there!\n\r", ch );
        return;
    }

    if ( IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||   victim->position != POS_STANDING
    ||   IS_SET(victim->extra, EXTRA_ILLUSION)
    ||   (!IS_NPC(victim) && !IS_EXTRA(victim, EXTRA_SUMMON) ) )
    {
        send_to_char( "You are unable to focus on their location.\n\r", ch );
        if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
        {
            guardian_message( victim );
            act("$n just failed to use Earth Control on you.",ch,NULL,victim,TO_VICT);
        }
        return;
    }

    if (victim->wizard == ch)
    {  send_to_char("You can't travel to that!", ch);
       return;
    }
    send_to_char("You use your powers of earth control to swiftly move under the ground.\n\r",ch);
    char_from_room(ch);
    char_to_room(ch,victim->in_room);
    return;
}


void do_disint( CHAR_DATA *ch, char *argument )
{
    char arg [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch))
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    if (get_disc(ch,DISC_VICISSITUDE) < 7)
    {
        send_to_char("You require level 7 Vicissitude to use Disintegration.\n\r",ch);
        return;
    }

    if (IS_ITEMAFF(ch, ITEMA_PEACE)) do_majesty(ch,"");

    if (arg[0] == '\0' && ch->fighting == NULL)
    {
        send_to_char("Who do you wish to use Disintegration on?\n\r",ch);
        return;
    }

    if ( ( victim = ch->fighting ) == NULL )
    {
        if ( ( victim = get_char_room( ch, arg ) ) == NULL )
        {
            send_to_char( "They aren't here.\n\r", ch );
            return;
        }
    }

    if (victim == ch)
	return;
    if (ch->blood[BLOOD_CURRENT] < ch->blood[BLOOD_POOL] /30)
    {
	send_to_char( "You do not have enough blood.\n\r", ch);
	return;
    }

    if (is_safe(ch,victim))
    {
        if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
        {
            guardian_message( victim );
            act("$n just failed to use Disintegration on you.",ch,NULL,victim,TO_VICT);
        }
        return;
    }

    WAIT_STATE(ch, 12);
    ch->blood[BLOOD_CURRENT] -= ch->blood[BLOOD_POOL] / 30;

    stc("Your body begins to disintegrate!\n\r", victim);
    act("$n's body begins to disintegrate!", victim, NULL, NULL, TO_ROOM);

    one_hit(ch, victim, TYPE_HIT, "disintegrate");
    SET_BIT(victim->more, MORE_DISINT);

    return;
}

void do_beastform( CHAR_DATA *ch, char *argument )
{
    char arg [MAX_INPUT_LENGTH];
    char buf [MAX_STRING_LENGTH];
    argument = one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch))
    {
        send_to_char("Huh?\n\r",ch);
        return;
    }

    if (get_disc(ch,DISC_PROTEAN) < 7)
    {
        send_to_char("You require level 7 Protean to use Beast Form.\n\r",ch);
        return;
    }

    if (!IS_POLYAFF(ch, POLY_ZULO) && !IS_POLYAFF(ch, POLY_BAT))
    {
        if (IS_AFFECTED(ch, AFF_POLYMORPH))
        {
            send_to_char( "You can only polymorph from human form.\n\r", ch );
            return;
        }
        if (ch->stance[0] != -1) do_stance(ch,"");
        SET_BIT(ch->polyaff, POLY_BAT);
        act( "You transform into werebat form.", ch, NULL, NULL, TO_CHAR );
        act( "$n transforms into a werebat.", ch, NULL, NULL, TO_ROOM );
        SET_BIT(ch->vampaff, VAM_CLAWS);
        SET_BIT(ch->vampaff, VAM_NIGHTSIGHT);
        SET_BIT(ch->affected_by, AFF_POLYMORPH);
        SET_BIT(ch->vampaff, VAM_DISGUISED);
        SET_BIT(ch->polyaff, POLY_ZULO);
        SET_BIT(ch->vampaff, VAM_FLYING);
        sprintf(buf, "#0A huge black werebat#n");
        free_string( ch->morph );
        ch->morph = str_dup( buf );
	update_style(ch);
        return;
    }
    else
    {
      /*  if (!IS_AFFECTED(ch, AFF_POLYMORPH))
        {
            send_to_char( "You are already in human form.\n\r", ch );
            return;
        }*/
        act( "You transform into human form.", ch, NULL, NULL, TO_CHAR );
        act( "$n transforms into human form.", ch, NULL, NULL, TO_ROOM );
            REMOVE_BIT(ch->vampaff, VAM_FLYING);
            REMOVE_BIT(ch->polyaff, POLY_BAT);
      REMOVE_BIT(ch->affected_by, AFF_POLYMORPH);
      REMOVE_BIT(ch->vampaff, VAM_DISGUISED);
      REMOVE_BIT(ch->vampaff, VAM_CLAWS);
        REMOVE_BIT(ch->vampaff, VAM_NIGHTSIGHT);
        REMOVE_BIT(ch->polyaff, POLY_ZULO);
        free_string( ch->morph );
        ch->morph = str_dup( "" );
	update_style(ch);
        return;
    }
    return;
}


void do_shadowforge( CHAR_DATA *ch, char *argument )
{
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    long vnum = 29639;
    long wt;

    if (!IS_VAMPIRE(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }
    if (get_disc(ch,DISC_OBTENEBRATION) < 6)
    {
	stc("You require level 6 Obtenebration to use Shadow Forge.\n\r", ch);
	return;
    }

    if (ch->blood[BLOOD_CURRENT] < 50)
    {
	stc("It costs 50 points of blood to create shadow weapons.\n\r", ch);
	return;
    }

    if      (!str_cmp(argument, "slash"))	wt = TYPE_SLASH;
    else if (!str_cmp(argument, "pierce"))	wt = TYPE_PIERCE;
    else if (!str_cmp(argument, "blunt"))	wt = TYPE_BLUNT;
    else
    {
	stc("Available weapon types are Slash, Pierce, and Blunt.\n\r", ch);
	return;
    }

    if ((pObjIndex = get_obj_index(vnum)) == NULL)
    {
	stc("Missing object, please inform the staff.\n\r", ch);
	return;
    }
    obj = create_object(pObjIndex, 50);
    free_string(obj->short_descr);
    free_string(obj->name);
    if (wt == TYPE_SLASH)
    {
	obj->short_descr = str_dup("#0Shadow Blade#n");
	obj->name = str_dup("shadowforge blade vampire relic");
    }
    else if (wt == TYPE_PIERCE)
    {
	obj->short_descr = str_dup("#0Ebony Knife#n");
	obj->name = str_dup("shadowforge knife vampire relic");
    }
    else if (wt == TYPE_BLUNT)
    {
	obj->short_descr = str_dup("#0Tenebrous Mace#n");
	obj->name = str_dup("shadowforge mace vampire relic");
    }
    obj->value[3] = wt;
    obj->timer = 2700;
    SET_BIT(obj->quest, QUEST_AGGRAVATED);
    obj_to_char(obj, ch);
    ch->blood[BLOOD_CURRENT] -= 50;

    free_string(obj->questmaker);
    obj->questmaker = str_dup(ch->name);
    free_string(obj->questowner);
    obj->questowner = str_dup(ch->name);


    act("The shadows merge and create $p in your hands.", ch, obj, NULL, TO_CHAR);
    act("The shadows merge and create $p in $n's hands.", ch, obj, NULL, TO_ROOM);
    return;
}


void do_fleshbond( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    char arg[MAX_INPUT_LENGTH];
    int mob_vnum;

    one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch))
    {
	send_to_char("Huh?\n\r",ch);
	return;
    }

    if (get_disc(ch, DISC_ANIMALISM) < 7)
    {
	send_to_char("You require the level 7 Animalism to Flesh Bond an animal to yourself.\n\r",ch);
	return;
    }

    mob_vnum = ch->pcdata->absorb[ABS_MOB_VNUM];

    if (mob_vnum > 0)
    {
	if ( ( victim = create_mobile( get_mob_index( mob_vnum ) ) ) == NULL )
	{
	    send_to_char("Bug - Please Inform KaVir.\n\r",ch);
	    return;
	}
	if ( ( ( ch->hit - ch->pcdata->absorb[ABS_MOB_HP] ) < 1 ) || 
	    ( ( ch->max_hit - ch->pcdata->absorb[ABS_MOB_MAX_HP]) < 1 ) )
	{
	    send_to_char("You would be unable to survive the removal of the Flesh Bonded creature.\n\r",ch);
	    return;
	}
	ch->hit -= ch->pcdata->absorb[ABS_MOB_HP];
	ch->max_hit -= ch->pcdata->absorb[ABS_MOB_MAX_HP];
	victim->hit = ch->pcdata->absorb[ABS_MOB_HP];
	victim->max_hit = ch->pcdata->absorb[ABS_MOB_MAX_HP];
	ch->pcdata->absorb[ABS_MOB_VNUM] = 0;
	ch->pcdata->absorb[ABS_MOB_HP] = 0;
	ch->pcdata->absorb[ABS_MOB_MAX_HP] = 0;
	char_to_room(victim,ch->in_room);
	act("You pull $N out of your body.",ch,NULL,victim,TO_CHAR);
	act("$n pulls $N out of $s body.",ch,NULL,victim,TO_ROOM);
	return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char( "What animal do you wish to Flesh Bond?\n\r", ch );
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

    switch ( victim->pIndexData->vnum )
    {
	default:
	    send_to_char( "You are unable to Flesh Bond with them.\n\r", ch );
	    return;
	case 3062:
	case 3066:
	case 5333:
	case 30006:
	case 30007:
	case 30008:
	case 30009:
	case 30010:
	case 30013:
	    break;
    }

    act("You absorb $N into your body.",ch,NULL,victim,TO_CHAR);
    act("$n absorbs $N into $s body.",ch,NULL,victim,TO_NOTVICT);
    act("$n absorbs you into $s body.",ch,NULL,victim,TO_VICT);

    for ( obj = victim->carrying; obj != NULL; obj = obj_next )
    {
	obj_next = obj->next_content;
	obj_from_char(obj);
	obj_to_room(obj,victim->in_room);
	act("$p falls to the ground.",victim,obj,NULL,TO_ROOM);
    }
    while ( victim->affected )
	affect_remove( victim, victim->affected );

    ch->pcdata->absorb[ABS_MOB_VNUM] = victim->pIndexData->vnum;
    ch->pcdata->absorb[ABS_MOB_HP] = victim->hit;
    ch->pcdata->absorb[ABS_MOB_MAX_HP] = victim->max_hit;
    ch->hit += victim->hit;
    ch->max_hit += victim->max_hit;
    extract_char(victim, TRUE);
    do_save(ch, "");
    return;
}


void do_leechblood( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char      arg [MAX_INPUT_LENGTH];
    int       blood = number_range(10,20);
    int       blooddam = number_range(10,20);
    int	      damage;

    argument = one_argument( argument, arg );

    if (!IS_VAMPIRE(ch))
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }
  

    if (get_disc(ch,DISC_QUIETUS) < 6)
    {
	send_to_char("You require level 6 Quietus to use leech.\n\r",ch);
	return;
    }

    if (IS_ITEMAFF(ch, ITEMA_PEACE)) do_majesty(ch,"");

    if (arg[0] == '\0' && ch->fighting == NULL)
    {
	send_to_char( "Who's Vitae do you wish to steal?\n\r", ch );
	return;
    }
    if ( ( victim = ch->fighting ) == NULL )
    {
	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}
    }

    if (ch == victim)
    {
	send_to_char( "Now that would be completely pointless.\n\r", ch );
	return;
    }

    if (IS_NPC(victim))
	{
	stc("Not on NPCs.\n\r", ch);
	return;
	}
    if (is_safe(ch,victim))
    {
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
	{
	    guardian_message( victim );
	    act("$n just failed to use Leech on you.",ch,NULL,victim,TO_VICT);
	}
	return;
    }
   
    if (!can_see(ch, victim))
    {
       send_to_char("you can't see\n\r",ch);
       return;
    }
    if (IS_VAMPAFF(ch, VAM_OBFUSCATE)) 
 	REMOVE_BIT(ch->vampaff,VAM_OBFUSCATE);

    if ( victim->blood[BLOOD_CURRENT] < blood )
	blood = 0;

    act("You leech some of  $N's blood.",ch,NULL,victim,TO_CHAR);
    act("$n leeches some of your blood.",ch,NULL,victim,TO_VICT);
    act("$n leeches some blood from $N. ",ch,NULL,victim,TO_NOTVICT);


    if (get_disc(victim, DISC_VICISSITUDE) >= 6)
     {
	act("$n screams in agony as aciding blood fills his veins.",ch,NULL,victim,TO_VICT);
	act("$n screams in agony as his blood is acidified.",ch,NULL,victim,TO_NOTVICT);
	act("You scream in agony as your veins are filled with acid.",ch,NULL,victim,TO_CHAR);

     if ( ch->blood[BLOOD_CURRENT] < blood )
	  blooddam=ch->blood[BLOOD_CURRENT];
	damage = 10000;
	ch->hit -= damage;
	if (ch->hit <= 0) ch->hit = 0; 		
	ch->blood[BLOOD_CURRENT] -= blood;	
	victim->blood[BLOOD_CURRENT] -= blood;
	return;
     }	  

    if(blood >= victim->blood[BLOOD_CURRENT])
     victim->blood[BLOOD_CURRENT] = 0;
else
    victim->blood[BLOOD_CURRENT] -= blood;
    ch->blood[BLOOD_CURRENT] += blood;
    if (ch->blood[BLOOD_CURRENT] >= ch->blood[BLOOD_POOL])
    {
        ch->blood[BLOOD_CURRENT] = ch->blood[BLOOD_POOL];
        if (IS_VAMPIRE(ch)) send_to_char("Your blood lust is sated.\n\r",ch); 
   }
    if (IS_NPC(victim) || (victim))
    {
	if (victim->blood[0] > 0 && victim->blood[0] <= 50)
	{
	    act("$n sways weakly from bloodloss.",victim,NULL,NULL,TO_ROOM);
	    send_to_char("You feel weak from bloodloss.\n\r",victim);
	}
	else if (victim->blood[0] < 1)
	{
	    act("$n's body falls limply to the ground.",victim,NULL,NULL,TO_ROOM);
	    killperson(ch,victim);
	    WAIT_STATE( ch, 12 );
	    return;
	}
    }
    
    if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
    {
	guardian_message( victim );
	act("$n just used leech on you.",ch,NULL,victim,TO_VICT);
    }
    if (IS_NPC(victim)) do_kill(victim,ch->name);
    WAIT_STATE( ch, 12 );
    return;
}

void do_fistofcaine( CHAR_DATA *ch, char *argument )
{
    char arg [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch))
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if (get_disc(ch,DISC_POTENCE) < 7)
    {
	send_to_char("You require level 7 Potence to use fist of caine.\n\r",ch);
	return;
    }

    if (IS_ITEMAFF(ch, ITEMA_PEACE)) do_majesty(ch,"");

    if (arg[0] == '\0' && ch->fighting == NULL)
    {
	send_to_char("Who do you wish to use Fist of Caine on?\n\r",ch);
	return;
    }

    if ( ( victim = ch->fighting ) == NULL )
    {
	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}
    }

    if (is_safe(ch,victim))
    {
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
	{
	    guardian_message( victim );
	    act("$n just failed to use Fist of Caine on you.",ch,NULL,victim,TO_VICT);
	}
	return;
    }

    WAIT_STATE( ch, 4 );

    one_hit(ch, victim, TYPE_UNARMED, "Fist of Caine");
    WAIT_STATE(ch, 14);	

    return;
}


void do_eyes( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg [MAX_INPUT_LENGTH];
    argument = one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch))
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if (get_disc(ch, DISC_SERPENTIS) < 1)
    {
	send_to_char( "You require the level 1 serpentis to use eyes of the serpent.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if (ch == victim)
    {
	send_to_char( "I think not...\n\r", ch );
	return;
    }
    if (!IS_AFFECTED(ch, AFF_SHADOWPLANE) && IS_AFFECTED(victim, AFF_SHADOWPLANE) )
    {
        send_to_char("You are in the wrong plane.\n\r", ch);
        return;
    }

    act("You stare deeply into $N's eyes.",ch,NULL,victim,TO_CHAR);
    act("$n stares deeply into $N's eyes.",ch,NULL,victim,TO_NOTVICT);
    act("$n stares deeply into your eyes.",ch,NULL,victim,TO_VICT);

    if (is_safe(ch,victim))
    {
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN)
         && get_breed(ch,BREED_HOMID) < 5)
	{
	    guardian_message( victim );
	    act("$n just failed to use eyes of the serpent on you.",ch,NULL,victim,TO_VICT);
	}
	return;
    }

    if( number_range(1,3) != 1 )
    {
        act("You stare back into $N's eyes.",victim,NULL,ch,TO_CHAR);
        act("$n stares back into $N's eyes.",victim,NULL,ch,TO_NOTVICT);
        act("$n stares back into your eyes.",victim,NULL,ch,TO_VICT);

	send_to_char( "You paralyze them with your stare!\n\r", ch );
	send_to_char( "You are momentarily paralyzed!\n\r", victim );

    	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN) && get_breed(ch,BREED_HOMID) < 5)
    	{
	   guardian_message( victim );
	   act("$n just used eyes of the serpent on you.",ch,NULL,victim,TO_VICT);
	}
	WAIT_STATE( victim, 20 );
    }
    else
    {
	act("Your eyes of the serpent failed on $N.",ch,NULL,victim,TO_CHAR);
	act("$n's eyes of the serpent has failed on $N.",ch,NULL,victim,TO_NOTVICT);
	act("$n's eyes of the serpent failed against you.",ch,NULL,victim,TO_VICT);
    }
    WAIT_STATE( ch, 20 );
    return;
}

void do_mark( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
  
    one_argument( argument, arg );
	if(!IS_VAMPIRE(ch))
	{
	stc("Huh?\n\r",ch);
	return;
	}
    if(get_disc(ch, DISC_SERPENTIS) < 7)
	{
	stc("You require level 7 serpentis to place the mark of corruption.\n\r", ch);
	return;
	}
    if ( arg[0] == '\0' )
    {
	send_to_char( "Use mark of corruption on whom?", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    

    if ( IS_SET(victim->extra, EXTRA_MARK) )
    {
	REMOVE_BIT(victim->extra, EXTRA_MARK);
	send_to_char( "You remove the mark of corruption from them.\n\r", ch );
	send_to_char( "The mark disappears from your forehead.\n\r", victim );
    }

    else
    {
	if(number_range(1, 8) > 2)
	{
	stc("You fail to place the Mark of Corruption.\n\r", ch);
	return;
	}
	stc("You place the Mark of Corruption on their forehead.\n\r", ch);
	act( "$n places the mark of corruption on $N's forehead.", ch, NULL, victim,TO_NOTVICT);
	send_to_char( "The Mark of corruption has been placed on your forhead\n\r", victim );
	SET_BIT(victim->extra, EXTRA_MARK);
	}

    

    return;
}

void do_enrage( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    char arg[MAX_INPUT_LENGTH];
	int chance = number_range(1, 5);
	int number_hit = 0;
    one_argument( argument, arg );

    if ( IS_NPC(ch) )
	return;

    if (!IS_VAMPIRE(ch))
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if (get_disc(ch,DISC_PRESENCE) < 6)
    {
	send_to_char("You require level 6 Presence to use Enrage.\n\r",ch);
	return;
    }

    if ( arg[0] == '\0' ) 
	{
	stc("Use Enrage on whom?\n\r", ch);
	return;
	}
    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if(chance < 3)
	{	
	stc("You are unable to concentrate.\n\r", ch);
        
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
	{
	    guardian_message( victim );
	    act("$n just failed to use Enrage on you.",ch,NULL,victim,TO_VICT);
	}
	return;
    }

    act("You growl loudly and lash out at your surroundings!",victim,NULL,NULL,TO_CHAR);
    act("$n growls loudly and lashes out at their surroundings!",victim,NULL,NULL,TO_ROOM);


    

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
	vch_next	= vch->next;
	if (number_hit > 4) continue;
	if ( vch->in_room == NULL )
	    continue;
	if ( !IS_NPC(vch) && vch->pcdata->chobj != NULL )
	    continue;
	if ( victim == vch )
	    continue;
	if ( vch->in_room == victim->in_room )
	{
	    if (can_see(victim,vch))
	    {
    
		multi_hit( victim, vch);
		number_hit++;
	    }
	}
    }
    do_beastlike(ch,"");
    victim->enrage_timer = 120;
    if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
    {
	guardian_message( victim );
	act("$n just used Enrage on you.",ch,NULL,victim,TO_VICT);
    }
    WAIT_STATE(ch,12);
    return;
}

void do_tenebrous( CHAR_DATA *ch, char *argument )
{
    char arg [MAX_INPUT_LENGTH];
    char buf [MAX_STRING_LENGTH];
    argument = one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (get_disc(ch,DISC_OBTENEBRATION) < 7)
    {
	send_to_char("You require level 7 obtenebration to change into Tenebrous Form.\n \r",ch);
	return;
    }

    if (IS_AFFECTED(ch,AFF_POLYMORPH) && !IS_VAMPAFF(ch,VAM_DISGUISED))
    {
	send_to_char( "Not while polymorphed.\n\r", ch );
	return;
    }

    if (IS_XTRA(ch,XTRA_TENEBROUS))
    {
	REMOVE_BIT(ch->xtra, XTRA_TENEBROUS);

	send_to_char("The shadows slip from your body falling to the ground.\n\r",ch);
	    act("The shadows slip away revealing the form of $n.",ch,NULL,NULL,TO_ROOM);
	return;
    }
    
    else
    {
	send_to_char("Shadows wrap around your body.\n\r",ch);
	act("$n's body is wrapped in shadow's.",ch,NULL,NULL,TO_ROOM);
    }
   
    SET_BIT(ch->xtra, XTRA_TENEBROUS);
        sprintf(buf, "#0A shadowy form#n[#0%s#n]", ch->name);
        free_string( ch->morph );
        ch->morph = str_dup( buf );
	return;
}

void do_horrid( CHAR_DATA *ch, char *argument )
{
    char arg [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int dam;
    one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch))
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if (get_disc(ch,DISC_CHIMERSTRY) < 7)
    {
	send_to_char("You require level 7 Chimerstry to use Horrid Reality.\n\r",ch);
	return;
    }

    if (IS_ITEMAFF(ch, ITEMA_PEACE)) do_majesty(ch,"");

    if (arg[0] == '\0' && ch->fighting == NULL)
    {
	send_to_char("Who do you wish to use Horrid Reality on?\n\r",ch);
	return;
    }

    if ( ( victim = ch->fighting ) == NULL )
    {
	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}
    }

    if (is_safe(ch,victim))
    {
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
	{
	    guardian_message( victim );
	    act("$n just failed to use Horrid Reality on you.",ch,NULL,victim,TO_VICT);
	}
	return;
    }

	dam = 4500;  
        if(IS_NPC(victim)) dam += 2000;

    send_to_char("You scream in agony as wounds appear all over your body!\n\r",victim);
    act("$n screams in agony as wounds appear all over $s body!",victim,NULL,NULL,TO_ROOM);
    if ( dam <= 0 )
	dam = 1;
    victim->hit -= dam;

    if (IS_NPC(victim) && victim->hit < 1)
        {
            act("$n's body falls appart in shreds.", victim, NULL, NULL, TO_ROOM);
            group_gain(ch, victim);
            killperson(ch, victim);
        }
        else
        {
            update_pos(victim);
            if (victim->fighting == NULL) set_fighting(victim,ch);
            victim->position = POS_FIGHTING;
            if (ch->fighting == NULL) set_fighting(ch, victim);
            ch->position = POS_FIGHTING;
        }

    WAIT_STATE(ch, 15);
    return;
}

void do_torment( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char      arg [MAX_INPUT_LENGTH];
    int       damage;

    argument = one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch))
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if (get_disc(ch,DISC_NECROMANCY) < 7)
    {
	send_to_char("You require level 7 Necromancy to use a Torment.\n\r",ch);
	return;
    }

    if (IS_ITEMAFF(ch, ITEMA_PEACE)) do_majesty(ch,"");

    if ( arg[0] == '\0' )
    {
	send_to_char( "Who do you want to Torment?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They are not here.\n\r", ch );
	return;
    }

    SET_BIT(ch->affected_by, AFF_ETHEREAL);  

    if (is_safe(ch,victim) || !IS_AFFECTED(ch, AFF_ETHEREAL))
    {
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
	{
	    guardian_message( victim );
	    act("$n just failed to use Torment on you.",ch,NULL,victim,TO_VICT);
	}
	REMOVE_BIT(ch->affected_by, AFF_ETHEREAL);
	return;
    }
    REMOVE_BIT(victim->affected_by, AFF_ETHEREAL);
    REMOVE_BIT(victim->affected_by, AFF_POLYMORPH);
    REMOVE_BIT(victim->affected_by, AFF_ETHEREAL);
    REMOVE_BIT(victim->polyaff, POLY_SPIRIT);
    REMOVE_BIT(victim->vampaff, VAM_DISGUISED);
    REMOVE_BIT(victim->vampaff, VAM_FLYING);
    REMOVE_BIT(ch->affected_by, AFF_POLYMORPH);
    REMOVE_BIT(ch->affected_by, AFF_ETHEREAL);
    REMOVE_BIT(ch->polyaff, POLY_SPIRIT);
    REMOVE_BIT(ch->vampaff, VAM_DISGUISED);
    REMOVE_BIT(ch->vampaff, VAM_FLYING);
    damage = victim->max_hit * 0.5;
    
    act( "A look of concentration crosses $n's face.", ch, NULL, NULL, TO_ROOM );

    send_to_char("You scream in agony as $N tears at your spirit!\n\r",victim);
    act("$n screams in agony as $s spirit is torn!",victim,NULL,NULL,TO_ROOM);
    victim->hit -= damage;
	
    if ((IS_NPC(victim) && victim->hit < 1) || 
	(!IS_NPC(victim) && victim->hit < -10))
    {
	act("$n's body falls limply to the ground.",victim,NULL,NULL,TO_ROOM);
	group_gain(ch,victim);
	killperson(ch,victim);
	WAIT_STATE( ch, 30 );
	return;
    }
    update_pos(victim);
    do_kill(victim,ch->name);
    WAIT_STATE( ch, 30 );
    return;
}
