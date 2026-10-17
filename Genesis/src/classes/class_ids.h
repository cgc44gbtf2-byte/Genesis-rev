#ifndef GENESIS_CLASS_IDS_H
#define GENESIS_CLASS_IDS_H

/*
 * Stable player-file identities, not combinable ancestry flags.
 * Preserve these values when organizing class implementations.
 * Hybrid permissions require a separate, explicitly reviewed design.
 */
#define CLASS_NONE       0
#define CLASS_VAMPIRE    1
#define CLASS_WEREWOLF   2
#define CLASS_SORCERER   4
#define CLASS_DROW       8
#define CLASS_HIGHLANDER 16
#define CLASS_MAGE       32
#define CLASS_DRAGON     64
#define CLASS_NINJA      128
#define CLASS_MONK       256
#define CLASS_ELEM       512

#endif
