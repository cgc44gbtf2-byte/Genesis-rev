/*
 * Room Traps by Akuma.
 */
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
 * Triggers traps.  NPCs and Imms are not affected by traps.
 * Checks to see if trap is avoided, displays the message, then checks the affects.
 * Most trap affects do not display their own messages, handle it in the first message.
 */
void trigger_trap( CHAR_DATA *ch, TRAP_DATA *trap )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    char buf[MSL];
    int dodge, roll;
    bool avoid;

    if (trap == NULL || IS_NPC(ch) || IS_BUI(ch)) return;

    dodge = char_stat(ch, STAT_DEX);
    if (IS_NINJA(ch)) dodge += get_ninja(ch, NINJA_NINGENNO) / 2;
    if (IS_VAMPIRE(ch)) dodge += (get_disc(ch, DISC_CELERITY) + 1) / 2;

    roll = number_range(1, 20);

    if (trap->diff < 0) avoid = FALSE;
    else if (roll == 1) avoid = FALSE;
    else if (roll == 20) avoid = TRUE;
    else if (roll + dodge >= trap->diff) avoid = TRUE;
    else avoid = FALSE;

    if (avoid)
    {
	if (!IS_TRAP(trap, TRAP_SILENT))
	{
	    stc("You narrowly avoid a trap!\n\r", ch);
	    act("$n narrowly avoids a trap!", ch, NULL, NULL, TO_ROOM);
	}
	return;
    }

    sprintf(buf, "%s\n\r", trap->charmess);
    stc(buf, ch);
    if (IS_TRAP(trap, TRAP_ALARM))
    {
	sprintf(buf, "#r%s#n %s", ch->name, trap->roommess);
	do_info(ch, buf);
    }
    else act(trap->roommess, ch, NULL, NULL, TO_ROOM);

    if (IS_TRAP(trap, TRAP_AREA))
    {
	for(vch = ch->in_room->people; vch != NULL; vch = vch_next)
	{
	    vch_next = vch->next_in_room;
	    if (IS_IMM(vch) || IS_NPC(vch)) continue;

	    if (trap->numhits > 0) tdamage(vch, trap);
	    if (trap->spellsn > 0) tspell(vch, trap);
	    if (trap->movevnum > 0) tmove(vch, trap);
	    else if (trap->summonvnum > 0) tsummon(vch, trap);
	}
    }
    else
    {
	if (trap->numhits > 0) tdamage(ch, trap);
	if (trap->spellsn > 0) tspell(ch, trap);
	if (trap->movevnum > 0) tmove(ch, trap);
	else if (trap->summonvnum > 0) tsummon(ch, trap);
    }

    return;
}


/*
 * A damage trap(spear, boulders, etc...)
 */
void tdamage( CHAR_DATA *ch, TRAP_DATA *trap )
{
    char buf[MSL];
    int damage, num;

    for(num = 0; num < trap->numhits; num++)
    {
	damage = number_range(trap->lodam, trap->hidam);
	sprintf(buf, "%s (#r%d#n)\n\r", trap->dammess, damage);
	stc(buf, ch);
	ch->hit -= damage;
	if (ch->hit < 0)
	{
	    raw_kill(ch, ch);
	    return;
	}
	if (num > 10) break;
    }

    return;
}


/*
 * Trap Spell, hits the player with a specific spell.
 */
void tspell( CHAR_DATA *ch, TRAP_DATA *trap )
{
    return;
}


/*
 * Trap Move, Either a pit or teleport type trap.  Triggers trap in next room.
 * Be careful of infinite recursion with looping traps and impossible difficulties.
 */
void tmove( CHAR_DATA *ch, TRAP_DATA *trap )
{
    if (trap->movevnum == ch->in_room->vnum)
    {
	bug("Bad Trap: Move Vnum %li", ch->in_room->vnum);
	return;
    }

    char_from_room(ch);
    char_to_room(ch, get_room_index(trap->movevnum));
    do_look(ch, "auto");
    trigger_trap(ch, ch->in_room->trap);

    return;
}


/*
 * Trap Summon, "Summons" a monster to attack the intruder.
 * Creates the monster, forces it into a fight with ch.
 */
void tsummon( CHAR_DATA *ch, TRAP_DATA *trap )
{
    return;
}
