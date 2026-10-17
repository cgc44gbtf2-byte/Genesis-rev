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

void do_setflag( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg1[MIL];
    char arg2[MIL];
    char buf[MSL];
    long flag;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    if (arg1[0] == '\0' || arg2[0] == '\0')
    {
	stc("Syntax: setflag <victim> <argument>\n\r", ch);
	stc("  Flag can be: PK, RP, Tester, AFK, Coding, Flamer, Lazy, Pimp(ST only)\n\r", ch);
	return;
    }
    if ((victim = get_char_world(ch, arg1)) == NULL)
    {
	stc("No such character.\n\r", ch);
	return;
    }

    if (victim->level > ch->level || IS_NPC(victim))
    {
	stc("Bad target.\n\r", ch);
	return;
    }

    if (!str_cmp(arg2, "rp")) flag = FLAG_RP;
    else if (!str_cmp(arg2, "pk")) flag = FLAG_PK;
    else if (!str_cmp(arg2, "tester")) flag = FLAG_TEST;
    else if (!str_cmp(arg2, "afk")) flag = FLAG_AFK;
    else if (!str_cmp(arg2, "pimp") && IS_ST(ch)) flag = FLAG_PIMP;
    else if (!str_cmp(arg2, "kinky")) flag = FLAG_KINKY;
    else if (!str_cmp(arg2, "ccc")) flag = FLAG_CCC;
    else if (!str_cmp(arg2, "lazy")) flag = FLAG_LAZY;
    else
    {
	do_setflag(ch, "");
	return;
    }

    if (IS_FLAG(victim, flag))
    {
	REMOVE_BIT(victim->pcdata->flag, flag);
	sprintf(buf, "Removed %s's %s flag.\n\r", victim->name, arg2);
    }
    else
    {
	SET_BIT(victim->pcdata->flag, flag);
	sprintf(buf, "Set %s's %s flag.\n\r", victim->name, arg2);
    }
    stc(buf, ch);
    return;
}

void do_pkme( CHAR_DATA *ch, char *argument )
{
    char arg[MSL];
    char buf[MSL];

    one_argument(argument, arg);

    if (IS_NPC(ch)) return;

    if (IS_FLAG(ch, FLAG_PK))
    {
	stc("You are already a PK player.\n\r", ch);
	return;
    }

    if (IS_FLAG(ch, FLAG_TEST))
    {
	stc("Testers may not go PK.\n\r", ch);
	return;
    }

    if (ch->max_hit < 10000)
    {
	stc("You are not strong enough to go PK.\n\r", ch);
	return;
    }

    if (arg[0] == '\0' || str_cmp(arg, "agree"))
    {
	do_help(ch, "pkme");
	stc("\n\rType #rPkme agree#n in order to open your self to PK.\n\r", ch);
	return;
    }

    SET_BIT(ch->pcdata->flag, FLAG_PK);
    stc("You are now a PK player.\n\r", ch);
    sprintf(buf, "It is now open season on #r%s#n!", ch->name);
    do_info(ch, buf);

    return;
}

void do_afk( CHAR_DATA *ch, char *argument )
{
    char buf[MSL];

    if (IS_NPC(ch)) return;

    if (IS_FLAG(ch, FLAG_AFK))
    {
	REMOVE_BIT(ch->pcdata->flag, FLAG_AFK);
	stc("You are no longer afk.\n\r", ch);
	sprintf(buf, "%s is no longer AFK.", ch->name);
    }
    else
    {
	if (ch->fight_timer > 0)
	{
	    stc("Not with a fight timer.\n\r", ch);
	    return;
	}
	SET_BIT(ch->pcdata->flag, FLAG_AFK);
	stc("You are now afk.\n\r", ch);
	sprintf(buf, "%s is now AFK.", ch->name);
    }
    do_info(ch, buf);
    return;
}

void do_roleplay( CHAR_DATA *ch, char *argument )
{
    char buf[MSL];

    if (IS_NPC(ch)) return;
    if (IS_FLAG(ch, FLAG_RP))
    {
	REMOVE_BIT(ch->pcdata->flag, FLAG_RP);
	stc("You are no longer in RP mode.\n\r", ch);
	sprintf(buf, "#r%s#n wishes to stop roleplaying.", ch->name);
    }
    else
    {
	SET_BIT(ch->pcdata->flag, FLAG_RP);
	stc("You are now in RP mode.\n\r", ch);
	sprintf(buf, "#r%s#n has chosen to roleplay.", ch->name);
    }
    do_info(ch, buf);
    return;
}
