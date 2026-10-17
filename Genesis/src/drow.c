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

void do_dinduct( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MIL];

    one_argument(argument, arg);

    if (!IS_DROW(ch) || ch->vampgen > 3)
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (arg[0] == '\0' || (victim = get_char_room(ch,arg)) == NULL)
    {
	stc("Induct whom?.\n\r", ch);
	return;
    }

    if (!IS_DROW(victim) || strlen(victim->clan) != 0 || !IS_EXTRA(victim, EXTRA_VAMPIRE))
    {
	stc("You failed.\n\r", ch);
	return;
    }

    act("You bring $N into your house.", ch, NULL, victim, TO_CHAR);
    act("$n brings $N into $s house.", ch, NULL, victim, TO_NOTVICT);
    act("$n brings you into $s house.", ch, NULL, victim, TO_VICT);

    free_string(victim->clan);
    victim->clan = str_dup(ch->clan);
    victim->vampgen = 5;
    do_save(victim,"");

    return;
}

void do_promotion(CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg [MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if (!IS_DROW(ch)) stc("Huh?\n\r",ch);
    else if (arg[0] == '\0') stc("Promote whom?\n\r",ch);
    else if ((victim = get_char_room(ch,arg)) == NULL) stc("They aren't here.\n\r",ch);
    else if (IS_NPC(victim)) stc("Not on NPC's.\n\r",ch);
    else if (!IS_DROW(victim)) stc("They aren't a drow.\n\r",ch);
    else if (victim->vampgen == (ch->vampgen + 1)) stc("You may not promote them further.\n\r",ch);
    else if (victim->vampgen <= ch->vampgen) stc("You may not promote them.\n\r",ch);
    else if (str_cmp(victim->clan, ch->clan)) stc("They are NOT in your clan!.\n\r",ch);
    else
    {
        victim->vampgen--;
        do_save(victim, "");
	act("You promote $N.",ch,NULL,victim,TO_CHAR);
	act("$n promotes $N.",ch,NULL,victim,TO_NOTVICT);
	act("$n promotes you.",ch,NULL,victim,TO_VICT);
    }
    return;
}
        
void do_failure( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char      arg [MAX_INPUT_LENGTH];

    argument = one_argument(argument, arg);

    if (!IS_DROW(ch)) send_to_char("Huh?\n\r",ch);
    else if (arg[0] == '\0') send_to_char("Demote whom?\n\r",ch);
    else if ((victim = get_char_room(ch,arg)) == NULL) send_to_char("They aren't here.\n\r",ch);
    else if (IS_NPC(victim)) send_to_char("Not on NPC's.\n\r",ch);
    else if (!IS_DROW(victim)) send_to_char("They aren't a drow.\n\r",ch);
    else if (victim->vampgen >= 5) stc("You may not demote them further.\n\r",ch);
    else if (victim->vampgen <= ch->vampgen) stc("You may not demote them.\n\r",ch);
    else if (str_cmp(victim->clan, ch->clan)) send_to_char("They are NOT in your clan!.\n\r",ch);
    else
    {
        victim->vampgen++;
        do_save(victim, "");
	act("You demote $N.",ch,NULL,victim,TO_CHAR);
	act("$n demotes $N.",ch,NULL,victim,TO_NOTVICT);
	act("$n demotes you.",ch,NULL,victim,TO_VICT);
    }
    return;
}

void do_attune( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument(argument, arg);

    if (!IS_DROW(ch))
    {
	stc("Huh?\n\r",ch);
	return;
    }
    if (!get_drow(ch, DROW_ATTUNE))
    {
	stc("You need to obtain attune first.\n\r",ch);
	return;
    }
    if (arg[0] == '\0' || (obj = get_obj_carry(ch, arg)) == NULL)
    {
	stc("What object do you wish to attune?\n\r", ch);
	return;
    }
    if (obj->item_type == ITEM_QUEST || obj->item_type == ITEM_AMMO || obj->item_type == ITEM_EGG
     || obj->item_type == ITEM_VOODOO || obj->item_type == ITEM_MONEY || obj->item_type == ITEM_TREASURE
     || obj->item_type == ITEM_PAGE || IS_SET(obj->quest, QUEST_ARTIFACT))
    {
	stc("You cannot attune that item.\n\r", ch);
	return;
    }
    if (str_cmp(ch->name,obj->questowner))
    {
	stc("You do not own this object.\n\r", ch);
	return;
    }
    if (IS_SET(obj->extra_flags, ITEM_LOYAL))
    {
	stc("This object is already attuned to you.\n\r", ch);
	return;
    }

    act("You attune $p to yourself.",ch,obj,NULL,TO_CHAR);
    SET_BIT(obj->extra_flags, ITEM_LOYAL);
    return;
}

void do_acidbolt( CHAR_DATA *ch, char *argument )
{
    char arg [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument(argument, arg);

    if (!IS_DROW(ch))
    {
	stc("Huh?\n\r",ch);
	return;
    }

    if (!get_drow(ch, DROW_ACID))
    {
	stc("You need to obtain acid bolt first.\n\r",ch);
	return;
    }

    if (IS_ITEMAFF(ch, ITEMA_PEACE)) do_majesty(ch,"");
 
    if (arg[0] == '\0' && ch->fighting == NULL)
    {
	stc("Who do you wish to use Acid Bolt on?\n\r",ch);
	return;
    }

    if ((victim = ch->fighting) == NULL && (victim = get_char_room(ch, arg)) == NULL)
    {
	stc("They aren't here.\n\r", ch);
	return;
    }
    
    if (is_safe(ch, victim)) return;

    act("You fire a bolt of acid at $N!", ch, NULL, victim, TO_CHAR);
    act("$n fires a bolt of acid at you!", ch, NULL, victim, TO_VICT);
    act("$n fires a bolt of acid at $N!", ch, NULL, victim, TO_NOTVICT);
    one_hit(ch, victim, TYPE_HIT, "acid bolt");
    WAIT_STATE(ch, 8);
    return;  
}

void do_faeriefire( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    AFFECT_DATA af;
    char arg[MAX_INPUT_LENGTH];
    long sn;

    one_argument(argument, arg);

    if (!IS_DROW(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (!get_drow(ch, DROW_FIRE))
    {
	stc("You need to obtain faerie fire first.\n\r",ch);
	return;
    }

    if (arg[0] == '\0' || (victim = get_char_room(ch, arg)) == NULL)
    {
	stc("Who do you wish to use Faerie Fire on?\n\r", ch);
	return;
    }
        
    if (IS_AFFECTED(victim, AFF_FAERIE_FIRE))
    {
	stc("They are already surrounded by faerie fire.\n\r",ch);
	return;
    }
    
    if (is_safe(ch, victim)) return;

    if (!IS_NPC(victim) && victim->pcdata->atm >= number_percent())
    {
	stc("Nothing happens.\n\r", ch);
	return;
    }

    if ((sn = skill_lookup("faerie fire")) < 0)
    {
	stc("Bug - please inform Akuma.\n\r", ch);
	return;
    }

    act("A look of concentration crosses $n's face.", ch, NULL, NULL, TO_ROOM);
    stc("You are surrounded by a dull red light!\n\r",victim);
    act("$n is surrounded by a dull red light!", victim, NULL, NULL, TO_ROOM);

    af.type      = sn;   
    af.duration  = 10;
    af.location  = APPLY_DAMROLL;
    af.modifier  = -50;
    af.bitvector = AFF_FAERIE_FIRE;
    affect_to_char(victim, &af);

    af.location  = APPLY_HITROLL;
    af.bitvector = 0;
    affect_to_char(victim, &af);

    af.location  = APPLY_AC;
    af.modifier  = -50;
    affect_to_char(victim, &af);

    stc("You are surrounded by a pink outline.\n\r", victim);
    act("$n is surrounded by a pink outline.", victim, NULL, NULL, TO_ROOM);

    WAIT_STATE(ch, 12);
    return;
}

void do_summonspider( CHAR_DATA *ch, char *argument )
{
    char buf [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    AFFECT_DATA af;
    long sn;

    if (!IS_DROW(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (!get_drow(ch, DROW_SPIDER))
    {
	stc("You need to obtain summon spider first.\n\r", ch);
	return;
    }
 
    if (ch->pcdata->followers >= 1)
    {
	stc("Nothing happens.\n\r",ch);
	return;
    }

    ch->pcdata->followers++;
    victim = create_mobile(get_mob_index(MOB_VNUM_GUARDIAN));
    char_to_room(victim,get_room_index(ROOM_VNUM_DISCONNECTION));
    free_string(victim->lord);
    victim->lord = str_dup(ch->name);
    free_string(victim->name);
    victim->name = str_dup("drow spider");
    free_string(victim->short_descr);
    victim->short_descr = str_dup("a fiendish spider");
    free_string(victim->long_descr);
    victim->long_descr = str_dup("A fiendish spider is here, awaiting orders.\n\r");
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

    strcpy(buf,"Come forth, minion of Lloth, and serve me!");
    do_say(ch, buf);

    stc("A spider skitters out of the shadows and kneels before you.\n\r", ch);
    act("$N skitters out of the shadows and kneels before $n.", ch, NULL, victim, TO_ROOM);

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
    return;

}

void do_drowarmor( CHAR_DATA *ch, char *argument )
{
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    char arg[MAX_INPUT_LENGTH];
    long vnum = 0;

    argument = one_argument(argument, arg);

    if (!IS_DROW(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (arg[0] == '\0')
    {
	stc("Please specify which piece of drow armor you wish to make:\n\r", ch);
	stc("  Cloak, Helmet, Breastplate, Bracer, Crossbow.\n\r", ch);
	return;
    }

    if (ch->pcdata->classpoints[CP_CURRENT] < 8000 || ch->pcdata->quest < 400)
    {
	stc("It costs 8000 drow points and 400 quest points to create drow armor.\n\r", ch);
	return;
    }

    if (!str_cmp(arg,"cloak")) vnum = 29652;
    else if (!str_cmp(arg,"helmet")) vnum = 29653;
    else if (!str_cmp(arg,"breastplate")) vnum = 29654;
    else if (!str_cmp(arg,"bracer")) vnum = 29655;
    else if (!str_cmp(arg,"crossbow")) vnum = 29658;
    else
    {
	do_drowarmor(ch,"");
	return;
    }
     
    if ((pObjIndex = get_obj_index(vnum)) == NULL)
    {
	stc("Missing object, please inform Akuma.\n\r", ch);
	return;
    }

    obj = create_object(pObjIndex, 50);
    obj_to_char(obj, ch);
    ch->pcdata->classpoints[CP_CURRENT] -= 8000;
    ch->pcdata->quest -= 400;
    free_string(obj->questmaker);
    obj->questmaker = str_dup(ch->name);
    free_string(obj->questowner);
    obj->questowner = str_dup(ch->name);

    act("$p appears in your hands.",ch,obj,NULL,TO_CHAR);
    act("$p appears in $n's hands.",ch,obj,NULL,TO_ROOM);
    return;
}

void do_door( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument( argument, arg );

    if (!IS_DROW(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (arg[0] == '\0')
    {
	stc("Who do you wish to open the door to?\n\r", ch);
	return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL || victim->in_room == NULL || ch->in_room == NULL
     || (IS_NPC(victim) && victim->pIndexData->vnum == MOB_VNUM_EYE) || victim->in_room == ch->in_room 
     || victim->level > 800)
    {
	stc("Nothing happens.\n\r", ch);
	return;
    }

    if (IS_SET(victim->in_room->room_flags, ROOM_SAFE) || IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
     || IS_SET(victim->in_room->room_flags, ROOM_SOLITARY) || IS_SET(victim->extra, EXTRA_ILLUSION)
     || (!IS_NPC(victim) && !IS_EXTRA(victim, EXTRA_SUMMON)))
    {
	stc("You are unable to focus on their location.\n\r", ch);
	if (!IS_NPC(victim) && IS_MORE(victim, MORE_GUARDIAN))
	{
	    guardian_message(victim);
	    act("$n just failed to open a Dimensional Door to you.", ch, NULL, victim, TO_VICT);
        }
        return;
    }

    stc("You step through a dimensional door.\n\r", ch);
    act("$n steps through a freshly created dimensional door and closes it behind $m.", ch, NULL, NULL, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, victim->in_room);    
    act("$n steps out of a dimensional door and closes it behind $m.", ch, NULL, NULL, TO_ROOM);
    do_look(ch, "auto");
    return;
}

void do_prayer( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg1[MIL];
    char arg2[MIL];
    long grant = 0, cost = 0;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (!IS_DROW(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (arg1[0] == '\0' || arg2[0] == '\0')
    {
	stc("Syntax: prayer <person> <power>\n\r", ch);
	stc("AcidBolt (5000), LlothsWeb (8000), Darkness (9500), FaerieFire (5000),\n\r", ch);
	stc("SpiderBite (7500), Dual Wield (10000), Blindfight (5000), SummonSpider (8000),\n\r", ch);
	stc("SpiderEyes (7500), Readaura (6000), Darkvision (5000), Heighten (20000).\n\r", ch);
	stc("PoisonImmunity (5000), SkipDisc (5000), Attunement (1000),\n\r", ch);
	stc("Speed (10000 per increase), Toughness (8000), Might (8000),\n\r", ch);
	return;
    }

    if ((victim = get_char_room(ch, arg1)) == NULL)
    {
	stc("Nobody by that name.\n\r", ch);
	return;
    }

    if (!IS_DROW(victim))
    {
	stc("Only on a drow.\n\r", ch);
	return;
    }

    if (victim != ch && (victim->clan[0] == '\0' || ch->clan[0] == '\0' || str_cmp(victim->clan, ch->clan)))
    {
	stc("They are not of your house.\n\r", ch);
	return;
    }

    if (!str_cmp(arg2, "acidbolt")) {grant = DROW_ACID; cost = 5000;}
    else if (!str_cmp(arg2, "llothsweb")) {grant = DROW_WEB; cost = 8000;}
    else if (!str_cmp(arg2, "darkness")) {grant = DROW_DARK; cost = 9500;}
    else if (!str_cmp(arg2, "faeriefire")) {grant = DROW_FIRE; cost = 5000;}
    else if (!str_cmp(arg2, "spiderbite")) {grant = DROW_BITE; cost = 7500;}
    else if (!str_cmp(arg2, "dualwield")) {grant = DROW_DUALWIELD; cost = 10000;}
    else if (!str_cmp(arg2, "blindfight")) {grant = DROW_BFIGHT; cost = 5000;}
    else if (!str_cmp(arg2, "summonspider")) {grant = DROW_SPIDER; cost = 8000;}
    else if (!str_cmp(arg2, "spidereyes")) {grant = DROW_EYES; cost = 7500;}
    else if (!str_cmp(arg2, "readaura")) {grant = DROW_READ; cost = 6000;}
    else if (!str_cmp(arg2, "darkvision")) {grant = DROW_NIGHT; cost = 5000;}
    else if (!str_cmp(arg2, "heighten")) {grant = DROW_SIGHT; cost = 20000;}
    else if (!str_cmp(arg2, "poisonimmunity")) {grant = DROW_PIMM; cost = 5000;}
    else if (!str_cmp(arg2, "skipdisc")) {grant = DROW_SKIP; cost = 5000;}
    else if (!str_cmp(arg2, "attunement")) {grant = DROW_ATTUNE; cost = 1000;}
    else if (!str_cmp(arg2, "might")) {grant = DROW_MIGHT; cost = 8000;}
    else if (!str_cmp(arg2, "toughness")) {grant = DROW_TOUGH; cost = 8000;}
    else if (!str_cmp(arg2, "speed"))
    {
	cost = 10000;
	if (victim->pcdata->drow[DROW_SPEED] >= 4)
	{
	    stc("They already move at full speed.\n\r", ch);
	    return;
	}
	if (ch->pcdata->classpoints[CP_CURRENT] < cost)
	{
	    stc("You have insufficient power to grant that gift.\n\r", ch);
	    return;
	}
	ch->pcdata->classpoints[CP_CURRENT] -= cost;
	victim->pcdata->drow[DROW_SPEED]++;
	if (victim != ch) stc("You have been granted a gift from your patron!\n\r", victim);
	stc("Ok.\n\r", ch);

	if (victim != ch) do_autosave(ch, "");
	do_autosave(victim, "");
	return;
    }
    else
    {
	do_prayer(ch, "");
	return;
    }

    if (get_drow(victim, grant))
    {
	stc("They have already got that power.\n\r", ch);
	return;
    }
    if (ch->pcdata->classpoints[CP_CURRENT] < cost)
    {
	stc("You have insufficient power to grant that gift.\n\r", ch);
	return;
    }
    SET_BIT(victim->pcdata->drow[DROW_POWERS], grant);
    ch->pcdata->classpoints[CP_CURRENT] -= cost;
    if (victim != ch) stc("You have been granted a gift from your patron!\n\r", victim);
    stc("Ok.\n\r", ch);
    if (victim != ch) do_autosave(ch, "");
    do_autosave(victim, "");
    return;
}

void do_skipdisc( CHAR_DATA *ch, char *argument )
{
    if (!IS_DROW(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (!get_drow(ch, DROW_SKIP))
    {
	stc("You need to obtain skip disc first.\n\r", ch );
	return;
    }

    if (IS_ITEMAFF(ch, AFF_FLYING))
    {
	stc("You leap off your skip disc.\n\r", ch);
	REMOVE_BIT(ch->affected_by, AFF_FLYING);
    }
    else
    {
	stc("You leap onto your skip disc.\n\r", ch);
	SET_BIT(ch->affected_by, AFF_FLYING);
    }
    return;
}

void do_drowupkeep( CHAR_DATA *ch, char *argument )
{
    char buf[15];
    int loop, counter = 0;

    if (!IS_DROW(ch))
    {
	stc("Huh?\n\r",ch);
	return;
    }

    stc("Drow Upkeep\n\r", ch);
    if (ch->pcdata->drow[DROW_POWERS] == 0)
    {
	stc("You have no drow abilities", ch);
	return;
    }

    for (loop = 1; loop <= DROW_MAX; loop *= 2)
    {
	if (!get_drow(ch, loop)) continue;

	if (loop == DROW_ACID) stc("Acid bolt         ", ch);
	else if (loop == DROW_WEB) stc("Lloth's web       ", ch);
	else if (loop == DROW_DARK) stc("Globe of darkness ", ch);
	else if (loop == DROW_FIRE) stc("Faerie fire       ", ch);
	else if (loop == DROW_BITE) stc("Spider bite       ", ch);
	else if (loop == DROW_DUALWIELD) stc("Dual Wield        ", ch);
	else if (loop == DROW_BFIGHT) stc("Blind fighting    ", ch);
	else if (loop == DROW_SPIDER) stc("Summon spider     ", ch);
	else if (loop == DROW_EYES) stc("Spider eyes       ", ch);
	else if (loop == DROW_READ) stc("Readaura          ", ch);
	else if (loop == DROW_NIGHT) stc("Darkvision        ", ch);
	else if (loop == DROW_SIGHT) stc("Heighten          ", ch);
	else if (loop == DROW_PIMM) stc("Poison immunity   ", ch);
	else if (loop == DROW_SKIP) stc("Skip disc         ", ch);
	else if (loop == DROW_ATTUNE) stc("Attunement        ", ch);
	else if (loop == DROW_MIGHT) stc("Might             ", ch);
	else if (loop == DROW_TOUGH) stc("Toughness         ", ch);

	counter++;
	counter %= 4;
	if (counter == 0) stc("\n\r", ch);
    }

    if (ch->pcdata->drow[DROW_SPEED] > 0)
    {
	sprintf(buf, "Speed %li%s", ch->pcdata->drow[DROW_SPEED], counter == 3 ? "" : "\n\r");
	stc(buf, ch);
    }

    return;
}

void do_globedark( CHAR_DATA *ch, char *argument )
{
    if (!IS_DROW(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (!get_drow(ch, DROW_DARK))
    {
	stc("You need to obtain darkness first.\n\r", ch);
	return;
    }

    if (get_drow(ch, DROW_GLOBE))
    {
	REMOVE_BIT(ch->pcdata->drow[DROW_POWERS], DROW_GLOBE);
	stc("You banish your globe of darkness.\n\r", ch);
	act("Darkness fades from the room as $n relaxes.", ch, NULL, NULL, TO_ROOM);
    }
    else
    {
	if (ch->pcdata->classpoints[CP_CURRENT] < 500)
	{
	    stc("You do not have enough drow points to activate globe of darkness.\n\r", ch);
	    return;
	}
	stc("You summon a globe of darkness!\n\r", ch);
	act("$n concentrates as the room fades into darkness!", ch, NULL, NULL, TO_ROOM);
	SET_BIT(ch->pcdata->drow[DROW_POWERS], DROW_GLOBE);
	ch->pcdata->classpoints[CP_CURRENT] -= 500;
    }
    return;
}

void do_shoot( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;
    char arg1[MIL];
    char arg2[MIL];
    char buf[MSL];
    char revdoor[MIL];
    int door;
    long sn, damage;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (!IS_DROW(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (ch->pcdata->classpoints[CP_CURRENT] < 500)
    {
	stc("You need at least 500 dp to shoot your crossbow.\n\r", ch);
	return;
    }

    if ((obj = get_eq_char(ch, WEAR_HOLD)) == NULL || obj->item_type != ITEM_MISSILE)
    {
	stc("You are not holding anything to shoot.\n\r", ch);
	return;
    }

    if (arg1[0] == '\0' || arg2[0] == '\0')
    {
	stc("Syntax: shoot <direction> <target>.\n\r", ch);
	return;
    }

    if ((get_char_room(ch, arg2)) != NULL)
    {
	stc("You can shoot them from here.\n\r", ch);
	return;
    }

    if (IS_SET(ch->in_room->room_flags, ROOM_SAFE))
    {
	stc("Something in the air prevents you from attacking.\n\r", ch);
	return;
    }

    if (!str_cmp(arg1, "n") || !str_cmp(arg1, "north"))
	{door = 0; sprintf(arg1, "north");}
    else if (!str_cmp(arg1, "e") || !str_cmp(arg1, "east"))
	{door = 1; sprintf(arg1, "east");}
    else if (!str_cmp(arg1, "s") || !str_cmp(arg1, "south"))
	{door = 2; sprintf(arg1, "south");}
    else if (!str_cmp(arg1, "w") || !str_cmp(arg1, "west"))
	{door = 3; sprintf(arg1, "west");}
    else if (!str_cmp(arg1, "u") || !str_cmp(arg1, "up"))
	{door = 4; sprintf(arg1, "up");}
    else if (!str_cmp(arg1, "d") || !str_cmp(arg1, "down"))
	{door = 5; sprintf(arg1, "down");}
    else
    {
	stc("You can only shoot north, south, east, west, up or down.\n\r", ch);
	return;
    }

    if (!str_cmp(arg1, "north")) {door = 0; sprintf(revdoor, "south");}
    else if (!str_cmp(arg1, "east")) {door = 1; sprintf(revdoor, "west");}
    else if (!str_cmp(arg1, "south")) {door = 2; sprintf(revdoor, "north");}
    else if (!str_cmp(arg1, "west")) {door = 3; sprintf(revdoor, "east");}
    else if (!str_cmp(arg1, "up")) {door = 4; sprintf(revdoor, "down");}
    else if (!str_cmp(arg1, "down")) {door = 5; sprintf(revdoor, "up");}
    else return;

    location = ch->in_room;

    sprintf(buf, "You point $p %s and shoot.", arg1);
    act(buf, ch, obj, NULL, TO_CHAR);
    sprintf(buf, "$n points $p %s and shoots.", arg1);
    act(buf, ch, obj, NULL, TO_ROOM);

    ch->pcdata->classpoints[CP_CURRENT] -= 500;

    if ((pexit = ch->in_room->exit[door]) == NULL || (to_room = pexit->to_room) == NULL)
    {
	act("$n's crossbow bolt bounces off a wall.", ch, NULL, NULL, TO_ROOM);
	act("Your crossbow bolt bounces off a wall.", ch, NULL, NULL, TO_CHAR);
	return;
    }

    pexit = ch->in_room->exit[door];
    if (IS_SET(pexit->exit_info, EX_CLOSED))
    {
	act("$n's crossbow bolt bounces off a door.", ch, NULL, NULL, TO_ROOM);
	act("Your crossbow bolt bounces off a door.", ch, NULL, NULL, TO_CHAR);
	return;
    }

    ch->in_room = to_room;

    if ((victim = get_char_room(ch, arg2)) != NULL)
    {
	if (is_safe(ch, victim))
	{
	    ch->in_room = location;
	    return;
	}
	sprintf(buf, "A crossbow bolt comes flying in from the %s and strikes $N.", revdoor);
	act(buf, ch, NULL, victim, TO_NOTVICT);
	sprintf(buf, "A crossbow bolt comes flying in from the %s and strikes you.", revdoor);
	act(buf, ch, NULL, victim, TO_VICT);

	damage = number_range(10 * char_damroll(ch), 20 * char_damroll(ch));
	if (!IS_NPC(victim)) damage /= 10;

	hurt_person(ch, victim, damage);
	if (victim == NULL) return;

	if (IS_DROW(victim) && get_drow(victim, DROW_PIMM))
	{
	    stc("Your poison has been resisted.\n\r", ch);
	    stc("You were hit by a crossbow bolt, but resisted it's poison.\n\r", victim);
	}
	else if (!IS_AFFECTED(victim, AFF_SLEEP))
	{
	    do_stance(victim, "");
    	    if ((sn = skill_lookup("sleep")) < 0) return;
    	    af.type = sn;
    	    af.location = APPLY_NONE;
    	    af.modifier = 0;
    	    af.duration = 1;
    	    af.bitvector = AFF_SLEEP;
    	    affect_to_char(victim, &af);
	    victim->position = POS_SLEEPING;
  	}
    }
 
    ch->in_room = location;
    return;
}
