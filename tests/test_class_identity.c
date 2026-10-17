/* Class identities are saved values, not an implicit multi-class API. */
#include <sys/types.h>
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include "../Genesis/src/merc.h"

_Static_assert(CLASS_NONE == 0 && CLASS_VAMPIRE == 1 && CLASS_WEREWOLF == 2
    && CLASS_SORCERER == 4 && CLASS_DROW == 8 && CLASS_HIGHLANDER == 16
    && CLASS_MAGE == 32 && CLASS_DRAGON == 64 && CLASS_NINJA == 128
    && CLASS_MONK == 256 && CLASS_ELEM == 512,
    "Serialized class IDs require an explicit migration, not a refactor");

static void check(long id, bool npc)
{
    CHAR_DATA ch = {0};
    ch.class = id;
    if (npc) SET_BIT(ch.act, ACT_IS_NPC);
    const long expected[] = {
        CLASS_VAMPIRE, CLASS_WEREWOLF, CLASS_SORCERER, CLASS_DROW,
        CLASS_HIGHLANDER, CLASS_MAGE, CLASS_DRAGON, CLASS_NINJA,
        CLASS_MONK, CLASS_ELEM
    };
    const bool actual[] = {
        IS_VAMPIRE(&ch), IS_WEREWOLF(&ch), IS_SORCERER(&ch), IS_DROW(&ch),
        IS_HIGHLANDER(&ch), IS_MAGE(&ch), IS_DRAGON(&ch), IS_NINJA(&ch),
        IS_MONK(&ch), IS_ELEM(&ch)
    };
    for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i)
        assert(actual[i] == (!npc && id == expected[i]));
}

int main(void)
{
    const long ids[] = {CLASS_NONE, CLASS_VAMPIRE, CLASS_WEREWOLF,
        CLASS_SORCERER, CLASS_DROW, CLASS_HIGHLANDER, CLASS_MAGE,
        CLASS_DRAGON, CLASS_NINJA, CLASS_MONK, CLASS_ELEM,
        CLASS_VAMPIRE | CLASS_WEREWOLF, CLASS_VAMPIRE | CLASS_MAGE,
        -1, LONG_MIN, LONG_MAX};
    for (size_t i = 0; i < sizeof(ids) / sizeof(ids[0]); ++i)
    {
        check(ids[i], FALSE);
        check(ids[i], TRUE);
    }
    puts("class identity regressions passed");
    return 0;
}
