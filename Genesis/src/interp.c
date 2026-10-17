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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h> /* unlink() */
#include "merc.h"



   bool	check_social	args( ( CHAR_DATA *ch, char *command, char *argument ) );


/*
 * Command logging types.
 */
#define LOG_NORMAL	0
#define LOG_ALWAYS	1
#define LOG_NEVER	2



/*
 * Log-all switch.
 */
   bool				fLogAll		= FALSE;

   bool  check_disabled (const struct cmd_type *command);
   BAN_DATA *ban_first;
   DISABLED_DATA *disabled_first;
#define END_MARKER    "END" /* for load_disabled() and save_disabled() */

/*
 * Command table.
 */
   const	struct	cmd_type	cmd_table	[] =
   {
    /*
     * Common movement commands.
     */
   { "blah",		do_blah,	POS_DEAD,	LEV_ST,   LOG_NORMAL	},
   { "north",		do_north,	POS_STANDING,	LEV_SLE,  LOG_NORMAL	},
   { "east",		do_east,	POS_STANDING,	LEV_SLE,  LOG_NORMAL	},
   { "south",		do_south,	POS_STANDING,	LEV_SLE,  LOG_NORMAL	},
   { "west",		do_west,	POS_STANDING,	LEV_SLE,  LOG_NORMAL	},
   { "up",		do_up,		POS_STANDING,	LEV_SLE,  LOG_NORMAL	},
   { "down",		do_down,	POS_STANDING,	LEV_SLE,  LOG_NORMAL	},
   { "inventory",	do_inventory,	POS_DEAD,	LEV_SLE,  LOG_NORMAL	},
   { "kill",		do_kill,	POS_FIGHTING,	LEV_SLE,  LOG_NORMAL	},
   { "look",		do_look,	POS_MEDITATING,	LEV_SLE,  LOG_NORMAL	},
   { "at",		do_at,		POS_DEAD,	LEV_IMM,  LOG_NORMAL	},

    /*
     * Common other commands.
     * Placed here so one and two letter abbreviations work.
     */
   { "buy",		do_buy,		POS_RESTING,	LEV_SLE,  LOG_NORMAL	},
   { "cast",		do_cast,	POS_FIGHTING,	LEV_SLE,  LOG_NORMAL	},
   { "command",		do_command,	POS_RESTING,	LEV_AWA,  LOG_NORMAL	},
   { "enter",		do_enter,	POS_STANDING,	LEV_SLE,  LOG_NORMAL	},
   { "exits",		do_exits,	POS_RESTING,	LEV_SLE,  LOG_NORMAL	},
   { "exp",		do_level,	POS_DEAD,	LEV_SLE,  LOG_NORMAL	},
   { "get",		do_get,		POS_RESTING,	LEV_SLE,  LOG_NORMAL	},
   { "grip",            do_grip,        POS_RESTING,    LEV_SLE,  LOG_NORMAL    },
   { "level",		do_level,	POS_DEAD,	LEV_SLE,  LOG_NORMAL	},
   { "order",		do_order,	POS_RESTING,	LEV_SLE,  LOG_ALWAYS	},
   { "peek",		do_peek,	POS_STANDING,	LEV_SLE,  LOG_NORMAL	},
   { "pour",		do_pour,	POS_DEAD,	LEV_SLE,  LOG_NORMAL	},
   { "rest",		do_rest,	POS_MEDITATING,	LEV_SLE,  LOG_NORMAL	},
   { "sit",		do_sit,		POS_SITTING,	LEV_SLE,  LOG_NORMAL	},
   { "stand",		do_stand,	POS_SLEEPING,	LEV_SLE,  LOG_NORMAL	},
   { "tell",		do_tell,	POS_DEAD,	LEV_SLE,  LOG_NORMAL	},
   { "whisper",		do_whisper,	POS_RESTING,	LEV_SLE,  LOG_NORMAL	},
   { "wield",		do_wear,	POS_RESTING,	LEV_SLE,  LOG_NORMAL	},
   { "wizlist",		do_wizlist,	POS_DEAD,	LEV_SLE,  LOG_NORMAL	},
   { "travel",		do_travel,	POS_STANDING,	LEV_AWA,  LOG_NORMAL	},

     /*
      * Shit commands by Carandir
      */
   { "horrid",		do_horrid,	POS_FIGHTING,	 LEV_SLE,  LOG_NORMAL	},
   { "judge",		do_judgement,	POS_STANDING,	 LEV_SLE,  LOG_NORMAL 	},
   { "forge",		do_forge,	POS_STANDING,	 LEV_SLE,  LOG_NORMAL 	},
   { "eshift",		do_earthshift,  POS_STANDING,	 LEV_SLE,  LOG_NORMAL 	},
   { "empathy",		do_empathy,	POS_STANDING,	 LEV_SLE,  LOG_NORMAL},
   { "enrage",		do_enrage,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "disintegrate",	do_disint,	POS_FIGHTING,	 LEV_SLE,  LOG_NORMAL 	},
   { "thinblood",	do_thinblood,   POS_FIGHTING, 	 LEV_SLE,  LOG_NORMAL 	},
   { "tenebrous",	do_tenebrous,   POS_FIGHTING,	 LEV_SLE,  LOG_NORMAL	},
   { "torment",		do_torment,	POS_FIGHTING,	 LEV_SLE,  LOG_NORMAL	},
   { "shadowforge",	do_shadowforge,	POS_STANDING,	 LEV_SLE,  LOG_NORMAL	},
   { "increase",	do_increase,	POS_FIGHTING,	 LEV_SLE,  LOG_NORMAL 	},
   { "dust",		do_dust,	POS_FIGHTING,	 LEV_SLE,  LOG_NORMAL 	},
   { "horns",		do_horns,	POS_FIGHTING,	 LEV_SLE,  LOG_NORMAL 	},
   { "dragonskill",	do_dragonskill,	POS_STANDING,	 LEV_SLE,  LOG_NORMAL 	},
   { "stail",		do_stail,	POS_FIGHTING,	 LEV_SLE,  LOG_NORMAL 	},
   { "morph",		do_dragonform,	POS_FIGHTING,	 LEV_SLE,  LOG_NORMAL	},
   { "htwist",		do_htwist,	POS_FIGHTING,	 LEV_SLE,  LOG_NORMAL	},
   { "bflash",		do_bladeflash,	POS_FIGHTING,	 LEV_SLE,  LOG_NORMAL	},
   { "bash",		do_bash,	POS_FIGHTING,	 LEV_SLE,  LOG_NORMAL	},
   { "lunge",		do_lunge,       POS_STANDING,    LEV_SLE,  LOG_NORMAL 	},  
   
     /*
      * Informational commands.
      */
   { "afk",		do_afk,         POS_STANDING,	LEV_SLE,  LOG_NORMAL 	},
   { "affects",		do_affects,	POS_DEAD,	 LEV_SLE,  LOG_NORMAL	},
   { "areas",		do_areas,	POS_DEAD,	 LEV_SLE,  LOG_NORMAL	},
   { "commands",	do_commands,	POS_DEAD,	 LEV_SLE,  LOG_NORMAL 	},
   { "compare",		do_compare,	POS_SITTING,	 LEV_SLE,  LOG_NORMAL 	},
   { "consider",	do_consider,	POS_SITTING,	 LEV_SLE,  LOG_NORMAL	},
   { "credits",		do_credits,	POS_DEAD,	 LEV_SLE,  LOG_NORMAL	},
   { "equipment",	do_equipment,	POS_DEAD,	 LEV_SLE,  LOG_NORMAL	},
   { "examine",		do_examine,	POS_SITTING,	 LEV_SLE,  LOG_NORMAL	},
   { "help",		do_help,	POS_DEAD,	 LEV_SLE,  LOG_NORMAL	},
   { "heal",		do_heal,        POS_STANDING,    LEV_SLE,  LOG_NORMAL 	},
   { "report",		do_report,	POS_DEAD,	 LEV_SLE,  LOG_NORMAL	},
   { "score",		do_score,	POS_DEAD,	 LEV_SLE,  LOG_NORMAL	},
   { "skill",		do_skill,	POS_MEDITATING,	 LEV_SLE,  LOG_NORMAL 	},
   { "socials",		do_socials,	POS_DEAD,	 LEV_SLE,  LOG_NORMAL 	},
   { "spells",		do_spells,	POS_DEAD,	 LEV_SLE,  LOG_NORMAL 	},
   { "statcheck",	do_statcheck,   POS_DEAD,	 LEV_IMM,  LOG_NORMAL   },
   { "time",		do_time,	POS_DEAD,	 LEV_SLE,  LOG_NORMAL	},
   { "totalscore",	do_totalscore,	POS_DEAD,	 LEV_SLE,  LOG_NORMAL	},
   { "weather",		do_weather,	POS_SITTING,	 LEV_SLE,  LOG_NORMAL	},
   { "who",		do_who,		POS_DEAD,	 LEV_SLE,  LOG_NORMAL	},
   
    /*
     * Configuration commands.
     */
   { "channels",	do_channels,	POS_DEAD,	 LEV_SLE,  LOG_NORMAL	},
   { "config",		do_config,	POS_DEAD,	 LEV_SLE,  LOG_NORMAL	},
   { "description",	do_description,	POS_DEAD,	 LEV_SLE,  LOG_NORMAL	},
   { "password",	do_password,	POS_DEAD,	 LEV_SLE,  LOG_NEVER	},
   { "title",		do_title,	POS_DEAD,	 LEV_SLE,  LOG_NORMAL	},
   { "ansi",		do_ansi,	POS_DEAD,	 LEV_SLE,  LOG_NORMAL	},
   { "autoexit",	do_autoexit,	POS_DEAD,	 LEV_SLE,  LOG_NORMAL	},
   { "autofollow",	do_autofollow,  POS_DEAD,	 LEV_SLE,  LOG_NORMAL   },
   { "autosearch",	do_autosearch,	POS_DEAD,	 LEV_SLE,  LOG_NORMAL	},
   { "blank",		do_blank,	POS_DEAD,	 LEV_SLE,  LOG_NORMAL	},
   { "brief",		do_brief,	POS_DEAD,	 LEV_SLE,  LOG_NORMAL	},
   { "cprompt",		do_cprompt,	POS_DEAD,	 LEV_SLE,  LOG_NORMAL	},
   { "prompt",		do_prompt,	POS_DEAD,	 LEV_SLE,  LOG_NORMAL	},
   { "showdamage",	do_showdamage,  POS_DEAD,	 LEV_SLE,  LOG_NORMAL	},
    /*
     * Communication commands.
     */
   { "board",		do_board,       POS_DEAD,        LEV_SLE,  LOG_NORMAL 	},   
   { "chat",		do_chat,	POS_DEAD,	 LEV_SLE,  LOG_NORMAL	},
   { ".",		do_chat,	POS_DEAD,	 LEV_SLE,  LOG_NORMAL	},
   { "email",		do_email,	POS_DEAD,	 LEV_SLE,  LOG_NORMAL	},
   { "gtell",		do_gtell,	POS_DEAD,	 LEV_SLE,  LOG_NORMAL	},
   { ";",		do_gtell,	POS_DEAD,	 LEV_SLE,  LOG_NORMAL	},
   { "hightalk",	do_hightalk,	POS_DEAD,	LEV_SLE,  LOG_NORMAL	},
   { "howl",		do_howl,	POS_DEAD,	 LEV_SLE,  LOG_NORMAL	},
   { "magetalk",	do_magetalk,	POS_DEAD,	LEV_SLE,  LOG_NORMAL	},
   { "mikt",	 	do_miktalk,	POS_DEAD,	LEV_SLE,  LOG_NORMAL	},
   { "monk",            do_monk,        POS_DEAD,        LEV_SLE,  LOG_NORMAL   },
   { "music",		do_music,	POS_SLEEPING,	 LEV_SLE,  LOG_NORMAL 	},
   { "newbie",		do_newbie,	POS_SLEEPING,	 LEV_SLE,  LOG_NORMAL 	},
   { "note",		do_note,	POS_DEAD,	 LEV_SLE,  LOG_NORMAL	},
   { "pose",		do_emote,	POS_SITTING,	 LEV_SLE,  LOG_NORMAL	},
   { "quest",		do_quest,	POS_SITTING,	 LEV_SLE,  LOG_NORMAL	},
   { "reply",		do_reply,	POS_DEAD,	 LEV_SLE,  LOG_NORMAL 	},
   { "say",		do_say,		POS_MEDITATING,	 LEV_SLE,  LOG_NORMAL	},
   { "'",		do_say,		POS_MEDITATING,	 LEV_SLE,  LOG_NORMAL	},
   { "shout",		do_shout,	POS_SITTING,	 LEV_SLE,  LOG_NORMAL	},
   { "sorctalk",	do_sorctalk,	POS_DEAD,	LEV_AWA,  LOG_NORMAL 	},
   { "telepathy",	do_telepathy,	POS_DEAD,	LEV_SLE,  LOG_NORMAL	},
   { "yell",		do_yell,	POS_SITTING,	 LEV_SLE,  LOG_NORMAL	},
   { "ooc",		do_ooc,         POS_DEAD,        LEV_SLE,  LOG_NORMAL 	},

    /*
     * Object manipulation commands.
     */
   { "activate",	do_activate,	POS_STANDING,	 LEV_SLE,  LOG_NORMAL	},
   { "brandish",	do_brandish,	POS_SITTING,	 LEV_SLE,  LOG_NORMAL	},
   { "close",		do_close,	POS_SITTING,	 LEV_SLE,  LOG_NORMAL	},
   { "drop",		do_drop,	POS_SITTING,	 LEV_SLE,  LOG_NORMAL	},
   { "eat",		do_eat,		POS_SITTING,	 LEV_SLE,  LOG_NORMAL	},
   { "give",		do_give,	POS_SITTING,	 LEV_SLE,  LOG_ALWAYS	},
   { "hold",		do_wear,	POS_SITTING,	 LEV_SLE,  LOG_NORMAL	},
   { "list",		do_list,	POS_SITTING,	 LEV_SLE,  LOG_NORMAL	},
   { "lock",		do_lock,	POS_SITTING,	 LEV_SLE,  LOG_NORMAL	},
   { "open",		do_open,	POS_SITTING,	 LEV_SLE,  LOG_NORMAL	},
   { "pick",		do_pick,	POS_SITTING,	 LEV_SLE,  LOG_NORMAL	},
   { "press",		do_activate,	POS_STANDING,	 LEV_SLE,  LOG_NORMAL	},
   { "pull",		do_activate,	POS_STANDING,	 LEV_SLE,  LOG_NORMAL	},
   { "put",		do_put,		POS_SITTING,	 LEV_SLE,  LOG_NORMAL 	},
   { "quaff",		do_quaff,	POS_SITTING,	LEV_SLE,  LOG_NORMAL	},
   { "recite",		do_recite,	POS_SITTING,	LEV_SLE,  LOG_NORMAL	},
   { "remove",		do_remove,	POS_SITTING,	LEV_SLE,  LOG_NORMAL	},
   { "sell",		do_sell,	POS_SITTING,	LEV_SLE,  LOG_NORMAL	},
   { "take",		do_get,		POS_SITTING,	LEV_SLE,  LOG_NORMAL	},
   { "read",		do_read,	POS_MEDITATING,	LEV_SLE,  LOG_NORMAL	},
   { "turn",		do_turn,	POS_MEDITATING,	LEV_SLE,  LOG_NORMAL	},
   { "twist",		do_activate,	POS_STANDING,	LEV_SLE,  LOG_NORMAL	},
   { "sacrifice",	do_sacrifice,	POS_SITTING,	LEV_SLE,  LOG_NORMAL	},
   { "unlock",		do_unlock,	POS_SITTING,	LEV_SLE,  LOG_NORMAL	},
   { "value",		do_value,	POS_SITTING,	LEV_SLE,  LOG_NORMAL	},
   { "wear",		do_wear,	POS_SITTING,	LEV_SLE,  LOG_NORMAL	},
   { "zap",		do_zap,		POS_SITTING,	LEV_SLE,  LOG_NORMAL	},
   
    /*
     * Combat commands.
     */
   { "autostance",	do_autostance,	POS_DEAD,	LEV_SLE,  LOG_NORMAL 	},
   { "berserk",		do_berserk,	POS_FIGHTING,	LEV_SLE,  LOG_NORMAL	},
   { "disarm",		do_disarm,	POS_FIGHTING,	LEV_SLE,  LOG_NORMAL	},
   { "flee",		do_flee,	POS_FIGHTING,	LEV_SLE,  LOG_NORMAL	},
   { "hurl",		do_hurl,	POS_STANDING,	LEV_SLE,  LOG_NORMAL	},
   { "kick",		do_kick,	POS_FIGHTING,	LEV_SLE,  LOG_NORMAL	},
   { "punch",		do_punch,	POS_STANDING,	LEV_SLE,  LOG_NORMAL	},
   { "redirect",	do_redirect,	POS_FIGHTING,	LEV_SLE,  LOG_NORMAL	},
   { "rescue",		do_rescue,	POS_FIGHTING,	LEV_SLE,  LOG_NORMAL	},
   { "stance",		do_stance,	POS_FIGHTING,	LEV_SLE,  LOG_NORMAL	},

    /*
     * Miscellaneous commands.
     */
   { "accep",		do_huh,		POS_STANDING,	 LEV_SLE,  LOG_NORMAL	},
   { "accept",		do_accept,	POS_STANDING,	 LEV_SLE,  LOG_NORMAL	},
   { "artifact",	do_artifact,	POS_STANDING,	 LEV_SLE,  LOG_NORMAL	},
   { "blindfold",	do_blindfold,	POS_STANDING,	 LEV_SLE,  LOG_ALWAYS	},
   { "breaku",		do_huh,		POS_STANDING,	 LEV_SLE,  LOG_NORMAL	},
   { "breakup",		do_breakup,	POS_STANDING,	 LEV_SLE,  LOG_NORMAL	},
   { "call",		do_call,	POS_FIGHTING,	 LEV_SLE,  LOG_NORMAL	},
   { "claim",		do_claim,	POS_STANDING,	 LEV_SLE,  LOG_NORMAL	},
   { "autoclass",	do_autoclass,	POS_FIGHTING,	LEV_AWA,  LOG_ALWAYS	},
   { "complete",	do_complete,	POS_RESTING,	LEV_SLE,  LOG_NORMAL	},
   { "demote",		do_demote,	POS_STANDING,	 LEV_SLE,  LOG_NORMAL 	},
   { "defences",	do_vampire,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "vamptalk",	do_vamptalk,	POS_DEAD,	 LEV_AWA,  LOG_NORMAL	},
   { "vtalk",		do_vamptalk,	POS_DEAD,	 LEV_AWA,  LOG_NORMAL	},
   { ">",		do_vamptalk,	POS_DEAD,	 LEV_AWA,  LOG_NORMAL	},
   { "vampire",		do_vampire,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "fist",		do_fistofcaine,	POS_FIGHTING,    LEV_AWA,  LOG_NORMAL	},
   { "finger",		do_finger,	POS_SITTING,	 LEV_SLE,  LOG_NORMAL	},
   { "eyesoftheserpent", do_eyes,	POS_FIGHTING,	 LEV_SLE,  LOG_NORMAL	},
   { "follow",		do_follow,	POS_SITTING,	 LEV_SLE,  LOG_NORMAL	},
   { "gag",		do_gag,		POS_STANDING,	LEV_SLE,  LOG_ALWAYS	},
   { "gift",		do_gift,	POS_STANDING,	 LEV_SLE,  LOG_NORMAL	},
   { "group",		do_group,	POS_DEAD,	 LEV_SLE,  LOG_NORMAL	},
   { "home",		do_home,	POS_STANDING,	 LEV_SLE,  LOG_NORMAL	},
   { "chome",		do_chome,	POS_STANDING,	 LEV_SLE,  LOG_NORMAL	},
   { "locate",		do_locate,	POS_SLEEPING,	 LEV_SLE,  LOG_NORMAL	},
   { "leaderboard",	do_leader,      POS_SLEEPING,    LEV_SLE,  LOG_NORMAL 	},
   { "propos",		do_huh,		POS_STANDING,	 LEV_SLE,  LOG_NORMAL	},
   { "propose",		do_propose,	POS_STANDING,	 LEV_SLE,  LOG_NORMAL	},
   { "promote",		do_promote,	POS_STANDING,	 LEV_SLE,  LOG_NORMAL 	}, 
   { "promotion",       do_promotion,   POS_STANDING,    LEV_SLE,  LOG_NORMAL   },
   { "failure",         do_failure,     POS_STANDING,    LEV_SLE,  LOG_NORMAL   },
   { "qui",		do_qui,		POS_DEAD,	 LEV_SLE,  LOG_NORMAL	},
   { "quit",		do_quit,	POS_SLEEPING,	 LEV_SLE,  LOG_NORMAL	},
   { "recall",		do_recall,      POS_FIGHTING,    LEV_SLE,  LOG_NORMAL 	},
   { "crecall",		do_crecall,	POS_FIGHTING,	 LEV_SLE,  LOG_NORMAL	},
   { "mark",		do_mark,	POS_FIGHTING,    LEV_SLE,  LOG_NORMAL	},
   { "move",		do_move,	POS_STANDING,	 LEV_SLE,  LOG_NORMAL   },
   { "escape",		do_escape,	POS_DEAD,	 LEV_SLE,  LOG_NORMAL	},
   { "/",		do_recall,	POS_FIGHTING,	 LEV_SLE,  LOG_NORMAL	},
   { "outcast",		do_outcast,	POS_STANDING,	 LEV_SLE,  LOG_NORMAL 	},
   { "recharge",	do_recharge,	POS_STANDING,	 LEV_SLE,  LOG_NORMAL	},
   { "safe",		do_safe,	POS_STANDING,	 LEV_SLE,  LOG_NORMAL	},
   { "save",		do_save,	POS_DEAD,	 LEV_SLE,  LOG_NORMAL	},
   { "sleep",		do_sleep,	POS_SLEEPING,	 LEV_SLE,  LOG_NORMAL	},
   { "smother",		do_smother,	POS_FIGHTING,	 LEV_SLE,  LOG_NORMAL	},
   { "sneak",		do_sneak,	POS_STANDING,	 LEV_SLE,  LOG_NORMAL	},
   { "scan",		do_scan,	POS_SITTING,	 LEV_SLE,  LOG_NORMAL	},
   { "summon",		do_summon,	POS_DEAD,	 LEV_SLE,  LOG_NORMAL	},
   { "tie",		do_tie,		POS_STANDING,	 LEV_SLE,  LOG_ALWAYS	},
   { "token",		do_token,	POS_STANDING,	 LEV_SLE,  LOG_NORMAL	},
   { "train",		do_train,	POS_SLEEPING,	 LEV_SLE,  LOG_NORMAL	},
   { "unpolymorph",	do_unpolymorph,	POS_STANDING,	 LEV_SOR,  LOG_NORMAL 	},
   { "untie",		do_untie,	POS_STANDING,	 LEV_SLE,  LOG_NORMAL	},
   { "visible",		do_visible,	POS_SLEEPING,	 LEV_SLE,  LOG_NORMAL 	},
   { "wake",		do_wake,	POS_SLEEPING,	 LEV_SLE,  LOG_NORMAL	},
   { "where",		do_where,	POS_SITTING,	 LEV_SLE,  LOG_NORMAL	},
   

    /*
     * Mage Stuffs.
     */
   { "arete",		 do_arete,	  POS_FIGHTING,    LEV_AWA, LOG_NORMAL   },
   { "beginnersluck",	 do_beginners_luck,   POS_STANDING, LEV_AWA, LOG_NORMAL  },
   { "breakdreamshell",  do_break_dreamshell, POS_STANDING, LEV_AWA, LOG_NORMAL  },
   { "brittlebones", 	 do_brittle_bones,   POS_FIGHTING, LEV_AWA, LOG_NORMAL  },
   { "contingency",	 do_contingency,     POS_FIGHTING, LEV_AWA, LOG_NORMAL },
   { "doublecast",	 do_double_cast,    POS_FIGHTING, LEV_AWA, LOG_NORMAL   },
   { "shatterwall", do_destroy_sculpture, POS_FIGHTING, LEV_AWA, LOG_NORMAL },
   { "electricalchaos",  do_electrical_chaos, POS_FIGHTING, LEV_AWA, LOG_NORMAL },
   { "friction",	 do_friction_curse, POS_FIGHTING,   LEV_AWA, LOG_NORMAL  },
   { "heartsblood",	 do_hearts_blood,  POS_STANDING,   LEV_AWA,  LOG_NORMAL },
   { "holysmote",	 do_holy_smote,    POS_FIGHTING,   LEV_AWA, LOG_NORMAL },
   { "inferno",		 do_inferno,	  POS_FIGHTING,	   LEV_AWA, LOG_NORMAL	},
   { "lifescan",	 do_readaura,	    POS_STANDING,  LEV_AWA, LOG_NORMAL	},
   { "mutateephemera",   do_mutate_ephemera, POS_FIGHTING, LEV_AWA, LOG_NORMAL  },
   { "polymorphother",   do_polymorph_other, POS_FIGHTING, LEV_AWA, LOG_NORMAL  },
   { "sculpture",	 do_sculpture,	   POS_FIGHTING,   LEV_AWA, LOG_NORMAL  },
   { "selectspheres",	 do_select_spheres, POS_DEAD,  LEV_AWA, LOG_NORMAL  },
   { "sphere",		do_spheres,	POS_SLEEPING,	LEV_AWA,  LOG_NORMAL	},
   { "alterweight",	do_alterweight,	POS_FIGHTING,	LEV_AWA,  LOG_NORMAL	},
   { "betterbody",	do_better_body,	POS_FIGHTING,	LEV_AWA,  LOG_NORMAL	},
   { "magearmor",	do_magearmor,	POS_STANDING,	LEV_AWA,  LOG_NORMAL	},
   { "mageenchant",	do_mageenchant,	POS_STANDING,	LEV_AWA,  LOG_NORMAL	},
   { "mageinvis",	do_mage_invis,	POS_STANDING,	LEV_AWA,  LOG_NORMAL	},
   { "mageheal",	do_mage_heal,	POS_FIGHTING,	LEV_AWA,  LOG_NORMAL	},
   { "rift",		do_rift,	POS_STANDING,	LEV_AWA,  LOG_NORMAL	},
   { "spiritcaress",     do_spirit_caress,  POS_STANDING, LEV_AWA,   LOG_NORMAL	},
   { "spiritsense",      do_spirit_sense,   POS_STANDING, LEV_AWA,   LOG_NORMAL },
   { "stepsideways",	 do_step_sideways, POS_STANDING, LEV_AWA, LOG_NORMAL   },
   { "ward",		 do_paradox_ward,   POS_STANDING,  LEV_AWA, LOG_NORMAL     },

    /*
     * Ninja commands.
     */
   { "bomuzite",	do_bomuzite,	POS_STANDING,	LEV_AWA,  LOG_NORMAL	},
   { "hakunetsu",	do_hakunetsu,	POS_STANDING,	LEV_AWA,  LOG_NORMAL	},
   { "harakiri",	do_harakiri,	POS_STANDING,	LEV_AWA,  LOG_NORMAL	},
   { "kakusu",		do_kakusu,	POS_STANDING,	LEV_AWA,  LOG_NORMAL	},
   { "michi",		do_michi,	POS_FIGHTING,	LEV_AWA,  LOG_NORMAL	},
   { "mienaku",		do_mienaku,	POS_FIGHTING,	LEV_AWA,  LOG_NORMAL	},
   { "mizotamouyo",	do_mizotamouyo,	POS_STANDING,	LEV_AWA,  LOG_NORMAL	},
   { "ninjaarmor",	do_ninjaarmor,	POS_STANDING,	LEV_AWA,  LOG_NORMAL	},
   { "orimasu",		do_orimasu,	POS_FIGHTING,	LEV_AWA,  LOG_NORMAL	}, 
   { "principle",	do_principles,	POS_SLEEPING,	LEV_AWA,  LOG_NORMAL	},
   { "shuurai",		do_shuurai,	POS_FIGHTING,	LEV_AWA,  LOG_NORMAL	},
   { "tsumazuku",	do_tsumazuku,	POS_FIGHTING,	LEV_AWA,  LOG_NORMAL	},
   { "tsume",		do_tsume,	POS_FIGHTING,	LEV_AWA,  LOG_NORMAL	},

    /*
     * Vampire and werewolf commands.
     */
   { "advance",		do_advance,	POS_STANDING,	LEV_AWA,  LOG_NORMAL 	},
   { "aeolus",		do_aeolus,	POS_STANDING,	LEV_AWA,  LOG_NORMAL	},
   { "armsabyss",	do_arms,	POS_FIGHTING,	 LEV_AWA,  LOG_NORMAL	},
   { "readaura",	do_readaura,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "awe",		do_awe,		POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "balorsgaze",	do_balor,	POS_FIGHTING,	 LEV_AWA,  LOG_NORMAL	},
   { "beastlife",	do_animal,      POS_STANDING,    LEV_AWA,  LOG_NORMAL 	},  
   { "beastwithin",	do_beastwithin,	POS_FIGHTING,	 LEV_AWA,  LOG_NORMAL	},
   { "beastform",       do_beastform,   POS_FIGHTING,    LEV_SLE,  LOG_NORMAL   },
   { "beckoning",	do_beckon,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "blissful",	do_blissful,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL 	},
   { "bloodagony",	do_bloodagony,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "bloodyfeast",	do_bloodyfeast,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "blur",		do_blur,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "bonecraft",	do_bonecraft,	POS_FIGHTING,	LEV_AWA,  LOG_NORMAL	},
   { "bodywrack",	do_bodywrack,	POS_FIGHTING,	 LEV_AWA,  LOG_NORMAL	},
   { "sbreath", 	do_sbreath,	POS_FIGHTING,	 LEV_AWA,  LOG_NORMAL   },
   { "breath",		do_dbreath,	POS_FIGHTING,	 LEV_AWA,  LOG_NORMAL	},
   { "breathwyld",	do_breathwyld,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "burrow",		do_burrow,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "calm",		do_calm,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "camouflage",	do_camouflage,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "celerity",	do_celerity,	POS_FIGHTING,	 LEV_AWA,  LOG_NORMAL	},
   { "change",		do_change,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "clandisc",	do_clandisc,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "clansphere",	do_clansphere,  POS_STANDING,	 LEV_AWA,  LOG_NORMAL   },
   { "clanname",	do_clanname,	POS_STANDING,	 LEV_IMM,  LOG_ALWAYS	},
   { "clapthunder",	do_clap,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "delete",		do_delete,	POS_STANDING,	 LEV_SLE,  LOG_NEVER	},
   { "claws",		do_claws,	POS_SITTING,	 LEV_AWA,  LOG_NORMAL	},
   { "cloneobject",	do_cloneobject,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "conditioning",	do_conditioning,POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "controlillusion",	do_control,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "controlbeast",	do_controlbeast,POS_MEDITATING,	 LEV_AWA,  LOG_NORMAL	},
   { "covenant",	do_covenant,	POS_DEAD,	 LEV_AWA,  LOG_NORMAL	},
   { "cutting",		do_cutting,	POS_FIGHTING,	 LEV_AWA,  LOG_NORMAL	},
   { "cvalk",		do_cvalk,	POS_DEAD,	 LEV_AWA,  LOG_NORMAL	},
   { "dazzle",		do_dazzle,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "decapitate",	do_decapitate,	POS_STANDING, 	 LEV_AWA,  LOG_ALWAYS	},
   { "destroy",		do_destroy,	POS_STANDING,	LEV_AWA,  LOG_ALWAYS	},
   { "diagnostics",	do_diagnostics,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL 	},
   { "discipline",	do_discipline,  POS_DEAD,        LEV_AWA,  LOG_NORMAL 	},
   { "dominator",	do_dominator,	POS_FIGHTING,	 LEV_AWA,  LOG_NORMAL	},
   { "doppleganger",	do_doppleganger,POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "humble",		do_humble,      POS_STANDING,    LEV_AWA,  LOG_NORMAL 	},
   { "dinduct",         do_dinduct,     POS_STANDING,    LEV_AWA,  LOG_NORMAL   },
   { "disquiet",	do_disquiet,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "dreadgaze",	do_dreadgaze,	POS_FIGHTING,	 LEV_AWA,  LOG_NORMAL	},
   { "earthmeld",	do_earthmeld,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "earthshift",	do_earthshift,  POS_STANDING,    LEV_SLE,  LOG_NORMAL 	},
   { "elemental",	do_elemental,	POS_FIGHTING,	 LEV_AWA,  LOG_NORMAL	},
   { "embrace",		do_embrace,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "entrancement",	do_entrancement,POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "faerielight",	do_faerielight,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "falling",		do_falling,	POS_FIGHTING,	 LEV_AWA,  LOG_NORMAL	},
   { "falseappearance",	do_rename,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "fangs",		do_fangs,	POS_SITTING,	 LEV_AWA,  LOG_NORMAL	},
   { "favour",		do_favour,	POS_SITTING,	 LEV_AWA,  LOG_NORMAL	},
   { "feed",		do_feed,	POS_SITTING,	 LEV_AWA,  LOG_NORMAL	},
   { "fetishdoll",	do_fetishdoll,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "fleshcraft",	do_fleshcraft,	POS_STANDING,	LEV_AWA,  LOG_NORMAL	},
   { "fleshbond",	do_fleshbond, 	POS_STANDING,	 LEV_AWA,  LOG_NORMAL   },
   { "flex",		do_flex,	POS_SITTING,	 LEV_AWA,  LOG_NORMAL	},
   { "fog",		do_aeolus,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "formillusion",	do_formillusion,POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "frenzytrigger",	do_frenzy,	POS_FIGHTING,	 LEV_AWA,  LOG_NORMAL	},
   { "friend",		do_friend,	POS_FIGHTING,	 LEV_AWA,  LOG_NORMAL	},
   { "geas",		do_geas,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "gifts",		do_gifts,	POS_DEAD,	 LEV_AWA,  LOG_NORMAL	},
   { "halosun",		do_halosun,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "heighten",	do_truesight,	POS_SITTING,	 LEV_AWA,  LOG_NORMAL	},
   { "helios",		do_helios,	POS_FIGHTING,	 LEV_AWA,  LOG_NORMAL	},
   { "howlbanshee",	do_banshee,	POS_FIGHTING,	 LEV_AWA,  LOG_NORMAL	},
   { "infest",		do_infest,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "infirmity",	do_infirmity,	POS_FIGHTING,	 LEV_AWA,  LOG_NORMAL	},
   { "invoke",		do_invoke,	POS_FIGHTING,	 LEV_AWA,  LOG_NORMAL	},
   { "jamtech", 	do_jamtech,     POS_FIGHTING,    LEV_AWA,  LOG_NORMAL 	},
   { "lamprey",		do_lamprey,	POS_FIGHTING,	 LEV_AWA,  LOG_NORMAL	},
   { "leech",		do_leechblood,	POS_FIGHTING,	 LEV_AWA,  LOG_NORMAL	},
   { "lobotomy",	do_lobotomy,	POS_FIGHTING,	 LEV_AWA,  LOG_NORMAL	},
   { "[",		do_fcommand,	POS_SITTING,	 LEV_AWA,  LOG_NORMAL	},
   { "malleable",	do_malleable,	POS_STANDING,	LEV_AWA,  LOG_NORMAL	},
   { "majesty",		do_majesty,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "mask",		do_mask,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "mindblock",	do_shield,	POS_SITTING,	 LEV_AWA,  LOG_NORMAL	},
   { "mindblast",	do_mindblast,	POS_FIGHTING,	 LEV_AWA,  LOG_NORMAL	},
   { "mirrorimage",	do_mirrorimage,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "mother",		do_mother,	POS_FIGHTING,	 LEV_AWA,  LOG_NORMAL	},
   { "mummify",		do_mummify,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "nightsight",	do_nightsight,	POS_SITTING,	 LEV_AWA,  LOG_NORMAL	},
   { "obedience",	do_obedience,	POS_SITTING,	 LEV_AWA,  LOG_ALWAYS	},
   { "omnipotence",	do_omnipotence,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "paralyzingstare",	do_paralyzing,	POS_FIGHTING,	 LEV_AWA,  LOG_NORMAL	},
   { "persuasion",	do_persuasion,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "pigeon",		do_pigeon,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "pkme",		do_pkme,	POS_DEAD,	 LEV_AWA,  LOG_ALWAYS	},
   { "plasmaform",	do_plasmaform,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "porcupine",	do_porcupine,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "possession",	do_possession,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "powers",		do_powers,	POS_DEAD,	 LEV_AWA,  LOG_NORMAL	},
   { "preserve",	do_preservation,POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "primalanger",	do_primalanger, POS_FIGHTING,    LEV_AWA,  LOG_NORMAL 	},
   { "rage",		do_rage,	POS_FIGHTING,	 LEV_AWA,  LOG_NORMAL	},
   { "regenerate",	do_regenerate,	POS_DEAD,	 LEV_AWA,  LOG_NORMAL	},
   { "relevel",		do_relevel,	POS_DEAD,	 LEV_SLE,  LOG_ALWAYS	},
   { "reshapeobject",	do_reshape,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "roleplay",        do_roleplay,    POS_DEAD,        LEV_AWA,  LOG_NORMAL   },
   { "scry",		do_scry,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "serenity",	do_serenity,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "serpent",		do_serpent,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "shadowbody",	do_shadowbody,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "shadowstep",	do_shadowstep,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "shield",		do_shield,	POS_SITTING,	 LEV_AWA,  LOG_NORMAL	},
   { "shroudnight",	do_shroudnight,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "silence",		do_silence,	POS_DEAD,	 LEV_IMM,  LOG_ALWAYS	},
   { "silencedeath",	do_silencedeath,POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "silenceofdeath",	do_silencedeath,POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "pulse",		do_pulse,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "songserenity",	do_serenity,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "songofrage",	do_songrage,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "soulmask",	do_soulmask,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "spiritgate",	do_spiritgate,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "spiritguardian",	do_spiritguardian,POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "spiritform",	do_spiritform,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "spiriteyes",	do_spiriteyes,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "spit",		do_spit,	POS_FIGHTING,	 LEV_AWA,  LOG_NORMAL	},
   { "step",		do_shadowstep,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "succulence",	do_succulence,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "summoning",	do_summoning,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "summonnetspider",	do_summonnet,   POS_STANDING,    LEV_AWA,  LOG_NORMAL 	},
   { "surge",		do_surge,	POS_FIGHTING,	 LEV_AWA,  LOG_NORMAL	},
   { "tastedeath",	do_spit,	POS_FIGHTING,	 LEV_AWA,  LOG_NORMAL	},
   { "teachdisc",	do_teachdisc,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "thanatopsis",	do_thanatopsis,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "tongue",		do_tongue,	POS_FIGHTING,	 LEV_AWA,  LOG_NORMAL	},
   { "transport",	do_transport,	POS_DEAD,	 LEV_SLE,  LOG_NORMAL	},
   { "tribegifts",	do_tribegifts,	POS_DEAD,	 LEV_AWA,  LOG_NORMAL	},
   { "truedisguise",	do_truedisguise,POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "umbra",		do_shadowplane,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "umbralvision",	do_shadowsight,	POS_SITTING,	 LEV_AWA,  LOG_NORMAL	},
   { "unveil",		do_unveil,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "upkeep",		do_upkeep,	POS_DEAD,	 LEV_AWA,  LOG_NORMAL	},
   { "vanish",		do_vanish,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "varmor",		do_vamparmor,   POS_DEAD,        LEV_AWA,  LOG_NORMAL 	},
   { "wwarmor",		do_wolfarmor,   POS_DEAD,        LEV_AWA,  LOG_NORMAL 	},
   { "wallgranite",	do_wallgranite,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "ward",		do_spiritward,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "weakness",	do_weakness,	POS_FIGHTING,	 LEV_AWA,  LOG_NORMAL	},
   { "web",		do_web,		POS_FIGHTING,	 LEV_AWA,  LOG_NORMAL	},
   { "whelp",		do_whelp,	POS_FIGHTING,	 LEV_AWA,  LOG_NORMAL 	},
   { "willpower",	do_willpower,	POS_DEAD,	 LEV_SLE,  LOG_NORMAL	},
   { "wwgifts",		do_gifts,	POS_DEAD,	 LEV_AWA,  LOG_NORMAL	},
   { "wwpowers",	do_wwpowers,	POS_DEAD,	 LEV_AWA,  LOG_NORMAL	},
   { "wyld",		do_wyld,	POS_DEAD,	 LEV_AWA,  LOG_NORMAL	},
   { "wyrmcall",	do_wyrmcall,	POS_FIGHTING,	 LEV_AWA,  LOG_NORMAL	},
   { "zombie",		do_zombie,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL	},
   { "zuloform",	do_zuloform,	POS_FIGHTING,	 LEV_AWA,  LOG_NORMAL	},
   { "chiroptean", 	do_chiroptean,	POS_FIGHTING,	 LEV_AWA,  LOG_NORMAL	},
   { "bridgewalk",	do_bridgewalk,	POS_STANDING,	 LEV_AWA,  LOG_NORMAL 	},
   { "grasp",		do_grasp,	POS_STANDING,    LEV_AWA,  LOG_NORMAL 	},
  
  /*
   * Monk Commands
   */
   
   { "decide",		do_decide,	POS_STANDING,	LEV_AWA,  LOG_NORMAL	},
   { "expel",		do_expel,	POS_STANDING,	LEV_AWA,  LOG_NORMAL	},
   { "mclan",		do_mclan,	POS_DEAD,	LEV_AWA,  LOG_NORMAL	},
   { "monklearn",	do_monklearn,	POS_DEAD,	LEV_AWA,  LOG_NORMAL	},
   { "monkarmor",	do_monkarmor,	POS_RESTING,	LEV_AWA,  LOG_NORMAL	},
   { "cripple",		do_cripple,	POS_FIGHTING,	LEV_AWA,  LOG_NORMAL	},
   { "repeat",		do_repeat,	POS_FIGHTING,	LEV_AWA,  LOG_NORMAL	},
   { "almighty",	do_almighty,	POS_STANDING,	LEV_AWA,  LOG_NORMAL	},
   { "lifecloak",	do_lifecloak,	POS_STANDING,	LEV_AWA,  LOG_NORMAL	},
   { "touch",		do_touch,	POS_FIGHTING,	LEV_AWA,  LOG_NORMAL	},
   { "aurashot",	do_aurashot,	POS_FIGHTING,	LEV_AWA,  LOG_NORMAL	},  
   { "blitz",		do_blitz,	POS_FIGHTING,	LEV_AWA,  LOG_NORMAL	},
   { "celestial",	do_celestial,	POS_STANDING,	LEV_AWA,  LOG_NORMAL	},
   { "darkblaze",	do_darkblaze,	POS_FIGHTING,	LEV_AWA,  LOG_NORMAL	},
   { "stunfist",	do_stunfist,	POS_FIGHTING,	LEV_AWA,  LOG_NORMAL	},   
   { "chimera",		do_chimera,	POS_FIGHTING,	LEV_AWA,  LOG_NORMAL	},
   { "invite",		do_invite,	POS_STANDING,	LEV_AWA,  LOG_NORMAL	},

  /*
   * Drow Commands
   */
   { "acidbolt",	do_acidbolt,	POS_FIGHTING,	LEV_AWA,  LOG_NORMAL	},
   { "attunement",	do_attune,	POS_STANDING,	LEV_AWA,  LOG_NORMAL	},
   { "darkvision",	do_nightsight,	POS_STANDING,	LEV_AWA,  LOG_NORMAL	},
   { "dimensiondoor",	do_door,	POS_STANDING,	LEV_AWA,  LOG_NORMAL	},
   { "drowarmor",	do_drowarmor,	POS_RESTING,	LEV_AWA,  LOG_NORMAL	},
   { "drowupkeep",	do_drowupkeep,	POS_DEAD,	LEV_AWA,  LOG_NORMAL	},
   { "faeriefire",	do_faeriefire,	POS_FIGHTING,	LEV_AWA,  LOG_NORMAL	},
   { "globedarkness",	do_globedark,	POS_FIGHTING,	LEV_AWA,  LOG_NORMAL	},
   { "prayer",		do_prayer,	POS_DEAD,	LEV_AWA,  LOG_NORMAL	},
   { "shoot",           do_shoot,       POS_FIGHTING,   LEV_AWA,  LOG_NORMAL    },
   { "sign",		do_sign,	POS_DEAD,	LEV_AWA,  LOG_NORMAL	},
   { "skipdisc",	do_skipdisc,	POS_STANDING,	LEV_AWA,  LOG_NORMAL	},
   { "summonspider",	do_summonspider,POS_STANDING,	LEV_AWA,  LOG_NORMAL	},

    /*
     * Elementalist commands.
     */
   { "flarearrow",      do_flarearrow,	POS_FIGHTING,	LEV_AWA,  LOG_NORMAL	},
   { "elemskill",	do_element,	POS_DEAD,	LEV_AWA,  LOG_NORMAL	},
   { "fireblade",	do_fireblade,	POS_FIGHTING,	LEV_AWA,  LOG_NORMAL	},
   { "phoenixform",     do_phoenixform,	POS_STANDING,	LEV_AWA,  LOG_NORMAL	},
   { "iciclestorm",     do_iciclestorm,	POS_FIGHTING,	LEV_AWA,  LOG_NORMAL	},
   { "aquabarrier",     do_aquabarrier,	POS_STANDING,	LEV_AWA,  LOG_NORMAL	},
   { "windwings",       do_windwings,	POS_STANDING,	LEV_AWA,  LOG_NORMAL	},
   { "watercontrol",    do_watercontrol,POS_FIGHTING,	LEV_AWA,  LOG_NORMAL	},
   { "windblades",      do_windblades,	POS_FIGHTING,	LEV_AWA,  LOG_NORMAL	},
   { "stoneblast",      do_stoneblast,	POS_FIGHTING,	LEV_AWA,  LOG_NORMAL	},
   { "localquake",      do_localquake,	POS_FIGHTING,	LEV_AWA,  LOG_NORMAL	},
   { "solarflash",      do_solarflash,	POS_FIGHTING,	LEV_AWA,  LOG_NORMAL	},
   { "holyaura",        do_holyaura,	POS_STANDING,	LEV_AWA,  LOG_NORMAL	},
   { "sunbath",         do_sunbath,	POS_FIGHTING,	LEV_AWA,  LOG_NORMAL	},
   { "solarlight",      do_solarlight,	POS_STANDING,	LEV_AWA,  LOG_NORMAL	},
   { "watertravel",     do_watertravel,	POS_STANDING,	LEV_AWA,  LOG_NORMAL	},
   { "mirageslice",     do_mirageslice,	POS_FIGHTING,	LEV_AWA,  LOG_NORMAL	},
   { "infinitesight",   do_infinitesight,POS_STANDING,	LEV_AWA,  LOG_NORMAL	},
   { "spiritspeak",     do_spiritspeak,	POS_DEAD,	LEV_AWA,  LOG_NORMAL	},
   { "ultimawave",      do_ultimawave,	POS_FIGHTING,	LEV_AWA,  LOG_NORMAL	},
   { "elemarmor",       do_elemarmor,	POS_STANDING,	LEV_AWA,  LOG_NORMAL	},
   { "cloakofsight",    do_cloak,	POS_STANDING,	LEV_AWA,  LOG_NORMAL	},
   { "atoningwinds",    do_atoningwinds,POS_FIGHTING,	LEV_AWA,  LOG_NORMAL	},
   { "spiralmoon",      do_spiralmoon,	POS_FIGHTING,	LEV_AWA,  LOG_NORMAL	},
   { "slow",            do_slow,	POS_FIGHTING,	LEV_AWA,  LOG_NORMAL	},
   { "liquifyearth",    do_liquifyearth,POS_FIGHTING,	LEV_AWA,  LOG_NORMAL	},
   { "farsight",        do_farsight,	POS_STANDING,	LEV_AWA,  LOG_NORMAL	},
   { "nightblade",      do_nightblade,	POS_FIGHTING,	LEV_AWA,  LOG_NORMAL	},

    /*
     * Sorcerer commands.
     */
   { "chant",		do_chant,	POS_FIGHTING,	LEV_SOR,  LOG_NORMAL	},
   { "colors",		do_colors,	POS_DEAD,	LEV_SOR,  LOG_NORMAL	},
   { "enchant",		do_enchant,	POS_RESTING,	LEV_SOR,  LOG_NORMAL	},
   { "sorccreate",	do_sorccreate,	POS_RESTING,	LEV_SOR,  LOG_NORMAL	},
   { "vision",		do_vision,	POS_RESTING,	LEV_SOR,  LOG_NORMAL	},
   { "write",		do_write,	POS_RESTING,	LEV_SLE,  LOG_NORMAL	},

   { "clearstats",	do_clearstats,	POS_STANDING,	 LEV_SLE,  LOG_NORMAL	},
   { "honor",		do_honor,	POS_DEAD,	LEV_SLE,  LOG_NORMAL	},

   
    /*
     * Immortal commands.
     */
   { "nuke",		do_nuke,	POS_DEAD,	LEV_ST,   LOG_ALWAYS	},
   { "superat",		do_for,		POS_DEAD,	LEV_ST,   LOG_ALWAYS	},

   { "allow",		do_allow,	POS_DEAD,	LEV_IMP,  LOG_ALWAYS	},
   { "anull",		do_anull,	POS_DEAD,	LEV_IMP,  LOG_NORMAL	},
   { "ban",		do_ban,		POS_DEAD,	LEV_IMP,  LOG_ALWAYS	},
   { "cdest",		do_cdest,	POS_DEAD,	LEV_IMP,  LOG_ALWAYS	},
   { "clearclas",	do_huh,		POS_DEAD,	LEV_IMP,  LOG_NORMAL	},
   { "clearclass",	do_clearclass,	POS_DEAD,	LEV_IMP,  LOG_ALWAYS	},
   { "copyover",	do_copyover,	POS_DEAD,	LEV_IMP,  LOG_ALWAYS	},
   { "delevel",		do_delevel,     POS_DEAD,       LEV_IMP,  LOG_ALWAYS	},
   { "disable",		do_disable,	POS_DEAD,	LEV_IMP,  LOG_ALWAYS	},
   { "exlist",		do_exlist,      POS_DEAD,       LEV_IMP,  LOG_NORMAL	},
   { "familiar",	do_familiar,	POS_DEAD,	LEV_IMP,  LOG_NORMAL	},
   { "godstrike",	do_godstrike,   POS_DEAD,       LEV_IMP,  LOG_ALWAYS	},
   { "oclone",		do_oclone,	POS_DEAD,	LEV_IMP,  LOG_ALWAYS	},
   { "pload",		do_pload,	POS_DEAD,	LEV_IMP,  LOG_ALWAYS	},
   { "prepfor",		do_prepfor,	POS_DEAD,	LEV_IMP,  LOG_ALWAYS	},
   { "shutdow",		do_huh,		POS_DEAD,	LEV_IMP,  LOG_NORMAL	},
   { "shutdown",	do_shutdown,	POS_DEAD,	LEV_IMP,  LOG_ALWAYS	},
   { "sla",		do_huh,		POS_DEAD,	LEV_IMP,  LOG_NORMAL	},
   { "slay",		do_slay,	POS_DEAD,	LEV_IMP,  LOG_ALWAYS	},
   { "vlist",		do_vlist,       POS_DEAD,       LEV_IMP,  LOG_NORMAL	},
   { "vopen",		do_vopen,       POS_DEAD,       LEV_IMP,  LOG_NORMAL	},
   { "wizlock",		do_wizlock,	POS_DEAD,	LEV_IMP,  LOG_ALWAYS	},

   { "clearreply",	do_clearreply,	POS_DEAD,	LEV_IMM,  LOG_NORMAL	},
   { "deny",		do_deny,	POS_DEAD,	LEV_IMM,  LOG_ALWAYS	},
   { "disconnect",	do_disconnect,	POS_DEAD,	LEV_IMM,  LOG_ALWAYS	},
   { "divorce",		do_divorce,	POS_DEAD,	LEV_IMM,  LOG_ALWAYS	},
   { "echo",		do_echo,	POS_DEAD,	LEV_IMM,  LOG_ALWAYS	},
   { "expmod",		do_expmod,	POS_DEAD,	LEV_IMM,  LOG_ALWAYS	},
   { "freeze",		do_freeze,	POS_DEAD,	LEV_IMM,  LOG_ALWAYS	},
   { "force",		do_force,	POS_DEAD,	LEV_IMM,  LOG_ALWAYS	},
   { "generation",	do_generation,	POS_DEAD,	LEV_IMM,  LOG_ALWAYS	},
   { "immtalk",		do_immtalk,	POS_DEAD,	LEV_IMM,  LOG_NORMAL	},
   { "invis",		do_invis,	POS_DEAD,	LEV_IMM,  LOG_NORMAL	},
   { "incog",		do_incog,	POS_DEAD,	LEV_IMM,  LOG_NORMAL	},
   { "leaderclear",	do_leaderclear,	POS_DEAD,	LEV_IMM,  LOG_ALWAYS	},
   { "log",		do_log,		POS_DEAD,	LEV_IMM,  LOG_ALWAYS	},
   { "marry",		do_marry,	POS_DEAD,	LEV_IMM,  LOG_ALWAYS	},
   { "mclear",		do_mclear,	POS_DEAD,	LEV_IMM,  LOG_ALWAYS	},
   { "memory",		do_memory,	POS_DEAD,	LEV_IMM,  LOG_NORMAL	},
   { "mset",		do_mset,	POS_DEAD,	LEV_IMM,  LOG_ALWAYS	},
   { "mwhere",		do_mwhere,	POS_DEAD,	LEV_IMM,  LOG_NORMAL	},
   { "newbielock",	do_newbielock,	POS_DEAD,	LEV_IMM,  LOG_ALWAYS	},
   { "notell",		do_notell,	POS_DEAD,	LEV_IMM,  LOG_NORMAL	},
   { "oreturn",		do_oreturn,	POS_DEAD,	LEV_IMM,  LOG_NORMAL	},
   { "oset",		do_oset,	POS_DEAD,	LEV_IMM,  LOG_ALWAYS	},
   { "oswitch",		do_oswitch,	POS_DEAD,	LEV_IMM,  LOG_NORMAL	},
   { "otransfer",	do_otransfer,	POS_DEAD,	LEV_IMM,  LOG_ALWAYS	},
   { "paradox",		do_wizdox,	POS_DEAD,	LEV_IMM,  LOG_ALWAYS	},
   { "peace",		do_peace,	POS_DEAD,	LEV_IMM,  LOG_NORMAL	},
   { "pset",		do_pset,	POS_DEAD,	LEV_IMM,  LOG_ALWAYS	},
   { "qmake",		do_qmake,	POS_DEAD,	LEV_IMM,  LOG_ALWAYS	},
   { "qset",		do_qset,	POS_DEAD,	LEV_IMM,  LOG_ALWAYS	},
   { "qstat",		do_qstat,	POS_DEAD,	LEV_IMM,  LOG_ALWAYS	},
   { "randomload",	do_randomload,	POS_DEAD,	LEV_IMM,  LOG_ALWAYS	},
   { "recho",		do_recho,	POS_DEAD,	LEV_IMM,  LOG_ALWAYS	},
   { "release",		do_release,	POS_DEAD,	LEV_IMM,  LOG_ALWAYS	},
   { "rename",		do_pcrename,	POS_DEAD,	LEV_IMM,  LOG_ALWAYS	},
   { "return",		do_return,	POS_DEAD,	LEV_IMM,  LOG_NORMAL	},
   { "restore",		do_restore,	POS_DEAD,	LEV_IMM,  LOG_ALWAYS	},
   { "rset",		do_rset,	POS_DEAD,	LEV_IMM,  LOG_ALWAYS	},
   { "setclandisc",	do_setclandisc,	POS_DEAD,	LEV_IMM,  LOG_ALWAYS	},
   { "setclansphere",	do_setclansphere, POS_DEAD,	LEV_IMM,  LOG_ALWAYS	},
   { "setdisc",		do_setdisc,	POS_DEAD,	LEV_IMM,  LOG_ALWAYS	},
   { "setsphere",       do_setsphere,	POS_DEAD,	LEV_IMM,  LOG_ALWAYS	},
   { "setgift",		do_setgift,	POS_DEAD,	LEV_IMM,  LOG_ALWAYS	},
   { "setflag",		do_setflag,	POS_DEAD,	LEV_IMM,  LOG_ALWAYS	},
   { "slookup",		do_slookup,	POS_DEAD,	LEV_IMM,  LOG_NORMAL	},
   { "snoop",		do_snoop,	POS_DEAD,	LEV_IMM,  LOG_NORMAL	},
   { "switch",		do_switch,	POS_DEAD,	LEV_IMM,  LOG_ALWAYS	},
   { "users",		do_users,	POS_DEAD,	LEV_IMM,  LOG_NORMAL	},
   { "worldaff",	do_worldaff,	POS_DEAD,	LEV_IMM,  LOG_NORMAL	},

   { "bamfin",          do_bamfin,	POS_DEAD,	LEV_BUI,  LOG_NORMAL	},
   { "bamfout",		do_bamfout,	POS_DEAD,	LEV_BUI,  LOG_NORMAL	},
   { "goto",		do_goto,	POS_DEAD,	LEV_BUI,  LOG_NORMAL	},
   { "mfind",		do_mfind,	POS_DEAD,	LEV_BUI,  LOG_NORMAL	},
   { "mload",		do_mload,	POS_DEAD,	LEV_BUI,  LOG_ALWAYS	},
   { "mstat",		do_mstat,	POS_DEAD,	LEV_BUI,  LOG_NORMAL	},
   { "ofind",		do_ofind,	POS_DEAD,	LEV_BUI,  LOG_NORMAL	},
   { "oload",		do_oload,	POS_DEAD,	LEV_BUI,  LOG_ALWAYS	},
   { "ostat",		do_ostat,	POS_DEAD,	LEV_BUI,  LOG_NORMAL	},
   { "purge",		do_purge,	POS_DEAD,	LEV_BUI,  LOG_NORMAL	},
   { "tick",		do_tick,	POS_DEAD,	LEV_BUI,  LOG_NORMAL	},
   { "transfer",	do_transfer,	POS_DEAD,	LEV_BUI,  LOG_ALWAYS	},
   { "wizhelp",		do_wizhelp,	POS_DEAD,	LEV_BUI,  LOG_NORMAL	},

    /*
     * Online creation commands.
     */
   { "aedit",		do_aedit,	POS_DEAD,	LEV_IMP,  LOG_NORMAL	},
   { "redit",		do_redit,	POS_DEAD,	LEV_BUI,  LOG_NORMAL 	},
   { "oedit",		do_oedit,	POS_DEAD,	LEV_BUI,  LOG_NORMAL 	},
   { "medit",		do_medit,	POS_DEAD,	LEV_BUI,  LOG_NORMAL 	},
   
   { "ashow",		do_ashow,	POS_DEAD,	LEV_BUI,  LOG_NORMAL 	},
   { "rshow",		do_rshow,	POS_DEAD,	LEV_BUI,  LOG_NORMAL 	},
   { "oshow",		do_oshow,	POS_DEAD,	LEV_BUI,  LOG_NORMAL 	},
   { "mshow",		do_mshow,	POS_DEAD,	LEV_BUI,  LOG_NORMAL 	},

   { "resets",		do_resets,      POS_DEAD,	LEV_BUI,  LOG_NORMAL 	},
   { "asave",		do_asave,	POS_DEAD,	LEV_BUI,  LOG_NORMAL 	},
   { "alist",		do_arealist,	POS_DEAD,	LEV_BUI,  LOG_NORMAL 	},
   
   { "meditate",	do_meditate,    POS_MEDITATING, LEV_SLE,  LOG_NORMAL	},
    
    /*
     * End of list.
     */
   { "",		0,		POS_DEAD,	 LEV_SLE,  LOG_NORMAL	}
   };
  



/*
 * The social table.
 * Add new socials here.
 * Alphabetical order is not required.
 */
   const	struct	social_type	social_table [] =
   {
   {
   "accuse",
   "Accuse whom?",
   "$n is in an accusing mood.",
   "You look accusingly at $M.",
   "$n looks accusingly at $N.",
   "$n looks accusingly at you.",
   "You accuse yourself.",
   "$n seems to have a bad conscience."
   },
   
   {
   "agree",
   "You seem to be in an agreeable mood.",
   "$n seems to agree.",
   "You agree with $M.",
   "$n agrees with $N.",
   "$n agrees with you.",
   "Well I hope you would agree with yourself!",
   "$n agrees with $mself, of course."
   },
   
   {
   "airguitar",
   "You sizzle the air with your BITCHIN' guitar playing!",
   "$n air-guitars like Jimi Hendrix!",
   "You sizzle the air with your BITCHIN' guitar playing!",
   "$n air-guitars like Jimi Hendrix!",
   "$n air-guitars like Jimi Hendrix, just for you!",
   "You forget all else as you sizzle the air with your BITCHIN' guitar playing!",
   "$n forgets all else as $e air-guitars like Jimi Hendrix."
   },
   
   {
   "apologize",
   "You apologize for your behavior.",
   "$n apologizes for $s rude behavior.",
   "You apologize to $M.",
   "$n apologizes to $N.",
   "$n apologizes to you.",
   "You apologize to yourself.",
   "$n apologizes to $mself.  Hmmmm."
   },
   
   {
   "applaud",
   "Clap, clap, clap.",
   "$n gives a round of applause.",
   "You clap at $S actions.",
   "$n claps at $N's actions.",
   "$n gives you a round of applause.  You MUST'VE done something good!",
   "You applaud at yourself.  Boy, are we conceited!",
   "$n applauds at $mself.  Boy, are we conceited!"
   },
   
   {
   "bark",
   "Woof!  Woof!",
   "$n barks like a dog.",
   "You bark at $M.",
   "$n barks at $N.",
   "$n barks at you.",
   "You bark at yourself.  Woof!  Woof!",
   "$n barks at $mself.  Woof!  Woof!"
   },
   
   {
   "bearhug",
   "You hug a grizzly bear.",
   "$n hugs a flea-infested grizzly bear.",
   "You bearhug $M.",
   "$n bearhugs $N.  Some ribs break.",
   "$n bearhugs you.  Wonder what's coming next?",
   "You bearhug yourself.",
   "$n bearhugs $mself."
   },
   
   {
   "beer",
   "You down a cold, frosty beer.",
   "$n downs a cold, frosty beer.",
   "You draw a cold, frosty beer for $N.",
   "$n draws a cold, frosty beer for $N.",
   "$n draws a cold, frosty beer for you.",
   "You draw yourself a beer.",
   "$n draws $mself a beer."
   },
   
   {
   "beg",
   "You beg the gods for mercy.",
   "The gods fall down laughing at $n's request for mercy.",
   "You desperately try to squeeze a few coins from $M.",
   "$n begs you for money.",
   "$n begs $N for a gold piece!",
   "Begging yourself for money doesn't help.",
   "$n begs $mself for money."
   },
   
   {
   "bkiss",
   "Blow a kiss to whom?",
   "$n blows at $s hand.",
   "You blow a kiss to $M.",
   "$n blows a kiss to $N.  Touching, ain't it?",
   "$n blows a kiss to you.  Not as good as a real one, huh?",
   "You blow a kiss to yourself.",
   "$n blows a kiss to $mself.  Wierd."
   },
   
   {
   "bleed",
   "You bleed all over the room!",
   "$n bleeds all over the room!  Get out of $s way!",
   "You bleed all over $M!",
   "$n bleeds all over $N.  Better leave, you may be next!",
   "$n bleeds all over you!  YUCK!",
   "You bleed all over yourself!",
   "$n bleeds all over $mself."
   },
   
   {
   "blink",
   "You blink in utter disbelief.",
   "$n blinks in utter disbelief.",
   "You blink at $M in confusion.",
   "$n blinks at $N in confusion.",
   "$n blinks at you in confusion.",
   "You are sooooooooooooo confused",
   "$n blinks at $mself in complete confusion."
   },
   
   {
   "blush",
   "Your cheeks are burning.",
   "$n blushes.",
   "You get all flustered up seeing $M.",
   "$n blushes as $e sees $N here.",
   "$n blushes as $e sees you here.  Such an effect on people!",
   "You blush at your own folly.",
   "$n blushes as $e notices $s boo-boo."
   },
   
   {
   "boggle",
   "You boggle at all the loonies around you.",
   "$n boggles at all the loonies around $m.",
   "You boggle at $S ludicrous idea.",
   "$n boggles at $N's ludicrous idea.",
   "$n boggles at your ludicrous idea.",
   "BOGGLE.",
   "$n wonders what BOGGLE means."
   },
   
   {
   "bonk",
   "BONK.",
   "$n spells 'potato' like Dan Quayle: 'B-O-N-K'.",
   "You bonk $M for being a numbskull.",
   "$n bonks $N.  What a numbskull.",
   "$n bonks you.  BONK BONK BONK!",
   "You bonk yourself.",
   "$n bonks $mself."
   },
   
   {
   "bounce",
   "BOIINNNNNNGG!",
   "$n bounces around.",
   "You bounce onto $S lap.",
   "$n bounces onto $N's lap.",
   "$n bounces onto your lap.",
   "You bounce your head like a basketball.",
   "$n plays basketball with $s head."
   },
   
   {
   "bow",
   "You bow deeply.",
   "$n bows deeply.",
   "You bow before $M.",
   "$n bows before $N.",
   "$n bows before you.",
   "You kiss your toes.",
   "$n folds up like a jack knife and kisses $s own toes."
   },
   
   {
   "brb",
   "You announce that you will be right back.",
   "$n says in a stern voice, 'I'll be back!'",
   "You announce to $M that you will be right back.",
   "$n says to $N in a stern voice, 'I'll be back!'",
   "$n says to you in a stern voice, 'I'll be right back!'",
   "You mumble to yourself, 'I'll be right back'",
   "$n mumbles to $mself, 'I'll be right back, won't I?'"
   },
   
   {
   "brush",
   "Brush what? Who? Where?",
   NULL,
   "You brush out $S hair for $M.  Very thoughtful.",
   "$n brushes $N's hair for $M.  Looks better now.",
   "$n brushes out your hair.  How nice of $m.",
   "You brush out your hair.  There - much better.",
   "$n brushes out $s hair.  Looks much better now."
   },
   
   {
   "burp",
   "You burp loudly.",
   "$n burps loudly.",
   "You burp loudly to $M in response.",
   "$n burps loudly in response to $N's remark.",
   "$n burps loudly in response to your remark.",
   "You burp at yourself.",
   "$n burps at $mself.  What a sick sight."
   },
   
   {
   "bye",
   "You say goodbye to all in the room.",
   "$n says goodbye to everyone in the room.",
   "You say goodbye to $N.",
   "$n says goodbye to $N.",
   "$n says goodbye to you.",
   "You say goodbye to yourself.  Contemplating suicide?",
   "$n says goodbye to $mself.  Is $e contemplating suicide?"
   },
   
   {
   "cackle",
   "You throw back your head and cackle with insane glee!",
   "$n throws back $s head and cackles with insane glee!",
   "You cackle gleefully at $N",
   "$n cackles gleefully at $N.",
   "$n cackles gleefully at you.  Better keep your distance from $m.",
   "You cackle at yourself.  Now, THAT'S strange!",
   "$n is really crazy now!  $e cackles at $mself."
   },

   {
   "caress",
   "Soft?",
   "$n touches $s arms",
   "Doesn't $E have the softest skin?",
   "$n gently caresses $N.",
   "$n slides $s hand gently over your skin.",
   "You rub your arms softly.",
   "$n rubs $s arms.  Cold?"
   },
   
   {
   "catnap",
   "You curl into a tiny ball and go to sleep.",
   "$n curls $mself into a tiny ball and goes to sleep.",
   "You curl up in $S lap and go to sleep.",
   "$n curls up in $N's lap and goes to sleep.",
   "$n curls up in your lap and goes to sleep.",
   "You curl into a tiny ball and go to sleep.",
   "$n curls $mself into a tiny ball and goes to sleep."
   },
   
   {
   "cheer",
   "Woohoo!",
   "$n cheers 'Woohoo!",
   "You cheer loudly: 'Go $N Go!'",
   "$n cheers loudly: 'Go $N Go!'",
   "$n cheers you on!",
   "You cheer yourself up.",
   "$n cheers $mself on."
   },
   
   {
   "chortle",
   "You chortle with glee.",
   "$n chortles with glee.",
   "You chortle loudly at $M.",
   "$n chortles loudly at $N.",
   "$n chortles loudly at you.",
   "You chortle loudly to yourself.",
   "$n chortles loudly to $mself."
   },
   
   {
   "chuckle",
   "You chuckle politely.",
   "$n chuckles politely.",
   "You chuckle at $S joke.",
   "$n chuckles at $N's joke.",
   "$n chuckles at your joke.",
   "You chuckle at your own joke, since no one else would.",
   "$n chuckles at $s own joke, since none of you would."
   },
   
   {
   "claphands",
   "You clap your hands together.",
   "$n shows $s approval by clapping $s hands together.",
   "You clap at $S performance.",
   "$n claps at $N's performance.",
   "$n claps at your performance.",
   "You clap at your own performance.",
   "$n claps at $s own performance."
   },
   
   {
   "comb",
   "You comb your hair - perfect.",
   "$n combs $s hair, how dashing!",
   "You patiently untangle $N's hair - what a mess!",
   "$n tries patiently to untangle $N's hair.",
   "$n pulls your hair in an attempt to comb it.",
   "You pull your hair, but it will not be combed.",
   "$n tries to comb $s tangled hair."
   },
   
   {
   "comfort",
   "Do you feel uncomfortable?",
   NULL,
   "You comfort $M.",
   "$n comforts $N.",
   "$n comforts you.",
   "You make a vain attempt to comfort yourself.",
   "$n has no one to comfort $m but $mself."
   },
   
   {
   "cough",
   "You cough to clear your throat.",
   "$n coughs loudly.",
   "You cough loudly.  It must be $S fault, $E gave you this cold.",
   "$n coughs loudly, and glares at $N, like it is $S fault.",
   "$n coughs loudly, and glares at you.  Did you give $m that cold?",
   "You cough loudly.  Why don't you take better care of yourself?",
   "$n coughs loudly.  $n should take better care of $mself." 
   },
   
   { 
   "cower",
   "What are you afraid of?",
   "$n cowers in the corner from claustrophobia.",
   "You cower in the corner at the sight of $M.",
   "$n cowers in the corner at the sight of $N.",
   "$n cowers in the corner at the sight of you.",
   "You cower in the corner at the thought of yourself.  You scaredy cat!",
   "$n cowers in the corner.  What is wrong with $m now?"  
   },
   
   {
   "cringe",
   "You cringe in terror.",
   "$n cringes in terror!",
   "You cringe away from $M.",
   "$n cringes away from $N in mortal terror.",
   "$n cringes away from you.",
   "I beg your pardon?",
   NULL   
   },
   
   {
   "cry",
   "Waaaaah ...",
   "$n bursts into tears.",
   "You cry on $S shoulder.",
   "$n cries on $N's shoulder.",
   "$n cries on your shoulder.",
   "You cry to yourself.",
   "$n sobs quietly to $mself."
   },
   
   {
   "cuddle",
   "Whom do you feel like cuddling today?",
   NULL,
   "You cuddle $M.",
   "$n cuddles $N.",
   "$n cuddles you.",
   "You must feel very cuddly indeed ... :)",
   "$n cuddles up to $s shadow.  What a sorry sight."
   },
   
   {
   "curse",
   "You swear loudly for a long time.",
   "$n swears: @*&^%@*&!",
   "You swear at $M.",
   "$n swears at $N.",
   "$n swears at you!  Where are $s manners?",
   "You swear at your own mistakes.",
   "$n starts swearing at $mself.  Why don't you help?"
   },
   
   {
   "curtsey",
   "You curtsey to your audience.",
   "$n curtseys gracefully.",
   "You curtsey to $M.",
   "$n curtseys gracefully to $N.",
   "$n curtseys gracefully for you.",
   "You curtsey to your audience (yourself).",
   "$n curtseys to $mself, since no one is paying attention to $m."
   },
   
   {
   "dance",
   "Feels silly, doesn't it?",
   "$n tries to break dance, but nearly breaks $s neck!",
   "You sweep $M into a romantic waltz.",
   "$n sweeps $N into a romantic waltz.",
   "$n sweeps you into a romantic waltz.",
   "You skip and dance around by yourself.",
   "$n dances a pas-de-une."
   },

   {
   "dimples",
   "Cute smile you have there.",
   "$n shows off $s dimples.",
   "You show $N your dimples.",
   "$n shows you $s dimples.",
   "$n shows $N $s dimples.",
   "You cover your dimples with your hands, laughing.",
   "$n laughs trying to cover $s dimples."
   },
   
   {
   "dive",
   "You dive into the ocean.",
   "$n dives into the ocean.",
   "You dive behind $M and hide.",
   "$n dives behind $N and hides.",
   "$n dives behind you and hides.",
   "You take a dive.",
   "$n takes a dive."
   },
   
   {
   "dizzy",
   "You are so dizzy from all this chatter.",
   "$n spins twice and hits the ground, dizzy from all this chatter.",
   "You are dizzy from all of $N's chatter.",
   "$n spins twice and hits the ground, dizzy from all $N's chatter.",
   "$n spins twice and hits the ground, dizzy from all your chatter.",
   "You are dizzy from lack of air.  Don't talk so much!",
   "$n spins twice and falls to the ground from lack of air."
   },
   
   {
   "drool",
   "You drool on yourself.",
   "$n drools on $mself.",
   "You drool all over $N.",
   "$n drools all over $N.",
   "$n drools all over you.",
   "You drool on yourself.",
   "$n drools on $mself."
   },
   
   {
   "eskimo",
   "Who do you want to eskimo kiss with?",
   "$n is looking for someone to rub noses with.  Any volunteers?",
   "You rub noses with $M.",
   "$n rubs noses with $N.  You didn't know they were eskimos!",
   "$n rubs noses with you.",
   "You ponder the difficulties involved in rubbing noses with yourself.",
   "$n ponders the difficulties involved in rubbing noses with $mself."
   },
   
   {
   "eyebrow",
   "You raise an eyebrow.",
   "$n raises an eyebrow.",
   "You raise an eyebrow at $M.",
   "$n raises an eyebrow at $N.",
   "$n raises an eyebrow at you.",
   "You raise an eyebrow at yourself.  That hurt!",
   "$n raises an eyebrow at $mself.  That must have hurt!"
   },
   
   {
   "faint",
   "You feel dizzy and hit the ground like a board.",
   "$n's eyes roll back in $s head and $e crumples to the ground.",
   "You faint into $S arms.",
   "$n faints into $N's arms.",
   "$n faints into your arms.  How romantic.",
   "You look down at your condition and faint.",
   "$n looks down at $s condition and faints dead away."
   },
   
   {
   "flash",
   "You flash your naked body at the gawking crowd.",
   "$n flashes $s naked body at everyone.  Gasp!",
   "You flash your naked body at $M.",
   "$n flashes $s naked body at $N.  Aren't you jealous?",
   "$n flashes $s naked body at you.  Everyone else is jealous.",
   "You flash your naked body at yourself.  How strange.",
   "$n is looking down $s shirt and grinning.  Very wierd!"
   },
   
   {
   "flip",
   "You flip head over heels.",
   "$n flips head over heels.",
   "You flip $M over your shoulder.",
   "$n flips $N over $s shoulder.",
   "$n flips you over $s shoulder.  Hmmmm.",
   "You tumble all over the room.",
   "$n does some nice tumbling and gymnastics."
   },
   
   {
   "flirt",
   "Wink wink!",
   "$n flirts -- probably needs a date, huh?",
   "You flirt with $M.",
   "$n flirts with $N.",
   "$n wants you to show some interest and is flirting with you.",
   "You flirt with yourself.",
   "$n flirts with $mself.  Hoo boy."
   },
   
   {
   "flutter",
   "You flutter your eyelashes.",
   "$n flutters $s eyelashes.",
   "You flutter your eyelashes at $M.",
   "$n flutters $s eyelashes in $N's direction.",
   "$n looks at you and flutters $s eyelashes.",
   "You flutter your eyelashes at the thought of yourself.",
   "$n flutters $s eyelashes at no one in particular." 
   },
   
   {
   "fondle",
   "Who needs to be fondled?",
   NULL,
   "You fondly fondle $M.",
   "$n fondly fondles $N.",
   "$n fondly fondles you.",
   "You fondly fondle yourself, feels funny doesn't it?",
   "$n fondly fondles $mself - this is going too far!!"
   },
   
   {
   "french",
   "Kiss whom?",
   NULL,
   "You give $N a long and passionate kiss.",
   "$n kisses $N passionately.",
   "$n gives you a long and passionate kiss.",
   "You gather yourself in your arms and try to kiss yourself.",
   "$n makes an attempt at kissing $mself."
   },
   
   {
   "frown",
   "What's bothering you ?",
   "$n frowns.",
   "You frown at what $E did.",
   "$n frowns at what $E did.",
   "$n frowns at what you did.",
   "You frown at yourself.  Poor baby.",
   "$n frowns at $mself.  Poor baby."
   },
   
   {
   "fume",
   "You grit your teeth and fume with rage.",
   "$n grits $s teeth and fumes with rage.",
   "You stare at $M, fuming.",
   "$n stares at $N, fuming with rage.",
   "$n stares at you, fuming with rage!",
   "That's right - hate yourself!",
   "$n clenches $s fists and stomps his feet, fuming with anger."
   },
   
   {
   "garth",
   "You will give your weapons away....NOT.",
   "$n yells 'WAYNE'S WORLD WAYNE'S WORLD -- PARTY TIME!  EXCELLENT!'",
   "You yell 'WAYNE'S WORLD WAYNE'S WORLD -- PARTY TIME!  EXCELLENT!'",
   "$n yells 'WAYNE'S WORLD WAYNE'S WORLD -- PARTY TIME!  EXCELLENT!'",
   "$n yells 'WAYNE'S WORLD WAYNE'S WORLD -- PARTY TIME!  EXCELLENT!'",
   "You yell 'WAYNE'S WORLD WAYNE'S WORLD -- PARTY TIME!  EXCELLENT!'",
   "$n yells 'WAYNE'S WORLD WAYNE'S WORLD -- PARTY TIME!  EXCELLENT!'"
   },
   
   {
   "gasp",
   "You gasp in astonishment.",
   "$n gasps in astonishment.",
   "You gasp as you realize what $E did.",
   "$n gasps as $e realizes what $N did.",
   "$n gasps as $e realizes what you did.",
   "You look at yourself and gasp!",
   "$n takes one look at $mself and gasps in astonisment!"
   },
   
   {
   "gawk",
   "You gawk at everyone around you.",
   "$n gawks at everyone in the room.",
   "You gawk at $M.",
   "$n gawks at $N.",
   "$n gawks at you.",
   "You gawk as you think what you must look like to others.",
   "$n is gawking again.  What is on $s mind?" 
   },
   
   {
   "giggle",
   "You giggle.",
   "$n giggles.",
   "You giggle in $N's presence.",
   "$n giggles at $N's actions.",
   "$n giggles at you.  Hope it's not contagious!",
   "You giggle at yourself.  You must be nervous or something.",
   "$n giggles at $mself.  $e must be nervous or something."
   },
   
   {
   "glare",
   "You glare at nothing in particular.",
   "$n glares around $m.",
   "You glare icily at $M.",
   "$n glares at $N.",
   "$n glares icily at you, you feel cold to your bones.",
   "You glare icily at your feet, they are suddenly very cold.",
   "$n glares at $s feet, what is bothering $m?"
   },
   
   {
   "grimace",
   "You contort your face in disgust.",
   "$n grimaces is disgust.",
   "You grimace in disgust at $M.",
   "$n grimaces in disgust at $N.",
   "$n grimaces in disgust at you.",
   "You grimace at yourself in disgust.",
   "$n grimaces at $mself in disgust."
   },
   
   {
   "grin",
   "You grin evilly.",
   "$n grins evilly.",
   "You grin evilly at $M.",
   "$n grins evilly at $N.",
   "$n grins evilly at you.  Hmmm.  Better keep your distance.",
   "You grin at yourself.  You must be getting very bad thoughts.",
   "$n grins at $mself.  You must wonder what's in $s mind."
   },
   
   {
   "groan",
   "You groan loudly.",
   "$n groans loudly.",
   "You groan at the sight of $M.",
   "$n groans at the sight of $N.",
   "$n groans at the sight of you.",
   "You groan as you realize what you have done.",
   "$n groans as $e realizes what $e has done."
   },
   
   {
   "grope",
   "Whom do you wish to grope?",
   NULL,
   "Well, what sort of noise do you expect here?",
   "$n gropes $N.",
   "$n gropes you.",
   "You grope yourself - YUCK.",
   "$n gropes $mself - YUCK."
   },
   
   {
   "grovel",
   "You grovel in the dirt.",
   "$n grovels in the dirt.",
   "You grovel before $M.",
   "$n grovels in the dirt before $N.",
   "$n grovels in the dirt before you.",
   "That seems a little silly to me.",
   NULL
   },
   
   {
   "growl",
   "Grrrrrrrrrr ...",
   "$n growls.",
   "Grrrrrrrrrr ... take that, $N!",
   "$n growls at $N.  Better leave the room before the fighting starts.",
   "$n growls at you.  Hey, two can play it that way!",
   "You growl at yourself.  Boy, do you feel bitter!",
   "$n growls at $mself.  This could get interesting..."
   },
   
   {
   "grumble",
   "You grumble.",
   "$n grumbles.",
   "You grumble to $M.",
   "$n grumbles to $N.",
   "$n grumbles to you.",
   "You grumble under your breath.",
   "$n grumbles under $s breath."
   },
   
   {
   "hand",
   "Kiss whose hand?",
   NULL,
   "You kiss $S hand.",
   "$n kisses $N's hand.  How continental!",
   "$n kisses your hand.  How continental!",
   "You kiss your own hand.",
   "$n kisses $s own hand."
   },

   {
   "hbeat",
   "Damn! Is that your heart?",
   "$n must be scared, you can hear $s heartbeat over here!",
   "You nestle quietly in $N's arms, listening to $S heartbeat.",
   "$n nestles into $N's arms, cuddling $M close.",
   "$n nestles quietly in your arms, listening to your heartbeat.",
   "You feel your heartbeat on your wrist.",
   "$n places $s fingers on $s inner wrist."
   },

   {
   "hsigh",
   "You sigh happily.",
   "$n sighs happily.",
   "You sigh happily on $N's shoulder.",
   "$n sighs happily on $N's shoulder.",
   "$n lies $s head on your shoulder and sighs happily.",
   "You sigh happily at your acomplishment.",
   "$n sighs happily, a smile on $s face.  Whats $e thinkin?!"
   },

   {   
   "hello",
   "You say hello to everyone in the room.",
   "$n says hello to everyone in the room.",
   "You tell $M how truly glad you are to see $M.",
   "$n tells $N 'Hi!'",
   "$n tells you how truly glad $e is that you are here.",
   "You greet yourself enthusiastically.",
   "$n greets $mself enthusiastically.  How odd."
   },
   
   {
   "highfive",
   "You jump in the air...oops, better get someone else to join you.",
   "$n jumps in the air by $mself.  Is $e a cheerleader, or just daft?",
   "You jump in the air and give $M a big highfive!",
   "$n jumps in the air and gives $N a big highfive!",
   "$n jumps in the air and gives you a big highfive!",
   "You jump in the air and congratulate yourself!",
   "$n jumps in the air and gives $mself a big highfive!  Wonder what $e did?"
   },
   
   {
   "hmm",
   "You Hmmmm out loud.",
   "$n thinks, 'Hmmmm.'",
   "You gaze thoughtfully at $M and say 'Hmmm.'",
   "$n gazes thoughtfully at $N and says 'Hmmm.'",
   "$n gazes thoughtfully at you and says 'Hmmm.'",
   "You Hmmmm out loud.",
   "$n thinks, 'Hmmmm.'"
   },
   
   {
   "hop",
   "You hop around like a little kid.",
   "$n hops around like a little kid.",
   "You hop into $N's lap.",
   "$n hops into $N's lap.",
   "$n hops into your lap.",
   NULL,
   NULL
   },
   
   {
   "hug",
   "Hug whom?",
   NULL,
   "You hug $M.",
   "$n hugs $N.",
   "$n hugs you.",
   "You hug yourself.",
   "$n hugs $mself in a vain attempt to get friendship."
   },

   {
   "hwave",
   "You wave happily to everyone around.",
   "$n waves happily to everyone in the area.",
   "You wave happily to $M.",
   "$n waves happily to $N.",
   "$n waves happily to you. Who is that?",
   "You wave at yourself. What are you thinking?",
   "$n waves happily to $mself. Hmm."
   },   

   {
   "iwhistle",
   "You whistle innocently.",
   "$n whistles innocently.",
   "You whistle innocently towards $M.",
   "$n whistles innocently at $N.",
   "$n whistles innocently at you.",
   "You whistle innocently about yourself.",
   "$n whistles innocently to $mself."
   },

    {
    "jeans",
    "You slide your hands into your pockets.",
    "$n slides $s hands into $s pockets.",
    "You sneak your hands into $N's jeans, pulling $M close.",
    "$n sneaks $s hands into $N's pockets.",
    "$n sneaks $s hands into your jeans, pulling you close.",
    "You slide your hands in your pocket. Looking for something?",
    "$n feels around in $s pockets. Whats $e doing?"
    },

   {
   "kiss",
   "Isn't there someone you want to kiss?",
   NULL,
   "You kiss $M.",
   "$n kisses $N.",
   "$n kisses you.",
   "All the lonely people :(",
   NULL
   },
   
   {
   "laugh",
   "You laugh.",
   "$n laughs.",
   "You laugh at $N mercilessly.",
   "$n laughs at $N mercilessly.",
   "$n laughs at you mercilessly.  Hmmmmph.",
   "You laugh at yourself.  I would, too.",
   "$n laughs at $mself.  Let's all join in!!!"
   },
   
   {
   "lag",
   "You complain about the terrible lag.",
   "$n starts complaining about the terrible lag.",
   "You complain to $N about the terrible lag.",
   "$n complains to $N about the terrible lag.",
   "$n complains to you about the terrible lag.",
   "You start muttering about the awful lag.",
   "$n starts muttering about the awful lag."
   },
   
   {
   "lick",
   "You lick your lips and smile.",
   "$n licks $s lips and smiles.",
   "You lick $M.",
   "$n licks $N.",
   "$n licks you.",
   "You lick yourself.",
   "$n licks $mself - YUCK."
   },

   {
   "lips",
   "You lick your bottom lip.",
   "$n licks $s bottom lip.",
   "You gently run your tongue over $N's lips.",
   "$n runs $s tongue gently over $N's lips.",
   "$n runs $s tongue over your lips gently.",
   "You bite your bottom lip and grin.",
   "$n bites $s bottom lip and grins sheepishly."
   },
   
   {
   "love",
   "You love the whole world.",
   "$n loves everybody in the world.",
   "You tell your true feelings to $N.",
   "$n whispers softly to $N.",
   "$n whispers to you sweet words of love.",
   "Well, we already know you love yourself (lucky someone does!)",
   "$n loves $mself, can you believe it ?"
   },
   
   {
   "lust",
   "You are getting lusty feelings!",
   "$n looks around lustily.",
   "You stare lustily at $N.",
   "$n stares lustily at $N.",
   "$n stares lustily at you.",
   "You stare lustily at...youself?",
   "$n looks $mself up and down lustily."
   },
   
   {
   "massage",
   "Massage what?  Thin air?",
   NULL,
   "You gently massage $N's shoulders.",
   "$n massages $N's shoulders.",
   "$n gently massages your shoulders.  Ahhhhhhhhhh ...",
   "You practice yoga as you try to massage yourself.",
   "$n gives a show on yoga positions, trying to massage $mself."
   },
   
   {
   "meow",
   "MEOW.",
   "$n meows.  What's $e going to do next, wash $mself with $s tongue?",
   "You meow at $M, hoping $E will give you some milk.",
   "$n meows at $N, hoping $E will give $m some milk. ",
   "$n meows at you.  Maybe $e wants some milk.",
   "You meow like a kitty cat.",
   "$n meows like a kitty cat."
   },
   
   {
   "mmm",
   "You go mmMMmmMMmmMMmm.",
   "$n says 'mmMMmmMMmmMMmm.'",
   "You go mmMMmmMMmmMMmm.",
   "$n says 'mmMMmmMMmmMMmm.'",
   "$n thinks of you and says, 'mmMMmmMMmmMMmm.'",
   "You think of yourself and go mmMMmmMMmmMMmm.",
   "$n thinks of $mself and says 'mmMMmmMMmmMMmm.'",
   },
   
   {
   "moan",
   "You start to moan.",
   "$n starts moaning.",
   "You moan for the loss of $m.",
   "$n moans for the loss of $N.",
   "$n moans at the sight of you.  Hmmmm.",
   "You moan at yourself.",
   "$n makes $mself moan."
   },
   
   {
   "mosh",
   "You MOSH insanely about the room.",
   "$n MOSHES insanely about the room.",
   "You MOSH $M into a pile of Jello pudding.  Ewww!",
   "$n MOSHES $N into Jello pudding.  Ah, the blood!",
   "$n MOSHES you into the ground.",
   "You MOSH yourself.  Ah the blood!",
   "$n MOSHES and MOSHES and MOSHES and MOSHES..."
   },
   
   {
   "muhaha",
   "You laugh diabolically.  MUHAHAHAHAHAHA!",
   "$n laughs diabolically.  MUHAHAHAHAHAHA!",
   "You laugh at $M diabolically.  MUHAHAHAHAHAHA!",
   "$n laughs at $N diabolically.  MUHAHAHAHAHAHA!",
   "$n laughs at you diabolically.  MUHAHAHAHAHAHA!",
   "Muhaha at yourself??  Wierd.",
   NULL
   },

   {
   "mutter",
   "You mutter to yourself.",
   "$n mutters.",
   "You mutter at $M.",
   "$n mutters at $N.",
   "$n mutters at you.",
   "You mutter to yourself.",
   "$n mutters.",
   },
  
   {
   "nibble",
   "Nibble on whom?",
   NULL,
   "You nibble on $N's ear.",
   "$n nibbles on $N's ear.",
   "$n nibbles on your ear.",
   "You nibble on your OWN ear.",
   "$n nibbles on $s OWN ear."
   },
   
   {
   "nod",
   "You nod affirmative.",
   "$n nods affirmative.",
   "You nod in recognition to $M.",
   "$n nods in recognition to $N.",
   "$n nods in recognition to you.  You DO know $m, right?",
   "You nod at yourself.  Are you getting senile?",
   "$n nods at $mself.  $e must be getting senile."
   },
   
   {
   "nose",
   "You wiggle your nose.",
   "$n wiggles $s nose.",
   "You tweek $S nose.",
   "$n tweeks $N's nose.",
   "$n tweeks your nose.",
   "You tweek your own nose!",
   "$n tweeks $s own nose!"
   },
   
   {
   "nudge",
   "Nudge whom?",
   NULL,
   "You nudge $M.",
   "$n nudges $N.",
   "$n nudges you.",
   "You nudge yourself, for some strange reason.",
   "$n nudges $mself, to keep $mself awake."
   },
   
   {
   "nuzzle",
   "Nuzzle whom?",
   NULL,
   "You nuzzle $S neck softly.",
   "$n softly nuzzles $N's neck.",
   "$n softly nuzzles your neck.",
   "I'm sorry, friend, but that's impossible.",
   NULL
   },
   
   {
   "ooo",
   "You go ooOOooOOooOOoo.",
   "$n says, 'ooOOooOOooOOoo.'",
   "You go ooOOooOOooOOoo.",
   "$n says, 'ooOOooOOooOOoo.'",
   "$n thinks of you and says, 'ooOOooOOooOOoo.'",
   "You go ooOOooOOooOOoo.",
   "$n says, 'ooOOooOOooOOoo.'"
   },
   
   {
   "pathead",
   "Pat whom?",
   NULL,
   "You pat $N on $S head.",
   "$n pats $N on $S head.",
   "$n pats you on your head.",
   "You pat yourself on your head.",
   "$n pats $mself on the head."
   },

   {
   "pcheek",
   "Oopsie!",
   "$n's cheeks turn pink.",
   "Isn't $E such a cutie!?!",
   "$n pinches $N's cheeks affectionately.",
   "$n pinches your cheeks affectionately.  Cutie!",
   "You pinch your cheeks for color.",
   "$n pinches $s own cheeks.  How odd."
   },

   {
   "peck",
   "You peck for seeds on the ground.",
   "$n pecks for seeds on the ground.",
   "You give $M a little peck on the cheek.",
   "$n gives $N a small peck on the cheek.",
   "$n gives you a sweet peck on the cheek.",
   "You kiss your own pectoral muscles.",
   "$n pecks $mself on $s pectoral muscles."
   },
   
   {
   "peer",
   "You peer intently about your surroundings.",
   "$n peers intently about the area, looking for thieves no doubt.",
   "You peer at $M quizzically.",
   "$n peers at $N quizzically.",
   "$n peers at you quizzically.",
   "You peer intently about your surroundings.",
   "$n peers intently about the area, looking for thieves no doubt."
   },
   
   {
   "pinch",
   "You toss a pinch of salt over your shoulder.",
   "$n tosses a pinch of salt over $s shoulder.",
   "You pinch $S rosy cheeks.",
   "$n pinches $N's rosy cheeks.",
   "$n pinches your chubby cheeks.",
   "You need a pinch of salt.",
   "$n needs a pinch of salt."
   },
   
   {
   "ping",
   "Ping who?",
   NULL,
   "You ping $N.  $N is unreachable.",
   "$n pings $N.  $N is unreachable.",
   "$n pings you.  Is your server dead or alive?",
   "You ping yourself. Ooof!",
   "$n pings $mself. Ooof!"
   },
   
   {
   "point",
   "Point at whom?",
   NULL,
   "You point at $M accusingly.",
   "$n points at $N accusingly.",
   "$n points at you accusingly.",
   "You point proudly at yourself.",
   "$n points proudly at $mself."
   },
   
   {
   "poke",
   "Poke whom?",
   NULL,
   "You poke $M in the ribs.",
   "$n pokes $N in the ribs.",
   "$n pokes you in the ribs.",
   "You poke yourself in the ribs, feeling very silly.",
   "$n pokes $mself in the ribs, looking very sheepish."
   },
   
   {
   "ponder",
   "You ponder the question.",
   "$n sits down and thinks deeply.",
   NULL,
   NULL,
   NULL,
   NULL,
   NULL
   },
   {
	"pounce",
	"You start to pounce.. and fall flat on your face.",
	"$n pounces at the air and lands on $s face.",
	"You pounce on $M and take $M to the ground.",
	"$n pounces on $N and they both fall to the ground.",
	"$n pounces you and takes you to the ground.",
	"You can't pounce on yourself!",
	"$n tries to pounce on $mself and fails."
    },
   
   {
   "pout",
   "Ah, don't take it so hard.",
   "$n pouts.",
   "You pout at $M.",
   "$n pouts at $N.",
   "$n pouts at you.",
   "Ah, don't take it so hard.",
   "$n pouts."
   },
   
   {   
   "prod",
   "Yeah, great, but WHO do you want to prod? Orphus?",
   NULL,
   "You pretend $N is Orphus, and prod $M firmly in the belly.",
   "$n pretend that $N is Orphus, and prods $M firmly in the belly.",
   "$n pretends that you are Orphus, and prods you firmly in the belly.",
   "You pretend you are Orphus, and prod yourself in the belly.",
   "$n pretends $e is Orphus, and prod's $mself in the belly."
   },
   
   {
   "purr",
   "You purr contentedly.",
   "$n purrs contentedly.",
   "You purr contentedly in $N's lap.",
   "$n purrs contentedly in $N's lap.",
   "$n purrs contentedly in your lap.",
   "You purr at yourself.",
   "$n purrs at $mself.  Must be a cat thing."
   },
   
   {
   "raise",
   "You raise your hand in response.",
   "$n raises $s hand in response.",
   "You raise your hand in response.",
   "$n raises $s hand in response.",
   "$n raises $s hand in response to you.",
   "You raise your hand in response.",
   "$n raises $s hand in response."
   },
   
   {
   "rofl",
   "You roll on the floor laughing hysterically.",
   "$n rolls on the floor laughing hysterically.",
   "You laugh your head off at $S remark.",
   "$n rolls on the floor laughing at $N's remark.",
   "$n can't stop laughing at your remark.",
   "You roll on the floor and laugh at yourself.",
   "$n laughs at $mself.  Join in the fun."
   },
   
   {
   "rolleyes",
   "You roll your eyes.",
   "$n rolls $s eyes.",
   "You roll your eyes at $M.",
   "$n rolls $s eyes at $N.",
   "$n rolls $s eyes at you.",
   "You roll your eyes at yourself.",
   "$n rolls $s eyes at $mself."
   },
   
   {
   "roll2",
   "You roll your eyes and make a repetative motion near your crotch.",
   "$n rolls $s eyes and makes a repetative motion near $s crotch.",
   "You roll your eyes at $M and make a repetative motion near your crotch.",
   "$n rolls $s eyes at $N and makes a repetative motion near $s crotch.",
   "$n rolls $s eyes at you and makes a repetative motion near $s crotch.",
   "You make a repetative motion near your crotch...you find you quite like it!",
   "$n starts making a repetative motion near $s crotch...how strange..."
   },
   
   {
   "rub",
   "You rub your eyes.  How long have you been at this?",
   "$n rubs $s eyes.  $n must have been playing all day.",
   "You rub your eyes.  Has $N been playing as long as you have?",
   "$n rubs $s eyes.  $n must have been playing all day.",
   "$n rubs $s eyes.  Have you been playing as long as $m?",
   "You rub your eyes.  How long have you been at this?",
   "$n rubs $s eyes.  $n must have been playing all day."
   },
   
   {
   "ruffle",
   "You've got to ruffle SOMEONE.",
   NULL,
   "You ruffle $N's hair playfully.",
   "$n ruffles $N's hair playfully.",
   "$n ruffles your hair playfully.",
   "You ruffle your hair.",
   "$n ruffles $s hair."
   },
   
   {
   "sad",
   "You put on a glum expression.",
   "$n looks particularly glum today.  *sniff*",
   "You give $M your best glum expression.",
   "$n looks at $N glumly.  *sniff*  Poor $n.",
   "$n looks at you glumly.  *sniff*   Poor $n.",
   "You bow your head and twist your toe in the dirt glumly.",
   "$n bows $s head and twists $s toe in the dirt glumly."
   },
   
   {
   "salute",
   "You salute smartly.",
   "$n salutes smartly.",
   "You salute $M.",
   "$n salutes $N.",
   "$n salutes you.",
   "Huh?",
   NULL
   },
   
   {
   "scowl",
   "You scowl angrilly.",
   "$n scowls angrilly.",
   "You scowl angrilly at $M.",
   "$n scowls angrilly at $N.",
   "$n scowls angrilly at you.",
   "You scowl angrilly at yourself.",
   "$n scowls angrilly at $mself."
   },
   
   {
   "shake",
   "You shake your head.",
   "$n shakes $s head.",
   "You shake your head at $N.",
   "$n shakes $s head at $N.",
   "$n shakes $s head at you.",
   "You are shaken by yourself.",
   "$n shakes and quivers like a bowl full of jelly."
   },
   
   {
   "shiver",
   "Brrrrrrrrr.",
   "$n shivers uncomfortably.",
   "You shiver at the thought of fighting $M.",
   "$n shivers at the thought of fighting $N.",
   "$n shivers at the suicidal thought of fighting you.",
   "You shiver to yourself?",
   "$n scares $mself to shivers."
   },

   {
   "shoulder",
   "You lean your head to one side, looking confused.",
   "$n cocks $s head to one side, looking confused.",
   "You lay your head on $S shoulder.",
   "$n lays $s head on $N's shoulder, smiling happily.",
   "$n snuggles close to you, leaning $s head on your shoulder.",
   "You try to lay your head on your own shoulder. Confused much?",
   "$n tries to lay $s head on $s shoulder. What a weirdo!"
   },

   {
   "shrug",
   "You shrug.",
   "$n shrugs helplessly.",
   "You shrug in response to $s question.",
   "$n shrugs in response to $N's question.",
   "$n shrugs in respopnse to your question.",
   "You shrug to yourself.",
   "$n shrugs to $mself.  What a strange person."
   },

   {
   "shysmile",
   "Well, shucks...",
   "$n gives a shy, hesitant smile.",
   "Feeling shy?",
   "$n smiles shyly at $N.",
   "$n smiles shyly at you.",
   "Uh-oh, my bad!",
   "$n smiles and blushes at $s mistake."
   },
   
   {
   "sigh",
   "You sigh.",
   "$n sighs loudly.",
   "You sigh as you think of $M.",
   "$n sighs at the sight of $N.",
   "$n sighs as $e thinks of you.  Touching, huh?",
   "You sigh at yourself.  You MUST be lonely.",
   "$n sighs at $mself.  What a sorry sight."
   },
   
   {
   "sing",
   "You raise your clear voice towards the sky.",
   "$n has begun to sing.",
   "You sing a ballad to $m.",
   "$n sings a ballad to $N.",
   "$n sings a ballad to you!  How sweet!",
   "You sing a little ditty to yourself.",
   "$n sings a little ditty to $mself."
   },
   
   {
   "slap",
   "Slap whom?",
   NULL,
   "You slap $M playfully.",
   "$n slaps $N for $S stupidity.",
   "$n slaps you for your stupidity.  OUCH!",
   "You slap yourself.  You deserve it.",
   "$n slaps $mself.  Why don't you join in?"
   },
   
   {
   "smile",
   "You smile happily.",
   "$n smiles happily.",
   "You smile at $M.",
   "$n beams a smile at $N.",
   "$n smiles at you.",
   "You smile at yourself.",
   "$n smiles at $mself."
   },
   
   {
   "smirk",
   "You smirk.",
   "$n smirks.",
   "You smirk at $S saying.",
   "$n smirks at $N's saying.",
   "$n smirks at your saying.",
   "You smirk at yourself.  Okay ...",
   "$n smirks at $s own 'wisdom'."
   },
   
   {
   "smoke",
   "You calmly light a cigarette and take a puff.",
   "$n calmly lights a cigarette and take a puff.",
   "You blow smoke into $S eyes.",
   "$n blows smoke into $N's eyes.",
   "$n blows smoke rings into your eyes.",
   "You call down lightning and SMOKE yourself.",
   "$n calls down lightning and SMOKES $mself."
   },
   
   {
   "smooch",
   "You are searching for someone to smooch.",
   "$n is looking for someone to smooch.",
   "You give $M a nice, wet smooch.",
   "$n and $N are smooching in the corner.",
   "$n smooches you passionately on the lips.",
   "You smooch yourself.",
   "$n smooches $mself.  Yuck."
   },
   
   {
   "snap",
   "PRONTO ! You snap your fingers.",
   "$n snaps $s fingers.",
   "You snap back at $M.",
   "$n snaps back at $N.",
   "$n snaps back at you!",
   "You snap yourself to attention.",
   "$n snaps $mself to attention."
   },
   
   {
   "snarl",
   "You grizzle your teeth and look mean.",
   "$n snarls angrily.",
   "You snarl at $M.",
   "$n snarls at $N.",
   "$n snarls at you, for some reason.",
   "You snarl at yourself.",
   "$n snarls at $mself."
   },
   
   {
   "sneer",
   "You sneer in contempt.",
   "$n sneers in contempt.",
   "You sneer at $M in contempt.",
   "$n sneers at $N in contempt.",
   "$n sneers at you in contempt.",
   "You sneer at yourself in contempt.",
   "$n sneers at $mself in contempt."
   },
   
   {
   "sneeze",
   "Gesundheit!",
   "$n sneezes.",
   "You sneeze all over $M.",
   "$n sneezes all over $N.",
   "$n sneezes all over you.",
   NULL,
   NULL
   },
   
   {
   "snicker",
   "You snicker softly.",
   "$n snickers softly.",
   "You snicker with $M about your shared secret.",
   "$n snickers with $N about their shared secret.",
   "$n snickers with you about your shared secret.",
   "You snicker at your own evil thoughts.",
   "$n snickers at $s own evil thoughts."
   },
   
   {
   "sniff",
   "You sniff sadly. *SNIFF*",
   "$n sniffs sadly.",
   "You sniff sadly at the way $E is treating you.",
   "$n sniffs sadly at the way $N is treating $m.",
   "$n sniffs sadly at the way you are treating $m.",
   "You sniff sadly at your lost opportunities.",
   "$n sniffs sadly at $mself.  Something MUST be bothering $m."
   },
   
   {
   "snore",
   "Zzzzzzzzzzzzzzzzz.",
   "$n snores loudly.",
   NULL,
   NULL,
   NULL,
   NULL,
   NULL
   },
   
   {
   "snort",
   "You snort in disgust.",
   "$n snorts in disgust.",
   "You snort at $M in disgust.",
   "$n snorts at $N in disgust.",
   "$n snorts at you in disgust.",
   "You snort at yourself in disgust.",
   "$n snorts at $mself in disgust."
   },
   
   {
   "snuggle",
   "Who?",
   NULL,
   "You snuggle up contentedly in $S lap.",
   "$n snuggles up contentedly in $N's lap.",
   "$n snuggles up contentedly in your lap.",
   "You snuggle up, getting ready to sleep.",
   "$n snuggles up, getting ready to sleep."
   },
   
   {
   "spank",
   "Spank whom?",
   NULL,
   "You spank $M playfully.",
   "$n spanks $N playfully.",
   "$n spanks you playfully.  OUCH!",
   "You spank yourself.  Kinky!",
   "$n spanks $mself.  Kinky!"
   },
   
   {
   "spin",
   "You twirl in a graceful pirouette.",
   "$n twirls in a graceful pirouette.",
   "You spin $M on one finger.",
   "$n spins $N on $s finger.",
   "$n spins you around on $s finger.",
   "You spin yourself around and around and around....",
   "$n spins $mself around and around and around..."
   },
   
   {
   "squeeze",
   "Where, what, how, whom?",
   NULL,
   "You squeeze $M fondly.",
   "$n squeezes $N fondly.",
   "$n squeezes you fondly.",
   "You squeeze yourself - try to relax a little!",
   "$n squeezes $mself."
   },
   
   {
   "squeak",
   "You squeak like a mouse.",
   "$n squeaks like a mouse.",
   "You squeak at $M.",
   "$n squeaks at $N.  Is $e a man or a mouse?",
   "$n squeaks at you.  Is $e a man or a mouse?",
   "You squeak at yourself like a mouse.",
   "$n squeaks at $mself like a mouse."
   },
   
   {
   "squeal",
   "You squeal with delight.",
   "$n squeals with delight.",
   "You squeal at $M.",
   "$n squeals at $N.  Wonder why?",
   "$n squeals at you.  You must be doing something good.",
   "You squeal at yourself.",
   "$n squeals at $mself."
   },
   
   {
   "squirm",
   "You squirm guiltily.",
   "$n squirms guiltily.  Looks like $e did it.",
   "You squirm in front of $M.",
   "$n squirms in front of $N.",
   "$n squirms in front of you.  You make $m nervous.",
   "You squirm and squirm and squirm....",
   "$n squirms and squirms and squirm....."
   },
   
   {
   "squish",
   "You squish your toes into the sand.",
   "$n squishes $s toes into the sand.",
   "You squish $M between your legs.",
   "$n squishes $N between $s legs.",
   "$n squishes you between $s legs.",
   "You squish yourself.",
   "$n squishes $mself.  OUCH."
   },
   
   {
   "stare",
   "You stare at the sky.",
   "$n stares at the sky.",
   "You stare dreamily at $N, completely lost in $S eyes..",
   "$n stares dreamily at $N.",
   "$n stares dreamily at you, completely lost in your eyes.",
   "You stare dreamily at yourself - enough narcissism for now.",
   "$n stares dreamily at $mself - NARCISSIST!"
   },
   
   {
   "sticktongue",
   "You stick out your tongue.",
   "$n sticks out $s tongue.",
   "You stick your tongue out at $M.",
   "$n sticks $s tongue out at $N.",
   "$n sticks $s tongue out at you.",
   "You stick out your tongue and touch your nose.",
   "$n sticks out $s tongue and touches $s nose."
   },
   
   {
   "stretch",
   "You stretch and relax your sore muscles.",
   "$n stretches luxuriously.  Make you want to, doesn't it.",
   "You stretch and relax your sore muscles.",
   "$n stretches luxuriously.  Make you want to, doesn't it.",
   "$n stretches luxuriously.  Make you want to, doesn't it.",
   "You stretch and relax your sore muscles.",
   "$n stretches luxuriously.  Make you want to, doesn't it."
   },
   
   {
   "strip",
   "You show some of your shoulder as you begin your performance.",
   "$n shows $s bare shoulder and glances seductively around the room.",
   "You show some of your shoulder as you begin your performance.",
   "$n shows $s bare shoulder and glances seductively at $N.",
   "$n shows $s bare shoulder and glances seductively at you.",
   "You show some of your shoulder as you begin your performance.",
   "$n shows $s bare shoulder and glances seductively around the room."
   },
   
   {
   "strut",
   "Strut your stuff.",
   "$n struts proudly.",
   "You strut to get $S attention.",
   "$n struts, hoping to get $N's attention.",
   "$n struts, hoping to get your attention.",
   "You strut to yourself, lost in your own world.",
   "$n struts to $mself, lost in $s own world."
   },
   
   {
   "sulk",
   "You sulk.",
   "$n sulks in the corner.",
   NULL,
   NULL,
   NULL,
   NULL,
   NULL
   },
   
   {
   "swoon",
   "You swoon in ecstacy.",
   "$n swoons in ecstacy.",
   "You swoon in ecstacy at the thought of $M.",
   "$n swoons in ecstacy at the thought of $N.",
   "$n swoons in ecstacy as $e thinks of you.",
   "You swoon in ecstacy.",
   "$n swoons in ecstacy."
   },
   
   {
   "tackle",
   "You can't tackle the AIR!",
   NULL,
   "You run over to $M and bring $M down!",
   "$n runs over to $N and tackles $M to the ground!",
   "$n runs over to you and tackles you to the ground!",
   "You wrap your arms around yourself, and throw yourself to the ground.",
   "$n wraps $s arms around $mself and brings $mself down!?"
   },
   
   {
   "tap",
   "You tap your foot impatiently.",
   "$n taps $s foot impatiently.",
   "You tap your foot impatiently.  Will $E ever be ready?",
   "$n taps $s foot impatiently as $e waits for $N.",
   "$n taps $s foot impatiently as $e waits for you.",
   "You tap yourself on the head.  Ouch!",
   "$n taps $mself on the head."
   },
   
   {
   "tender",
   "You will enjoy it more if you choose someone to kiss.",
   NULL,
   "You give $M a soft, tender kiss.",
   "$n gives $N a soft, tender kiss.",
   "$n gives you a soft, tender kiss.",
   "You'd better not, people may start to talk!",
   NULL
   },
   
   {
   "thank",
   "Thank you too.",
   NULL,
   "You thank $N heartily.",
   "$n thanks $N heartily.",
   "$n thanks you heartily.",
   "You thank yourself since nobody else wants to !",
   "$n thanks $mself since you won't."
   },
   
   {
   "think",
   "You think about times past and friends forgotten.",
   "$n thinks deeply and is lost in thought.",
   "You think about times past and friends forgotten.",
   "$n thinks deeply and is lost in thought.",
   "$n thinks deeply and is lost in thought.  Maybe $e is thinking of you?",
   "You think about times past and friends forgotten.",
   "$n thinks deeply and is lost in thought."
   },
   
   {
   "throttle",
   "Whom do you want to throttle?",
   NULL,
   "You throttle $M till $E is blue in the face.",
   "$n throttles $N about the neck, until $E passes out.  THUNK!",
   "$n throttles you about the neck until you pass out.  THUNK!",
   "That might hurt!  Better not do it!",
   "$n is getting a crazy look in $s eye again."
   },
   
   {
   "tickle",
   "Whom do you want to tickle?",
   NULL,
   "You tickle $N.",
   "$n tickles $N.",
   "$n tickles you - hee hee hee.",
   "You tickle yourself, how funny!",
   "$n tickles $mself."
   },

   {
   "twiddle",
   "You patiently twiddle your thumbs.",
   "$n patiently twiddles $s thumbs.",
   "You twiddle $S ears.",
   "$n twiddles $N's ears.",
   "$n twiddles your ears.",
   "You twiddle your ears like Dumbo.",
   "$n twiddles $s own ears like Dumbo."
   },
   
   {
   "type",
   "You throw up yor handz in dizgust at yur losy typing skils.",
   "$n couldn't type a period if there was only one key on the keyboard.",
   "You throw up yor handz in dizgust at yur losy typing skils.",
   "$n couldn't type a period if there was only one key on the keyboard.",
   "$n couldn't type a period if there was only one key on the keyboard.",
   "You throw up yor handz in dizgust at yur losy typing skils.",
   "$n couldn't type a period if there was only one key on the keyboard."
   },

   {
   "wave",
   "You wave.",
   "$n waves happily.",
   "You wave goodbye to $N.",
   "$n waves goodbye to $N.",
   "$n waves goodbye to you.  Have a good journey.",
   "Are you going on adventures as well?",
   "$n waves goodbye to $mself."
   },
   
   {
   "whistle",
   "You whistle appreciatively.",
   "$n whistles appreciatively.",
   "You whistle at the sight of $M.",
   "$n whistles at the sight of $N.",
   "$n whistles at the sight of you.",
   "You whistle a little tune to yourself.",
   "$n whistles a little tune to $mself."
   },
   
   {
   "wiggle",
   "Your wiggle your bottom.",
   "$n wiggles $s bottom.",
   "You wiggle your bottom toward $M.",
   "$n wiggles $s bottom toward $N.",
   "$n wiggles $s bottom toward you.",
   "You wiggle about like a fish.",
   "$n wiggles about like a fish."
   },
   
   {
   "wince",
   "You wince.  Ouch!",
   "$n winces.  Ouch!",
   "You wince at $M.",
   "$n winces at $N.",
   "$n winces at you.",
   "You wince at yourself.  Ouch!",
   "$n winces at $mself.  Ouch!"
   },
   
   {
   "wink",
   "You wink suggestively.",
   "$n winks suggestively.",
   "You wink suggestively at $N.",
   "$n winks at $N.",
   "$n winks suggestively at you.",
   "You wink at yourself ?? - what are you up to ?",
   "$n winks at $mself - something strange is going on..."
   },

   {
   "wkiss",
   "You lean against the wall, and kiss it hard!",
   "$n leans against the wall, kissing it hard! Thats not right!!",
   "You press $N up against a wall and kiss $M hard!",
   "$n presses $N up against a wall and kisses $M hard!",
   "$n presses you up against a wall and kisses you hard!",
   "You lean into the wall, kissing your hand.",
   "$n leans into the wall, kissing $s hand hard! Want what $e's on?"
   },
   
   {
   "worship",
   "You worship the powers that be.",
   "$n worships the powers that be.",
   "You drop to your knees in homage of $M.",
   "$n prostrates $mself before $N.",
   "$n believes you are all powerful.",
   "You worship yourself.",
   "$n worships $mself - ah, the conceitedness of it all."
   },
   
   {
   "wrap",
   "You wrap a present for your love.",
   "$n wraps a present for someone special.",
   "You wrap your legs around $M.",
   "$n wraps $s legs around $N.",
   "$n wraps $s legs around you.  Wonder what's coming next?",
   "You wrap yourself with some paper.",
   "$n wraps $mself with some wrapping paper.  Is it Christmas?"
   },

   {
   "yawn",
   "You must be tired.",
   "$n yawns.",
   "You must be tired.",
   "$n yawns at $N.  Maybe you should let them go to bed?",
   "$n yawns at you.  Maybe $e wants you to go to bed with $m?",
   "You must be tired.",
   "$n yawns."
   },
   
   {
   "",
   NULL, NULL, NULL, NULL, NULL, NULL, NULL
   }
   };


/*
 * The main entry point for executing commands.
 * Can be recursively called from 'at', 'order', 'force'.
 */
   void interpret( CHAR_DATA *ch, char *argument )
   {
      CHAR_DATA *vch;
      char arg[MAX_STRING_LENGTH];
      char argu[MAX_STRING_LENGTH];
      char command[MAX_STRING_LENGTH];
      char logline[MAX_STRING_LENGTH];
      char buf[MAX_STRING_LENGTH];
   
   
      long cmd;
      long trust;
      bool found;
   
      {
         char *rest = one_argument(argument, arg);
         snprintf(argu, sizeof(argu), "%s %s", arg, rest);
      }
   
    /*
     * Strip leading spaces.
     */
      while ( isspace(*argument) )
         argument++;
      if ( argument[0] == '\0' )
         return;
   
    /*
     * Implement freeze command.
     */
      if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_FREEZE) )
      {
         send_to_char( "You're totally frozen!\n\r", ch );
         return;
      }
      else if (!IS_NPC(ch) && IS_FLAG(ch, FLAG_AFK) && str_cmp(argument,"afk"))
      {
         stc("You may not act while afk.  Use the afk command to return.\n\r",ch);
         return;
      }
   
    /*
     * Grab the command word.
     * Special parsing so ' can be a command,
     *   also no spaces needed after punctuation.
     */
      strcpy( logline, argument );
      if ( !isalpha(argument[0]) && !isdigit(argument[0]) )
      {
         command[0] = argument[0];
         command[1] = '\0';
         argument++;
         while ( isspace(*argument) )
            argument++;
      }
      else
      {
         argument = one_argument( argument, command );
      }
   
    /*
     * Look for command in command table.
     */
      found = FALSE;
      trust = get_trust( ch );
      for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
      {
         if ( command[0] == cmd_table[cmd].name[0]
            &&   !str_prefix( command, cmd_table[cmd].name )
            &&   cmd_table[cmd].level <= trust )
         {
            if (IS_MORE(ch,MORE_HEAD) || IS_EXTRA(ch,EXTRA_OSWITCH) )
            {
               if (IS_MORE(ch, MORE_MUMMIFIED))
               {
                  if      (!str_cmp( cmd_table[cmd].name, "reply"   )) found = TRUE;
                  else if (!str_cmp( cmd_table[cmd].name, "look"    )) found = TRUE;
                  else if (!str_cmp( cmd_table[cmd].name, "where"   )) found = TRUE;
                  else if (!str_cmp( cmd_table[cmd].name, "score"   )) found = TRUE;
                  else if (!str_cmp( cmd_table[cmd].name, "save"    )) found = TRUE;
                  else if (!str_cmp( cmd_table[cmd].name, "who"     )) found = TRUE;
                  else if (!str_cmp( cmd_table[cmd].name, "quit"    )) found = TRUE;
                  else if (!str_cmp( cmd_table[cmd].name, "mummify" )) found = TRUE;
                  else
                  {
                     send_to_char( "Not while mummified!\n\r", ch );
                     return;
                  }
               }
               else
               {
                  if      (!str_cmp( cmd_table[cmd].name, "say"     )) found = TRUE;
                  else if (!str_cmp( cmd_table[cmd].name, "'"       )) found = TRUE;
                  else if (!str_cmp( cmd_table[cmd].name, "immtalk" )) found = TRUE;
                  else if (!str_cmp( cmd_table[cmd].name, ":"       )) found = TRUE;
                  else if (!str_cmp( cmd_table[cmd].name, "vamptalk")) found = TRUE;
                  else if (!str_cmp( cmd_table[cmd].name, "ctalk"   )) found = TRUE;
                  else if (!str_cmp( cmd_table[cmd].name, "chat"    )) found = TRUE;
                  else if (!str_cmp( cmd_table[cmd].name, "."       )) found = TRUE;
                  else if (!str_cmp( cmd_table[cmd].name, "look"    )) found = TRUE;
                  else if (!str_cmp( cmd_table[cmd].name, "save"    )) found = TRUE;
                  else if (!str_cmp( cmd_table[cmd].name, "exits"   )) found = TRUE;
                  else if (!str_cmp( cmd_table[cmd].name, "emote"   )) found = TRUE;
                  else if (!str_cmp( cmd_table[cmd].name, "tell"    )) found = TRUE;
                  else if (!str_cmp( cmd_table[cmd].name, "reply"   )) found = TRUE;
                  else if (!str_cmp( cmd_table[cmd].name, "order"   )) found = TRUE;
                  else if (!str_cmp( cmd_table[cmd].name, "ashestoashes")) found = TRUE;
                  else if (!str_cmp( cmd_table[cmd].name, "plasmaform")) found = TRUE;
                  else if (!str_cmp( cmd_table[cmd].name, "who"     )) found = TRUE;
                  else if (!str_cmp( cmd_table[cmd].name, "weather" )) found = TRUE;
                  else if (!str_cmp( cmd_table[cmd].name, "where"   )) found = TRUE;
                  else if (!str_cmp( cmd_table[cmd].name, "relevel" )) found = TRUE;
                  else if (!str_cmp( cmd_table[cmd].name, "safe"    )) found = TRUE;
                  else if (!str_cmp( cmd_table[cmd].name, "scan"    )) found = TRUE;
                  else if (!str_cmp( cmd_table[cmd].name, "spy"     )) found = TRUE;
                  else if (!str_cmp( cmd_table[cmd].name, "score"   )) found = TRUE;
                  else if (!str_cmp( cmd_table[cmd].name, "save"    )) found = TRUE;
                  else if (!str_cmp( cmd_table[cmd].name, "inventory" )) found = TRUE;
                  else if (!str_cmp( cmd_table[cmd].name, "oreturn" )) found = TRUE;
                  else if (!str_cmp( cmd_table[cmd].name, "pour" )) found = TRUE;
                  else if (!str_cmp( cmd_table[cmd].name, "nightsight" )) found = TRUE;
                  else if (!str_cmp( cmd_table[cmd].name, "truesight" )) found = TRUE;
                  else if (!str_cmp( cmd_table[cmd].name, "horns" )) found = TRUE;
                  else if (!str_cmp( cmd_table[cmd].name, "fangs" )) found = TRUE;
                  else if (!str_cmp( cmd_table[cmd].name, "cast" )) found = TRUE;
                  else if (!str_cmp( cmd_table[cmd].name, "quit" ) &&
                          !IS_NPC(ch) && ch->pcdata->obj_vnum != 0) found = TRUE;
                  else 
                  {
			send_to_char( "Not without a body!\n\r", ch );
                        return;
                  }
               }
            }
            else if (ch->embraced != ARE_NONE )
            {
               if (!str_cmp( cmd_table[cmd].name, "immtalk" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, ":"       )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "vamptalk")) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "ctalk"   )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "look" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "save" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "kill" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "inventory" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "embrace" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "equipment" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "tell" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "reply" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "who" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "relevel" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "wake" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "stand" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "regenerate" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "vclan" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "upkeep" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "score" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "affects" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "goto" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "where" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "burn" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "flex" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "fcommand" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "[" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "group" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "gtell" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, ";" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "ooc" )) found = TRUE;
               else {
                  send_to_char( "Not while you are in an embrace.\n\r", ch );
                  return; }
            }
            else if (IS_EXTRA(ch,TIED_UP) )
            {
               if      (!str_cmp( cmd_table[cmd].name, "say"  )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "'"    )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "immtalk" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, ":"       )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "vamptalk")) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "ctalk"   )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "chat" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "."    )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "yell" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "shout")) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "look" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "save" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "exits")) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "inventory" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "tell" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "reply" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "order" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "who" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "weather" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "where" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "introduce" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "relevel" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "safe" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "scan" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "spy"  )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "sleep" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "wake" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "stand" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "fangs" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "claws" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "nightsight" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "umbralvision" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "umbra" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "regenerate" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "shield" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "vclan" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "upkeep" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "score" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "affects" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "immune" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "report" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "goto" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "flex" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "mindflex" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "contort" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "change" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "drink" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "fcommand" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "[" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "astral" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "projection" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "plasmaform" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "burrow" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "earthmeld" )) found = TRUE;
               else {
                  send_to_char( "Not while tied up.\n\r", ch );
                  if (ch->position > POS_STUNNED)
                     act("$n strains against $s bonds.",ch,NULL,NULL,TO_ROOM);
                  return; }
            }
            else if (IS_VAMPAFF(ch,VAM_EARTHMELDED) )
            {
               if (!str_cmp( cmd_table[cmd].name, "save" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "quit" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "call" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "inventory" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "who" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "relevel" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "safe" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "vclan" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "upkeep" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "score" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "affects" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "immune" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "earthmeld" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "burrow" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "eshift" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "emeld" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "ooc" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "vt" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "howl" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, ">" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "vtalk" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "vamptalk" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "earthshift" )) found = TRUE;
               else if (!str_cmp( cmd_table[cmd].name, "regenerate" )) found = TRUE;
               else {
                  send_to_char( "You cannot do that while you are burried in the earth.\n\r", ch );
                  return; }
            }
            found = TRUE;
            break;
         }
      }
   
    /*
     * Log and snoop.
     */
      if ( cmd_table[cmd].log == LOG_NEVER )
         strcpy( logline, "XXXXXXXX XXXXXXXX XXXXXXXX" );
   
      if ( ( !IS_NPC(ch) && IS_SET(ch->act, PLR_LOG) )
         ||   fLogAll
         ||   cmd_table[cmd].log == LOG_ALWAYS )
      {
         sprintf( log_buf, "Log %s: %s", ch->name, logline );
         log_string( log_buf );
      }
   
      if ( ch->desc != NULL && ch->desc->snoop_by != NULL )
      {
         if ( (vch = ch->desc->snoop_by->character) == NULL)
            ch->desc->snoop_by = NULL;
         else if (IS_IMM(vch))
         {
            write_to_buffer( ch->desc->snoop_by, "% ",    2 );
            write_to_buffer( ch->desc->snoop_by, logline, 0 );
            write_to_buffer( ch->desc->snoop_by, "\n\r",  2 );
         }
         else if (ch->in_room == NULL || vch->in_room == NULL)
         {
            sprintf(buf, "You loose your mental link with %s.\n\r",ch->name);
            send_to_char(buf,vch);
            bug( "Auspex power 4 in-room bug.", 0 );
            ch->desc->snoop_by = NULL;
         }
         else if (ch->in_room != vch->in_room)
         {
            sprintf(buf, "You loose your mental link with %s.\n\r",ch->name);
            send_to_char(buf,vch);
            ch->desc->snoop_by = NULL;
         }
         else
         {
            sprintf(buf, "%s thinks: %s.\n\r",ch->name,logline);
            send_to_char(buf,vch);
         }
      }
   
      if ( !found )
      {
      /*
      * Look for command in socials table.
      */
         if ( !check_social( ch, command, argument ) )
         {
               send_to_char( "Huh?\n\r", ch );
         }
         return;
      }
      else /* a normal valid command.. check if it is disabled */
         if (check_disabled (&cmd_table[cmd]))
         {
            send_to_char ("This command has been temporarily disabled.\n\r", ch);
            return;
         }
   
    /*
     * Character not in position for command?
     */
      if ( ch->position < cmd_table[cmd].position && !IS_NPC(ch) && 
         IS_VAMPIRE(ch) && get_disc(ch,DISC_PROTEAN) >= 7)
      {
         bool stop_now = TRUE;
         switch( ch->position )
         {
            case POS_DEAD:
               send_to_char( "Lie still; you are DEAD.\n\r", ch );
               break;
         
            case POS_MORTAL:
            case POS_INCAP:
               stop_now = FALSE;
               WAIT_STATE(ch,12);
               break;
         
            case POS_STUNNED:
               stop_now = FALSE;
               WAIT_STATE(ch,12);
               break;
         
            case POS_SLEEPING:
               send_to_char( "In your dreams, or what?\n\r", ch );
               break;
         
            case POS_MEDITATING:
            case POS_SITTING:
            case POS_RESTING:
               send_to_char( "Nah... You feel too relaxed...\n\r", ch);
               break;
         
            case POS_FIGHTING:
               send_to_char( "No way!  You are still fighting!\n\r", ch);
               break;
         
         }
         if (stop_now) 
            return;
      }
      else if ( ch->position < cmd_table[cmd].position )
      {
         switch( ch->position )
         {
            case POS_DEAD:
               send_to_char( "Lie still; you are DEAD.\n\r", ch );
               break;
         
            case POS_MORTAL:
            case POS_INCAP:
               send_to_char( "You are hurt far too bad for that.\n\r", ch );
               break;
         
            case POS_STUNNED:
               send_to_char( "You are too stunned to do that.\n\r", ch );
               break;
         
            case POS_SLEEPING:
               send_to_char( "In your dreams, or what?\n\r", ch );
               break;
         
            case POS_MEDITATING:
            case POS_SITTING:
            case POS_RESTING:
               send_to_char( "Nah... You feel too relaxed...\n\r", ch);
               break;
         
            case POS_FIGHTING:
               send_to_char( "No way!  You are still fighting!\n\r", ch);
               break;
         
         }
         return;
      }
   
    /*
     * Dispatch the command.
     */
   
      (*cmd_table[cmd].do_fun) ( ch, argument );
      tail_chain( );
      /* Commands such as quit may have freed ch; do not dereference it here. */
   
      return;
   }



   bool check_social( CHAR_DATA *ch, char *command, char *argument )
   {
      char arg[MAX_STRING_LENGTH];
      CHAR_DATA *victim;
      long cmd;
      bool found;
   
      found  = FALSE;
      for ( cmd = 0; social_table[cmd].name[0] != '\0'; cmd++ )
      {
         if ( command[0] == social_table[cmd].name[0]
            &&   !str_prefix( command, social_table[cmd].name ) )
         {
            found = TRUE;
            break;
         }
      }
   
      if ( !found )
         return FALSE;
   
      if ( !IS_NPC(ch) && IS_VAMPAFF(ch, VAM_EARTHMELDED))
      {
         send_to_char( "You cannot socialise while earthmelded!\n\r", ch );
         return TRUE;
      }
   
      if ( !IS_NPC(ch) && get_disc(ch,DISC_PROTEAN) >= 7 && 
         ch->position <= POS_SLEEPING)
      {
         send_to_char( "You use Movement of the Slowed body.\n\r", ch );
         WAIT_STATE(ch,12);
      }
      else 
         switch ( ch->position )
         {
            case POS_DEAD:
               send_to_char( "Lie still; you are DEAD.\n\r", ch );
               return TRUE;
            
            case POS_INCAP:
            case POS_MORTAL:
               send_to_char( "You are hurt far too bad for that.\n\r", ch );
               return TRUE;
            
            case POS_STUNNED:
               send_to_char( "You are too stunned to do that.\n\r", ch );
               return TRUE;
            
            case POS_SLEEPING:
            /*
            * I just know this is the path to a 12" 'if' statement.  :(
            * But two players asked for it already!  -- Furey
            */
               if ( !str_cmp( social_table[cmd].name, "snore" ) )
                  break;
               send_to_char( "In your dreams, or what?\n\r", ch );
               return TRUE;
         
         }
   
      one_argument( argument, arg );
      victim = NULL;
   
      if ( arg[0] == '\0' )
      {
         act( social_table[cmd].others_no_arg, ch, NULL, victim, TO_ROOM    );
         act( social_table[cmd].char_no_arg,   ch, NULL, victim, TO_CHAR    );
      }
      else if ( ( victim = get_char_room( ch, arg ) ) == NULL )
      {
         send_to_char( "They aren't here.\n\r", ch );
      }
      else if ( victim == ch )
      {
         act( social_table[cmd].others_auto,   ch, NULL, victim, TO_ROOM    );
         act( social_table[cmd].char_auto,     ch, NULL, victim, TO_CHAR    );
      }
      else
      {
         act( social_table[cmd].others_found,  ch, NULL, victim, TO_NOTVICT );
         act( social_table[cmd].char_found,    ch, NULL, victim, TO_CHAR    );
         act( social_table[cmd].vict_found,    ch, NULL, victim, TO_VICT    );
      
         if ( !IS_NPC(ch) && IS_NPC(victim)
            &&   !IS_AFFECTED(victim, AFF_CHARM)
            &&   !IS_EXTRA(victim, EXTRA_ILLUSION)
            &&   !IS_ITEMAFF(ch, ITEMA_PEACE)
            &&   can_see(victim,ch)
            &&   IS_AWAKE(victim) )
         {
               switch ( number_bits( 4 ) )
               {
                  default:
                  case 0:
                  case 1: case 2: case 3: case 4:
                  case 5: case 6: case 7: case 8:
                     act( social_table[cmd].others_found,
                        victim, NULL, ch, TO_NOTVICT );
                     act( social_table[cmd].char_found,
                        victim, NULL, ch, TO_CHAR    );
                     act( social_table[cmd].vict_found,
                        victim, NULL, ch, TO_VICT    );
                     break;
               
                  case 9: case 10: case 11: case 12:
                     act( "$n slaps $N.",  victim, NULL, ch, TO_NOTVICT );
                     act( "You slap $N.",  victim, NULL, ch, TO_CHAR    );
                     act( "$n slaps you.", victim, NULL, ch, TO_VICT    );
                     break;
               }
         }
      }
      return TRUE;
   }



/*
 * Return true if an argument is completely numeric.
 */
   bool is_number( char *arg )
   {
      if ( *arg == '\0' )
         return FALSE;
   
      for ( ; *arg != '\0'; arg++ )
      {
         if ( !isdigit(*arg) )
            return FALSE;
      }
   
      return TRUE;
   }



/*
 * Given a string like 14.foo, return 14 and 'foo'
 */
long number_argument(char *argument, char *arg)
{
    char *pdot;
    long number;

    if (*argument == '\0')
	return 0;
    for (pdot = argument; *pdot != '\0'; pdot++)
    {
	if (*pdot == '.')
	{
	    *pdot = '\0';
	    number = atoi(argument);
	    *pdot = '.';
	    strcpy(arg, pdot+1);
	    return number;
	}
    }
   
    strcpy(arg, argument);
    return 1;
}



/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.
 */
   char *one_argument( char *argument, char *arg_first )
   {
      char cEnd;
   
      while ( isspace(*argument) )
         argument++;
   
      cEnd = ' ';
      if ( *argument == '\'' || *argument == '"' )
         cEnd = *argument++;
   
      while ( *argument != '\0' )
      {
         if ( *argument == cEnd )
         {
            argument++;
            break;
         }
         *arg_first = LOWER(*argument);
         arg_first++;
         argument++;
      }
      *arg_first = '\0';
   
      while ( isspace(*argument) )
         argument++;
   
      return argument;
   }

/* Syntax is:
disable - shows disabled commands
disable <command> - toggles disable status of command
*/

   void do_disable (CHAR_DATA *ch, char *argument)
   {
      long i;
      DISABLED_DATA *p,*q;
      char buf[100];
   
      if (IS_NPC(ch))
      {
         send_to_char ("RETURN first.\n\r",ch);
         return;
      }
   
      if (!argument[0]) /* Nothing specified. Show disabled commands. */
      {
         if (!disabled_first) /* Any disabled at all ? */
         {
            send_to_char ("There are no commands disabled.\n\r",ch);
            return;
         }
      
         send_to_char ("Disabled commands:\n\r"
                      "Command      Level   Disabled by\n\r",ch);
      
         for (p = disabled_first; p; p = p->next)
         {
            sprintf (buf, "%-12s %5li %-12s\n\r",p->command->name,p->level, p->disabled_by);
            send_to_char (buf,ch);
         }
         return;
      }
   
        /* command given */
   
        /* First check if it is one of the disabled commands */
      for (p = disabled_first; p ; p = p->next)
         if (!str_cmp(argument, p->command->name))
            break;
   
      if (p) /* this command is disabled */
      {
        /* Optional: The level of the imm to enable the command must equal or exceed level
           of the one that disabled it */
      
         if (get_trust(ch) < p->level)
         {
            send_to_char ("This command was disabled by a higher power.\n\r",ch);
            return;
         }
      
                /* Remove */
      
         if (disabled_first == p) /* node to be removed == head ? */
            disabled_first = p->next;
         else /* Find the node before this one */
         {
            for (q = disabled_first; q->next != p; q = q->next); /* empty for */
            q->next = p->next;
         }
      
         free_string (p->disabled_by); /* free name of disabler */
         free_mem (p,sizeof(DISABLED_DATA)); /* free node */
         save_disabled(); /* save to disk */
         send_to_char ("Command enabled.\n\r",ch);
      }
      else /* not a disabled command, check if that command exists */
      {
                /* IQ test */
         if (!str_cmp(argument,"disable"))
         {
            send_to_char ("You cannot disable the disable command.\n\r",ch);
            return;
         }
      
                /* Search for the command */
         for (i = 0; cmd_table[i].name[0] != '\0'; i++)
            if (!str_cmp(cmd_table[i].name, argument))
               break;
      
                /* Found? */
         if (cmd_table[i].name[0] == '\0')
         {
            send_to_char ("No such command.\n\r",ch);
            return;
         }
      
                /* Can the imm use this command at all ? */
      
         if (cmd_table[i].level > get_trust(ch))
         {
            send_to_char ("You dot have access to that command; you cannot disable it.\n\r",ch);
            return;
         }
      
                /* Disable the command */
      
         p = alloc_mem (sizeof(DISABLED_DATA));
      
         p->command = &cmd_table[i];
         p->disabled_by = str_dup (ch->name); /* save name of disabler */
         p->level = get_trust(ch); /* save trust */
         p->next = disabled_first;
         disabled_first = p; /* add before the current first element */
      
         send_to_char ("Command disabled.\n\r",ch);
         save_disabled(); /* save to disk */
      }
   }

/* Check if that command is disabled
   Note that we check for equivalence of the do_fun pointers; this means
   that disabling 'chat' will also disable the '.' command
*/
   bool check_disabled (const struct cmd_type *command)
   {
      DISABLED_DATA *p;
   
      for (p = disabled_first; p ; p = p->next)
         if (p->command->do_fun == command->do_fun)
            return TRUE;
   
      return FALSE;
   }

/* Load disabled commands */
   void load_disabled()
   {
      FILE *fp;
      DISABLED_DATA *p;
      char *name;
      long i;
   
      disabled_first = NULL;
   
      fp = fopen (DISABLED_FILE, "r");
   
      if (!fp) /* No disabled file.. no disabled commands : */
         return;
   
      name = fread_word (fp);
   
      while (str_cmp(name, END_MARKER)) /* as long as name is NOT END_MARKER :) */
      {
                /* Find the command in the table */
         for (i = 0; cmd_table[i].name[0] ; i++)
            if (!str_cmp(cmd_table[i].name, name))
               break;
      
         if (!cmd_table[i].name[0]) /* command does not exist? */
         {
            bug ("Skipping uknown command in " DISABLED_FILE " file.",0);
            fread_number(fp); /* level */
            fread_word(fp); /* disabled_by */
         }
         else /* add new disabled command */
         {
            p = alloc_mem(sizeof(DISABLED_DATA));
            p->command = &cmd_table[i];
            p->level = fread_number(fp);
            p->disabled_by = str_dup(fread_word(fp));
            p->next = disabled_first;
         
            disabled_first = p;
         
         }
      
         name = fread_word(fp);
      }
   
      fclose (fp);
   }

/* Save disabled commands */
   void save_disabled()
   {
      FILE *fp;
      DISABLED_DATA *p;
   
      if (!disabled_first) /* delete file if no commands are disabled */
      {
         unlink (DISABLED_FILE);
         return;
      }
   
      fp = fopen (DISABLED_FILE, "w");
   
      if (!fp)
      {
         bug ("Could not open " DISABLED_FILE " for writing",0);
         return;
      }
   
      for (p = disabled_first; p ; p = p->next)
         fprintf (fp, "%s %li %s\n", p->command->name, p->level, p->disabled_by);
   
      fprintf (fp, "%s\n",END_MARKER);
   
      fclose (fp);
   }

   void load_bans() {
      FILE *fp;
      BAN_DATA *ban_last;
   
      ban_last = NULL;
      fp = fopen ("ban.txt", "r");
   
      if (!fp) 
         return;
   
      for ( ; ; ) {
         BAN_DATA *p;
         if (feof(fp))  {
            fclose(fp);
            return;}
      
         p = alloc_mem(sizeof(BAN_DATA));
         p->name = str_dup(fread_word(fp));
         fread_to_eol(fp);
         if (ban_list == NULL)
            ban_list = p;
         else
            ban_last->next = p;
         ban_last = p;
      }
   /*
   
        while (str_cmp(name, END_MARKER)) {
                p = alloc_mem(sizeof(BAN_DATA));
                p->name = name;
                p->next = ban_first;
                ban_first = p;
                name = fread_word(fp);}
   
        fclose(fp);*/
   }

   void save_bans(void)
   {
      FILE *fp;
      BAN_DATA *p;
   
      if (!ban_list)       {
         unlink ("ban.txt");
         return;
      }
   
      fclose(fpReserve);
      fp = fopen ("ban.txt", "w");
   
      if (!fp) {bug("could not open ban.txt",0);
         return;}
   
      for (p = ban_list; p != NULL; p = p->next) {
         fprintf(fp, "%s\n", p->name);}
      fclose(fp);
      fpReserve = fopen(NULL_FILE, "r");
   
   }
