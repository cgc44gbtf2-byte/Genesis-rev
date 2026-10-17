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

void do_increase( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];
    char buf[MIL];
    long cost, power;

    argument = one_argument(argument, arg);

    if (!IS_HIGHLANDER(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (arg[0] == '\0')
    {
	sprintf(buf, "Increases: Attack (%li), Defense (%li), Weaponry (%li), Powers (%li).\n\r",
	 get_highland(ch, HIGH_ATTACK), get_highland(ch, HIGH_DEFENSE), get_highland(ch, HIGH_WEAPON), get_highland(ch, HIGH_ABILITY));
	stc(buf, ch);
	stc("Syntax: increase <attack/defense/weaponry/powers>",ch);
	return;
    }

    if (!str_cmp(arg, "attack")) power = HIGH_ATTACK;
    else if (!str_cmp(arg, "defense")) power = HIGH_DEFENSE;
    else if (!str_cmp(arg, "weaponry")) power = HIGH_WEAPON;
    else if (!str_cmp(arg, "powers")) power = HIGH_ABILITY;
    else
    {
	do_increase(ch, "");
	return;
    }

    if (get_highland(ch, power) >= 6)
    {
	sprintf(buf, "You have already achieved maximum proficiency in highlander %s.\n\r", arg);
	stc(buf, ch);
	return;
    }

    cost = (get_highland(ch, power) + 1) * 15;
    if (ch->primal < cost)
    {
	sprintf(buf, "It costs %li primal to improve your skill with highlander %s.\n\r", cost, arg);
	stc(buf, ch);
	return;
    }

    ch->primal -= cost;
    ch->pcdata->highland[power]++;
    sprintf(buf, "You have improved your skill with highlander %s.\n\r", arg);
    stc(buf, ch);
    return;
}

void do_bladeflash(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_STRING_LENGTH];
    long sn;
    AFFECT_DATA af;
    CHAR_DATA *victim;

    argument = one_argument(argument, arg);

    if (!IS_HIGHLANDER(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }
    if (get_highland(ch, HIGH_ABILITY) < 2)
    {
	stc("You have not learned the Powers ability to level 2 yet.\n\r", ch);
	return;
    }
    if (arg[0] == '\0')
    {
	stc("Who do you wish to blade flash.\n\r",ch);
	return;
    }
    if ((victim = get_char_room(ch, arg)) == NULL)
    {
	stc("They are not here.\n\r",ch);
	return;
    }
    if (is_safe(ch,victim)) return;

    if (!IS_NPC(victim))
    {
	if (IS_SET(victim->act, PLR_HOLYLIGHT)) REMOVE_BIT(victim->act, PLR_HOLYLIGHT);
	else
	{
/*	    stc("Your blade flash has no affect.\n\r",ch);
	    WAIT_STATE(ch,24);
	    return; */
	}
    }

    if (!IS_AFFECTED(victim, AFF_BLIND))
    {
    	if ( ( sn = skill_lookup( "blind" ) ) < 0 ) return;
    	af.type      = sn;
    	af.location  = APPLY_HITROLL;
    	af.modifier  = -50;
    	af.duration  = 20;
    	af.bitvector = AFF_BLIND;
    	affect_to_char( victim, &af );
    }

    act("You catches a bit of light and shines it in $N's eyes.",ch,NULL,victim,TO_CHAR);
    act("$n catches a bit of light and shines it in $N's eyes.",ch,NULL,victim,TO_NOTVICT);
    act("$n catches a bit of light and shines it in your eyes.",ch,NULL,victim,TO_VICT);
    WAIT_STATE(ch,24);
    return;
}

void do_bash(CHAR_DATA *ch, char *argument) 
{
    char arg[MAX_STRING_LENGTH];
    char dam[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    long damage = (850 + number_range(1,100));
    argument = one_argument(argument, arg);

    if (!IS_HIGHLANDER(ch))
    {
	stc("Huh?\n\r",ch);
	return;
    }
    if (get_highland(ch, HIGH_ABILITY) < 3)
    {
	stc("You have not learned the Powers ability to level 3 yet.\n\r", ch);
	return;
    }
    if( (victim = ch->fighting) == NULL )
    {
	if( arg[0] == '\0' )
	{
	    stc("Bash whom?\r\n", ch);
	    return;
	}
	if( (victim = get_char_room(ch, arg)) == NULL )
	{
	    stc("How can you bash someone who is not here?\r\n", ch);
	    return;
	}
    }

    if (!hurt_fighting(ch, victim)) return;

    if (number_range(1,4) == 1 || (IS_WEREWOLF(victim) && get_breed(victim,BREED_LUPUS) >= 3 && number_range(1,3) == 1))
    {
	act("You rush forward, missing $N completely!",ch,NULL,victim,TO_CHAR);
	act("$n rushes forward, missing $N completely!",ch,NULL,victim,TO_NOTVICT);
	act("$n rushes forward, missing you completely!",ch,NULL,victim,TO_VICT);
    }
    else
    {
	sprintf(dam, "%li", damage);
	act("You rush forward, slamming into $N! [#g$t#n]", ch, dam, victim, TO_CHAR);
	act("$n rushes forward, slamming into $N!", ch, dam, victim, TO_NOTVICT);
	act("$n rushes forward, slamming into you! [#r$t#n]", ch, dam, victim, TO_VICT);
	hurt_person(ch, victim, damage);
	victim->wait += 10;
    }
    WAIT_STATE(ch, 20);
    return;
}

void do_htwist( CHAR_DATA *ch, char *argument)
{
    if (!IS_HIGHLANDER(ch))
    {
	stc("Huh?\n\r",ch);
	return;
    }
    if (get_highland(ch, HIGH_ABILITY) < 4)
    {
	stc("You have not learned the Powers ability to level 4 yet.\n\r",ch);
	return;
    }
    if (ch->pcdata->celerity > 1)
    {
	stc("You are already using increased speed!\n\r",ch);
	return;
    }
    send_to_char("You start moving with supernatural speed!\n\r",ch);
    act("$n starts moving with supernatural speed!",ch,NULL,NULL,TO_ROOM);
    ch->pcdata->celerity += 5 * get_highland(ch, HIGH_ABILITY);
    WAIT_STATE(ch,4);
    return;
}

void do_judgement(CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg [MAX_INPUT_LENGTH];
    char buf [MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (!IS_HIGHLANDER(ch))
    {
	stc("Huh?\n\r",ch);
	return;
    }
    if (get_highland(ch, HIGH_ABILITY) < 5)
    {
	stc("You have not learned the Powers ability to level 5 yet.\n\r",ch);
	return;
    }
    if (arg[0] == '\0')
    {
	stc("Judge whom?\n\r",ch);
	return;
    }
    if ((victim = get_char_room(ch,arg)) == NULL)
    {
	stc("They aren't here.\n\r",ch);
	return;
    }

    act("You examine $N intently.",ch,NULL,victim,TO_CHAR);
    act("$n examines $N intently.",ch,NULL,victim,TO_NOTVICT);
    act("$n examines you intently.",ch,NULL,victim,TO_VICT);

    if (IS_NPC(victim)) sprintf(buf, "%s is an NPC.\n\r",victim->short_descr);
    else if (IS_ST(victim)) sprintf(buf, "%s is a Story Teller.\n\r", victim->name);
    else if (IS_IMP(victim)) sprintf(buf, "%s is an Implementor.\n\r", victim->name);
    else if (IS_IMM(victim)) sprintf(buf, "%s is an Immortal.\n\r", victim->name);
    else if (IS_BUI(victim)) sprintf(buf, "%s is a Builder.\n\r", victim->name);
    else if (IS_AWA(victim)) sprintf(buf, "%s is Awakened.\n\r", victim->name);
    else sprintf(buf, "%s is a Sleeper.\n\r", victim->name);
    stc(buf,ch);

    sprintf(buf, "Health: %li/%li\n\r", (victim->hit/2000) * 2000, (victim->max_hit/2000) * 2000);
    stc(buf,ch);
    sprintf(buf, "Magic:  %li/%li\n\r", (victim->mana/2000) * 2000, (victim->max_mana/2000) * 2000);
    stc(buf,ch);
    sprintf(buf, "Armor:  %li.\n\r",char_ac(victim));
    stc(buf,ch);
    return;
}

void do_lunge( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument(argument, arg);

    if (IS_NPC(ch)) return;

    if (!IS_HIGHLANDER(ch))
    {
	stc("Huh?\n\r",ch );
	return;
    }
    if (get_highland(ch, HIGH_ABILITY) < 6)
    {
	stc("You have not learned this attack.\n\r",ch);
	return;
    }
    if ((victim = get_char_room(ch,arg)) == NULL)
    {
	stc("They aren't here.\n\r",ch);
	return;
    }
    if (victim == ch)
    {
	stc("You can't lunge yourself.\n\r", ch);
	return;
    } 
    if (is_safe(ch,victim)) return;

    act("You lunge at $N, catching $M off guard!", ch, NULL, victim, TO_CHAR);
    act("$n lunges at you, catching you off guard!", ch, NULL, victim, TO_VICT);
    act("$n lunges at $N, catching $M off guard!", ch, NULL, victim, TO_NOTVICT);

    if (victim->position != POS_FIGHTING) set_fighting(victim,ch);
    if (ch->position != POS_FIGHTING) set_fighting(ch,victim);
    do_stance(victim,"");
    WAIT_STATE(ch, 8);
    return;   
}

void do_forge( CHAR_DATA *ch, char *argument )
{
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    OBJ_DATA *invobj;
    char arg[MAX_STRING_LENGTH];
    long vnum = 29650, value;

    one_argument(argument,arg);

    if (IS_NPC(ch)) return;

    if (!IS_HIGHLANDER(ch))
    {
	stc("Huh?\n\r",ch);
	return;
    }

    if (ch->primal < 50 || ch->pcdata->quest < 100)
    {
        stc("It costs 50 points of primal and 100 quest points to create a katana.\n\r",ch);
        return;
    }

    if ((pObjIndex = get_obj_index(vnum)) == NULL)
    {
	stc("Missing object, inform Akuma.\n\r",ch);
	return;
    }

    if (arg[0] == '\0')
    {
	obj = create_object(pObjIndex, 50);
	obj_to_char(obj, ch);
	ch->primal -= 50;
	ch->pcdata->quest -= 100;
	SET_BIT(obj->quest, QUEST_AGGRAVATED);
	free_string( obj->questmaker );
	obj->questmaker = str_dup( ch->name );
	free_string( obj->questowner );
	obj->questowner = str_dup( ch->name );
	act("$p appears in your hands.",ch,obj,NULL,TO_CHAR);
	act("$p appears in $n's hands.",ch,obj,NULL,TO_ROOM);
    }
    else
    {
	if ((invobj = get_obj_carry(ch,arg)) == NULL)
	{
	    stc("You don't have that object.\n\r",ch);
	    return;
	}
	if (invobj->item_type != ITEM_WEAPON)
	{
	    stc("That isn't a weapon!\n\r",ch);
	    return;
	}
	if (IS_SET(invobj->quest, QUEST_ARTIFACT) || IS_SET(invobj->quest, QUEST_RELIC))
	{
	    stc("You can't!.\n\r",ch);
	    return;
	}
	if (invobj->questowner == NULL)
	{
	    stc("Claim it first.\n\r",ch);
	    return;
	}
	if (str_cmp(ch->name,invobj->questowner))
	{
	    stc("It's not yours!\n\r",ch);
	    return;
	}
	obj = create_object(pObjIndex, 50);
	obj_to_char(obj, ch);
	ch->primal -= 50;
	ch->pcdata->quest -= 100;
	free_string( obj->questmaker );
	obj->questmaker = str_dup( ch->name );
	free_string( obj->questowner );
	obj->questowner = str_dup( ch->name );
	SET_BIT(obj->quest, QUEST_AGGRAVATED);
	for (value = 0;value <= 4;value++)
	    obj->value[value] = invobj->value[value];
	act("You reforge $p.",ch,invobj,NULL,TO_CHAR);
	act("$n reforges $p.",ch,invobj,NULL,TO_ROOM);
	extract_obj(invobj);
    }
    return;
}
