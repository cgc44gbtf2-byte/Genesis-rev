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


void do_mindblast( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char      arg [MAX_INPUT_LENGTH];
    int chance;
    argument = one_argument( argument, arg );

    if (IS_NPC(ch)) return;
    if (!IS_VAMPIRE(ch))
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if (get_disc(ch,DISC_AUSPEX) < 7)
    {
	send_to_char("You require level 7 Auspex to use a Mind Blast.\n\r",ch);
	return;
    }

    if (IS_ITEMAFF(ch, ITEMA_PEACE)) do_majesty(ch,"");

    if ( arg[0] == '\0' )
    {
	send_to_char( "Who do you want to Mind Blast?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They are not here.\n\r", ch );
	return;
    }

    if (is_safe(ch,victim))
    {
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
	{
	    guardian_message( victim );
	    act("$n just failed to use Mind Blast on you.",ch,NULL,victim,TO_VICT);
	}
	return;
    }

    if (ch == victim)
    {
	stc("You can't blast yourself.\n\r", ch);
	return;
    }

    act("A look of concentration crosses $n's face.", ch, NULL, NULL, TO_ROOM);
    send_to_char("You clutch your head as your mind seems to explode!\n\r",victim);
    act("$n clutches $s head in agony!",victim,NULL,NULL,TO_ROOM);

    if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
    {
	guardian_message( victim );
	act("$n just used Mind Blast on you.",ch,NULL,victim,TO_VICT);
    }
    chance = number_range(1,5);
/*    if (chance > 4)
    {
	stop_fighting(victim, TRUE);
	update_pos(victim);
	if(victim->position > POS_STUNNED) victim->position = POS_STUNNED;
	return;
    }*/
    one_hit(ch, victim, TYPE_HIT, "mindblast");

    if (ch->fighting == NULL) set_fighting(ch, victim);
    if (victim->fighting == NULL) set_fighting(victim, ch);
    update_pos(victim); 

    WAIT_STATE(ch, 12);
    return;
}

void do_willpower( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if ( IS_NPC(ch) )
	return;

    if (arg[0] == '\0')
    {
	send_to_char("--------------------------------------------------------------------------------\n\r",ch);
	send_to_char("                                -= Will Power =-\n\r",ch);
	send_to_char("--------------------------------------------------------------------------------\n\r",ch);
	if (IS_WILLPOWER(ch, RES_ANIMALISM))
	    send_to_char("Animalism: You automatically try to resist Beast Within and Song of Serenity.\n\r",ch);
	else
	    send_to_char("Animalism: You don't try to resist Beast Within and Song of Serenity.\n\r",ch);
	if (IS_WILLPOWER(ch, RES_AUSPEX))
	    send_to_char("Auspex: You automatically try to resist Unveil the Shrouded Mind.\n\r",ch);
	else
	    send_to_char("Auspex: You don't try to resist Unveil the Shrouded Mind.\n\r",ch);
	if (IS_WILLPOWER(ch, RES_DOMINATE))
	    send_to_char("Dominate: You automatically try to resist Dominate attempts.\n\r",ch);
	else
	    send_to_char("Dominate: You don't try to resist Dominate attempts.\n\r",ch);
	if (IS_WILLPOWER(ch, RES_PRESENCE))
	    send_to_char("Presence: You automatically try to resist Dread Gaze and Entrancement.\n\r",ch);
	else
	    send_to_char("Presence: You don't try to resist Dread Gaze and Entrancement.\n\r",ch);
	if (IS_WILLPOWER(ch, RES_TOGGLE))
	    send_to_char("Toggle: You automatically keep burning willpower to resist attempts.\n\r",ch);
	else
	    send_to_char("Toggle: You will only burn willpower to resist each first attempt.\n\r",ch);
	send_to_char("--------------------------------------------------------------------------------\n\r",ch);
	send_to_char("You may change any of the above by typing: willpower <value>\n\r",ch);
	send_to_char("--------------------------------------------------------------------------------\n\r",ch);
	send_to_char("Remaining resistance in seconds:\n\r", ch );
	sprintf( buf,"Animalism: %li.  Auspex: %li.  Presence: %li.  Dominate: %li.\n\r",
	ch->pcdata->resist[WILL_ANIMALISM], ch->pcdata->resist[WILL_AUSPEX],
	ch->pcdata->resist[WILL_PRESENCE],  ch->pcdata->resist[WILL_DOMINATE]);
	send_to_char( buf, ch );
	send_to_char("--------------------------------------------------------------------------------\n\r",ch);
	return;
    }

    if (!str_cmp(arg,"animalism"))
    {
	if (IS_WILLPOWER(ch, RES_ANIMALISM))
	{
	    send_to_char("You will no longer try to resist Beast Within and Song of Serenity.\n\r",ch);
	    REMOVE_BIT(ch->pcdata->resist[0], RES_ANIMALISM);
	}
	else
	{
	    send_to_char("You will now automatically try to resist Beast Within and Song of Serenity.\n\r",ch);
	    SET_BIT(ch->pcdata->resist[0], RES_ANIMALISM);
	}
    }
    else if (!str_cmp(arg,"auspex"))
    {
	if (IS_WILLPOWER(ch, RES_AUSPEX))
	{
	    send_to_char("You will no longer try to resist Unveil the Shrouded Mind.\n\r",ch);
	    REMOVE_BIT(ch->pcdata->resist[0], RES_AUSPEX);
	}
	else
	{
	    send_to_char("You will now automatically try to resist Unveil the Shrouded Mind.\n\r",ch);
	    SET_BIT(ch->pcdata->resist[0], RES_AUSPEX);
	}
    }
    else if (!str_cmp(arg,"dominate"))
    {
	if (IS_WILLPOWER(ch, RES_DOMINATE))
	{
	    send_to_char("You will no longer try to resist Domination attempts.\n\r",ch);
	    REMOVE_BIT(ch->pcdata->resist[0], RES_DOMINATE);
	}
	else
	{
	    send_to_char("You will now automatically try to resist Domination attempts.\n\r",ch);
	    SET_BIT(ch->pcdata->resist[0], RES_DOMINATE);
	}
    }
    else if (!str_cmp(arg,"presence"))
    {
	if (IS_WILLPOWER(ch, RES_PRESENCE))
	{
	    send_to_char("You will no longer try to resist Dread Gaze and Entrancement.\n\r",ch);
	    REMOVE_BIT(ch->pcdata->resist[0], RES_PRESENCE);
	}
	else
	{
	    send_to_char("You will now automatically try to resist Dread Gaze and Entrancement.\n\r",ch);
	    SET_BIT(ch->pcdata->resist[0], RES_PRESENCE);
	}
    }
    else if (!str_cmp(arg,"toggle"))
    {
	if (IS_WILLPOWER(ch, RES_TOGGLE))
	{
	    send_to_char("You will now only burn willpower to resist each first attempt.\n\r",ch);
	    REMOVE_BIT(ch->pcdata->resist[0], RES_TOGGLE);
	}
	else
	{
	    send_to_char("You will now automatically keep burning willpower to resist attempts.\n\r",ch);
	    SET_BIT(ch->pcdata->resist[0], RES_TOGGLE);
	}
    }
    else
    {
	send_to_char("--------------------------------------------------------------------------------\n\r",ch);
	send_to_char("                                -= Will Power =-\n\r",ch);
	send_to_char("--------------------------------------------------------------------------------\n\r",ch);
	if (IS_WILLPOWER(ch, RES_ANIMALISM))
	    send_to_char("Animalism: You automatically try to resist Beast Within and Song of Serenity.\n\r",ch);
	else
	    send_to_char("Animalism: You don't try to resist Beast Within and Song of Serenity.\n\r",ch);
	if (IS_WILLPOWER(ch, RES_AUSPEX))
	    send_to_char("Auspex: You automatically try to resist Unveil the Shrouded Mind.\n\r",ch);
	else
	    send_to_char("Auspex: You don't try to resist Unveil the Shrouded Mind.\n\r",ch);
	if (IS_WILLPOWER(ch, RES_DOMINATE))
	    send_to_char("Dominate: You automatically try to resist Dominate attempts.\n\r",ch);
	else
	    send_to_char("Dominate: You don't try to resist Dominate attempts.\n\r",ch);
	if (IS_WILLPOWER(ch, RES_PRESENCE))
	    send_to_char("Presence: You automatically try to resist Dread Gaze and Entrancement.\n\r",ch);
	else
	    send_to_char("Presence: You don't try to resist Dread Gaze and Entrancement.\n\r",ch);
	if (IS_WILLPOWER(ch, RES_TOGGLE))
	    send_to_char("Toggle: You automatically keep burning willpower to resist attempts.\n\r",ch);
	else
	    send_to_char("Toggle: You will only burn willpower to resist each first attempt.\n\r",ch);
	send_to_char("--------------------------------------------------------------------------------\n\r",ch);
	send_to_char("You may change any of the above by typing: willpower <value>\n\r",ch);
	send_to_char("--------------------------------------------------------------------------------\n\r",ch);
	send_to_char("Remaining resistance in seconds:\n\r", ch );
	sprintf( buf,"Animalism: %li.  Auspex: %li.  Presence: %li.  Dominate: %li.\n\r",
	ch->pcdata->resist[WILL_ANIMALISM], ch->pcdata->resist[WILL_AUSPEX],
	ch->pcdata->resist[WILL_PRESENCE],  ch->pcdata->resist[WILL_DOMINATE]);
	send_to_char( buf, ch );
	send_to_char("--------------------------------------------------------------------------------\n\r",ch);
	return;
    }
    return;
}

void do_malleable( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MIL];
    char buf[MSL];

    argument = one_argument(argument, arg);

    if (!IS_VAMPIRE(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (get_disc(ch, DISC_VICISSITUDE) < 1)
    {
	stc("You require level 1 Vicissitude to use Malleable Visage.\n\r", ch);
	return;
    }

    if (arg[0] == '\0')
    {
	stc("Change to look like whom?\n\r", ch);
	return;
    }

    if (IS_AFFECTED(ch, AFF_POLYMORPH) && !IS_VAMPAFF(ch, VAM_DISGUISED))
    {
	stc("Not while polymorphed.\n\r", ch);
	return;
    }

    if (IS_POLYAFF(ch, POLY_ZULO))
    {
	stc("Not while in Zulo Form.\n\r", ch);
	return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL)
    {
	stc("They aren't here.\n\r", ch);
	return;
    }

    if (ch == victim)
    {
	if (!IS_AFFECTED(ch, AFF_POLYMORPH) && !IS_VAMPAFF(ch, VAM_DISGUISED))
	{
	    stc("You already look like yourself!\n\r", ch);
	    return;
	}
	if (!IS_CRAFT(ch, CRAFT_MALLEABLE))
	{
	    stc("Nothing happens.\n\r", ch);
	    return;
	}
	sprintf(buf, "$n's features twist and distort until $e looks like %s.", ch->name);
	act(buf, ch, NULL, NULL, TO_ROOM);
	REMOVE_BIT(ch->affected_by, AFF_POLYMORPH);
	REMOVE_BIT(ch->pcdata->bodycrafts, CRAFT_MALLEABLE);
	REMOVE_BIT(ch->vampaff, VAM_DISGUISED);
    	free_string(ch->morph);
    	ch->morph = str_dup("");
	act("Your features twist and distort until your looks return to normal.", ch, NULL, NULL, TO_CHAR);
	return;
    }

    if (!IS_NPC(victim))
    {
	stc("Only on NPCs.\n\r", ch);
	return;
    }

    if (IS_VAMPAFF(ch, VAM_DISGUISED))
    {
	if (!IS_CRAFT(ch, CRAFT_MALLEABLE))
	{
	    stc("Nothing happens.\n\r", ch);
	    return;
	}
	act("Your features twist and distort until you looks like $N.", ch, NULL, victim, TO_CHAR);
	act("$n's features twist and distort until $e looks like you.", ch, NULL, victim, TO_VICT);
	act("$n's features twist and distort until $e looks like $N.", ch, NULL, victim, TO_NOTVICT);
	free_string(ch->morph);
	ch->morph = str_dup(victim->short_descr);
	return;
    }
    act("Your features twist and distort until you looks like $N.", ch, NULL, victim, TO_CHAR);
    act("$n's features twist and distort until $e looks like you.", ch, NULL, victim, TO_VICT);
    act("$n's features twist and distort until $e looks like $N.", ch, NULL, victim, TO_NOTVICT);
    SET_BIT(ch->pcdata->bodycrafts, CRAFT_MALLEABLE);
    SET_BIT(ch->affected_by, AFF_POLYMORPH);
    SET_BIT(ch->vampaff, VAM_DISGUISED);
    free_string(ch->morph);
    ch->morph = str_dup(victim->short_descr);
    return;
}

void do_fleshcraft( CHAR_DATA *ch, char *argument )
{
    if (!IS_VAMPIRE(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (get_disc(ch, DISC_VICISSITUDE) < 2)
    {
	stc("You require level 2 Vicissitude to use Fleshcraft.\n\r", ch);
	return;
    }

    if (!IS_CRAFT(ch, CRAFT_FLESHSOAK))
    {
	stc("You reform your flesh to provide more protection.\n\r", ch);
	SET_BIT(ch->pcdata->bodycrafts, CRAFT_FLESHSOAK);
    }
    else
    {
	stc("You reform your flesh to its \"normal\" placement.\n\r", ch);
	REMOVE_BIT(ch->pcdata->bodycrafts, CRAFT_FLESHSOAK);
    }
    act("$n uses $s hands to shift $s flesh around.", ch, NULL, NULL, TO_ROOM);

    return;
}

void do_bonecraft( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];
    CHAR_DATA *victim;

    argument = one_argument(argument, arg);

    if (!IS_VAMPIRE(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (get_disc(ch, DISC_VICISSITUDE) < 3)
    {
	stc("You require level 3 Vicissitude to use Bonecraft.\n\r", ch);
	return;
    }

    if (arg[0] == '\0')
    {
	stc("Syntax: Bonecraft <target/talons/quills>\n\r", ch);
	return;
    }

    if (!str_cmp(arg, "talons"))
    {
	if (!IS_CRAFT(ch, CRAFT_BONETALONS))
	{
	    stc("You painfully pull bone spikes from your knuckles.\n\r", ch);
	    act("$n rips bone spikes out from $s knuckles!", ch, NULL, NULL, TO_ROOM);
	    SET_BIT(ch->pcdata->bodycrafts, CRAFT_BONETALONS);
	}
	else
	{
	    stc("You painfully push your bone spikes back into your knuckles.\n\r", ch);
	    act("$n forces $s bone spikes back into his knuckles!", ch, NULL, NULL, TO_ROOM);
	    REMOVE_BIT(ch->pcdata->bodycrafts, CRAFT_BONETALONS);
	}
	hurt_person(ch, ch, ch->max_hit * .05);
	return;
    }
    else if (!str_cmp(arg, "quills"))
    {
	if (!IS_CRAFT(ch, CRAFT_BONEQUILLS))
	{
	    stc("You rip bone quills out from your body!\n\r", ch);
	    act("$n rips bone quills out of $s body!\n\r", ch, NULL, NULL, TO_ROOM);
	    SET_BIT(ch->pcdata->bodycrafts, CRAFT_BONEQUILLS);
	}
	else
	{
	    stc("You shove your bone quills back into your flesh!\n\r", ch);
	    act("$n shoves $s bone quills back into $s flesh!\n\r", ch, NULL, NULL, TO_ROOM);
	    REMOVE_BIT(ch->pcdata->bodycrafts, CRAFT_BONEQUILLS);
	}
	hurt_person(ch, ch, ch->max_hit * .5);
	return;
    }
    else if ((victim = ch->fighting) == NULL || (victim = get_char_room(ch, arg)) == NULL)
    {
	stc("No such person.\n\r", ch);
	return;
    }

    if (is_safe(ch, victim)) return;

    one_hit(ch, victim, TYPE_HIT, "bone crafting");
    WAIT_STATE(ch, 12);
    return;
}

void do_zuloform( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];
    char buf[MIL];

    argument = one_argument(argument, arg);

    if (!IS_VAMPIRE(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (get_disc(ch, DISC_VICISSITUDE) < 5)
    {
	stc("You require level 5 Vicissitude to change into Zulo Form.\n\r", ch);
	return;
    }
    if (IS_AFFECTED(ch, AFF_POLYMORPH))
    {
        if (!IS_POLYAFF(ch, POLY_ZULO))
        {
            stc("You cannot change from this form.\n\r", ch);
            return;
        }
    }
    if (IS_AFFECTED(ch, AFF_POLYMORPH) && !IS_VAMPAFF(ch, VAM_DISGUISED))
    {
	stc("Not while polymorphed.\n\r", ch);
	return;
    }
    if (IS_POLYAFF(ch, POLY_ZULO))
    {
	free_string(ch->morph);
	ch->morph = str_dup("");
	REMOVE_BIT(ch->affected_by, AFF_POLYMORPH);
	REMOVE_BIT(ch->vampaff, VAM_DISGUISED);
	REMOVE_BIT(ch->vampaff, VAM_NIGHTSIGHT);
	REMOVE_BIT(ch->vampaff, VAM_CLAWS);
	REMOVE_BIT(ch->polyaff, POLY_ZULO);
	stc("Your body shrinks and distorts back to its normal form.\n\r", ch);
	act("A huge black monster's body shrinks and distorts into the form of $n.", ch, NULL, NULL, TO_ROOM);
	update_style(ch);
	return;
    }
    stc("Your body grows and distorts into a huge black monster.\n\r", ch);
    act("$n's body grows and distorts into a huge black monster.", ch, NULL, NULL, TO_ROOM);
    SET_BIT(ch->affected_by, AFF_POLYMORPH);
    SET_BIT(ch->vampaff, VAM_DISGUISED);
    SET_BIT(ch->vampaff, VAM_NIGHTSIGHT);
    SET_BIT(ch->polyaff, POLY_ZULO);
    sprintf(buf, "#0A huge black monster#n");
    free_string(ch->morph);
    ch->morph = str_dup(buf);
    update_style(ch);
    return;
}

/* The bundled world has no Hell room. Resolve an object-form escape before
 * destroying its body; prefer the historical destination, then the temple.
 * If neither exists, leave the form and object intact for staff recovery. */
static ROOM_INDEX_DATA *object_form_return_room(CHAR_DATA *ch)
{
    ROOM_INDEX_DATA *destination = ch->in_room;
    if (destination != NULL && destination->vnum == ROOM_VNUM_IN_OBJECT)
    {
        destination = get_room_index(ROOM_VNUM_HELL);
        if (destination == NULL) destination = get_room_index(ROOM_VNUM_TEMPLE);
    }
    return destination;
}

void do_plasmaform( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    ROOM_INDEX_DATA *destination;
    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch) || IS_MORE(ch, MORE_HEAD))
    {
	send_to_char("Huh?\n\r",ch);
	return;
    }

    if (get_disc(ch,DISC_VICISSITUDE) < 4)
    {
	send_to_char("You require level 4 Vicissitude to change into Plasma Form.\n\r",ch);
	return;
    }
    if(ch->fight_timer > 0)
    {
	stc("Not with a fight timer.\n\r",ch);
	return;
    }

    if (!IS_MORE(ch, MORE_PLASMA) && IS_AFFECTED(ch,AFF_POLYMORPH))
    {
	send_to_char("Not from this form.\n\r",ch);
	return;
    }

    if (IS_MORE(ch, MORE_PLASMA))
    {
        obj = ch->pcdata->chobj;
        destination = object_form_return_room(ch);
        if (destination == NULL || (obj != NULL && obj->chobj != ch))
        {
            stc("Your return is unavailable. Please contact staff.\n\r", ch);
            return;
        }
	ch->pcdata->obj_vnum = 0;
	if (obj != NULL) obj->chobj = NULL;
	ch->pcdata->chobj = NULL;
	REMOVE_BIT(ch->affected_by, AFF_POLYMORPH);
	REMOVE_BIT(ch->extra, EXTRA_OSWITCH);
	REMOVE_BIT(ch->more, MORE_PLASMA);
	free_string(ch->morph);
	ch->morph = str_dup("");
	if (obj != NULL) act("$p transforms into $n.",ch,obj,NULL,TO_ROOM);
        else act("$n reforms a human body.",ch,NULL,NULL,TO_ROOM);
	act("Your reform your human body.",ch,obj,NULL,TO_CHAR);
	if (obj != NULL) extract_obj(obj);
	if (ch->in_room != destination)
	{
	    char_from_room(ch);
	    char_to_room(ch, destination);
	}
	return;
    }
    if (IS_AFFECTED(ch,AFF_POLYMORPH))
    {
	send_to_char( "You cannot do this while polymorphed.\n\r", ch);
	return;
    }
    if ((obj = create_object(get_obj_index(OBJ_VNUM_SPILT_BLOOD),0)) == NULL)
    {
	send_to_char( "Bug - please inform KaVir.\n\r", ch );
	return;
    }
    obj_to_room(obj,ch->in_room);
    SET_BIT(obj->quest, QUEST_VANISH);
    act("$n transforms into $p and splashes to the ground.",ch,obj,NULL,TO_ROOM);
    act("You transform into $p and splash to the ground.",ch,obj,NULL,TO_CHAR);
    if (IS_EXTRA(ch, TIED_UP))
    {
	act("The ropes binding you fall through your liquid form.",ch,NULL,NULL,TO_CHAR);
	act("The ropes binding $n fall through $s liquid form.",ch,NULL,NULL,TO_ROOM);
	REMOVE_BIT(ch->extra, TIED_UP);
	REMOVE_BIT(ch->extra, GAGGED);
	REMOVE_BIT(ch->extra, BLINDFOLDED);
    }
    if (is_affected(ch, gsn_web))
    {
	act("The webbing entrapping $n falls through $s liquid form.",ch,NULL,NULL,TO_ROOM);
	send_to_char("The webbing entrapping you falls through your liquid form.\n\r",ch);
	affect_strip(ch, gsn_web);
    }
    if (IS_AFFECTED(ch, AFF_WEBBED))
    {
	act("The webbing entrapping $n falls through $s liquid form.",ch,NULL,NULL,TO_ROOM);
	send_to_char("The webbing entrapping you falls through your liquid form.\n\r",ch);
	REMOVE_BIT(ch->affected_by, AFF_WEBBED);
    }
    if (is_affected(ch, gsn_tendrils))
    {
	act("The tendrils of darkness entrapping $n fall through $s liquid form.",ch,NULL,NULL,TO_ROOM);
	send_to_char("The tendrils of darkness entrapping you fall through your liquid form.\n\r",ch);
	affect_strip(ch, gsn_tendrils);
    }
    if (IS_AFFECTED(ch, AFF_SHADOW))
    {
	act("The tendrils of darkness entrapping $n fall through $s liquid form.",ch,NULL,NULL,TO_ROOM);
	send_to_char("The tendrils of darkness entrapping you fall through your liquid form.\n\r",ch);
	REMOVE_BIT(ch->affected_by, AFF_SHADOW);
    }
    ch->pcdata->obj_vnum = OBJ_VNUM_SPILT_BLOOD;
    obj->chobj = ch;
    ch->pcdata->chobj = obj;
    SET_BIT(ch->affected_by, AFF_POLYMORPH);
    SET_BIT(ch->extra, EXTRA_OSWITCH);
    SET_BIT(ch->more, MORE_PLASMA);
    free_string(ch->morph);
    ch->morph = str_dup(obj->short_descr);
    return;
}

void do_conditioning( CHAR_DATA *ch, char *argument )
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

    if (get_disc(ch,DISC_DOMINATE) < 2)
    {
	send_to_char("You require level 2 Dominate to use Conditioning.\n\r",ch);
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if (ch == victim)
    {
	send_to_char( "You can't condition yourself!\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if (is_safe(ch,victim)) return;

    if (!can_see(victim,ch))
    {
	if (get_disc(ch,DISC_DOMINATE) < 6)
	{
	    send_to_char( "You are unable to make eye contact with them.\n\r", ch );
	    return;
	}
    }
    if(!CAN_PK(ch))
    {
	stc("You can't condition others if you're not PK.\n",ch);
	return;
    }
    if(!CAN_PK(victim))
    {
	stc("You can't condition Non-PK. Don't even try.\n",ch);
	return;
    }

    act("You stare deeply into $N's eyes.",ch,NULL,victim,TO_CHAR);
    act("$n stares deeply into $N's eyes.",ch,NULL,victim,TO_NOTVICT);
    act("$n stares deeply into your eyes.",ch,NULL,victim,TO_VICT);

    if (strlen(victim->pcdata->conding) > 2 && !str_cmp(victim->pcdata->conding,ch->name))
    {
	free_string(victim->pcdata->conding);
	victim->pcdata->conding = str_dup( "" );
	REMOVE_BIT(victim->more, MORE_LOYAL);
	act("You remove $N's conditioning.",ch,NULL,victim,TO_CHAR);
	act("$n removes your conditioning.",ch,NULL,victim,TO_VICT);
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
	{
	    guardian_message( victim );
	    act("$n just used Conditioning on you.",ch,NULL,victim,TO_VICT);
	}
	return;
    }

    if (!IS_NPC(victim) && victim->pcdata->resist[WILL_DOMINATE] > 0)
    {
	act("You shake off $N's attempt to Condition you.",victim,NULL,ch,TO_CHAR);
	act("$n shakes off $N's attempt to Condition $m.",victim,NULL,ch,TO_NOTVICT);
	act("$n shakes off your attempt to Condition $m.",victim,NULL,ch,TO_VICT);
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
	{
	    guardian_message( victim );
	    act("$n just failed to use Conditioning on you.",ch,NULL,victim,TO_VICT);
	}
	return;
    }

    if (!IS_NPC(victim) && victim->pcdata->willpower[0] >= 5 && strlen(victim->pcdata->conding) > 1)
    {
	victim->pcdata->willpower[0] -= 5;
	victim->pcdata->resist[WILL_DOMINATE] = 60;
	send_to_char("You burn some willpower to resist Conditioning.\n\r",victim);
	act("You shake off $N's attempt to Condition you.",victim,NULL,ch,TO_CHAR);
	act("$n shakes off $N's attempt to Condition $m.",victim,NULL,ch,TO_NOTVICT);
	act("$n shakes off your attempt to Condition $m.",victim,NULL,ch,TO_VICT);
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
	{
	    guardian_message( victim );
	    act("$n just failed to use Conditioning on you.",ch,NULL,victim,TO_VICT);
	}
	return;
    }

    if (!IS_NPC(victim) && IS_WILLPOWER(victim, RES_DOMINATE) && victim->pcdata->willpower[0] >= 10)
    {
	victim->pcdata->willpower[0] -= 10;
	victim->pcdata->resist[WILL_DOMINATE] = 60;
	if (!IS_WILLPOWER(victim, RES_TOGGLE))
	    REMOVE_BIT(victim->pcdata->resist[0], RES_DOMINATE);
	send_to_char("You burn some willpower to resist Conditioning.\n\r",victim);
	act("You shake off $N's attempt to Condition you.",victim,NULL,ch,TO_CHAR);
	act("$n shakes off $N's attempt to Condition $m.",victim,NULL,ch,TO_NOTVICT);
	act("$n shakes off your attempt to Condition $m.",victim,NULL,ch,TO_VICT);
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
	{
	    guardian_message( victim );
	    act("$n just failed to use Conditioning on you.",ch,NULL,victim,TO_VICT);
	}
	return;
    }

    if (strlen(victim->pcdata->conding) > 1)
    {
	free_string(victim->pcdata->conding);
	victim->pcdata->conding = str_dup( "" );
	REMOVE_BIT(victim->more, MORE_LOYAL);
	act("You remove $N's conditioning.",ch,NULL,victim,TO_CHAR);
	act("$n removes your conditioning.",ch,NULL,victim,TO_VICT);
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
	{
	    guardian_message( victim );
	    act("$n just used Conditioning on you.",ch,NULL,victim,TO_VICT);
	}
	return;
    }

    free_string(victim->pcdata->conding);
    victim->pcdata->conding = str_dup( ch->name );
    SET_BIT(victim->more, MORE_LOYAL);
    act("You condition $N's mind.",ch,NULL,victim,TO_CHAR);
    act("$n conditions your mind.",ch,NULL,victim,TO_VICT);
    WAIT_STATE( ch, 12 );
    if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
    {
	guardian_message( victim );
	act("$n just used Conditioning on you.",ch,NULL,victim,TO_VICT);
    }
    return;
}

long true_generation( CHAR_DATA *ch, char *argument )
{
    char      arg1 [MAX_INPUT_LENGTH];
    char      arg2 [MAX_INPUT_LENGTH];
    char      arg3 [MAX_INPUT_LENGTH];
    char      arg4 [MAX_INPUT_LENGTH];
    char      arg5 [MAX_INPUT_LENGTH];
    char      arg6 [MAX_INPUT_LENGTH];
    char      arg7 [MAX_INPUT_LENGTH];
    char      arg8 [MAX_INPUT_LENGTH];
    char      arg9 [MAX_INPUT_LENGTH];
    char      arg10[MAX_INPUT_LENGTH];
    char      arg11[MAX_INPUT_LENGTH];
    char      arg12[MAX_INPUT_LENGTH];
    long       vampgen = 1;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    argument = one_argument( argument, arg4 );
    argument = one_argument( argument, arg5 );
    argument = one_argument( argument, arg6 );
    argument = one_argument( argument, arg7 );
    argument = one_argument( argument, arg8 );
    argument = one_argument( argument, arg9 );
    argument = one_argument( argument, arg10 );
    argument = one_argument( argument, arg11 );
    argument = one_argument( argument, arg12 );

    if (IS_NPC(ch)) return 0;

    if (arg1[0] != '\0')  {arg1[0] = UPPER(arg1[0]);vampgen = 2;}
    if (arg2[0] != '\0')  {arg2[0] = UPPER(arg2[0]);vampgen = 3;}
    if (arg3[0] != '\0')  {arg3[0] = UPPER(arg3[0]);vampgen = 4;}
    if (arg4[0] != '\0')  {arg4[0] = UPPER(arg4[0]);vampgen = 5;}
    if (arg5[0] != '\0')  {arg5[0] = UPPER(arg5[0]);vampgen = 6;}
    if (arg6[0] != '\0')  {arg6[0] = UPPER(arg6[0]);vampgen = 7;}
    if (arg7[0] != '\0')  {arg7[0] = UPPER(arg7[0]);vampgen = 8;}
    if (arg8[0] != '\0')  {arg8[0] = UPPER(arg8[0]);vampgen = 9;}
    if (arg9[0] != '\0')  {arg9[0] = UPPER(arg9[0]);vampgen = 10;}
    if (arg10[0] != '\0') {arg10[0] = UPPER(arg10[0]);vampgen = 11;}
    if (arg11[0] != '\0') {arg11[0] = UPPER(arg11[0]);vampgen = 12;}
    if (arg12[0] != '\0') {arg12[0] = UPPER(arg12[0]);vampgen = 13;}

    return vampgen;
}

void do_tongue( CHAR_DATA *ch, char *argument )
{
    char arg [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch))
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if (get_disc(ch,DISC_SERPENTIS) < 2)
    {
	send_to_char("You require level 2 Serpentis to use Tongue of the Serpent.\n\r",ch);
	return;
    }
    if (arg[0] == '\0' && ch->fighting == NULL)
    {
	send_to_char("Who do you wish to use Tongue of the Serpent on?\n\r",ch);
	return;
    }

    if ( ( victim = ch->fighting ) == NULL )
    {
	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
	    if ( ( obj = get_obj_carry( ch, arg ) ) != NULL )
	    {
		if ( obj->item_type != ITEM_WEAPON )
		{
		    send_to_char( "You can only poison weapons with your serpentine tongue.\n\r", ch );
		    return;
		}
	/*	if ( obj->value[0] != 0 )
		{
		    send_to_char( "This weapon cannot be poisoned.\n\r", ch );
		    return;
		}*/
		act("You run your long serpentine tongue along $p, poisoning it.",ch,obj,NULL,TO_CHAR);
		act("$n runs $s long serpentine tongue along $p, poisoning it.",ch,obj,NULL,TO_ROOM);
		obj->level = 5 * get_disc(ch,DISC_SERPENTIS);
		obj->value[0] = 53;
		return;
	    }
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}
    }

    if (IS_ITEMAFF(ch, ITEMA_PEACE)) do_majesty(ch,"");

    if (is_safe(ch,victim))
    {
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
	{
	    guardian_message( victim );
	    act("$n just failed to use Tongue of the Serpent on you.",ch,NULL,victim,TO_VICT);
	}
	return;
    }

    WAIT_STATE( ch, 6 );

    one_hit( ch, victim, TYPE_HIT, "tongue" );
    return;
}

void do_mummify( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    ROOM_INDEX_DATA *destination;
    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch))
    {
	send_to_char("Huh?\n\r",ch);
	return;
    }

    if (get_disc(ch,DISC_SERPENTIS) < 3)
    {
	send_to_char("You require level 3 Serpentis to Mummify yourself.\n\r",ch);
	return;
    }

    if (IS_MORE(ch, MORE_MUMMIFIED))
    {
        obj = ch->pcdata->chobj;
        destination = object_form_return_room(ch);
        if (destination == NULL || (obj != NULL && obj->chobj != ch))
        {
            stc("Your return is unavailable. Please contact staff.\n\r", ch);
            return;
        }
	REMOVE_BIT(ch->more, MORE_MUMMIFIED);
	ch->pcdata->obj_vnum = 0;
	if (obj != NULL) obj->chobj = NULL;
	ch->pcdata->chobj = NULL;
	REMOVE_BIT(ch->affected_by, AFF_POLYMORPH);
	REMOVE_BIT(ch->extra, EXTRA_OSWITCH);
	free_string(ch->morph);
	ch->morph = str_dup("");
	if (obj != NULL) act("$p clambers to $s feet.",ch,obj,NULL,TO_ROOM);
        else act("$n reforms a human body.",ch,NULL,NULL,TO_ROOM);
	act("Your body unshrivels and returns to normal.",ch,obj,NULL,TO_CHAR);
	if (obj != NULL) extract_obj(obj);
	if (ch->in_room != destination)
	{
	    char_from_room(ch);
	    char_to_room(ch, destination);
	}
	return;
    }
    if (IS_AFFECTED(ch,AFF_POLYMORPH))
    {
	send_to_char( "You cannot do this while polymorphed.\n\r", ch);
	return;
    }
    if ((obj = create_object(get_obj_index(OBJ_VNUM_CORPSE_PC),0)) == NULL)
    {
	send_to_char( "Bug - please inform KaVir.\n\r", ch );
	return;
    }
    mummify_corpse(ch,obj);
    obj_to_room(obj,ch->in_room);
    SET_BIT(obj->quest, QUEST_VANISH);
    act("$n shrivels up and falls to the ground.",ch,NULL,NULL,TO_ROOM);
    act("You shrivel up and fall to the ground.",ch,NULL,NULL,TO_CHAR);
    ch->pcdata->obj_vnum = OBJ_VNUM_CORPSE_PC;
    obj->chobj = ch;
    ch->pcdata->chobj = obj;
    SET_BIT(ch->more, MORE_MUMMIFIED);
    SET_BIT(ch->affected_by, AFF_POLYMORPH);
    SET_BIT(ch->extra, EXTRA_OSWITCH);
    free_string(ch->morph);
    ch->morph = str_dup(obj->short_descr);
    return;
}

void mummify_corpse( CHAR_DATA *ch, OBJ_DATA *obj )
{
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch)) return;

    if (obj->pIndexData->vnum != OBJ_VNUM_CORPSE_PC)
    {
	send_to_char( "Bug - please inform KaVir.\n\r", ch );
	return;
    }

    sprintf(buf, obj->short_descr, ch->name);
    free_string(obj->short_descr);
    obj->short_descr = str_dup( buf );

    sprintf(buf, obj->description, ch->name);
    free_string(obj->description);
    obj->description = str_dup( buf );

    SET_BIT(obj->quest, QUEST_VANISH);

    return;
}


void do_controlbeast( CHAR_DATA *ch, char *argument )
{
    char arg [MAX_INPUT_LENGTH];
    char buf [MAX_STRING_LENGTH];
    long  cost = (100 - ch->truebeast);

    argument = one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch))
    {
	send_to_char("Huh?\n\r",ch);
	return;
    }

    if (ch->pcdata->wolf > 0)
    {
	send_to_char("Not while the beast is in control!\n\r",ch);
	return;
    }

    if (ch->beast > 99 || ch->truebeast > 99)
    {
	send_to_char("The beast has already taken over.\n\r",ch);
	return;
    }

    if (ch->pcdata->diableries[DIAB_EVER] > 0)
    {
	send_to_char("Those who drink of the hearts blood may not attain Golconda.\n\r",ch);
	return;
    }

    if (ch->beast < 1)
    {
	send_to_char("But you have already attained Golconda!\n\r",ch);
	return;
    }

    if (ch->truebeast < 1)
    {
	send_to_char("You are unable to exert any further control over your beast.\n\r",ch);
	return;
    }

    if (ch->primal < cost )
    {
	sprintf(buf,"You need %li primal to try and control your beast.\n\r",cost);
	send_to_char(buf,ch);
	return;
    }

    if (arg[0] == '\0' || str_cmp(arg,"buy"))
    {
	sprintf(buf,"To lower your beast for the cost of %li primal, type: 'controlbeast buy'.\n\r",cost);
	send_to_char(buf,ch);
	return;
    }

    ch->primal -= cost;
    ch->truebeast--;
    send_to_char("You exert some control over your beast.\n\r",ch);
    return;
}

void do_shroudnight( CHAR_DATA *ch, char *argument )
{
    char buf [MAX_INPUT_LENGTH];

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch) && !IS_WEREWOLF(ch))
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if (IS_VAMPIRE(ch) && get_disc(ch,DISC_OBTENEBRATION) < 1)
    {
	send_to_char("You require level 1 Obtenebration to use Shroud of Night.\n\r",ch);
	return;
    }
    else if (IS_WEREWOLF(ch) && get_tribe(ch,TRIBE_UKTENA) < 1)
    {
	send_to_char("You require the level 1 power of the Uktena to call the Shroud of Night.\n\r",ch);
	return;
    }

    if (ch->in_room == NULL)
    {
	send_to_char("Nothing happens.\n\r",ch);
	return;
    }

    if (IS_MORE(ch, MORE_DARKNESS))
    {
	REMOVE_BIT(ch->more, MORE_DARKNESS);
	REMOVE_BIT(ch->in_room->added_flags, ROOM2_DARKNESS);
	send_to_char("You banish the darkness from the room!\n\r", ch);
	act("A look of concentration crosses $n's face.", ch, NULL, NULL, TO_ROOM );
	act("$n banishes the darkness from the room!",ch,NULL,NULL,TO_ROOM);
	ch->pcdata->darkness = 60;
	return;
    }

    if (IS_SET(ch->in_room->added_flags, ROOM2_DARKNESS))
    {
	send_to_char("The room is already in darkness.\n\r", ch);
	return;
    }

    if (ch->pcdata->darkness > 0)
    {
	if (ch->pcdata->darkness > 1)
	    sprintf(buf,"You cannot call upon the Shroud of Night for another %li seconds.\n\r",ch->pcdata->darkness);
	else
	    sprintf(buf,"You cannot call upon the Shroud of Night for another 1 second.\n\r");
	send_to_char(buf,ch);
	return;
    }

    SET_BIT(ch->in_room->added_flags, ROOM2_DARKNESS);
    SET_BIT(ch->more, MORE_DARKNESS);
    ch->pcdata->darkness = 60;
    send_to_char("You call forth the Shroud of Night to darken the room!\n\r", ch);
    act("A look of concentration crosses $n's face.", ch, NULL, NULL, TO_ROOM );
    act("The room is filled with darkness!",ch,NULL,NULL,TO_ROOM);
    return;
}  

void do_silencedeath( CHAR_DATA *ch, char *argument )
{
    char buf [MAX_INPUT_LENGTH];

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch))
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if (get_disc(ch,DISC_QUIETUS) < 1)
    {
	send_to_char("You require level 1 Quietus to use Silence of Death.\n\r",ch);
	return;
    }

    if (ch->in_room == NULL)
    {
	send_to_char("Nothing happens.\n\r",ch);
	return;
    }

    if (IS_MORE(ch, MORE_SILENCE))
    {
	REMOVE_BIT(ch->more, MORE_SILENCE);
	REMOVE_BIT(ch->in_room->added_flags, ROOM2_SILENCE);
	send_to_char("You banish the silence from the room!\n\r", ch);
	act("A look of concentration crosses $n's face.", ch, NULL, NULL, TO_ROOM );
	act("The room is no longer silent.",ch,NULL,NULL,TO_ROOM);
	ch->pcdata->silence = 60;
	return;
    }

    if (IS_SET(ch->in_room->added_flags, ROOM2_SILENCE))
    {
	send_to_char("The room is already silent.\n\r", ch);
	return;
    }

    if (ch->pcdata->silence > 0)
    {
	if (ch->pcdata->silence > 1)
	    sprintf(buf,"You cannot call upon the Silence of Death for another %li seconds.\n\r",ch->pcdata->silence);
	else
	    sprintf(buf,"You cannot call upon the Silence of Death for another 1 second.\n\r");
	send_to_char(buf,ch);
	return;
    }

    SET_BIT(ch->in_room->added_flags, ROOM2_SILENCE);
    SET_BIT(ch->more, MORE_SILENCE);
    ch->pcdata->silence = 60;
    send_to_char("You call forth the Silence of Death!\n\r", ch);
    act("A look of concentration crosses $n's face.", ch, NULL, NULL, TO_ROOM );
    act("The room suddenly goes completely silent!",ch,NULL,NULL,TO_ROOM);
    return;
}  

void do_weakness( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    AFFECT_DATA af;
    char arg [MAX_INPUT_LENGTH];
    long sn;

    argument = one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch))
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }
    if (get_disc(ch,DISC_QUIETUS) < 2)
    {
	send_to_char("You require level 2 Quietus to use Weakness.\n\r",ch);
	return;
    }

    if (IS_ITEMAFF(ch, ITEMA_PEACE)) do_majesty(ch,"");

    if (arg[0] == '\0')
    {
	send_to_char("Who do you wish to use Weakness on?\n\r",ch);
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if (ch == victim)
    {
	send_to_char( "Now that would just be plain stupid...\n\r", ch );
	return;
    }

    if ( ( sn = skill_lookup( "weakness" ) ) < 0 )
    {
	send_to_char( "Bug - please inform KaVir.\n\r", ch );
	return;
    }

    if ( is_safe(ch, victim) )
    {
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
	{
	    guardian_message( victim );
	    act("$n just failed to use Weakness on you.",ch,NULL,victim,TO_VICT);
	}
	return;
    }

    act( "A look of concentration crosses $n's face.", ch, NULL, NULL, TO_ROOM );
    send_to_char("You feel weaker!\n\r",victim);
    act("$n looks weaker.",victim,NULL,NULL,TO_ROOM);

    af.type      = sn;
    af.location  = APPLY_STR;
    af.modifier  = 0 - get_disc(ch,DISC_QUIETUS);
    af.duration  = number_range(1,2);
    af.bitvector = 0;
    affect_join( victim, &af );
    WAIT_STATE( ch, 12 );
    if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
    {
	guardian_message( victim );
	act("$n just used Weakness on you.",ch,NULL,victim,TO_VICT);
    }
    return;
}

void do_infirmity( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    AFFECT_DATA af;
    char arg [MAX_INPUT_LENGTH];
    long sn;

    argument = one_argument( argument, arg );

    if (IS_NPC(ch)) return;
    if (!IS_VAMPIRE(ch))
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if (get_disc(ch,DISC_QUIETUS) < 3)
    {
	send_to_char("You require level 3 Quietus to use Infirmity.\n\r",ch);
	return;
    }

    if (IS_ITEMAFF(ch, ITEMA_PEACE)) do_majesty(ch,"");

    if (arg[0] == '\0')
    {
	send_to_char("Who do you wish to use Infirmity on?\n\r",ch);
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if (ch == victim)
    {
	send_to_char( "Now that would just be plain stupid...\n\r", ch );
	return;
    }

    if ( is_safe(ch, victim) )
    {
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
	{
	    guardian_message( victim );
	    act("$n just failed to use Infirmity on you.",ch,NULL,victim,TO_VICT);
	}
	return;
    }

    if ( ( sn = skill_lookup( "weakness" ) ) < 0 )
    {
	send_to_char( "Bug - please inform KaVir.\n\r", ch );
	return;
    }

    act( "A look of concentration crosses $n's face.", ch, NULL, NULL, TO_ROOM );
    send_to_char("You feel weaker!\n\r",victim);
    send_to_char("You feel slower!\n\r",victim);
    send_to_char("You feel less healthy!\n\r",victim);
    act("$n looks weaker.",victim,NULL,NULL,TO_ROOM);
    act("$n looks slower.",victim,NULL,NULL,TO_ROOM);
    act("$n looks less healthy.",victim,NULL,NULL,TO_ROOM);

    af.type      = sn;
    af.location  = APPLY_STR;
    af.modifier  = 0 - get_disc(ch,DISC_QUIETUS);
    af.duration  = number_range(1,2);
    af.bitvector = 0;
    affect_join( victim, &af );

    if ( ( sn = skill_lookup( "slowness" ) ) < 0 )
    {
	send_to_char( "Bug - please inform KaVir.\n\r", ch );
	return;
    }

    af.type      = sn;
    af.location  = APPLY_DEX;
    af.modifier  = 0 - get_disc(ch,DISC_QUIETUS);
    af.duration  = number_range(1,2);
    af.bitvector = 0;
    affect_join( victim, &af );

    if ( ( sn = skill_lookup( "infirmity" ) ) < 0 )
    {
	send_to_char( "Bug - please inform KaVir.\n\r", ch );
	return;
    }

    af.type      = sn;
    af.location  = APPLY_CON;
    af.modifier  = 0 - get_disc(ch,DISC_QUIETUS);
    af.duration  = number_range(1,2);
    af.bitvector = 0;
    affect_join( victim, &af );
    WAIT_STATE( ch, 15 );
    if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
    {
	guardian_message( victim );
	act("$n just used Infirmity on you.",ch,NULL,victim,TO_VICT);
    }
    return;
}

void do_bloodagony( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA  *obj;
    char arg [MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch))
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if (get_disc(ch,DISC_QUIETUS) < 4)
    {
	send_to_char("You require level 4 Quietus to use Blood Agony.\n\r",ch);
	return;
    }

    if ( ch->blood[BLOOD_CURRENT] < 10 )
    {
	send_to_char("You have insufficient blood.\n\r",ch);
	return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char( "What weapon do you wish to use Blood Agony on?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
	send_to_char( "You are not carrying that object.\n\r", ch );
	return;
    }

    if ( obj->item_type != ITEM_WEAPON )
    {
	send_to_char( "You may only use Blood Agony on a weapon.\n\r", ch );
	return;
    }

    if ( IS_SET(obj->quest, QUEST_AGGRAVATED)
    ||   IS_SET(obj->quest, QUEST_ARTIFACT)
    ||   IS_SET(obj->quest, QUEST_RELIC)
    ||   obj->chobj != NULL )
    {
	send_to_char("You are unable to smear any blood on this weapon.\n\r",ch);
	return;
    }

    act("You cut open your wrist and smear some blood onto $p.",ch,obj,NULL,TO_CHAR);
    act("$n cuts open $s wrist and smears some blood onto $p.",ch,obj,NULL,TO_ROOM);
    SET_BIT(obj->quest, QUEST_AGGRAVATED);
    ch->blood[BLOOD_CURRENT] -= 10;

    return;
}

void do_spit( CHAR_DATA *ch, char *argument )
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

    if (get_disc(ch,DISC_QUIETUS) < 5)
    {
	send_to_char("You require level 5 Quietus to use Taste of Death.\n\r",ch);
	return;
    }

    if (IS_ITEMAFF(ch, ITEMA_PEACE)) do_majesty(ch,"");

    if (arg[0] == '\0' && ch->fighting == NULL)
    {
	send_to_char("Who do you wish to use Taste of Death on?\n\r",ch);
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
	    act("$n just failed to use Taste of Death on you.",ch,NULL,victim,TO_VICT);
	}
	return;
    }

    WAIT_STATE( ch, 6 );

    one_hit( ch, victim, TYPE_HIT, "spit" );
    return;
}

void do_shadowbody( CHAR_DATA *ch, char *argument )
{
    char buf [MAX_STRING_LENGTH]; 

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch))
    {
	send_to_char("Huh?\n\r",ch);
	return;
    }

    if (get_disc(ch,DISC_OBTENEBRATION) < 3)
    {
	send_to_char("You require level 3 Obtenebration to transform into your Shadow Body.\n\r",ch);
	return;
    }

	if(ch->fight_timer > 0)
	{
	stc("Not with a fight timer.\n\r", ch);
	return;
	}
    if (IS_AFFECTED(ch, AFF_POLYMORPH))
    {
	if (IS_POLYAFF(ch, POLY_SHADOW))
	{
	    REMOVE_BIT(ch->polyaff, POLY_SHADOW);
	    REMOVE_BIT(ch->affected_by, AFF_ETHEREAL);
	}
	else
	{
	    send_to_char( "You seem to be stuck in this form.\n\r", ch );
	    return;
	}
	act( "You transform into human form.", ch, NULL, NULL, TO_CHAR );
	act( "$n transforms into human form.", ch, NULL, NULL, TO_ROOM );
	REMOVE_BIT(ch->affected_by, AFF_POLYMORPH);
	clear_stats(ch);
  	free_string( ch->morph );
    	ch->morph = str_dup( "" );
    }
    else
    {
	if (ch->stance[0] != -1) do_stance(ch,"");
	act( "You transform into a pool of shadow.", ch, NULL, NULL, TO_CHAR );
	act( "$n transforms into a pool of shadow.", ch, NULL, NULL, TO_ROOM );
	if (IS_EXTRA(ch, TIED_UP))
	{
	    act("The ropes binding you fall through your ethereal form.",ch,NULL,NULL,TO_CHAR);
	    act("The ropes binding $n fall through $s ethereal form.",ch,NULL,NULL,TO_ROOM);
	    REMOVE_BIT(ch->extra, TIED_UP);
	    REMOVE_BIT(ch->extra, GAGGED);
	    REMOVE_BIT(ch->extra, BLINDFOLDED);
	}
	if (is_affected(ch, gsn_web))
	{
	    act("The webbing entrapping $n falls through $s ethereal form.",ch,NULL,NULL,TO_ROOM);
	    send_to_char("The webbing entrapping you falls through your ethereal form.\n\r",ch);
	    affect_strip(ch, gsn_web);
	}
	SET_BIT(ch->polyaff, POLY_SHADOW);
	clear_stats(ch);
	SET_BIT(ch->affected_by, AFF_POLYMORPH);
	SET_BIT(ch->affected_by, AFF_ETHEREAL);
	sprintf(buf, "A pool of shadow");
    	free_string( ch->morph );
    	ch->morph = str_dup( buf );
    }
    return;
}

void do_arms( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg [MAX_INPUT_LENGTH];
    long sn;
    long level;
    long spelltype;

    argument = one_argument( argument, arg );

    if (IS_NPC(ch)) return;
    if (!IS_VAMPIRE(ch))
    {
	send_to_char("Huh?\n\r",ch);
	return;
    }

    if (get_disc(ch,DISC_OBTENEBRATION) < 2)
    {
	send_to_char("You require level 2 Obtenebration to use Arms of the Abyss.\n\r",ch);
	return;
    }

    if (IS_ITEMAFF(ch, ITEMA_PEACE)) do_majesty(ch,"");

    if (arg[0] == '\0' && ch->fighting == NULL)
    {
	send_to_char("Who do you wish to use the Arms of the Abyss on?\n\r",ch);
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

    if ( ch == victim )
    {
	if (is_affected(ch, gsn_tendrils))
	{
	    act("The tendrils of darkness entrapping $n fall away.",ch,NULL,NULL,TO_ROOM);
	    send_to_char("The tendrils of darkness entrapping you fall away.\n\r",ch);
	    affect_strip(ch, gsn_tendrils);
	}
	else if (IS_AFFECTED(ch, AFF_SHADOW))
	{
	    act("The tendrils of darkness entrapping $n fall away.",ch,NULL,NULL,TO_ROOM);
	    send_to_char("The tendrils of darkness entrapping you fall away.\n\r",ch);
	    REMOVE_BIT(ch->affected_by, AFF_SHADOW);
	}
	else send_to_char( "Nothing happens.\n\r", ch );
	return;
    }

    if ( IS_AFFECTED(victim, AFF_ETHEREAL) )
    {
	send_to_char( "You cannot use Arms of the Abyss on an ethereal person.\n\r", ch );
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
	{
	    guardian_message( victim );
	    act("$n just failed to use Arms of the Abyss on you.",ch,NULL,victim,TO_VICT);
	}
	return;
    }

    if (is_safe(ch,victim))
    {
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
	{
	    guardian_message( victim );
	    act("$n just failed to use Arms of the Abyss on you.",ch,NULL,victim,TO_VICT);
	}
	return;
    }

    if ( ( sn = skill_lookup( "tendrils" ) ) < 0 ) return;
    spelltype = skill_table[sn].target;
    level = get_disc(ch,DISC_OBTENEBRATION) * 5;
    (*skill_table[sn].spell_fun) ( sn, level, ch, victim );
    WAIT_STATE( ch, 12 );
    return;
}

void do_lamprey( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MIL];
    long level, dam;
    long blood = number_range(1, 10);

    argument = one_argument(argument, arg);

    if (!IS_VAMPIRE(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (get_disc(ch, DISC_OBTENEBRATION) < 4)
    {
	stc("You require level 4 Obtenebration to Call the Lamprey.\n\r", ch);
	return;
    }

    if (IS_ITEMAFF(ch, ITEMA_PEACE)) do_majesty(ch, "");

    if (arg[0] == '\0' && ch->fighting == NULL)
    {
	stc("Who do you wish to Call the Lamprey on?\n\r", ch);
	return;
    }

    if ((victim = ch->fighting) == NULL)
    {
	if ((victim = get_char_room(ch, arg)) == NULL)
	{
	    stc("They aren't here.\n\r", ch);
	    return;
	}
    }

    if (ch == victim)
    {
	stc("That would not be a good idea.\n\r", ch);
	return;
    }

    if (is_safe(ch, victim)) return;

    level = get_disc(ch, DISC_OBTENEBRATION);
    dam = blood * level * level * 3;
    damage(ch, victim, dam, TYPE_HIT, "tendrils");

    if (victim == NULL || victim->position == POS_DEAD) return;

    if (victim->blood[BLOOD_CURRENT] < blood) blood = victim->blood[BLOOD_CURRENT];

    victim->blood[BLOOD_CURRENT] -= blood;
    ch->blood[BLOOD_CURRENT] += blood;
    if (ch->blood[BLOOD_CURRENT] >= ch->blood[BLOOD_POOL])
    {
	ch->blood[BLOOD_CURRENT] = ch->blood[BLOOD_POOL];
	stc("Your blood lust is sated.\n\r", ch);
    }
    if (!IS_VAMPIRE(victim) && victim->blood[0] < 1) victim->blood[BLOOD_CURRENT] = 1;

    WAIT_STATE(ch, 12);
    return;
}

void do_thanatopsis( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    ROOM_INDEX_DATA *pRoomIndex;
    ROOM_INDEX_DATA *location;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];

    one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch))
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if (get_disc(ch,DISC_NECROMANCY) < 1)
    {
	send_to_char("You require level 1 Necromancy to perform Thanatopsis on a corpse.\n\r",ch);
	return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char( "What do you wish to perform Thanatopsis on?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) == NULL )
    {
	send_to_char( "You cannot seem to find that object.\n\r", ch );
	return;
    }

    if (obj->item_type != ITEM_CORPSE_NPC && obj->item_type != ITEM_CORPSE_PC)
    {
	send_to_char( "That isn't a corpse.\n\r", ch );
	return;
    }

    act("You examine $p carefully.",ch,obj,NULL,TO_CHAR);
    act("$n examines $p carefully.",ch,obj,NULL,TO_ROOM);

    if (strlen(obj->questmaker) < 1 || strlen(obj->questowner) < 1)
    {
	send_to_char( "You are unable to discover anything about the killer.\n\r", ch );
	return;
    }

    sprintf(buf,"It seems that %s was killed by %s.\n\r",obj->questowner,obj->questmaker);
    send_to_char(buf,ch);

    if ( ( location = ch->in_room ) == NULL )
    {
	send_to_char("Bug - Please inform KaVir.\n\r",ch);
	return;
    }
    if ( ( pRoomIndex = get_room_index(obj->specpower) ) == NULL )
    {
	sprintf(buf,"You are unsure where %s was killed.\n\r",obj->questowner);
	send_to_char(buf,ch);
    }
    else
    {
	if (location == pRoomIndex)
	    sprintf(buf,"It seems that %s was killed right here.\n\r", obj->questowner);
	else
	{
	    char_from_room(ch);
	    char_to_room(ch,pRoomIndex);
	    sprintf(buf,"It seems that %s was killed at %s.\n\r", obj->questowner, ch->in_room->name);
	    char_from_room(ch);
	    char_to_room(ch,location);
	}
	send_to_char(buf,ch);
    }

    return;
}

void do_spiritgate( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA  *corpse;
    OBJ_DATA  *obj;
    ROOM_INDEX_DATA *pRoomIndex;
    ROOM_INDEX_DATA *location;
    char       arg [MAX_INPUT_LENGTH];
    long        duration, destination;

    one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch))
    {
	send_to_char("Huh?\n\r",ch);
	return;
    }

    if (get_disc(ch,DISC_NECROMANCY) < 3)
    {
	send_to_char("You require level 3 Necromancy to create a Spirit Gate.\n\r",ch);
	return;
    }

    if ( arg[0] == '\0')
    {
	send_to_char( "Which corpse do you wish to use to make a Spirit Gate?\n\r", ch );
	return;
    }

    if ( ( corpse = get_obj_here( ch, arg ) ) == NULL )
    {
	send_to_char( "You cannot seem to find that object.\n\r", ch );
	return;
    }

    if (corpse->item_type == ITEM_CORPSE_PC)
    {
	send_to_char( "Not on that type of corpse.\n\r", ch );
	return;
    }

    if (corpse->item_type != ITEM_CORPSE_NPC)
    {
	send_to_char( "That isn't a corpse.\n\r", ch );
	return;
    }

    duration = number_range(2,3) * 15;

    if ( ( location = ch->in_room ) == NULL )
    {
	send_to_char("Bug - Please inform KaVir.\n\r",ch);
	return;
    }
    if ( ( pRoomIndex = get_room_index(corpse->specpower) ) == NULL )
    {
	send_to_char("Nothing happens.\n\r",ch);
	return;
    }
    if (location == pRoomIndex)
    {
	send_to_char("Nothing happens.\n\r",ch);
	return;
    }

    destination = ch->in_room->vnum;

    obj = create_object( get_obj_index( OBJ_VNUM_PORTAL ), 0 );
    obj->value[0] = corpse->specpower;
    obj->value[3] = destination;
    obj->timer = duration;
    free_string(obj->name);
    obj->name = str_dup("spirit gate portal");
    free_string(obj->short_descr);
    obj->short_descr = str_dup("a spirit gate");
    free_string(obj->description);
    obj->description = str_dup("A glowing spirit gate floats here.");
    if (IS_AFFECTED(ch,AFF_SHADOWPLANE))
	obj->extra_flags = ITEM_SHADOWPLANE;
    obj_to_room( obj, location );
    act( "$p shimmers into existance in front of $n.", ch, obj, NULL, TO_ROOM );
    act( "$p shimmers into existance in front of you.", ch, obj, NULL, TO_CHAR );

    char_from_room(ch);
    char_to_room(ch,pRoomIndex);

    obj = create_object( get_obj_index( OBJ_VNUM_PORTAL ), 0 );
    obj->value[0] = destination;
    obj->value[3] = corpse->specpower;
    obj->timer = duration;
    free_string(obj->name);
    obj->name = str_dup("spirit gate portal");
    free_string(obj->short_descr);
    obj->short_descr = str_dup("a spirit gate");
    free_string(obj->description);
    obj->description = str_dup("A glowing spirit gate floats here.");
    if (IS_AFFECTED(ch,AFF_SHADOWPLANE))
	obj->extra_flags = ITEM_SHADOWPLANE;
    obj_to_room( obj, ch->in_room );
    act( "$p shimmers into existance.", ch, obj, NULL, TO_ROOM );

    char_from_room(ch);
    char_to_room(ch,location);

    act("$p crumbles to dust.",ch,corpse,NULL,TO_CHAR);
    act("$n crumbles to dust.",ch,corpse,NULL,TO_ROOM);
    extract_obj(corpse);
    return;
}

void do_preservation( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];

    one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch))
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if (get_disc(ch,DISC_NECROMANCY) < 2)
    {
	send_to_char("You require level 2 Necromancy to perform Preservation.\n\r",ch);
	return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char( "What do you wish to perform Preservation on?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) == NULL )
    {
	send_to_char( "You cannot seem to find that object.\n\r", ch );
	return;
    }

    if ( obj->chobj != NULL )
    {
	send_to_char( "You are unable to preserve it.\n\r", ch );
	return;
    }

    if (obj->item_type == ITEM_CORPSE_PC)
	send_to_char( "You cannot preserve that type of corpse.\n\r", ch );
    else if (obj->item_type == ITEM_FOOD)
    {
	act("$p shrivels up.",ch,obj,NULL,TO_CHAR);
	act("$p shrivels up.",ch,obj,NULL,TO_ROOM);
	if (obj->value[0] > 1) obj->value[0] *= 0.5;
	ch->carry_weight -= obj->weight;
	if (obj->weight > 1) obj->weight *= 0.5;
	ch->carry_weight += obj->weight;
	obj->timer = -1;
    }
    else if (obj->item_type == ITEM_CORPSE_NPC)
    {
	if (strlen(obj->questowner) < 1)
	{
	    act("You are unable to preserve $p.",ch,obj,NULL,TO_CHAR);
	    return;
	}

	if (obj->timer < 1)
	{
	    send_to_char( "It is already preserved.\n\r", ch );
	    return;
	}
	act("$p shrivels up.",ch,obj,NULL,TO_CHAR);
	act("$p shrivels up.",ch,obj,NULL,TO_ROOM);

	free_string(obj->short_descr);
	sprintf(buf, "the shriveled up corpse of %s",obj->questowner);
	obj->short_descr = str_dup(buf);

	free_string(obj->description);
	sprintf(buf, "The shriveled up corpse of %s is lying here.",obj->questowner);
	obj->description = str_dup(buf);

	ch->carry_weight -= obj->weight;
	if (obj->weight > 1) obj->weight *= 0.5;
	ch->carry_weight += obj->weight;
	obj->timer = -1;
    }
    else
	send_to_char( "You cannot preserve that object.\n\r", ch );
    return;
}

void do_zombie( CHAR_DATA *ch, char *argument )
{
    char arg [MAX_INPUT_LENGTH];
    char buf [MAX_STRING_LENGTH];
    OBJ_DATA		*obj;
    OBJ_DATA		*obj_content;
    OBJ_DATA		*obj_next;
    CHAR_DATA		*victim;
    MOB_INDEX_DATA	*pMobIndex;
    AFFECT_DATA af;
    long sn;

    one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch))
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if (IS_VAMPIRE(ch) && get_disc(ch,DISC_NECROMANCY) < 5)
    {
	send_to_char("You require level 5 Necromancy to transform a corpse into a Zombie.\n\r",ch);
	return;
    }

    if (ch->pcdata->necromancy > 0)
    {
	if (ch->pcdata->necromancy > 1)
	    sprintf(buf,"You cannot animate a Zombie for another %li seconds.\n\r",ch->pcdata->necromancy);
	else
	    sprintf(buf,"You cannot animate a Zombie for another 1 second.\n\r");
	send_to_char(buf,ch);
	return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char( "What do you wish to animate as a Zombie?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) == NULL )
    {
	send_to_char( "You cannot seem to find that object.\n\r", ch );
	return;
    }

    if (obj->item_type != ITEM_CORPSE_NPC)
    {
	send_to_char( "You cannot animate that as a Zombie.\n\r", ch );
	return;
    }

    if (obj->in_room == NULL)
    {
	send_to_char( "You better drop it first...don't want the little bugger animating in your pants.\n\r", ch );
	return;
    }

    act( "A look of concentration crosses $n's face.", ch, NULL, NULL, TO_ROOM );

    if ( ( pMobIndex = get_mob_index( obj->value[2] ) ) == NULL )
    {
	send_to_char( "Nothing happens.\n\r", ch );
	return;
    }

    victim = create_mobile( pMobIndex );

    if (victim->level > 200) victim->level = 200;
    victim->hitroll = victim->level;
    victim->damroll = victim->level;

    char_to_room(victim,ch->in_room);
    sprintf(buf,"The zombie of %s stands here.\n\r",victim->short_descr);
    free_string(victim->long_descr);
    victim->long_descr = str_dup(buf);
    sprintf(buf,"zombie %s",victim->name);
    free_string(victim->name);
    victim->name = str_dup(buf);
    sprintf(buf,"the zombie of %s",victim->short_descr);
    free_string(victim->short_descr);
    victim->short_descr = str_dup(buf);
    act("$n clambers to $s feet.",victim,NULL,NULL,TO_ROOM);
    for ( obj_content = obj->contains; obj_content != NULL; obj_content = obj_next )
    {
	obj_next = obj_content->next_content;
	obj_from_obj(obj_content);
	obj_to_char(obj_content,victim);
    }
    extract_obj(obj);
    do_wear(victim,"all");

    ch->pcdata->necromancy = 60;

    if (ch->pcdata->followers > 4) return;

    free_string(victim->lord);
    victim->lord = str_dup(ch->name);

    ch->pcdata->followers++;

    if ( ( sn = skill_lookup( "charm person" ) ) < 0 )
    {
	send_to_char( "Bug - please inform KaVir.\n\r", ch );
	return;
    }

    add_follower( victim, ch );
    af.type      = sn;
    af.duration  = 5000;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char( victim, &af );

    return;
}

void do_spiritform( CHAR_DATA *ch, char *argument )
{
    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch))
    {
	send_to_char("Huh?\n\r",ch);
	return;
    }

    if (get_disc(ch,DISC_PROTEAN) < 5)
    {
	send_to_char("You require level 5 Protean to transform into Spirit Form.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(ch, AFF_POLYMORPH))
    {
	if (!IS_POLYAFF(ch,POLY_SPIRIT))
	{
	    send_to_char( "You cannot change from this form.\n\r", ch );
	    return;
	}
	act( "Your body regains its substance.", ch, NULL, NULL, TO_CHAR );
	act( "$n's ethereal body regains its substance.", ch, NULL, NULL, TO_ROOM );
	REMOVE_BIT(ch->affected_by, AFF_POLYMORPH);
	REMOVE_BIT(ch->affected_by, AFF_ETHEREAL);
	REMOVE_BIT(ch->polyaff, POLY_SPIRIT);
	REMOVE_BIT(ch->vampaff, VAM_DISGUISED);
	REMOVE_BIT(ch->vampaff, VAM_FLYING);
	free_string( ch->morph );
	ch->morph = str_dup( "" );
	return;
    }
    act( "You body becomes translucent and insubstancial.", ch, NULL, NULL, TO_CHAR );
    act( "$n's body becomes translucent and insubstancial.", ch, NULL, NULL, TO_ROOM );
    if (IS_EXTRA(ch, TIED_UP))
    {
	act("The ropes binding you fall through your ethereal form.",ch,NULL,NULL,TO_CHAR);
	act("The ropes binding $n fall through $s ethereal form.",ch,NULL,NULL,TO_ROOM);
	REMOVE_BIT(ch->extra, TIED_UP);
	REMOVE_BIT(ch->extra, GAGGED);
	REMOVE_BIT(ch->extra, BLINDFOLDED);
    }
    if (is_affected(ch, gsn_web))
    {
	act("The webbing entrapping $n falls through $s ethereal form.",ch,NULL,NULL,TO_ROOM);
	send_to_char("The webbing entrapping you falls through your ethereal form.\n\r",ch);
	affect_strip(ch, gsn_web);
    }
    if (IS_AFFECTED(ch, AFF_WEBBED))
    {
	act("The webbing entrapping $n falls through $s ethereal form.",ch,NULL,NULL,TO_ROOM);
	send_to_char("The webbing entrapping you falls through your ethereal form.\n\r",ch);
	REMOVE_BIT(ch->affected_by, AFF_WEBBED);
    }
    if (is_affected(ch, gsn_tendrils))
    {
	act("The tendrils of darkness entrapping $n fall through $s ethereal form.",ch,NULL,NULL,TO_ROOM);
	send_to_char("The tendrils of darkness entrapping you fall through your ethereal form.\n\r",ch);
	affect_strip(ch, gsn_tendrils);
    }
    if (IS_AFFECTED(ch, AFF_SHADOW))
    {
	act("The tendrils of darkness entrapping $n fall through $s ethereal form.",ch,NULL,NULL,TO_ROOM);
	send_to_char("The tendrils of darkness entrapping you fall through your ethereal form.\n\r",ch);
	REMOVE_BIT(ch->affected_by, AFF_SHADOW);
    }
    SET_BIT(ch->affected_by, AFF_POLYMORPH);
    SET_BIT(ch->affected_by, AFF_ETHEREAL);
    SET_BIT(ch->polyaff, POLY_SPIRIT);
    SET_BIT(ch->vampaff, VAM_DISGUISED);
    SET_BIT(ch->vampaff, VAM_FLYING);
    free_string( ch->morph );
    ch->morph = str_dup( ch->name );
    return;
}

void do_spiritguardian( CHAR_DATA *ch, char *argument )
{
    if (!IS_VAMPIRE(ch) && !IS_WEREWOLF(ch) && !IS_MONK(ch))
    {
	stc("Huh?\n\r",ch);
	return;
    }

    if (IS_VAMPIRE(ch) && get_disc(ch,DISC_NECROMANCY) < 4)
    {
	stc("You require level 4 Necromancy to summon your Spirit Guardian.\n\r", ch);
	return;
    }
    else if (IS_WEREWOLF(ch) && get_tribe(ch,TRIBE_CHILDREN_OF_GAIA) < 4)
    {
	stc("You require a Children of Gaia of 4 to summon your Spirit Guardian.\n\r", ch);
	return;
    }
    else if (IS_MONK(ch) && get_monk(ch, MONK_MANTRA) < 6)
    {
	stc("You need Mantra 6 for Spirit Guardian.\n\r", ch);
	return;
    }

    if (IS_MORE(ch, MORE_GUARDIAN))
    {
	stc("You dismiss your Spirit Guardian.\n\r", ch);
	guardian_message(ch);
	stc("I shall return when you call me, Master.\n\r", ch);
	REMOVE_BIT(ch->more, MORE_GUARDIAN);
	return;
    }
    stc("You call your Spirit Guardian.\n\r", ch);
    guardian_message(ch);
    send_to_char("I have arrived, Master.\n\r", ch);
    SET_BIT(ch->more, MORE_GUARDIAN);
    return;
}

void do_succulence( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );

    if (!IS_VAMPIRE(ch))
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if (get_disc(ch,DISC_ANIMALISM) < 5)
    {
	send_to_char("You require level 5 Animalism to use Succulence.\n\r",ch);
	return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char( "Who do you wish to use Succulence on?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch == victim )
    {
	send_to_char( "Not on yourself.\n\r", ch );
	return;
    }

    if (!IS_NPC(victim))
    {
	send_to_char( "Not on players.\n\r", ch );
	return;
    }

    if (IS_AFFECTED(victim, AFF_SUCCULENCE))
    {
	stc("Their blood has already been improved.\n\r", ch);
	return;
    }

    victim->blood[BLOOD_POOL] *= 2;
    victim->blood[BLOOD_CURRENT] *= 2;
    SET_BIT(victim->affected_by, AFF_SUCCULENCE);

    act( "A look of concentration crosses $n's face.", ch, NULL, NULL, TO_ROOM );
    send_to_char("You increase the quality of their blood.\n\r",ch);
    return;
}

void do_frenzy( CHAR_DATA *ch, char *argument )
{
    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch))
    {
	send_to_char("Huh?\n\r",ch);
	return;
    }

    if (get_disc(ch,DISC_ANIMALISM) < 6)
    {
	send_to_char("You require level 6 Animalism to use Frenzy Trigger.\n\r",ch);
	return;
    }

    if ( ch->pcdata->wolf > 0 )
    {
	send_to_char("You take a deep breath and force back your inner beast.\n\r",ch);
	act("$n takes a deep breath and forces back $s inner beast.",ch,NULL,NULL,TO_ROOM);
	ch->pcdata->wolf = 0;
	if (IS_VAMPAFF(ch, VAM_NIGHTSIGHT) && get_disc(ch,DISC_PROTEAN) > 0) 
	    do_nightsight(ch,"");
	if (IS_VAMPAFF(ch, VAM_FANGS)) do_fangs(ch,"");
	if (IS_VAMPAFF(ch, VAM_CLAWS) && get_disc(ch,DISC_PROTEAN) > 1) 
	    do_claws(ch,"");
	WAIT_STATE(ch,12);
	return;
    }

    if (!IS_VAMPAFF(ch, VAM_NIGHTSIGHT) && 
	get_disc(ch,DISC_PROTEAN) > 0) 
	do_nightsight(ch,"");
    if (!IS_VAMPAFF(ch, VAM_FANGS)) do_fangs(ch,"");
    if (!IS_VAMPAFF(ch, VAM_CLAWS) && 
	get_disc(ch,DISC_PROTEAN) > 1) 
	do_claws(ch,"");
    send_to_char("You bare your fangs and growl as your inner beast consumes you.\n\r",ch);
    act("$n bares $s fangs and growls as $s inner beast consumes $m.",ch,NULL,NULL,TO_ROOM);
    ch->pcdata->wolf += number_range(50,100);
    WAIT_STATE(ch,12);
    return;
}

void do_covenant( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    CHAR_DATA *gch;
    char      arg [MAX_INPUT_LENGTH];
    char      buf [MAX_STRING_LENGTH];
    char      *strtime;

    /* Remove the first 'word' from the entered string, and copy it into 
     * arg.
     */
    argument = one_argument( argument, arg );

    /* Removes the tildes from argument for file-saving purposes. */
    smash_tilde( argument );

    /* This command cannot be used by Non-Player Characters (mobiles). */
    if (IS_NPC(ch)) return;

    if ( arg[0] == '\0' || !str_cmp(arg, "info") )
    {
	/* If they person belongs to a covenant, then this should list all 
         * the covenant members.
         */
	if (strlen(ch->pcdata->covenant) > 1 && strlen(ch->pcdata->vleader) > 1 &&
	    strlen(ch->pcdata->cov_time) > 1)
	{
	    /* Show the name of the covenant, and who founded it. */
	    sprintf( buf, "The %s covenant, lead by %s:\n\r", 
		ch->pcdata->covenant, ch->pcdata->vleader );
	    send_to_char( buf, ch );
	    send_to_char("--------------------------------------------------------------------------------\n\r",ch);
	    send_to_char("[      Name      ] [  Rank  ] [ Hits ] [ Mana ] [ Status ] [ PK Score ]\n\r",ch );

	    /* Check each player in the char_list for a match. */
	    for ( gch = char_list; gch != NULL; gch = gch->next )
	    {
		if ( IS_NPC(gch) ) continue;
		if ( strlen(gch->pcdata->covenant) < 2) continue;
		if ( strlen(gch->pcdata->cov_time) < 2) continue;
		if ( strlen(gch->pcdata->vleader) < 2) continue;
		if ( str_cmp(ch->pcdata->vleader,gch->pcdata->vleader) ) continue;
		if ( str_cmp(ch->pcdata->cov_time,gch->pcdata->cov_time) ) continue;
		if ( !str_cmp(ch->pcdata->covenant,gch->pcdata->covenant) )
		{
		    sprintf( buf,
		    "[%-16s] [%8s] [%-6li] [%-6li] [   %-3li  ] [   %-4li   ]\n\r",
			capitalize( gch->name ),
			IS_MORE(gch, MORE_LEADER) ? " Leader " : 
			IS_MORE(gch, MORE_CHAMPION) ? "Champion" : " Member ",
			gch->hit, gch->mana, gch->status, get_ratio(gch));
			send_to_char( buf, ch );
		}
	    }
	    send_to_char("--------------------------------------------------------------------------------\n\r",ch);
	    send_to_char( "For more information type 'covenant help'.\n\r", ch );
	    send_to_char("--------------------------------------------------------------------------------\n\r",ch);
	    return;
	}
	send_to_char("--------------------------------------------------------------------------------\n\r",ch);
	send_to_char("                              -= Covenant Options =-\n\r",ch);
	send_to_char("--------------------------------------------------------------------------------\n\r",ch);
	send_to_char("CHAMPION : Make a member into a champion.\n\r",ch);
	send_to_char("FOUND    : Founds a new Covenant.\n\r",ch);
	send_to_char("INDUCT   : Inducts a member into your covenant.\n\r",ch);
	send_to_char("INFO     : Lists information on your covenant.\n\r",ch);
	send_to_char("LEAVE    : Leaves your current Covenant.\n\r",ch);
	send_to_char("OUTCAST  : Outcasts a member from your covenant.\n\r",ch);
	send_to_char("--------------------------------------------------------------------------------\n\r",ch);
	return;
    }

    if ( !str_cmp(arg, "leave") )
    {
	if (strlen(ch->pcdata->covenant) < 2 || 
	    strlen(ch->pcdata->cov_time) < 2 || 
	    strlen(ch->pcdata->vleader) < 2)
	{
	    send_to_char( "But you are not a member of any covenant!\n\r", ch );
	    return;
	}

	/* Check each player in the char_list for a match. */
	for ( gch = char_list; gch != NULL; gch = gch->next )
	{
	    if ( IS_NPC(gch) ) continue;
	    if ( strlen(gch->pcdata->covenant) < 2) continue;
	    if ( strlen(gch->pcdata->vleader) < 2) continue;
	    if ( strlen(gch->pcdata->cov_time) < 2) continue;
	    if ( ch == gch ) continue;
	    if ( str_cmp(ch->pcdata->vleader,gch->pcdata->vleader) ) continue;
	    if ( str_cmp(ch->pcdata->cov_time,gch->pcdata->cov_time) ) continue;
	    if ( !str_cmp(ch->pcdata->covenant,gch->pcdata->covenant) )
	    {
		if (IS_MORE(ch, MORE_LEADER))
		{
		    act( "Info -> $n has disbanded the covenant.", ch, NULL, gch, TO_VICT );
		    free_string(gch->pcdata->covenant);
		    gch->pcdata->covenant = str_dup( "" );
		    free_string(gch->pcdata->vleader);
		    gch->pcdata->vleader = str_dup( "" );
		}
		else
		    act( "Info -> $n has left the covenant.", ch, NULL, gch, TO_VICT );
	    }
	}
	if (IS_MORE(ch, MORE_LEADER))
	    send_to_char("You disband the covenant.\n\r",ch);
	else
	    send_to_char("You leave the covenant.\n\r",ch);
	REMOVE_BIT(ch->more, MORE_CHAMPION);
	free_string(ch->pcdata->covenant);
	ch->pcdata->covenant = str_dup( "" );
	free_string(ch->pcdata->vleader);
	ch->pcdata->vleader = str_dup( "" );
	REMOVE_BIT(ch->more, MORE_LEADER);
	return;
    }

    if ( !str_cmp(arg, "found") )
    {
	if (strlen(ch->pcdata->covenant) > 1)
	{
	    send_to_char( "But you are already the member of a covenant!\n\r", ch );
	    return;
	}
	if (argument[0] == '\0')
	{
	    send_to_char( "What do you wish to call your covenant?\n\r", ch );
	    return;
	}
	if (strlen(argument) < 5 || strlen(argument) > 50 )
	{
	    send_to_char( "Your covenant name should be between 5 and 50 characters long.\n\r", ch );
	    return;
	}
	strtime = ctime( &current_time );
	strtime[strlen(strtime)-1] = '\0';
	free_string(ch->pcdata->cov_time);
	ch->pcdata->cov_time = str_dup( strtime );

	send_to_char("You have founded a new covenant!\n\r",ch);
	free_string(ch->pcdata->covenant);
	ch->pcdata->covenant = str_dup( argument );
	free_string(ch->pcdata->vleader);
	ch->pcdata->vleader = str_dup( ch->name );
	SET_BIT(ch->more, MORE_LEADER);
	SET_BIT(ch->more, MORE_CHAMPION);
	send_to_char( "Your covenant is now ready to accept new members!\n\r", ch );
	return;
    }

    if ( !str_cmp(arg, "induct") )
    {
	if (strlen(ch->pcdata->covenant) < 2 || 
	    strlen(ch->pcdata->cov_time) < 2 || 
	    strlen(ch->pcdata->vleader) < 2)
	{
	    send_to_char( "But you are not a member of any covenant!\n\r", ch );
	    return;
	}
	if (!IS_MORE(ch, MORE_LEADER) && !IS_MORE(ch, MORE_CHAMPION))
	{
	    send_to_char( "Only a covenant leader or champion can induct new members.\n\r",ch );
	    return;
	}
	if (!IS_MORE(ch, MORE_CHAMPION) && IS_MORE(ch, MORE_LEADER))
	{
	    send_to_char( "Your covenant has not yet been completely set up.\n\r", ch );
	    return;
	}
	if (argument[0] == '\0')
	{
	    send_to_char( "Who do you wish to induct?\n\r", ch );
	    return;
	}
	if ( ( victim = get_char_room( ch, argument ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	if (IS_NPC(victim))
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( strlen(victim->pcdata->covenant) > 1 && 
	     strlen(victim->pcdata->vleader) > 1 &&
	     strlen(victim->pcdata->cov_time) > 1 &&
	    !str_cmp(ch->pcdata->covenant,victim->pcdata->covenant) &&
	    !str_cmp(ch->pcdata->cov_time,victim->pcdata->cov_time) &&
	    !str_cmp(ch->pcdata->vleader, victim->pcdata->vleader) )
	{
	    send_to_char( "They are already a member of your covenant.\n\r", ch );
	    return;
	}

	if (strlen(victim->pcdata->covenant) > 1)
	{
	    send_to_char( "You cannot induct an unwilling person.\n\r", ch );
	    return;
	}
	if (!IS_EXTRA(victim,EXTRA_VAMPIRE))
	{
	    send_to_char( "You cannot induct an unwilling person.\n\r", ch );
	    return;
	}

	act("You induct $N into your covenant.",ch,NULL,victim,TO_CHAR);
	act("$n inducts you into $s covenant.",ch,NULL,victim,TO_VICT);

	/* Check each player in the char_list for a match. */
	for ( gch = char_list; gch != NULL; gch = gch->next )
	{
	    if ( IS_NPC(gch) ) continue;
	    if ( strlen(gch->pcdata->covenant) < 2) continue;
	    if ( strlen(gch->pcdata->cov_time) < 2) continue;
	    if ( strlen(gch->pcdata->vleader) < 2) continue;
	    if ( str_cmp(ch->pcdata->vleader,gch->pcdata->vleader) ) continue;
	    if ( str_cmp(ch->pcdata->cov_time,gch->pcdata->cov_time) ) continue;
	    if ( !str_cmp(ch->pcdata->covenant,gch->pcdata->covenant) )
		act( "Info -> $N has been inducted to the covenant!", gch, NULL, victim, TO_CHAR );
	}

	free_string(victim->pcdata->covenant);
	victim->pcdata->covenant = str_dup( ch->pcdata->covenant );
	free_string(victim->pcdata->cov_time);
	victim->pcdata->cov_time = str_dup( ch->pcdata->cov_time );
	free_string(victim->pcdata->vleader);
	victim->pcdata->vleader = str_dup( ch->pcdata->vleader );
	REMOVE_BIT(victim->more, MORE_CHAMPION);
	REMOVE_BIT(victim->more, MORE_LEADER);
	return;
    }
    if ( !str_cmp(arg, "outcast") )
    {
	if (strlen(ch->pcdata->covenant) < 2 || 
	    strlen(ch->pcdata->cov_time) < 2 || 
	    strlen(ch->pcdata->vleader) < 2)
	{
	    send_to_char( "But you are not a member of any covenant!\n\r", ch );
	    return;
	}
	if (!IS_MORE(ch, MORE_LEADER) && !IS_MORE(ch, MORE_CHAMPION))
	{
	    send_to_char( "Only a covenant leader or champion can outcast members.\n\r", ch );
	    return;
	}
	if (!IS_MORE(ch, MORE_CHAMPION) && IS_MORE(ch, MORE_LEADER))
	{
	    send_to_char( "Your covenant has not yet been completely set up.\n\r", ch );
	    return;
	}

	/* Find the person to be outcast, and check that they are valid. */
	if (argument[0] == '\0')
	{
	    send_to_char( "Who do you wish to outcast?\n\r", ch );
	    return;
	}
	if ( ( victim = get_char_world( ch, argument ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}
	if (IS_NPC(victim))
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}
	if (victim == ch)
	{
	    send_to_char( "You cannot outcast yourself.\n\r", ch );
	    return;
	}
	if (strlen(victim->pcdata->covenant) < 2 || 
	    strlen(victim->pcdata->cov_time) < 2 || 
	    strlen(victim->pcdata->vleader) < 2)
	{
	    send_to_char( "They are not a member of your covenant.\n\r", ch );
	    return;
	}
	if (str_cmp(ch->pcdata->vleader,victim->pcdata->vleader))
	{
	    send_to_char( "They are not a member of your covenant.\n\r", ch );
	    return;
	}
	if (str_cmp(ch->pcdata->covenant,victim->pcdata->covenant))
	{
	    send_to_char( "They are not a member of your covenant.\n\r", ch );
	    return;
	}
	if (str_cmp(ch->pcdata->cov_time,victim->pcdata->cov_time))
	{
	    send_to_char( "They are not a member of your covenant.\n\r", ch );
	    return;
	}
	if (IS_MORE(victim, MORE_LEADER) || IS_MORE(victim, MORE_CHAMPION))
	{
	    send_to_char( "You can only outcast regular members.\n\r", ch );
	    return;
	}

	act("You outcast $N from the covenant.",ch,NULL,victim,TO_CHAR);
	act("$n outcasts you from the covenant.",ch,NULL,victim,TO_VICT);

	free_string(victim->pcdata->covenant);
	victim->pcdata->covenant = str_dup( "" );
	free_string(victim->pcdata->cov_time);
	victim->pcdata->cov_time = str_dup( "None" );
	free_string(victim->pcdata->vleader);
	victim->pcdata->vleader = str_dup( "" );
	REMOVE_BIT(victim->more, MORE_CHAMPION);

	for ( gch = char_list; gch != NULL; gch = gch->next )
	{
	    if ( IS_NPC(gch) ) continue;
	    if ( strlen(gch->pcdata->covenant) < 2) continue;
	    if ( strlen(gch->pcdata->cov_time) < 2) continue;
	    if ( strlen(gch->pcdata->vleader) < 2) continue;
	    if ( str_cmp(ch->pcdata->vleader,gch->pcdata->vleader) ) continue;
	    if ( str_cmp(ch->pcdata->cov_time,gch->pcdata->cov_time) ) continue;
	    if ( !str_cmp(ch->pcdata->covenant,gch->pcdata->covenant) )
		act( "Info -> $N has been kicked out of the covenant!", gch, NULL, victim,TO_CHAR );
	}
	return;
    }

    if ( !str_cmp(arg, "champion") )
    {
	if (strlen(ch->pcdata->covenant) < 2 || 
	    strlen(ch->pcdata->cov_time) < 2 || 
	    strlen(ch->pcdata->vleader) < 2)
	{
	    send_to_char( "But you are not a member of any covenant!\n\r", ch );
	    return;
	}
	if (!IS_MORE(ch, MORE_LEADER))
	{
	    send_to_char( "Only a covenant leader can make someone a champion.\n\r", ch );
	    return;
	}
	if (!IS_MORE(ch, MORE_CHAMPION))
	{
	    send_to_char( "Your covenant has not yet been completely set up.\n\r", ch );
	    return;
	}

	/* Find the person to be made a champion, and check they are valid. */
	if (argument[0] == '\0')
	{
	    send_to_char( "Who do you wish to make into a champion?\n\r", ch );
	    return;
	}
	if ( ( victim = get_char_world( ch, argument ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}
	if (IS_NPC(victim))
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}
	if (victim == ch)
	{
	    send_to_char( "You cannot make yourself a champion!\n\r", ch );
	    return;
	}

	if (strlen(victim->pcdata->covenant) < 2 || 
	    strlen(victim->pcdata->cov_time) < 2 || 
	    strlen(victim->pcdata->vleader) < 2)
	{
	    send_to_char( "They are not a member of your covenant.\n\r", ch );
	    return;
	}
	if (str_cmp(ch->pcdata->vleader,victim->pcdata->vleader))
	{
	    send_to_char( "They are not a member of your covenant.\n\r", ch );
	    return;
	}
	if (str_cmp(ch->pcdata->covenant,victim->pcdata->covenant))
	{
	    send_to_char( "They are not a member of your covenant.\n\r", ch );
	    return;
	}

	if (str_cmp(ch->pcdata->cov_time,victim->pcdata->cov_time))
	{
	    send_to_char( "They are not a member of your covenant.\n\r", ch );
	    return;
	}

	if (!IS_MORE(victim, MORE_CHAMPION))
	{
	    act("You make $N a covenant champion.",ch,NULL,victim,TO_CHAR);
	    act("$n makes you a covenant champion.",ch,NULL,victim,TO_VICT);
	    sprintf(buf,"Info -> $N has been made a champion!");
	    SET_BIT(victim->more, MORE_CHAMPION);
	}
	else
	{
	    act("You remove $N's covenant champion position.",ch,NULL,victim,TO_CHAR);
	    act("$n removes your covenant champion position.",ch,NULL,victim,TO_VICT);
	    sprintf(buf,"Info -> $N is no longer a covenant champion!");
	    REMOVE_BIT(victim->more, MORE_CHAMPION);
	}

	for ( gch = char_list; gch != NULL; gch = gch->next )
	{
	    if ( IS_NPC(gch) ) continue;
	    if ( strlen(gch->pcdata->covenant) < 2) continue;
	    if ( strlen(gch->pcdata->cov_time) < 2) continue;
	    if ( strlen(gch->pcdata->vleader) < 2) continue;
	    if ( str_cmp(ch->pcdata->vleader,gch->pcdata->vleader) ) continue;
	    if ( str_cmp(ch->pcdata->cov_time,gch->pcdata->cov_time) ) continue;
	    if ( !str_cmp(ch->pcdata->covenant,gch->pcdata->covenant) )
		act( buf, gch, NULL, victim, TO_CHAR );
	}
	return;
    }

    send_to_char("--------------------------------------------------------------------------------\n\r",ch);
    send_to_char("                              -= Covenant Options =-\n\r",ch);
    send_to_char("--------------------------------------------------------------------------------\n\r",ch);
    send_to_char("CHAMPION : Make a member into a champion.\n\r",ch);
    send_to_char("FOUND    : Founds a new Covenant.\n\r",ch);
    send_to_char("INDUCT   : Inducts a member into your covenant.\n\r",ch);
    send_to_char("INFO     : Lists information on your covenant.\n\r",ch);
    send_to_char("LEAVE    : Leaves your current Covenant.\n\r",ch);
    send_to_char("OUTCAST  : Outcasts a member from your covenant.\n\r",ch);
    send_to_char("--------------------------------------------------------------------------------\n\r",ch);
    return;
}

void do_cvalk( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;
    char buf[MSL];
    
    if (IS_NPC(ch)) return;
    if (strlen(ch->pcdata->covenant) < 2 || strlen(ch->pcdata->vleader) < 2)
    {
	stc("But you don't belong to a covenant!\n\r", ch);
	return;
    }

    if (argument[0] == '\0')
    {
	stc("Covenant-talk what?\n\r", ch);
	return;
    }

    if (IS_SET(ch->act, PLR_SILENCE) || (ch->in_room != NULL && IS_SET(ch->in_room->added_flags, ROOM2_SILENCE) && !IS_BUI(ch)))
    {
	stc("You can't cvalk.\n\r", ch);
	return;
    }

    if (IS_MORE(ch, MORE_LEADER)) sprintf(buf, "#p-=#r%s#p=-#n '#g$t#n'.", ch->name);
    else if (IS_MORE(ch, MORE_CHAMPION)) sprintf(buf, "#p=#r%s#p=#n '#g$t#n'.", ch->name);
    else sprintf(buf, "#p-#r%s#p-#n '#g$t#n'.", ch->name);

    for (d = descriptor_list; d != NULL; d = d->next)
    {
	if (d->connected != CON_PLAYING) continue;
	if ((victim = d->character) == NULL) continue;
	if (IS_NPC(victim)) continue;
	if (!IS_IMM(victim))
	{
	    if (strlen(victim->pcdata->covenant) < 2) continue;
	    if (strlen(victim->pcdata->vleader) < 2) continue;
	    if (str_cmp(ch->pcdata->covenant, victim->pcdata->covenant)) continue;
	    if (str_cmp(ch->pcdata->vleader, victim->pcdata->vleader)) continue;
	}
	act(buf, victim, argument, NULL, TO_CHAR);
    }
    return;
}

void do_shadowplay( CHAR_DATA *ch, char *argument )
{
    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch))
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    if (get_disc(ch, DISC_OBTENEBRATION) < 1)
    {
	send_to_char("You require the level 1 Obtenebration to use Shadowplay.\n\r",ch);
	return;
    }

    if (weather_info.sunlight != SUN_DARK)
    {
	if (!ch->in_room->sector_type == SECT_INSIDE && !room_is_dark(ch->in_room))
	{
	    send_to_char("You cannot use this while outside in the sunlight.\n\r", ch);
	    return;
	}
    }

    if (IS_AFFECTED(ch, AFF_HIDE))
    {
        send_to_char( "You are already hidden in the shadows.\n\r", ch );
        return;
    }

    act( "A look of concentration crosses $n's face.", ch, NULL, NULL, TO_ROOM );

    if (!IS_AFFECTED(ch, AFF_HIDE))
    {
        act("$n seems to blend into the shadows.",ch,NULL,NULL,TO_ROOM);
        send_to_char("You emerse yourself deep into the shadows.\n\r",ch);
        SET_BIT(ch->affected_by, AFF_HIDE);
    }
    WAIT_STATE( ch, 12 );
    return;
}

void do_shadowstep( CHAR_DATA *ch, char *argument )
{
    char      arg [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch))
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if (arg[0] == '\0')
    {
	send_to_char( "Who do you wish to shadowstep to?\n\r", ch );
	return;
    }

    if (get_disc(ch,DISC_OBTENEBRATION) < 5)
    {
	send_to_char("You require level 5 Obtenebration to use Shadow Step.\n\r",ch);
	return;
    }
    if (ch->fight_timer > 0)
    {
      stc("Not with a fight timer!\n\r", ch);
      return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL
    ||   victim == ch
    ||   victim->in_room == NULL
    ||   ch->in_room == NULL
    ||   (IS_NPC(victim) && victim->pIndexData->vnum == MOB_VNUM_EYE)
    ||   victim->level > (get_disc(ch,DISC_OBTENEBRATION) * 140))
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
    ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_TELEPORT)
    ||   victim->position != POS_STANDING
    ||   IS_SET(victim->extra, EXTRA_ILLUSION)
    ||   (!IS_NPC(victim) && !IS_EXTRA(victim, EXTRA_SUMMON) ) )
    {
	send_to_char( "You are unable to focus on their location.\n\r", ch );
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim, MORE_GUARDIAN))
	{
	    guardian_message( victim );
	    act("$n just failed to use Summoning on you.",ch,NULL,victim,TO_VICT);
	}
	return;
    }

    if (victim->wizard == ch)
    {  send_to_char("You can't step to that!", ch);
       return;
    }

    send_to_char("You step into the shadows.\n\r",ch);
    act("$n steps into the shadows and vanishes from sight.",ch,NULL,NULL,TO_ROOM);
    char_from_room(ch);
    char_to_room(ch,victim->in_room);
    do_look(ch,"");
/*    send_to_char("You step out of the shadows.\n\r",ch);         */
    act("You step from the shadows.",ch,NULL,victim,TO_CHAR);       
    act("$n steps from the shadows.",ch,NULL,victim,TO_ROOM);
    return;
}


void do_spiriteyes( CHAR_DATA *ch, char *argument )
{
    char      arg [MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch))
    {
	send_to_char("Huh?\n\r",ch);
	return;
    }

    if (get_disc(ch,DISC_NECROMANCY) < 1)
    {
	send_to_char("You require level 1 Necromancy to use Spirit Eyes.\n\r",ch);
	return;
    }

    if (IS_VAMPAFF(ch,VAM_SPIRITEYES))
    {
	send_to_char("You can no longer see into the shadowlands.\n\r",ch);
	REMOVE_BIT(ch->vampaff, VAM_SPIRITEYES);
	REMOVE_BIT(ch->affected_by, AFF_SHADOWSIGHT);
	return;
    }
    send_to_char("You can now see into the shadowlands.\n\r",ch);
    SET_BIT(ch->vampaff, VAM_SPIRITEYES);
    SET_BIT(ch->affected_by,AFF_SHADOWSIGHT);
    return;
}


void do_sbreath( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;


    if (!IS_VAMPIRE(ch))
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if (get_disc(ch,DISC_SERPENTIS) < 5)
    {
	send_to_char("You require level 5 Serpentis to use Breath of the Basilisk.\n\r",ch);
	return;
    }

    if (IS_ITEMAFF(ch, ITEMA_PEACE)) do_majesty(ch,"");

    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
        vch_next = vch->next_in_room;
	if (ch == vch || IS_BUI(vch)) continue;
	if (is_safe(ch, vch)) continue;

	one_hit(ch, vch, TYPE_HIT, "breath");
    }

    WAIT_STATE( ch, 12 );

    return;
}

void do_celerity( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if (IS_NINJA(ch)) return;

    if (!IS_VAMPIRE(ch))
    {
	send_to_char("Huh?\n\r",ch);
	return;
    }

    if (get_disc(ch, DISC_CELERITY) < 1)
    {
	send_to_char("But you don't have any Celerity!\n\r",ch);
	return;
    }

    if (arg[0] != '\0')
    {
	if (!str_cmp(arg,"renew"))
	{
	    if (IS_WILLPOWER(ch, RES_CELERITY))
	    {
		send_to_char("You will no longer automatically renew Celerity when you are in a fight.\n\r",ch);
		REMOVE_BIT(ch->pcdata->resist[0], RES_CELERITY);
	    }
	    else
	    {
		send_to_char("You will now automatically renew Celerity when you are in a fight.\n\r",ch);
		SET_BIT(ch->pcdata->resist[0], RES_CELERITY);
	    }
	}
	else
	    send_to_char("Type 'celerity' to activate, or 'celerity renew' to toggle autorenew.\n\r",ch);
	return;
    }
    else
    {
	if ( ch->blood[BLOOD_CURRENT] < 10 )
	{
	    send_to_char("You have insufficient blood.\n\r",ch);
	    return;
	}

	if ( ch->pcdata->celerity > 50 )
	{
	    send_to_char("You are unable to burn any more blood for celerity.\n\r",ch);
	    return;
	}
	else
	{
	    if ( ch->pcdata->celerity > 0 )
		send_to_char("You burn some more blood to keep your celerity running longer.\n\r",ch);
	    else
	    {
		send_to_char("You start moving with supernatural speed!\n\r",ch);
		act("$n starts moving with supernatural speed!",ch,NULL,NULL,TO_ROOM);
	    }
	    ch->pcdata->celerity += 50;
	    ch->blood[BLOOD_CURRENT] -= 10;
	    return;
	}
    }
    return;
}

void guardian_message( CHAR_DATA *ch )
{
    send_to_char("Your guardian spirit sends you a telepathic message:\n\r",ch);
    return;
}

void do_chiroptean( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];
    char buf[MSL];

    argument = one_argument(argument, arg);

    if (IS_NPC(ch)) return;

    if (!IS_VAMPIRE(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (get_disc(ch, DISC_VICISSITUDE) < 6)
    {
	stc("You require level 6 Vicissitude to change into Chiroptean Marauder.\n\r", ch);
	return;
    }

    if (IS_AFFECTED(ch,AFF_POLYMORPH) && !IS_VAMPAFF(ch,VAM_DISGUISED))
    {
	send_to_char( "Not while polymorphed.\n\r", ch );
	return;
    }

    if (IS_POLYAFF(ch,POLY_CHIRO))
    {
	free_string(ch->morph);
	ch->morph = str_dup("");
	REMOVE_BIT(ch->affected_by, AFF_POLYMORPH);
	REMOVE_BIT(ch->vampaff, VAM_DISGUISED);
	REMOVE_BIT(ch->vampaff, VAM_CLAWS);
	REMOVE_BIT(ch->vampaff, VAM_NIGHTSIGHT);
	REMOVE_BIT(ch->polyaff, POLY_CHIRO);
	send_to_char("Your body shrinks and distorts back to its normal form.\n\r", ch);
	act("#rAn enormous bipedal bat#n's body shrinks and distorts into the form of $n.", ch, NULL, NULL, TO_ROOM);
	return;
    }

    stc("Your body grows and distorts into an enormous bipedal bat.\n\r",ch);
    act("$n's body grows and distorts into an #renormous bipedal bat#n.",ch,NULL,NULL,TO_ROOM);
    SET_BIT(ch->vampaff, VAM_NIGHTSIGHT);
    SET_BIT(ch->affected_by, AFF_POLYMORPH);
    SET_BIT(ch->vampaff, VAM_DISGUISED);
    SET_BIT(ch->polyaff, POLY_CHIRO);
    sprintf(buf, "#rAn enormous bipedal bat#n[#0%s#n]", ch->name);
    free_string( ch->morph );
    ch->morph = str_dup( buf );

    return;
}
void do_empathy( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MIL];

    one_argument(argument, arg);

    if (!IS_VAMPIRE(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (get_disc(ch, DISC_PRESENCE) < 7)
    {
	stc("You require level 7 presence to use Mask Empathy.\n\r", ch);
	return;
    }

    if (arg[0] == '\0')
    {
	stc("Who do you wish to use Mask Empathy on?\n\r", ch);
	return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL)
    {
	stc("They aren't here.\n\r", ch);
	return;
    }

    if(IS_SET(victim->more, MORE_LOYAL))
    {
	stc("$n removes your conditioning.\n\r", victim);
	stc("You remove $N's conditioning.\n\r", ch);
	REMOVE_BIT(victim->more, MORE_LOYAL);
    }
    else stc("They have not been conditioned.\n\r", ch);

    return;
}

