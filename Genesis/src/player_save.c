/* Runtime save scheduling. Retry metadata is deliberately not serialized.
 * Atomic file replacement and serialization remain in save.c/atomic_save.c. */
#include <stdio.h>
#include <time.h>
#include "merc.h"
#include "atomic_save.h"

CHAR_DATA *player_save_owner(CHAR_DATA *ch)
{
    if (ch == NULL) return NULL;
    if (ch->desc != NULL) {
        if (!descriptor_is_in_game(ch->desc)) return NULL;
        if (ch->desc->original != NULL) ch = ch->desc->original;
    }
    if (IS_NPC(ch) || ch->pcdata == NULL || ch->level < LEV_AWA ||
        !atomic_save_name_valid(ch->name)) return NULL;
    return ch;
}

static DESCRIPTOR_DATA *save_descriptor(CHAR_DATA *ch)
{
    if (descriptor_is_in_game(ch->desc)) return ch->desc;
    /* Only switched originals need a descriptor-list search. Link-dead
     * ordinary players have no normal autosave, but still receive retries. */
    if (IS_EXTRA(ch,EXTRA_SWITCH))
        for (DESCRIPTOR_DATA *d = descriptor_list; d != NULL; d = d->next)
            if (d->original == ch && descriptor_is_in_game(d)) return d;
    return NULL;
}

unsigned int player_save_state(CHAR_DATA *ch, unsigned int destinations)
{
    CHAR_DATA *owner = player_save_owner(ch);
    unsigned int saved = 0;
    destinations &= PLAYER_SAVE_ALL;
    if (owner == NULL || destinations == 0) return 0;
    get_totalscore(owner);
    check_leaderboard(owner,"");
    if ((destinations & PLAYER_SAVE_PRIMARY) && save_char_obj_checked(owner)) {
        saved |= PLAYER_SAVE_PRIMARY;
        /* Only primary durable success makes the migrated credential safe.
         * Failed/uncertain/skipped writes and a good backup cannot clear it. */
        DESCRIPTOR_DATA *d = save_descriptor(owner);
        if (d != NULL) d->password_save_pending = FALSE;
    }
    if ((destinations & PLAYER_SAVE_BACKUP) && save_char_obj_backup_checked(owner))
        saved |= PLAYER_SAVE_BACKUP;
    return saved;
}

static bool retry_ready(const CHAR_DATA *ch, unsigned int slot)
{
    double elapsed = difftime(current_time,ch->save_retry_time[slot]);
    /* A backwards wall-clock adjustment must not freeze a retry indefinitely.
     * Using elapsed time also avoids adding delays near time_t's upper bound. */
    return elapsed < 0 || elapsed >= ch->save_retry_delay[slot];
}

bool player_primary_retry_ready(const CHAR_DATA *ch)
{
    return ch != NULL &&
        (!(ch->save_pending & PLAYER_SAVE_PRIMARY) || retry_ready(ch,0));
}

static unsigned int autosave_destinations(CHAR_DATA *ch)
{
    bool online = save_descriptor(ch) != NULL;
    unsigned int destinations = 0;
    if (ch->save_attempt_time == current_time) return 0;
    for (unsigned int slot = 0; slot < 2; ++slot) {
        unsigned int bit = slot == 0 ? PLAYER_SAVE_PRIMARY : PLAYER_SAVE_BACKUP;
        if (ch->save_pending & bit) {
            if (retry_ready(ch,slot)) destinations |= bit;
        } else if (online) {
            destinations |= bit;
        }
    }
    return destinations;
}

void player_autosave_tick(void)
{
    CHAR_DATA *selected = NULL;
    unsigned int destinations = 0;
    double greatest_age = -1;
    /* Same one-player budget as the old oldest-save selector, but rotate by
     * actual attempts so one broken destination cannot starve other players. */
    for (CHAR_DATA *ch = char_list; ch != NULL; ch = ch->next) {
        if (player_save_owner(ch) != ch || ch->in_room == NULL) continue;
        unsigned int due = autosave_destinations(ch);
        if (due == 0) continue;
        double age = difftime(current_time,ch->save_attempt_time);
        if (age < 0) age = 1e300; /* Reset future timestamps after clock rollback. */
        if (selected == NULL || age > greatest_age) {
            selected = ch; destinations = due; greatest_age = age;
        }
    }
    if (selected != NULL) player_save_state(selected,destinations);
}
