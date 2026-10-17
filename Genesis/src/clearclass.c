#include <stdio.h>
#include <time.h>
#include "merc.h"

void do_clearclass( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );

    if ( arg[0] == '\0' || is_number(arg) )
    {
        send_to_char( "Clear whose class?\n\r", ch );
        return;
    }
    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }
    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }
	do_mclear(victim, "self");

    victim->max_hit                     = 10000;
    victim->hit                         = 10000;
    victim->max_mana                    = 10000;
    victim->mana                        = 10000;
    victim->exp				= 0;
    victim->stats[STAT_STR]		= 1;
    victim->stats[STAT_DEX]		= 1;
    victim->stats[STAT_CON]		= 1;
    victim->stats[STAT_INT]		= 1;
    victim->stats[STAT_WIS]		= 1;

    if (IS_EXTRA(victim, EXTRA_MARRIED)) victim->extra	= 131072;
    else victim->extra			= 0;

    victim->more                            = 0;
    victim->status			    = 0;
    victim->pcdata->familiar                = NULL;
    victim->pcdata->propose                 = NULL;
    victim->pcdata->chobj                   = NULL;
    victim->pcdata->conding                 = str_dup( "" );
    victim->pcdata->love                    = str_dup( "" );
    victim->pcdata->covenant                = str_dup( "" );
    victim->pcdata->cov_time                = str_dup( "None" );
    victim->pcdata->leader                  = str_dup( "" );
    victim->pcdata->vleader                 = str_dup( "" );
    victim->lord                            = str_dup( "" );
    victim->clan                            = str_dup( "" );
    victim->morph                           = str_dup( "" );
    victim->pcdata->denydate                = 0;
    victim->home                            = 3001;
    victim->pcdata->homecv                  = 0;
    victim->vampgen                         = 0;
    victim->vampaff                         = 0;
    victim->itemaffect                      = 0;
    victim->polyaff                         = 0;
    victim->immune                          = 0;
    victim->beast                           = 20;
    victim->truebeast                       = 20;
    victim->weapon[0]			= 0;
    victim->weapon[1]			= 0;
    victim->weapon[2]			= 0;
    victim->weapon[3]			= 0;
    victim->weapon[4]			= 0;
    victim->weapon[5]			= 0;
    victim->style[0]			= 0;
    victim->style[1]			= 0;
    victim->style[2]			= 0;
    victim->style[3]			= 0;
    victim->style[4]			= 0;
    victim->style[5]			= 0;
    victim->style[6]			= 0;
    victim->stance[0]                       = 0;
    victim->stance[1]                       = 0;
    victim->stance[2]                       = 0;
    victim->stance[3]                       = 0;
    victim->stance[4]                       = 0;
    victim->stance[5]                       = 0;
    victim->stance[6]                       = 0;
    victim->stance[7]                       = 0;
    victim->stance[8]                       = 0;
    victim->stance[9]                       = 0;
    victim->stance[10]                      = 0;
    victim->embraced                        = 0;
    victim->blood[0]                        = 100;
    victim->blood[1]                        = 100;
    victim->pcdata->quest                   = 0;
    victim->pcdata->wolf                    = 0;
    victim->pcdata->rank                    = 0;
    victim->pcdata->regenerate              = 0;
    victim->pcdata->atm                     = 0;
    victim->pcdata->necromancy              = 0;
    victim->pcdata->obeah                   = 0;
    victim->pcdata->weather			= 0;
    victim->pcdata->darkness                = 0;
    victim->pcdata->silence                 = 0;
    victim->pcdata->bodycrafts		    = 0;
    victim->pcdata->celerity                = 0;
    victim->pcdata->followers               = 0;
    victim->pcdata->score[0]                = 0;
    victim->pcdata->score[1]                = 0;
    victim->pcdata->score[2]                = 0;
    victim->pcdata->score[3]                = 0;
    victim->pcdata->score[4]                = 0;
    victim->pcdata->score[5]                = 0;
    victim->pcdata->dragcol                 = 0;
    victim->pcdata->cland[0]                = 0;
    victim->pcdata->cland[1]                = 0;
    victim->pcdata->cland[2]                = 0;
    victim->pcdata->tgift[0]		    = 0;
    victim->pcdata->tgift[1]		    = 0;
    victim->pcdata->tgift[2]		    = 0;
    victim->pcdata->diableries[0]           = 0;
    victim->pcdata->diableries[1]           = 0;
    victim->pcdata->breed[0]                = 0;
    victim->pcdata->breed[1]                = 0;
    victim->pcdata->breed[2]                = 0;
    victim->pcdata->auspice[0]              = 0;
    victim->pcdata->auspice[1]              = 0;
    victim->pcdata->auspice[2]              = 0;
    victim->pcdata->auspice[3]              = 0;
    victim->pcdata->auspice[4]              = 0;
    victim->pcdata->tribes[0]               = 0;
    victim->pcdata->tribes[1]               = 0;
    victim->pcdata->tribes[2]               = 0;
    victim->pcdata->tribes[3]               = 0;
    victim->pcdata->tribes[4]               = 0;
    victim->pcdata->tribes[5]               = 0;
    victim->pcdata->tribes[6]               = 0;
    victim->pcdata->tribes[7]               = 0;
    victim->pcdata->tribes[8]               = 0;
    victim->pcdata->tribes[9]               = 0;
    victim->pcdata->tribes[10]              = 0;
    victim->pcdata->tribes[11]              = 0;
    victim->pcdata->tribes[12]              = 0;
    victim->pcdata->tribes[13]              = 0;
    victim->pcdata->tribes[14]              = 0;
    victim->pcdata->tribes[15]              = 0;
    victim->pcdata->tribes[16]              = 0;
    victim->pcdata->tribes[17]              = 0;
    victim->pcdata->tribes[18]              = 0;
    victim->pcdata->tribes[19]              = 0;
    victim->pcdata->powers[0]               = 0;
    victim->pcdata->powers[1]               = 0;
    victim->pcdata->powers[2]               = 0;
    victim->pcdata->powers[3]               = 0;
    victim->pcdata->powers[4]               = 0;
    victim->pcdata->powers[5]               = 0;
    victim->pcdata->powers[6]               = 0;
    victim->pcdata->powers[7]               = 0;
    victim->pcdata->powers[8]               = 0;
    victim->pcdata->powers[9]               = 0;
    victim->pcdata->powers[10]              = 0;
    victim->pcdata->powers[11]              = 0;
    victim->pcdata->powers[12]              = 0;
    victim->pcdata->powers[13]              = 0;
    victim->pcdata->powers[14]              = 0;
    victim->pcdata->powers[15]              = 0;
    victim->pcdata->powers[16]              = 0;
    victim->pcdata->powers[17]              = 0;
    victim->pcdata->powers[18]              = 0;
    victim->pcdata->powers[19]              = 0;
    victim->pcdata->willpower[0]            = 50;
    victim->pcdata->willpower[1]            = 50;
    victim->pcdata->resist[0]               = 0;
    victim->pcdata->resist[1]               = 0;
    victim->pcdata->resist[2]               = 0;
    victim->pcdata->resist[3]               = 0;
    victim->pcdata->resist[4]               = 0;
    victim->pcdata->drow[0]			= 0;
    victim->pcdata->drow[1]			= 0;
    victim->pcdata->highland[0]			= 0;
    victim->pcdata->highland[1]			= 0;
    victim->pcdata->highland[2]			= 0;
    victim->pcdata->highland[3]			= 0;
    victim->pcdata->monk[0]			= 0;
    victim->pcdata->monk[1]			= 0;
    victim->pcdata->monk[2]			= 0;
    victim->pcdata->monk[3]			= 0;
    victim->pcdata->ninja[0]			= 0;
    victim->pcdata->ninja[1]			= 0;
    victim->pcdata->ninja[2]			= 0;
    victim->pcdata->ninja[3]			= 0;
    victim->pcdata->ninja[4]			= 0;
    victim->pcdata->dragskills		    = 0;
    victim->pcdata->fake_skill              = 0;
    victim->pcdata->fake_stance             = 0;
    victim->pcdata->fake_hit                = 0;
    victim->pcdata->fake_dam                = 0;
    victim->pcdata->fake_ac                 = 0;
    victim->pcdata->fake_hp                 = 0;
    victim->pcdata->fake_mana               = 0;
    victim->pcdata->fake_max_hp             = 0;
    victim->pcdata->fake_max_mana           = 0;
    victim->pcdata->obj_vnum                = 0;
    victim->pcdata->majesty                 = 0;
    victim->pcdata->flag		    = 0;
    victim->pcdata->classpoints[0]		= 0;
    victim->pcdata->classpoints[1]		= 0;
    victim->class                           = 0;
    victim->affected_by			    = 0;
    if (!IS_BUI(victim)) victim->level = LEV_AWA;

    send_to_char( "Ok.\n\r", ch );
    send_to_char( "Your class has been cleared.\n\r", victim );
    do_save(victim, "");
    return;
}


