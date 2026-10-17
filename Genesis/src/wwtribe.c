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

void do_breathwyld( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int heal;

    if (IS_NPC(ch)) return;

    if (!IS_WEREWOLF(ch))
    {
	stc("Huh?\n\r",ch);
	return;
    }

    if (get_tribe(ch,TRIBE_BLACK_FURIES) < 1)
    {
	stc("You require Black Furies 1 to use Breath of the Wyld.\n\r",ch);
	return;
    }

    stc("You summon forth the Wyld.\n\r",ch);
    act("$n smiles as the room begins to thicken with a magical mist",ch,NULL,NULL,TO_ROOM);

    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
	vch_next = vch->next_in_room;
	if (vch->hit >= vch->max_hit) continue;
	heal = number_range(200,400) * get_tribe(ch,TRIBE_BLACK_FURIES);
	heal += (500 * ch->pcdata->rank);
	act("You instill life in $N.",ch,NULL,vch,TO_CHAR);
	act("$n instills life in $N.",ch,NULL,vch,TO_NOTVICT);
	stc("You feel your wounds close.\n\r",vch);
	vch->hit += heal;
	if (vch->hit > vch->max_hit) vch->hit = vch->max_hit;
    }
    WAIT_STATE(ch,12);
    return;
}

void do_aeolus( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;
    char buf[MAX_INPUT_LENGTH];

    if (IS_NPC(ch)) return;

    if (!IS_WEREWOLF(ch))
    {
	send_to_char("Huh?\n\r",ch);
	return;
    }

    if (get_tribe(ch, TRIBE_BLACK_FURIES) < 2)
    {
	send_to_char("You require the level 2 power of the Black Furies to use the Curse of Aeolus.\n\r",ch);
	return;
    }

    if (ch->in_room == NULL)
    {
	send_to_char("Nothing happens.\n\r",ch);
	return;
    }

    if (ch->pcdata->weather > 0)
    {
	if (ch->pcdata->weather > 1)
	    sprintf(buf,"You cannot call the Curse of Aeolus for another %li seconds.\n\r",ch->pcdata->weather);
	else
	    sprintf(buf,"You cannot call the Curse of Aeolus for another 1 second.\n\r");
	send_to_char(buf,ch);
	return;
    }

    if ( ch->in_room->sector_type == SECT_INSIDE || !IS_OUTSIDE(ch) )
    {
	send_to_char("You can only use this power outside.\n\r",ch);
	return;
    }

    act( "A look of concentration crosses $n's face.", ch, NULL, NULL, TO_ROOM );
    if (IS_SET(world_affects, WORLD_FOG))
    {
	send_to_char("You banish the fog.\n\r",ch);
	REMOVE_BIT(world_affects, WORLD_FOG);
	ch->pcdata->weather = 60;
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected != CON_PLAYING ) continue;
	    if ( (victim = d->character) == NULL ) continue;
	    if ( IS_NPC(victim) ) continue;
	    if ( victim->in_room == NULL ) continue;
	    if ( ch == victim ) continue;
	    if ( victim->in_room->sector_type == SECT_INSIDE ) continue;
	    if ( ch->in_room == victim->in_room )
		act("$n holds up his arms and the fog disperses.", ch, NULL, victim, TO_VICT);
	    else
		send_to_char("The fog disperses.\n\r", victim);
	}
    }
    else
    {
	send_to_char("You summon a thick blanket of fog.\n\r",ch);
	SET_BIT(world_affects, WORLD_FOG);
	ch->pcdata->weather = 60;
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected != CON_PLAYING ) continue;
	    if ( (victim = d->character) == NULL ) continue;
	    if ( IS_NPC(victim) ) continue;
	    if ( victim->in_room == NULL ) continue;
	    if ( ch == victim ) continue;
	    if ( victim->in_room->sector_type == SECT_INSIDE ) continue;
	    if ( ch->in_room == victim->in_room )
		act("$n holds up his arms and calls forth a thick blanket of fog.", ch, NULL, victim, TO_VICT);
	    else
		send_to_char("A thick blanket of fog rises from the ground.\n\r", victim);
	}
    }
    return;
}

void do_bodywrack( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char      arg [MAX_INPUT_LENGTH];
    long damage;

    argument = one_argument( argument, arg );

    if (IS_NPC(ch)) return;
    if (!IS_WEREWOLF(ch))
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if (get_tribe(ch,TRIBE_BLACK_FURIES) < 5)
    {
	send_to_char("You require the level 5 Black Furies power to use Body Wrack.\n\r",ch);
	return;
    }

    if (IS_ITEMAFF(ch, ITEMA_PEACE)) do_majesty(ch,"");

    if ( arg[0] == '\0' )
    {
	send_to_char( "Who do you wish to use Body Wrack on?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They are not here.\n\r", ch );
	return;
    }

    if ( is_safe(ch, victim) )
    {
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN)
         && get_breed(ch,BREED_HOMID) < 5)
	{
	    guardian_message( victim );
	    act("$n just failed to use Body Wrack on you.",ch,NULL,victim,TO_VICT);
	}
	return;
    }

    damage = number_range(500,1500);

    act( "A look of concentration crosses $n's face.", ch, NULL, NULL, TO_ROOM );

    send_to_char("You scream in agony as your body seems to be torn apart!\n\r",victim);
    act("$n doubles over and screams in agony!",victim,NULL,NULL,TO_ROOM);

    if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN)
     && get_breed(ch,BREED_HOMID) < 5)
    {
	guardian_message( victim );
	act("$n just used Body Wrack on you.",ch,NULL,victim,TO_VICT);
    }

    victim->hit -= damage;
    if ((IS_NPC(victim) && victim->hit < 1))
    {
	act("$n's body falls limply to the ground.",victim,NULL,NULL,TO_ROOM);
	group_gain(ch,victim);
	killperson(ch,victim);
	WAIT_STATE( ch, 12 );
	return;
    }
    update_pos(victim);
    if (IS_NPC(victim)) do_kill(victim,ch->name);
    WAIT_STATE( ch, 12 );
    return;
}


void do_friend( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MIL];
    int give = number_range(50,100);
    int rage = 300;

    one_argument(argument, arg);

    if (IS_NPC(ch)) return;

    if (!IS_WEREWOLF(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (get_tribe(ch,TRIBE_BONE_GNAWERS) < 3)
    {
	stc("You require Bone Gnawers 3 to use Friend in Need.\n\r", ch);
	return;
    }

    if (arg[0] == '\0')
    {
	stc("Help whom?\n\r", ch);
	return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL)
    {
	stc("They aren't here.\n\r", ch);
	return;
    }

    if (!IS_WEREWOLF(victim))
    {
	stc("You can't help them.\n\r", ch);
	return;
    }

    if (ch->pcdata->wolf <= 0)
    {
	stc("You have no rage to give.\n\r", ch);
	return;
    }

    if (get_tribe(victim, TRIBE_RED_TALONS) >= 3) rage += 50;

    act("$n smiles at $N.", ch, NULL, victim, TO_NOTVICT);
    act("You give your rage to $N.", ch, NULL, victim, TO_CHAR);
    act("You feel rage pour into you as $n smiles at you.", ch, NULL, victim, TO_VICT);
    if (give > ch->pcdata->wolf) give = ch->pcdata->wolf;
    ch->pcdata->wolf -= give;
    victim->pcdata->wolf += give;
    if (ch->pcdata->wolf <= 0) do_unwerewolf(ch, "");
    if (victim->pcdata->wolf >= rage - 100) do_werewolf(victim, "");
    if (victim->pcdata->wolf >= rage) victim->pcdata->wolf = rage;
    WAIT_STATE(ch, 12);
    return;
}

void do_infest( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    AFFECT_DATA af;
    long sn;
    long loop;

    if (IS_NPC(ch)) return;

    if (!IS_WEREWOLF(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (get_tribe(ch, TRIBE_BONE_GNAWERS) < 4)
    {
	stc("You require Bone Gnawers 4 to use Infest.\n\r", ch);
	return;
    }

    if (ch->pcdata->followers >= 1)
    {
	stc("Nothing happens.\n\r", ch);
	return;
    }

    stc("You summon a horde of vermin from the shadows!\n\r", ch);
//    act("$n summons a horde of vermin from the shadows!", ch, NULL, NULL, TO_ROOM);

    for (loop = 1; loop <= 5; loop++)
    {
	ch->pcdata->followers++;
	victim=create_mobile(get_mob_index(MOB_VNUM_GUARDIAN));
	char_to_room(victim, get_room_index(ROOM_VNUM_DISCONNECTION));
	free_string(victim->lord);
	victim->lord = str_dup(ch->name);
	free_string(victim->name);
	victim->name = str_dup("sewer rat");
	free_string(victim->short_descr);
	victim->short_descr = str_dup("a sewer rat");
	free_string(victim->long_descr);
	victim->long_descr = str_dup("A sewer rat is here.\n\r");
	victim->sex = 1;
	free_string(victim->description);
	victim->description = str_dup("He looks extremely powerful.\n\r");

	victim->level = 100;
	victim->hit = 10000;
	victim->max_hit = 10000;
	victim->hitroll = 100;
	victim->damroll = 100;
	victim->armor = 100;
	char_from_room(victim);
	char_to_room(victim, ch->in_room);

	if ((sn = skill_lookup("guardian")) < 0)
	{
	    stc("Bug - please inform Akuma.\n\r", ch);
	    return;
        }

	add_follower(victim, ch);
	af.type      = sn;
	af.duration  = 6666;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_CHARM;
	affect_to_char(victim, &af);

    }
    return;
}


void do_dazzle( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MIL];

    argument = one_argument(argument, arg);

    if (IS_NPC(ch)) return;

    if (!IS_WEREWOLF(ch))
    {
	send_to_char("Huh?\n\r", ch);
	return;
    }

    if (get_tribe(ch,TRIBE_CHILDREN_OF_GAIA) < 3)
    {
	send_to_char("You require the level 3 Children of Gaia power to use Dazzle.\n\r",ch);
	return;
    }

    if (IS_ITEMAFF(ch, ITEMA_PEACE)) do_majesty(ch,"");

    if ( arg[0] == '\0' )
    {
	send_to_char( "Who do you want to Dazzle?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They are not here.\n\r", ch );
	return;
    }

    if (victim->position != POS_FIGHTING)
    {
	send_to_char( "Nothing happens.\n\r", ch );
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN)
         && get_breed(ch,BREED_HOMID) < 5)
	{
	    guardian_message( victim );
	    act("$n just failed to use Dazzle on you.",ch,NULL,victim,TO_VICT);
	}
	return;
    }

    if (is_safe(ch,victim))
    {
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN)
         && get_breed(ch,BREED_HOMID) < 5)
	{
	    guardian_message( victim );
	    act("$n just failed to use Dazzle on you.",ch,NULL,victim,TO_VICT);
	}
	return;
    }

    act( "A look of concentration crosses $n's face.", ch, NULL, NULL, TO_ROOM );

    send_to_char("You cover your eyes as you are dazzled!\n\r",victim);
    act("$n covers $s eyes, looking dazzled!",victim,NULL,NULL,TO_ROOM);

    if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN)
     && get_breed(ch,BREED_HOMID) < 5)
    {
	guardian_message( victim );
	act("$n just used Dazzle on you.",ch,NULL,victim,TO_VICT);
    }

    stop_fighting(victim,TRUE);
    update_pos(victim);
    if (victim->position > POS_STUNNED)
	victim->position = POS_STUNNED;
    WAIT_STATE( ch, 24 );
    return;
}

void do_halosun( CHAR_DATA *ch, char *argument )
{
    if (IS_NPC(ch)) return;

    if (!IS_WEREWOLF(ch))
    {
	send_to_char("Huh?\n\r",ch);
	return;
    }

    if (get_tribe(ch,TRIBE_CHILDREN_OF_GAIA) < 5)
    {
	send_to_char("You require the level 5 Children of Gaia power to use the Halo of the Sun.\n\r",ch);
	return;
    }

    if (IS_ITEMAFF(ch,ITEMA_SUNSHIELD) )
    {
	send_to_char("Your Halo of the Sun gradually fades away.\n\r",ch);
	act("$n's Halo of the Sun gradually fades away.", ch, NULL, NULL, TO_ROOM);
	REMOVE_BIT(ch->itemaffect, ITEMA_SUNSHIELD);
	return;
    }
    send_to_char("You are surrounded by a halo of sunlight!\n\r",ch);
    act("$n is surrounded by a halo of sunlight!", ch, NULL, NULL, TO_ROOM);
    SET_BIT(ch->itemaffect, ITEMA_SUNSHIELD);
    return;
}

void do_faerielight( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *light;

    if (IS_NPC(ch)) return;

    if (!IS_WEREWOLF(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (get_tribe(ch, TRIBE_FIANNA) < 1)
    {
	stc("You require Fianna 1 to use Faerie Light.\n\r", ch);
	return;
    }

    light = create_object( get_obj_index( OBJ_VNUM_LIGHT_BALL ), 0 );
    obj_to_room( light, ch->in_room );
    act( "$n summons the POWER OF GAIA to create a ball of light.", ch, NULL, NULL, TO_ROOM );
    stc( "You summon the POWER OF GAIA to create a ball of light.\n\r",ch);
    return;
}

void do_banshee( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *ich;
    CHAR_DATA *fch;
    bool follow = FALSE;

    if (IS_NPC(ch)) return;

    if (!IS_WEREWOLF(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (get_tribe(ch, TRIBE_FIANNA) < 2)
    {
	stc("You require Fianna 2 to use Howl of the Banshee.\n\r", ch);
	return;
    }

    if (IS_SET(ch->in_room->added_flags, ROOM2_SILENCE)) 
    {
	stc("You attempt to howl, but your voice fades into the silence.\n\r", ch);
	return;
    }

    stc("You begin the Howl of the Banshee.\n\r",ch);
    act("$n begins to howl a painfully sad call.",ch,NULL,NULL,TO_ROOM);

    if (IS_SET(ch->act, PLR_AUTOFOLLOW))
    {
	REMOVE_BIT(ch->act, PLR_AUTOFOLLOW);
	follow = TRUE;
    }

    for (ich = ch->in_room->people; ich != NULL; ich = fch)
    {
	fch = ich->next_in_room;
	if (ich == ch) continue;
	if (!IS_NPC(ich) && ich->pcdata->resist[WILL_PRESENCE] > 0) continue;

	WAIT_STATE(ich, get_tribe(ch, TRIBE_FIANNA) * 4);
	if (!IS_NPC(ich)) ich->pcdata->resist[WILL_PRESENCE] = 70 - (get_tribe(ch, TRIBE_FIANNA) * 10);
	stc("You are frozen in fear!\n\r", ich);
	act("$n screams in terror!", ich, NULL, NULL, TO_ROOM);
	if (ich->position == POS_FIGHTING) do_flee(ich, "");
	if (ich->position == POS_FIGHTING && get_tribe(ch, TRIBE_FIANNA) >= 5) do_flee(ich, "");
    }

    if (follow) SET_BIT(ch->act, PLR_AUTOFOLLOW);
    WAIT_STATE(ch, 18);
    return;
}

void do_balor( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MSL];

    one_argument(argument, arg);

    if (!IS_WEREWOLF(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (get_tribe(ch, TRIBE_FIANNA) < 5)
    {
	stc("You require Fianna 5 to use Balor's Gaze.\n\r", ch);
	return;
    }

    if (arg[0] == '\0')
    {
	stc("Gaze at whom?\n\r", ch);
	return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL)
    {
	stc("They are not here.\n\r", ch);
	return;
    }

    if (is_safe(ch, victim)) return;

    if (IS_NPC(victim))
    {
	stc("Not on NPCs.\n\r", ch);
	return;
    }

    if (victim->pcdata->pain >= 30)
    {
	act("$E is already in immense pain.\n\r", ch, NULL, victim, TO_CHAR);
	return;
    }

    act("You glare at $N.", ch, NULL, victim, TO_CHAR);
    act("$n glares at $N.", ch, NULL, victim, TO_NOTVICT);
    act("Pain rushes into your body under $n's glare.", ch, NULL, victim, TO_VICT);

    victim->pcdata->pain += 30;
    return;
}

void do_diagnostics( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    char arg[MAX_INPUT_LENGTH];

    one_argument(argument,arg);

    if (IS_NPC(ch)) return;
    if (!IS_WEREWOLF(ch))
    {
	stc("Huh?\n\r",ch);
	return;
    }
    if (get_tribe(ch,TRIBE_GLASS_WALKERS) < 1)
    {
	stc("You require Glass Walkers 1 to use diagnostic.\n\r",ch);
	return;
    }
    if (arg[0] == '\0')
    {
	stc("Diagnostic what?",ch);
	return;
    }
    if ((obj = get_obj_carry(ch,arg)) == NULL)
    {
	stc("You don't have that.\n\r",ch);
	return;
    }

    spell_identify(skill_lookup("identify"),50,ch,obj);
    return;
}

void do_surge( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;

    if (IS_NPC(ch)) return;

    if (!IS_WEREWOLF(ch))
    {
	stc("Huh?\n\r",ch);
	return;
    }

    if (get_tribe(ch,TRIBE_GLASS_WALKERS) < 3)
    {
	stc("You require Glass Walkers 3 to use Power Surge.\n\r",ch);
	return;
    }

    stc("You summon forth electricity.\n\r", ch);
    act("$n turns $s palms up and smirks as the air crackles with electricity.", ch, NULL, NULL, TO_ROOM);

    for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
    {
	vch_next = vch->next_in_room;
	if (vch == ch) continue;
	if (is_same_group(ch, vch)) continue;
	if (is_safe(ch, vch)) continue;
	if (!CAN_PK(vch)) continue;
	if (IS_BUI(vch)) continue;
	one_hit(ch, vch, TYPE_HIT, "surge");
    }

    WAIT_STATE(ch, 12);
    return;
}

void do_doppleganger( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char      arg [MAX_INPUT_LENGTH];
    char      buf [MAX_STRING_LENGTH];

    argument = one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_WEREWOLF(ch))
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if (get_tribe(ch,TRIBE_GLASS_WALKERS) < 4)
    {
	stc("You require Glass Walkers 4 to use Doppleganger.\n\r",ch);
	return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char( "Change to look like whom?\n\r", ch );
	return;
    }

    if (IS_AFFECTED(ch,AFF_POLYMORPH) && !IS_VAMPAFF(ch,VAM_DISGUISED))
    {
	send_to_char( "Not while polymorphed.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch == victim )
    {
	if (!IS_AFFECTED(ch,AFF_POLYMORPH) && !IS_VAMPAFF(ch,VAM_DISGUISED))
	{
	    send_to_char( "You already look like yourself!\n\r", ch );
	    return;
	}
	if (!IS_POLYAFF(ch, POLY_MASK))
	{
	    send_to_char("Nothing happens.\n\r",ch);
	    return;
	}
	sprintf(buf,"$n's form shimmers and transforms into a clone of %s.",ch->name);
	act(buf,ch,NULL,NULL,TO_ROOM);
	REMOVE_BIT(ch->affected_by, AFF_POLYMORPH);
	REMOVE_BIT(ch->vampaff, VAM_DISGUISED);
	REMOVE_BIT(ch->polyaff, POLY_MASK);
    	free_string( ch->morph );
    	ch->morph = str_dup( "" );
	act("Your form shimmers and transforms into a clone of $n.",ch,NULL,NULL,TO_CHAR);
	return;
    }

    if (IS_BUI(victim))
    {
	send_to_char( "You can only mask avatars or lower.\n\r", ch );
	return;
    }

    if (IS_VAMPAFF(ch,VAM_DISGUISED))
    {
	if (!IS_POLYAFF(ch, POLY_MASK))
	{
	    send_to_char("Nothing happens.\n\r",ch);
	    return;
	}
	act("Your form shimmers and transforms into a clone of $N.",ch,NULL,victim,TO_CHAR);
	act("$n's form shimmers and transforms into a clone of you.",ch,NULL,victim,TO_VICT);
	act("$n's form shimmers and transforms into a clone of $N.",ch,NULL,victim,TO_NOTVICT);
    	free_string( ch->morph );
    	ch->morph = str_dup( victim->name );
	SET_BIT(ch->polyaff, POLY_MASK);
	return;
    }
    act("Your form shimmers and transforms into a clone of $N.",ch,NULL,victim,TO_CHAR);
    act("$n's form shimmers and transforms into a clone of you.",ch,NULL,victim,TO_VICT);
    act("$n's form shimmers and transforms into a clone of $N.",ch,NULL,victim,TO_NOTVICT);
    SET_BIT(ch->affected_by, AFF_POLYMORPH);
    SET_BIT(ch->vampaff, VAM_DISGUISED);
    SET_BIT(ch->polyaff, POLY_MASK);
    free_string( ch->morph );
    ch->morph = str_dup( victim->name );
    return;
}

void do_summonnet( CHAR_DATA *ch, char *argument )
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

    if (get_tribe(ch,TRIBE_GLASS_WALKERS) < 5)
    {
        stc("You need a Glass Walkers of 5.\n\r",ch);
        return;
    }
    
    if (ch->pcdata->followers >= 1)
    {
        stc("Nothing happens.\n\r",ch);
        return;
    }
 
    ch->pcdata->followers++;
    victim=create_mobile( get_mob_index( MOB_VNUM_GUARDIAN ) );
    char_to_room(victim,get_room_index(ROOM_VNUM_DISCONNECTION));
    free_string(victim->lord);
    victim->lord = str_dup(ch->name);
    free_string(victim->name);
    victim->name = str_dup("net spider");
    free_string(victim->short_descr);
    victim->short_descr = str_dup("a Net-Spider");
    free_string(victim->long_descr);
    victim->long_descr = str_dup("A Net-Spider is here, awaiting orders.\n\r");
    victim->sex = 1;
    free_string(victim->description);
    victim->description = str_dup("He looks extremely powerful.\n\r");

    victim->level = 100;
    victim->hit = 30000;
    victim->max_hit = 30000;
    victim->mana = 1000;
    victim->max_mana = 1000;
    victim->hitroll = char_hitroll(ch) - 50;
    victim->damroll = char_damroll(ch) - 50;
    victim->armor = char_ac(ch) - 50;
    char_from_room(victim);
    char_to_room(victim,ch->in_room);

    stc("You call forth a Net-Spider, ready to aid you.\n\r",ch);
    act("$n concentrates, causing $N to come out of the Weaver's web.", ch, NULL, victim, TO_ROOM);

    if ( ( sn = skill_lookup( "guardian" ) ) < 0 )
    {
        stc( "Bug - please inform Akuma.\n\r", ch );
        return;
    }

    add_follower(victim, ch );
    af.type      = sn;
    af.duration  = 10101;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char( victim, &af );

    return;
}


void do_quicksand( CHAR_DATA *ch, char *argument )
{
    if (!IS_WEREWOLF(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (get_tribe(ch,TRIBE_RED_TALONS) < 4)
    {
	stc("You require Red Talons 4 to use Quicksand.\n\r", ch);
	return;
    }

    stc("Power Gutted - Akuma.\n\r", ch);
    return;
}


void do_lycaon( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MSL];

    one_argument(argument,arg);

    if (IS_NPC(ch)) return;

    if (!IS_WEREWOLF(ch))
    {
	stc("Huh?\n\r",ch);
	return;
    }

    if (get_tribe(ch,TRIBE_RED_TALONS) < 5)
    {
	stc("You require Red Talons 5 to use Curse of Lycaon.\n\r",ch);
	return;
    }

    if (arg[0] == '\0')
    {
	stc("Curse whom?\n\r",ch);
	return;
    }

    if ((victim = get_char_room(ch,arg)) == NULL)
    {
	stc("They aren't here.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim)) return;

    stc("This power has been gutted.\n\r", ch);

/*    stc("You sneer as you force them into the form of a wolf.\n\r",ch);
    act("$n sneers as $e concentrates on $N",ch,NULL,victim,TO_NOTVICT);
    act("$n sneers as $e concentrates on you.",ch,NULL,victim,TO_VICT);*/

    return;
}

void do_clap( CHAR_DATA *ch, char *argument )
{
    char      arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *victim_next;
    long        damage;
    bool       no_dam = FALSE;

    one_argument( argument, arg );
    if (IS_NPC(ch)) no_dam = TRUE;
    else
    {
	if (!IS_WEREWOLF(ch)) no_dam = TRUE;
	else if (get_tribe(ch,TRIBE_SHADOW_LORDS) < 2) no_dam = TRUE;
    }

    {
	send_to_char( "You clap your hands together.\n\r", ch );
	act("$n claps $s hands together.",ch,NULL,NULL,TO_ROOM);
	if (no_dam) return;
	damage = get_tribe(ch, TRIBE_SHADOW_LORDS) * number_range(50,100);
	for ( victim = ch->in_room->people; victim; victim = victim_next )
	{
	    victim_next = victim->next_in_room;

	    if ( victim == ch ) continue;
	    if (IS_BUI(victim)) continue;
	    if (is_same_group(ch, victim) ) continue;
	    if (is_safe(ch,victim))
	    {
		if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN)
	         && get_breed(ch,BREED_HOMID) < 5)
		{
		    guardian_message( victim );
		    act("$n just used Clap of Thunder on you.",ch,NULL,victim,TO_VICT);
		}
		return;
	    }
	    act("The room reverberates with a huge roll of thunder!",victim,NULL,NULL,TO_ROOM);
	    send_to_char("The room reverberates with a huge roll of thunder!\n\r",victim);
	    send_to_char("You clutch your head in agony as your eardrums seem to explode!\n\r",victim);
	    act("$n clutches $s head in agony!",victim,NULL,NULL,TO_ROOM);
	    if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN)
	     && get_breed(ch,BREED_HOMID) < 5)
	    {
		guardian_message( victim );
		act("$n just used Clap of Thunder on you.",ch,NULL,victim,TO_VICT);
	    }
	    victim->hit -= damage;
	    if ((IS_NPC(victim) && victim->hit < 1))
	    {
		act("$n's body falls limply to the ground.",victim,NULL,NULL,TO_ROOM);
		group_gain(ch,victim);
		killperson(ch,victim);
	    }
	    else
	    {
		update_pos(victim);

		if (number_range(1,20) <= get_tribe(ch, TRIBE_SHADOW_LORDS) && victim != ch)
		{
		    victim->position = POS_STUNNED;
		    send_to_char("You fall to the ground, stunned!\n\r",ch);
		    act("$n falls to the ground, stunned!",victim,NULL,NULL,TO_ROOM);
		}
		else if (IS_NPC(victim)) do_kill(victim,ch->name);
	    }
	}
	WAIT_STATE( ch, 24 );
	return;
    }
    return;
}

void do_paralyzing( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg [MIL];
    argument = one_argument(argument, arg);

    if (!IS_WEREWOLF(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (get_tribe(ch, TRIBE_SHADOW_LORDS) < 3)
    {
	stc("You require the level 3 power of the Shadow Lords to use Paralyzing Stare.\n\r", ch);
	return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL)
    {
	stc("They aren't here.\n\r", ch);
	return;
    }

    if (ch == victim)
    {
	stc("I think not...\n\r", ch);
	return;
    }

    act("You stare deeply into $N's eyes.", ch, NULL, victim, TO_CHAR);
    act("$n stares deeply into $N's eyes.", ch, NULL, victim, TO_NOTVICT);
    act("$n stares deeply into your eyes.", ch, NULL, victim, TO_VICT);

    if (is_safe(ch, victim))
    {
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN)
	 && get_breed(ch, BREED_HOMID) < 5)
	{
	    guardian_message(victim);
	    act("$n just failed to use Paralyzing Stare on you.", ch, NULL, victim, TO_VICT);
	}
	return;
    }

    if(number_range(1, 3) != 1)
    {
        act("You stare back into $N's eyes.", victim, NULL, ch, TO_CHAR);
        act("$n stares back into $N's eyes.", victim, NULL, ch, TO_NOTVICT);
        act("$n stares back into your eyes.", victim, NULL, ch, TO_VICT);

	stc("You paralyze them with your stare!\n\r", ch);
	stc("You are momentarily paralyzed!\n\r", victim);

    	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN) && get_breed(ch, BREED_HOMID) < 5)
    	{
	    guardian_message(victim);
	    act("$n just used Paralyzing stare on you.", ch, NULL, victim, TO_VICT);
	}
	WAIT_STATE(victim, 24);
    }
    else
    {
	act("Your Paralyzing Stare failed on $N.", ch, NULL, victim, TO_CHAR);
	act("$n's Paralyzing Stare has failed on $N.", ch, NULL, victim, TO_NOTVICT);
	act("$n's Paralyzing Stare failed against you.", ch, NULL, victim, TO_VICT);
    }
    WAIT_STATE(ch, 21);
    return;
}

void do_dominator( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MIL];
    int steal = number_range(50, 100);

    one_argument(argument, arg);

    if (IS_NPC(ch)) return;

    if (!IS_WEREWOLF(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (get_tribe(ch, TRIBE_SHADOW_LORDS) < 4)
    {
	stc("You require Shadow Lords 4 to use Strength of the Dominator.\n\r", ch);
	return;
    }

    if (arg[0] == '\0')
    {
	stc("Dominate whom?\n\r",ch);
	return;
    }

    if ((victim = get_char_room(ch,arg)) == NULL)
    {
	stc("They aren't here.\n\r",ch);
	return;
    }

    act("$n snarls and taunts $N.",ch,NULL,victim,TO_NOTVICT);
    if (IS_WEREWOLF(victim) && victim->pcdata->wolf > 0)
    {
	act("You draw $N's anger into yourself.",ch,NULL,victim,TO_CHAR);
	act("You feel your anger drained away as $n taunts you.",ch,NULL,victim,TO_VICT);
	if (steal > victim->pcdata->wolf)
	    steal = victim->pcdata->wolf;
	if (ch->pcdata->wolf + steal > 300)
	{
	    steal = 300 - ch->pcdata->wolf;
	    do_werewolf(ch,"");
	}
	victim->pcdata->wolf -= steal;
	ch->pcdata->wolf += steal;
	if (victim->pcdata->wolf <= 0)
	    do_unwerewolf(victim,"");
    }
    else if (IS_VAMPIRE(victim) && victim->pcdata->wolf > 0)
    {
	act("You draw $N's anger into yourself.",ch,NULL,victim,TO_CHAR);
	act("You feel your anger drained away as $n taunts you.",ch,NULL,victim,TO_VICT);
	if (steal > victim->pcdata->wolf * 5)
	    steal = victim->pcdata->wolf * 5;
	if (ch->pcdata->wolf + steal > 300)
	{
	    steal = 300 - ch->pcdata->wolf;
	    do_werewolf(ch,"");
	}
	victim->pcdata->wolf -= steal/5;
	ch->pcdata->wolf += steal;
	if (victim->pcdata->wolf <= 0)
	{
	    if (IS_VAMPAFF(victim, VAM_NIGHTSIGHT) && !IS_POLYAFF(victim, POLY_ZULO))
        	do_nightsight(victim,"");
	    if (IS_VAMPAFF(victim, VAM_FANGS) && !IS_POLYAFF(victim, POLY_ZULO))
		do_fangs(victim,"");
	    if (IS_VAMPAFF(victim, VAM_CLAWS) && !IS_POLYAFF(victim, POLY_ZULO))
		do_claws(victim,"");
	    stc("You feel your beast release its hold.\n\r",victim);
	}
    }
    else stc("They have no anger to draw out.\n\r",ch);
    WAIT_STATE(ch,12);
    return;
}

void do_obedience( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char action[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *vch_next;
    long level;

    argument = one_argument( argument, arg );

    if (IS_NPC(ch)) return;
    if (!IS_WEREWOLF(ch))
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }
    if (get_tribe(ch,TRIBE_SHADOW_LORDS) < 5)
    {
	send_to_char( "You require the level 5 power of the Shadow Lords to use Obedience on people.\n\r", ch );
	return;
    }

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Make whom to do what?\n\r", ch );
	return;
    }

    strcpy(action,argument);

    if (is_in(action,"|quit*gift*claim*consent*propose*vampi*sid *side */*ha *hav*"))
    {
	send_to_char( "I think not.\n\r", ch );
	return;
    }

    level = 5*get_tribe(ch,TRIBE_SHADOW_LORDS);

    if (!str_cmp(arg,"all"))
    {
	sprintf( buf, "I command everyone to %s", action );
	do_say(ch,buf);
	WAIT_STATE(ch,24);
	for ( victim = ch->in_room->people; victim != NULL; victim = vch_next )
	{
	    vch_next = victim->next_in_room;
	    WAIT_STATE(ch,24);
	    if ( victim == ch ) continue;
	    if ( IS_BUI(victim) ) continue;
            if (is_safe(ch, victim)) continue;

	    if ( IS_NPC(victim) && victim->level >= level )
	    {
	    	act("$s ignores you.",victim,NULL,ch,TO_VICT);
		continue;
	    }

	    if (!IS_NPC(victim) && victim->pcdata->resist[WILL_DOMINATE] > 0)
	    {
		act("You ignore $N.",victim,NULL,ch,TO_CHAR);
		act("$n ignores $N.",victim,NULL,ch,TO_NOTVICT);
		act("$n ignores you.",victim,NULL,ch,TO_VICT);
		if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN)
		 && get_breed(ch,BREED_HOMID) < 5)
		{
		    guardian_message( victim );
		    act("$n just failed to use Obedience on you.",ch,NULL,victim,TO_VICT);
		}
		continue;
	    }
	
	    if (!IS_NPC(victim) && IS_WILLPOWER(victim, RES_DOMINATE) && 
		victim->pcdata->willpower[0] >= 20)
	    {
		victim->pcdata->willpower[0] -= 20;
		victim->pcdata->resist[WILL_DOMINATE] = 60;
		if (!IS_WILLPOWER(victim, RES_TOGGLE))
		    REMOVE_BIT(victim->pcdata->resist[0], RES_DOMINATE);
		send_to_char("You burn some willpower to resist Obedience.\n\r",victim);
		act("You ignore $N.",victim,NULL,ch,TO_CHAR);
		act("$n ignores $N.",victim,NULL,ch,TO_NOTVICT);
		act("$n ignores you.",victim,NULL,ch,TO_VICT);
		if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN)
		 && get_breed(ch,BREED_HOMID) < 5)
		{
		    guardian_message( victim );
		    act("$n just failed to use Obedience on you.",ch,NULL,victim,TO_VICT);
		}
		continue;
	    }

	    act("You obey $N.",victim,NULL,ch,TO_CHAR);
	    if (IS_NPC(victim)) SET_BIT(victim->act, ACT_COMMANDED);
	    interpret( victim, action );
	    if (IS_NPC(victim)) REMOVE_BIT(victim->act, ACT_COMMANDED);
	    if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN)
	     && get_breed(ch,BREED_HOMID) < 5)
	    {
		guardian_message( victim );
		act("$n just used Obedience on you.",ch,NULL,victim,TO_VICT);
	    }
	}
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Now that would be just plain stupid.\n\r", ch );
	return;
    }

    if ( IS_BUI(victim) )
    {
	send_to_char( "I think not...\n\r", ch );
	return;
    }
    if(!IS_NPC(victim) && !CAN_PK(victim))
     {
	send_to_char("Newbie killing is not allowed\n\r", ch);
	return;
     }
    if ( ch->in_room == NULL || IS_SET(ch->in_room->room_flags,ROOM_SAFE))
     {
	send_to_char("Something in the air stops your attack!\n\r",ch);
	return;
     }

    if (IS_NPC(victim))
	sprintf( buf, "I command %s to %s", victim->short_descr, action );
    else if (!IS_NPC(victim) && IS_AFFECTED(victim, AFF_POLYMORPH))
	sprintf( buf, "I command %s to %s", victim->morph, action );
    else
	sprintf( buf, "I command %s to %s", victim->name, action );
    do_say(ch,buf);
    WAIT_STATE(ch,24);

    if ( IS_NPC(victim) && victim->level >= level )
    {
	act("You ignores $N.",victim,NULL,ch,TO_CHAR);
	act("$n ignores $N.",victim,NULL,ch,TO_NOTVICT);
	act("$n ignores you.",victim,NULL,ch,TO_VICT);
	return;
    }

    if (!IS_NPC(victim) && victim->pcdata->resist[WILL_DOMINATE] > 0)
    {
	act("You ignores $N.",victim,NULL,ch,TO_CHAR);
	act("$n ignores $N.",victim,NULL,ch,TO_NOTVICT);
	act("$n ignores you.",victim,NULL,ch,TO_VICT);
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN)
         && get_breed(ch,BREED_HOMID) < 5)
	{
	    guardian_message( victim );
	    act("$n just failed to use Obedience on you.",ch,NULL,victim,TO_VICT);
	}
	return;
    }

    if (!IS_NPC(victim) && IS_WILLPOWER(victim, RES_DOMINATE) && 
	victim->pcdata->willpower[0] >= 10)
    {
	victim->pcdata->willpower[0] -= 10;
	victim->pcdata->resist[WILL_DOMINATE] = 60;
	if (!IS_WILLPOWER(victim, RES_TOGGLE))
	    REMOVE_BIT(victim->pcdata->resist[0], RES_DOMINATE);
	send_to_char("You burn some willpower to resist Obedience.\n\r",victim);
	act("You ignores $N.",victim,NULL,ch,TO_CHAR);
	act("$n ignores $N.",victim,NULL,ch,TO_NOTVICT);
	act("$n ignores you.",victim,NULL,ch,TO_VICT);
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN)
         && get_breed(ch,BREED_HOMID) < 5)
	{
	    guardian_message( victim );
	    act("$n just failed to use Obedience on you.",ch,NULL,victim,TO_VICT);
	}
	return;
    }

    act("You obey $N.",victim,NULL,ch,TO_CHAR);
    if (IS_NPC(victim)) SET_BIT(victim->act, ACT_COMMANDED);
    interpret( victim, action );
    if (IS_NPC(victim)) REMOVE_BIT(victim->act, ACT_COMMANDED);
    if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN)
     && get_breed(ch,BREED_HOMID) < 5)
    {
	guardian_message( victim );
	act("$n just used Obedience on you.",ch,NULL,victim,TO_VICT);
    }
    return;
}

void do_reach( CHAR_DATA *ch, char *argument )
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

    if (get_tribe(ch,TRIBE_SILENT_STRIDERS) < 5)
    {
	stc("You require Theurge 5 to Reach the Umbra.\n\r",ch);
	return;
    }

    if ( arg[0] == '\0' )
    {
        stc( "Who do you want to pull with you into Umbra?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        stc( "They are not here.\n\r", ch );
        return;
    }

    if (ch == victim)
    {
	stc("All you have to do is step sideways.\n\r", ch);
	return;
    }

    if (IS_AFFECTED(ch, AFF_SHADOWPLANE))
    {
	stc("Not while in the Umbra.\n\r", ch);
	return;
    }
    if (!IS_AFFECTED(victim, AFF_SHADOWPLANE))
    {
	stc("They aren't in the Umbra.\n\r", ch);
	return;
    }

    act("You reach toward $N and pull $m out of the Umbra",ch,NULL,victim,TO_CHAR);
    act("$n reaches toward $N and pulls $m out of the Umbra.",ch,NULL,victim,TO_NOTVICT);
    act("$n reaches toward you and pulls you out of the Umbra.",ch,NULL,victim,TO_VICT);
    REMOVE_BIT(victim->affected_by, AFF_SHADOWPLANE);
    return;
}

void do_wrath( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *ich; 
    CHAR_DATA *fch;
    AFFECT_DATA af;
    long sn;

    if (IS_NPC(ch)) return;

    if (!IS_WEREWOLF(ch))
    {
	stc("Huh?\n\r",ch);
	return;
    }

    if (get_tribe(ch,TRIBE_SILVER_FANGS) < 3)
    {
	stc("You require Silver Fangs 3 to use Wrath of Gaia.\n\r",ch);
	return;
    }

    if ( ( sn = skill_lookup( "fear" ) ) < 0 )
    {
	stc( "Bug - please inform Akuma.\n\r", ch );
	return;
    }

    stc("You start to howl.\n\r",ch);
    act("$n starts to howl.",ch,NULL,NULL,TO_ROOM);

    for ( ich = ch->in_room->people; ich != NULL; ich = fch )
    {
	fch = ich->next_in_room;
        if (IS_NPC(ich))
	    continue;
        if ((!IS_VAMPIRE(ich) || ch->beast <= 0))
	    continue;
	if (ich == ch)
	    continue;
	if ( IS_AFFECTED(ich, AFF_FEAR) )
	    continue;

	stc("You are overcome by fear.\n\r",ich);
	act("$n screams in terror!",ich,NULL,NULL,TO_ROOM);
	af.type      = sn;
	af.location  = APPLY_HITROLL;
	af.modifier  = 0 - get_tribe(ch,TRIBE_SILVER_FANGS);
	af.duration  = number_range(1,2);
	af.bitvector = AFF_FEAR;
	affect_to_char( ich, &af );

	af.location  = APPLY_DAMROLL;
	af.modifier  = 0 - get_tribe(ch,TRIBE_SILVER_FANGS);
	affect_to_char( ich, &af );
    }
    WAIT_STATE(ch,20);
    return;
}

void do_mastery( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument(argument,arg);

    if (IS_NPC(ch)) return;

    if (!IS_WEREWOLF(ch))
    {
	stc("Huh?\n\r",ch);
	return;
    }

    if (get_tribe(ch,TRIBE_SILVER_FANGS) < 5)
    {
	stc("You require Silver Fangs 5 to use Mastery.\n\r",ch);
	return;
    }

    if (arg[0] =='\0')
    {
	stc("Use Mastery on whom?\n\r",ch);
	return;
    }

    if ((victim = get_char_room(ch,arg)) == NULL)
    {
	stc("They aren't here.\n\r",ch);
	return;
    }

    if (IS_NPC(victim) || !IS_WEREWOLF(victim))
    {
	stc("Only on other Garou.\n\r",ch);
	return;
    }

    if (ch->pcdata->rank <= victim->pcdata->rank)
    {
	stc("Mastery only works on those of a lower rank.\n\r",ch);
	return;
    }

    act( "$n orders you to '$t'.", ch, argument, victim, TO_VICT );
    interpret( victim, argument );
    WAIT_STATE(ch,12);
}

void do_falling( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MIL];
    char buf[MIL];
    long damage;

    one_argument(argument, arg);

    if (IS_NPC(ch)) return;

    if (!IS_WEREWOLF(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (get_tribe(ch, TRIBE_STARGAZERS) < 1)
    {
	stc("You require Stargazers 1 to use Falling Touch.\n\r", ch);
	return;
    }

    if (arg[0] == '\0')
    {
	stc("Use Falling Touch on whom?\n\r", ch);
	return;
    }
    if ((victim = get_char_room(ch, arg)) == NULL)
    {
	stc("They aren't here.\n\r", ch);
	return;
    }
    if (ch == victim)
    {
	stc("Very very stupid.  No.\n\r", ch);
	return;
    }

    if (!hurt_fighting(ch, victim)) return;

    damage = number_range(125, 250) * get_tribe(ch, TRIBE_STARGAZERS);

    sprintf(buf, "You touch $N, sending them sprawling! [#g%li#n]", damage);
    act(buf, ch, NULL, victim, TO_CHAR);
    sprintf(buf, "You touch $N, sending them sprawling! [#0%li#n]", damage);
    act(buf, ch, NULL, victim, TO_NOTVICT);
    sprintf(buf, "$n touches you, sending you sprawling! [#r%li#n]",damage);
    act(buf, ch, NULL, victim, TO_VICT);

    hurt_person(ch, victim, damage);
    WAIT_STATE(ch, 24);
    WAIT_STATE(victim, 12);
    return;
}

void do_invisibility( CHAR_DATA *ch, char *argument )
{
    long sn;

    if (IS_NPC(ch)) return;

    if (!IS_WEREWOLF(ch))
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if (get_tribe(ch,TRIBE_UKTENA) < 4)
    {
	send_to_char("You require the level 4 power of the Uktena to use Invisibility.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(ch, AFF_INVISIBLE))
    {
	send_to_char("But you are already invisible!\n\r",ch);
	return;
    }

    act( "A look of concentration crosses $n's face.", ch, NULL, NULL, TO_ROOM );

    if ( ( sn = skill_lookup( "invis" ) ) < 0 ) return;
    (*skill_table[sn].spell_fun) ( sn, 50, ch, ch );
    WAIT_STATE( ch, 12 );
    return;
}

void do_fetishdoll( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA  *obj;
    char       buf [MAX_INPUT_LENGTH];
    char       arg [MAX_INPUT_LENGTH];
    char     part1 [MAX_INPUT_LENGTH];
    char     part2 [MAX_INPUT_LENGTH];
    long       worn;

    one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_WEREWOLF(ch) )
    {
	send_to_char("Huh?\n\r",ch);
	return;
    }

    if (get_tribe(ch,TRIBE_UKTENA) < 5)
    {
	send_to_char( "You require the level 5 power of the Uktena to create a Fetish Doll.\n\r", ch );
	return;
    }

    if (arg[0] == '\0')
    {
	send_to_char( "Who do you wish to make a Fetish Doll of?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "Nobody by that name is playing.\n\r", ch );
	return;
    }

    if ( ( obj = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
    {
	send_to_char( "You are not holding any body parts.\n\r", ch );
	return;
    }
    else worn = WEAR_HOLD;

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if      (obj->value[2] == 12) sprintf(part1,"head %s",victim->name);
    else if (obj->value[2] == 13) sprintf(part1,"heart %s",victim->name);
    else if (obj->value[2] == 14) sprintf(part1,"arm %s",victim->name);
    else if (obj->value[2] == 15) sprintf(part1,"leg %s",victim->name);
    else if (obj->value[2] == 30004) sprintf(part1,"entrails %s",victim->name);
    else if (obj->value[2] == 30005) sprintf(part1,"brain %s",victim->name);
    else if (obj->value[2] == 30006) sprintf(part1,"eye eyeball %s",victim->name);
    else if (obj->value[2] == 30012) sprintf(part1,"face %s",victim->name);
    else if (obj->value[2] == 30013) sprintf(part1,"windpipe %s",victim->name);
    else if (obj->value[2] == 30014) sprintf(part1,"cracked head %s",victim->name);
    else if (obj->value[2] == 30025) sprintf(part1,"ear %s",victim->name);
    else if (obj->value[2] == 30026) sprintf(part1,"nose %s",victim->name);
    else if (obj->value[2] == 30027) sprintf(part1,"tooth %s",victim->name);
    else if (obj->value[2] == 30028) sprintf(part1,"tongue %s",victim->name);
    else if (obj->value[2] == 30029) sprintf(part1,"hand %s",victim->name);
    else if (obj->value[2] == 30030) sprintf(part1,"foot %s",victim->name);
    else if (obj->value[2] == 30031) sprintf(part1,"thumb %s",victim->name);
    else if (obj->value[2] == 30032) sprintf(part1,"index finger %s",victim->name);
    else if (obj->value[2] == 30033) sprintf(part1,"middle finger %s",victim->name);
    else if (obj->value[2] == 30034) sprintf(part1,"ring finger %s",victim->name);
    else if (obj->value[2] == 30035) sprintf(part1,"little finger %s",victim->name);
    else if (obj->value[2] == 30036) sprintf(part1,"toe %s",victim->name);
    else
    {
	sprintf(buf,"%s isn't a part of %s!\n\r",obj->name,victim->name);
	send_to_char( buf, ch );
	return;
    }

    sprintf(part2,obj->name);

    if ( str_cmp(part1,part2) )
    {
	sprintf(buf,"But you are holding %s, not %s!\n\r",obj->short_descr,victim->name);
	send_to_char( buf, ch );
	return;
    }

    act("$p vanishes from your hand in a puff of smoke.", ch, obj, NULL, TO_CHAR);
    act("$p vanishes from $n's hand in a puff of smoke.", ch, obj, NULL, TO_ROOM);
    obj_from_char(obj);
    extract_obj(obj);

    obj = create_object( get_obj_index( OBJ_VNUM_VOODOO_DOLL ), 0 );

    sprintf(buf,"%s voodoo doll",victim->name);
    free_string(obj->name);
    obj->name=str_dup(buf);

    sprintf(buf,"a voodoo doll of %s",victim->name);
    free_string(obj->short_descr);
    obj->short_descr=str_dup(buf);

    sprintf(buf,"A voodoo doll of %s lies here.",victim->name);
    free_string(obj->description);
    obj->description=str_dup(buf);

    obj_to_char(obj,ch);
    equip_char(ch,obj,worn);

    act("$p appears in your hand.", ch, obj, NULL, TO_CHAR);
    act("$p appears in $n's hand.", ch, obj, NULL, TO_ROOM);

    return;
}

void do_camouflage( CHAR_DATA *ch, char *argument )
{
    if (!IS_WEREWOLF(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (get_tribe(ch, TRIBE_WENDIGOS) < 1)
    {
	stc("You require the level 1 power of the Wendigos to use Camouflage.\n\r", ch);
	return;
    }

    if (IS_AFFECTED(ch, AFF_HIDE))
    {
	stc("You are already camouflaging yourself.\n\r", ch);
	return;
    }

    act("A look of concentration crosses $n's face.", ch, NULL, NULL, TO_ROOM);

    if (!IS_AFFECTED(ch, AFF_HIDE)) 
    {
	act("$n seems to blend into the shadows.", ch, NULL, NULL, TO_ROOM);
	send_to_char("You Camouflage yourself.\n\r", ch);
	SET_BIT(ch->affected_by, AFF_HIDE);
    }
    WAIT_STATE(ch, 12);
    return;
}

void do_cutting( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MSL];

    one_argument(argument, arg);

    if (!IS_WEREWOLF(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (get_tribe(ch, TRIBE_WENDIGOS) < 2)
    {
	stc("You require Wendigos 2 to use Cutting Wind.\n\r", ch);
	return;
    }

    if (arg[0] == '\0')
    {
	stc("Direct Cutting Wind at whom?\n\r", ch);
	return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL)
    {
	stc("They aren't here.\n\r", ch);
	return;
    }

    if (is_safe(ch, victim)) return;

    one_hit(ch, victim, TYPE_HIT, "cutting wind");

    if (victim != NULL && number_range(1, (40 - get_tribe(ch, TRIBE_WENDIGOS) * 5)) == 1)
    {
	act("Your cutting wind blasts $N off $S feet, leaving $M stunned!", ch, NULL, victim, TO_CHAR);
	act("$n's cutting wind blasts $N off $S feet, leaving $M stunned!", ch, NULL, victim, TO_NOTVICT);
	act("$n's cutting wind blasts you off your feet, leaving you stunned!", ch, NULL, victim, TO_VICT);
	stop_fighting(victim, TRUE);
	update_pos(victim);
	if (victim->position > POS_STUNNED) victim->position = POS_STUNNED;
    }

    WAIT_STATE(ch, 12);
    return;
}

void do_bloodyfeast( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *corpse;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    AFFECT_DATA af;
    char arg[MIL];
    int sn;

    one_argument(argument, arg);

    if (!IS_WEREWOLF(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (get_tribe(ch, TRIBE_WENDIGOS) < 3)
    {
	stc("You require Wendigos 3 to use Bloody Feast.\n\r", ch);
	return;
    }

    if (arg[0] == '\0')
    {
	stc("Devour which corpse?\n\r", ch);
	return;
    }

    if ((corpse = get_obj_room(ch, arg)) == NULL)
    {
	stc("It isn't here.\n\r", ch);
	return;
    }

    if (corpse->item_type == ITEM_CORPSE_PC)
    {
	stc("Not on player corpses.\n\r", ch);
	return;
    }

    if (corpse->item_type != ITEM_CORPSE_NPC)
    {
	stc("You may only devour corpses.\n\r", ch);
	return;
    }

    stc("You savagely devour a corpse.\n\r", ch);
    act("$n savagely devours a corpse.", ch, NULL, NULL, TO_ROOM);
    for (obj = corpse->contains; obj; obj = obj_next)
    {
	obj_next = obj->next_content;
	obj_from_obj(obj);
	obj_to_room(obj, ch->in_room);
    }
    if (corpse != NULL) extract_obj(corpse);

    ch->hit += 1000;
    if (ch->hit > ch->max_hit) ch->hit = ch->max_hit;
    if ((sn = skill_lookup("giant strength")) < 0)
    {
	stc("Bug, report this to Akuma.\n\r", ch);
	return;
    }
    if (!is_affected(ch, sn))
    {
	af.type      = sn;
	af.duration  = get_tribe(ch, TRIBE_WENDIGOS) * 10;
	af.location  = APPLY_DAMROLL;
	af.modifier  = get_tribe(ch, TRIBE_WENDIGOS) * 50;
	af.bitvector = 0;
	affect_to_char(ch, &af);
    }
    stc("You feel a rush of energy!\n\r", ch);
    return;
}

void do_invoke( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;
    char arg1[MIL];
    char arg2[MIL];
    char buf[MSL];

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (!IS_WEREWOLF(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (get_tribe(ch, TRIBE_WENDIGOS) < 5)
    {
	stc("You require Wendigos 5 to use Invoke the Spirits of the Storm.\n\r", ch);
	return;
    }

    if (arg1[0] == '\0')
    {
	stc("Syntax: invoke <fog/rain/lightning>\n\r", ch);
	return;
    }

    if (ch->pcdata->weather > 0)
    {
	if (ch->pcdata->weather > 1)
	    sprintf(buf, "You may not Invoke the Spirits of the Storm for %li seconds.\n\r", ch->pcdata->weather);
	else
	    sprintf(buf, "You may not Invoke the Spirits of the Storm for 1 second.\n\r");
	stc(buf, ch);
	return;
    }

    if (!IS_OUTSIDE(ch))
    {
	stc("You can only use this power outside.\n\r", ch);
	return;
    }

    stc("In deep concentration, you call to the spirits of the wind.\n\r", ch);
    act("While concentrating, $n starts to howl.", ch, NULL, NULL, TO_ROOM);

    if (!str_cmp(arg1, "fog"))
    {
	if (IS_SET(world_affects, WORLD_FOG))
	{
	    stc("You banish the fog.\n\r", ch);
	    REMOVE_BIT(world_affects, WORLD_FOG);
	    for (d = descriptor_list; d != NULL; d = d->next)
	    {
		if (d->connected != CON_PLAYING || (victim = d->character) == NULL || IS_NPC(victim) || 
		 victim->in_room == NULL || ch == victim || victim->in_room->sector_type == SECT_INSIDE)
		    continue;
		stc("The fog disperses.\n\r", victim);
	    }
	}
	else
	{
	    stc("You summon a thick blanket of fog.\n\r", ch);
	    SET_BIT(world_affects, WORLD_FOG);
	    for (d = descriptor_list; d != NULL; d = d->next)
	    {
		if (d->connected != CON_PLAYING || (victim = d->character) == NULL || IS_NPC(victim) || 
		 victim->in_room == NULL || ch == victim || victim->in_room->sector_type == SECT_INSIDE)
		    continue;
		stc("A thick blanket of fog rises from the ground.\n\r", victim);
	    }
	}
	ch->pcdata->weather = 30;
    }
    else if (!str_cmp(arg1, "rain"))
    {
	if (IS_SET(world_affects, WORLD_RAIN))
	{
	    stc("You banish the rain.\n\r", ch);
	    REMOVE_BIT(world_affects, WORLD_RAIN);
	    weather_info.sky = SKY_CLOUDY;
	    for (d = descriptor_list; d != NULL; d = d->next)
	    {
		if (d->connected != CON_PLAYING || (victim = d->character) == NULL || IS_NPC(victim) || 
		 victim->in_room == NULL || ch == victim || victim->in_room->sector_type == SECT_INSIDE)
		    continue;
		stc("It stops raining.\n\r", victim);
	    }
	}
	else
	{
	    stc("You summon a heavy shower of rain.\n\r", ch);
	    SET_BIT(world_affects, WORLD_RAIN);
	    weather_info.sky = SKY_RAINING;
	    for (d = descriptor_list; d != NULL; d = d->next)
	    {
		if (d->connected != CON_PLAYING || (victim = d->character) == NULL || IS_NPC(victim) || 
		 victim->in_room == NULL || ch == victim || victim->in_room->sector_type == SECT_INSIDE)
		    continue;
		stc("It starts to rain heavily.\n\r", victim);
	    }
	}
	ch->pcdata->weather = 20;
    }
    else if (!str_cmp(arg1, "lightning"))
    {
	int dam;
	if (arg2[0] == '\0' && ch->fighting == NULL)
	{
	    stc("Call lightning down on whom?\n\r", ch);
	    return;
	}
	if ((victim = ch->fighting) == NULL && (victim = get_char_room(ch, arg2)) == NULL)
	{
	    stc("They aren't here.\n\r", ch);
	    return;
	}

	act("You call a bolt of lightning down on $N", ch, NULL, victim, TO_CHAR);
	act("$n calls a bolt of lightning down on $N", ch, NULL, victim, TO_NOTVICT);
	act("$n calls a bolt of lightning down on you.", ch, NULL, victim, TO_VICT);
	dam = number_range(1000, 2000) + (100 * ch->pcdata->rank);
	damage(ch, victim, dam, TYPE_HIT, "lightning blast");
	dam = number_range(1000, 2000) + (100 * ch->pcdata->rank);
	damage(ch, victim, dam, TYPE_HIT, "lightning blast");
	dam = number_range(1000, 2000) + (100 * ch->pcdata->rank);
	damage(ch, victim, dam, TYPE_HIT, "lightning blast");
	ch->pcdata->weather = 10;
    }
    else stc("Syntax: invoke <fog/rain/lightning>\n\r", ch);

    return;
}
