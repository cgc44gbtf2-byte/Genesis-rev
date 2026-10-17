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


void do_monklearn( CHAR_DATA *ch, char *argument )
{
    char arg1[MIL];
    char arg2[MIL];
    char buf[MIL];
    long improve, cost;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (!IS_MONK(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (arg1[0] == '\0')
    {
	sprintf(buf, "Monk Skills: Mantras(%li), Arts(%li), Techs(%li).\n\r",
	 get_monk(ch, MONK_MANTRA), get_monk(ch, MONK_ART), get_monk(ch, MONK_TECH));
	stc(buf, ch);
	stc("Syntax: Display: Monklearn <mantra/art/tech>.\n\r", ch);
	stc("        Improve: Monklearn <mantra/art/tech> gain.\n\r", ch);
	return;
    }

    if (!str_cmp(arg1, "mantra"))
    {
	if (arg2[0] == '\0')
	{
	    stc("Mantras - A myriad of non-combat abilities.\n\r",ch);
	    if (get_monk(ch, MONK_MANTRA) < 1) stc("You have no Mantras.\n\r", ch);
	    if (get_monk(ch, MONK_MANTRA) >= 1) stc("1: Eyes of God      - Increases awareness. (heighten)\n\r", ch);
	    if (get_monk(ch, MONK_MANTRA) >= 2) stc("2: Trio Mantra      - Grants knowledge. (scry/readaura/shield)\n\r", ch);
	    if (get_monk(ch, MONK_MANTRA) >= 3) stc("3: Rejuvenation     - Gives regeneration. (auto)\n\r", ch);
	    if (get_monk(ch, MONK_MANTRA) >= 4) stc("4: Healing Touch    - Allows you to heal with a touch. (touch)\n\r", ch);
	    if (get_monk(ch, MONK_MANTRA) >= 5) stc("5: Almighty Favor   - Grants a blessing. (almighty/almighty group)\n\r", ch);
	    if (get_monk(ch, MONK_MANTRA) >= 6) stc("6: Spirit Guardian  - Summons a guardian. (spiritguardian)\n\r", ch);
	    if (get_monk(ch, MONK_MANTRA) >= 7) stc("7: Celestial Path   - Instantaneous travel. (celestial)\n\r", ch);
	    if (get_monk(ch, MONK_MANTRA) >= 8) stc("8: Cloak of Life    - Saves you from death. (lifecloak)\n\r", ch);
	    return;
	}

	if (!str_cmp(arg2, "gain")) improve = MONK_MANTRA;
	else
	{
	    do_monklearn(ch, "");
	    return;
	}
    }
    else if (!str_cmp(arg1, "tech"))
    {
	if (arg2[0] == '\0')
	{
	    stc("Techs - Fighting techniques and powers.\n\r", ch);
	    if (get_monk(ch, MONK_TECH) < 1) stc("You have no Techs.\n\r", ch);
	    if (get_monk(ch, MONK_TECH) >= 1) stc("1: Repeating Fist - Multihit technique. (repeat)\n\r", ch);
	    if (get_monk(ch, MONK_TECH) >= 2) stc("2: Aura Shot      - Life stealing attack. (aura)\n\r", ch);
	    if (get_monk(ch, MONK_TECH) >= 3) stc("3: Blitz          - Area affect damage. (blitz)\n\r", ch);
	    if (get_monk(ch, MONK_TECH) >= 4) stc("4: Invite Beast   - Possesses weak creatures. (invite)\n\r", ch);
	    if (get_monk(ch, MONK_TECH) >= 5) stc("5: Crippling Kick - Mana burning kick. (cripple)\n\r", ch);
	    if (get_monk(ch, MONK_TECH) >= 6) stc("6: Dark Blaze     - Blinds your foes. (darkblaze)\n\r", ch);
	    if (get_monk(ch, MONK_TECH) >= 7) stc("7: Stun Fist      - Damaging stun attack. (stun)\n\r", ch);
	    if (get_monk(ch, MONK_TECH) >= 8) stc("8: Chimera Form   - Powerful form. (chimera)\n\r", ch);
	    return;
	}

	if (!str_cmp(arg2, "gain")) improve = MONK_TECH;
	else
	{
	    do_monklearn(ch, "");
	    return;
	}
    }
    else if (!str_cmp(arg1, "art"))
    {
	if (arg2[0] == '\0')
	{
	    stc("Arts - Combat abilities and styles.\n\r", ch);
	    if (get_monk(ch, MONK_ART) < 1) stc("You have no Arts.\n\r", ch);
	    if (get_monk(ch, MONK_ART) >= 1) stc("1: Focus of Mind    - Boosts damage. (auto)\n\r", ch);
	    if (get_monk(ch, MONK_ART) >= 2) stc("2: Reinforce        - Boosts accuracy. (auto)\n\r", ch);
	    if (get_monk(ch, MONK_ART) >= 3) stc("3: Speed of Wind    - Boosts speed. (auto)\n\r", ch);
	    if (get_monk(ch, MONK_ART) >= 4) stc("4: Illusionary Form - Boosts avoid. (auto)\n\r", ch);
	    if (get_monk(ch, MONK_ART) >= 5) stc("5: Heart of Oak     - Strengthens the body. (auto)\n\r", ch);
	    if (get_monk(ch, MONK_ART) >= 6) stc("6: Adamantium Hands - Hardens your fists. (auto)\n\r", ch);
	    if (get_monk(ch, MONK_ART) >= 7) stc("7: Mystic Soul      - Boosts damage. (auto)\n\r", ch);
	    if (get_monk(ch, MONK_ART) >= 8) stc("8: Dragon Movement  - Grants Wavefist attack. (auto)\n\r", ch);
	    return;
	}

	if (!str_cmp(arg2, "gain")) improve = MONK_ART;
	else
	{
	    do_monklearn(ch, "");
	    return;
	}
    }
    else
    {
	do_monklearn(ch, "");
	return;
    }

    if (get_monk(ch, improve) >= 8)
    {
	sprintf(buf, "You may not gain any more %ss.\n\r", arg1);
	stc(buf, ch);
	return;
    }

    cost = (ch->pcdata->monk[improve] + 1) * 10;
    if (ch->primal < cost)
    {
	sprintf(buf, "It costs %li primal to raise %s.\n\r", cost, arg1);
	stc(buf, ch);
	return;
    }

    ch->primal -= cost;
    ch->pcdata->monk[improve]++;
    sprintf(buf, "You use %li primal to improve %s.\n\r", cost, arg1);
    stc(buf, ch);

    return;
}


void do_repeat( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MIL];
    int loop;

    one_argument(argument, arg);

    if (!IS_MONK(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }
    
    if (get_monk(ch, MONK_TECH) < 1)
    {
	stc("You need Tech 1 for Repeating Fist.\n\r", ch);
	return;
    }

    if (ch->mana < 1200)
    {
	stc("You don't have enough mana.\n\r", ch);
	return;
    }    

    if (arg[0] == '\0' && ch->fighting == NULL)
    {
	stc("Who shall you hit?\n\r", ch);
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
        stc("Use this? On Yourself?\n\r", ch);
        return;
    }
    
    if (is_safe(ch, victim)) return;

    stc("Your begin an onslaught of attacks!\n\r", ch);
    act("$n concentrates and releases a barrage of punches!", ch, NULL, NULL, TO_ROOM);

    for (loop = 0; loop <= get_monk(ch, MONK_TECH) / 2; loop++)
    {
	if (victim == NULL) break;
	if (ch->mana < 600)
	{
	    stc("You don't have enough mana to continue\n\r", ch);
	    break;
	}
	one_hit(ch, victim, TYPE_UNARMED, "repeating fist");
	ch->mana -= 600;
    }

    WAIT_STATE(ch, 15);
    return;
}

void do_aurashot( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MIL];
    char buf[MIL];
    int dam;

    one_argument(argument, arg);

    if (!IS_MONK(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (get_monk(ch, MONK_TECH) < 2)
    {
	stc("You need Tech 2 for Aura Shot.\n\r", ch);
	return;
    }

    if (ch->mana < 2000)
    {
	stc("You do not have enough mana for Aura Shot.\n\r", ch);
	return;
    }

    if (arg[0] == '\0' && ch->fighting == NULL)
    {
	stc("Use Aura Shot on whom?\n\r", ch);
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

    if (is_safe(ch, victim)) return;
    
    if (ch == victim)
    {
	stc("Are you insane?\n\r", ch);
	return;
    }  

    ch->mana -= 2000;
    WAIT_STATE(ch, 12);

    if (!hurt_fighting(ch, victim)) return;

    if (!IS_NPC(victim) && check_avoid(ch, victim, TYPE_HIT))
    {
	act("Your Aura Shot is avoided!", ch, NULL, victim, TO_CHAR);
	act("You avoid $n's Aura Shot!", ch, NULL, victim, TO_VICT);
	act("$N avoids $n's Aura Shot!", ch, NULL, victim, TO_NOTVICT);
	return;
    }
    dam =  number_range(200, 300) * get_monk(ch, MONK_TECH);

    sprintf(buf, "Your Aura Shot strikes $N and begins draining. [#g%i#n]", dam);
    act(buf, ch, NULL, victim, TO_CHAR);
    sprintf(buf, "$n's Aura Shot strikes you. [#r%i#n]", dam);
    act(buf, ch, NULL, victim, TO_VICT);
    sprintf(buf, "$n's Aura Shot stikes $N and drains his lifeforce. [#0%i#n]", dam);
    act(buf, ch, NULL, victim, TO_NOTVICT);

    ch->hit += dam * .8;
    if (ch->hit > ch->max_hit) ch->hit = ch->max_hit;
    hurt_person(ch, victim, dam);
    return;
}

void do_blitz( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    CHAR_DATA *victim_next;
    char arg[MIL];
    char buf[MIL];
    int damage;

    one_argument(argument, arg);

    if (!IS_MONK(ch))
    {
        stc("Huh?\n\r", ch);
        return;
    }

    if (get_monk(ch, MONK_TECH) < 3)
    {
	stc("You need Tech 3 to use Blitz.\n\r", ch);
	return;
    }

    if (ch->mana < 5000)
    {
	stc("You do not have enough mana!\n\r", ch);
	return;
    }
    
    stc("You concentrate your force and strike down your enemies!\n\r", ch);
    act("$n glows yellow and releases massive energy waves!", ch, NULL, NULL, TO_ROOM);
    ch->mana -= 5000;

    for (victim = ch->in_room->people; victim; victim = victim_next)
    {
	victim_next = victim->next_in_room;
	if (ch == victim) continue;
	if (is_same_group(ch, victim)) continue;
	if (is_safe(ch, victim)) continue;
	if (!IS_NPC(victim) && ch->fighting != victim && victim->fighting != ch) continue;

	if (!hurt_fighting(ch, victim)) continue;

	if (IS_NPC(victim)) damage = number_range(750, 1250) * get_monk(ch, MONK_TECH);
	else damage = number_range(500, 750) * get_monk(ch, MONK_TECH);

	sprintf(buf, "$N flies back as the force wracks $S body! [#g%i#n]", damage);
	act(buf, ch, NULL, victim, TO_CHAR);
	sprintf(buf, "$N flies back as the force wracks $S body! [#0%i#n]", damage);
	act(buf, ch, NULL, victim, TO_NOTVICT);
	sprintf(buf, "$n wracks your body with a holy force! [#r%i#n]", damage);
	act(buf, ch, NULL, victim, TO_VICT);
	hurt_person(ch, victim, damage);
    }

    WAIT_STATE(ch, 18);
    return;
}

void do_invite( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    CHAR_DATA *familiar;
    char arg[MIL];
    int max_invite;

    one_argument(argument, arg);

    if (!IS_MONK(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (get_monk(ch, MONK_TECH) < 4)
    {
	stc("You need Tech 4 for Invite Beast.\n\r", ch);
	return;
    }

    if (arg[0] == '\0')
    {
	stc("What do you wish to possess?\n\r", ch);
	return;
    }

    if ((familiar = ch->pcdata->familiar) != NULL)
    {
        stc("You already have a possessed monster.\n\r", ch);
        return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL)
    {
	stc("They aren't here.\n\r", ch);
	return;
    }

    if (!IS_NPC(victim))
    {
	stc("Not on players.\n\r", ch);
	return;
    }

    act("You stare deeply into $N's eyes.", ch, NULL, victim, TO_CHAR);
    act("$n stares deeply into $N's eyes.", ch, NULL, victim, TO_NOTVICT);
    act("$n stares deeply into your eyes." ,ch, NULL, victim, TO_VICT);

    if ((familiar = victim->wizard) != NULL)
    {
	stc("This monster is already possessed.\n\r", ch);
	return;
    }

    max_invite = get_monk(ch, MONK_TECH) * 20;
    max_invite += get_monk(ch, MONK_ART) * 10;
    max_invite += get_monk(ch, MONK_MANTRA) * 10;
    max_invite += ch->max_hit / 500;

    if (victim->level > max_invite)
    {
	stc("Your invite fails.\n\r", ch);
	return;
    }

    act("You invite $N", ch, NULL, victim, TO_CHAR);
    ch->pcdata->familiar = victim;
    victim->wizard = ch;
    return;
}

void do_cripple( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MIL];
    char buf[MIL];
    int dam;

    one_argument(argument, arg);

    if (!IS_MONK(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (get_monk(ch, MONK_TECH) < 5)
    {
	stc("You need Tech 5 for Crippling Kick.\n\r", ch);
	return;
    }

    if (ch->mana < 2000)
    {
	stc("You do not have enough mana for Crippling Kick.\n\r", ch);
	return;
    }

    if (arg[0] == '\0' && ch->fighting == NULL)
    {
	stc("Use Crippling Kick on whom?\n\r", ch);
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

    if (is_safe(ch, victim)) return;
    
    if (ch == victim)
    {
	stc("Are you insane?\n\r", ch);
	return;
    }

    ch->mana -= 2000;
    WAIT_STATE(ch, 9);
    dam = number_range(1500, 2500);

    if (!hurt_fighting(ch, victim)) return;

    sprintf(buf, "You kick $N right in the head. [#g%i#n]", dam);
    act(buf, ch, NULL, victim, TO_CHAR);
    sprintf(buf, "$n delivers a kick to your skull. [#r%i#n]", dam); 
    act(buf, ch, NULL, victim, TO_VICT);
    sprintf(buf, "$n kick $N in the head. [#0%i#n]", dam);
    act(buf, ch, NULL, victim, TO_NOTVICT);

    victim->mana -= dam * 3;
    if (victim->mana < 0) victim->mana = 0;

    hurt_person(ch, victim, dam);
    return;
}

void do_darkblaze( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA af;
    CHAR_DATA *victim;
    char arg [MIL];
    int sn;

    if (!IS_MONK(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    argument = one_argument(argument, arg);
    
    if (arg[0] == '\0')
    {
	stc("DarkBlaze who?\n\r", ch);
	return;
    }

    if (get_monk(ch, MONK_TECH) < 6)
    {
	stc("You need Tech 6 to use Dark Blaze.\n\r", ch);
	return;
    }

    if (ch->mana < 2000)
    {
	stc("You don't have enough mana.\n\r", ch);
	return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL)
    {
	stc("They aren't here.\n\r", ch);
	return;
    }

    if (victim == ch)
    {
	stc("You cannot Dark Blaze yourself!\n\r", ch);
	return;
    }
    
    if (is_safe(ch, victim)) return;

    ch->mana -= 2000;
    WAIT_STATE(ch, 18);

    if (!IS_NPC(victim))
    {
	if (IS_SET(victim->act, PLR_HOLYLIGHT)) REMOVE_BIT(victim->act, PLR_HOLYLIGHT);
    }

    act("You cloak your fist in a blaze of righteous anger and release upon $N.", ch, NULL, victim, TO_CHAR);
    act("$n throws a burst of white light at $N.", ch, NULL, victim, TO_NOTVICT);
    act("$n throws a burst of white light at you, destroying your vision.", ch, NULL, victim, TO_VICT);

    if (!IS_AFFECTED(victim, AFF_BLIND))
    {
	if ((sn = skill_lookup("blind")) < 0) return;
	af.type      = sn;
	af.location  = APPLY_HITROLL;
	af.modifier  = -50;
	af.duration  = 1;
	af.bitvector = AFF_BLIND;
	affect_to_char(victim, &af);
    }

    return;
}

void do_stunfist( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MIL];

    argument = one_argument(argument, arg);

    if (!IS_MONK(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (get_monk(ch, MONK_TECH) < 7)
    {
	stc("You need Tech 7 for Stun Fist.\n\r", ch);
	return;
    }

    stc("Disabled by Akuma.\n\r", ch);
    return;

    if (ch->mana < 5000)
    {
	stc("You don't have enough mana.\n\r", ch);
	return;
    }

    if (arg[0] == '\0' && ch->fighting == NULL)
    {
	stc("Use stunfist on whom?\n\r", ch);
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

    if (is_safe(ch, victim)) return;

    if (ch == victim)
    {
        stc("I think not...\n\r", ch);
        return;
    }

    ch->mana -= 5000;

    act("You punch $N in the gut.", ch, NULL, victim, TO_CHAR);
    act("$n punches $N in the gut.", ch, NULL, victim, TO_NOTVICT);
    act("$n punches you in the gut.", ch, NULL, victim, TO_VICT);

    act("You keel over from the force of $N's blow.", victim, NULL, ch, TO_CHAR);
    act("$n falls to one knee from $N's blow.", victim, NULL, ch, TO_NOTVICT);
    act("$n keels over from your blow.", victim, NULL, ch, TO_VICT);

    send_to_char( "You paralyze them with your punch!\n\r", ch );
    send_to_char( "You rest a moment to catch your breath.\n\r", victim );

    WAIT_STATE(ch, 24);
    WAIT_STATE(victim, 24);
    return;
}

void do_chimera( CHAR_DATA *ch, char *argument )
{
    char buf[MSL];

    if (!IS_MONK(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (get_monk(ch, MONK_TECH) < 8)
    {
	stc("You need Tech 8 for Chimera Form.\n\r", ch);
	return;
    }

    if (IS_AFFECTED(ch,AFF_POLYMORPH) && !IS_VAMPAFF(ch,VAM_DISGUISED))
    {
	stc("Not while polymorphed.\n\r", ch);
	return;
    }

    if (IS_POLYAFF(ch, POLY_CHIMERA))
    {
	free_string(ch->morph);
	ch->morph = str_dup("");
	REMOVE_BIT(ch->affected_by, AFF_POLYMORPH);
	REMOVE_BIT(ch->vampaff, VAM_DISGUISED);
	REMOVE_BIT(ch->polyaff, POLY_CHIMERA);

	stc("Your body changes back to its normal form.\n\r", ch);
        act("A menacing chimera's body changes into the form of $n.", ch, NULL, NULL, TO_ROOM);
	return;
    }
    
    stc("Your body changes into a menacing chimera.\n\r", ch);
    act("$n's body changes into a menacing chimera.", ch, NULL, NULL, TO_ROOM);
    SET_BIT(ch->affected_by, AFF_POLYMORPH);
    SET_BIT(ch->vampaff, VAM_DISGUISED);
    SET_BIT(ch->polyaff, POLY_CHIMERA);

    free_string(ch->morph);
    sprintf(buf, "#7A massive Chimera#n[#0%s#n]", ch->name);
    ch->morph = str_dup(buf);
    return;
}


void do_touch(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    char arg[MIL];
    long heal;

    argument = one_argument(argument, arg);

    if (!IS_MONK(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (get_monk(ch, MONK_MANTRA) < 4)
    {
	stc("You need Mantra 4 for Healing Touch.\n\r", ch);
	return;
    }

    if (ch->mana < 5000)
    {
	stc("You must have at least 5000 mana to use Healing Touch.\r\n", ch);
	return;
    }
    
    if (arg[0] == '\0') victim = ch;
    else if ((victim = get_char_room(ch, arg)) == NULL)
    {
    	stc("They aren't here.\n\r", ch);
     	return;
    }

    heal = number_range(400, 600) * get_monk(ch, MONK_MANTRA);
    victim->hit += heal;
    if (victim->hit > victim->max_hit) victim->hit = victim->max_hit;

    if ((ch == victim))
    {
	act("You heal yourself.", ch, NULL, victim, TO_CHAR);
	act("$n closes their eyes and begins heal.", ch,NULL, victim, TO_NOTVICT);
    }
    else
    {
	act("You lay your hands on $N and heal them.", ch, NULL, victim, TO_CHAR);
	act("$n lays their hands on you and heals your wounds.", ch,NULL, victim, TO_VICT);
	act("$n lays their hands on $N and heal them", ch,NULL, victim, TO_NOTVICT);
    }

    ch->mana -= heal * 1.5;
    update_pos(victim);

    WAIT_STATE(ch, 18);
    return;
}

void do_almighty( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    CHAR_DATA *vch;
    char arg[MIL];
    bool any_affected;

    argument = one_argument(argument, arg);

    if (!IS_MONK(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (get_monk(ch, MONK_MANTRA) < 5)
    {
	stc("You need Mantra 5 for Almighty Favor.\n\r", ch);
	return;
    }

    if (arg[0] == '\0') victim = ch;

    if (!str_cmp(arg, "group"))
    {
	if (ch->mana < 20000)
	{
	    stc("You don't have enough mana.\n\r", ch);
	    return;
	}

	stc("You pray for your group.\n\r", ch);
	act("$n kneels to pray.", ch, NULL, NULL, TO_ROOM);

	for(vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
	{
	    if (!is_same_group(ch, vch)) continue;
	    if (IS_MORE(vch, MORE_FAVOUR)) continue;
	    if (IS_NPC(vch)) continue;

	    any_affected = TRUE;
	    stc("You feel your worries disappear.\n\r", vch);
	    act("$n begins to glow with a soft light.", vch, NULL, NULL, TO_ROOM);

	    SET_BIT(vch->more, MORE_FAVOUR);
	}

	if (!any_affected)
	{
	    stc("All in your group are already blessed.\n\r", ch);
	    return;
	}

	ch->mana -= 20000;
	WAIT_STATE(ch, 24);
	return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL)
    {
	stc("They aren't here.\n\r", ch);
	return;
    }

    if (ch->mana < 5000)
    {
	stc("You don't have enough mana.\n\r", ch);
	return;
    }

    if (ch == victim)
    {
	if (IS_SET(ch->more, MORE_FAVOUR))
	{
	    stc("You are already blessed by the almighty\n\r", ch);
	    return;
	}

	stc("You take some time to quietly pray for aid.\n\r", ch);
	act("$n kneels to pray and begins to glow with a soft light.", ch, NULL, NULL, TO_ROOM);
    }
    else
    {
	if (IS_SET(victim->more, MORE_FAVOUR))
	{
	    stc("They already have the almighty one's favour.\n\r", ch);
	    return;
	}

	act("You pray for $N.", ch, NULL, victim, TO_CHAR);
	act("$n begins praying and you feel your worries disappear.", ch, NULL, victim, TO_VICT);
	act("$n begins praying and $N begins to glow with a soft light.", ch, NULL, victim, TO_NOTVICT);
    }

    ch->mana -= 5000;
    SET_BIT(victim->more, MORE_FAVOUR);
    WAIT_STATE(ch, 12);
    return;
}

void do_celestial( CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    char arg[MIL];

    argument = one_argument(argument, arg);

    if (!IS_MONK(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (get_monk(ch, MONK_MANTRA) < 7)
    {
	stc("You need Mantra 7 for Celestial Path.\n\r", ch);
	return;
    }

    if (arg[0] == '\0')
    {
	stc("Open a celestial path to who?\n\r", ch);
	return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL || victim->in_room == NULL || ch->in_room == NULL
     || victim->in_room->vnum == ch->in_room->vnum || victim->level > 800)
    {
	stc("Nothing happens.\n\r", ch);
	return;
    }
    if (IS_SET(victim->in_room->room_flags, ROOM_PRIVATE) || IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
     || IS_SET(victim->in_room->room_flags, ROOM_NO_TELEPORT)
     || (!IS_NPC(victim) && !IS_EXTRA(victim, EXTRA_SUMMON)))
    {
	stc("You are unable to focus on their location.\n\r", ch);
	if (!IS_NPC(victim) && ch != victim && IS_MORE(victim,MORE_GUARDIAN))
	{
	    guardian_message(victim);
	    act("$n just failed to use Celestial Path on you.", ch, NULL, victim, TO_VICT);
	}
	return;
    }

    act("$n opens a Celestial path and disappears in a bright flash.", ch, NULL, NULL, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, victim->in_room);

    act("You concentrate and open a Celestial path to $N!", ch, NULL, victim, TO_CHAR);
    act("$n appears before you through a Celestial portal!", ch, NULL, NULL, TO_ROOM);
    do_look(ch, "auto");
    ch->mana -= 1500;
    return;
}

void do_lifecloak( CHAR_DATA *ch, char *argument)
{
    if (!IS_MONK(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (get_monk(ch, MONK_MANTRA) < 8)
    {
	stc("You need Mantra 8 for Cloak of Life.\n\r", ch);
	return;
    }

    if (IS_MORE(ch, MORE_CLOAK))
    {
	stc("You are already protected by a Cloak of Life.\n\r", ch);
	return;
    }

    if (ch->hit < ch-> max_hit || ch->mana < ch->max_mana)
    {
	stc("You may only use this power while completely full.\n\r", ch);
	return;
    }

    stc("You wrap yourself in divine essence.\n\r", ch);
    act("$n calls upon divine forces to protect $mself.", ch, NULL, NULL, TO_ROOM);
    SET_BIT(ch->more, MORE_CLOAK);
    ch->hit = ch->hit * .8;
    ch->mana = 0;
    return;
}


void do_mclan( CHAR_DATA *ch, char *argument )
{
    char buf[MSL];
    CHAR_DATA *gch;

    if (!IS_MONK(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (strlen(ch->clan) < 2)
    {
	stc("You have no clan.\n\r", ch);
	return;
    }

    sprintf(buf, "%s Monks:\n\r", ch->clan);
    stc(buf, ch);
    stc("[      Name      ] [  Health   ] [   Mana    ]\n\r", ch);
    for (gch = char_list; gch != NULL; gch = gch->next)
    {
	if (IS_NPC(gch)) continue;
	if (!IS_MONK(gch)) continue;
	if (!str_cmp(ch->clan, gch->clan))
	{
	    sprintf(buf, "[%-16s] [%-5li/%5li] [%-5li/%5li]\n\r",
	     capitalize(gch->name), gch->hit, gch->max_hit, gch->mana, gch->max_mana);
	    stc(buf, ch);
	}
    }
    return;
}

void do_monkarmor( CHAR_DATA *ch, char *argument )
{
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    char arg[MIL];
    int vnum = 0;

    argument = one_argument(argument, arg);

    if (!IS_MONK(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }
    if (arg[0] == '\0')
    {
	stc("Please specify what kind of equipment you want to create.\n\r", ch);
	stc("Necklace, Habit, Belt, Tiger, or Dragon.\n\r", ch);
	return;
    }

    if (!str_cmp(arg, "necklace")) vnum = 29610;
    else if (!str_cmp(arg, "habit")) vnum = 29611;
    else if (!str_cmp(arg, "belt")) vnum = 29612;
    else if (!str_cmp(arg, "tiger")) vnum = 29613;
    else if (!str_cmp(arg, "dragon")) vnum = 29614;
    else
    {
	stc("That is an invalid type.\n\r", ch);
	return;
    }

    if (ch->primal < 100 || ch->pcdata->quest < 400)
    {
	stc("It costs 100 points of primal and 400 quest points to create a piece of Monk armor.\n\r", ch);
	return;
    }
    if (vnum == 0 || (pObjIndex = get_obj_index(vnum)) == NULL)
    {
	stc("Missing object, please inform Akuma.\n\r", ch);
	return;
    }

    obj = create_object(pObjIndex, 50);
    obj_to_char(obj, ch);
    ch->primal -= 100;
    ch->pcdata->quest -= 400;
    free_string(obj->questmaker);
    obj->questmaker = str_dup(ch->name);

    free_string(obj->questowner);
    obj->questowner = str_dup(ch->name);

    act("$p appears in your hands.", ch, obj, NULL, TO_CHAR);
    act("$p appears in $n's hands.", ch, obj, NULL, TO_ROOM);
    return;
}

void do_decide( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;    
    char arg[MIL];   

    one_argument(argument, arg);

    if (!IS_MONK(ch) || ch->vampgen > 3)
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (arg[0] == '\0')
    {
	stc("Syntax: Decide <victim>\n\r", ch);
	return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL)
    {
	stc("Decide who's fate?\n\r", ch);
	return;
    }

    if (IS_NPC(victim) || !IS_MONK(victim) || strlen(victim->clan) != 0)
    {
	stc("You may only decide the fate of unclanned monks.\n\r", ch);
	return;
    }

    act("You induct $N into your clan.", ch, NULL, victim, TO_CHAR);
    act("$n inducts $N into his clan.", ch, NULL, victim, TO_NOTVICT);
    act("$n inducts you in his clan.", ch, NULL, victim, TO_VICT);

    free_string(victim->clan);
    victim->clan = str_dup(ch->clan);
    victim->vampgen = 4;
    save_char_obj(ch);
    save_char_obj(victim);
    return;
}

void do_expel( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MIL];

    one_argument(argument, arg);

    if (!IS_MONK(ch) || ch->vampgen > 2)
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (arg[0] == '\0')
    {
	stc("Expel who?\n\r", ch);
	stc("Syntax 'Expel <victim>'\n\r", ch);
	return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL)
    {
	stc("They aren't here.\n\r", ch);
	return;
    }

    if (IS_NPC(victim) || !IS_MONK(victim) || str_cmp(victim->clan, ch->clan))
    {
	stc("You can only expel monks in your clan.\n\r", ch);
	return;
    }

    act("You expel $N from your clan.", ch, NULL, victim, TO_CHAR);
    act("$n expels $N from his clan.", ch, NULL, victim, TO_NOTVICT);
    act("$n expelled you from his clan.", ch, NULL, victim, TO_VICT);

    free_string(victim->clan);
    victim->clan = str_dup("");
    save_char_obj(ch);
    save_char_obj(victim);
    return;
}
