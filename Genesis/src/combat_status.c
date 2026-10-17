/* Transient combat interruption timers. Updated once per main-loop pulse,
 * before input, for every loaded character (including NPCs and link-dead PCs).
 * Never decrement these from descriptor processing or violence_update. */
#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include "merc.h"

/* A damage callback may extract its target. Compare against live room members
 * before dereferencing the old pointer; checking victim != NULL is not enough.
 * Class attackers are players and remain allocated through death handling. */
bool combat_pair_in_room(ROOM_INDEX_DATA *room, CHAR_DATA *ch, CHAR_DATA *victim)
{
    CHAR_DATA *member, *actor = NULL, *target = NULL;
    if (room == NULL || ch == NULL || victim == NULL) return FALSE;
    /* Neither callback argument may be dereferenced until membership is known. */
    for (member = room->people; member != NULL; member = member->next_in_room)
    {
        if (member == ch) actor = member;
        if (member == victim) target = member;
    }
    return actor != NULL && target != NULL && actor->hit > 0
        && actor->position >= POS_FIGHTING && target->hit > 0
        && target->position != POS_DEAD;
}

bool combat_target_live(CHAR_DATA *ch, CHAR_DATA *victim)
{
    /* Class callers are players; generic callbacks use a cached room instead. */
    return ch != NULL && combat_pair_in_room(ch->in_room, ch, victim);
}

void combat_status_update(void)
{
    CHAR_DATA *ch;

    for (ch = char_list; ch != NULL; ch = ch->next)
    {
        if (ch->combat_pause > 0) --ch->combat_pause;
        if (ch->stun_immunity > 0) --ch->stun_immunity;
    }
}
