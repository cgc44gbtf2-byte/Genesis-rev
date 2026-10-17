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

void do_persuasion( CHAR_DATA *ch, char *argument )
{
    if (!IS_WEREWOLF(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }
    if (get_breed(ch, BREED_HOMID) < 1)
    {
	stc("You require the level 1 Homid power to become persuasive.\n\r", ch);
	return;
    }

    if (IS_EXTRA(ch,EXTRA_AWE) )
    {
	send_to_char("You are no longer so persuasive.\n\r",ch);
	act("$n no longer appears so persuasive.", ch, NULL, NULL, TO_ROOM);
	REMOVE_BIT(ch->extra, EXTRA_AWE);
	return;
    }
    send_to_char("Your are now persuasive.\n\r",ch);
    act("$n suddenly appears very persuasive.", ch, NULL, NULL, TO_ROOM);
    SET_BIT(ch->extra, EXTRA_AWE);
    return;
}

void do_jamtech( CHAR_DATA *ch, char *argument )
{
  OBJ_DATA *obj;
  CHAR_DATA *victim;
  char arg[MIL];
  int chance;
  argument = one_argument( argument, arg );
    
  if ( IS_NPC(ch) ) return;
  if (!IS_WEREWOLF(ch))
    stc("Huh?\n\r",ch);
  else if (get_breed(ch,BREED_HOMID) < 2)
    stc("You require Homid 2 to use Jam Technology.\n\r",ch);
  else if (arg[0] == '\0')
    stc("Whom do you wish to use this on?\n\r",ch);
  else if ((victim = get_char_room( ch, arg ) ) == NULL )
      stc("You are unable to find them in this room.\n\r",ch);
  else if ((obj = get_eq_char(victim,WEAR_WEAPON)) == NULL)
      stc("They aren't using any weapons!\n\r",ch);
  else
  {
	chance = number_range(1, 10);
	if (obj->item_type != ITEM_WEAPON) chance -= 3;
	if (chance <= 4)
	{
	    stc("You lose your concentration.\n\r", ch);
	    return;
	}
    obj_from_char( obj );
    obj_to_char( obj, victim );
    act("You cause $N's weapon to stop working.",ch,NULL,victim,TO_CHAR);
    act("$n's eyes take on a metallic sheen as he concentrates.",ch,NULL,NULL,TO_ROOM);
    act("$p stops working!",ch,obj,victim,TO_VICT);
    WAIT_STATE(ch, 16);
  }
  return;
}

void do_reshape( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    char      arg1[MAX_INPUT_LENGTH];
    char      arg2[MAX_INPUT_LENGTH];
    char      buf [MAX_STRING_LENGTH];
    char      endchar[15];

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (IS_NPC(ch)) return;

    if (!IS_WEREWOLF(ch))
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if (get_breed(ch, BREED_HOMID) < 3)
    {
	send_to_char("You require the level 3 Homid power to use Reshape Object.\n\r",ch);
	return;
    }

    if ( arg1[0] == '\0' || arg2[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Syntax: Reshape <object> rename  <short description>\n\r", ch );
	send_to_char( "Syntax: Reshape <object> keyword <extra keyword/s>\n\r", ch );
	send_to_char( "Syntax: Reshape <object> flags   <glow/hum>\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
    {
	send_to_char( "You are not carrying that object.\n\r", ch );
	return;
    }

    if (obj->item_type == ITEM_QUEST || obj->item_type == ITEM_AMMO ||
        obj->item_type == ITEM_EGG   || obj->item_type == ITEM_VOODOO ||
        obj->item_type == ITEM_MONEY || obj->item_type == ITEM_TREASURE ||
        obj->item_type == ITEM_PEN   ||
	obj->item_type == ITEM_PAGE  || IS_SET(obj->quest, QUEST_ARTIFACT))
    {
	send_to_char( "You cannot quest-change that item.\n\r", ch );
	return;
    }

    if (!IS_IMM(ch) && (obj->questowner == NULL || str_cmp(ch->name,obj->questowner)))
    {
	send_to_char("You can only change an item you own.\n\r", ch);
	return;
    }

    if ( !str_cmp( arg2, "rename" ) )
    {
	if (IS_SET(obj->quest, QUEST_RELIC))
	{
	    send_to_char("Not on a relic.\n\r",ch);
	    return;
	}
	if (strlen(argument) < 5)
	{
	    send_to_char("Name should be at least 5 characters long.\n\r",ch);
	    return;
	}
	strcpy(argument,capitalize(argument));
	argument[0] = LOWER(argument[0]);
	endchar[0] = (argument[strlen(argument)-1]);
	endchar[1] = '\0';
	if (!str_cmp(endchar,".")) argument[strlen(argument)-1] = '\0';
	free_string( obj->name );
	obj->name = str_dup( argument );
	free_string( obj->short_descr );
	obj->short_descr = str_dup( argument );
	if (is_in(argument,"|pair of*"))
	    sprintf(buf, "%s are lying here.",argument);
	else
	    sprintf(buf, "%s is lying here.",argument);
	buf[0] = UPPER(buf[0]);
	free_string( obj->description );
	obj->description = str_dup( buf );
	send_to_char("Ok.\n\r",ch);
    	if (obj->questmaker != NULL) free_string(obj->questmaker);
    	obj->questmaker = str_dup(ch->name);
	return;
    }

    if ( !str_cmp( arg2, "keyword" ) || !str_cmp( arg2, "keywords" ) )
    {
	if (IS_SET(obj->quest, QUEST_RELIC))
	{
	    send_to_char("Not on a relic.\n\r",ch);
	    return;
	}
	if (strlen(argument) < 3)
	{
	    send_to_char("Keywords should be at least 3 characters long.\n\r",ch);
	    return;
	}
	strcpy(argument,capitalize(argument));
	argument[0] = LOWER(argument[0]);
	endchar[0] = (argument[strlen(argument)-1]);
	endchar[1] = '\0';
	if (!str_cmp(endchar,".")) argument[strlen(argument)-1] = '\0';
	strcpy(buf,obj->name);
	if (strlen(buf) > 80)
	{
	    send_to_char("You cannot fit any more keywords on this object.\n\r",ch);
	    return;
	}
	strcat(buf," ");
	strcat(buf,argument);
	free_string( obj->name );
	obj->name = str_dup( buf );
	send_to_char("Ok.\n\r",ch);
    	if (obj->questmaker != NULL) free_string(obj->questmaker);
    	obj->questmaker = str_dup(ch->name);
	return;
    }

    if ( !str_cmp( arg2, "flags" ) )
    {
	if (!str_cmp(argument,"glow"))
	{
	    if (IS_SET(obj->extra_flags, ITEM_GLOW))
		REMOVE_BIT(obj->extra_flags, ITEM_GLOW);
	    else
		SET_BIT(obj->extra_flags, ITEM_GLOW);
	}
	else if (!str_cmp(argument,"hum"))
	{
	    if (IS_SET(obj->extra_flags, ITEM_HUM))
		REMOVE_BIT(obj->extra_flags, ITEM_HUM);
	    else
		SET_BIT(obj->extra_flags, ITEM_HUM);
	}
	else
	{
	    send_to_char( "Syntax: Reshape <object> flags   <glow/hum>\n\r", ch );
	    return;
	}
	send_to_char("Ok.\n\r",ch);
    	if (obj->questmaker != NULL) free_string(obj->questmaker);
    	obj->questmaker = str_dup(ch->name);
	return;
    }

    send_to_char( "Syntax: Reshape <object> rename  <short description>\n\r", ch );
    send_to_char( "Syntax: Reshape <object> keyword <extra keyword/s>\n\r", ch );
    send_to_char( "Syntax: Reshape <object> flags   <glow/hum>\n\r", ch );
    return;
}

void do_disquiet( CHAR_DATA *ch, char *argument )
{
    if (!IS_WEREWOLF(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (get_breed(ch, BREED_HOMID) < 4)
    {
	stc("You require the level 4 Homid power to use Disquiet.\n\r", ch);
	return;
    }

    stc("Gutted!\n\r", ch);
    return;
}

void do_spiritward( CHAR_DATA *ch, char *argument )
{
    if (!IS_WEREWOLF(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (get_breed(ch, BREED_HOMID) < 5)
    {
	stc("You require Homid 5 to use Spirit Ward.\n\r", ch);
	return;
    }

    if (!IS_MORE(ch, MORE_SHIELDED))
    {
	stc("You ward yourself from spirits.\n\r", ch);
	SET_BIT(ch->more, MORE_SHIELDED);
	return;
    }
    stc("You stop warding yourself from spirits.\n\r", ch);
    REMOVE_BIT(ch->more, MORE_SHIELDED);
    return;
}

void do_primalanger( CHAR_DATA *ch, char *argument )
{
    if (!IS_WEREWOLF(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (get_breed(ch, BREED_METIS) < 1)
    {
	stc("You need 1 Metis to release Primal Anger\n\r", ch);
	return;
    }

    if (ch->hit < ch->max_hit * .5)
    {
	stc("You are too hurt to risk Primal Anger.\n\r", ch);
	return;
    }

    stc("Your anger burns through you pulling you into a rage.\n\r", ch);
    act("Pain covers $n's face as $e begins to snarl in rage.", ch, NULL, NULL, TO_ROOM);
    ch->hit -= ch->hit * .1;
    do_werewolf(ch, "");
    ch->pcdata->wolf = 400;
    if (ch->position == POS_FIGHTING) WAIT_STATE(ch, 12);
    return;
}

void do_burrow( CHAR_DATA *ch, char *argument )
{
    if (IS_NPC(ch)) return;

    if (!IS_WEREWOLF(ch))
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if (get_breed(ch,BREED_METIS) < 2)
    {
	send_to_char("You require the level 2 Metis power to Burrow into the ground.\n\r",ch);
	return;
    }

    if (IS_VAMPAFF(ch,VAM_EARTHMELDED) )
    {
	REMOVE_BIT(ch->vampaff, VAM_EARTHMELDED);
	send_to_char("You burrow up out of the ground.\n\r",ch);
	act("$n burrows up out of the ground.", ch, NULL, NULL, TO_ROOM);
	return;
    }
    send_to_char("You burrow down into the ground.\n\r",ch);
    act("$n burrows down into the ground.", ch, NULL, NULL, TO_ROOM);
    SET_BIT(ch->vampaff, VAM_EARTHMELDED);
    REMOVE_BIT(ch->affected_by, AFF_FLAMING);
    return;
}

void do_porcupine( CHAR_DATA *ch, char *argument )
{
    if (IS_NPC(ch)) return;

    if (!IS_WEREWOLF(ch) )
    {
	send_to_char("Huh?\n\r",ch);
	return;
    }

    if (get_breed(ch,BREED_METIS) < 4)
    {
	send_to_char("You require the level 4 Metis power to use the Gift of the Porcupine.\n\r",ch);
	return;
    }

    if (!IS_SET(ch->act, PLR_WOLFMAN))
    {
	send_to_char("This power can only be activated in Crinos form.\n\r",ch);
	return;
    }

    if (IS_MORE(ch,MORE_BRISTLES) )
    {
	send_to_char("Your long quill-like spines retract into your body.\n\r",ch);
	act("$n's long quill-like spines retract into your body.", ch, NULL, NULL, TO_ROOM);
	REMOVE_BIT(ch->more, MORE_BRISTLES);
	return;
    }
    send_to_char("Long quill-like spines extend from your body.\n\r",ch);
    act("Long quill-like spines extend from $n's body.", ch, NULL, NULL, TO_ROOM);
    SET_BIT(ch->more, MORE_BRISTLES);
    return;
}

void do_animal( CHAR_DATA *ch, char *argument )
{
    if (IS_NPC(ch)) return;

    if (!IS_WEREWOLF(ch))
    {
	send_to_char("Huh?\n\r",ch);
	return;
    }

    if (get_breed(ch, BREED_LUPUS) < 4)
    {
	send_to_char("You require Lupus 4 to use Beast Life.\n\r",ch);
	return;
    }
    if (argument[0] != '\0')
    {
	do_beckon(ch,argument);
	return;
    }

    if (IS_MORE(ch,MORE_ANIMAL_MASTER) )
    {
	send_to_char("You are no longer so attractive to animals.\n\r",ch);
	REMOVE_BIT(ch->more, MORE_ANIMAL_MASTER);
	return;
    }
    send_to_char("You make yourself attractive to animals.\n\r",ch);
    SET_BIT(ch->more, MORE_ANIMAL_MASTER);
    return;
}

void do_elemental( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg1[MIL];
    char arg2[MIL];
    long dam;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (IS_NPC(ch)) return;

    if (!IS_WEREWOLF(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (get_breed(ch, BREED_LUPUS) < 5)
    {
	stc("You require Lupus 5 to use Elemental Gift.\n\r", ch);
	return;
    }

    if (arg1[0] == '\0' || (arg2[0] == '\0' && ch->fighting == NULL))
    {
	stc("Syntax: elemental [fire/water/earth/air] victim.\n\r", ch);
	return;
    }

    if (arg2[0] == '\0' || (victim = get_char_room(ch, arg2)) == NULL)
    {
	if ((victim = ch->fighting) == NULL)
	{
	    stc("They aren't here.\n\r", ch);
	    return;
	}
    }

    act("$n begins to howl loudly!", ch, NULL, NULL, TO_ROOM);
    if (!str_cmp(arg1, "fire"))
    {
	stc("You call elemental fire to strike down your foes!\n\r",ch);
	act("The raging inferno that is the elemental spirit of fire engulfs $n.",victim,NULL,NULL,TO_ROOM);
	stc("Your flesh is seared off as the elemental spirit of fire engulfs you!\n\r",victim);
	dam = number_range(1000, 1500);
	damage(ch, victim, dam, TYPE_HIT, "fire blast");
	WAIT_STATE(ch, 12);
    }
    else if (!str_cmp(arg1, "water"))
    {
        stc("You call elemental water to strike down your foes!\n\r",ch);
        act("The churning ocean that is the elemental spirit of water drowns $n.",victim,NULL,NULL,TO_ROOM);
	stc("You gag as you breath in the very essence of the elemental spirit of water!\n\r",victim);
	dam = number_range(1000, 1500);
        damage(ch, victim, dam, TYPE_HIT, "tidal wave");
        WAIT_STATE(ch, 6);
    }
    else if (!str_cmp(arg1, "earth"))
    {
        stc("You call elemental earth to strike down your foes!\n\r", ch);
        act("Boulders pile atop $n as the elemental spirit of earth flexes his might.",victim,NULL,NULL,TO_ROOM);
        stc("You are smashed into a bloody pulp by the elemental spirit of earth!\n\r", victim);
        dam = number_range(250, 500);
        damage(ch, victim, dam, TYPE_HIT, "boulder rain");
        dam = number_range(250, 500);  
        damage(ch, victim, dam, TYPE_HIT, "boulder rain");
        dam = number_range(250, 500);  
        damage(ch, victim, dam, TYPE_HIT, "boulder rain");
        dam = number_range(250, 500);  
        damage(ch, victim, dam, TYPE_HIT, "boulder rain");
        WAIT_STATE(ch, 12);
    }
    else if (!str_cmp(arg1, "air"))
    {
        stc("You call elemental air to strike down your foes!\n\r",ch);
        act("A rampaging hurricane that is the elemental spirit of air swallows $n.",victim,NULL,NULL,TO_ROOM);
	stc("You are cut to the bone by the razor sharp winds of the elemental spirit of air!\n\r",victim);
        dam = number_range(2000, 3000);
        damage(ch, victim, dam, TYPE_HIT, "wind slash");
        WAIT_STATE(ch, 12);
    }
    else stc("The elemental forces ignore you!\n\r",ch);
    return;
}

