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
 * Local functions.
 */
void	say_spell	args( ( CHAR_DATA *ch, long sn ) );



bool is_obj( CHAR_DATA *ch )
{
    OBJ_DATA *obj;

    if ((obj = ch->pcdata->chobj) == NULL)
    {
	stc("Huh?\n\r", ch);
	return FALSE;
    }
    if (obj->chobj == NULL || obj->chobj != ch)
    {
	stc("Huh?\n\r", ch);
	return FALSE;
    }
    return TRUE;
}



/*
 * Lookup a skill by name.
 */
long skill_lookup( const char *name )
{
    long sn;

    for (sn = 0; sn < MAX_SKILL; sn++)
    {
	if (skill_table[sn].name == NULL) break;
	if (LOWER(name[0]) == LOWER(skill_table[sn].name[0]) && !str_prefix( name, skill_table[sn].name)) return sn;
    }
    return -1;
}



/*
 * Lookup a skill by slot number.
 * Used for object loading.
 */
long slot_lookup( long slot )
{
    extern bool fBootDb;
    long sn;

    if (slot <= 0) return -1;

    for (sn = 0; sn < MAX_SKILL; sn++)
    {
	if (slot == skill_table[sn].slot) return sn;
    }

    if (fBootDb)
    {
	bug("Slot_lookup: bad slot %li.", slot);
	abort();
    }

    return -1;
}



long sn_lookup( long sn )
{
    long num;

    if (sn <= 0 || sn > MAX_SKILL) return 0;

    num = skill_table[sn].slot;

    return num;
}



/*
 * Utter mystical words for an sn.
 */
void say_spell( CHAR_DATA *ch, long sn )
{
    char color[5];
    char colorword[20];

    if (skill_table[sn].target == 0)
    {
	strcpy(colorword, "bright purple");
	strcpy(color, "#p");
    }
    else if (skill_table[sn].target == 1)
    {
	strcpy(colorword, "dark red");
	strcpy(color, "#r");
    }
    else if (skill_table[sn].target == 2)
    {
	strcpy(colorword, "deep blue");
	strcpy(color, "#l");
    }
    else if (skill_table[sn].target == 3)
    {
	strcpy(colorword, "neon green");
	strcpy(color, "#g");
    }
    else if (skill_table[sn].target == 4)
    {
	strcpy(colorword, "bright yellow");
	strcpy(color, "#y");
    }
    else if (skill_table[sn].target == 5)
    {
	strcpy(colorword, "jet black");
	strcpy(color, "#b");
    }

    act("$tYour eyes glow $T for a moment.", ch, color, colorword, TO_CHAR);
    act("$t$n$t's eyes glow $T for a moment.", ch, color, colorword, TO_ROOM);
    return;
}



/*
 * Compute a saving throw.
 * Negative apply's make saving throw better.
 */
bool saves_spell( long level, CHAR_DATA *victim )
{
    long save;
    long tsave;

    tsave = 5 + victim->stats[STAT_WIS] * 5;

    save = 50 + (tsave - level);
    save = URANGE(15, save, 85);
    return number_percent() < save;
}



/*
 * The kludgy global is for spells who want more stuff from command line.
 */
char *target_name;

void do_cast( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *casting_room;
    bool restored;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    void *vo;
    long mana;
    long sn;
    long level;
    long wait;

    if (IS_NPC(ch)) return;

    if (ch->in_room != NULL && IS_SET(ch->in_room->added_flags, ROOM2_SILENCE) && !IS_BUI(ch))
    {
	stc("Your words make no sound!\n\r", ch);
	return;
    }

    target_name = one_argument(argument, arg1);
    one_argument(target_name, arg2);

    if (arg1[0] == '\0')
    {
	stc("Cast which what where?\n\r", ch);
	return;
    }

    if ((sn = skill_lookup(arg1)) < 0 || (!IS_NPC(ch) && ch->level < skill_table[sn].skill_level))
    {
	stc("You can't do that.\n\r", ch);
	return;
    }
  
    restored = skill_table[sn].spell_fun == spell_control_weather
        || skill_table[sn].spell_fun == spell_energy_drain;
    if (restored && (ch->wait > 0 || ch->combat_pause > 0))
    {
        stc("You have not recovered enough to cast again.\n\r", ch);
        return;
    }
    if (skill_table[sn].spell_fun == spell_control_weather
        && str_cmp(target_name, "better") && str_cmp(target_name, "worse"))
    {
        stc("Use cast 'control weather' better or worse.\n\r", ch);
        return;
    }
    mana = skill_table[sn].min_mana;

    if (IS_SET(ch->act, PLR_WOLFMAN)) mana *= 2;

    /*
     * Locate targets.
     */
    victim	= NULL;
    obj		= NULL;
    vo		= NULL;
      
    switch ( skill_table[sn].target )
    {
	default:
	    bug("Do_cast: bad target for sn %li.", sn);
	    return;

	case TAR_IGNORE:
	    break;

	case TAR_CHAR_OFFENSIVE: case TAR_DEATH:
	    if (arg2[0] == '\0')
	    {
		if ((victim = ch->fighting) == NULL)
		{
		    stc("Cast the spell on whom?\n\r", ch);
		    return;
		}
	    }
	    else
	    {
		if ((victim = get_char_room(ch, arg2)) == NULL)
		{
		    stc("They aren't here.\n\r", ch);
		    return;
		}
	    }

    if (ch == victim && skill_table[sn].spell_fun == spell_energy_drain)
    {
        stc("You cannot drain your own energy.\n\r", ch);
        return;
    }
    if (ch == victim) stc("Cast this on yourself? Ok...\n\r", ch);

	    if (is_safe(ch, victim)) return;

	    if (IS_ITEMAFF(ch, ITEMA_PEACE)) do_majesty(ch, "");

	    vo = (void *) victim;
	    break;

	case TAR_CHAR_DEFENSIVE:
	    if (arg2[0] == '\0') victim = ch;
	    else
	    {
		if ((victim = get_char_room(ch, arg2)) == NULL)
		{
		    stc("They aren't here.\n\r", ch);
		    return;
		}
	    }
	    vo = (void *) victim;
	    break;

	case TAR_CHAR_SELF:
	    if (arg2[0] != '\0' && !is_name(arg2, ch->name))
	    {
		stc("You cannot cast this spell on another.\n\r", ch);
		return;
	    }

	    vo = (void *) ch;
	    break;

	case TAR_OBJ_INV:
	    if (arg2[0] == '\0')
	    {
		stc("What should the spell be cast upon?\n\r", ch);
		return;
	    }

	    if ((obj = get_obj_carry(ch, arg2)) == NULL)
	    {
		stc("You are not carrying that.\n\r", ch);
		return;
	    }

	    vo = (void *) obj;
	    break;
    }
	    
    if (ch->mana < mana)
    {
	stc("You don't have enough mana.\n\r", ch);
	return;
    }

    if (IS_AFFECTED(ch, AFF_KAKUSU) || IS_ITEMAFF(ch, ITEMA_BLISSFUL))
    {
	REMOVE_BIT(ch->affected_by, AFF_KAKUSU);
	REMOVE_BIT(ch->itemaffect, ITEMA_BLISSFUL);
	stc("Your reveal your presence.\n\r", ch);
    }
    say_spell(ch, sn);

    wait = skill_table[sn].beats;
    wait -= ch->stats[STAT_INT] / 2;
    if (IS_SORCERER(ch)) wait *= 1 - (get_color(ch, skill_table[sn].target) * .1);
    else if (IS_DRAGON(ch) && IS_DRAGAFF(ch, DRAG_MAGIC)) wait *= .7;
    else if (IS_ELEM(ch) && (ch->pcdata->powers[ELEM_TIME] >= 8)) wait *= .7;

    if (restored) wait = UMAX(1, wait);
    WAIT_STATE(ch, wait);

    ch->mana -= mana;

    level = restored ? 5 + URANGE(0, ch->stats[STAT_INT], 19) * 5
        : 5 + ch->stats[STAT_INT] * 5;

    casting_room = ch->in_room;
    (*skill_table[sn].spell_fun) (sn, level, ch, vo);

    if ((skill_table[sn].target == TAR_DEATH || skill_table[sn].target == TAR_CHAR_OFFENSIVE) && victim != ch
     && combat_pair_in_room(casting_room, ch, victim)
     && victim->fighting == NULL && IS_AWAKE(victim))
	multi_hit(victim, ch);

    return;
}



/*
 * Cast spells at targets using a magical object.
 */
void obj_cast_spell( long sn, long level, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj )
{
    ROOM_INDEX_DATA *casting_room;
    void *vo;

    if ( sn <= 0 )
	return;

    if ( sn >= MAX_SKILL || skill_table[sn].spell_fun == 0 )
    {
	bug( "Obj_cast_spell: bad sn %li.", sn );
	return;
    }

    switch ( skill_table[sn].target )
    {
    default:
	bug( "Obj_cast_spell: bad target for sn %li.", sn );
	return;

    case TAR_IGNORE:
	vo = NULL;
	break;

    case TAR_CHAR_OFFENSIVE:
	if ( victim == NULL )
	    victim = ch->fighting;
	if ( victim == NULL )
	{
	    send_to_char( "You can't do that.\n\r", ch );
	    return;
	}
	if (is_safe(ch, victim)) return;
	if (!IS_NPC(ch) && IS_ITEMAFF(ch, ITEMA_PEACE)) do_majesty(ch,"");
	vo = (void *) victim;
	break;

    case TAR_CHAR_DEFENSIVE:
	if ( victim == NULL )
	    victim = ch;
	vo = (void *) victim;
	break;

    case TAR_CHAR_SELF:
	vo = (void *) ch;
	break;

    case TAR_OBJ_INV:
	if ( obj == NULL )
	{
	    send_to_char( "You can't do that.\n\r", ch );
	    return;
	}
	vo = (void *) obj;
	break;

    case TAR_DEATH:
        if ( victim == NULL )
            victim = ch->fighting;
        if ( victim == NULL )
        {
            send_to_char( "You can't do that.\n\r", ch );
            return;
        }
	if (is_safe(ch, victim)) return;
        if (!IS_NPC(ch) && IS_ITEMAFF(ch, ITEMA_PEACE)) do_majesty(ch,"");
        vo = (void *) victim;
        break;
    }

    target_name = "";
    casting_room = ch->in_room;
    WAIT_STATE(ch, skill_table[sn].beats);
    (*skill_table[sn].spell_fun) ( sn, level, ch, vo );

    if ((skill_table[sn].target == TAR_CHAR_OFFENSIVE || skill_table[sn].target == TAR_DEATH) && victim != ch
     && combat_pair_in_room(casting_room, ch, victim)
     && victim->fighting == NULL && IS_AWAKE(victim))
	multi_hit(victim, ch);

    return;
}



/*
 * Spell functions.
 */
void spell_acid_blast( long sn, long level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    long dam;

    if (IS_ITEMAFF(victim, ITEMA_ACIDSHIELD)) return;
    if (!IS_NPC(victim) && victim->pcdata->atm >= number_percent())
    {
	send_to_char("Nothing happens.\n\r",ch);
	return;
    }

    dam = dice(level, 50);
    if (saves_spell(level, victim)) dam /= 2;

    damage(ch, victim, dam, TYPE_SPELL, "acid blast");

    return;
}



void spell_armor( long sn, long level, CHAR_DATA *ch, void *vo )
{
    return;
}



void spell_bless( long sn, long level, CHAR_DATA *ch, void *vo )
{
    return;
}



void spell_blindness( long sn, long level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    char buf [MAX_INPUT_LENGTH];

    if ( IS_AFFECTED(victim, AFF_BLIND) || saves_spell( level, victim ) )
    {
	send_to_char("Nothing happens.\n\r",ch);
	return;
    }
    if (!IS_NPC(victim) && victim->pcdata->atm >= number_percent())
    {
	send_to_char("Nothing happens.\n\r",ch);
	return;
    }

    af.type      = sn;
    af.location  = APPLY_HITROLL;
    af.modifier  = -4;
    af.duration  = 1+level;
    af.bitvector = AFF_BLIND;
    affect_to_char( victim, &af );
    send_to_char( "You are blinded!\n\r", victim );
    if (!IS_NPC(victim))
	sprintf(buf,"%s is blinded!\n\r",victim->name);
    else
	sprintf(buf,"%s is blinded!\n\r",victim->short_descr);
    send_to_char(buf,ch);
    return;
}



void spell_burning_hands( long sn, long level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const long dam_each[] = 
    {
	 4,
	 6,  8, 10, 12,	14,	17, 20, 23, 26, 29,
	29, 29, 30, 30,	31,	31, 32, 32, 33, 33,
	34, 34, 35, 35,	36,	36, 37, 37, 38, 38,
	39, 39, 40, 40,	41,	41, 42, 42, 43, 43,
	44, 44, 45, 45,	46,	46, 47, 47, 48, 48,
	50, 60, 70, 80,	90,	100,125,150,175,200
    };
    long dam;
    long hp;

    if (IS_ITEMAFF(victim, ITEMA_FIRESHIELD)) return;

    if (!IS_NPC(victim) && victim->pcdata->atm >= number_percent())
    {
	send_to_char("Nothing happens.\n\r",ch);
	return;
    }

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell( level, victim ) ) dam >>= 1;
    hp = victim->hit;
    if (!IS_NPC(victim) && IS_VAMPIRE(victim) )
    	damage( ch, victim, (dam*2), TYPE_SPELL, "burning hands");
    else
    	damage( ch, victim, dam, TYPE_SPELL, "burning hands" );
    if (victim == NULL || victim->position == POS_DEAD) return;
    if (!IS_NPC(victim) && IS_WEREWOLF(victim) && IS_MORE(ch,MORE_HELIOS))
	victim->hit = hp;
    update_pos(victim);
    return;
}



void spell_call_lightning( long sn, long level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    long dam;
    long hp;

    if ( !IS_OUTSIDE(ch) )
    {
	send_to_char( "You must be out of doors.\n\r", ch );
	return;
    }

    if ( weather_info.sky < SKY_RAINING )
    {
	send_to_char( "You need bad weather.\n\r", ch );
	return;
    }

    dam = dice(level/2, 8);

    send_to_char( "God's lightning strikes your foes!\n\r", ch );
    act( "$n calls God's lightning to strike $s foes!",
	ch, NULL, NULL, TO_ROOM );

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
	vch_next	= vch->next;
	if ( vch->in_room == NULL )
	    continue;
	if ( vch->in_room == ch->in_room )
	{
	    if ( vch != ch && ( IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch) ) )
	    {
		if (IS_ITEMAFF(vch, ITEMA_SHOCKSHIELD)) continue;
    		if ( saves_spell( level, vch ) )
		    dam /= 2;
    		hp = vch->hit;
    		damage( ch, vch, dam, TYPE_SPELL, "lightning blast" );
		if (vch == NULL || vch->position == POS_DEAD) continue;
	    }
	    continue;
	}

	if ( vch->in_room->area == ch->in_room->area
	&&   IS_OUTSIDE(vch)
	&&   IS_AWAKE(vch) )
	    send_to_char( "Lightning flashes in the sky.\n\r", vch );
    }
    return;
}



void spell_cause_critical( long sn, long level, CHAR_DATA *ch, void *vo )
{
    long dam = number_range(4 * level, 8 * level);
    damage(ch, (CHAR_DATA *) vo, dam, TYPE_SPELL, "cause critical" );
    return;
}



void spell_cause_light( long sn, long level, CHAR_DATA *ch, void *vo )
{
    long dam = number_range(level, 2 * level);
    damage(ch, (CHAR_DATA *) vo, dam, TYPE_SPELL, "cause light");
    return;
}



void spell_cause_serious( long sn, long level, CHAR_DATA *ch, void *vo )
{
    long dam = number_range(2 * level, 4 * level);
    damage(ch, (CHAR_DATA *) vo, dam, TYPE_SPELL, "cause serious" );
    return;
}



void spell_change_sex( long sn, long level, CHAR_DATA *ch, void *vo )
{
    return;
}



void spell_charm_person( long sn, long level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (victim == ch)
    {
	stc("You like yourself even better!\n\r", ch);
	return;
    }

    if (!IS_NPC(victim) || IS_AFFECTED(ch, AFF_CHARM))
    {
	stc("You failed.\n\r", ch);
	return;
    }

    if (IS_AFFECTED(victim, AFF_CHARM) || level < (victim->level / 10) || saves_spell(level, victim))
	return;

    if (victim->master) stop_follower(victim);

    add_follower(victim, ch);
    af.type      = sn;
    af.duration  = number_fuzzy(level / 4);
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char(victim, &af);
    act("Isn't $n just so nice?", ch, NULL, victim, TO_VICT);
    if (ch != victim) stc("Ok.\n\r", ch);
    return;
}



void spell_chill_touch( long sn, long level, CHAR_DATA *ch, void *vo )
{
    AFFECT_DATA af;
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    long dam;

    if (IS_ITEMAFF(victim, ITEMA_ICESHIELD)) return;
    if (!IS_NPC(victim) && victim->pcdata->atm >= number_percent())
    {
	stc("Nothing happens.\n\r", ch);
	return;
    }

    dam = number_range(level * 10, level * 20);
    if (!saves_spell(level, victim) && !IS_VAMPIRE(victim))
    {
	af.type      = sn;
	af.duration  = 6;
	af.location  = APPLY_STR;
	af.modifier  = -1;
	af.bitvector = 0;
	affect_join(victim, &af);
    }
    else dam /= 2;

    damage(ch, victim, dam, TYPE_SPELL, "chill touch");
    return;
}



void spell_colour_spray( long sn, long level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    long dam;

    if (!IS_NPC(victim) && victim->pcdata->atm >= number_percent())
    {
	stc("Nothing happens.\n\r", ch);
	return;
    }

    dam = number_range(level * 15, level * 30);
    if (saves_spell(level, victim)) dam /= 2;

    damage(ch, victim, dam, TYPE_SPELL, "color spray");
    return;
}



void spell_continual_light( long sn, long level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *light;

    light = create_object(get_obj_index( OBJ_VNUM_LIGHT_BALL), 0);
    obj_to_room(light, ch->in_room);
    act("$n twiddles $s thumbs and $p appears.",   ch, light, NULL, TO_ROOM);
    act("You twiddle your thumbs and $p appears.", ch, light, NULL, TO_CHAR);
    return;
}



void spell_control_weather( long sn, long level, CHAR_DATA *ch, void *vo )
{
    long change;
    (void)sn; (void)vo;
    if (str_cmp(target_name, "better") && str_cmp(target_name, "worse"))
    {
        stc("Use cast 'control weather' better or worse.\n\r", ch);
        return;
    }
    /* EM's weather-trend effect, with bounded arithmetic and no empty cast. */
    change = dice(UMAX(1, URANGE(1, level, 100) / 3), 4);
    if (!str_cmp(target_name, "worse")) change = -change;
    weather_info.change = URANGE(-12, weather_info.change, 12) + change;
    stc("You bend the weather toward your will.\n\r", ch);
}



void spell_create_food( long sn, long level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *mushroom;

    mushroom = create_object(get_obj_index( OBJ_VNUM_MUSHROOM ), 0);
    mushroom->value[0] = 5 + level;
    obj_to_room( mushroom, ch->in_room);

    act("$p suddenly appears.", ch, mushroom, NULL, TO_ROOM);
    act("$p suddenly appears.", ch, mushroom, NULL, TO_CHAR);
    return;
}



void spell_create_spring( long sn, long level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *spring;

    spring = create_object(get_obj_index(OBJ_VNUM_SPRING), 0);
    spring->timer = level * 2;
    obj_to_room(spring, ch->in_room);

    act( "$p flows from the ground.", ch, spring, NULL, TO_ROOM );
    act( "$p flows from the ground.", ch, spring, NULL, TO_CHAR );
    return;
}



void spell_create_water( long sn, long level, CHAR_DATA *ch, void *vo )
{
    return;
}



void spell_cure_blindness( long sn, long level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if (!is_affected(victim, gsn_blindness)) return;
    affect_strip(victim, gsn_blindness);

    stc("Your vision returns!\n\r", victim);
    if (ch != victim) stc("Ok.\n\r", ch);
    return;
}



void spell_cure_critical( long sn, long level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    long heal;

    heal = number_range(4 * level, 8 * level);
    if (victim->position == POS_FIGHTING) heal /= 2;
    victim->hit = UMIN(victim->hit + heal, victim->max_hit);
    update_pos(victim);

    stc("You feel better!\n\r", victim);
    if (ch != victim) stc("Ok.\n\r", ch);
    return;
}


void spell_cure_light( long sn, long level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    long heal;

    heal = number_range(level, 2 * level);
    if (victim->position == POS_FIGHTING) heal /= 2;
    victim->hit = UMIN(victim->hit + heal, victim->max_hit);
    update_pos(victim);

    stc("You feel better!\n\r", victim);
    if (ch != victim) stc("Ok.\n\r", ch);
    return;
}


void spell_cure_poison( long sn, long level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if (!is_affected(victim, gsn_poison)) return;
    affect_strip(victim, gsn_poison);

    act("$N looks healthier.", ch, NULL, victim, TO_NOTVICT);
    stc("A warm feeling runs through your body.\n\r", victim);
    stc("Ok.\n\r", ch);
    return;
}


void spell_cure_serious( long sn, long level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    long heal;

    heal = number_range(2 * level, 4 * level);
    if (victim->position == POS_FIGHTING) heal /= 2;
    victim->hit = UMIN(victim->hit + heal, victim->max_hit);
    update_pos(victim);

    stc("You feel better!\n\r", victim);
    if (ch != victim) stc("Ok.\n\r", ch);
    return;
}


void spell_curse( long sn, long level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_AFFECTED(victim, AFF_CURSE) || saves_spell(level, victim))
    {
	stc("Nothing happens.\n\r", ch);
	return;
    }
    if (!IS_NPC(victim) && victim->pcdata->atm >= number_percent())
    {
	stc("Nothing happens.\n\r", ch);
	return;
    }

    af.type      = sn;
    af.duration  = 4 * level;
    af.location  = APPLY_HITROLL;
    af.modifier  = -1 * level;
    af.bitvector = AFF_CURSE;
    affect_to_char(victim, &af);
    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = level;
    affect_to_char(victim, &af);

    stc("You feel unclean.\n\r", victim);
    if (ch != victim) stc("Ok.\n\r", ch);
    return;
}



void spell_detect_evil( long sn, long level, CHAR_DATA *ch, void *vo )
{
    return;
}



void spell_detect_hidden( long sn, long level, CHAR_DATA *ch, void *vo )
{
    return;
}



void spell_detect_invis( long sn, long level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_AFFECTED(victim, AFF_DETECT_INVIS)) return;

    af.type		= sn;
    af.duration		= level;
    af.modifier		= 0;
    af.location		= APPLY_NONE;
    af.bitvector	= AFF_DETECT_INVIS;

    affect_to_char(victim, &af);
    stc("Your eyes tingle.\n\r", victim);
    if (ch != victim) stc("Ok.\n\r", ch);
    return;
}



void spell_detect_magic( long sn, long level, CHAR_DATA *ch, void *vo )
{
    return;
}



void spell_detect_poison( long sn, long level, CHAR_DATA *ch, void *vo )
{
    return;
}



void spell_dispel_evil( long sn, long level, CHAR_DATA *ch, void *vo )
{
    return;
}



void spell_dispel_magic( long sn, long level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if (victim->position == POS_STUNNED) update_pos(victim);

    if (victim != ch && (saves_spell(level, victim) || IS_DRAGON(victim)))
    {
	stc("You failed.\n\r", ch);
	return;
    }

    if (!IS_NPC(victim) && victim->pcdata->atm >= number_percent() && ch != victim)
    {
	stc("Nothing happens.\n\r", ch);
	return;
    }

    if (!(victim->affected) && !IS_SET(victim->more, MORE_THINBLOOD) && !IS_AFFECTED(victim, AFF_FLAMING))
    {
	stc("Nothing happens.\n\r", ch);
	return;
    }

    while (victim->affected) affect_remove(victim, victim->affected);
    REMOVE_BIT(victim->affected_by, AFF_FLAMING);
    REMOVE_BIT(victim->more, MORE_THINBLOOD);

    if (ch == victim)
    {
	act("You remove all magical affects from yourself.", ch, NULL, victim, TO_CHAR);
	act("$n has removed all magical affects from $mself.", ch, NULL, victim, TO_NOTVICT);
    }
    else
    {
	act("You remove all magical affects from $N.", ch, NULL, victim, TO_CHAR);
	act("$n has removed all magical affects from $N.", ch, NULL, victim, TO_NOTVICT);
	act("$n has removed all magical affects from you.", ch, NULL, victim, TO_VICT);
    }

    return;
}



void spell_earthquake( long sn, long level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;

    stc("The earth trembles beneath your feet!\n\r", ch);
    act("$n makes the earth tremble and shiver.", ch, NULL, NULL, TO_ROOM);

    for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
    {
	vch_next = vch->next_in_room;

	if (IS_NPC(ch) == IS_NPC(vch)) continue;
	if (IS_AFFECTED(ch, AFF_FLYING)) continue;

	damage(ch, vch, number_range(10 * level, 20 * level), TYPE_SPELL, "earthquake");
    }

    return;
}



void spell_enchant_weapon( long sn, long level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA *paf;

    if (obj->item_type != ITEM_WEAPON || IS_SET(obj->quest, QUEST_ENCHANTED)
     || IS_SET(obj->quest, QUEST_ARTIFACT) || IS_SET(obj->quest, QUEST_RELIC) || obj->chobj != NULL)
    {
	stc("You are unable to enchant this weapon.\n\r", ch);
	return;
    }

    if (affect_free == NULL)
    {
	paf		= alloc_perm(sizeof(*paf));
    }
    else
    {
	paf		= affect_free;
	affect_free	= affect_free->next;
    }
    paf->type		= -1;
    paf->duration	= -1;
    paf->location	= APPLY_HITROLL;
    paf->modifier	= level / 6;
    paf->bitvector	= 0;
    paf->next		= obj->affected;
    obj->affected	= paf;

    if (affect_free == NULL)
    {
	paf		= alloc_perm(sizeof(*paf));
    }
    else
    {
	paf		= affect_free;
	affect_free	= affect_free->next;
    }
    paf->type		= -1;
    paf->duration	= -1;
    paf->location	= APPLY_DAMROLL;
    paf->modifier	= level / 6;
    paf->bitvector	= 0;
    paf->next		= obj->affected;
    obj->affected	= paf;

    SET_BIT(obj->quest, QUEST_ENCHANTED);
    act("$p glows blue.", ch, obj, NULL, TO_CHAR);
    act("$p glows blue.", ch, obj, NULL, TO_ROOM);
    return;
}


void spell_enchant_armor( long sn, long level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA *paf;

    if (obj->item_type != ITEM_ARMOR || IS_SET(obj->quest, QUEST_ENCHANTED)
     || IS_SET(obj->quest, QUEST_ARTIFACT) || IS_SET(obj->quest, QUEST_RELIC) || obj->chobj != NULL)
    {
	stc("You are unable to enchant this armor.\n\r", ch);
	return;
    }

    if (affect_free == NULL)
    {
	paf		= alloc_perm(sizeof(*paf));
    }
    else
    {
	paf		= affect_free;
	affect_free	= affect_free->next;
    }
    paf->type		= sn;
    paf->duration	= -1;
    paf->location	= APPLY_HITROLL;
    paf->modifier	= level / 6;
    paf->bitvector	= 0;
    paf->next		= obj->affected;
    obj->affected	= paf;
 
    if (affect_free == NULL)
    {
	paf		= alloc_perm(sizeof(*paf));
    }
    else
    {
	paf		= affect_free;
	affect_free	= affect_free->next;
    }
    paf->type		= sn;
    paf->duration	= -1;
    paf->location	= APPLY_DAMROLL;
    paf->modifier	= level / 6;
    paf->bitvector	= 0;
    paf->next		= obj->affected;
    obj->affected	= paf;
 
    SET_BIT(obj->quest, QUEST_ENCHANTED);
    act("$p glows blue.", ch, obj, NULL, TO_CHAR);
    act("$p glows blue.", ch, obj, NULL, TO_ROOM);
    return;
}



void spell_energy_drain( long sn, long level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = vo;
    ROOM_INDEX_DATA *origin;
    long before, dealt, dam, drain;
    if (ch == NULL || ch == victim) return;
    origin = ch->in_room;
    if (!combat_pair_in_room(origin, ch, victim) || is_safe(ch, victim)) return;
    level = URANGE(1, level, 100);
    if (saves_spell(level, victim)
        || (!IS_NPC(victim) && victim->pcdata->atm >= number_percent()))
    {
        stc("Your target resists the energy drain.\n\r", ch);
        return;
    }
    /* EM's ordinary die roll, not its caster-HP attack against low levels.
     * Genesis has no corresponding IMM_DRAIN/Hero/Demon mechanics to import. */
    before = victim->hit;
    dam = number_range(1, level);
    dam = UMIN(dam, before);
    damage(ch, victim, dam, sn, "energy drain");
    /* Damage can reflect, recall, incapacitate or extract either participant.
     * Grant benefits only from confirmed damage to a surviving local target. */
    if (!combat_pair_in_room(origin, ch, victim) || victim->hit >= before) return;
    dealt = before - victim->hit;
    drain = number_range(1, 10);
    if (victim->mana > 0) victim->mana -= UMIN(victim->mana, drain);
    /* Genesis has no movement resource; do not import EM's move drain. */
    if (ch->hit < ch->max_hit) ch->hit += UMIN(dealt, ch->max_hit - ch->hit);
}



void spell_fireball( long sn, long level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    long dam;

    if (IS_ITEMAFF(victim, ITEMA_FIRESHIELD)) return;
    if (IS_WEREWOLF(victim) && IS_MORE(ch, MORE_HELIOS)) return;
    if (!IS_NPC(victim) && victim->pcdata->atm >= number_percent())
    {
	stc("Nothing happens.\n\r", ch);
	return;
    }

    dam = number_range(level * 20, level * 40);
    if (saves_spell(level, victim)) dam /= 2;

    damage(ch, victim, dam, TYPE_SPELL, "fireball");
    return;
}



void spell_flamestrike( long sn, long level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    long dam;
    long hp;

    if (IS_ITEMAFF(victim, ITEMA_FIRESHIELD)) return;
    if (!IS_NPC(victim) && victim->pcdata->atm >= number_percent())
    {
	send_to_char("Nothing happens.\n\r",ch);
	return;
    }
    dam = dice(6, 8);
    if ( saves_spell( level, victim ) )
	dam /= 2;
    hp = victim->hit;
    	damage( ch, victim, dam, TYPE_SPELL, "flamestrike" );
    if (victim == NULL || victim->position == POS_DEAD) return;
    if (!IS_NPC(victim) && IS_WEREWOLF(victim) && IS_MORE(ch,MORE_HELIOS))               
	victim->hit = hp;
    update_pos(victim);
    return;
}



void spell_faerie_fire( long sn, long level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_AFFECTED(victim, AFF_FAERIE_FIRE))
    {
	stc("Nothing happens.\n\r", ch);
	return;
    }
    if (!IS_NPC(victim) && victim->pcdata->atm >= number_percent())
    {
	stc("Nothing happens.\n\r", ch);
	return;
    }

    af.type		= sn;
    af.duration		= level;
    af.location		= APPLY_AC;
    af.modifier		= -2 * level;
    af.bitvector	= AFF_FAERIE_FIRE;
    affect_to_char(victim, &af);

    stc("You are surrounded by a pink outline.\n\r", victim);
    act("$n is surrounded by a pink outline.", victim, NULL, NULL, TO_ROOM);
    return;
}



void spell_faerie_fog( long sn, long level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *ich;

    act( "$n conjures a cloud of purple smoke.", ch, NULL, NULL, TO_ROOM );
    send_to_char( "You conjure a cloud of purple smoke.\n\r", ch );

    for ( ich = ch->in_room->people; ich != NULL; ich = ich->next_in_room )
    {
	if (IS_IMM(ich) || ich == ch)
	    continue;
	if (IS_VAMPIRE(ich) && get_disc(ich, DISC_OBFUSCATE) >= 7) continue;

	do_visible(ich, "fogged");
	act("$n is revealed!", ich, NULL, NULL, TO_ROOM);
	send_to_char("You are revealed!\n\r", ich);
    }
    return;
}



void spell_fly( long sn, long level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_FLYING) )
	return;
    af.type      = sn;
    af.duration  = level + 3;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_FLYING;
    affect_to_char( victim, &af );
    send_to_char( "You rise up off the ground.\n\r", victim );
    act( "$n rises up off the ground.", victim, NULL, NULL, TO_ROOM );
    return;
}



void spell_gate( long sn, long level, CHAR_DATA *ch, void *vo )
{
    char_to_room(create_mobile(get_mob_index(3404)), ch->in_room);
    return;
}



/*
 * Spell for mega1.are from Glop/Erkenbrand.
 */
void spell_general_purpose( long sn, long level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    long dam;

    dam = number_range( 25, 100 );
    if ( saves_spell( level, victim ) )
	dam /= 2;
    damage( ch, victim, dam, TYPE_BLUNT, "explosive ammo" );
    return;
}



void spell_giant_strength( long sn, long level, CHAR_DATA *ch, void *vo )
{
    return;
}



void spell_harm( long sn, long level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    long dam;

    if (!IS_NPC(victim) && victim->pcdata->atm >= number_percent())
    {
	send_to_char("Nothing happens.\n\r",ch);
	return;
    }

    dam = number_range(8*level, 16*level);

    damage( ch, victim, dam, TYPE_SPELL, "harm" );
    return;
}



void spell_heal( long sn, long level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    long heal = number_range(8*level, 16*level);

    if (victim->position == POS_FIGHTING) heal /= 2;

    victim->hit = UMIN(victim->hit + heal, victim->max_hit);
    update_pos(victim);

    stc("A warm feeling fills your body.\n\r", victim);
    if (ch == victim) act( "$n heals $mself.", ch, NULL, NULL, TO_ROOM);
    else act( "$n heals $N.", ch, NULL, victim, TO_NOTVICT);
    if (ch != victim) stc("Ok.\n\r", ch);
    return;
}



/*
 * Spell for mega1.are from Glop/Erkenbrand.
 */
void spell_high_explosive( long sn, long level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    long dam;

    dam = number_range( 30, 120 );
    if ( saves_spell( level, victim ) )
	dam /= 2;
    damage( ch, victim, dam, TYPE_SPELL, "high explosive" );
    return;
}



void spell_identify( long sn, long level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    char buf[MAX_STRING_LENGTH];
    AFFECT_DATA *paf;
    long itemtype;

    act("You examine $p carefully.",ch,obj,NULL,TO_CHAR);
    act("$n examines $p carefully.",ch,obj,NULL,TO_ROOM);

    sprintf( buf,
	"Object '%s' is type %s, extra flags %s.\n\rWeight is %li, value is %li.\n\r",

	obj->name,
	item_type_name( obj->item_type ),	/* OLC */
	extra_bit_name( obj->extra_flags ),
	obj->weight,
	obj->cost
	);
    send_to_char( buf, ch );

    if (obj->points > 0 && obj->item_type != ITEM_QUEST
	 && obj->item_type != ITEM_PAGE)
    {
	sprintf( buf, "Quest point value is %li.\n\r", obj->points );
	send_to_char( buf, ch );
    }
    if (obj->questmaker != NULL && strlen(obj->questmaker) > 1 &&
	obj->questowner != NULL && strlen(obj->questowner) > 1 &&
	obj->item_type != ITEM_CORPSE_NPC &&
	obj->item_type != ITEM_CORPSE_PC)
    {
	sprintf( buf, "This object was created by %s, and is owned by %s.\n\r", obj->questmaker,obj->questowner );
	send_to_char( buf, ch );
    }
    else if (obj->questmaker != NULL && strlen(obj->questmaker) > 1 && 
	obj->item_type != ITEM_CORPSE_NPC && obj->item_type != ITEM_CORPSE_PC)
    {
	sprintf( buf, "This object was created by %s.\n\r", obj->questmaker );
	send_to_char( buf, ch );
    }
    else if (obj->questowner != NULL && strlen(obj->questowner) > 1 && 
	obj->item_type != ITEM_CORPSE_NPC && obj->item_type != ITEM_CORPSE_PC)
    {
	sprintf( buf, "This object is owned by %s.\n\r", obj->questowner );
	send_to_char( buf, ch );
    }

    if (obj->item_type != ITEM_PAGE)
    {
	if (IS_SET(obj->quest, QUEST_ENCHANTED))
	    send_to_char( "This item has been enchanted.\n\r", ch );
	if (IS_SET(obj->quest, QUEST_AGGRAVATED))
            send_to_char( "This weapon has been made aggravated.\n\r", ch );
	else if (IS_SET(obj->spectype, SITEM_SILVER))
	    send_to_char( "This item is crafted from gleaming silver.\n\r", ch );
    }

    switch ( obj->item_type )
    {
    case ITEM_PILL: 
    case ITEM_SCROLL: 
    case ITEM_POTION:
	sprintf( buf, "Level %li spells of:", obj->value[0] );
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
	sprintf( buf, "Quest point value is %li.\n\r", obj->value[0] );
	send_to_char( buf, ch );
	break;

    case ITEM_QUESTCARD:
	sprintf( buf, "Quest completion reward is %li quest points.\n\r", obj->level );
	send_to_char( buf, ch );
	break;

    case ITEM_WAND: 
    case ITEM_STAFF: 
	sprintf( buf, "Has %li(%li) charges of level %li",
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
	sprintf( buf, "Damage is %li to %li (average %li).\n\r",
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
		sprintf(buf,"%s is a minor spell weapon.\n\r",capitalize(obj->short_descr));
	    else if (obj->level < 20)
		sprintf(buf,"%s is a lesser spell weapon.\n\r",capitalize(obj->short_descr));
	    else if (obj->level < 30)
		sprintf(buf,"%s is an average spell weapon.\n\r",capitalize(obj->short_descr));
	    else if (obj->level < 40)
		sprintf(buf,"%s is a greater spell weapon.\n\r",capitalize(obj->short_descr));
	    else if (obj->level < 50)
		sprintf(buf,"%s is a major spell weapon.\n\r",capitalize(obj->short_descr));
	    else if (obj->level > 50)
		sprintf(buf,"%s is an ultimate spell weapon.\n\r",capitalize(obj->short_descr));
	    else
		sprintf(buf,"%s is a supreme spell weapon.\n\r",capitalize(obj->short_descr));
	    send_to_char(buf,ch);
	}

	if (itemtype == 1)
	    sprintf (buf, "This weapon is dripping with corrosive acid.\n\r");
	else if (itemtype == 4)
	    sprintf (buf, "This weapon radiates an aura of darkness.\n\r");
	else if (itemtype == 30)
	    sprintf (buf, "This ancient relic is the bane of all evil.\n\r");
	else if (itemtype == 34)
	    sprintf (buf, "This vampiric weapon drinks the souls of its victims.\n\r");
	else if (itemtype == 37)
	    sprintf (buf, "This weapon has been tempered in hellfire.\n\r");
	else if (itemtype == 48)
	    sprintf (buf, "This weapon crackles with sparks of lightning.\n\r");
	else if (itemtype == 53)
	    sprintf (buf, "This weapon is dripping with a dark poison.\n\r");
	else if (itemtype > 0)
	    sprintf (buf, "This weapon has been imbued with the power of %s.\n\r",skill_table[itemtype].name);
	if (itemtype > 0)
	    send_to_char( buf, ch );

	if (obj->value[0] >= 1000)
	    itemtype = obj->value[0] / 1000;
	else
	    break;

	if (itemtype == 4)
	    sprintf (buf, "This weapon radiates an aura of darkness.\n\r");
	else if (itemtype == 27 || itemtype == 2)
	    sprintf (buf, "This weapon allows the wielder to see invisible things.\n\r");
	else if (itemtype == 39 || itemtype == 3)
	    sprintf (buf, "This weapon grants the power of flight.\n\r");
	else if (itemtype == 45 || itemtype == 1)
	    sprintf (buf, "This weapon allows the wielder to see in the dark.\n\r");
	else if (itemtype == 46 || itemtype == 5)
	    sprintf (buf, "This weapon renders the wielder invisible to the human eye.\n\r");
	else if (itemtype == 52 || itemtype == 6)
	    sprintf (buf, "This weapon allows the wielder to walk through solid doors.\n\r");
	else if (itemtype == 54 || itemtype == 7)
	    sprintf (buf, "This holy weapon protects the wielder from evil.\n\r");
	else if (itemtype == 57 || itemtype == 8)
	    sprintf (buf, "This ancient weapon protects the wielder in combat.\n\r");
	else if (itemtype == 9)
	    sprintf (buf, "This crafty weapon allows the wielder to walk in complete silence.\n\r");
	else if (itemtype == 10)
	    sprintf (buf, "This powerful weapon surrounds its wielder with a shield of lightning.\n\r");
	else if (itemtype == 11)
	    sprintf (buf, "This powerful weapon surrounds its wielder with a shield of fire.\n\r");
	else if (itemtype == 12)
	    sprintf (buf, "This powerful weapon surrounds its wielder with a shield of ice.\n\r");
	else if (itemtype == 13)
	    sprintf (buf, "This powerful weapon surrounds its wielder with a shield of acid.\n\r");
	else if (itemtype == 14)
	    sprintf (buf, "This ancient weapon allows its wielder to move at impossible speeds.\n\r");
	else if (itemtype == 15)
	    sprintf (buf, "This razor sharp weapon can slice through armour without difficulty.\n\r");
	else if (itemtype == 16)
	    sprintf (buf, "This ancient weapon regenerates the wounds of its wielder.\n\r");
	else if (itemtype == 17)
	    sprintf (buf, "This ancient weapon allows its wielder to move at supernatural speed.\n\r");
	else if (itemtype == 18)
	    sprintf (buf, "This ancient weapon grants its wielder supernatural vision.\n\r");
	else if (itemtype == 19)
	    sprintf (buf, "This ancient weapon protects its wearer from player attacks.\n\r");
	else if (itemtype == 20)
	    sprintf (buf, "This ancient weapon surrounds its wielder with a shield of darkness.\n\r");
	else if (itemtype == 21)
	    sprintf (buf, "This ancient weapon grants superior protection to its wielder.\n\r");
	else if (itemtype == 24)
	    sprintf (buf, "This ancient weapon conceals its wielder from sight.\n\r");
	else
	    sprintf (buf, "This item is bugged...please report it.\n\r");
	if (itemtype > 0)
	    send_to_char( buf, ch );
	break;

    case ITEM_ARMOR:
	sprintf( buf, "Armor class is %li.\n\r", obj->value[0] );
	send_to_char( buf, ch );
	if (obj->value[3] < 1)
	    break;
	if (obj->value[3] == 4)
	    sprintf (buf, "This object radiates an aura of darkness.\n\r");
	else if (obj->value[3] == 27 || obj->value[3] == 2)
	    sprintf (buf, "This item allows the wearer to see invisible things.\n\r");
	else if (obj->value[3] == 39 || obj->value[3] == 3)
	    sprintf (buf, "This object grants the power of flight.\n\r");
	else if (obj->value[3] == 45 || obj->value[3] == 1)
	    sprintf (buf, "This item allows the wearer to see in the dark.\n\r");
	else if (obj->value[3] == 46 || obj->value[3] == 5)
	    sprintf (buf, "This object renders the wearer invisible to the human eye.\n\r");
	else if (obj->value[3] == 52 || obj->value[3] == 6)
	    sprintf (buf, "This object allows the wearer to walk through solid doors.\n\r");
	else if (obj->value[3] == 54 || obj->value[3] == 7)
	    sprintf (buf, "This holy relic protects the wearer from evil.\n\r");
	else if (obj->value[3] == 57 || obj->value[3] == 8)
	    sprintf (buf, "This ancient relic protects the wearer in combat.\n\r");
	else if (obj->value[3] == 9)
	    sprintf (buf, "This crafty item allows the wearer to walk in complete silence.\n\r");
	else if (obj->value[3] == 10)
	    sprintf (buf, "This powerful item surrounds its wearer with a shield of lightning.\n\r");
	else if (obj->value[3] == 11)
	    sprintf (buf, "This powerful item surrounds its wearer with a shield of fire.\n\r");
	else if (obj->value[3] == 12)
	    sprintf (buf, "This powerful item surrounds its wearer with a shield of ice.\n\r");
	else if (obj->value[3] == 13)
	    sprintf (buf, "This powerful item surrounds its wearer with a shield of acid.\n\r");
	else if (obj->value[3] == 14)
	    sprintf (buf, "This ancient item allows its wearer to move at impossible speeds.\n\r");
	else if (obj->value[3] == 15)
	    sprintf (buf, "This powerful item allows its wearer to shear through armour without difficulty.\n\r");
	else if (obj->value[3] == 16)
	    sprintf (buf, "This ancient item regenerates the wounds of its wearer.\n\r");
	else if (obj->value[3] == 17)
	    sprintf (buf, "This ancient item allows its wearer to move at supernatural speed.\n\r");
	else if (obj->value[3] == 18)
	    sprintf (buf, "This ancient item grants its wearer supernatural vision.\n\r");
	else if (obj->value[3] == 19)
	    sprintf (buf, "This powerful item protects its wearer from player attacks.\n\r");
	else if (obj->value[3] == 20)
	    sprintf (buf, "This ancient item surrounds its wearer with a shield of darkness.\n\r");
	else if (obj->value[3] == 21)
	    sprintf (buf, "This ancient item grants superior protection to its wearer.\n\r");
	else if (obj->value[3] == 24)
	    sprintf (buf, "This ancient item conceals its wearer from sight.\n\r");
	else if (obj->value[3] == 58)
	    sprintf (buf, "This powerful item allows its wearer to shear through armor without difficulty.\n\r");
	else
	    sprintf (buf, "This item is bugged...please report it.\n\r");
	if (obj->value[3] > 0)
	    send_to_char( buf, ch );
	break;
    }

    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->location != APPLY_NONE && paf->modifier != 0 )
	{
	    sprintf( buf, "%s %li.\n\r",
		affect_loc_name( paf->location ), paf->modifier );
	    send_to_char( buf, ch );
	}
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->location != APPLY_NONE && paf->modifier != 0 )
	{
	    sprintf( buf, "%s %li.\n\r",
		affect_loc_name( paf->location ), paf->modifier );
	    send_to_char( buf, ch );
	}
    }

    return;
}



void spell_infravision( long sn, long level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_INFRARED) )
	return;
    act( "$n's eyes glow red.\n\r", ch, NULL, NULL, TO_ROOM );
    af.type      = sn;
    af.duration  = 2 * level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_INFRARED;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes glow red.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}



void spell_invis( long sn, long level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_INVISIBLE) )
	return;

    act( "$n fades out of existence.", victim, NULL, NULL, TO_ROOM );
    af.type      = sn;
    af.duration  = 24;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_INVISIBLE;
    affect_to_char( victim, &af );
    send_to_char( "You fade out of existence.\n\r", victim );
    return;
}



void spell_know_alignment( long sn, long level, CHAR_DATA *ch, void *vo )
{
    return;
}



void spell_lightning_bolt( long sn, long level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const long dam_each[] = 
    {
	10,
	15, 15, 15, 20, 20,	25, 25, 25, 25, 28,
	31, 34, 37, 40, 40,	41, 42, 42, 43, 44,
	44, 45, 46, 46, 47,	48, 48, 49, 50, 50,
	51, 52, 52, 53, 54,	54, 55, 56, 56, 57,
	58, 58, 59, 60, 60,	61, 62, 62, 63, 64,
	70, 80, 90,120,150,	200,250,300,350,400
    };
    long dam;
    long hp;

    if (IS_ITEMAFF(victim, ITEMA_SHOCKSHIELD)) return;
    if (!IS_NPC(victim) && victim->pcdata->atm >= number_percent())
    {
	send_to_char("Nothing happens.\n\r",ch);
	return;
    }

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell( level, victim ) )
	dam /= 2;
    hp = victim->hit;
    damage( ch, victim, dam, TYPE_SPELL, "lightning bolt" );
    if (victim == NULL || victim->position == POS_DEAD) return;
    return;
}



void spell_locate_object( long sn, long level, CHAR_DATA *ch, void *vo )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found;
    long i;
    long listcap;

    found = FALSE;
    listcap = 250;
    i = 0;

    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if ( !can_see_obj( ch, obj ) || !is_name( target_name, obj->name ) )
	    continue;
	if ( IS_SET(obj->quest, QUEST_ARTIFACT) && !IS_IMM(ch)) continue;

	found = TRUE;
	i++;

	for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
	    ;

	if ( in_obj->carried_by != NULL )
	{
	    sprintf( buf, "%3li: %s carried by %s.\n\r",
		i, obj->short_descr, PERS(in_obj->carried_by, ch) );
	}
	else
	{
	    sprintf( buf, "%3li: %s in %s.\n\r",
		i, obj->short_descr, in_obj->in_room == NULL
		    ? "somewhere" : in_obj->in_room->name );
	}

	buf[0] = UPPER(buf[0]);
	send_to_char( buf, ch );
	if (i >= 250) break;
    }

    if ( !found )
	send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );

    return;
}



void spell_magic_missile( long sn, long level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    long dam;
    long count;

    if (!IS_NPC(victim) && victim->pcdata->atm >= number_percent())
    {
	send_to_char("Nothing happens.\n\r",ch);
	return;
    }

    for (count = level / 10; count > 0; count--)
    {
	dam = number_range(level * 2, level * 3);
	damage(ch, victim, dam, TYPE_SPELL, "magic missile");
    }
    return;
}



void spell_mass_invis( long sn, long level, CHAR_DATA *ch, void *vo )
{
    AFFECT_DATA af;
    CHAR_DATA *gch;

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( !is_same_group( gch, ch ) || IS_AFFECTED(gch, AFF_INVISIBLE) )
	    continue;
	act( "$n slowly fades out of existence.", gch, NULL, NULL, TO_ROOM );
	send_to_char( "You slowly fade out of existence.\n\r", gch );
	af.type      = sn;
	af.duration  = 24;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_INVISIBLE;
	affect_to_char( gch, &af );
    }
    send_to_char( "Ok.\n\r", ch );

    return;
}



void spell_null( long sn, long level, CHAR_DATA *ch, void *vo )
{
    stc("That's not a spell!\n\r", ch);
    return;
}



void spell_pass_door( long sn, long level, CHAR_DATA *ch, void *vo )
{
    return;
}



void spell_poison( long sn, long level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    char buf [MAX_INPUT_LENGTH];

    /* Ghosts cannot be poisoned - KaVir */
    if (IS_VAMPIRE(victim)) return;
    else if (IS_WEREWOLF(victim) && get_tribe(victim, TRIBE_BONE_GNAWERS) >= 1) return;
    else if (get_drow(victim, DROW_PIMM)) return;
    if (!IS_NPC(victim) && victim->pcdata->atm >= number_percent())
	return;

    if ( saves_spell( level, victim ) )
	return;
    af.type      = sn;
    af.duration  = level;
    af.location  = APPLY_STR;
    af.modifier  = 0 - number_range(1,3);
    af.bitvector = AFF_POISON;
    affect_join( victim, &af );
    send_to_char( "You feel very sick.\n\r", victim );
    if ( ch == victim ) return;
    if (!IS_NPC(victim))
	sprintf(buf,"%s looks very sick as your poison takes affect.\n\r",victim->name);
    else
	sprintf(buf,"%s looks very sick as your poison takes affect.\n\r",victim->short_descr);
    send_to_char(buf,ch);
    return;
}



void spell_protection( long sn, long level, CHAR_DATA *ch, void *vo )
{
    return;
}



void spell_refresh( long sn, long level, CHAR_DATA *ch, void *vo )
{
    return;
}



void spell_remove_curse( long sn, long level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    OBJ_DATA  *obj;
    char       arg [MAX_INPUT_LENGTH];

    one_argument( target_name, arg );

    if ( arg[0] == '\0')
    {
	send_to_char( "Remove curse on what?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, target_name ) ) != NULL )
    {
	if ( is_affected( victim, gsn_curse ) )
    	{
	    affect_strip( victim, gsn_curse );
	    send_to_char( "You feel better.\n\r", victim );
	    if ( ch != victim )
		send_to_char( "Ok.\n\r", ch );
	}
	return;
    }
    if ( ( obj = get_obj_carry( ch, arg ) ) != NULL )
    {
	if(IS_SET(obj->quest, QUEST_ARTIFACT) || IS_SET(obj->quest, QUEST_RELIC))
	{
	    act( "$p is not affected by your attempts.", ch, obj, NULL, TO_CHAR );
	    return;
	}
	if ( IS_SET(obj->extra_flags, ITEM_NOREMOVE) )
	{
	    REMOVE_BIT(obj->extra_flags, ITEM_NOREMOVE);
	    act( "$p flickers with energy.", ch, obj, NULL, TO_CHAR );
	}
	else if ( IS_SET(obj->extra_flags, ITEM_NODROP) )
	{
	    REMOVE_BIT(obj->extra_flags, ITEM_NODROP);
	    act( "$p flickers with energy.", ch, obj, NULL, TO_CHAR );
	}
	return;
    }
    send_to_char( "No such creature or object to remove curse on.\n\r", ch );
    return;
}



void spell_sanctuary( long sn, long level, CHAR_DATA *ch, void *vo )
{
    AFFECT_DATA af;

    if (IS_AFFECTED(ch, AFF_SANCTUARY)) return;

    af.type		= sn;
    af.duration		= level / 2;
    af.location		= APPLY_NONE;
    af.modifier		= 0;
    af.bitvector	= AFF_SANCTUARY;
    affect_to_char(ch, &af);

    act("$n is surrounded by a white aura.", ch, NULL, NULL, TO_ROOM);
    stc("You are surrounded by a white aura.\n\r", ch);
    return;
}



void spell_shield( long sn, long level, CHAR_DATA *ch, void *vo )
{
    return;
}



void spell_shocking_grasp( long sn, long level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const long dam_each[] = 
    {
	10,
	10, 10, 15, 15, 15,	20, 20, 25, 29, 33,
	36, 39, 39, 39, 40,	40, 41, 41, 42, 42,
	43, 43, 44, 44, 45,	45, 46, 46, 47, 47,
	48, 48, 49, 49, 50,	50, 51, 51, 52, 52,
	53, 53, 54, 54, 55,	55, 56, 56, 57, 57,
	60, 70, 85,100,125,	150,175,200,225,300
    };
    long dam;
    long hp;

    if (IS_ITEMAFF(victim, ITEMA_SHOCKSHIELD)) return;
    if (!IS_NPC(victim) && victim->pcdata->atm >= number_percent())
    {
	send_to_char("Nothing happens.\n\r",ch);
	return;
    }

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell( level, victim ) )
	dam /= 2;
    hp = victim->hit;
    damage( ch, victim, dam, TYPE_SPELL, "shocking grasp" );
    if (victim == NULL || victim->position == POS_DEAD) return;
    return;
}



void spell_sleep( long sn, long level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
  
    if ( IS_AFFECTED(victim, AFF_SLEEP)
    ||   level < victim->level
    || (  IS_NPC(victim) && IS_AFFECTED(victim, AFF_ETHEREAL) )
    || ( !IS_NPC(victim) && IS_VAMPIRE(victim) )
    ||   saves_spell( level, victim ) )
    {
	send_to_char("Nothing happens.\n\r",ch);
	return;
    }

    if (!IS_NPC(victim) && victim->pcdata->atm >= number_percent())
    {
	send_to_char("Nothing happens.\n\r",ch);
	return;
    }
    af.type      = sn;
    af.duration  = 4 + level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_SLEEP;
    affect_join( victim, &af );

    if ( IS_AWAKE(victim) )
    {
	send_to_char( "You feel very sleepy ..... zzzzzz.\n\r", victim );
	act( "$n goes to sleep.", victim, NULL, NULL, TO_ROOM );
	victim->position = POS_SLEEPING;
    }

    return;
}



void spell_stone_skin( long sn, long level, CHAR_DATA *ch, void *vo )
{
    return;
}



void spell_summon( long sn, long level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;

    if ( ( victim = get_char_world( ch, target_name ) ) == NULL
    ||   victim == ch
    ||   victim->in_room == NULL
    ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||   victim->level >= level + 3
    ||   victim->fighting != NULL
    ||   victim->in_room->area != ch->in_room->area
    ||   (!IS_NPC(victim) && !IS_EXTRA(victim, EXTRA_SUMMON) )
    ||   (IS_NPC(victim) && IS_AFFECTED(victim, AFF_ETHEREAL) )
    ||   (IS_NPC(victim) && saves_spell( level, victim ) ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    act( "$n disappears suddenly.", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, ch->in_room );
    act( "$n arrives suddenly.", victim, NULL, NULL, TO_ROOM );
    act( "$N has summoned you!", victim, NULL, ch,   TO_CHAR );
    do_look( victim, "auto" );
    return;
}



void spell_teleport( long sn, long level, CHAR_DATA *ch, void *vo )
{
    ROOM_INDEX_DATA *pRoomIndex;
    long to_room;

    if (!IS_EXTRA(ch, EXTRA_SUMMON))
    {
	stc("You need to turn summon on to teleport.\n\r", ch);
	return;
    }
    if (ch->position == POS_FIGHTING)
    {
	stc("Not while fighting!\n\r", ch);
	return;
    } 

    for ( ; ; )
    {
	to_room = number_range( 100, 16000 );
	pRoomIndex = get_room_index( to_room );
	if (pRoomIndex != NULL
	 && !IS_SET(pRoomIndex->room_flags, ROOM_PRIVATE)
	 && !IS_SET(pRoomIndex->room_flags, ROOM_SOLITARY)
	 && !IS_SET(pRoomIndex->room_flags, ROOM_NO_TELEPORT))
	    break;
    }

    act( "$n slowly fades out of existence.", ch, NULL, NULL, TO_ROOM );
    char_from_room( ch );
    char_to_room( ch, pRoomIndex );
    act( "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM );
    do_look( ch, "auto" );
    return;
}



void spell_ventriloquate( long sn, long level, CHAR_DATA *ch, void *vo )
{
    return;
}



void spell_weaken( long sn, long level, CHAR_DATA *ch, void *vo )
{
    return;
}



void spell_word_of_recall( long sn, long level, CHAR_DATA *ch, void *vo )
{
    return;
}



/*
 * NPC spells.
 */
void spell_acid_breath( long sn, long level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    long dam;

    if (IS_ITEMAFF(victim, ITEMA_ACIDSHIELD)) return;
    if (!IS_NPC(victim) && victim->pcdata->atm >= number_percent())
    {
	stc("Nothing happens.\n\r", ch);
	return;
    }

    dam = number_range(level * 50, level * 70);
    if (saves_spell(level, victim)) dam /= 2;
    damage(ch, victim, dam, TYPE_SPELL, "acid breath");
    return;
}



void spell_fire_breath( long sn, long level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    long dam;

    if (IS_ITEMAFF(victim, ITEMA_FIRESHIELD)) return;
    if (IS_WEREWOLF(victim) && IS_MORE(victim, MORE_HELIOS)) return;
    if (!IS_NPC(victim) && victim->pcdata->atm >= number_percent())
    {
	send_to_char("Nothing happens.\n\r",ch);
	return;
    }

    dam = number_range(level * 40, level * 60);
    if (saves_spell(level, victim)) dam /= 2;
    damage(ch, victim, dam, TYPE_SPELL, "fire breath");
    return;
}



void spell_frost_breath( long sn, long level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    long dam;

    if (IS_ITEMAFF(victim, ITEMA_ICESHIELD)) return;
    if (IS_WEREWOLF(victim) && get_tribe(victim, TRIBE_WENDIGOS) >= 4) return;
    if (!IS_NPC(victim) && victim->pcdata->atm >= number_percent())
    {
	stc("Nothing happens.\n\r",ch);
	return;
    }

    dam = number_range(level * 70, level * 90);
    if (saves_spell(level, victim)) dam /= 2;
    damage(ch, victim, dam, TYPE_SPELL, "frost breath");
    return;
}

void spell_gas_breath( long sn, long level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    long dam;

    for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
    {
	vch_next = vch->next_in_room;
	if (IS_NPC(ch) != IS_NPC(vch))
	{
	    dam = number_range(level * 30, level * 50);
	    if (saves_spell(level, vch)) dam /= 2;
	    damage(ch, vch, dam, TYPE_SPELL, "gas breath");
	}
    }
    return;
}

void spell_lightning_breath( long sn, long level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    long dam;

    if (IS_ITEMAFF(victim, ITEMA_SHOCKSHIELD)) return;
    if (!IS_NPC(victim) && victim->pcdata->atm >= number_percent())
    {
	send_to_char("Nothing happens.\n\r",ch);
	return;
    }

    dam = number_range(level * 30, level * 50);
    if (saves_spell(level, victim)) dam /= 2;
    damage(ch, victim, dam, TYPE_SPELL, "lightning breath");
    dam = number_range(level * 30, level * 50);
    if (saves_spell(level, victim)) dam /= 2;
    damage(ch, victim, dam, TYPE_SPELL, "lightning breath");
    return;
}



/* Extra spells written by KaVir. */
void spell_guardian( long sn, long level, CHAR_DATA *ch, void *vo )
{
    char buf [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    AFFECT_DATA af;

    if (IS_NPC(ch))
    {
	send_to_char("Nothing happens.\n\r",ch);
	return;
    }

    if (ch->pcdata->followers > 1)
    {
	send_to_char("Nothing happens.\n\r",ch);
	return;
    }

    ch->pcdata->followers++;


    victim=create_mobile( get_mob_index( MOB_VNUM_GUARDIAN ) );
    victim->level = level;
    victim->hit = 100*level;
    victim->max_hit = 100*level;
    victim->hitroll = level;
    victim->damroll = level;
    victim->armor = 100 - (level*7);


    free_string(victim->lord);
    victim->lord = str_dup(ch->name);
    strcpy(buf,"Come forth, creature of darkness, and do my bidding!");
    do_say( ch, buf );


    send_to_char( "A demon bursts from the ground and bows before you.\n\r",ch );

//    act( "$N bursts from the ground and bows before $n.", ch, NULL, victim, TO_ROOM ); this made the mud crash.......

    char_to_room( victim, ch->in_room );

    add_follower( victim, ch );
    af.type      = sn;
    af.duration  = 666;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char( victim, &af );
    return;

}

void spell_soulblade( long sn, long level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA  *obj = (OBJ_DATA *) vo;
    char buf [MAX_STRING_LENGTH];
    char arg [MAX_INPUT_LENGTH];
    char wpnname [MAX_INPUT_LENGTH];
    long weapontype;

    if (IS_NPC(ch)) return;

    target_name = one_argument(target_name,arg);

    if      (!str_cmp(arg, "slash"))	{weapontype = TYPE_SLASH;strcpy(wpnname, "blade");}
    else if (!str_cmp(arg, "pierce"))	{weapontype = TYPE_PIERCE;strcpy(wpnname, "dagger");}
    else if (!str_cmp(arg, "blunt"))	{weapontype = TYPE_BLUNT;strcpy(wpnname, "mace");}
    else
    {
	stc("Pick slash, pierce, or blunt.\n\r", ch);
	return;
    }

    obj = create_object(get_obj_index(OBJ_VNUM_SOULBLADE), 0);

    free_string(obj->name);
    sprintf(buf,"%s soul %s",ch->name,wpnname);
    obj->name=str_dup(buf);
    free_string(obj->short_descr);
    sprintf(buf,"%s's soul %s",ch->name,wpnname);
    buf[0] = UPPER(buf[0]);
    obj->short_descr=str_dup(buf);
    free_string(obj->description);
    sprintf(buf,"%s's soul %s is lying here.",ch->name,wpnname);
    buf[0] = UPPER(buf[0]);
    obj->description=str_dup(buf);

    obj->level = level;
    if (obj->level > 60) obj->level = 60;
    obj->value[0] = 13034;
    obj->value[1] = 10;
    obj->value[2] = 20;
    obj->value[3] = weapontype;
    if (obj->questmaker != NULL) free_string(obj->questmaker);
    obj->questmaker = str_dup(ch->name);
    if (obj->questowner != NULL) free_string(obj->questowner);
    obj->questowner = str_dup(ch->name);
    obj_to_char(obj,ch);
    act("$p fades into existance in your hand.", ch, obj, NULL, TO_CHAR);
    act("$p fades into existance in $n's hand.", ch, obj, NULL, TO_ROOM);
    return;
}

void spell_mana( long sn, long level, CHAR_DATA *ch, void *vo)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    long heal = level * 20;

    victim->mana = UMIN( victim->mana + heal, victim->max_mana);
    if (ch == victim)
    {
        send_to_char("You draw in energy from your surrounding area.\n\r", ch);
        act("$n draws in energy from $s surrounding area.", ch, NULL, NULL, TO_ROOM);
        return;
    }
    act("You draw in energy from around you and channel it into $N.", ch, NULL, victim, TO_CHAR);
    act("$n draws in energy and channels it into $N.", ch, NULL, victim, TO_NOTVICT);
    act("$n draws in energy and channels it into you.", ch, NULL, victim, TO_VICT);
    return;
}

void spell_frenzy( long sn, long level, CHAR_DATA *ch, void *vo )
{
    return;
}

void spell_darkblessing( long sn, long level, CHAR_DATA *ch, void *vo )
{
    return;
}

void spell_transport( long sn, long level, CHAR_DATA *ch, void *vo )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    CHAR_DATA *victim;

    target_name = one_argument( target_name, arg1 );
    target_name = one_argument( target_name, arg2 );

    if ( arg1[0] == '\0' )
    {
        send_to_char( "Transport which object?\n\r", ch );
        return;
    }

    if ( arg2[0] == '\0' )
    {
        send_to_char( "Transport who whom?\n\r", ch );
        return;
    }
    if ( ( victim = get_char_world( ch, arg2 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if(IS_NPC(victim))
	{
	stc("Not to NPCs.\n\r",ch);
	return;
	}
    if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
    {
	send_to_char( "You are not carrying that item.\n\r", ch );
	return;
    }

    if (!IS_NPC(victim) && !IS_EXTRA(victim, EXTRA_TRANSPORT))
    {
	send_to_char( "You are unable to transport anything to them.\n\r", ch );
	return;
    }
    if (victim->level >= 1500)
    { 
	send_to_char( "You are unable to transport anything to them.\n\r", ch );
        return;
    }

    act("$p vanishes from your hands in an swirl of smoke.",ch,obj,NULL,TO_CHAR);
    act("$p vanishes from $n's hands in a swirl of smoke.",ch,obj,NULL,TO_ROOM);
    obj_from_char(obj);
    obj_to_char(obj,victim);
    act("$p appears in your hands in an swirl of smoke.",victim,obj,NULL,TO_CHAR);
    act("$p appears in $n's hands in an swirl of smoke.",victim,obj,NULL,TO_ROOM);
    do_autosave(ch,"");
    do_autosave(victim,"");
    return;
}


void spell_quest( long sn, long level, CHAR_DATA *ch, void *vo )
{
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;

    if (ch->primal < 1)
    {
	send_to_char( "It costs at least 1 point of primal energy to create a quest card.\n\r", ch );
	return;
    }

    if ( (pObjIndex = get_obj_index( OBJ_VNUM_QUESTCARD )) == NULL)
    {
	send_to_char("Missing object, please inform KaVir.\n\r",ch);
	return;
    }
    if (ch->in_room == NULL) return;
    obj = create_object(pObjIndex, 0);
    obj_to_char(obj, ch);
    quest_object(ch,obj);
    if (ch->primal >= 100) {ch->primal -= 100; obj->level = 100;}
    else {obj->level = ch->primal; ch->primal = 0;}
    act("$p fades into existance in your hands.",ch,obj,NULL,TO_CHAR);
    act("$p fades into existance in $n's hands.",ch,obj,NULL,TO_ROOM);
    if (obj->questmaker != NULL) free_string(obj->questmaker);
    obj->questmaker = str_dup(ch->name);
    if (obj->questowner != NULL) free_string(obj->questowner);
    obj->questowner = str_dup(ch->name);
    return;
}

void spell_mount( long sn, long level, CHAR_DATA *ch, void *vo )
{
    char buf[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    if (IS_NPC(ch)) return;

    if (ch->pcdata->followers > 4)
    {
	send_to_char("Nothing happens.\n\r",ch);
	return;
    }
    ch->pcdata->followers++;
    victim=create_mobile( get_mob_index( MOB_VNUM_MOUNT ) );
    victim->level = level;
    victim->armor = 0 - (level<<1);
    victim->hitroll = level;
    victim->damroll = level;
    victim->hit = 100*level;
    victim->max_hit = 100*level;
    free_string(victim->lord);
    victim->lord = str_dup(ch->name);
    SET_BIT(victim->affected_by,AFF_FLYING);
	free_string(victim->name);
	victim->name = str_dup("mount black horse");
	sprintf(buf,"%s's black stallion",ch->name);
	free_string(victim->short_descr);
	victim->short_descr = str_dup(buf);
	free_string(victim->long_descr);
	victim->long_descr = str_dup("A large black horse stands here.\n\r");
    char_to_room( victim, ch->in_room );
    act( "$N fades into existance.", ch, NULL, victim, TO_CHAR );
    act( "$N fades into existance.", ch, NULL, victim, TO_ROOM );
    return;
}


void spell_insert_page( long sn, long level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *page;
    OBJ_DATA *book;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    target_name = one_argument( target_name, arg1 );
    target_name = one_argument( target_name, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {send_to_char("Syntax is: cast 'insert page' <page> <book>.\n\r",ch);return;}
    if ( ( page = get_obj_carry( ch, arg1 ) ) == NULL )
    {send_to_char("You are not carrying that page.\n\r",ch);return;}
    if ( page->item_type != ITEM_PAGE )
    {send_to_char("That item isn't a page.\n\r",ch);return;}
    if ( ( book = get_obj_carry( ch, arg2 ) ) == NULL )
    {send_to_char("You are not carrying that book.\n\r",ch);return;}
    if ( book->item_type != ITEM_BOOK )
    {send_to_char("That item isn't a book.\n\r",ch);return;}
    if ( IS_SET(book->value[1], CONT_CLOSED) )
    {send_to_char("First you need to open it!\n\r",ch);return;}

    obj_from_char(page);
    obj_to_obj(page,book);
    book->value[3] += 1;
    book->value[2] = book->value[3];
    page->value[0] = book->value[3];
    page->specpower = book->value[3]+1;

    act("You insert $p into $P.",ch,page,book,TO_CHAR);
    act("$n inserts $p into $P.",ch,page,book,TO_ROOM);
    return;
}

void spell_remove_page( long sn, long level, CHAR_DATA *ch, void *vo )
{
    char arg1[MIL];
    OBJ_DATA *page;
    OBJ_DATA *book;
    OBJ_DATA *page_next;
    long count = 0;

    target_name = one_argument(target_name, arg1);

    if (arg1[0] == '\0')
    {
	stc("Syntax is: cast 'remove page' <book>.\n\r", ch);
	return;
    }
    if ((book = get_obj_carry(ch, arg1)) == NULL)
    {
	stc("You are not carrying that book.\n\r", ch);
	return;
    }
    if (book->item_type != ITEM_BOOK)
    {
	stc("That item isn't a book.\n\r", ch);
	return;
    }
    if (IS_SET(book->value[1], CONT_CLOSED))
    {
	stc("First you need to open it!\n\r", ch);
	return;
    }
    if (book->value[2] == 0)
    {
	stc("You cannot remove the index page!\n\r",ch);
	return;
    }

    if ((page = get_page(book, book->value[2])) == NULL)
    {
	stc("The page seems to have been torn out.\n\r", ch);
	return;
    }

    obj_from_obj(page);
    obj_to_char(page, ch);
    page->value[0] = 0;

    act("You remove $p from $P.", ch, page, book, TO_CHAR);
    act("$n removes $p from $P.", ch, page, book, TO_ROOM);

    for (page = book->contains; page != NULL; page = page_next)
    {
	page_next = page->next_content;
	count += 1;
	page->value[0] = count;
    }
    book->value[3] = count;

    if (book->value[2] > book->value[3]) book->value[2] = book->value[3];

    return;
}

void enhance_stat( long sn, long level, CHAR_DATA *ch, CHAR_DATA *victim, long apply_bit, long bonuses, long affect_bit )
{
    AFFECT_DATA af;

    if (IS_SET(affect_bit, AFF_ETHEREAL) && victim->position == POS_FIGHTING)
	affect_bit -= AFF_ETHEREAL;
    if (IS_SET(affect_bit, AFF_SHADOWPLANE) && victim->position == POS_FIGHTING)
	affect_bit -= AFF_SHADOWPLANE;
    if (IS_SET(affect_bit, AFF_WEBBED) && (IS_AFFECTED(victim, AFF_WEBBED) || auto_safe(ch, victim)))
	affect_bit -= AFF_WEBBED;

    if (IS_SET(affect_bit, AFF_CHARM) && !IS_AFFECTED(victim, AFF_CHARM))
    {
	if (victim->level <= 50 && IS_NPC(victim))
	{
	    if (victim->master) stop_follower(victim);
            add_follower(victim, ch);
	}
	else
	{
	    stc("The spell failed.\n\r",ch);
	    return;
	}
    }

    af.type      = sn;
    af.duration  = level;
    af.location  = apply_bit;
    af.modifier  = bonuses;
    af.bitvector = affect_bit;
    affect_to_char(victim, &af);

    return;
}

void spell_chaos_blast( long sn, long level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    long dam;

    if (!IS_NPC(victim) && victim->pcdata->atm >= number_percent())
    {
	return;
    }
    dam = dice(level, 6);
    if (saves_spell(level, victim)) dam *= 0.5;
    damage(ch, victim, dam, TYPE_SPELL, "chaos blast");
    return;
}

void spell_web( long sn, long level, CHAR_DATA *ch, void *vo )
{
    AFFECT_DATA af;
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    act("You point your finger at $N and a web flies from your hand!", ch, NULL, victim, TO_CHAR);
    act("$n points $s finger at $N and a web flies from $s hand!", ch, NULL, victim, TO_NOTVICT);
    act("$n points $s finger at you and a web flies from $s hand!", ch, NULL, victim, TO_VICT);

    if (IS_AFFECTED(victim, AFF_WEBBED))
    {
	stc("But they are already webbed!\n\r", ch);
	return;
    }

    if (is_safe(ch, victim)) return;

    if (!IS_NPC(victim) && number_percent() < (char_stat(victim, STAT_DEX) * 2) && victim->position >= POS_FIGHTING)
    {
	stc("You dodge the web!\n\r", victim);
	act("$n dodges the web!", victim, NULL, NULL, TO_ROOM);
	return;
    }

    af.type      = sn;
    af.location  = APPLY_AC;
    af.modifier  = -100;
    af.duration  = 2;
    af.bitvector = AFF_WEBBED;
    affect_to_char(victim, &af);
    stc("You are coated in a sticky web!\n\r", victim);
    act("$n is coated in a sticky web!", victim, NULL, NULL, TO_ROOM);
    return;
}

void spell_find_familiar( long sn, long level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    CHAR_DATA *familiar;
    if ( target_name[0] == '\0' )
    {
	send_to_char( "Your familiar can be a frog, a raven, a cat or a dog.\n\r", ch );
	return;
    }

    if ( ( familiar = ch->pcdata->familiar ) != NULL ) 
    {
	send_to_char( "But you already have a familiar!\n\r", ch );
	return;
    }

    if ( !str_cmp( target_name, "frog" ) )
    {
	victim = create_mobile( get_mob_index( MOB_VNUM_FROG ) );
	if (victim == NULL)
	{send_to_char("Error - please inform KaVir.\n\r",ch); return;}
    }
    else if ( !str_cmp( target_name, "raven" ) )
    {
	victim = create_mobile( get_mob_index( MOB_VNUM_RAVEN ) );
	if (victim == NULL)
	{send_to_char("Error - please inform KaVir.\n\r",ch); return;}
    }
    else if ( !str_cmp( target_name, "cat" ) )
    {
	victim = create_mobile( get_mob_index( MOB_VNUM_CAT ) );
	if (victim == NULL)
	{send_to_char("Error - please inform KaVir.\n\r",ch); return;}
    }
    else if ( !str_cmp( target_name, "dog" ) )
    {
	victim = create_mobile( get_mob_index( MOB_VNUM_DOG ) );
	if (victim == NULL)
	{send_to_char("Error - please inform KaVir.\n\r",ch); return;}
    }
    else 
    {
	send_to_char( "Your familiar can be a frog, a raven, a cat or a dog.\n\r", ch );
	return;
    }
	stc("You summon a familiar.\n\r", ch);
//    act( "You make a few gestures and $N appears in a swirl of smoke.", ch, NULL, victim, TO_CHAR );
//    act( "$n makes a few gestures and $N appears in a swirl of smoke.", ch, NULL, victim, TO_ROOM );
    char_to_room( victim, ch->in_room );

    ch->pcdata->familiar = victim;
    SET_BIT(victim->extra, EXTRA_ILLUSION);
    victim->wizard = ch;
    victim->level = 501;

    return;
}

void spell_improve( long sn, long level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;

    if ( IS_SET(obj->quest, QUEST_ARTIFACT) )
    {
	send_to_char( "Not on artifacts.\n\r", ch );
	return;
    }
    if ( obj->pIndexData->vnum != OBJ_VNUM_PROTOPLASM )
    {
	send_to_char( "You cannot enhance this object.\n\r", ch );
	return;
    }
    if (IS_SET(obj->quest, QUEST_IMPROVED))
    {
	send_to_char( "This item has already been improved.\n\r", ch );
	return;
    }
    else if ( obj->points < 750 && obj->item_type != ITEM_WEAPON )
    {
	send_to_char( "The object must be worth at least 750 quest points.\n\r", ch );
	return;
    }
    else if ( obj->points < 1500 && obj->item_type == ITEM_WEAPON )
    {
	send_to_char( "The object must be worth at least 1500 quest points.\n\r", ch );
	return;
    }
    REMOVE_BIT(obj->quest, QUEST_HITROLL);
    REMOVE_BIT(obj->quest, QUEST_DAMROLL);
    REMOVE_BIT(obj->quest, QUEST_HIT);
    REMOVE_BIT(obj->quest, QUEST_MANA);
    REMOVE_BIT(obj->quest, QUEST_AC);
    SET_BIT(obj->quest, QUEST_IMPROVED);
    act("$p flickers for a moment.",ch,obj,NULL,TO_CHAR);
    act("$p flickers for a moment.",ch,obj,NULL,TO_ROOM);
    return;
}

void spell_fear( long sn, long level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim == ch )
    {
	send_to_char( "You're really scared of yourself!\n\r", ch );
	return;
    }

    if ( IS_AFFECTED(victim, AFF_FEAR) )
    {
	send_to_char( "They already seem pretty scared!\n\r", ch );
	return;
    }

    if (!IS_NPC(victim))
    {
	if (victim->pcdata->atm >= number_percent()
	|| (IS_VAMPIRE(victim) && (victim->vampgen < ch->vampgen)) )
	{send_to_char("Nothing happens.\n\r",ch);return;}
    }

    if ( is_safe(ch, victim) ) return;

    if ( saves_spell( level, victim ) && victim->position >= POS_FIGHTING )
    {
	act("$E ignores you.",ch,NULL,victim,TO_CHAR);
	act("You ignore $m.",ch,NULL,victim,TO_VICT);
	act("$N ignores $n.",ch,NULL,victim,TO_NOTVICT);
	return;
    }

    af.type      = sn;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.duration  = number_range(1,2);
    af.bitvector = AFF_FEAR;
    affect_to_char( victim, &af );
    send_to_char( "You scream in terror!\n\r", victim );
    act("$n screams in terror!",victim,NULL,NULL,TO_ROOM);
    return;
}

void spell_tendrils( long sn, long level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    long dam;

    if (!IS_VAMPIRE(ch)) return;

    dam = number_range(1,10) * get_disc(ch,DISC_OBTENEBRATION) * level;

    if ( is_safe(ch, victim) ) return;

    send_to_char( "Tendrils of darkness shoot from your fingertips.\n\r", ch );
    act("Tendrils of darkness shoot from $n's fingertips.",ch,NULL,NULL,TO_ROOM);

    if ( saves_spell( level*2, victim ) && victim->position >= POS_FIGHTING ) //testing
    {
	dam = 0;
	damage( ch, victim, dam, TYPE_SPELL, "tendrils" );
	return;
    }

    damage( ch, victim, dam, TYPE_SPELL, "tendrils" );

    if (victim == NULL || victim->position == POS_DEAD) return;

    if (IS_AFFECTED(victim, AFF_SHADOW)) return;

    af.type      = sn;
    af.location  = APPLY_AC;
    af.modifier  = -100;
    af.duration  = number_range(1,2);
    af.bitvector = AFF_SHADOW;
    affect_to_char( victim, &af );
    send_to_char( "You are entrapped by tendrils of darkness!\n\r", victim );
    act("$n is entrapped by tendrils of darkness!",victim,NULL,NULL,TO_ROOM);
    return;
}
