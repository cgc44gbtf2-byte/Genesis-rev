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
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <malloc.h>
#include "merc.h"
#include "atomic_save.h"

#if !defined(macintosh)
extern	long	_filbuf		args( (FILE *) );
#endif



/*
 * Array of containers read for proper re-nesting of objects.
 */
#define MAX_NEST	100
static	OBJ_DATA *	rgObjNest	[MAX_NEST];

char *print_flags(long flag)
{
    long count, pos = 0;
    static char buf[52];


    for (count = 0; count < 32;  count++)
    {
        if (IS_SET(flag,1<<count))
        {
            if (count < 26)
                buf[pos] = 'A' + count;
            else
                buf[pos] = 'a' + (count - 26);
            pos++;
        }
    }

    if (pos == 0)
    {
        buf[pos] = '0';
        pos++;
    }

     buf[pos] = '\0';

    return buf;
}

/*
 * Local functions.
 */
void	fwrite_char	args( ( CHAR_DATA *ch,  FILE *fp ) );
void	fwrite_obj	args( ( CHAR_DATA *ch,  OBJ_DATA  *obj,
			    FILE *fp, long iNest ) );
void	fread_char	args( ( CHAR_DATA *ch,  FILE *fp ) );
void	fread_obj	args( ( CHAR_DATA *ch,  FILE *fp ) );
void	fread_tiny_obj	args( ( CHAR_DATA *ch,  FILE *fp ) );



/*
 * Save a character and inventory.
 */
static int write_player_save(FILE *fp, void *context)
{
    CHAR_DATA *ch = context;
    fwrite_char(ch, fp);
    if (ch->carrying != NULL)
        fwrite_obj(ch, ch->carrying, fp, 0);
    return fprintf(fp, "#END\n") < 0 || ferror(fp) ? -1 : 0;
}

static void save_player_atomic(CHAR_DATA *ch, const char *directory)
{
    struct atomic_save_error error;
    enum atomic_save_result result;
    char message[MAX_STRING_LENGTH];
    char name[256];
    size_t i;

    if ( IS_NPC(ch) || ch->level < 2 )
        return;

    if ( ch->desc != NULL && ch->desc->connected != CON_PLAYING )
        return;

    if ( ch->desc != NULL && ch->desc->original != NULL )
        ch = ch->desc->original;

    /* Validate before capitalization or construction of any filesystem path.
     * Match capitalize()'s ASCII spelling without its shared static buffer. */
    if (!atomic_save_name_valid(ch->name))
    {
        bug("Player save rejected: invalid player filename", 0);
        return;
    }
    for (i = 0; ch->name[i] != '\0'; ++i)
    {
        unsigned char c = (unsigned char)ch->name[i];
        name[i] = i == 0 ? UPPER(c) : LOWER(c);
    }
    name[i] = '\0';
    ch->save_time = current_time;
    if (fpReserve != NULL)
    {
        fclose(fpReserve);
        fpReserve = NULL;
    }
    result = atomic_save(directory, name, write_player_save, ch, &error);
    fpReserve = fopen( NULL_FILE, "r" );
    if (fpReserve == NULL)
        bug("Player save: could not reopen reserve file", 0);
    if (result != ATOMIC_SAVE_OK)
    {
        /* No player name, serialized data or player path goes to diagnostics. */
        snprintf(message, sizeof(message), "%s save: %s: %s",
                 strcmp(directory, BACKUP_DIR) == 0 ? "Backup" : "Player",
                 error.stage, strerror(error.number));
        bug(message, 0);
        if (error.cleanup_number != 0)
        {
            snprintf(message, sizeof(message),
                     "Player save: temporary cleanup failed: %s",
                     strerror(error.cleanup_number));
            bug(message, 0);
        }
    }
}

void save_char_obj( CHAR_DATA *ch )
{
    save_player_atomic(ch, PLAYER_DIR);
}

void save_char_obj_backup( CHAR_DATA *ch )
{
    save_player_atomic(ch, BACKUP_DIR);
}

/*
 * Write the char.
 */
void fwrite_char( CHAR_DATA *ch, FILE *fp )
{
    AFFECT_DATA *paf;
    long i;

    fprintf( fp, "#PLAYER\n"					);

    fprintf( fp, "Name         %s~\n",	ch->name		);

    fprintf( fp, "ShortDescr   %s~\n",	ch->short_descr		);
    fprintf( fp, "LongDescr    %s~\n",	ch->long_descr		);
    fprintf( fp, "Description  %s~\n",	ch->description		);
    fprintf( fp, "Lord         %s~\n",	ch->lord		);
    fprintf( fp, "Clan         %s~\n",	ch->clan		);
    fprintf( fp, "Morph        %s~\n",	ch->morph		);
    fprintf( fp, "Createtime   %s~\n",	ch->createtime		);
    fprintf( fp, "Lasttime     %s~\n",	ch->lasttime		);
    fprintf( fp, "Lasthost     %s~\n",	ch->lasthost		);
    fprintf( fp, "Prompt       %s~\n",	ch->prompt		);
    fprintf( fp, "Cprompt      %s~\n",	ch->cprompt		);
    fprintf( fp, "Sex          %li\n",	ch->sex			);
    fprintf( fp, "Status       %li\n",	ch->status		);
    fprintf( fp, "Class        %li\n",	ch->class		);
    fprintf( fp, "Polyaff      %li\n",	ch->polyaff		);
    fprintf( fp, "Itemaffect   %li\n",	ch->itemaffect		);
    fprintf( fp, "Vampaff      %li\n",	ch->vampaff		);
    fprintf( fp, "Beast        %li\n",	ch->beast		);
    fprintf( fp, "Truebeast    %li\n",	ch->truebeast		);
    fprintf( fp, "Vampgen      %li\n",	ch->vampgen		);
    fprintf( fp, "Polytimer    %li\n",  ch->polymorph_timer	);
    fprintf( fp, "Better_timer %li\n",  ch->better_body_timer	);
    fprintf( fp, "Fight_timer  %li\n",  ch->fight_timer         );
    fprintf( fp, "Home         %li\n",  ch->home		);
    fprintf( fp, "HomeCV       %li\n",	ch->pcdata->homecv	);
    fprintf( fp, "Level        %li\n",	ch->level		);
    fprintf( fp, "Played       %li\n", ch->played + (int) (current_time - ch->logon) );
    fprintf( fp, "Room         %li\n",
	(ch->in_room == get_room_index( ROOM_VNUM_LIMBO ) && ch->was_in_room != NULL ) ? ch->was_in_room->vnum : ch->in_room->vnum );
    fprintf( fp, "PkPdMkMd     %li %li %li %li\n", ch->pkill, ch->pdeath, ch->mkill, ch->mdeath );
    fprintf( fp, "Stance       %li %li %li %li %li %li %li %li %li %li %li %li\n",
	ch->stance[0], ch->stance[1], ch->stance[2], ch->stance[3],
	ch->stance[4], ch->stance[5], ch->stance[6], ch->stance[7],
	ch->stance[8], ch->stance[9], ch->stance[10],ch->stance[11]);
    fprintf( fp, "Style        %li %li %li %li %li %li %li\n",
	ch->style[0], ch->style[1], ch->style[2], ch->style[3],
	ch->style[4], ch->style[5], ch->style[6]);
    fprintf( fp, "Weapon       %li %li %li %li %li %li\n",
	ch->weapon[0], ch->weapon[1], ch->weapon[2], ch->weapon[3],
	ch->weapon[4], ch->weapon[5]);
    fprintf( fp, "HpMana       %li %li %li %li\n",
	ch->hit, ch->max_hit, ch->mana, ch->max_mana );
    fprintf( fp, "Gold         %li\n",	ch->gold		);
    fprintf( fp, "Exp          %li\n",	ch->exp			);
    fprintf( fp, "Act          %li\n",  ch->act			);
    fprintf( fp, "Extra        %li\n",  ch->extra		);
    fprintf( fp, "More         %li\n",  ch->more		);
    fprintf( fp, "Xtra         %li\n",  ch->xtra		);
    fprintf( fp, "AffectedBy   %li\n",	ch->affected_by		);
    fprintf( fp, "Spheres      %li %li %li %li %li %li %li %li %li\n",
         ch->spheres[0], ch->spheres[1], ch->spheres[2], ch->spheres[3],
         ch->spheres[4], ch->spheres[5], ch->spheres[6], ch->spheres[7],
         ch->spheres[8] );
    fprintf( fp, "Paradox      %li\n",	ch->paradox	);
    fprintf( fp, "Ward         %li\n", ch->paradox_ward            );
    fprintf( fp, "Arete        %li\n",  ch->arete );
    fprintf( fp, "Quint        %li %li %li\n", ch->quint[0], ch->quint[1], ch->quint[2]);
    fprintf( fp, "Contingency	%s~\n", ch->contingency);
    /* Bug fix from Alander */
    fprintf( fp, "Position     %li\n",
        ch->position == POS_FIGHTING ? POS_STANDING : ch->position );

    fprintf( fp, "Primal       %li\n",	ch->primal		);
    fprintf( fp, "SavingThrow  %li\n",	ch->saving_throw	);
    fprintf( fp, "Hitroll      %li\n",	ch->hitroll		);
    fprintf( fp, "Damroll      %li\n",	ch->damroll		);
    fprintf( fp, "Armor        %li\n",	ch->armor		);
    fprintf( fp, "Deaf         %li\n",	ch->deaf		);
    if ( IS_NPC(ch) )
    {
	fprintf( fp, "Vnum         %li\n",	ch->pIndexData->vnum	);
    }
    else
    {
	fprintf (fp, "Boards       %d ", MAX_BOARD);
	for (i = 0; i < MAX_BOARD; i++) fprintf (fp, "%s %li ", boards[i].short_name, ch->pcdata->last_note[i]);
	fprintf (fp, "\n");
	fprintf( fp, "Password     %s~\n",	ch->pcdata->pwd		);
	fprintf( fp, "Bamfin       %s~\n",	ch->pcdata->bamfin	);
	fprintf( fp, "Bamfout      %s~\n",	ch->pcdata->bamfout	);
	fprintf( fp, "Cdest        %s~\n",	ch->pcdata->cdest	);
	fprintf( fp, "Title        %s~\n",	ch->pcdata->title	);
	fprintf( fp, "Decap_char   %s~\n",	ch->pcdata->decap_char  );
	fprintf( fp, "Marriage     %s~\n",	ch->pcdata->marriage	);
	fprintf( fp, "Email        %s~\n",	ch->pcdata->email	);
	fprintf( fp, "Conding      %s~\n",	ch->pcdata->conding	);
	fprintf( fp, "Love         %s~\n",	ch->pcdata->love	);
	fprintf( fp, "Covenant     %s~\n",	ch->pcdata->covenant	);
	fprintf( fp, "CovTime      %s~\n",	ch->pcdata->cov_time	);
	fprintf( fp, "Leader       %s~\n",	ch->pcdata->leader	);
	fprintf( fp, "VLeader      %s~\n",	ch->pcdata->vleader	);
	fprintf( fp, "Stats        %li %li %li %li %li\n",
	    ch->stats[0], ch->stats[1], ch->stats[2],
	    ch->stats[3], ch->stats[4] );
	fprintf( fp, "DenyDate     %li\n",	ch->pcdata->denydate	);
	fprintf( fp, "Quest        %li\n", ch->pcdata->quest );
	fprintf( fp, "Wolf         %li\n", ch->pcdata->wolf );
	fprintf( fp, "Rank         %li\n", ch->pcdata->rank );
	fprintf( fp, "Regenerate   %li\n", ch->pcdata->regenerate );
	fprintf( fp, "Atm          %li\n", ch->pcdata->atm );
	fprintf( fp, "Bodycrafts   %li\n", ch->pcdata->bodycrafts );
	fprintf( fp, "Absorb       %li %li %li\n",
	    ch->pcdata->absorb[0],
	    ch->pcdata->absorb[1],
	    ch->pcdata->absorb[2] );

	fprintf( fp, "Score        %li %li %li %li %li %li\n",
	    ch->pcdata->score[0], ch->pcdata->score[1], ch->pcdata->score[2],
	    ch->pcdata->score[3], ch->pcdata->score[4], ch->pcdata->score[5]);
	fprintf( fp, "Dragcol      %li\n", ch->pcdata->dragcol);
	fprintf( fp, "Colors       %li %li %li %li %li %li\n",
	    ch->pcdata->colors[0], ch->pcdata->colors[1], ch->pcdata->colors[2],
	    ch->pcdata->colors[3], ch->pcdata->colors[4], ch->pcdata->colors[5]);
	fprintf( fp, "Clandisc     %li %li %li\n",
	    ch->pcdata->cland[0], ch->pcdata->cland[1], ch->pcdata->cland[2]);
        fprintf( fp, "Tribegift    %li %li %li\n",
	    ch->pcdata->tgift[0], ch->pcdata->tgift[1], ch->pcdata->tgift[2]);
	fprintf( fp, "Diableries   %li %li\n", ch->pcdata->diableries[0], ch->pcdata->diableries[1] );
	fprintf( fp, "Breed        %li %li %li\n",
	    ch->pcdata->breed[0], ch->pcdata->breed[1], ch->pcdata->breed[2]);
	fprintf( fp, "Auspice      %li %li %li %li %li\n",
	    ch->pcdata->auspice[0], ch->pcdata->auspice[1],
	    ch->pcdata->auspice[2], ch->pcdata->auspice[3],
	    ch->pcdata->auspice[4]);
	fprintf( fp, "Tribes       %li %li %li %li %li %li %li %li %li %li %li %li %li %li %li %li %li %li %li %li\n",
	    ch->pcdata->tribes[0], ch->pcdata->tribes[1], ch->pcdata->tribes[2],
	    ch->pcdata->tribes[3], ch->pcdata->tribes[4], ch->pcdata->tribes[5], 
	    ch->pcdata->tribes[6], ch->pcdata->tribes[7], ch->pcdata->tribes[8],
	    ch->pcdata->tribes[9], ch->pcdata->tribes[10],ch->pcdata->tribes[11],
	    ch->pcdata->tribes[12],ch->pcdata->tribes[13],ch->pcdata->tribes[14],
	    ch->pcdata->tribes[15],ch->pcdata->tribes[16],ch->pcdata->tribes[17],
	    ch->pcdata->tribes[18],ch->pcdata->tribes[19]);
	fprintf( fp, "Powers       %li %li %li %li %li %li %li %li %li %li %li %li %li %li %li %li %li %li %li %li\n",
	    ch->pcdata->powers[0], ch->pcdata->powers[1], ch->pcdata->powers[2],
	    ch->pcdata->powers[3], ch->pcdata->powers[4], ch->pcdata->powers[5], 
	    ch->pcdata->powers[6], ch->pcdata->powers[7], ch->pcdata->powers[8],
	    ch->pcdata->powers[9], ch->pcdata->powers[10],ch->pcdata->powers[11],
	    ch->pcdata->powers[12],ch->pcdata->powers[13],ch->pcdata->powers[14],
	    ch->pcdata->powers[15],ch->pcdata->powers[16],ch->pcdata->powers[17],
	    ch->pcdata->powers[18],ch->pcdata->powers[19]);
	fprintf( fp, "Blood        %li %li\n",
	    ch->blood[0],
	    ch->blood[1]);
        fprintf( fp, "DSkills      %li\n", ch->pcdata->dragskills );
	fprintf( fp, "Drowa        %li %li\n", ch->pcdata->drow[0], ch->pcdata->drow[1]);
	fprintf( fp, "Highland     %li %li %li %li\n",
	 ch->pcdata->highland[0], ch->pcdata->highland[1], ch->pcdata->highland[2], ch->pcdata->highland[3]);
	fprintf( fp, "Monk         %li %li %li %li\n",
	 ch->pcdata->monk[0], ch->pcdata->monk[1], ch->pcdata->monk[2], ch->pcdata->monk[3]);
	fprintf( fp, "Ninja        %li %li %li %li %li\n",
	 ch->pcdata->ninja[0], ch->pcdata->ninja[1], ch->pcdata->ninja[2],
	 ch->pcdata->ninja[3], ch->pcdata->ninja[4]);
	fprintf( fp, "Willpower    %li %li\n",
	    ch->pcdata->willpower[0], ch->pcdata->willpower[1] );
	fprintf( fp, "Resist       %li %li %li %li %li\n",
	    ch->pcdata->resist[0],
	    ch->pcdata->resist[1],
	    ch->pcdata->resist[2],
	    ch->pcdata->resist[3],
	    ch->pcdata->resist[4] );
	fprintf( fp, "Fake         %li %li %li %li %li %li %li %li %li\n",
	    ch->pcdata->fake_skill, 
	    ch->pcdata->fake_stance, 
	    ch->pcdata->fake_hit,
	    ch->pcdata->fake_dam,
	    ch->pcdata->fake_ac,
	    ch->pcdata->fake_hp,
	    ch->pcdata->fake_mana,
	    ch->pcdata->fake_max_hp,
	    ch->pcdata->fake_max_mana);
	if (ch->pcdata->obj_vnum != 0)
	    fprintf( fp, "Objvnum      %li\n", ch->pcdata->obj_vnum );
	fprintf( fp, "Majesty      %li\n", ch->pcdata->majesty );
	fprintf( fp, "Flag         %li\n", ch->pcdata->flag );
	fprintf( fp, "Classpoints  %li %li\n", ch->pcdata->classpoints[0], ch->pcdata->classpoints[1]);
    }

    for ( paf = ch->affected; paf != NULL; paf = paf->next )
    {
	/* Thx Alander */
	if ( paf->type < 0 || paf->type >= MAX_SKILL )
	    continue;

	fprintf( fp, "AffectData   '%s' %3li %3li %3li %10li\n",
	    skill_table[paf->type].name,
	    paf->duration,
	    paf->modifier,
	    paf->location,
	    paf->bitvector
	    );
    }

    fprintf( fp, "End\n\n" );
    return;
}



/*
 * Write an object and its contents.
 */
void fwrite_obj( CHAR_DATA *ch, OBJ_DATA *obj, FILE *fp, long iNest )
{
    EXTRA_DESCR_DATA *ed;
    AFFECT_DATA *paf;

    /*
     * Slick recursion to write lists backwards, so loading them will load in forwards order.
     */
    if (obj->next_content != NULL) fwrite_obj(ch, obj->next_content, fp, iNest);

    if (obj->chobj != NULL) return;
    if ((obj->questmaker == NULL || strlen(obj->questmaker) <= 1) && 
	(obj->questowner == NULL || strlen(obj->questowner) <= 1) && 
	(obj->wear_loc != WEAR_NONE)) return;

    fprintf( fp, "#OBJECT\n" );
    fprintf( fp, "Nest         %li\n",	iNest			     );
    fprintf( fp, "Name         %s~\n",	obj->name		     );
    fprintf( fp, "ShortDescr   %s~\n",	obj->short_descr	     );
    fprintf( fp, "Description  %s~\n",	obj->description	     );
    if (obj->chpoweron != NULL && strlen(obj->chpoweron) > 1 && str_cmp(obj->chpoweron,"(null)") )
    	fprintf( fp, "Poweronch    %s~\n",	obj->chpoweron	     );
    if (obj->chpoweroff != NULL && strlen(obj->chpoweroff) > 1 && str_cmp(obj->chpoweroff,"(null)"))
    	fprintf( fp, "Poweroffch   %s~\n",	obj->chpoweroff      );
    if (obj->chpoweruse != NULL && strlen(obj->chpoweruse) > 1 && str_cmp(obj->chpoweruse,"(null)"))
    	fprintf( fp, "Powerusech   %s~\n",	obj->chpoweruse      );
    if (obj->victpoweron != NULL && strlen(obj->victpoweron) > 1 && str_cmp(obj->victpoweron,"(null)"))
    	fprintf( fp, "Poweronvict  %s~\n",	obj->victpoweron     );
    if (obj->victpoweroff != NULL && strlen(obj->victpoweroff) > 1 && str_cmp(obj->victpoweroff,"(null)"))
    	fprintf( fp, "Poweroffvict %s~\n",	obj->victpoweroff    );
    if (obj->victpoweruse != NULL && strlen(obj->victpoweruse) > 1 && str_cmp(obj->victpoweruse,"(null)"))
    	fprintf( fp, "Powerusevict %s~\n",	obj->victpoweruse    );
    if (obj->actcommands != NULL && strlen(obj->actcommands) > 1)
    	fprintf( fp, "ActCommands  %s~\n",	obj->actcommands     );
    if (obj->questmaker != NULL && strlen(obj->questmaker) > 1)
    	fprintf( fp, "Questmaker   %s~\n",	obj->questmaker      );
    if (obj->questowner != NULL && strlen(obj->questowner) > 1)
    	fprintf( fp, "Questowner   %s~\n",	obj->questowner      );
    fprintf( fp, "Vnum         %li\n",	obj->pIndexData->vnum	     );

    fprintf( fp, "ExtraFlags   %li\n",	obj->extra_flags	     );
    fprintf( fp, "WearFlags    %li\n",	obj->wear_flags		     );
    fprintf( fp, "WearLoc      %li\n",	obj->wear_loc		     );
    fprintf( fp, "ItemType     %li\n",	obj->item_type		     );
    fprintf( fp, "Weight       %li\n",	obj->weight		     );
    if (obj->spectype != 0)
    	fprintf( fp, "Spectype     %li\n",	obj->spectype	     );
    if (obj->specpower != 0)
    	fprintf( fp, "Specpower    %li\n",	obj->specpower	     );
    fprintf( fp, "Toughness    %li\n",	obj->toughness		     );
    if (obj->quest != 0)
    	fprintf( fp, "Quest        %li\n",	obj->quest	     );
    if (obj->points != 0)
    	fprintf( fp, "Points       %li\n",	obj->points	     );
    fprintf( fp, "Level        %li\n",	obj->level		     );
    fprintf( fp, "Timer        %li\n",	obj->timer 	     );
    fprintf( fp, "Cost         %li\n",	obj->cost		     );
    fprintf( fp, "Values       %li %li %li %li\n",
	obj->value[0], obj->value[1], obj->value[2], obj->value[3]	     );

    switch ( obj->item_type )
    {
    case ITEM_POTION:
	if ( obj->value[1] > 0 )
	{
	    fprintf( fp, "Spell 1      '%s'\n", 
		skill_table[obj->value[1]].name );
	}

	if ( obj->value[2] > 0 )
	{
	    fprintf( fp, "Spell 2      '%s'\n", 
		skill_table[obj->value[2]].name );
	}

	if ( obj->value[3] > 0 )
	{
	    fprintf( fp, "Spell 3      '%s'\n", 
		skill_table[obj->value[3]].name );
	}

	break;

    case ITEM_SCROLL:
	if ( obj->value[1] > 0 )
	{
	    fprintf( fp, "Spell 1      '%s'\n", 
		skill_table[obj->value[1]].name );
	}

	if ( obj->value[2] > 0 )
	{
	    fprintf( fp, "Spell 2      '%s'\n", 
		skill_table[obj->value[2]].name );
	}

	if ( obj->value[3] > 0 )
	{
	    fprintf( fp, "Spell 3      '%s'\n", 
		skill_table[obj->value[3]].name );
	}

	break;

    case ITEM_PILL:
    case ITEM_STAFF:
    case ITEM_WAND:
	if ( obj->value[3] > 0 )
	{
	    fprintf( fp, "Spell 3      '%s'\n", 
		skill_table[obj->value[3]].name );
	}

	break;
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
/*
	if ( paf->type < 0 || paf->type >= MAX_SKILL ) continue;
*/
	fprintf( fp, "AffectData   %li %li %li\n",
	    paf->duration, paf->modifier, paf->location );
    }
/*
    if ( vnum >= 17 && ( vnum < 30003 || vnum > 30037 ) )
    {
	for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
	{
**
	    if ( paf->type < 0 || paf->type >= MAX_SKILL ) continue;
**
	    fprintf( fp, "AffectData   %li %li %li\n",
		paf->duration, paf->modifier, paf->location );
	}
    }
*/
    for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )
    {
	fprintf( fp, "ExtraDescr   %s~ %s~\n",
	    ed->keyword, ed->description );
    }
/*
    if ( vnum >= 17 && ( vnum < 30003 || vnum > 30037 ) )
    {
	for ( ed = obj->pIndexData->extra_descr; ed != NULL; ed = ed->next )
	{
	    fprintf( fp, "ExtraDescr   %s~ %s~\n",
		ed->keyword, ed->description );
	}
    }
*/
    fprintf( fp, "End\n\n" );

    if ( obj->contains != NULL )
	fwrite_obj( ch, obj->contains, fp, iNest + 1 );

    return;
}



/*
 * Load a char and inventory into a new ch structure.
 */
bool load_char_obj( DESCRIPTOR_DATA *d, char *name )
{
    static PC_DATA pcdata_zero;
    char strsave[MAX_STRING_LENGTH];
    char *strtime;
    CHAR_DATA *ch;
    FILE *fp;
    bool found;

    if ( char_free == NULL )
    {
	ch				= alloc_perm( sizeof(*ch) );
    }
    else
    {
	ch				= char_free;
	char_free			= char_free->next;
    }
    clear_char( ch );

    if ( pcdata_free == NULL )
    {
	ch->pcdata			= alloc_perm( sizeof(*ch->pcdata) );
    }
    else
    {
	ch->pcdata			= pcdata_free;
	pcdata_free			= pcdata_free->next;
    }
    *ch->pcdata				= pcdata_zero;

    d->character			= ch;
    ch->desc				= d;
    ch->name				= str_dup( name );
    ch->act				= PLR_BLANK
					| PLR_COMBINE
					| PLR_PROMPT;
    ch->extra				= 0;
    ch->more				= 0;
    ch->xtra				= 0;
    ch->pcdata->familiar		= NULL;
    ch->pcdata->propose			= NULL;
    ch->pcdata->chobj			= NULL;
    /* every characters starts at default board from login.. this board
       should be read_level == 0 !
    */
    ch->pcdata->board                   = &boards[DEFAULT_BOARD];
    ch->pcdata->pwd			= str_dup( "" );
    ch->pcdata->bamfin			= str_dup( "" );
    ch->pcdata->bamfout			= str_dup( "" );
    ch->pcdata->cdest			= str_dup( "" );
    ch->pcdata->title			= str_dup( "" );
    ch->pcdata->decap_char		= str_dup( "" );
    ch->pcdata->marriage		= str_dup( "" );
    ch->pcdata->email			= str_dup( "" );
    ch->pcdata->conding			= str_dup( "" );
    ch->pcdata->love			= str_dup( "" );
    ch->pcdata->covenant		= str_dup( "" );
    ch->pcdata->cov_time		= str_dup( "None" );
    ch->pcdata->leader			= str_dup( "" );
    ch->pcdata->vleader			= str_dup( "" );
    ch->lord				= str_dup( "" );
    ch->clan				= str_dup( "" );
    ch->morph				= str_dup( "" );
    ch->prompt				= str_dup( "" );
    ch->cprompt				= str_dup( "" );
    ch->contingency			= str_dup( "" );
    strtime = ctime( &current_time );
    strtime[strlen(strtime)-1] = '\0';
    free_string(ch->lasttime);
    ch->createtime			= str_dup( strtime );
    ch->lasttime			= str_dup( "" );
    ch->lasthost			= str_dup( "" );
    ch->stats[0]			= 1;
    ch->stats[1]			= 1;
    ch->stats[2]			= 1;
    ch->stats[3]			= 1;
    ch->stats[4]			= 1;
    ch->pcdata->denydate		= 0;
    ch->home				= 3001;
    ch->pcdata->homecv                          = 0;
    ch->vampgen				= 0;
    ch->vampaff				= 0;
    ch->itemaffect			= 0;
    ch->polyaff				= 0;
    ch->init				= 0;
    ch->beast				= 20;
    ch->truebeast			= 20;
    ch->stance[0]			= 0;
    ch->stance[1]			= 0;
    ch->stance[2]			= 0;
    ch->stance[3]			= 0;
    ch->stance[4]			= 0;
    ch->stance[5]			= 0;
    ch->stance[6]			= 0;
    ch->stance[7]			= 0;
    ch->stance[8]			= 0;
    ch->stance[9]			= 0;
    ch->stance[10]			= 0;
    ch->stance[11]			= 0;
    ch->style[0]			= 0;
    ch->style[1]			= 0;
    ch->style[2]			= 0;
    ch->style[3]			= 0;
    ch->style[4]			= 0;
    ch->style[5]			= 0;
    ch->style[6]			= 0;
    ch->weapon[0]			= 0;
    ch->weapon[1]			= 0;
    ch->weapon[2]			= 0;
    ch->weapon[3]			= 0;
    ch->weapon[4]			= 0;
    ch->weapon[5]			= 0;
    ch->pkill				= 0;
    ch->pdeath				= 0;
    ch->mkill				= 0;
    ch->mdeath				= 0;
    ch->embraced			= 0;
    ch->blood[0]			= 100;
    ch->blood[1]			= 100;
    ch->pcdata->quest			= 0;
    ch->pcdata->wolf			= 0;
    ch->pcdata->rank			= 0;
    ch->pcdata->regenerate		= 0;
    ch->pcdata->atm			= 0;
    ch->pcdata->necromancy		= 0;
    ch->pcdata->obeah			= 0;
    ch->pcdata->darkness		= 0;
    ch->pcdata->weather			= 0;
    ch->pcdata->silence			= 0;
    ch->pcdata->bodycrafts		= 0;
    ch->pcdata->celerity		= 0;
    ch->pcdata->followers		= 0;
    ch->pcdata->absorb[0]		= 0;
    ch->pcdata->absorb[1]		= 0;
    ch->pcdata->absorb[2]		= 0;
    ch->pcdata->score[0]		= 0;
    ch->pcdata->score[1]		= 0;
    ch->pcdata->score[2]		= 0;
    ch->pcdata->score[3]		= 0;
    ch->pcdata->score[4]		= 0;
    ch->pcdata->score[5]		= 0;
    ch->pcdata->dragcol			= 0;
    ch->pcdata->colors[0]		= 0;
    ch->pcdata->colors[1]		= 0;
    ch->pcdata->colors[2]		= 0;
    ch->pcdata->colors[3]		= 0;
    ch->pcdata->colors[4]		= 0;
    ch->pcdata->colors[5]		= 0;
    ch->pcdata->cland[0]		= 0;
    ch->pcdata->cland[1]		= 0;
    ch->pcdata->cland[2]		= 0;
    ch->pcdata->tgift[0]		= 0;
    ch->pcdata->tgift[1]                = 0;
    ch->pcdata->tgift[2]                = 0;
    ch->pcdata->diableries[0]		= 0;
    ch->pcdata->diableries[1]		= 0;
    ch->pcdata->breed[0]		= 0;
    ch->pcdata->breed[1]		= 0;
    ch->pcdata->breed[2]		= 0;
    ch->pcdata->auspice[0]		= 0;
    ch->pcdata->auspice[1]		= 0;
    ch->pcdata->auspice[2]		= 0;
    ch->pcdata->auspice[3]		= 0;
    ch->pcdata->auspice[4]		= 0;
    ch->pcdata->tribes[0]		= 0;
    ch->pcdata->tribes[1]		= 0;
    ch->pcdata->tribes[2]		= 0;
    ch->pcdata->tribes[3]		= 0;
    ch->pcdata->tribes[4]		= 0;
    ch->pcdata->tribes[5]		= 0;
    ch->pcdata->tribes[6]		= 0;
    ch->pcdata->tribes[7]		= 0;
    ch->pcdata->tribes[8]		= 0;
    ch->pcdata->tribes[9]		= 0;
    ch->pcdata->tribes[10]		= 0;
    ch->pcdata->tribes[11]		= 0;
    ch->pcdata->tribes[12]		= 0;
    ch->pcdata->tribes[13]		= 0;
    ch->pcdata->tribes[14]		= 0;
    ch->pcdata->tribes[15]		= 0;
    ch->pcdata->tribes[16]		= 0;
    ch->pcdata->tribes[17]		= 0;
    ch->pcdata->tribes[18]		= 0;
    ch->pcdata->tribes[19]		= 0;
    ch->pcdata->powers[0]		= 0;
    ch->pcdata->powers[1]		= 0;
    ch->pcdata->powers[2]		= 0;
    ch->pcdata->powers[3]		= 0;
    ch->pcdata->powers[4]		= 0;
    ch->pcdata->powers[5]		= 0;
    ch->pcdata->powers[6]		= 0;
    ch->pcdata->powers[7]		= 0;
    ch->pcdata->powers[8]		= 0;
    ch->pcdata->powers[9]		= 0;
    ch->pcdata->powers[10]		= 0;
    ch->pcdata->powers[11]		= 0;
    ch->pcdata->powers[12]		= 0;
    ch->pcdata->powers[13]		= 0;
    ch->pcdata->powers[14]		= 0;
    ch->pcdata->powers[15]		= 0;
    ch->pcdata->powers[16]		= 0;
    ch->pcdata->powers[17]		= 0;
    ch->pcdata->powers[18]		= 0;
    ch->pcdata->powers[19]		= 0;
    ch->pcdata->willpower[0]		= 50;
    ch->pcdata->willpower[1]		= 50;
    ch->pcdata->dragskills		= 0;
    ch->pcdata->drow[0]			= 0;
    ch->pcdata->drow[1]			= 0;
    ch->pcdata->highland[0]		= 0;
    ch->pcdata->highland[1]		= 0;
    ch->pcdata->highland[2]		= 0;
    ch->pcdata->highland[3]		= 0;
    ch->pcdata->monk[0]			= 0;
    ch->pcdata->monk[1]			= 0;
    ch->pcdata->monk[2]			= 0;
    ch->pcdata->monk[3]			= 0;
    ch->pcdata->ninja[0]		= 0;
    ch->pcdata->ninja[1]		= 0;
    ch->pcdata->ninja[2]		= 0;
    ch->pcdata->ninja[3]		= 0;
    ch->pcdata->ninja[4]		= 0;
    ch->fight_timer			= 0;
    ch->polymorph_timer			= 0;
    ch->better_body_timer		= 0;
    ch->pcdata->resist[0]		= 0;
    ch->pcdata->resist[1]		= 0;
    ch->pcdata->resist[2]		= 0;
    ch->pcdata->resist[3]		= 0;
    ch->pcdata->resist[4]		= 0;
    ch->pcdata->classpoints[0]		= 0;
    ch->pcdata->classpoints[1]		= 0;
    ch->pcdata->fake_skill		= 0;
    ch->pcdata->fake_stance		= 0;
    ch->pcdata->fake_hit		= 0;
    ch->pcdata->fake_dam		= 0;
    ch->pcdata->fake_ac			= 0;
    ch->pcdata->fake_hp			= 0;
    ch->pcdata->fake_mana		= 0;
    ch->pcdata->fake_max_hp		= 0;
    ch->pcdata->fake_max_mana		= 0;
    ch->pcdata->obj_vnum		= 0;
    ch->pcdata->majesty			= 0;
    ch->pcdata->flag			= 0;
    found = FALSE;
    fflush( fpReserve );
    fclose( fpReserve );
    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( name ) );
    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
    {
	long iNest;

	for ( iNest = 0; iNest < MAX_NEST; iNest++ )
	    rgObjNest[iNest] = NULL;

	found = TRUE;
	for ( ; ; )
	{
	    char letter;
	    char *word;

	    letter = fread_letter( fp );
	    if ( letter == '*' )
	    {
		fread_to_eol( fp );
		continue;
	    }

	    if ( letter != '#' )
	    {
		bug( "Load_char_obj: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );
	    if      ( !str_cmp( word, "PLAYER" ) ) fread_char( ch, fp );
	    else if ( !str_cmp( word, "OBJECT" ) ) fread_obj( ch, fp );
	    else if ( !str_cmp( word, "O"      ) ) fread_tiny_obj( ch, fp );
	    else if ( !str_cmp( word, "END"    ) ) break;
	    else
	    {
		bug( "Load_char_obj: bad section.", 0 );
		break;
	    }
	}
	fflush( fp );
	fclose( fp );
    }

    fpReserve = fopen( NULL_FILE, "r" );
    return found;
}



bool load_char_short( DESCRIPTOR_DATA *d, char *name )
{
    static PC_DATA pcdata_zero;
    char strsave[MAX_STRING_LENGTH];
    CHAR_DATA *ch;
    char *strtime;
    FILE *fp;
    bool found;

    if ( char_free == NULL )
    {
	ch				= alloc_perm( sizeof(*ch) );
    }
    else
    {
	ch				= char_free;
	char_free			= char_free->next;
    }
    clear_char( ch );

    if ( pcdata_free == NULL )
    {
	ch->pcdata			= alloc_perm( sizeof(*ch->pcdata) );
    }
    else
    {
	ch->pcdata			= pcdata_free;
	pcdata_free			= pcdata_free->next;
    }
    *ch->pcdata				= pcdata_zero;

    d->character			= ch;
    ch->desc				= d;
    ch->name				= str_dup( name );
    ch->act				= PLR_BLANK
					| PLR_COMBINE
					| PLR_PROMPT;
    ch->extra				= 0;
    ch->more				= 0;
    ch->xtra				= 0;
    ch->pcdata->familiar		= NULL;
    ch->pcdata->propose			= NULL;
    ch->pcdata->chobj			= NULL;
    ch->pcdata->pwd			= str_dup( "" );
    ch->pcdata->bamfin			= str_dup( "" );
    ch->pcdata->bamfout			= str_dup( "" );
    ch->pcdata->cdest			= str_dup( "" );
    ch->pcdata->title			= str_dup( "" );
    ch->pcdata->decap_char		= str_dup( "" );
    ch->pcdata->marriage		= str_dup( "" );
    ch->pcdata->email			= str_dup( "" );
    ch->pcdata->conding			= str_dup( "" );
    ch->pcdata->love			= str_dup( "" );
    ch->pcdata->covenant		= str_dup( "" );
    ch->pcdata->cov_time		= str_dup( "None" );
    ch->pcdata->leader			= str_dup( "" );
    ch->pcdata->vleader			= str_dup( "" );
    ch->lord				= str_dup( "" );
    ch->clan				= str_dup( "" );
    ch->morph				= str_dup( "" );
    ch->prompt				= str_dup( "" );
    ch->cprompt				= str_dup( "" );
    ch->contingency			= str_dup( "" );
    strtime = ctime( &current_time );
    strtime[strlen(strtime)-1] = '\0';
    free_string(ch->lasttime);
    ch->createtime			= str_dup( strtime );
    ch->lasttime			= str_dup( "" );
    ch->lasthost			= str_dup( "" );
    ch->stats[0]			= 1;
    ch->stats[1]			= 1;
    ch->stats[2]			= 1;
    ch->stats[3]			= 1;
    ch->stats[4]			= 1;
    ch->pcdata->denydate		= 0;
    ch->home				= 3001;
    ch->pcdata->homecv                          = 0;
    ch->vampgen				= 0;
    ch->vampaff				= 0;
    ch->itemaffect			= 0;
    ch->polyaff				= 0;
    ch->init				= 0;
    ch->beast				= 20;
    ch->truebeast			= 20;
    ch->stance[0]			= 0;
    ch->stance[1]			= 0;
    ch->stance[2]			= 0;
    ch->stance[3]			= 0;
    ch->stance[4]			= 0;
    ch->stance[5]			= 0;
    ch->stance[6]			= 0;
    ch->stance[7]			= 0;
    ch->stance[8]			= 0;
    ch->stance[9]			= 0;
    ch->stance[10]			= 0;
    ch->stance[11]			= 0;
    ch->style[0]			= 0;
    ch->style[1]			= 0;
    ch->style[2]			= 0;
    ch->style[3]			= 0;
    ch->style[4]			= 0;
    ch->style[5]			= 0;
    ch->style[6]			= 0;
    ch->weapon[0]			= 0;
    ch->weapon[1]			= 0;
    ch->weapon[2]			= 0;
    ch->weapon[3]			= 0;
    ch->weapon[4]			= 0;
    ch->weapon[5]			= 0;
    ch->pkill				= 0;
    ch->pdeath				= 0;
    ch->mkill				= 0;
    ch->mdeath				= 0;
    ch->embraced			= 0;
    ch->blood[0]			= 100;
    ch->blood[1]			= 100;
    ch->pcdata->quest			= 0;
    ch->pcdata->wolf			= 0;
    ch->pcdata->rank			= 0;
    ch->pcdata->regenerate		= 0;
    ch->pcdata->atm			= 0;
    ch->pcdata->necromancy		= 0;
    ch->pcdata->obeah			= 0;
    ch->pcdata->darkness		= 0;
    ch->pcdata->weather			= 0;
    ch->pcdata->silence			= 0;
    ch->pcdata->bodycrafts		= 0;
    ch->pcdata->celerity		= 0;
    ch->pcdata->followers		= 0;
    ch->pcdata->absorb[0]		= 0;
    ch->pcdata->absorb[1]		= 0;
    ch->pcdata->absorb[2]		= 0;
    ch->pcdata->score[0]		= 0;
    ch->pcdata->score[1]		= 0;
    ch->pcdata->score[2]		= 0;
    ch->pcdata->score[3]		= 0;
    ch->pcdata->score[4]		= 0;
    ch->pcdata->score[5]		= 0;
    ch->pcdata->dragcol			= 0;
    ch->pcdata->colors[0]		= 0;
    ch->pcdata->colors[1]		= 0;
    ch->pcdata->colors[2]		= 0;
    ch->pcdata->colors[3]		= 0;
    ch->pcdata->colors[4]		= 0;
    ch->pcdata->colors[5]		= 0;
    ch->pcdata->cland[0]		= 0;
    ch->pcdata->cland[1]		= 0;
    ch->pcdata->cland[2]		= 0;
    ch->pcdata->tgift[0]                = 0;
    ch->pcdata->tgift[1]                = 0;
    ch->pcdata->tgift[2]                = 0;
    ch->pcdata->diableries[0]		= 0;
    ch->pcdata->diableries[1]		= 0;
    ch->pcdata->breed[0]		= 0;
    ch->pcdata->breed[1]		= 0;
    ch->pcdata->breed[2]		= 0;
    ch->pcdata->auspice[0]		= 0;
    ch->pcdata->auspice[1]		= 0;
    ch->pcdata->auspice[2]		= 0;
    ch->pcdata->auspice[3]		= 0;
    ch->pcdata->auspice[4]		= 0;
    ch->pcdata->tribes[0]		= 0;
    ch->pcdata->tribes[1]		= 0;
    ch->pcdata->tribes[2]		= 0;
    ch->pcdata->tribes[3]		= 0;
    ch->pcdata->tribes[4]		= 0;
    ch->pcdata->tribes[5]		= 0;
    ch->pcdata->tribes[6]		= 0;
    ch->pcdata->tribes[7]		= 0;
    ch->pcdata->tribes[8]		= 0;
    ch->pcdata->tribes[9]		= 0;
    ch->pcdata->tribes[10]		= 0;
    ch->pcdata->tribes[11]		= 0;
    ch->pcdata->tribes[12]		= 0;
    ch->pcdata->tribes[13]		= 0;
    ch->pcdata->tribes[14]		= 0;
    ch->pcdata->tribes[15]		= 0;
    ch->pcdata->tribes[16]		= 0;
    ch->pcdata->tribes[17]		= 0;
    ch->pcdata->tribes[18]		= 0;
    ch->pcdata->tribes[19]		= 0;
    ch->pcdata->powers[0]		= 0;
    ch->pcdata->powers[1]		= 0;
    ch->pcdata->powers[2]		= 0;
    ch->pcdata->powers[3]		= 0;
    ch->pcdata->powers[4]		= 0;
    ch->pcdata->powers[5]		= 0;
    ch->pcdata->powers[6]		= 0;
    ch->pcdata->powers[7]		= 0;
    ch->pcdata->powers[8]		= 0;
    ch->pcdata->powers[9]		= 0;
    ch->pcdata->powers[10]		= 0;
    ch->pcdata->powers[11]		= 0;
    ch->pcdata->powers[12]		= 0;
    ch->pcdata->powers[13]		= 0;
    ch->pcdata->powers[14]		= 0;
    ch->pcdata->powers[15]		= 0;
    ch->pcdata->powers[16]		= 0;
    ch->pcdata->powers[17]		= 0;
    ch->pcdata->powers[18]		= 0;
    ch->pcdata->powers[19]		= 0;
    ch->pcdata->willpower[0]		= 50;
    ch->pcdata->willpower[1]		= 50;
    ch->pcdata->dragskills              = 0;
    ch->pcdata->drow[0]			= 0;
    ch->pcdata->drow[1]			= 0;
    ch->pcdata->highland[0]		= 0;
    ch->pcdata->highland[1]		= 0;
    ch->pcdata->highland[2]		= 0;
    ch->pcdata->highland[3]		= 0;
    ch->pcdata->monk[0]			= 0;
    ch->pcdata->monk[1]			= 0;
    ch->pcdata->monk[2]			= 0;
    ch->pcdata->monk[3]			= 0;
    ch->pcdata->ninja[0]		= 0;
    ch->pcdata->ninja[1]		= 0;
    ch->pcdata->ninja[2]		= 0;
    ch->pcdata->ninja[3]		= 0;
    ch->pcdata->ninja[4]		= 0;
    ch->fight_timer			= 0;
    ch->polymorph_timer			= 0;
    ch->better_body_timer		= 0;
    ch->pcdata->resist[0]		= 0;
    ch->pcdata->resist[1]		= 0;
    ch->pcdata->resist[2]		= 0;
    ch->pcdata->resist[3]		= 0;
    ch->pcdata->resist[4]		= 0;
    ch->pcdata->classpoints[0]		= 0;
    ch->pcdata->classpoints[1]		= 0;
    ch->pcdata->fake_skill		= 0;
    ch->pcdata->fake_stance		= 0;
    ch->pcdata->fake_hit		= 0;
    ch->pcdata->fake_dam		= 0;
    ch->pcdata->fake_ac			= 0;
    ch->pcdata->fake_hp			= 0;
    ch->pcdata->fake_mana		= 0;
    ch->pcdata->fake_max_hp		= 0;
    ch->pcdata->fake_max_mana		= 0;
    ch->pcdata->obj_vnum		= 0;
    ch->pcdata->majesty			= 0;
    ch->pcdata->flag			= 0;

    found = FALSE;
    fflush( fpReserve );
    fclose( fpReserve );
    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( name ) );
    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
    {
	long iNest;

	for ( iNest = 0; iNest < MAX_NEST; iNest++ )
	    rgObjNest[iNest] = NULL;

	found = TRUE;
	for ( ; ; )
	{
	    char letter;
	    char *word;

	    letter = fread_letter( fp );
	    if ( letter == '*' )
	    {
		fread_to_eol( fp );
		continue;
	    }

	    if ( letter != '#' )
	    {
		bug( "Load_char_obj: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );
	    if      ( !str_cmp( word, "PLAYER" ) ) fread_char ( ch, fp );
	    else if ( !str_cmp( word, "OBJECT" ) ) break;
	    else if ( !str_cmp( word, "O"      ) ) break;
	    else if ( !str_cmp( word, "END"    ) ) break;
	    else
	    {
		bug( "Load_char_obj: bad section.", 0 );
		break;
	    }
	}
	fflush( fp );
	fclose( fp );
    }

    fpReserve = fopen( NULL_FILE, "r" );
    return found;
}



/*
 * Read in a char.
 */

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}

void fread_char( CHAR_DATA *ch, FILE *fp )
{
    char buf[MSL];
    char *word;
    bool fMatch;

    for ( ; ; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
	    if ( !str_cmp( word, "Absorb" ) )
	    {
		ch->pcdata->absorb[0] = fread_number( fp );
		ch->pcdata->absorb[1] = fread_number( fp );
		ch->pcdata->absorb[2] = fread_number( fp );
		fMatch = TRUE;
		break;
	    }

	    KEY( "Act",		ch->act,		fread_number( fp ) );
	    KEY( "AffectedBy",	ch->affected_by,	fread_number( fp ) );
	    KEY( "Armor",	ch->armor,		fread_number( fp ) );

	    if ( !str_cmp( word, "Affect" ) || !str_cmp( word, "AffectData" ) )
	    {
		AFFECT_DATA *paf;

		if ( affect_free == NULL )
		{
		    paf		= alloc_perm( sizeof(*paf) );
		}
		else
		{
		    paf		= affect_free;
		    affect_free	= affect_free->next;
		}

		if ( !str_cmp( word, "Affect" ) )
		{
		    /* Obsolete 2.0 form. */
		    paf->type	= fread_number( fp );
		}
		else
		{
		    long sn;

		    sn = skill_lookup( fread_word( fp ) );
		    if ( sn < 0 )
			bug( "Fread_char: unknown skill.", 0 );
		    else
			paf->type = sn;
		}

		paf->duration	= fread_number( fp );
		paf->modifier	= fread_number( fp );
		paf->location	= fread_number( fp );
		paf->bitvector	= fread_number( fp );
		paf->next	= ch->affected;
		ch->affected	= paf;
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Auspice" ) )
	    {
		ch->pcdata->auspice[0] = fread_number( fp );
		ch->pcdata->auspice[1] = fread_number( fp );
		ch->pcdata->auspice[2] = fread_number( fp );
		ch->pcdata->auspice[3] = fread_number( fp );
		ch->pcdata->auspice[4] = fread_number( fp );
		fMatch = TRUE;
		break;
	    }
	
	    if ( !str_cmp( word, "Arete" ) )
	    {
		ch->arete = fread_number( fp );
		fMatch = TRUE;
		break;
	    }

	    KEY( "Atm",     	ch->pcdata->atm,	fread_number( fp ) );
	    break;

	case 'B':
	    KEY( "Bamfin",	ch->pcdata->bamfin,	fread_string( fp ) );
	    KEY( "Bamfout",	ch->pcdata->bamfout,	fread_string( fp ) );
	    KEY( "Beast",       ch->beast,		fread_number( fp ) );
	    KEY( "Better_timer", ch->better_body_timer, fread_number( fp ) );
	    if ( !str_cmp( word, "Blood" ) )
	    {
		ch->blood[0] = fread_number( fp );
		ch->blood[1] = fread_number( fp );
		fMatch = TRUE;
		break;
	    }
	    /* Read in board status */
	    if (!str_cmp(word, "Boards"))
	    {
		long i, num = fread_number(fp); /* number of boards saved */
		char *boardname;

		for ( ; num; num--) /* for each of the board saved */
		{
		    boardname = fread_word(fp);
		    i = board_lookup(boardname); /* find board number */
 
		    if (i == BOARD_NOTFOUND) /* Does board still exist ? */
		    {
			sprintf(buf, "fread_char: %s had unknown board name: %s. Skipped.", ch->name, boardname);
			log_string(buf);
			fread_number(fp); /* read last_note and skip info */
		    }
		    else /* Save it */
			ch->pcdata->last_note[i] = fread_number(fp);
		}        /* for */
		fMatch = TRUE;
            } /* Boards */
	    KEY( "Bodycrafts",	ch->pcdata->bodycrafts,	fread_number( fp ) );
	    if ( !str_cmp( word, "Breed" ) )
	    {
		ch->pcdata->breed[0] = fread_number( fp );
		ch->pcdata->breed[1] = fread_number( fp );
		ch->pcdata->breed[2] = fread_number( fp );
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'C':
	    KEY( "Clan",	ch->clan,		fread_string( fp ) );
	    KEY( "Cdest",	ch->pcdata->cdest,	fread_string( fp ) );
	    KEY( "Contingency", ch->contingency,	fread_string( fp ) );

	    if ( !str_cmp( word, "Clandisc" ) )
	    {
		ch->pcdata->cland[0] = fread_number( fp );
		ch->pcdata->cland[1] = fread_number( fp );
		ch->pcdata->cland[2] = fread_number( fp );
		fMatch = TRUE;
		break;
	    }
	    KEY( "Class",	ch->class,		fread_number( fp ) );
	    if (!str_cmp(word, "Classpoints"))
	    {
		ch->pcdata->classpoints[0] = fread_number(fp);
		ch->pcdata->classpoints[1] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Colors" ) )
	    {
		ch->pcdata->colors[0] = fread_number( fp );
		ch->pcdata->colors[1] = fread_number( fp );
		ch->pcdata->colors[2] = fread_number( fp );
		ch->pcdata->colors[3] = fread_number( fp );
		ch->pcdata->colors[4] = fread_number( fp );
		ch->pcdata->colors[5] = fread_number( fp );
		fMatch = TRUE;
		break;
	    }

	    KEY( "Conding",	ch->pcdata->conding,	fread_string( fp ) );
	    KEY( "Covenant",	ch->pcdata->covenant,	fread_string( fp ) );
	    KEY( "CovTime",	ch->pcdata->cov_time,	fread_string( fp ) );
	    KEY( "Cprompt",	ch->cprompt,		fread_string( fp ) );
	    KEY( "Createtime",	ch->createtime,		fread_string( fp ) );
	    break;

	case 'D':
	    KEY( "Damroll",	ch->damroll,		fread_number( fp ) );
	    KEY( "Deaf",	ch->deaf,		fread_number( fp ) );
	    if ( !str_cmp( word, "Decap_char" ) )
	    {
		ch->pcdata->decap_char = fread_string( fp );
		if ( isalpha(ch->pcdata->decap_char[0])
		||   isdigit(ch->pcdata->decap_char[0]) )
		{
		    sprintf( buf, " %s", ch->pcdata->decap_char );
		    free_string( ch->pcdata->decap_char );
		    ch->pcdata->title = str_dup( buf );
		}
		fMatch = TRUE;
		break;
	    }

	    KEY( "DenyDate",	ch->pcdata->denydate,	fread_number( fp ) );
	    KEY( "Description",	ch->description,	fread_string( fp ) );

	    if ( !str_cmp( word, "Diableries" ) )
	    {
		ch->pcdata->diableries[0] = fread_number( fp );
		ch->pcdata->diableries[1] = fread_number( fp );
		fMatch = TRUE;
		break;
	    }

	    if (!str_cmp(word, "Drowa"))
	    {
		ch->pcdata->drow[0] = fread_number(fp);
		ch->pcdata->drow[1] = fread_number(fp);
		fMatch = TRUE;
	    }
	    KEY( "Drow",	ch->pcdata->drow[0],	fread_number( fp ) );
	    KEY( "Dragcol",	ch->pcdata->dragcol,	fread_number( fp ) );
            KEY( "DSkills",     ch->pcdata->dragskills, fread_number( fp ) );
	    break;

	case 'E':
	    KEY( "Email",	ch->pcdata->email,	fread_string( fp ) );
	    if ( !str_cmp( word, "End" ) )
		return;
	    KEY( "Exp",		ch->exp,		fread_number( fp ) );
	    KEY( "Extra",	ch->extra,		fread_number( fp ) );
	    break;

	case 'F':
	    if ( !str_cmp( word, "Fake" ) )
	    {
		ch->pcdata->fake_skill	= fread_number( fp );
		ch->pcdata->fake_stance	= fread_number( fp );
		ch->pcdata->fake_hit	= fread_number( fp );
		ch->pcdata->fake_dam	= fread_number( fp );
		ch->pcdata->fake_ac	= fread_number( fp );
		ch->pcdata->fake_hp	= fread_number( fp );
		ch->pcdata->fake_mana	= fread_number( fp );
		ch->pcdata->fake_max_hp		= fread_number( fp );
		ch->pcdata->fake_max_mana	= fread_number( fp );
		fMatch = TRUE;
		break;
	    }

	    KEY( "Fight_timer", ch->fight_timer,	fread_number( fp ) );
	    KEY( "Flag",   	ch->pcdata->flag,	fread_number( fp ) );
	    break;

	case 'G':
	    KEY( "Gold",	ch->gold,		fread_number( fp ) );
	    break;

	case 'H':
	    if (!str_cmp(word, "Highland"))
	    {
		ch->pcdata->highland[0] = fread_number(fp);
		ch->pcdata->highland[1] = fread_number(fp);
		ch->pcdata->highland[2] = fread_number(fp);
		ch->pcdata->highland[3] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }

	    KEY( "Hitroll",	ch->hitroll,		fread_number( fp ) );
            KEY( "Home",        ch->home,               fread_number( fp ) );
	    KEY( "HomeCV",        ch->pcdata->homecv,		fread_number( fp ) );

	    if ( !str_cmp( word, "HpMana" ) )
	    {
		ch->hit		= fread_number( fp );
		ch->max_hit	= fread_number( fp );
		ch->mana	= fread_number( fp );
		ch->max_mana	= fread_number( fp );
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'I':
	    KEY( "Itemaffect",    ch->itemaffect,	fread_number( fp ) );
	    break;

	case 'L':
	    KEY( "Lasthost",	ch->lasthost,		fread_string( fp ) );
	    KEY( "Lasttime",	ch->lasttime,		fread_string( fp ) );
	    KEY( "Leader",	ch->pcdata->leader,	fread_string( fp ) );
	    KEY( "Level",	ch->level,		fread_number( fp ) );
	    KEY( "LongDescr",	ch->long_descr,		fread_string( fp ) );
	    KEY( "Lord",	ch->lord,		fread_string( fp ) );
	    KEY( "Love",	ch->pcdata->love,	fread_string( fp ) );
	    break;

	case 'M':
	    KEY( "Majesty",     ch->pcdata->majesty,	fread_number( fp ) );
	    KEY( "Marriage",	ch->pcdata->marriage,	fread_string( fp ) );
	    KEY( "More",	ch->more,		fread_number( fp ) );
	    KEY( "Morph",	ch->morph,		fread_string( fp ) );

	    if (!str_cmp(word, "Monk"))
	    {
		ch->pcdata->monk[0] = fread_number(fp);
		ch->pcdata->monk[1] = fread_number(fp);
		ch->pcdata->monk[2] = fread_number(fp);
		ch->pcdata->monk[3] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'N':
	    if ( !str_cmp( word, "Name" ) )
	    {
		fread_to_eol( fp );
		fMatch = TRUE;
		break;
	    }
	    if (!str_cmp(word, "Ninja"))
	    {
		ch->pcdata->ninja[0] = fread_number(fp);
		ch->pcdata->ninja[1] = fread_number(fp);
		ch->pcdata->ninja[2] = fread_number(fp);
		ch->pcdata->ninja[3] = fread_number(fp);
		ch->pcdata->ninja[4] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'O':
	    KEY( "Objvnum",     ch->pcdata->obj_vnum,	fread_number( fp ) );
	    break;

	case 'P':
	    KEY( "Paradox",	ch->paradox,		fread_number( fp ) );
	    KEY( "Password",	ch->pcdata->pwd,	fread_string( fp ) );
	    KEY( "Played",	ch->played,		fread_number( fp ) );
	    KEY( "Polyaff",     ch->polyaff,		fread_number( fp ) );
	    KEY( "Polytimer",   ch->polymorph_timer,    fread_number( fp ) );

	    if ( !str_cmp( word, "Powers" ) )
	    {
		ch->pcdata->powers[0] = fread_number( fp );
		ch->pcdata->powers[1] = fread_number( fp );
		ch->pcdata->powers[2] = fread_number( fp );
		ch->pcdata->powers[3] = fread_number( fp );
		ch->pcdata->powers[4] = fread_number( fp );
		ch->pcdata->powers[5] = fread_number( fp );
		ch->pcdata->powers[6] = fread_number( fp );
		ch->pcdata->powers[7] = fread_number( fp );
		ch->pcdata->powers[8] = fread_number( fp );
		ch->pcdata->powers[9] = fread_number( fp );
		ch->pcdata->powers[10] = fread_number( fp );
		ch->pcdata->powers[11] = fread_number( fp );
		ch->pcdata->powers[12] = fread_number( fp );
		ch->pcdata->powers[13] = fread_number( fp );
		ch->pcdata->powers[14] = fread_number( fp );
		ch->pcdata->powers[15] = fread_number( fp );
		ch->pcdata->powers[16] = fread_number( fp );
		ch->pcdata->powers[17] = fread_number( fp );
		ch->pcdata->powers[18] = fread_number( fp );
		ch->pcdata->powers[19] = fread_number( fp );
		fMatch = TRUE;
		break;
	    }

	    KEY( "Position",	ch->position,		fread_number( fp ) );
	    KEY( "Primal",	ch->primal,		fread_number( fp ) );
	    if ( !str_cmp( word, "PkPdMkMd" ) )
	    {
		ch->pkill	= fread_number( fp );
		ch->pdeath	= fread_number( fp );
		ch->mkill	= fread_number( fp );
		ch->mdeath	= fread_number( fp );
		fMatch = TRUE;
		break;
	    }
	    KEY( "Prompt",	ch->prompt,		fread_string( fp ) );
	    break;

	case 'Q':
            if ( !str_cmp( word, "Quint" ) )
            {
                ch->quint[0]    = fread_number( fp );
                ch->quint[1]    = fread_number( fp );
                ch->quint[2]    = fread_number( fp );
                fMatch = TRUE;
                break;
            }

	    KEY( "Quest",        ch->pcdata->quest,	fread_number( fp ) );
	    break;

	case 'R':
	    KEY( "Rank",        ch->pcdata->rank,	fread_number( fp ) );

	    KEY( "Regenerate",	ch->pcdata->regenerate,	fread_number( fp ) );

	    if ( !str_cmp( word, "Resist" ) )
	    {
		ch->pcdata->resist[0] = fread_number( fp );
		ch->pcdata->resist[1] = fread_number( fp );
		ch->pcdata->resist[2] = fread_number( fp );
		ch->pcdata->resist[3] = fread_number( fp );
		ch->pcdata->resist[4] = fread_number( fp );
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Room" ) )
	    {
		ch->in_room = get_room_index( fread_number( fp ) );
		if ( ch->in_room == NULL )
		    ch->in_room = get_room_index( ROOM_VNUM_LIMBO );
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'S':
	    KEY( "SavingThrow",	ch->saving_throw,	fread_number( fp ) );

	    if ( !str_cmp( word, "Score" ) )
	    {
		ch->pcdata->score[0] = fread_number( fp );
		ch->pcdata->score[1] = fread_number( fp );
		ch->pcdata->score[2] = fread_number( fp );
		ch->pcdata->score[3] = fread_number( fp );
		ch->pcdata->score[4] = fread_number( fp );
		ch->pcdata->score[5] = fread_number( fp );
		fMatch = TRUE;
		break;
	    }

            if ( !str_cmp( word, "Spheres" ) )
            {
                ch->spheres[0] = fread_number( fp );
                ch->spheres[1] = fread_number( fp );
                ch->spheres[2] = fread_number( fp );
                ch->spheres[3] = fread_number( fp );
                ch->spheres[4] = fread_number( fp );
                ch->spheres[5] = fread_number( fp );
                ch->spheres[6] = fread_number( fp );
                ch->spheres[7] = fread_number( fp );
                ch->spheres[8] = fread_number( fp );
                fMatch = TRUE;
                break;
            }

	    KEY( "Sex",		ch->sex,		fread_number( fp ) );
	    KEY( "ShortDescr",	ch->short_descr,	fread_string( fp ) );

	    if ( !str_cmp( word, "Stance" ) )
	    {
		ch->stance[0]	= fread_number( fp );
		ch->stance[1]	= fread_number( fp );
		ch->stance[2]	= fread_number( fp );
		ch->stance[3]	= fread_number( fp );
		ch->stance[4]	= fread_number( fp );
		ch->stance[5]	= fread_number( fp );
		ch->stance[6]	= fread_number( fp );
		ch->stance[7]	= fread_number( fp );
		ch->stance[8]	= fread_number( fp );
		ch->stance[9]	= fread_number( fp );
		ch->stance[10]	= fread_number( fp );
		ch->stance[11]	= fread_number( fp );
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Style" ) )
	    {
		ch->style[0]	= fread_number( fp );
		ch->style[1]	= fread_number( fp );
		ch->style[2]	= fread_number( fp );
		ch->style[3]	= fread_number( fp );
		ch->style[4]	= fread_number( fp );
		ch->style[5]	= fread_number( fp );
		ch->style[6]	= fread_number( fp );
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Stats" ) )
	    {
		ch->stats[0] = fread_number( fp );
		ch->stats[1] = fread_number( fp );
		ch->stats[2] = fread_number( fp );
		ch->stats[3] = fread_number( fp );
		ch->stats[4] = fread_number( fp );
		fMatch = TRUE;
		break;
	    }

	    KEY( "Status",	ch->status,		fread_number( fp ) );

	    break;

	case 'T':

	    if ( !str_cmp( word, "Tribegift" ) )
            {
                ch->pcdata->tgift[0] = fread_number( fp );
                ch->pcdata->tgift[1] = fread_number( fp );
                ch->pcdata->tgift[2] = fread_number( fp );
                fMatch = TRUE;
                break;
            }

	    if ( !str_cmp( word, "Tribes" ) )
	    {
		ch->pcdata->tribes[0] = fread_number( fp );
		ch->pcdata->tribes[1] = fread_number( fp );
		ch->pcdata->tribes[2] = fread_number( fp );
		ch->pcdata->tribes[3] = fread_number( fp );
		ch->pcdata->tribes[4] = fread_number( fp );
		ch->pcdata->tribes[5] = fread_number( fp );
		ch->pcdata->tribes[6] = fread_number( fp );
		ch->pcdata->tribes[7] = fread_number( fp );
		ch->pcdata->tribes[8] = fread_number( fp );
		ch->pcdata->tribes[9] = fread_number( fp );
		ch->pcdata->tribes[10] = fread_number( fp );
		ch->pcdata->tribes[11] = fread_number( fp );
		ch->pcdata->tribes[12] = fread_number( fp );
		ch->pcdata->tribes[13] = fread_number( fp );
		ch->pcdata->tribes[14] = fread_number( fp );
		ch->pcdata->tribes[15] = fread_number( fp );
		ch->pcdata->tribes[16] = fread_number( fp );
		ch->pcdata->tribes[17] = fread_number( fp );
		ch->pcdata->tribes[18] = fread_number( fp );
		ch->pcdata->tribes[19] = fread_number( fp );
		fMatch = TRUE;
		break;
	    }

	    KEY( "Truebeast",	ch->truebeast,		fread_number( fp ) );

	    if ( !str_cmp( word, "Title" ) )
	    {
		ch->pcdata->title = fread_string( fp );
		if ( isalpha(ch->pcdata->title[0])
		||   isdigit(ch->pcdata->title[0]) )
		{
		    sprintf( buf, " %s", ch->pcdata->title );
		    free_string( ch->pcdata->title );
		    ch->pcdata->title = str_dup( buf );
		}
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'V':
	    KEY( "VLeader",	   ch->pcdata->vleader,	fread_string( fp ) );
	    KEY( "Vampaff",        ch->vampaff,		fread_number( fp ) );
	    KEY( "Vampgen",        ch->vampgen,		fread_number( fp ) );
	    if ( !str_cmp( word, "Vnum" ) )
	    {
		ch->pIndexData = get_mob_index( fread_number( fp ) );
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'W':
	    KEY( "Ward",        ch->paradox_ward,         fread_number( fp ) );
	    if ( !str_cmp( word, "Weapon" ) )
	    {
		ch->weapon[0]	= fread_number( fp );
		ch->weapon[1]	= fread_number( fp );
		ch->weapon[2]	= fread_number( fp );
		ch->weapon[3]	= fread_number( fp );
		ch->weapon[4]	= fread_number( fp );
		ch->weapon[5]	= fread_number( fp );
		fMatch = TRUE;
		break;
	    }
	    if ( !str_cmp( word, "Willpower" ) )
	    {
		ch->pcdata->willpower[0] = fread_number( fp );
		ch->pcdata->willpower[1] = fread_number( fp );
		fMatch = TRUE;
		break;
	    }
	    KEY( "Wolf",        ch->pcdata->wolf,	fread_number( fp ) );
	    break;

	case 'X':
	    KEY( "Xtra",        ch->xtra,               fread_number( fp ) );
	    break;
	}

	if ( !fMatch )
	{
	    sprintf(buf, "Fread_char: no match.  %s.", word);
	    log_string(buf);
	    fread_to_eol( fp );
	}
    }

    return;
}



void fread_obj( CHAR_DATA *ch, FILE *fp )
{
    static OBJ_DATA obj_zero;
    OBJ_DATA *obj;
    char *word;
    char log_buf[MIL];
    long iNest;
    long lago;
    bool fMatch;
    bool fNest;
    bool fVnum;

    if ( obj_free == NULL )
    {
	obj		= alloc_perm( sizeof(*obj) );
    }
    else
    {
	obj		= obj_free;
	obj_free	= obj_free->next;
    }

    *obj		= obj_zero;
    obj->name		= str_dup( "" );
    obj->short_descr	= str_dup( "" );
    obj->description	= str_dup( "" );
    obj->chpoweron	= str_dup( "(null)" );
    obj->chpoweroff	= str_dup( "(null)" );
    obj->chpoweruse	= str_dup( "(null)" );
    obj->victpoweron	= str_dup( "(null)" );
    obj->victpoweroff	= str_dup( "(null)" );
    obj->victpoweruse	= str_dup( "(null)" );
    obj->actcommands	= str_dup( "" );
    obj->questmaker	= str_dup( "" );
    obj->questowner	= str_dup( "" );
    obj->spectype	= 0;
    obj->specpower	= 0;
    obj->toughness	= 5;
    obj->wear_loc	= -1;
    obj->quest		= 0;
    obj->points		= 0;

    fNest		= FALSE;
    fVnum		= TRUE;
    iNest		= 0;

    for ( ; ; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
	    KEY( "ActCommands",	obj->actcommands,	fread_string( fp ) );
	    if ( !str_cmp( word, "Affect" ) || !str_cmp( word, "AffectData" ) )
	    {
		AFFECT_DATA *paf;

		if ( affect_free == NULL )
		{
		    paf		= alloc_perm( sizeof(*paf) );
		}
		else
		{
		    paf		= affect_free;
		    affect_free	= affect_free->next;
		}

		paf->type	= -1;
		paf->duration	= fread_number( fp );
		paf->modifier	= fread_number( fp );
		paf->location	= fread_number( fp );
		paf->bitvector	= 0;
		paf->next	= obj->affected;
		obj->affected	= paf;
		fMatch		= TRUE;
		break;
	    }
	    break;

	case 'C':
	    KEY( "Condition",	lago,			fread_number( fp ) );
	    KEY( "Cost",	obj->cost,		fread_number( fp ) );
	    break;

	case 'D':
	    KEY( "Description",	obj->description,	fread_string( fp ) );
	    break;

	case 'E':
	    KEY( "ExtraFlags",	obj->extra_flags,	fread_number( fp ) );

	    if ( !str_cmp( word, "ExtraDescr" ) )
	    {
		EXTRA_DESCR_DATA *ed;

		if ( extra_descr_free == NULL )
		{
		    ed			= alloc_perm( sizeof(*ed) );
		}
		else
		{
		    ed			= extra_descr_free;
		    extra_descr_free	= extra_descr_free->next;
		}

		ed->keyword		= fread_string( fp );
		ed->description		= fread_string( fp );
		ed->next		= obj->extra_descr;
		obj->extra_descr	= ed;
		fMatch = TRUE;
	    }

	    if ( !str_cmp( word, "End" ) )
	    {
		if ( !fNest || !fVnum )
		{
		    bug( "Fread_obj: incomplete object.", 0 );
		    free_string( obj->name        );
		    free_string( obj->description );
		    free_string( obj->short_descr );
		    obj->next = obj_free;
		    obj_free  = obj;
		    return;
		}
		else
		{
		    obj->next	= object_list;
		    object_list	= obj;
		    obj->pIndexData->count++;
		    if ( iNest == 0 || rgObjNest[iNest] == NULL )
			obj_to_char( obj, ch );
		    else
			obj_to_obj( obj, rgObjNest[iNest-1] );
		    return;
		}
	    }
	    break;

	case 'I':
	    KEY( "ItemType",	obj->item_type,		fread_number( fp ) );
	    break;

	case 'L':
	    KEY( "Level",	obj->level,		fread_number( fp ) );
	    break;

	case 'N':
	    KEY( "Name",	obj->name,		fread_string( fp ) );

	    if ( !str_cmp( word, "Nest" ) )
	    {
		iNest = fread_number( fp );
		if ( iNest < 0 || iNest >= MAX_NEST )
		{
		    bug( "Fread_obj: bad nest %li.", iNest );
		}
		else
		{
		    rgObjNest[iNest] = obj;
		    fNest = TRUE;
		}
		fMatch = TRUE;
	    }
	    break;

	case 'P':
	    KEY( "Points",	obj->points,		fread_number( fp ) );
	    KEY( "Poweronch",	obj->chpoweron,		fread_string( fp ) );
	    KEY( "Poweroffch",	obj->chpoweroff,	fread_string( fp ) );
	    KEY( "Powerusech",	obj->chpoweruse,	fread_string( fp ) );
	    KEY( "Poweronvict",	obj->victpoweron,	fread_string( fp ) );
	    KEY( "Poweroffvict",obj->victpoweroff,	fread_string( fp ) );
	    KEY( "Powerusevict",obj->victpoweruse,	fread_string( fp ) );
	    break;

	case 'Q':
	    KEY( "Quest",	obj->quest,		fread_number( fp ) );
	    KEY( "Questmaker",	obj->questmaker,	fread_string( fp ) );
	    KEY( "Questowner",	obj->questowner,	fread_string( fp ) );
	    break;

	case 'R':
	    KEY( "Resistance",	lago,			fread_number( fp ) );
	    break;

	case 'S':
	    KEY( "ShortDescr",	obj->short_descr,	fread_string( fp ) );
	    KEY( "Spectype",	obj->spectype,		fread_number( fp ) );
	    KEY( "Specpower",	obj->specpower,		fread_number( fp ) );

	    if ( !str_cmp( word, "Spell" ) )
	    {
		long iValue;
		long sn;

		iValue = fread_number( fp );
		sn     = skill_lookup( fread_word( fp ) );
		if ( iValue < 0 || iValue > 3 )
		{
		    bug( "Fread_obj: bad iValue %li.", iValue );
		}
		else if ( sn < 0 )
		{
		    bug( "Fread_obj: unknown skill.", 0 );
		}
		else
		{
		    obj->value[iValue] = sn;
		}
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'T':
	    KEY( "Timer",	obj->timer,		fread_number( fp ) );
	    KEY( "Toughness",	obj->toughness,		fread_number( fp ) );
	    break;

	case 'V':
	    if ( !str_cmp( word, "Values" ) )
	    {
		obj->value[0]	= fread_number( fp );
		obj->value[1]	= fread_number( fp );
		obj->value[2]	= fread_number( fp );
		obj->value[3]	= fread_number( fp );
		if (obj->item_type == ITEM_WEAPON && obj->value[3] > 4) obj->value[3] = 4;
		fMatch		= TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Vnum" ) )
	    {
		long vnum;

		vnum = fread_number( fp );
		if ( ( obj->pIndexData = get_obj_index( vnum ) ) == NULL )
		{
		    vnum = 30041;
		    if ( ( obj->pIndexData = get_obj_index( vnum ) ) == NULL )
			bug( "Fread_obj: bad vnum %li.", vnum );
		    else
			fVnum = TRUE;
		}
		else
		    fVnum = TRUE;
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'W':
	    KEY( "WearFlags",	obj->wear_flags,	fread_number( fp ) );
	    if (!str_cmp(word, "WearLoc"))
	    {
		obj->wear_loc = fread_number(fp);
		if (obj->wear_loc >= MAX_WEAR) obj->wear_loc = WEAR_NONE;
		fMatch = TRUE;
		break;
	    }
	    KEY( "Weight",	obj->weight,		fread_number( fp ) );
	    break;

	}

	if ( !fMatch )
	{
	    sprintf(log_buf, "Fread_obj: no match - %s.", word);
	    log_string( log_buf );
	    fread_to_eol( fp );
	}
    }
}



void fread_tiny_obj( CHAR_DATA *ch, FILE *fp )
{
    OBJ_DATA *obj;
    long iNest, vnum, level;
    bool fNest = FALSE;

    vnum		= fread_number( fp );
    level		= fread_number( fp );

    if ( ( obj = create_object( get_obj_index( vnum ), level ) ) == NULL )
    {
	bug( "Fread_tiny_obj: bad vnum %li.", vnum );
	return;
    }

    iNest		= fread_number( fp );

    if ( iNest < 0 || iNest >= MAX_NEST )
    {
	bug( "Fread_tiny_obj: bad nest %li.", iNest );
    }
    else
    {
	rgObjNest[iNest] = obj;
	fNest = TRUE;
    }

    obj->wear_loc 	= fread_number( fp );
    obj->cost	 	= fread_number( fp );
    obj->timer	 	= fread_number( fp );
			  fread_number( fp );
		 	  fread_number( fp );
    obj->quest	 	= fread_number( fp );
    obj->value[0] 	= fread_number( fp );
    obj->value[1] 	= fread_number( fp );
    obj->value[2] 	= fread_number( fp );
    obj->value[3] 	= fread_number( fp );

    if ( iNest == 0 || rgObjNest[iNest] == NULL )
	obj_to_char( obj, ch );
    else
	obj_to_obj( obj, rgObjNest[iNest-1] );

    return;
}

void do_finger( CHAR_DATA *ch, char *argument )
{
    char strsave[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char letter;
    CHAR_DATA *victim;
    FILE *fp;
    char *word;
    bool fMatch;
    bool drop_out = FALSE;

    char *f_name	= capitalize( argument );
    char *f_title	= "";
    char *f_lasthost	= "";
    char *f_lasttime	= "";
    long   f_extra	= 0;
    long   f_sex	= 0;
    char *f_createtime	= 0;
    long   f_level	= 0;
    long   f_status	= 0;
    char *f_marriage	= "";
    long   f_pkill	= 0;
    long   f_pdeath	= 0;
    long   f_mkill	= 0;
    long   f_mdeath	= 0;
    char *f_email	= "";
    long   f_played	= 0;

    if ( IS_NPC(ch) ) return;

    if (!check_parse_name( argument ))
    {
	send_to_char( "Thats an illegal name.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_finger( ch, argument ) ) != NULL )
    {
	/* Just point directly to the string, no str_dup needed */
	f_title		= victim->pcdata->title;
	f_lasthost	= victim->lasthost;
	f_lasttime	= victim->lasttime;
	f_extra		= victim->extra;
	f_sex		= victim->sex;
	f_createtime	= victim->createtime;
	f_level		= victim->level;
	f_status	= victim->status;
	f_marriage	= victim->pcdata->marriage;
	f_pkill		= victim->pkill;
	f_pdeath	= victim->pdeath;
	f_mkill		= victim->mkill;
	f_mdeath	= victim->mdeath;
	f_email		= victim->pcdata->email;
	f_played	= victim->played;
    }
    else
    {

	if ( !char_exists(argument) )
	{
	    send_to_char( "That player doesn't exist.\n\r", ch );
	    return;
	}

	fflush( fpReserve );
	fclose( fpReserve );
	sprintf( strsave, "%s%s", PLAYER_DIR, capitalize(argument) );
	if ( ( fp = fopen( strsave, "r" ) ) != NULL )
	{
	    if ( ( letter = fread_letter( fp ) ) == EOF)
	    {
		send_to_char("Their playerfile has been corrupted.\n\r",ch);
		fflush( fp );
		fclose( fp );
		fpReserve = fopen( NULL_FILE, "r" );
		return;
	    }
	    fread_to_eol( fp );

	    while ( !drop_out )
	    {
		word   = feof( fp ) ? "End" : fread_word( fp );
		fMatch = FALSE;

		switch ( UPPER(word[0]) )
		{
		default:
		case '*':
		    fMatch = TRUE;
		    fread_to_eol( fp );
		    break;

		case 'C':
		    KEY( "Createtime",	f_createtime,		fread_string( fp ) );
		    break;

		case 'E':
		    KEY( "Email",	f_email,		fread_string( fp ) );
		    if ( !str_cmp( word, "End" ) ) drop_out = TRUE;
		    KEY( "Extra",	f_extra,		fread_number( fp ) );
		    break;

		case 'M':
		    KEY( "Marriage",	f_marriage,		fread_string( fp ) );
		    break;

		case 'L':
		    KEY( "Lasthost",	f_lasthost,		fread_string( fp ) );
		    KEY( "Lasttime",	f_lasttime,		fread_string( fp ) );
		    KEY( "Level",	f_level,		fread_number( fp ) );
		    break;

		case 'P':
		    KEY( "Played",	f_played,		fread_number( fp ) );
		    if ( !str_cmp( word, "PkPdMkMd" ) )
		    {
			f_pkill		= fread_number( fp );
			f_pdeath	= fread_number( fp );
			f_mkill		= fread_number( fp );
			f_mdeath	= fread_number( fp );
			fMatch = TRUE;
			break;
		    }
		    break;

		case 'S':
		    KEY( "Sex",		f_sex,			fread_number( fp ) );
		    KEY( "Status",	f_status,		fread_number( fp ) );
		    break;

		case 'T':
		    if ( !str_cmp( word, "Title" ) )
		    {
			f_title = fread_string( fp );
			if ( isalpha(ch->pcdata->title[0])
			||   isdigit(ch->pcdata->title[0]) )
			{
			    sprintf( buf, " %s", f_title );
			    free_string( f_title );
			    f_title = str_dup( buf );
			}
			fMatch = TRUE;
			break;
		    }
		    break;
		}
		if ( !fMatch ) fread_to_eol( fp );
	    }
	}
	else
	{
	    bug( "Do_finger: fopen", 0 );
	    perror( strsave );
	}
	fflush( fp );
	fclose( fp );
	fpReserve = fopen( NULL_FILE, "r" );
    }
    send_to_char("#l--------------------------------------------------------------------------------#n\n\r",ch);
    sprintf( buf, "%s%s%s.\n\r", f_name, drop_out ? " ": "", f_title );
    send_to_char( buf, ch );
    send_to_char("#l--------------------------------------------------------------------------------#n\n\r",ch);
    if (IS_IMM(ch))
    {
    	sprintf( buf, "Last connected from %s at %s.\n\r", f_lasthost, f_lasttime );
	send_to_char( buf, ch );
    }
    else if ( f_level < LEV_IMM )
    {
        sprintf( buf, "Last connected at %s.\n\r", f_lasttime );
        send_to_char( buf, ch );
    }
    else send_to_char("You may not see the last time a god connected.\n\r",ch);
    send_to_char("#l--------------------------------------------------------------------------------#n\n\r",ch);
    sprintf( buf, "Sex: %s. ", 
	f_sex == SEX_MALE ? "Male" : f_sex == SEX_FEMALE ? "Female" : "None");
    send_to_char( buf, ch );
    sprintf(buf, "Create Time: %s.\n\r", f_createtime);
    stc(buf, ch);

    if (f_sex == SEX_MALE) stc("He ", ch);
    else if (f_sex == SEX_FEMALE) stc("She ", ch);
    else stc("It ", ch);

    switch ( f_level )
    {
	default:
	    stc("is a sleeper, ", ch);
	    break;
	case LEV_AWA:
	case LEV_SOR:
	{
	    switch ( f_status )
	    {
		default:	stc("is a Supreme God, ", ch);
				break;
		case 0:		stc("is a Mortal, ", ch);
				break;
		case 1: case 2: case 3: case 4:
				stc("is an Avatar, ", ch);
				break;
		case 5: case 6: case 7: case 8: case 9:
				stc("is a Godling, ", ch);
				break;
		case 10: case 11: case 12: case 13: case 14:
				stc("is a Demigod, ", ch);
				break;
		case 15: case 16: case 17: case 18: case 19:
				stc("is a Lesser God, ", ch);
				break;
		case 20: case 21: case 22: case 23: case 24:
				stc("is a Greater God, ", ch);
				break;
	    }
	    break;
	}

	case LEV_BUI:
	    stc("is a Builder, ", ch);
	case LEV_IMM:
	    stc("is an Immortal, ", ch);
	    break;
	case LEV_IMP:
	    stc("is an Implementor, ", ch);
	    break;
	case LEV_ST:
	    stc("is a Storyteller, ", ch);
	    break;
    }
    if (f_level < LEV_BUI)
    {
	sprintf(buf, "has %li status, ", f_status);
	stc(buf, ch);
    }
    if (f_played > 0) f_played = f_played / 3600;
    else f_played = 0; /* Avoid negative playing hours */
    sprintf(buf,"and has been playing for %li hours.\n\r", f_played);
    stc(buf,ch);

    if (strlen(f_marriage) > 2)
    {
	if (f_sex == SEX_MALE) stc("He ", ch);
	else if (f_sex == SEX_FEMALE) stc("She ", ch);
	else stc("It ", ch);
	if (IS_SET(f_extra, EXTRA_MARRIED)) sprintf(buf,"is married to %s.\n\r",f_marriage);
	else sprintf(buf,"is engaged to %s.\n\r",f_marriage);
	send_to_char(buf,ch);
    }
    sprintf(buf,"Player kills: %li, Player Deaths: %li.\n\r", f_pkill, f_pdeath);
    send_to_char(buf,ch);
    sprintf(buf,"Mob kills: %li, Mob Deaths: %li.\n\r", f_mkill, f_mdeath);
    send_to_char(buf, ch);
    send_to_char("#l--------------------------------------------------------------------------------#n\n\r",ch);
    if (strlen(f_email) > 2 && (!IS_SET(f_extra, EXTRA_INVIS_EMAIL) || IS_IMM(ch)))
    {
	sprintf(buf,"Email: %s\n\r", f_email);
	send_to_char(buf, ch);
	send_to_char("#l--------------------------------------------------------------------------------#n\n\r",ch);
    }
    return;
}
