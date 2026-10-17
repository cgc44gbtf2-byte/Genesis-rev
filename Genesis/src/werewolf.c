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

/* Local functions */

char *	gift_breed_name		args( ( int gift_no ) );
char *	gift_auspice_name	args( ( int gift_no ) );
char *	gift_tribe_name		args( ( int gift_no ) );

void do_advance( CHAR_DATA *ch, char *argument )
{
    char buf[MIL];
    int rank, qdone, mkills;

    if (!IS_WEREWOLF(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (ch->pcdata->rank < 1) ch->pcdata->rank = 1;

    rank = ch->pcdata->rank;

    if (rank >= 4)
    {
	stc("You may not advance farther on your own.\n\r", ch);
	return;
    }

    qdone = rank * 20;

    if (ch->pcdata->score[SCORE_NUM_QUEST] < qdone)
    {
	sprintf(buf,"You need to complete %i quests to advance a rank.\n\r", qdone);
	stc(buf, ch);
	return;
    }

    mkills = rank * 1000;

    if (ch->mkill < mkills)
    {
        sprintf(buf,"You need to kill %i monsters to advance a rank.\n\r", mkills);
        stc(buf, ch);
        return;
    }

    stc("You advance a rank!\n\r", ch);
    ch->pcdata->rank++;
    return;
}

void do_wwpowers( CHAR_DATA *ch, char *argument )
{
    char buf[MSL];
    int sn;
    bool any_powers = FALSE;

    if (!IS_WEREWOLF(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    for (sn = 0; sn <= BREED_MAX; sn++)
	if (get_breed(ch, sn) > 0) any_powers = TRUE;
    for (sn = 0; sn <= AUSPICE_MAX; sn++)
	if (get_auspice(ch, sn) > 0) any_powers = TRUE;
    for (sn = 0; sn <= TRIBE_MAX; sn++)
	if (get_tribe(ch, sn) > 0) any_powers = TRUE;

    stc("--------------------------------------------------------------------------------\n\r", ch);
    stc("                               -= Gift Commands =-\n\r", ch);
    stc("--------------------------------------------------------------------------------\n\r", ch);
    if (get_breed(ch, BREED_HOMID) >= 1)
    {
	strcpy(buf, "Homid: Persuasion");
	if (get_breed(ch, BREED_HOMID) >= 2) strcat(buf, " Jamtech");
	if (get_breed(ch, BREED_HOMID) >= 3) strcat(buf, " Reshape");
	if (get_breed(ch, BREED_HOMID) >= 4) strcat(buf, " Disquiet");
	if (get_breed(ch, BREED_HOMID) >= 5) strcat(buf, " Ward");
	strcat(buf, ".\n\r");
	stc(buf, ch);
    }
    if (get_breed(ch, BREED_METIS) >= 1)
    {
	strcpy(buf, "Metis: Primalanger");
	if (get_breed(ch, BREED_METIS) >= 2) strcat(buf, " Burrow");
	if (get_breed(ch, BREED_METIS) >= 3) strcat(buf, " Nightsight");
	if (get_breed(ch, BREED_METIS) >= 4) strcat(buf, " Porcupine");
	if (get_breed(ch, BREED_METIS) >= 5) strcat(buf, " Totemgift");
	strcat(buf, ".\n\r");
	stc(buf, ch);
    }
    if (get_breed(ch, BREED_LUPUS) >= 1)
    {
	strcpy(buf, "Lupus: Heighten");
	if (get_breed(ch, BREED_LUPUS) >= 2) strcat(buf, " Scry");
	if (get_breed(ch, BREED_LUPUS) >= 4) strcat(buf, " Beastlife");
	if (get_breed(ch, BREED_LUPUS) >= 5) strcat(buf, " Elemental");
	strcat(buf, ".\n\r");
	stc(buf, ch);
    }
    if (get_auspice(ch, AUSPICE_RAGABASH) >= 1)
    {
	strcpy(buf, "Ragabash: Blur");
	if (get_auspice(ch, AUSPICE_RAGABASH) >= 2) strcat(buf, " Blissful");
	if (get_auspice(ch, AUSPICE_RAGABASH) >= 4) strcat(buf, " Whelp");
	strcat(buf, ".\n\r");
	stc(buf, ch);
    }
    if (get_auspice(ch, AUSPICE_THEURGE) >= 1)
    {
	strcpy(buf, "Theurge: Mother");
	if (get_auspice(ch, AUSPICE_THEURGE) >= 3) strcat(buf, " Pulse");
	if (get_auspice(ch, AUSPICE_THEURGE) >= 4) strcat(buf, " Grasp");
	if (get_auspice(ch, AUSPICE_THEURGE) >= 5) strcat(buf, " Lobotomy");
	strcat(buf, ".\n\r");
	stc(buf, ch);
    }
    if (get_auspice(ch, AUSPICE_PHILODOX) >= 1)
    {
	strcpy(buf, "Philodox:");
	if (get_auspice(ch, AUSPICE_PHILODOX) >= 4) strcat(buf, " Wallgranite");
	if (get_auspice(ch, AUSPICE_PHILODOX) >= 5) strcat(buf, " Geas");
	strcat(buf, ".\n\r");
	stc(buf, ch);
    }
    if (get_auspice(ch, AUSPICE_GALLIARD) >= 1)
    {
	strcpy(buf, "Galliard: Wyld");
	if (get_auspice(ch, AUSPICE_GALLIARD) >= 3) strcat(buf, " Wyrmcall");
	if (get_auspice(ch, AUSPICE_GALLIARD) >= 4) strcat(buf, " Songofrage");
	if (get_auspice(ch, AUSPICE_GALLIARD) >= 5) strcat(buf, " Bridgewalk");
	strcat(buf, ".\n\r");
	stc(buf, ch);
    }
    if (get_auspice(ch, AUSPICE_AHROUN) >= 1)
    {
	strcpy(buf, "Ahroun:");
	if (get_auspice(ch, AUSPICE_AHROUN) >= 5) strcat(buf, " Helios");
	strcat(buf, ".\n\r");
	stc(buf, ch);
    }
    if (get_tribe(ch, TRIBE_BLACK_FURIES) >= 1)
    {
	strcpy(buf, "Black Furies: Breathwyld");
	if (get_auspice(ch, TRIBE_BLACK_FURIES) >= 2) strcat(buf, " Aeolus");
	if (get_auspice(ch, TRIBE_BLACK_FURIES) >= 4) strcat(buf, " Bodywrack");
	strcat(buf, ".\n\r");
	stc(buf, ch);
    }
    if (get_tribe(ch, TRIBE_BONE_GNAWERS) >= 1)
    {
	strcpy(buf, "Bone Gnawers:");
	if (get_auspice(ch, TRIBE_BLACK_FURIES) >= 3) strcat(buf, " Friend");
	if (get_auspice(ch, TRIBE_BLACK_FURIES) >= 4) strcat(buf, " Infest");
	strcat(buf, ".\n\r");
	stc(buf, ch);
    }
    if (get_tribe(ch, TRIBE_CHILDREN_OF_GAIA) >= 1)
    {
	strcpy(buf, "Children of Gaia: Calm");
	if (get_tribe(ch, TRIBE_CHILDREN_OF_GAIA) >= 3) strcat(buf, " Dazzle");
	if (get_tribe(ch, TRIBE_CHILDREN_OF_GAIA) >= 4) strcat(buf, " Spiritfriend");
	if (get_tribe(ch, TRIBE_CHILDREN_OF_GAIA) >= 5) strcat(buf, " Halo");
	strcat(buf, ".\n\r");
	stc(buf, ch);
    }
    if (get_tribe(ch, TRIBE_FIANNA) >= 1)
    {
	strcpy(buf, "Fianna: Faerielight");
	if (get_tribe(ch, TRIBE_FIANNA) >= 2) strcat(buf, " Howlbanshee");
	if (get_tribe(ch, TRIBE_FIANNA) >= 3) strcat(buf, " Illusionaryform");
	if (get_tribe(ch, TRIBE_FIANNA) >= 4) strcat(buf, " Cloneobject");
	if (get_tribe(ch, TRIBE_FIANNA) >= 5) strcat(buf, " Balorsgaze");
	strcat(buf, ".\n\r");
	stc(buf, ch);
    }
    if (get_tribe(ch, TRIBE_GET_OF_FENRIS) >= 1)
    {
	strcpy(buf, "Get of Fenris:");
	if (get_tribe(ch, TRIBE_GET_OF_FENRIS) >= 4) strcat(buf, " Scream");
	strcat(buf, ".\n\r");
	stc(buf, ch);
    }
    if (get_tribe(ch, TRIBE_GLASS_WALKERS) >= 1)
    {
	strcpy(buf, "Glass Walkers: Diagnostics");
	if (get_tribe(ch, TRIBE_GLASS_WALKERS) >= 2) strcat(buf, " Readaura");
	if (get_tribe(ch, TRIBE_GLASS_WALKERS) >= 3) strcat(buf, " Surge");
	if (get_tribe(ch, TRIBE_GLASS_WALKERS) >= 4) strcat(buf, " Doppleganger");
	if (get_tribe(ch, TRIBE_GLASS_WALKERS) >= 5) strcat(buf, " Summonnetspider");
	strcat(buf, ".\n\r");
	stc(buf, ch);
    }
    if (get_tribe(ch, TRIBE_RED_TALONS) >= 1)
    {
	strcpy(buf, "Red Talons:");
	if (get_tribe(ch, TRIBE_RED_TALONS) >= 4) strcat(buf, " Quicksand");
	if (get_tribe(ch, TRIBE_RED_TALONS) >= 5) strcat(buf, " Lycaon");
	strcat(buf, ".\n\r");
	stc(buf, ch);
    }
    if (get_tribe(ch, TRIBE_SHADOW_LORDS) >= 1)
    {
	strcpy(buf, "Shadow Lords:");
	if (get_tribe(ch, TRIBE_SHADOW_LORDS) >= 2) strcat(buf, " Clap");
	if (get_tribe(ch, TRIBE_SHADOW_LORDS) >= 3) strcat(buf, " Paralyz");
	if (get_tribe(ch, TRIBE_SHADOW_LORDS) >= 4) strcat(buf, " Dominator");
	if (get_tribe(ch, TRIBE_SHADOW_LORDS) >= 5) strcat(buf, " Obedience");
	strcat(buf, ".\n\r");
	stc(buf, ch);
    }
    if (get_tribe(ch, TRIBE_SILENT_STRIDERS) >= 1)
    {
	strcpy(buf, "Silent Striders:");
	if (get_tribe(ch, TRIBE_SILENT_STRIDERS) >= 5) strcat(buf, " Reach");
	strcat(buf, ".\n\r");
	stc(buf, ch);
    }
    if (get_tribe(ch, TRIBE_SILVER_FANGS) >= 1)
    {
	strcpy(buf, "Silver Fangs:");
	if (get_tribe(ch, TRIBE_SILVER_FANGS) >= 3) strcat(buf, " Wrath");
	if (get_tribe(ch, TRIBE_SILVER_FANGS) >= 4) strcat(buf, " Shield");
	if (get_tribe(ch, TRIBE_SILVER_FANGS) >= 5) strcat(buf, " Mastery");
	strcat(buf, ".\n\r");
	stc(buf, ch);
    }
    if (get_tribe(ch, TRIBE_STARGAZERS) >= 1)
    {
	strcpy(buf, "Stargazers: Falling");
	strcat(buf, ".\n\r");
	stc(buf, ch);
    }
    if (get_tribe(ch, TRIBE_UKTENA) >= 1)
    {
	strcpy(buf, "Uktena: Shroud");
	if (get_tribe(ch, TRIBE_UKTENA) >= 4) strcat(buf, " Invisibility");
	if (get_tribe(ch, TRIBE_UKTENA) >= 5) strcat(buf, " Fetishdoll");
	strcat(buf, ".\n\r");
	stc(buf, ch);
    }
    if (get_tribe(ch, TRIBE_WENDIGOS) >= 1)
    {
	strcpy(buf, "Wendigos: Camouflage");
	if (get_tribe(ch, TRIBE_WENDIGOS) >= 2) strcat(buf, " Cutting");
	if (get_tribe(ch, TRIBE_WENDIGOS) >= 3) strcat(buf, " Bloodyfeast");
	if (get_tribe(ch, TRIBE_WENDIGOS) >= 5) strcat(buf, " Invoke");
	strcat(buf, ".\n\r");
	stc(buf, ch);
    }
    if (!any_powers) stc("You have no gifts.\n\r", ch);
    stc("--------------------------------------------------------------------------------\n\r", ch);
    return;
}

void do_gifts( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];
    char buf[MIL];
    char disc[25];
    int improve, imp_cost, sn, col, gt, gift, ww_max;

    one_argument(argument, arg);

    if (!IS_WEREWOLF(ch))
    {
	stc("Huh?\n\r",ch);
	return;
    }

    if (arg[0] == '\0')
    {
	stc("--------------------------------------------------------------------------------\n\r", ch);
	stc("                                  -= Gifts =-\n\r", ch);
	stc("--------------------------------------------------------------------------------\n\r", ch);
	col = 0;
	for (sn = BREED_HOMID; sn <= BREED_METIS; sn++)
	{
	    switch (sn)
	    {
		default: strcpy(disc, "None"); break;
		case BREED_HOMID: strcpy(disc, "Homid"); break;
		case BREED_LUPUS: strcpy(disc, "Lupus"); break;
		case BREED_METIS: strcpy(disc, "Metis"); break;
	    }
	    if (ch->pcdata->tgift[0] == sn) strcpy(disc, upper_cap(disc));

	    sprintf(buf, "%-16s: %-2li      ", disc, get_breed(ch, sn));
	    stc(buf, ch);
	    if (++col % 3 == 0) stc("\n\r", ch);
	}

	for (sn = AUSPICE_RAGABASH; sn <= AUSPICE_AHROUN; sn++)
	{
	    switch (sn)
	    {
		default: strcpy(disc, "None"); break;
		case AUSPICE_RAGABASH: strcpy(disc, "Ragabash"); break;
		case AUSPICE_THEURGE: strcpy(disc, "Theurge"); break;
		case AUSPICE_PHILODOX: strcpy(disc, "Philodox"); break;
		case AUSPICE_GALLIARD: strcpy(disc, "Galliard"); break;
		case AUSPICE_AHROUN: strcpy(disc, "Ahroun"); break;
	    }
	    if (ch->pcdata->tgift[1] == sn) strcpy(disc, upper_cap(disc));

	    sprintf(buf, "%-16s: %-2li      ", disc, get_auspice(ch, sn));
	    stc(buf, ch);
	    if (++col % 3 == 0) stc("\n\r", ch);
	}

	for (sn = TRIBE_BLACK_FURIES; sn <= TRIBE_WENDIGOS; sn++)
	{
	    switch (sn)
	    {
		default: strcpy(disc, "None"); break;
		case TRIBE_BLACK_FURIES: strcpy(disc, "Black Furies"); break;
		case TRIBE_BONE_GNAWERS: strcpy(disc, "Bone Gnawers"); break;
		case TRIBE_CHILDREN_OF_GAIA: strcpy(disc, "Children of Gaia"); break;
		case TRIBE_FIANNA: strcpy(disc, "Fianna"); break;
		case TRIBE_GET_OF_FENRIS: strcpy(disc, "Get of Fenris"); break;
		case TRIBE_GLASS_WALKERS: strcpy(disc, "Glass Walkers"); break;
		case TRIBE_RED_TALONS: strcpy(disc, "Red Talons"); break;
		case TRIBE_SHADOW_LORDS: strcpy(disc, "Shadow Lords"); break;
		case TRIBE_SILENT_STRIDERS: strcpy(disc, "Silent Striders"); break;
		case TRIBE_SILVER_FANGS: strcpy(disc, "Silver Fangs"); break;
		case TRIBE_STARGAZERS: strcpy(disc, "Stargazers"); break;
		case TRIBE_UKTENA: strcpy(disc, "Uktena"); break;
		case TRIBE_WENDIGOS: strcpy(disc, "Wendigos"); break;
	    }
	    if (ch->pcdata->tgift[2] == sn) strcpy(disc, upper_cap(disc));

	    sprintf(buf, "%-16s: %-2li      ", disc, get_tribe(ch, sn));
	    stc(buf, ch);
	    if (++col % 3 == 0) stc("\n\r", ch);
	}

	if (col % 3 != 0) stc("\n\r", ch);
	stc("--------------------------------------------------------------------------------\n\r", ch);
	stc("To improve a gift rating, type: gifts <name>\n\r", ch);
	stc("--------------------------------------------------------------------------------\n\r", ch);
	return;
    }

    if (!str_cmp(argument, "homid")) {improve = BREED_HOMID; gt = 0;}
    else if (!str_cmp(argument, "lupus")) {improve = BREED_LUPUS; gt = 0;}
    else if (!str_cmp(argument, "metis")) {improve = BREED_METIS; gt = 0;}
    else if (!str_cmp(argument, "ragabash")) {improve = AUSPICE_RAGABASH; gt = 1;}
    else if (!str_cmp(argument, "theurge")) {improve = AUSPICE_THEURGE; gt = 1;}
    else if (!str_cmp(argument, "philodox")) {improve = AUSPICE_PHILODOX; gt = 1;}
    else if (!str_cmp(argument, "galliard")) {improve = AUSPICE_GALLIARD; gt = 1;}
    else if (!str_cmp(argument, "ahroun")) {improve = AUSPICE_AHROUN; gt = 1;}
    else if (!str_cmp(argument, "black furies")) {improve = TRIBE_BLACK_FURIES; gt = 2;}
    else if (!str_cmp(argument, "bone gnawers")) {improve = TRIBE_BONE_GNAWERS; gt = 2;}
    else if (!str_cmp(argument, "children of gaia")) {improve = TRIBE_CHILDREN_OF_GAIA; gt = 2;}
    else if (!str_cmp(argument, "fianna")) {improve = TRIBE_FIANNA; gt = 2;}
    else if (!str_cmp(argument, "get of fenris")) {improve = TRIBE_GET_OF_FENRIS; gt = 2;}
    else if (!str_cmp(argument, "glass walkers")) {improve = TRIBE_GLASS_WALKERS; gt = 2;}
    else if (!str_cmp(argument, "red talons")) {improve = TRIBE_RED_TALONS; gt = 2;}
    else if (!str_cmp(argument, "shadow lords")) {improve = TRIBE_SHADOW_LORDS; gt = 2;}
    else if (!str_cmp(argument, "silent striders")) {improve = TRIBE_SILENT_STRIDERS; gt = 2;}
    else if (!str_cmp(argument, "silver fangs")) {improve = TRIBE_SILVER_FANGS; gt = 2;}
    else if (!str_cmp(argument, "stargazers")) {improve = TRIBE_STARGAZERS; gt = 2;}
    else if (!str_cmp(argument, "uktena")) {improve = TRIBE_UKTENA; gt = 2;}
    else if (!str_cmp(argument, "wendigos")) {improve = TRIBE_WENDIGOS; gt = 2;}
    else
    {
	stc("You know of no such gift.\n\r", ch);
	return;
    }

    switch (gt)
    {
	default: stc("Error, Inform Akuma.\n\r", ch); return;
	case 0: gift = ch->pcdata->breed[improve]; break;
	case 1: gift = ch->pcdata->auspice[improve]; break;
	case 2: gift = ch->pcdata->tribes[improve]; break;
    }

    if (ch->pcdata->tgift[gt] == improve) ww_max = 5;
    else ww_max = 4;

    if (gift >= ww_max)
    {
	stc("You are unable to improve your ability in that gift any further.\n\r", ch);
	return;
    }

    if (ch->pcdata->tgift[gt] == improve) imp_cost = (gift + 1) * 10;
    else imp_cost = (gift + 1) * 15;

    if (ch->primal < imp_cost)
    {
	sprintf(buf, "You need %i primal to increase that gift.\n\r", imp_cost);
	stc(buf, ch);
	return;
    }

    switch (gt)
    {
	default: stc("Error, inform Akuma.\n\r", ch); return;
	case 0: ch->pcdata->breed[improve]++; break;
	case 1: ch->pcdata->auspice[improve]++; break;
	case 2: ch->pcdata->tribes[improve]++; break;
    }

    ch->primal -= imp_cost;
    stc("You improve your gift.\n\r", ch);

    return;
}

void do_tribegifts( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];
    char buf[MIL];
    int improve, sn, col, gtype;

    strcpy(arg, argument);

    if (!IS_WEREWOLF(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (arg[0] == '\0')
    {
	stc("--------------------------------------------------------------------------------\n\r", ch);
	stc("                                  -= Gifts =-\n\r", ch);
	stc("--------------------------------------------------------------------------------\n\r", ch);
	sprintf(buf, "Breed Gift: %s.\n\r", gift_breed_name(ch->pcdata->tgift[0]));
	stc(buf, ch);
	sprintf(buf, "Auspice Gift: %s.\n\r", gift_auspice_name(ch->pcdata->tgift[1]));
	stc(buf, ch);
	sprintf(buf, "Tribe Gift: %s.\n\r", gift_tribe_name(ch->pcdata->tgift[2]));
	stc(buf, ch);

	stc("--------------------------------------------------------------------------------\n\r", ch);

	if (ch->pcdata->tgift[0] == -1)
	{
	    stc("Please select a breed gift from the list below.\n\r", ch);
	    stc("--------------------------------------------------------------------------------\n\r", ch);
	    col = 0;
	    for (sn = 0; sn <= BREED_MAX; sn++)
	    {
		sprintf(buf, "%-15s           ", gift_breed_name(sn));
		stc(buf, ch);
		if (++col % 3 == 0) stc("\n\r", ch);
	    }
	    if (col % 3 != 0) stc("\n\r", ch);
	    stc("--------------------------------------------------------------------------------\n\r", ch);
	}
	else if (ch->pcdata->tgift[1] == -1)
	{
	    stc("Please select an auspice gift from the list below.\n\r",ch);
	    stc("--------------------------------------------------------------------------------\n\r", ch);
	    col = 0;
	    for ( sn = 0; sn <= AUSPICE_MAX; sn++ )
	    {
		sprintf(buf, "%-15s           ", gift_auspice_name(sn));
		stc(buf, ch);
		if (++col % 3 == 0) stc("\n\r", ch);
	    }
	    if (col % 3 != 0) stc("\n\r", ch);
	    stc("--------------------------------------------------------------------------------\n\r", ch);
	}
	else if (ch->pcdata->tgift[2] == -1)
	{
	    stc("Please select a tribe gift from the list below.\n\r", ch);
	    stc("--------------------------------------------------------------------------------\n\r", ch);
	    col = 0;
	    for (sn = 0; sn <= TRIBE_MAX; sn++)
	    {
		sprintf(buf, "%-15s           ", gift_tribe_name(sn));
		stc(buf, ch);
		if (++col % 3 == 0) stc("\n\r", ch);
	    }
	    if (col % 3 != 0) stc("\n\r", ch);
	    stc("--------------------------------------------------------------------------------\n\r", ch);
	}
	return;
    }

    if (!str_cmp(arg, "homid")) {improve = BREED_HOMID; gtype = 0;}
    else if (!str_cmp(arg, "lupus")) {improve = BREED_LUPUS; gtype = 0;}
    else if (!str_cmp(arg, "metis")) {improve = BREED_METIS; gtype = 0;}
    else if (!str_cmp(arg, "ragabash")) {improve = AUSPICE_RAGABASH; gtype = 1;}
    else if (!str_cmp(arg, "theurge")) {improve = AUSPICE_THEURGE; gtype = 1;}
    else if (!str_cmp(arg, "philodox")) {improve = AUSPICE_PHILODOX; gtype = 1;}
    else if (!str_cmp(arg, "galliard")) {improve = AUSPICE_GALLIARD; gtype = 1;}
    else if (!str_cmp(arg, "ahroun")) {improve = AUSPICE_AHROUN; gtype = 1;}
    else if (!str_cmp(arg, "black furies")) {improve = TRIBE_BLACK_FURIES; gtype = 2;}
    else if (!str_cmp(arg, "bone gnawers")) {improve = TRIBE_BONE_GNAWERS; gtype = 2;}
    else if (!str_cmp(arg, "children of gaia")) {improve = TRIBE_CHILDREN_OF_GAIA; gtype = 2;}
    else if (!str_cmp(arg, "fianna")) {improve = TRIBE_FIANNA; gtype = 2;}
    else if (!str_cmp(arg, "get of fenris")) {improve = TRIBE_GET_OF_FENRIS; gtype = 2;}
    else if (!str_cmp(arg, "glass walkers")) {improve = TRIBE_GLASS_WALKERS; gtype = 2;}
    else if (!str_cmp(arg, "red talons")) {improve = TRIBE_RED_TALONS; gtype = 2;}
    else if (!str_cmp(arg, "shadow lords")) {improve = TRIBE_SHADOW_LORDS; gtype = 2;}
    else if (!str_cmp(arg, "silent striders")) {improve = TRIBE_SILENT_STRIDERS; gtype = 2;}
    else if (!str_cmp(arg, "silver fangs")) {improve = TRIBE_SILVER_FANGS; gtype = 2;}
    else if (!str_cmp(arg, "stargazers")) {improve = TRIBE_STARGAZERS; gtype = 2;}
    else if (!str_cmp(arg, "uktena")) {improve = TRIBE_UKTENA; gtype = 2;}
    else if (!str_cmp(arg, "wendigos")) {improve = TRIBE_WENDIGOS; gtype = 2;}
    else
    {
	stc("You know of no such gift.\n\r", ch);
	return;
    }

    switch (gtype)
    {
	default:
	    if (ch->pcdata->tgift[0] != -1) stc("You already have a breed gift.\n\r", ch);
	    else 
	    {
		ch->pcdata->tgift[0] = improve;
		stc("Ok.\n\r", ch);
	    }
	    break;
	case 1:
	    if (ch->pcdata->tgift[1] != -1) stc("You already have an auspice gift.\n\r", ch);
	    else
	    {
		ch->pcdata->tgift[1] = improve;
		stc("Ok.\n\r", ch);
	    }
	    break;
	case 2:
	    if (ch->pcdata->tgift[2] != -1) stc("You already have a tribe gift.\n\r", ch);
	    else
	    {
		ch->pcdata->tgift[2] = improve;
		stc("Ok.\n\r", ch);
	    }
	    break;
    }
    return;
}

char *gift_breed_name( int gift_no )
{
    static char disc[6];

    switch (gift_no)
    {
	default: strcpy(disc, "None"); break;
	case BREED_HOMID: strcpy(disc, "Homid"); break;
	case BREED_LUPUS: strcpy(disc, "Lupus"); break;
	case BREED_METIS: strcpy(disc, "Metis"); break;
    }
    return &disc[0];
}

char *gift_auspice_name( int gift_no )
{
    static char disc[9];

    switch (gift_no)
    {
	default: strcpy(disc, "None"); break;
	case AUSPICE_RAGABASH: strcpy(disc, "Ragabash"); break;
	case AUSPICE_THEURGE: strcpy(disc, "Theurge"); break;
	case AUSPICE_PHILODOX: strcpy(disc, "Philodox"); break;
	case AUSPICE_GALLIARD: strcpy(disc, "Galliard"); break;
	case AUSPICE_AHROUN: strcpy(disc, "Ahroun"); break;
    }
    return &disc[0];
}

char *gift_tribe_name( int gift_no )
{
    static char disc[17];

    switch (gift_no)
    {
	default: strcpy(disc, "None"); break;
	case TRIBE_BLACK_FURIES: strcpy(disc, "Black Furies"); break;
	case TRIBE_BONE_GNAWERS: strcpy(disc, "Bone Gnawers"); break;
	case TRIBE_CHILDREN_OF_GAIA: strcpy(disc, "Children of Gaia"); break;
	case TRIBE_FIANNA: strcpy(disc, "Fianna"); break;
	case TRIBE_GET_OF_FENRIS: strcpy(disc, "Get of Fenris"); break;
	case TRIBE_GLASS_WALKERS: strcpy(disc, "Glass Walkers"); break;
	case TRIBE_RED_TALONS: strcpy(disc, "Red Talons"); break;
	case TRIBE_SHADOW_LORDS: strcpy(disc, "Shadow Lords"); break;
	case TRIBE_SILENT_STRIDERS: strcpy(disc, "Silent Striders"); break;
	case TRIBE_SILVER_FANGS: strcpy(disc, "Silver Fangs"); break;
	case TRIBE_STARGAZERS: strcpy(disc, "Stargazers"); break;
	case TRIBE_UKTENA: strcpy(disc, "Uktena"); break;
	case TRIBE_WENDIGOS: strcpy(disc, "Wendigos"); break;
    }
    return &disc[0];
}
