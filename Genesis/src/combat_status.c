/* Transient combat interruption timers. Updated once per main-loop pulse,
 * before input, for every loaded character (including NPCs and link-dead PCs).
 * Never decrement these from descriptor processing or violence_update. */
#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include "merc.h"

void combat_status_update(void)
{
    CHAR_DATA *ch;

    for (ch = char_list; ch != NULL; ch = ch->next)
    {
        if (ch->combat_pause > 0) --ch->combat_pause;
        if (ch->stun_immunity > 0) --ch->stun_immunity;
    }
}
