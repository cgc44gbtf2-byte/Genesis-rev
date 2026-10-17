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

#include "mage.h"
#include "memwatch.h"

/*
 * Accommodate old non-Ansi compilers.
 */
#if defined(TRADITIONAL)
#define const
#define args( list )			( )
#define DECLARE_DO_FUN( fun )		void fun( )
#define DECLARE_SPEC_FUN( fun )		bool fun( )
#define DECLARE_SPELL_FUN( fun )	void fun( )
#else
#define args( list )			list
#define DECLARE_DO_FUN( fun )		DO_FUN    fun
#define DECLARE_SPEC_FUN( fun )		SPEC_FUN  fun
#define DECLARE_SPELL_FUN( fun )	SPELL_FUN fun
#endif



/*
 * Short scalar types.
 * Diavolo reports AIX compiler has bugs with short types.
 */
#if	!defined(FALSE)
#define FALSE	 0
#endif

#if	!defined(TRUE)
#define TRUE	 1
#endif

#if	defined(_AIX)
#if	!defined(const)
#define const
#endif
typedef long                             sh_int;
typedef long				bool;
#define unix
#else
typedef short    int			sh_int;
typedef unsigned char			bool;
#endif



/*
 * Structure types.
 */
typedef struct	affect_data		AFFECT_DATA;
typedef struct	area_data		AREA_DATA;
typedef struct	ban_data		BAN_DATA;
typedef struct  buf_type		BUFFER;
typedef struct	char_data		CHAR_DATA;
typedef struct	descriptor_data		DESCRIPTOR_DATA;
typedef struct	exit_data		EXIT_DATA;
typedef struct	extra_descr_data	EXTRA_DESCR_DATA;
typedef struct	help_data		HELP_DATA;
typedef struct	mob_index_data		MOB_INDEX_DATA;
typedef struct	note_data		NOTE_DATA;
typedef struct	obj_data		OBJ_DATA;
typedef struct	obj_index_data		OBJ_INDEX_DATA;
typedef struct	pc_data			PC_DATA;
typedef struct	reset_data		RESET_DATA;
typedef struct	room_index_data		ROOM_INDEX_DATA;
typedef	struct	trap_data		TRAP_DATA;
typedef struct	shop_data		SHOP_DATA;
typedef struct	time_info_data		TIME_INFO_DATA;
typedef struct	weather_data		WEATHER_DATA;
typedef struct  editing_data            EDITING_DATA;	/* OLC */
typedef struct  disabled_data		DISABLED_DATA;
typedef struct  exp_data		EXP_DATA;
typedef struct  l_board                 LEADER_BOARD;

/*
 * Function types.
 */
typedef	void DO_FUN	args( ( CHAR_DATA *ch, char *argument ) );
typedef bool SPEC_FUN	args( ( CHAR_DATA *ch ) );
typedef void SPELL_FUN	args( ( long sn, long level, CHAR_DATA *ch, void *vo ) );



/*
 * String and memory management parameters.
 */
#define	MAX_KEY_HASH		 1024
#define MAX_STRING_LENGTH	 4096
#define MAX_INPUT_LENGTH	  400

#define SCREEN_WIDTH            79

#include "board.h"

/*
 * Game parameters.
 * Increase the max'es if you add more of something.
 * Adjust the pulse numbers to suit yourself.
 */
#define AGE_PK			30 /* This age = can pk */
#define MAX_SKILL		132
#define MAX_SPELL		58
#define MAX_LEVEL		7
#define LEV_ST			(MAX_LEVEL)
#define LEV_IMP			(LEV_ST - 1)
#define LEV_IMM			(LEV_IMP - 1)
#define LEV_BUI			(LEV_IMM - 1)
#define LEV_SOR			(LEV_BUI - 1)
#define LEV_AWA			(LEV_SOR - 1)
#define LEV_SLE			(LEV_AWA - 1)

#define PULSE_PER_SECOND	4
#define PULSE_VIOLENCE		1
#define PULSE_MOBILE		(2 * PULSE_PER_SECOND)
#define PULSE_TICK		(60 * PULSE_PER_SECOND)
#define PULSE_AREA		(20 * PULSE_PER_SECOND)

/* Stun Fist timings are server pulses, not descriptor command lag. */
#define STUN_FIST_COST             5000
#define STUN_FIST_DURATION         PULSE_PER_SECOND
#define STUN_FIST_RECOVERY         (2 * PULSE_PER_SECOND)
#define STUN_FIST_IMMUNITY         (6 * PULSE_PER_SECOND)

/*
 * Site ban structure.
 */
struct	ban_data
{
    BAN_DATA *	next;
    char *	name;
};

struct buf_type
{
    BUFFER *    next;
    bool        valid;
    sh_int      state;  /* error state of the buffer */
    sh_int      size;   /* size in k */
    char *      string; /* buffer's string */
};


/*
 * Time and weather stuff.
 */
#define SUN_DARK		    0
#define SUN_RISE		    1
#define SUN_LIGHT		    2
#define SUN_SET			    3

#define SKY_CLOUDLESS		    0
#define SKY_CLOUDY		    1
#define SKY_RAINING		    2
#define SKY_LIGHTNING		    3

struct	time_info_data
{
    long		hour;
    long		day;
    long		month;
    long		year;
};

struct	weather_data
{
    long		mmhg;
    long		change;
    long		sky;
    long		sunlight;
};



/*
 * Connected state for a 'channel.
 */
#define CON_PLAYING			 0 /* IN: MOTD, OUT: BEGIN PLAY */
#define CON_GET_NAME			 1 /* IN: NONE, OUT: Enter name */
#define CON_GET_OLD_PASSWORD		 2 /* IN: Name, OUT: Show MOTD */
#define CON_CONFIRM_NEW_NAME		 3 /* IN: Sure, OUT: Password */
#define CON_GET_NEW_PASSWORD		 4 /* IN: Pass, OUT: Confirm */
#define CON_CONFIRM_NEW_PASSWORD	 5 /* IN: Pass, OUT: What sex */
#define CON_GET_NEW_SEX			 6 /* IN: Sex,  OUT: Start as vamp */
#define CON_READ_MOTD			 7 /* IN: NONE, OUT: Play */
#define CON_COPYOVER_RECOVER		 8
#define CON_GET_INVIS 			 9
#define CON_GET_NEW_ANSI                15 /* Prevent conflict with board.h set value higher */
/*
 * Connected states for OLC.
 */
#define CON_AEDITOR                     -1
#define CON_REDITOR                     -2
#define CON_MEDITOR                     -3
#define CON_OEDITOR                     -4


/*
 * Online editing structure.
 */
struct  editing_data	/* OLC */
{
    void *		pIndex;
    long		vnum;
    long		type;
    long		command;
    char *		text;
    char **		destination;
};

/*
 * Area flags. OLC
 */
#define         AREA_NONE       0
#define         AREA_CHANGED    1	/* Area has been modified. */
#define         AREA_ADDED      2	/* Area has been added to. */
#define         AREA_LOADING    4	/* Used for counting in db.c */


/*
 * Descriptor (channel) structure.
 */
struct	descriptor_data
{
    DESCRIPTOR_DATA *	next;
    DESCRIPTOR_DATA *	snoop_by;
    CHAR_DATA *		character;
    CHAR_DATA *		original;
    char *		host;
    long		descriptor;
    long		connected;
    bool		fcommand;
    bool                password_save_pending;
    char		inbuf		[4 * MAX_INPUT_LENGTH];
    char		incomm		[MAX_INPUT_LENGTH];
    char		inlast		[MAX_INPUT_LENGTH];
    long		repeat;
    char *		outbuf;
    long		outsize;
    long		outtop;
    EDITING_DATA        editing;	/* OLC */
    void *              pEdit;		/* OLC */
    char **             pString;	/* OLC */
};



/* Expmod Structure */
struct exp_data
{
    long exp;
    long time;
    long bit;
};

#define	EXP_RANDOM		1
#define	EXP_DEGRADE		2


/*
 * TO types for act.
 */
#define TO_ROOM			0
#define TO_NOTVICT		1
#define TO_VICT			2
#define TO_CHAR			3


/*
 * Help table types.
 */
struct	help_data
{
    HELP_DATA *	next;
    long	level;
    char *	keyword;
    char *	text;
};



/*
 * Shop types.
 */
#define MAX_TRADE	 5

struct	shop_data
{
    SHOP_DATA *	next;			/* Next shop in list		*/
    long	keeper;			/* Vnum of shop keeper mob	*/
    long	buy_type [MAX_TRADE];	/* Item types shop will buy	*/
    long	profit_buy;		/* Cost multiplier for buying	*/
    long	profit_sell;		/* Cost multiplier for selling	*/
    long	open_hour;		/* First opening hour		*/
    long    	close_hour;		/* First closing hour		*/
};



/*
 * Data structure for notes.
 */
struct	note_data
{
    NOTE_DATA *	next;
    char *	sender;
    char *	date;
    char *	to_list;
    char *	subject;
    char *	text;
    time_t      date_stamp;
    time_t      expire;
};



/*
 * An affect.
 */
struct	affect_data
{
    AFFECT_DATA *	next;
    long		type;
    long    		duration;
    long		location;
    long		modifier;
    long		bitvector;
};



/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (Start of section ... start here)                     *
 *                                                                         *
 ***************************************************************************/

/*
 * Flag Bits.
 */
#define FLAG_RP			1
#define FLAG_PK			2
#define FLAG_TEST		4
#define FLAG_AFK		8
#define FLAG_PIMP		16
#define FLAG_KINKY		32
#define FLAG_CCC		64
#define FLAG_LAZY		128


/*
 * Stats.
 */
#define STAT_STR		0
#define STAT_DEX		1
#define STAT_CON		2
#define STAT_INT		3
#define STAT_WIS		4


/*
 * Classes.  KaVir.  Note to self(Akuma): these don't have to be "bits".
 */
#define CLASS_NONE	      	0
#define CLASS_VAMPIRE	      	1
#define CLASS_WEREWOLF	      	2
#define CLASS_SORCERER		4
#define CLASS_DROW		8
#define CLASS_HIGHLANDER        16
#define CLASS_MAGE		32
#define CLASS_DRAGON		64
#define CLASS_NINJA             128
#define CLASS_MONK              256
#define CLASS_ELEM              512


/*
 * ACT bits for mobs.
 * Used in #MOBILES.
 */
#define ACT_IS_NPC		1	/* Auto set for mobs	*/
#define ACT_COMMANDED		2	/* Can't attack newbies */
#define ACT_NOKILL		4	/* No Killing		*/
#define ACT_NOEXP		8	/* No exp for killing	*/
#define ACT_SENTINEL		16	/* Stays in one room	*/
#define ACT_STAY_AREA		32	/* Won't leave area	*/
#define ACT_AGGRESSIVE		64	/* Attacks PC's		*/
#define ACT_SCAVENGER		128	/* Picks up objects	*/
#define ACT_SKILLED		256	/* Mobile uses stances	*/



/*
 * Bits for 'affected_by'.
 * Used in #MOBILES.
 */
#define AFF_BLIND		      1
#define AFF_INVISIBLE		      2

#define AFF_DETECT_INVIS	      8

#define AFF_SHADOWPLANE		     64 /* Creatures in shadow plane - KaVir */
#define AFF_SANCTUARY		    128
#define AFF_FAERIE_FIRE		    256
#define AFF_INFRARED		    512
#define AFF_CURSE		   1024
#define AFF_FLAMING		   2048 /* For burning creatures - KaVir */
#define AFF_POISON		   4096

#define AFF_ETHEREAL		  16384 /* For ethereal creatures - KaVir */
#define AFF_SNEAK		  32768
#define AFF_HIDE		  65536
#define AFF_SLEEP		 131072
#define AFF_CHARM		 262144
#define AFF_FLYING		 524288
#define AFF_PASS_DOOR		1048576
#define AFF_POLYMORPH		2097152 /* For polymorphed creatures - KaVir */
#define AFF_SHADOWSIGHT		4194304 /* Can see between planes - KaVir */
#define AFF_WEBBED		8388608 /* Cannot move - KaVir */
#define AFF_SHADOW             33554432 /* For Arms of the Abyss - KaVir */   

#define AFF_FEAR	      134217728 /* Fear affects - KaVir */
#define AFF_SUCCULENCE	      268435456 /* Succulence, Animalism 5 */
#define AFF_DEEPUMBRA	      536870912 /* Mage's breaking the dreamshell - Dude */
#define AFF_KAKUSU        1073741824 /* Kakusu, Ninja hide */

/*
 * Bits for 'itemaffect'.
 * Used in #MOBILES.
 */
#define ITEMA_SHOCKSHIELD	1
#define ITEMA_FIRESHIELD	2
#define ITEMA_ICESHIELD		4
#define ITEMA_ACIDSHIELD	8
#define ITEMA_HASTE		16
#define ITEMA_VORPAL		32
#define ITEMA_ARTIFACT		64
#define ITEMA_REGENERATE	128
#define ITEMA_SPEED		256
#define ITEMA_VISION		512
#define ITEMA_PEACE		1024
#define ITEMA_RIGHT_SILVER	2048
#define ITEMA_LEFT_SILVER	4096
#define ITEMA_REFLECT		8192
#define ITEMA_RESISTANCE	16384


#define ITEMA_VANISH		131072



#define ITEMA_SUNSHIELD		2097152

#define ITEMA_BLISSFUL		16777216
#define ITEMA_FLAMING		33554432 /* Fire Blade: Elementalist */
#define ITEMA_AURA		67108864 /* Holy Aura: Elementalist */
#define ITEMA_AQUABARRIER	134217728 /* Aqua Barrier: Elementalist */


/*
 * Advanced spells.
 */
#define ADV_DAMAGE		1
#define ADV_AFFECT		2
#define ADV_ACTION		4
#define ADV_NEXT_PAGE		8
#define ADV_REVERSED		16
#define ADV_STARTED		32
#define ADV_FINISHED		64
#define ADV_FAILED		128
#define ADV_MESSAGE_1		256
#define ADV_MESSAGE_2		512
#define ADV_MESSAGE_3		1024

#define MIL (MAX_INPUT_LENGTH)
#define MSL (MAX_STRING_LENGTH)

/*
 * Advanced spell affects.
 */
#define ADV_HIT			1
#define ADV_MANA		2
#define ADV_AC			4
#define ADV_HITROLL		8
#define ADV_DAMROLL		16
#define ADV_SAVING_SPELL	32

/*
 * Colour scale macros - added 12th Aug 1995 by Calamar
 */
#define NO_COLOUR	""		/* Blank */
#define GREY		"[0;1;30m"	/* Dark Grey */
#define D_RED		"[0;0;31m"	/* Dark Red */
#define L_RED		"[0;1;31m"	/* Light Red */
#define D_GREEN		"[0;0;32m"	/* Dark Green */
#define L_GREEN		"[0;1;32m"	/* Light Green */
#define BROWN		"[0;0;33m"	/* Brown */
#define YELLOW		"[0;1;33m"	/* Yellow */
#define D_BLUE		"[0;0;34m"	/* Dark Blue */
#define L_BLUE		"[0;1;34m"	/* Light Blue */
#define MAGENTA		"[0;0;35m"	/* Magenta */
#define PINK		"[0;1;35m"	/* Pink */
#define D_CYAN		"[0;0;36m"	/* Dark Cyan */
#define L_CYAN		"[0;1;36m"	/* Light Cyan */
#define NORMAL		"[0;0;37m"	/* Light Grey */
#define WHITE		"[0;0;38m"	/* White */

#define ADD_COLOUR(_player,_str,_col) { long swh_len = strlen(_str) + 20; \
					char swh_temp[swh_len]; \
if (!IS_NPC(_player) && IS_SET(_player->act, PLR_ANSI)) {swh_temp[0] = '\0'; \
strcpy(swh_temp, _col); strcat(swh_temp, _str); strcat(swh_temp, NORMAL); \
strcpy(_str, swh_temp);}} 

#define SCALE_COLS 4

#define COL_SCALE(_swh_str,_swh_ch,_swh_curr,_swh_max) \
	ADD_COLOUR(_swh_ch, _swh_str, \
	(_swh_curr < 1) ? L_RED : \
	(_swh_curr < _swh_max) ? \
	scale[(SCALE_COLS * _swh_curr) / ((_swh_max > 0) \
	? _swh_max : 1)] : L_CYAN)

extern char *scale[SCALE_COLS];

extern CHAR_DATA Score_ch;      /* Score_ch */

/*
 * File bits.
 */
#define FILE_PLAYER		      0

/*
 * Bits for 'vampire'.
 * Used for player vampires.
 */
#define VAM_FANGS		      1 /* Fangs currently extended */
#define VAM_CLAWS		      2 /* Protean level 2, claws extended */
#define VAM_NIGHTSIGHT		      4 /* Protean level 1, nightsight on */
#define VAM_FLYING		      8 /* Protean bat form */
#define VAM_SONIC		     16 /* Protean bat form */
#define VAM_CHANGED		     32 /* Protean powers level 4 and 5 */
#define VAM_SPIRITEYES		     64 /* Path of Spirit level 2 */
#define VAM_OBFUSCATE		    128 /* Concealed from sight */

#define VAM_DISGUISED		  16384 /* For the Obfuscate disguise ability */

#define VAM_EARTHMELDED		 524288 /* Protean level 3, in the ground */

/*
 * Bits for vicissitude body modifications.
 * Used for player vampires.
 */
#define CRAFT_MALLEABLE		1	/* Masked through Vicissitude */
#define CRAFT_FLESHSOAK		2	/* +Soak */
#define CRAFT_BONETALONS	4	/* Aggy claws */
#define CRAFT_BONEQUILLS	8	/* Counter hit */

/*
 * Bits for disciplines.
 * Used for player vampires.
 */
#define RES_ANIMALISM		      1
#define RES_AUSPEX		      2
#define RES_DOMINATE		      4
#define RES_PRESENCE		      8
#define RES_TOGGLE		     16
#define RES_CELERITY		     32

#define WILL_ANIMALISM		      1
#define WILL_AUSPEX		      2
#define WILL_DOMINATE		      3
#define WILL_PRESENCE		      4

/*#define CLAN_FIRST		      0
#define CLAN_SECOND		      1
#define CLAN_THIRD		      2*/

#define DISC_ANIMALISM		      0
#define DISC_AUSPEX		      1
#define DISC_CELERITY		      2
#define DISC_CHIMERSTRY		      3

#define DISC_DOMINATE		      5
#define DISC_FORTITUDE		      6
#define DISC_NECROMANCY		      7

#define DISC_OBFUSCATE		      9
#define DISC_OBTENEBRATION	     10
#define DISC_POTENCE		     11
#define DISC_PRESENCE		     12
#define DISC_PROTEAN		     13
#define DISC_QUIETUS		     14
#define DISC_SERPENTIS		     15

#define DISC_THAUMATURGY	     17
#define DISC_VICISSITUDE	     18

#define DISC_MAX		     19

#define BREED_HOMID		      0
#define BREED_LUPUS		      1
#define BREED_METIS		      2
#define BREED_MAX		      2

#define AUSPICE_RAGABASH	      0
#define AUSPICE_THEURGE		      1
#define AUSPICE_PHILODOX	      2
#define AUSPICE_GALLIARD	      3
#define AUSPICE_AHROUN		      4
#define AUSPICE_MAX		      4

#define TRIBE_BLACK_FURIES	      0
#define TRIBE_BONE_GNAWERS	      1
#define TRIBE_CHILDREN_OF_GAIA	      2
#define TRIBE_FIANNA		      3
#define TRIBE_GET_OF_FENRIS	      4
#define TRIBE_GLASS_WALKERS	      5
#define TRIBE_RED_TALONS	      6
#define TRIBE_SHADOW_LORDS	      7
#define TRIBE_SILENT_STRIDERS	      8
#define TRIBE_SILVER_FANGS	      9
#define TRIBE_STARGAZERS	      10
#define TRIBE_UKTENA		      11
#define TRIBE_WENDIGOS		      12
#define TRIBE_MAX		      12


/*
 * World affect bits.
 */
#define WORLD_FOG		      1
#define WORLD_RAIN		      2
#define WORLD_ECLIPSE		      4


/*
 * Bits for blood.
 * Used for player vampires.
 */
#define BLOOD_CURRENT		      0
#define BLOOD_POOL		      1

/*
 * Bits for 'polymorph'.
 * Used for players.
 */
#define POLY_BAT		      1
#define POLY_WOLF		      2
#define POLY_MIST		      4
#define POLY_SERPENT		      8
#define POLY_RAVEN		     16
#define POLY_FISH		     32
#define POLY_FROG		     64
#define POLY_ZULO		    128
#define POLY_SHADOW		    256
#define POLY_SPIRIT		    512
#define POLY_MASK		   1024 /* Vicissitude 3 */
#define POLY_TRUE		   2048 /* Chimerstry 6 */

#define POLY_DRAGONFORM		  65536
#define POLY_PHOENIXFORM	 131072
#define POLY_CHIRO		 262144
#define POLY_CHIMERA	 	 524288	/* Monk Chimera Form */
#define POLY_PIG		1048576

/*
 * Score.
 */
#define SCORE_TOTAL_XP		      0
#define SCORE_HIGH_XP		      1
#define SCORE_TOTAL_LEVEL	      2
#define SCORE_HIGH_LEVEL	      3
#define SCORE_QUEST		      4
#define SCORE_NUM_QUEST		      5


/*
 * Drow bits
 */
#define CP_CURRENT		0
#define CP_TOTAL		1

#define DROW_POWERS		0
#define DROW_SPEED		1

#define DROW_ACID		1
#define DROW_WEB		2
#define DROW_DARK		4
#define DROW_FIRE		8
#define DROW_BITE		16
#define DROW_DUALWIELD		32
#define DROW_BFIGHT		64
#define DROW_SPIDER		128
#define DROW_EYES		256
#define DROW_READ		512
#define DROW_NIGHT		1024
#define DROW_SIGHT		2048
#define DROW_PIMM		4096
#define DROW_SKIP		8192
#define DROW_ATTUNE             16384
#define DROW_MIGHT              32768
#define DROW_TOUGH              65536
#define DROW_MAX		65536

#define DROW_GLOBE		262144


/*
 * Highlander Bits
 */
#define HIGH_WEAPON		0
#define HIGH_ATTACK		1
#define HIGH_DEFENSE		2
#define HIGH_ABILITY		3


/*
 * Dragon Bits
 */
#define DRAG_FANGS		1
#define DRAG_NIGHT		2
#define DRAG_HORNS		4
#define DRAG_CLAWS		8
#define DRAG_WINGS		16
#define DRAG_BREATH		32
#define DRAG_TAIL		64
#define DRAG_TRUESIGHT		128
#define DRAG_DUST		256
#define DRAG_SCRY		512
#define DRAG_MAGIC		1024
#define DRAG_READAURA		2048
#define DRAG_SHIELD		4096

#define DRAG_WINGSOUT		16384
#define DRAG_HORNSOUT		32768

#define AGE_HATCHLING		12
#define AGE_VYOUNG		11
#define AGE_YOUNG		10
#define AGE_JUVENILE		9
#define AGE_YADULT		8
#define AGE_ADULT		7
#define AGE_MATURE		6
#define AGE_OLD			5
#define AGE_VOLD		4
#define AGE_ANCIENT		3
#define AGE_WYRM		2
#define AGE_AWYRM		1



/*
 * Monk Bits
*/
#define MONK_TECH	0
#define MONK_MANTRA	1
#define MONK_ART	2


/*
 * Ninja Bits
 */
#define NINJA_SORA		0
#define NINJA_CHIKYU		1
#define NINJA_NINGENNO		2
#define NINJA_KI		3
#define NINJA_HARAKIRI		4

/*
 * Elementalist Bits
 */
#define ELEM_CHOICE                  0
#define ELEM_FIRE                    1
#define ELEM_WATER                   2
#define ELEM_EARTH                   3
#define ELEM_WIND                    4
#define ELEM_SUN                     5
#define ELEM_MOON                    6
#define ELEM_TIME                    7
#define ELEM_INFIN                   8

/*
* Mage bits
*/
#define SPHERE_CORRESPONDENCE		0
#define SPHERE_LIFE                     1
#define SPHERE_PRIME                    2
#define SPHERE_ENTROPY			3
#define SPHERE_MIND                     4
#define SPHERE_SPIRIT                   5
#define SPHERE_FORCES                   6
#define SPHERE_MATTER                   7
#define SPHERE_TIME                     8

/*
 * Embrace positions.
 */
#define ARE_NONE		      0
#define ARE_EMBRACING		      1
#define ARE_EMBRACED		      2



/*
 * Sex.
 * Used in #MOBILES.
 */
#define SEX_NEUTRAL		      0
#define SEX_MALE		      1
#define SEX_FEMALE		      2



/*
 * Well known object virtual numbers.
 * Defined in #OBJECTS.
 */
#define OBJ_VNUM_MONEY_ONE	      2
#define OBJ_VNUM_MONEY_SOME	      3
#define OBJ_VNUM_CORPSE_NPC	     10
#define OBJ_VNUM_CORPSE_PC	     11
#define OBJ_VNUM_SEVERED_HEAD	     12

#define OBJ_VNUM_MUSHROOM	     20
#define OBJ_VNUM_LIGHT_BALL	     21
#define OBJ_VNUM_SPRING		     22

/* For KaVir's stuff */
#define OBJ_VNUM_SOULBLADE	  30000
#define OBJ_VNUM_PORTAL		  30001

#define OBJ_VNUM_SPILT_BLOOD      30007
#define OBJ_VNUM_VOODOO_DOLL      30010

#define OBJ_VNUM_PROTOPLASM	  30037
#define OBJ_VNUM_QUESTCARD	  30039
#define OBJ_VNUM_QUESTMACHINE	  30040

#define MOB_VNUM_GUARDIAN	  30001
#define MOB_VNUM_MOUNT		  30006
#define MOB_VNUM_FROG		  30007
#define MOB_VNUM_RAVEN		  30008
#define MOB_VNUM_CAT		  30009
#define MOB_VNUM_DOG		  30010
#define MOB_VNUM_EYE		  30012
#define MOB_VNUM_MOLERAT	  30013
#define MOB_VNUM_YAK		  30014
#define MOB_VNUM_ASTRAL		  30015
#define MOB_VNUM_CLONE		  30016



/*
 * Item types.
 * Used in #OBJECTS.
 */
#define ITEM_LIGHT		      1
#define ITEM_SCROLL		      2
#define ITEM_WAND		      3
#define ITEM_STAFF		      4
#define ITEM_WEAPON		      5
#define ITEM_PTOKEN	    	      6
#define ITEM_TREASURE		      8
#define ITEM_ARMOR		      9
#define ITEM_POTION		     10
#define ITEM_FURNITURE		     12
#define ITEM_TRASH		     13
#define ITEM_CONTAINER		     15

#define ITEM_DRINK_CON		     17
#define ITEM_KEY		     18
#define ITEM_FOOD		     19
#define ITEM_MONEY		     20
#define ITEM_BOAT		     22
#define ITEM_CORPSE_NPC		     23
#define ITEM_CORPSE_PC		     24
#define ITEM_FOUNTAIN		     25
#define ITEM_PILL		     26
#define ITEM_PORTAL		     27
#define ITEM_EGG		     28
#define ITEM_VOODOO		     29
#define ITEM_STAKE		     30
#define ITEM_MISSILE		     31 /* Ammo vnum, cur, max, type */
#define ITEM_AMMO		     32 /* ???, dam min, dam max, type */
#define ITEM_QUEST		     33
#define ITEM_QUESTCARD		     34
#define ITEM_QUESTMACHINE	     35

#define ITEM_BOOK		     37
#define ITEM_PAGE		     38
#define ITEM_PEN		     39
#define ITEM_WALL		     40 /* For wall of water */

#define ITEM_WGATE                   42
#define ITEM_SIT_SINGLE		     43 //For chairs  sits one
#define ITEM_SIT_DOUBLE		     44 //For love seats sits two
#define ITEM_SIT_TRIPLE		     45 //For Couches  sits three

/*
 * Extra flags.
 * Used in #OBJECTS.
 */
#define ITEM_GLOW		      1
#define ITEM_HUM		      2
#define ITEM_THROWN		      4
#define ITEM_KEEP		      8
#define ITEM_VANISH		     16 /* Illusion created by Chimerstry */
#define ITEM_INVIS		     32
#define ITEM_MAGIC		     64
#define ITEM_NODROP		    128
#define ITEM_BLESS		    256
#define ITEM_ANTI_GOOD		    512
#define ITEM_ANTI_EVIL		   1024
#define ITEM_ANTI_NEUTRAL	   2048
#define ITEM_NOREMOVE		   4096
#define ITEM_INVENTORY		   8192
#define ITEM_LOYAL		  16384
#define ITEM_SHADOWPLANE	  32768


/*
 * Wear flags.
 * Used in #OBJECTS.
 */
#define ITEM_TAKE		1
#define ITEM_WEAR_FINGER	2
#define ITEM_WEAR_NECK		4
#define ITEM_WEAR_BODY		8
#define ITEM_WEAR_HEAD		16
#define ITEM_WEAR_LEGS		32
#define ITEM_WEAR_FEET		64
#define ITEM_WEAR_HANDS		128 
#define ITEM_WEAR_ARMS		256

#define ITEM_WEAR_ABOUT		1024 
#define ITEM_WEAR_WAIST		2048
#define ITEM_WEAR_WRIST		4096
#define ITEM_TEMP		8192
#define ITEM_WEAR_HOLD		16384
#define ITEM_WEAR_FACE		32768



/*
 * Special types.
 * Used in #OBJECTS for special items - KaVir.
 */
#define SITEM_ACTIVATE		1
#define SITEM_TWIST		2
#define SITEM_PRESS		4
#define SITEM_PULL		8
#define SITEM_TARGET		16
#define SITEM_SPELL		32
#define SITEM_TRANSPORTER	64
#define SITEM_TELEPORTER	128
#define SITEM_DELAY1		256
#define SITEM_DELAY2		512
#define SITEM_OBJECT		1024
#define SITEM_MOBILE		2048
#define SITEM_ACTION		4096

#define SITEM_SILVER		16384

#define SITEM_MAGE		65536

#define SITEM_MONK		262144
#define SITEM_HIGHLANDER	524288
#define SITEM_ELEM		1048576
#define SITEM_VAMPIRE		2097152
#define SITEM_WEREWOLF		4194304
#define SITEM_NINJA		8388608
#define SITEM_DROW		16777216


/*
 * Apply types (for quest affects).
 * Used in #OBJECTS.
 */

#define QUEST_HITROLL		32
#define QUEST_DAMROLL		64
#define QUEST_HIT		128
#define QUEST_MANA		256

#define QUEST_AC		1024

#define QUEST_ENCHANTED		32768
#define QUEST_SORCENCHANT	65536
#define QUEST_ARTIFACT		131072
#define QUEST_IMPROVED		262144
#define QUEST_AGGRAVATED	524288
#define QUEST_RELIC		1048576
#define QUEST_VANISH		2097152 /* Object that doesn't save */


/*
 * Apply types (for affects).
 * Used in #OBJECTS.
 */
#define APPLY_NONE		      0
#define APPLY_STR		      1
#define APPLY_DEX		      2
#define APPLY_INT		      3
#define APPLY_WIS		      4
#define APPLY_CON		      5
#define APPLY_SEX		      6
#define APPLY_CLASS		      7
#define APPLY_LEVEL		      8
#define APPLY_AGE		      9
#define APPLY_HEIGHT		     10
#define APPLY_WEIGHT		     11
#define APPLY_MANA		     12
#define APPLY_HIT		     13
#define APPLY_MOVE		     14
#define APPLY_GOLD		     15
#define APPLY_EXP		     16
#define APPLY_AC		     17
#define APPLY_HITROLL		     18
#define APPLY_DAMROLL		     19
#define APPLY_SAVING_PARA	     20
#define APPLY_SAVING_ROD	     21
#define APPLY_SAVING_PETRI	     22
#define APPLY_SAVING_BREATH	     23
#define APPLY_SAVING_SPELL	     24
#define APPLY_POLY		     25
#define APPLY_BLOOD_MAX               26

/*
 * Absorbed things
 */
#define ABS_MOB_VNUM		      0 /* Vnum of Flesh Bonded mob */
#define ABS_MOB_HP		      1 /* Hp of Flesh Bonded mob */
#define ABS_MOB_MAX_HP		      2 /* Vnum of Object 1 */


/*
 * Values for containers (value[1]).
 * Used in #OBJECTS.
 */
#define CONT_CLOSEABLE		      1
#define CONT_PICKPROOF		      2
#define CONT_CLOSED		      4
#define CONT_LOCKED		      8



/*
 * Well known room virtual numbers.
 * Defined in #ROOMS.
 */
#define ROOM_VNUM_LIMBO		      2
#define ROOM_VNUM_CHAT		   1200
#define ROOM_VNUM_TEMPLE	   3001
#define ROOM_VNUM_FREEZE	   3014
#define ROOM_VNUM_ALTAR		   3054
#define ROOM_VNUM_SCHOOL	   3700
#define ROOM_VNUM_HELL		  30000
#define ROOM_VNUM_CRYPT		  30001
#define ROOM_VNUM_DISCONNECTION	  30002
#define ROOM_VNUM_IN_OBJECT	  30008


/*
 * Room flags.
 * Used in #ROOMS.
 */
#define ROOM_DARK		1

#define ROOM_NO_MOB		4
#define ROOM_INDOORS		8
#define ROOM_PRIVATE		512
#define ROOM_SAFE		1024
#define ROOM_SOLITARY		2048

#define ROOM_NO_RECALL		8192
#define ROOM_NO_TELEPORT	16384

/*
 * Added Room Flags.
 * Toggled by player abilities.
 */
#define ROOM2_DARKNESS		1
#define ROOM2_SILENCE		2


/*
 * Room text flags (KaVir).
 * Used in #ROOMS.
 */
#define RT_LIGHTS		      1 /* Toggles lights on/off */
#define RT_SAY			      2 /* Use this if no others powers */
#define RT_ENTER		      4
#define RT_CAST			      8
#define RT_THROWOUT		     16 /* Erm...can't remember ;) */
#define RT_OBJECT		     32 /* Creates an object */
#define RT_MOBILE		     64 /* Creates a mobile */
#define RT_LIGHT		    128 /* Lights on ONLY */
#define RT_DARK			    256 /* Lights off ONLY */
#define RT_OPEN_LIFT		    512 /* Open lift */
#define RT_CLOSE_LIFT		   1024 /* Close lift */
#define RT_MOVE_LIFT		   2048 /* Move lift */
#define RT_SPELL		   4096 /* Cast a spell */
#define RT_PORTAL		   8192 /* Creates a one-way portal */
#define RT_TELEPORT		  16384 /* Teleport player to room */

#define RT_ACTION		  32768
#define RT_BLANK_1		  65536
#define RT_BLANK_2		 131072

#define RT_RETURN		1048576 /* Perform once */
#define RT_PERSONAL		2097152 /* Only shows message to char */
#define RT_TIMER		4194304 /* Sets object timer to 1 tick */



/*
 * Directions.
 * Used in #ROOMS.
 */
#define DIR_NORTH		      0
#define DIR_EAST		      1
#define DIR_SOUTH		      2
#define DIR_WEST		      3
#define DIR_UP			      4
#define DIR_DOWN		      5
#define MAX_DIR			      6	/* OLC */



/*
 * Exit flags.
 * Used in #ROOMS.
 */
#define EX_ISDOOR		      1
#define EX_CLOSED		      2
#define EX_LOCKED		      4
#define EX_BASHPROOF		      8 /* KaVir */
#define EX_BASHED		     16 /* KaVir */
#define EX_PICKPROOF		     32
#define EX_PASSPROOF		     64 /* KaVir */
#define EX_WALL_WATER		    128 /* KaVir - Path of Water power 4 */

#define EX_WALL_FLAME		   4096 /* KaVir - Path of Flame power 4 */
#define EX_WALL_GRANITE		   8192 /* KaVir - Philodox power 5 */
#define EX_WALL_WIND		  16384 


/*
 * Sector types.
 * Used in #ROOMS.
 */
#define SECT_INSIDE		      0
#define SECT_CITY		      1
#define SECT_FIELD		      2
#define SECT_FOREST		      3
#define SECT_HILLS		      4
#define SECT_MOUNTAIN		      5
#define SECT_WATER_SWIM		      6
#define SECT_WATER_NOSWIM	      7
#define SECT_UNDERWATER		      8
#define SECT_AIR		      9
#define SECT_DESERT		     10
#define SECT_MAX		     11



/*
 * Equipment wear locations.
 * Used in #RESETS.
 */
#define WEAR_NONE		-1
#define WEAR_FINGER_L		0
#define WEAR_FINGER_R		1
#define WEAR_NECK_1		2
#define WEAR_NECK_2		3
#define WEAR_BODY		4
#define WEAR_HEAD		5
#define WEAR_LEGS		6
#define WEAR_FEET		7
#define WEAR_HANDS		8
#define WEAR_ARMS		9
#define WEAR_ABOUT		10
#define WEAR_WAIST		11
#define WEAR_WRIST_L		12
#define WEAR_WRIST_R		13
#define WEAR_WEAPON		14
#define WEAR_HOLD		15
#define WEAR_FACE		16
#define MAX_WEAR		17



/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (End of this section ... stop here)                   *
 *                                                                         *
 ***************************************************************************/

/*
 * Positions.
 */
#define POS_DEAD		      0
#define POS_MORTAL		      1
#define POS_INCAP		      2
#define POS_STUNNED		      3
#define POS_SLEEPING		      4
#define POS_MEDITATING		      5
#define POS_SITTING		      6
#define POS_RESTING		      7
#define POS_FIGHTING		      8
#define POS_STANDING		      9



/*
 * ACT bits for players.
 */
#define PLR_IS_NPC		      1		/* Don't EVER set.	*/

#define PLR_AUTOEXIT		      8
#define PLR_AUTOSEARCH		     16
#define PLR_AUTOFOLLOW		     32

#define PLR_BLANK		     64
#define PLR_BRIEF		    128
#define PLR_COMBINE		    512
#define PLR_PROMPT		   1024
#define PLR_TELNET_GA		   2048
#define PLR_HOLYLIGHT		   4096
#define PLR_WIZINVIS		   8192
#define PLR_ANSI		  16384
#define	PLR_SILENCE		  32768

#define PLR_NO_TELL		 262144
#define PLR_LOG			 524288

#define PLR_FREEZE		2097152
#define PLR_WOLFMAN		4194304

#define PLR_SDAM		16777216 /* Non-opponent damage */

#define PLR_HONOR		134217728
#define PLR_WIZINCOG		268435456 /* Incognito */

/*
 * EXTRA bits for players. (KaVir)
 */
#define EXTRA_SUMMON		4
#define EXTRA_NEWPASS		      8
#define EXTRA_OSWITCH		     16
#define EXTRA_SWITCH		     32
#define EXTRA_FAKE_CON		     64
#define TIED_UP			    128
#define GAGGED			    256
#define BLINDFOLDED		    512
#define EXTRA_TRANSPORT		1024
#define EXTRA_VAMPIRE		8192
#define EXTRA_PROMPT		  65536
#define EXTRA_MARRIED		 131072

#define EXTRA_CALL_ALL		1048576
#define EXTRA_CAN_SEE		2097152 /* You can't see, but familiar can */
#define EXTRA_AWE		8388608 /* Everyone is in awe of you */
#define EXTRA_ILLUSION	       33554432 /* Mob is really just an illusion */

#define EXTRA_INVIS_EMAIL     134217728 /* Cannot see email via finger */

/*
 * MORE bits for players. (KaVir)
 */
#define MORE_NPC		1	/* NPC - ie cannot improve */

#define MORE_MUMMIFIED		4	/* Serpentis 3 */
#define MORE_DARKNESS		8	/* Obtenebration 1 */
#define MORE_SILENCE		16	/* Quietus 1 */
#define MORE_PLASMA		32	/* Vicissitude 5 */

#define MORE_GUARDIAN		128	/* Necromancy 4 */
#define MORE_LOYAL		256	/* Dominate 6 */
#define MORE_SPIRIT		512	/* Is a Spirit, for Omnipotence */
#define MORE_LEADER		1024	/* Coterie leader */
#define MORE_HELIOS		2048	/* Kiss of Helios */

#define MORE_HEAD		16384	/* You are a fucking head you ass*/
#define MORE_SHIELDED		32768	/* Aura shielding */
#define MORE_CHAMPION		65536	/* Coterie champion */
#define MORE_DISINT		131072  /* Disintegrate */

#define MORE_CLOAK		1048576	/* Monk Mantra 8, Cloak of life */
#define MORE_THINBLOOD		2097152 /* Thinblood */		
#define MORE_ANIMAL_MASTER	4194304	/* Animals obey and dont attack you */

#define MORE_BRISTLES	       33554432 /* Metis power level 5 */

#define MORE_FAVOUR          1073741824 /* Monk Mantra 5, Almighty Favor */


/*
 * Xtra bits for players.
 */
#define XTRA_LIQUIFY			1
#define XTRA_ATONING			2
#define XTRA_SLOW			4
#define XTRA_TENEBROUS			8
#define XTRA_HEARTS_BLOOD	       16
#define XTRA_HOLY_SMOTE		       32
#define XTRA_BETTER_STR		       64
#define XTRA_BETTER_DEX		      128
#define XTRA_BETTER_CON	 	      256
#define XTRA_BETTER_INT		      512
#define XTRA_BETTER_WIS		     1024
#define XTRA_MAGE_CARESS	     2048
#define XTRA_MUTATE_EPHEMERA	     4096



#define DIAB_TIME		      0 /* Duration of diablerie aura */
#define DIAB_EVER		      1 /* Number of diableries committed */

/*
 * Stances for combat
 */
#define STANCE_NONE		0
#define STANCE_VIPER		1
#define STANCE_CRANE		2
#define STANCE_CRAB		3
#define STANCE_MONGOOSE		4
#define STANCE_BULL		5
#define STANCE_MANTIS		6
#define STANCE_DRAGON		7
#define STANCE_TIGER		8
#define STANCE_MONKEY		9
#define STANCE_SWALLOW		10


/*
 * Damage Types.
 */
#define TYPE_HIT		0
#define TYPE_UNARMED		1
#define TYPE_SLASH		2
#define TYPE_PIERCE		3
#define TYPE_BLUNT		4
#define TYPE_SPELL		5


/*
 * Combat Styles.
 */
#define STYLE_CURRENT		0
#define STYLE_UNARMED		1
#define STYLE_NATURAL		2
#define STYLE_ONEHAND		3
#define STYLE_SHIELD		4
#define STYLE_TWOHAND		5
#define STYLE_TWOWEAPON		6



/*
 * Channel bits.
 */
#define	CHANNEL_SORCTALK	1
#define	CHANNEL_CHAT		2
#define CHANNEL_HONOR		4
#define	CHANNEL_IMMTALK		8
#define	CHANNEL_MUSIC		16
#define CHANNEL_MAGETALK	32
#define	CHANNEL_SHOUT		64
#define	CHANNEL_YELL		128
#define	CHANNEL_VAMPTALK	256
#define	CHANNEL_HOWL		512
#define	CHANNEL_LOG		1024
#define CHANNEL_HIGHTALK	2048
#define	CHANNEL_INFO		4096
#define	CHANNEL_NEWBIE		8192
#define	CHANNEL_TELL		16384

#define CHANNEL_MIKTALK		65536
#define CHANNEL_ELEM		131072
#define CHANNEL_OOC		262144

#define CHANNEL_DROW		2097152
#define CHANNEL_TELEPATHY	4194304
#define CHANNEL_MONK		8388608

#define EXTRA_MARK		1
struct l_board
{
  char *pk_name;
  long pk_number;
  char *pd_name;
  long pd_number;
  char *tt_name;
  long tt_number;
  char *qc_name;
  long qc_number;
  char *mk_name;
  long mk_number;
  char *md_name;
  long md_number;
  char *bestpk_name;
  long bestpk_number;
  char *worstpk_name;
  long worstpk_number;
};

/*
 * Prototype for a mob.
 * This is the in-memory version of #MOBILES.
 */
struct	mob_index_data
{
    MOB_INDEX_DATA *	next;
    SPEC_FUN *		spec_fun;
    char *              spec_name;	/* OLC */
    SHOP_DATA *		pShop;
    AREA_DATA *         area;		/* OLC */
    char *		player_name;
    char *		short_descr;
    char *		long_descr;
    char *		description;
    long		vnum;
    long		count;
    long		sex;
    long		level;
    long		act;
unsigned long		affected_by;
};



/*
 * One character (PC or NPC).
 */
struct	char_data
{
    CHAR_DATA *		next;
    CHAR_DATA *		next_in_room;
    CHAR_DATA *		next_fight;
    CHAR_DATA *		prev_fight;
    CHAR_DATA *		master;
    CHAR_DATA *		leader;
    CHAR_DATA *		fighting;
    CHAR_DATA *		reply;
    CHAR_DATA *		wizard;
    CHAR_DATA *		embrace;
    SPEC_FUN *		spec_fun;
    MOB_INDEX_DATA *	pIndexData;
    DESCRIPTOR_DATA *	desc;
    AFFECT_DATA *	affected;
    OBJ_DATA *		carrying;
    ROOM_INDEX_DATA *	in_room;
    ROOM_INDEX_DATA *	was_in_room;
    PC_DATA *		pcdata;
    char *		name;
    char *		short_descr;
    char *		long_descr;
    char *		description;
    char *		lord;
    char *		clan;
    char *		morph;
    char *		createtime;
    char *		lasttime;
    char *		lasthost;
    char *		prompt;
    char *		cprompt;
    char *		contingency;
    long		stats[5];
    long		sex;
    long		status;
    long		class;
    long		immune;
    long		polyaff;
    long		vampaff;
    long		itemaffect;
    long		vampgen;
    long		stance[12];
    long		weapon[6];
    long		style[7];
    long		blood[2];
    long		beast;
    long		truebeast;
    long		embraced;
    long		home;
    long		init;
    long		level;
    long		played;
    time_t		logon;
    time_t		save_time;
    long		timer;
    long		wait;
    /* Transient combat state; never serialized into player saves. */
    long                combat_pause;
    long                stun_immunity;
    long		pkill;
    long		pdeath;
    long		mkill;
    long		mdeath;
    long		hit;
    long		max_hit;
    long		mana;
    long		max_mana;
    long		gold;
    long		exp;
    long		act;
    long		extra;
    long		more;
    long		xtra;
    long		affected_by;
    long		position;
    long		primal;
    long		carry_weight;
    long		carry_number;
    long		saving_throw;
    long		hitroll;
    long		damroll;
    long		armor;
    long 		deaf;
    long		save_timer;
    long		spheres[9];
    long		quint[3];
    long		paradox; //paradox that actually works....paradox like ;) mage only
    long		arete; //mage's power basiclly....
    long		beginners_luck;
    long                fight_timer;
    long		polymorph_timer;
    long		doublecast_timer;
    long		enrage_timer;
    long		liquify_timer;
    long		atoningwinds_timer;
    long		slow_timer;
    long		better_body_timer;
    long		paradox_ward;
    long		ephemera_timer;
};


/*
 * Data which only PC's have.
 */
struct	pc_data
{
    /* Transient named lesson consent; never serialized or kept as pointers. */
    char                teach_from[32];
    long                teach_disc;
    time_t              teach_until;
    PC_DATA *		next;
    BUFFER *		buffer;
    CHAR_DATA *		familiar;
    CHAR_DATA *		propose;
    OBJ_DATA *		chobj;
    BOARD_DATA *        board; /* The current board */
    time_t              last_note[MAX_BOARD]; /* last note for the boards */
    NOTE_DATA *         in_progress;
    char *		pwd;
    char *		bamfin;
    char *		bamfout;
    char *		cdest;
    char *		title;
    char *		marriage;
    char *		email;
    char *		conding;
    char *		love;
    char *		covenant;
    char *		cov_time;
    char *		leader;
    char *		vleader;
    char *		decap_char;
    long		denydate;
    long		quest;
    long		wolf;
    long		rank;
    long		regenerate;
    long		atm;
    long		homecv;
    long		necromancy;
    long		obeah;
    long		darkness;
    long		pain;
    long		silence;
    long		bodycrafts;
    long		weather;
    long		celerity;
    long		followers;
    long		dragcol;
    long		absorb[3];
    long		score[6];
    long		colors[6];
    long		cland[3];
    long		tgift[3];
    long		diableries[2];
    long		resist[6];
    long		breed[3];
    long		auspice[5];
    long		tribes[20];
    long		powers[20];
    long		dragskills;
    long		highland[4];
    long		ninja[5];
    long		monk[4];
    long		drow[2];
    long		classpoints[2];
    long		willpower[2];
    long		fake_skill;
    long		fake_stance;
    long		fake_hit;
    long		fake_dam;
    long		fake_hp;
    long		fake_mana;
    long		fake_max_hp;
    long		fake_max_mana;
    long		fake_ac;
    long		obj_vnum;
    long		majesty;
    long		flag;
};



/*
 * Liquids.
 */
#define LIQ_WATER        0
#define LIQ_MAX		16

struct	liq_type
{
    char *	liq_name;
    char *	liq_color;
    long	liq_affect[3];
};



/*
 * Extra description data for a room or object.
 */
struct	extra_descr_data
{
    EXTRA_DESCR_DATA *next;	/* Next in list                     */
    char *keyword;              /* Keyword in look/examine          */
    char *description;          /* What to see                      */
};



/*
 * Prototype for an object.
 */
struct	obj_index_data
{
    OBJ_INDEX_DATA *	next;
    EXTRA_DESCR_DATA *	extra_descr;
    AFFECT_DATA *	affected;
    AREA_DATA *         area;	/* OLC */
    char *		name;
    char *		short_descr;
    char *		description;
    char *		chpoweron;
    char *		chpoweroff;
    char *		chpoweruse;
    char *		victpoweron;
    char *		victpoweroff;
    char *		victpoweruse;
    char *		actcommands;
    long		vnum;
    long		item_type;
    long		extra_flags;
    long		wear_flags;
    long		count;
    long		weight;
    long			spectype;
    long			specpower;
    long		condition;
    long		toughness;
    long		resistance;
    long			quest;
    long		points;
    long			cost;			/* Unused */
    long			value	[4];
};



/*
 * One object.
 */
struct	obj_data
{
    OBJ_DATA *		next;
    OBJ_DATA *		next_content;
    OBJ_DATA *		contains;
    OBJ_DATA *		in_obj;
    CHAR_DATA *		carried_by;
    CHAR_DATA *		chobj;
    EXTRA_DESCR_DATA *	extra_descr;
    AFFECT_DATA *	affected;
    OBJ_INDEX_DATA *	pIndexData;
    ROOM_INDEX_DATA *	in_room;
    ROOM_INDEX_DATA *	to_room;
    char *		name;
    char *		short_descr;
    char *		description;
    char *		chpoweron;
    char *		chpoweroff;
    char *		chpoweruse;
    char *		victpoweron;
    char *		victpoweroff;
    char *		victpoweruse;
    char *		actcommands;
    char *		questmaker;
    char *		questowner;
    long		item_type;
    long		extra_flags;
    long		wear_flags;
    long		wear_loc;
    long		weight;
    long		spectype;
    long		specpower;
    long		toughness;
    long		quest;
    long		points;
    long		cost;
    long 		temp_timer;
    long		level;
    long		timer;
    long		value[4];
};



/*
 * Exit data.
 */
struct	exit_data
{
    ROOM_INDEX_DATA *	to_room;
    EXIT_DATA *         next;	/* OLC */
    long		vnum;
    long		exit_info;
    long		key;
    char *		keyword;
    char *		description;
    long			rs_flags;	/* OLC */
};



/*
 * Room text checking data.
 */
typedef struct roomtext_data
{
    long				type;
    long				power;
    long				mob;
    char *			input;
    char *			output;
    char *			choutput;
    char *			name;
    struct roomtext_data	*next;
} ROOMTEXT_DATA;



/*
 * Reset commands:
 *   '*': comment
 *   'M': read a mobile 
 *   'O': read an object
 *   'P': put object in object
 *   'G': give object to mobile
 *   'E': equip object to mobile
 *   'D': set state of door
 *   'R': randomize room exits
 *   'S': stop (end of list)
 */

/*
 * Area-reset definition.
 */
struct	reset_data
{
    RESET_DATA *	next;
    char		command;
    long		arg1;
    long		arg2;
    long		arg3;
};



/*
 * Area definition.
 */
struct	area_data
{
    AREA_DATA *		next;
    RESET_DATA *	reset_first;
    RESET_DATA *	reset_last;
    char *		name;
    long		age;
    long		nplayer;
    char *		filename;	/* OLC */
    char *		builders;	/* OLC */ /* Listing of */
    long		lvnum;		/* OLC */ /* Lower vnum */
    long		uvnum;		/* OLC */ /* Upper vnum */
    long		vnum;		/* OLC */ /* Area vnum  */
    long		area_flags;	/* OLC */
};



/*
 * Room type.
 */
struct	room_index_data
{
    ROOM_INDEX_DATA *	next;
    CHAR_DATA *		people;
    OBJ_DATA *		contents;
    OBJ_DATA *		to_obj;
    EXTRA_DESCR_DATA *	extra_descr;
    AREA_DATA *		area;
    EXIT_DATA *		exit[6];
    ROOMTEXT_DATA *	roomtext;
    TRAP_DATA *		trap;
    char *		name;
    char *		description;
    long		vnum;
    long 		room_flags;
    long		added_flags;
    long		light;
    long		blood;
    long		sector_type;
};


/*
 * Trap Crap.
 */
struct	trap_data
{
    char *		charmess;
    char *		roommess;
    char *		dammess;
    long		trapbits;
    long		diff;
    long		lodam;
    long		hidam;
    long		numhits;
    long		spellsn;
    long		movevnum;
    long		summonvnum;
};

#define TRAP_AREA		1	/* Affects whole room */
#define TRAP_ALARM		2	/* Alerts whole mud */
#define TRAP_SILENT		4	/* Gives no message on avoid */

#define TRAP_REPEAT		16	/* Goes off every tick */

#define IS_TRAP(trap, sn)		(IS_SET(trap->trapbits, sn))



/*
 *  Target types.
 */
#define TAR_IGNORE		    0
#define TAR_CHAR_OFFENSIVE	    1
#define TAR_CHAR_DEFENSIVE	    2
#define TAR_CHAR_SELF		    3
#define TAR_OBJ_INV		    4
#define TAR_DEATH                   5

#define PURPLE_MAGIC		0
#define RED_MAGIC		1
#define BLUE_MAGIC		2
#define GREEN_MAGIC		3
#define YELLOW_MAGIC		4
#define BLACK_MAGIC		5
#define RANDOM_MAGIC		6



/*
 * Skills include spells as a particular case.
 */
struct	skill_type
{
    char *	name;			/* Name of skill		*/
    long	skill_level;	        /* Level needed by class	*/
    SPELL_FUN *	spell_fun;		/* Spell pointer (for spells)	*/
    long	target;			/* Legal targets		*/
    long	minimum_position;	/* Position for caster / user	*/
    long *	pgsn;			/* Pointer to associated gsn	*/
    long	slot;			/* Slot for #OBJECT loading	*/
    long	min_mana;		/* Minimum mana used		*/
    long	beats;			/* Waiting time after use	*/
    char *	noun_damage;		/* Damage message		*/
    char *	msg_off;		/* Wear off message		*/
};



/*
 * These are skill_lookup return values for common skills and spells.
 */
extern	long	gsn_hide;
extern	long	gsn_sneak;

extern	long	gsn_polymorph;
extern	long	gsn_web;
extern	long	gsn_tendrils;
extern	long	gsn_rot;
extern	long	gsn_blindness;
extern	long	gsn_charm_person;
extern	long	gsn_curse;
extern	long	gsn_invis;
extern	long	gsn_mass_invis;
extern	long	gsn_poison;
extern  long    gsn_potency;
extern	long	gsn_sleep;
extern	long	gsn_fear;
extern  long    gsn_atoningwind;
extern  long    gsn_spiralmoon;
extern  long    gsn_slow;
extern  long    gsn_liquifyearth;
extern  long    gsn_friction_curse;
extern  long    gsn_mage_invis;
extern  long	gsn_alter_weight;
extern  long    gsn_brittle_bones;

/*
 * Utility macros.
 */
#define UMIN(a, b)		((a) < (b) ? (a) : (b))
#define UMAX(a, b)		((a) > (b) ? (a) : (b))
#define URANGE(a, b, c)		((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
#define LOWER(c)		((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))
#define UPPER(c)		((c) >= 'a' && (c) <= 'z' ? (c)+'A'-'a' : (c))
#define IS_SET(flag, bit)	((flag) & (bit))
#define SET_BIT(var, bit)	((var) |= (bit))
#define REMOVE_BIT(var, bit)	((var) &= ~(bit))
#define TOGGLE_BIT(var, bit)    ((var) ^= (bit))	/* OLC */
#define STR(dat, field)         (( (dat)->field != NULL                    \
                                             ? (dat)->field                \
                                             : (dat)->pIndexData->field ))	/* OLC */



/*
 * Character macros.
 */
#define IS_NPC(ch)		(IS_SET((ch)->act, ACT_IS_NPC))
#define IS_ST(ch)		(get_trust(ch) >= LEV_ST && !IS_NPC(ch))
#define IS_IMP(ch)		(get_trust(ch) >= LEV_IMP && !IS_NPC(ch))
#define IS_IMM(ch)		(get_trust(ch) >= LEV_IMM && !IS_NPC(ch))
#define IS_BUI(ch)		(get_trust(ch) >= LEV_BUI && !IS_NPC(ch))
#define IS_AWA(ch)		(get_trust(ch) >= LEV_AWA && !IS_NPC(ch))
#define CAN_PK(ch)		(IS_NPC(ch) || IS_FLAG((ch),FLAG_PK))

#define IS_AFFECTED(ch, sn)	(IS_SET((ch)->affected_by, (sn)))
#define IS_ITEMAFF(ch, sn)	(IS_SET((ch)->itemaffect, (sn)))
#define IS_IMMUNE(ch, sn)	(IS_SET((ch)->immune, (sn)))
#define IS_WILLPOWER(ch, sn)	(IS_SET((ch)->pcdata->resist[0], (sn)))
#define IS_VAMPAFF(ch, sn)	(IS_SET((ch)->vampaff, (sn)))
#define IS_POLYAFF(ch, sn)	(IS_SET((ch)->polyaff, (sn)))
#define IS_DRAGAFF(ch, sn)	(IS_SET((ch)->pcdata->dragskills, (sn)))
#define IS_EXTRA(ch, sn)	(IS_SET((ch)->extra, (sn)))
#define IS_ACT(ch, sn)		(IS_SET((ch)->act, (sn)))
#define IS_MORE(ch, sn)		(IS_SET((ch)->more, (sn)))
#define IS_CRAFT(ch, sn)	(IS_SET((ch)->pcdata->bodycrafts, (sn)))
#define IS_STANCE(ch, sn)	(ch->stance[0] == sn)
#define IS_STYLE(ch, sn)	(ch->style[0] == sn)
#define IS_SPIRIT(ch)		(IS_SET((ch)->more, MORE_SPIRIT))
#define IS_FLAG(ch,sn)		(IS_SET((ch)->pcdata->flag, sn))
#define IS_XTRA(ch,sn)		(IS_SET((ch)->xtra, (sn)))


#define IS_VAMPIRE(ch)		((ch)->class == CLASS_VAMPIRE && !IS_NPC(ch))
#define IS_WEREWOLF(ch)		((ch)->class == CLASS_WEREWOLF && !IS_NPC(ch))
#define IS_SORCERER(ch)		((ch)->class == CLASS_SORCERER && !IS_NPC(ch))
#define IS_HIGHLANDER(ch)	((ch)->class == CLASS_HIGHLANDER && !IS_NPC(ch))
#define IS_DROW(ch)		((ch)->class == CLASS_DROW && !IS_NPC(ch))
#define IS_NINJA(ch)            ((ch)->class == CLASS_NINJA && !IS_NPC(ch))
#define IS_DRAGON(ch)		((ch)->class == CLASS_DRAGON && !IS_NPC(ch))
#define IS_MAGE(ch)		((ch)->class == CLASS_MAGE && !IS_NPC(ch))
#define IS_MONK(ch)             ((ch)->class == CLASS_MONK && !IS_NPC(ch))
#define IS_ELEM(ch)             ((ch)->class == CLASS_ELEM && !IS_NPC(ch))

#define IN_CLAN( ch )           (IS_NPC(ch)?0:ch->clan)
#define IS_PLAYING( d )         (d->connected==CON_PLAYING)
 

#define IS_AWAKE(ch)		(ch->position > POS_SLEEPING)
#define PC(ch,field)            ((ch)->pcdata->field)
#define GET_PC(ch,field,nopc)   ((ch)->pcdata != NULL ? (ch)->pcdata->field : (nopc))
#define GET_AC(ch)		((ch)->armor)
#define GET_HITROLL(ch)		((ch)->hitroll)
#define GET_DAMROLL(ch)		((ch)->damroll)

#define IS_OUTSIDE(ch)		(!IS_SET((ch)->in_room->room_flags, ROOM_INDOORS))

#define WAIT_STATE(ch, npulse)	((ch)->wait = (UMAX((ch)->wait, npulse)))
#define IS_COMBAT_PAUSED(ch)     ((ch)->combat_pause > 0)



/*
 * Object Macros.
 */
#define CAN_WEAR(obj, part)	(IS_SET((obj)->wear_flags,  (part)))
#define IS_OBJ_STAT(obj, stat)	(IS_SET((obj)->extra_flags, (stat)))



/*
 * Description macros.
 */
#define PERS(ch, looker)	( can_see( looker, (ch) ) ?		\
				( IS_NPC(ch) ? (ch)->short_descr	\
				: ( IS_AFFECTED( (ch), AFF_POLYMORPH) ?   \
				(ch)->morph : (ch)->name ) )		\
				: "someone" )



/*
 * Structure for a command in the command lookup table.
 */
struct	cmd_type
{
    char * const	name;
    DO_FUN *		do_fun;
    long		position;
    long		level;
    long		log;
};



/*
 * Structure for a social in the socials table.
 */
struct	social_type
{
    char * const	name;
    char * const	char_no_arg;
    char * const	others_no_arg;
    char * const	char_found;
    char * const	others_found;
    char * const	vict_found;
    char * const	char_auto;
    char * const	others_auto;
};



/*
 * Global constants.
 */
extern	const	struct	cmd_type	cmd_table	[];
extern	const	struct	liq_type	liq_table	[LIQ_MAX];
extern	const	struct	skill_type	skill_table	[MAX_SKILL];
extern	const	struct	social_type	social_table	[];
extern  const   char *  sphere_name     [9];
extern	char *	const	dir_name        [];	/* OLC */
extern	const	long	rev_dir         [];	/* OLC */



/*
 * Global variables.
 */
extern		HELP_DATA	  *	help_first;
extern		SHOP_DATA	  *	shop_first;

extern		BAN_DATA	  *	ban_list;
extern		CHAR_DATA	  *	char_list;
extern		DESCRIPTOR_DATA   *	descriptor_list;
extern		NOTE_DATA	  *	note_list;
extern		OBJ_DATA	  *	object_list;

extern		AFFECT_DATA	  *	affect_free;
extern		BAN_DATA	  *	ban_free;
extern		CHAR_DATA	  *	char_free;
extern		DESCRIPTOR_DATA	  *	descriptor_free;
extern		EXTRA_DESCR_DATA  *	extra_descr_free;
extern		ROOMTEXT_DATA     *	roomtext_free;
extern		NOTE_DATA	  *	note_free;
extern		OBJ_DATA	  *	obj_free;
extern		PC_DATA		  *	pcdata_free;

extern		char			bug_buf		[];
extern		time_t			current_time;
extern		bool			fLogAll;
extern		FILE *			fpReserve;
extern		char			log_buf		[];
extern		TIME_INFO_DATA		time_info;
extern		WEATHER_DATA		weather_info;
extern		EXP_DATA		exp_info;



/*
 * Extra global's by KaVir.
 */
extern		CHAR_DATA	  *	first_fight;
extern		CHAR_DATA	  *	last_fight;
extern		char			first_place	[];
extern		char			second_place	[];
extern		char			third_place	[];
extern		long			score_1;
extern		long			score_2;
extern		long			score_3;
extern		long			world_affects;



/*
 * online creation externals  OLC
 */
extern          AREA_DATA *             area_first;
extern          AREA_DATA *             area_last;
extern  	SHOP_DATA *             shop_last;

extern          long                     top_affect;
extern          long                     top_area;
extern          long                     top_ed;
extern          long                     top_exit;
extern          long                     top_help;
extern          long                     top_mob_index;
extern          long                     top_obj_index;
extern          long                     top_reset;
extern          long                     top_room;
extern          long                     top_shop;

extern          long                     top_vnum_mob;
extern          long                     top_vnum_obj;
extern          long                     top_vnum_room;

extern          char                    str_empty       [1];

extern  MOB_INDEX_DATA *        mob_index_hash  [MAX_KEY_HASH];
extern  OBJ_INDEX_DATA *        obj_index_hash  [MAX_KEY_HASH];
extern  ROOM_INDEX_DATA *       room_index_hash [MAX_KEY_HASH];



/*
 * Command functions.
 * Defined in act_*.c (mostly).
 */
DECLARE_DO_FUN(	do_increase	);
DECLARE_DO_FUN(	do_beastform	);
DECLARE_DO_FUN( do_jamtech      );
DECLARE_DO_FUN( do_showdamage	);
DECLARE_DO_FUN( do_summonnet    );
DECLARE_DO_FUN( do_stail	);
DECLARE_DO_FUN( do_dust		);
DECLARE_DO_FUN( do_dbreath	);
DECLARE_DO_FUN( do_dragonskill  );
DECLARE_DO_FUN( do_dragonform	);
DECLARE_DO_FUN( do_bladeflash	);
DECLARE_DO_FUN( do_bash		);
DECLARE_DO_FUN( do_blitz        );
DECLARE_DO_FUN( do_lunge        );
DECLARE_DO_FUN( do_horrid	);
DECLARE_DO_FUN( do_htwist	);
DECLARE_DO_FUN( do_judgement	);
DECLARE_DO_FUN( do_forge	);
DECLARE_DO_FUN( do_earthshift	);
DECLARE_DO_FUN( do_empathy	);
DECLARE_DO_FUN( do_enrage	);
DECLARE_DO_FUN( do_eyes		);
DECLARE_DO_FUN( do_disint	);
DECLARE_DO_FUN( do_thinblood	);
DECLARE_DO_FUN( do_shadowforge	);
DECLARE_DO_FUN(	do_accept	);
DECLARE_DO_FUN(	do_activate	);
DECLARE_DO_FUN( do_advance	);
DECLARE_DO_FUN(	do_aeolus	);
DECLARE_DO_FUN(	do_affects	);
DECLARE_DO_FUN( do_afk          );
DECLARE_DO_FUN(	do_allow	);
DECLARE_DO_FUN(	do_almighty	);
DECLARE_DO_FUN( do_alterweight  );
DECLARE_DO_FUN( do_analyze      );  
DECLARE_DO_FUN(	do_anesthetic	);
DECLARE_DO_FUN(	do_animal	);
DECLARE_DO_FUN(	do_ansi		);
DECLARE_DO_FUN( do_anull	);
DECLARE_DO_FUN( do_aquabarrier  );
DECLARE_DO_FUN(	do_areas	);
DECLARE_DO_FUN( do_arete	);
DECLARE_DO_FUN(	do_arms		);
DECLARE_DO_FUN(	do_artifact	);
DECLARE_DO_FUN( do_ashow        );
DECLARE_DO_FUN(	do_at		);
DECLARE_DO_FUN( do_atoningwinds );
DECLARE_DO_FUN( do_aurashot     );
DECLARE_DO_FUN(	do_autoexit	);
DECLARE_DO_FUN( do_autofollow   );
DECLARE_DO_FUN(	do_autosearch	);
DECLARE_DO_FUN(	do_autosave	);
DECLARE_DO_FUN( do_autostance	);
DECLARE_DO_FUN(	do_awe		);
DECLARE_DO_FUN(	do_baal		);
DECLARE_DO_FUN(	do_balor	);
DECLARE_DO_FUN(	do_banshee	);
DECLARE_DO_FUN(	do_beastlike	);
DECLARE_DO_FUN(	do_beastwithin	);
DECLARE_DO_FUN(	do_beckon	);
DECLARE_DO_FUN( do_beginners_luck );
DECLARE_DO_FUN(	do_berserk	);
DECLARE_DO_FUN( do_better_body	); 
DECLARE_DO_FUN(	do_blank	);
DECLARE_DO_FUN(	do_blindfold	);
DECLARE_DO_FUN( do_blissful	);
DECLARE_DO_FUN(	do_bloodagony	);
DECLARE_DO_FUN(	do_bloodyfeast	);
DECLARE_DO_FUN(	do_blur		);
DECLARE_DO_FUN(	do_bodywrack	);
DECLARE_DO_FUN(	do_bonecraft	);
DECLARE_DO_FUN(	do_brandish	);
DECLARE_DO_FUN( do_breach	);
DECLARE_DO_FUN( do_break_dreamshell );
DECLARE_DO_FUN(	do_breakup	);
DECLARE_DO_FUN(	do_sbreath	);
DECLARE_DO_FUN(	do_breathwyld	);
DECLARE_DO_FUN(	do_brief	);
DECLARE_DO_FUN(	do_burrow	);
DECLARE_DO_FUN(	do_buy		);
DECLARE_DO_FUN(	do_call		);
DECLARE_DO_FUN( do_call_lightning );
DECLARE_DO_FUN(	do_calm		);
DECLARE_DO_FUN(	do_camouflage	);
DECLARE_DO_FUN(	do_cast		);
DECLARE_DO_FUN(	do_cdest	);
DECLARE_DO_FUN(	do_celerity	);
DECLARE_DO_FUN( do_celestial    );
DECLARE_DO_FUN(	do_change	);
DECLARE_DO_FUN(	do_changelight	);
DECLARE_DO_FUN(	do_channels	);
DECLARE_DO_FUN(	do_chat		);
DECLARE_DO_FUN(	do_chimera	);
DECLARE_DO_FUN(	do_claim	);
DECLARE_DO_FUN(	do_clandisc	);
DECLARE_DO_FUN( do_clansphere	);
DECLARE_DO_FUN(	do_clanname	);
DECLARE_DO_FUN(	do_clap		);
DECLARE_DO_FUN( do_autoclass	);
DECLARE_DO_FUN(	do_claws	);
DECLARE_DO_FUN(	do_clearclass	);
DECLARE_DO_FUN(	do_clearclas	);
DECLARE_DO_FUN(	do_clearstats	);
DECLARE_DO_FUN( do_cloak        );
DECLARE_DO_FUN(	do_cloneobject	);
DECLARE_DO_FUN(	do_close	);
DECLARE_DO_FUN(	do_command	);
DECLARE_DO_FUN(	do_commands	);
DECLARE_DO_FUN(	do_compare	);
DECLARE_DO_FUN(	do_complete	);
DECLARE_DO_FUN(	do_conditioning	);
DECLARE_DO_FUN(	do_config	);
DECLARE_DO_FUN(	do_consider	);
DECLARE_DO_FUN( do_contingency  );
DECLARE_DO_FUN(	do_control	);
DECLARE_DO_FUN(	do_controlbeast	);
DECLARE_DO_FUN( do_controlweather );
DECLARE_DO_FUN( do_copyover	);
DECLARE_DO_FUN(	do_covenant	);
DECLARE_DO_FUN(	do_cprompt	);
DECLARE_DO_FUN(	do_create	);
DECLARE_DO_FUN(	do_credits	);
DECLARE_DO_FUN(	do_cripple	);
DECLARE_DO_FUN(	do_cvalk	);
DECLARE_DO_FUN(	do_cutting	);
DECLARE_DO_FUN( do_darkblaze    );
DECLARE_DO_FUN(	do_dazzle	);
DECLARE_DO_FUN(	do_decapitate	);
DECLARE_DO_FUN( do_decide       );
DECLARE_DO_FUN( do_delete	);
DECLARE_DO_FUN( do_demote	);
DECLARE_DO_FUN(	do_deny		);
DECLARE_DO_FUN(	do_description	);
DECLARE_DO_FUN(	do_destroy	);
DECLARE_DO_FUN( do_destroy_sculpture );
DECLARE_DO_FUN( do_diagnostics	);
DECLARE_DO_FUN( do_dinduct      );
DECLARE_DO_FUN( do_disable	);
DECLARE_DO_FUN(	do_disarm	);
DECLARE_DO_FUN(	do_discipline	);
DECLARE_DO_FUN( do_humble       );
DECLARE_DO_FUN(	do_disconnect	);
DECLARE_DO_FUN(	do_disquiet	);
DECLARE_DO_FUN(	do_divorce	);
DECLARE_DO_FUN(	do_dominator	);
DECLARE_DO_FUN( do_doppleganger	);
DECLARE_DO_FUN(	do_down		);
DECLARE_DO_FUN(	do_dreadgaze	);
DECLARE_DO_FUN(	do_drop		);
DECLARE_DO_FUN( do_earthembrace	);
DECLARE_DO_FUN(	do_earthmeld	);
DECLARE_DO_FUN(	do_east		);
DECLARE_DO_FUN(	do_eat		);
DECLARE_DO_FUN(	do_echo		);
DECLARE_DO_FUN( do_elemarmor    );
DECLARE_DO_FUN( do_element      );
DECLARE_DO_FUN( do_elemental	);
DECLARE_DO_FUN(	do_email	);
DECLARE_DO_FUN(	do_emote	);
DECLARE_DO_FUN(	do_engulf	);
DECLARE_DO_FUN(	do_enter	);
DECLARE_DO_FUN(	do_embrace	);
DECLARE_DO_FUN(	do_entrancement	);
DECLARE_DO_FUN(	do_equipment	);
DECLARE_DO_FUN(	do_escape	);
DECLARE_DO_FUN(	do_examine	);
DECLARE_DO_FUN(	do_exits	);
DECLARE_DO_FUN( do_expel        );
DECLARE_DO_FUN(	do_expmod	);
DECLARE_DO_FUN(	do_faerielight	);
DECLARE_DO_FUN( do_failure      );
DECLARE_DO_FUN(	do_falling	);
DECLARE_DO_FUN(	do_familiar	);
DECLARE_DO_FUN(	do_fangs	);
DECLARE_DO_FUN( do_farsight     );
DECLARE_DO_FUN(	do_fatality	);
DECLARE_DO_FUN(	do_favour	);
DECLARE_DO_FUN(	do_fcommand	);
DECLARE_DO_FUN(	do_feed		);
DECLARE_DO_FUN(	do_fetishdoll	);
DECLARE_DO_FUN( do_fistofcaine  );
DECLARE_DO_FUN(	do_finger	);
DECLARE_DO_FUN( do_fireblade	);
DECLARE_DO_FUN(	do_flamebolt	);
DECLARE_DO_FUN( do_flarearrow   );
DECLARE_DO_FUN(	do_flee		);
DECLARE_DO_FUN(	do_fleshcraft	);
DECLARE_DO_FUN(	do_flex		);
DECLARE_DO_FUN( do_fleshbond	);
DECLARE_DO_FUN(	do_follow	);
DECLARE_DO_FUN(	do_force	);
DECLARE_DO_FUN(	do_forceauto	);
DECLARE_DO_FUN(	do_formillusion	);
DECLARE_DO_FUN(	do_freeze	);
DECLARE_DO_FUN(	do_frenzy	);
DECLARE_DO_FUN( do_friction_curse);
DECLARE_DO_FUN(	do_friend	);
DECLARE_DO_FUN(	do_gag		);
DECLARE_DO_FUN( do_geas		);
DECLARE_DO_FUN(	do_generation	);
DECLARE_DO_FUN(	do_get		);
DECLARE_DO_FUN(	do_gift		);
DECLARE_DO_FUN(	do_gifts	);
DECLARE_DO_FUN(	do_give		);
DECLARE_DO_FUN( do_godstrike    );
DECLARE_DO_FUN(	do_grip		);
DECLARE_DO_FUN(	do_group	);
DECLARE_DO_FUN(	do_gtell	);
DECLARE_DO_FUN( do_heal		);
DECLARE_DO_FUN(	do_helios	);
DECLARE_DO_FUN(	do_help		);
DECLARE_DO_FUN(	do_hightalk	);
DECLARE_DO_FUN(	do_halosun	);
DECLARE_DO_FUN( do_holyaura     );
DECLARE_DO_FUN( do_holy_smote	);
DECLARE_DO_FUN( do_chome         );
DECLARE_DO_FUN(	do_home		);
DECLARE_DO_FUN(	do_honor	);
DECLARE_DO_FUN(	do_horns	);
DECLARE_DO_FUN( do_hotienchi	);
DECLARE_DO_FUN( do_hover	);
DECLARE_DO_FUN(	do_howl		);
DECLARE_DO_FUN(	do_huh		);
DECLARE_DO_FUN(	do_humanity	);
DECLARE_DO_FUN(	do_hurl		);
DECLARE_DO_FUN( do_iciclestorm  );
DECLARE_DO_FUN(	do_immtalk	);
DECLARE_DO_FUN( do_induct	);
DECLARE_DO_FUN(	do_infest	);
DECLARE_DO_FUN( do_infinitesight);
DECLARE_DO_FUN(	do_infirmity	);
DECLARE_DO_FUN(	do_info		);
DECLARE_DO_FUN(	do_inventory	);
DECLARE_DO_FUN(	do_invis	);
DECLARE_DO_FUN(	do_invite	);
DECLARE_DO_FUN(	do_invoke	);
DECLARE_DO_FUN(	do_incog	);
DECLARE_DO_FUN(	do_invisibility	);
DECLARE_DO_FUN(	do_kick		);
DECLARE_DO_FUN(	do_kill		);
DECLARE_DO_FUN(	do_lamprey	);
DECLARE_DO_FUN( do_layhands	);
DECLARE_DO_FUN( do_leader       );
DECLARE_DO_FUN( do_leaderclear  );
DECLARE_DO_FUN( do_leechblood	);
DECLARE_DO_FUN( do_level	);
DECLARE_DO_FUN(	do_lifecloak	);
DECLARE_DO_FUN( do_liquifyearth );
DECLARE_DO_FUN(	do_list		);
DECLARE_DO_FUN( do_lobotomy	);
DECLARE_DO_FUN( do_localquake   );
DECLARE_DO_FUN(	do_locate	);
DECLARE_DO_FUN(	do_lock		);
DECLARE_DO_FUN(	do_log		);
DECLARE_DO_FUN(	do_look		);
DECLARE_DO_FUN( do_mgate	);
DECLARE_DO_FUN( do_magetalk	);
DECLARE_DO_FUN(	do_malleable	);
DECLARE_DO_FUN(	do_majesty	);
DECLARE_DO_FUN( do_mantra       );
DECLARE_DO_FUN( do_mark		);
DECLARE_DO_FUN(	do_marry	);
DECLARE_DO_FUN(	do_mask		);
DECLARE_DO_FUN( do_mclan        );
DECLARE_DO_FUN(	do_mclear	);
DECLARE_DO_FUN(	do_meditate	);
DECLARE_DO_FUN(	do_memory	);
DECLARE_DO_FUN(	do_mfind	);
DECLARE_DO_FUN(	do_mindblast	);
DECLARE_DO_FUN( do_mindflex     );
DECLARE_DO_FUN( do_mindshield   );
DECLARE_DO_FUN( do_mirageslice  );
DECLARE_DO_FUN(	do_mirrorimage	);
DECLARE_DO_FUN( do_monkarmor    );
DECLARE_DO_FUN( do_mutate_ephemera );
DECLARE_DO_FUN(	do_mload	);
DECLARE_DO_FUN( do_monklearn	);
DECLARE_DO_FUN(	do_mother	);
DECLARE_DO_FUN( do_move		);
DECLARE_DO_FUN(	do_mset		);
DECLARE_DO_FUN( do_mshow	);
DECLARE_DO_FUN( do_untether	);
DECLARE_DO_FUN(	do_mstat	);
DECLARE_DO_FUN(	do_mwhere	);
DECLARE_DO_FUN(	do_mummify	);
DECLARE_DO_FUN(	do_music	);
DECLARE_DO_FUN(	do_newbie	);
DECLARE_DO_FUN( do_newbielock	);
DECLARE_DO_FUN( do_nightblade   );
DECLARE_DO_FUN(	do_nightsight	);
DECLARE_DO_FUN(	do_north	);
DECLARE_DO_FUN(	do_note		);
DECLARE_DO_FUN(	do_notell	);
DECLARE_DO_FUN(	do_obedience	);
DECLARE_DO_FUN(	do_oclone	);
DECLARE_DO_FUN(	do_ofind	);
DECLARE_DO_FUN(	do_oload	);
DECLARE_DO_FUN(	do_omnipotence	);
DECLARE_DO_FUN(	do_open		);
DECLARE_DO_FUN(	do_order	);
DECLARE_DO_FUN(	do_oreturn	);
DECLARE_DO_FUN(	do_oset		);
DECLARE_DO_FUN( do_oshow	);
DECLARE_DO_FUN(	do_ostat	);
DECLARE_DO_FUN(	do_oswitch	);
DECLARE_DO_FUN(	do_otransfer	);
DECLARE_DO_FUN( do_outcast	);
DECLARE_DO_FUN(	do_paralyzing	);
DECLARE_DO_FUN( do_paradox_ward );
DECLARE_DO_FUN( do_perceivespace );
DECLARE_DO_FUN( do_patterntear	);
DECLARE_DO_FUN(	do_password	);
DECLARE_DO_FUN(	do_peace	);
DECLARE_DO_FUN(	do_peek		);
DECLARE_DO_FUN( do_perception	);
DECLARE_DO_FUN(	do_persuasion	);
DECLARE_DO_FUN( do_phoenixform  );
DECLARE_DO_FUN(	do_pick		);
DECLARE_DO_FUN(	do_pigeon	);
DECLARE_DO_FUN(	do_pkme		);
DECLARE_DO_FUN(	do_plasmaform	);
DECLARE_DO_FUN(	do_pload	);
DECLARE_DO_FUN( do_polyappear	);
DECLARE_DO_FUN(	do_porcupine	);
DECLARE_DO_FUN(	do_possession	);
DECLARE_DO_FUN(	do_powers	);
DECLARE_DO_FUN(	do_pour		);
DECLARE_DO_FUN(	do_prepfor	);
DECLARE_DO_FUN(	do_preservation	);
DECLARE_DO_FUN( do_primalanger  );
DECLARE_DO_FUN( do_promote	);
DECLARE_DO_FUN( do_promotion    );
DECLARE_DO_FUN(	do_prompt	);
DECLARE_DO_FUN(	do_propose	);
DECLARE_DO_FUN(	do_pset		);
DECLARE_DO_FUN( do_nuke         );
DECLARE_DO_FUN( do_pcrename     );
DECLARE_DO_FUN(	do_punch	);
DECLARE_DO_FUN(	do_purge	);
DECLARE_DO_FUN(	do_put		);
DECLARE_DO_FUN(	do_qmake	);
DECLARE_DO_FUN(	do_quaff	);
DECLARE_DO_FUN( do_quintessence	);
DECLARE_DO_FUN(	do_qset		);
DECLARE_DO_FUN(	do_qstat	);
DECLARE_DO_FUN(	do_quest	);
DECLARE_DO_FUN(	do_qui		);
DECLARE_DO_FUN(	do_quit		);
DECLARE_DO_FUN( do_randomload   );
DECLARE_DO_FUN(	do_rage		);
DECLARE_DO_FUN(	do_read		);
DECLARE_DO_FUN(	do_readaura	);
DECLARE_DO_FUN(	do_recall	);
DECLARE_DO_FUN( do_crecall	);
DECLARE_DO_FUN(	do_recharge	);
DECLARE_DO_FUN(	do_recho	);
DECLARE_DO_FUN(	do_recite	);
DECLARE_DO_FUN(	do_redirect	);
DECLARE_DO_FUN(	do_regenerate	);
DECLARE_DO_FUN(	do_release	);
DECLARE_DO_FUN(	do_relevel	);
DECLARE_DO_FUN(	do_remove	);
DECLARE_DO_FUN(	do_rename	);
DECLARE_DO_FUN(	do_repeat	);
DECLARE_DO_FUN(	do_reply	);
DECLARE_DO_FUN(	do_report	);
DECLARE_DO_FUN(	do_rescue	);
DECLARE_DO_FUN(	do_reshape	);
DECLARE_DO_FUN(	do_rest		);
DECLARE_DO_FUN(	do_restore	);
DECLARE_DO_FUN(	do_return	);
DECLARE_DO_FUN( do_roleplay     );
DECLARE_DO_FUN(	do_rset		);
DECLARE_DO_FUN( do_rshow	);
DECLARE_DO_FUN(	do_sacrifice	);
DECLARE_DO_FUN(	do_safe		);
DECLARE_DO_FUN(	do_save		);
DECLARE_DO_FUN(	do_say		);
DECLARE_DO_FUN(	do_scan		);
DECLARE_DO_FUN(	do_score	);
DECLARE_DO_FUN(	do_scry		);
DECLARE_DO_FUN( do_select_spheres );
DECLARE_DO_FUN(	do_sell		);
DECLARE_DO_FUN(	do_serenity	);
DECLARE_DO_FUN(	do_serpent	);
DECLARE_DO_FUN( do_setclandisc  );
DECLARE_DO_FUN( do_setclansphere );
DECLARE_DO_FUN( do_setdisc      );
DECLARE_DO_FUN( do_setsphere    );
DECLARE_DO_FUN( do_setgift      );
DECLARE_DO_FUN(	do_setflag	);
DECLARE_DO_FUN(	do_shadowbody	);
DECLARE_DO_FUN(	do_shadowplane	);
DECLARE_DO_FUN(	do_shadowsight	);
DECLARE_DO_FUN(	do_shadowstep	);
DECLARE_DO_FUN( do_shapechange	);
DECLARE_DO_FUN(	do_shield	);
DECLARE_DO_FUN( do_shoot	);
DECLARE_DO_FUN(	do_shout	);
DECLARE_DO_FUN(	do_shroudnight	);
DECLARE_DO_FUN(	do_shutdown	);
DECLARE_DO_FUN( do_slow         );
DECLARE_DO_FUN(	do_pulse	);
DECLARE_DO_FUN(	do_silence	);
DECLARE_DO_FUN(	do_cdest	);
DECLARE_DO_FUN( do_sculpture	);
DECLARE_DO_FUN(	do_silencedeath	);
DECLARE_DO_FUN(	do_sing		);
DECLARE_DO_FUN(	do_sit		);
DECLARE_DO_FUN(	do_skill	);
DECLARE_DO_FUN(	do_slay		);
DECLARE_DO_FUN(	do_sleep	);
DECLARE_DO_FUN(	do_slookup	);
DECLARE_DO_FUN( do_solarflash   );
DECLARE_DO_FUN( do_solarlight   );
DECLARE_DO_FUN( do_songrage	);
DECLARE_DO_FUN(	do_soulmask	);
DECLARE_DO_FUN(	do_spells	);
DECLARE_DO_FUN( do_magepowers	);
DECLARE_DO_FUN( do_sorctalk	);
DECLARE_DO_FUN( do_spheres	);
DECLARE_DO_FUN( do_spiralmoon   );
DECLARE_DO_FUN( do_spirit_caress );
DECLARE_DO_FUN( do_spirit_sense );
DECLARE_DO_FUN(	do_spiriteyes	);
DECLARE_DO_FUN(	do_spiritform	);
DECLARE_DO_FUN(	do_spiritgate	);
DECLARE_DO_FUN(	do_spiritguardian);
DECLARE_DO_FUN( do_spiritspeak  );
DECLARE_DO_FUN( do_spiritward	);
DECLARE_DO_FUN(	do_spit		);
DECLARE_DO_FUN(	do_stance	);
DECLARE_DO_FUN(	do_status	);
DECLARE_DO_FUN( do_step_sideways );
DECLARE_DO_FUN(	do_smother	);
DECLARE_DO_FUN(	do_sneak	);
DECLARE_DO_FUN(	do_snoop	);
DECLARE_DO_FUN(	do_socials	);
DECLARE_DO_FUN( do_mage_heal);
DECLARE_DO_FUN( do_mage_invis	);
DECLARE_DO_FUN(	do_south	);
DECLARE_DO_FUN(	do_spydirection	);
DECLARE_DO_FUN(	do_stand	);
DECLARE_DO_FUN( do_statcheck	);
DECLARE_DO_FUN( do_stoneblast   );
DECLARE_DO_FUN( do_stunfist     );
DECLARE_DO_FUN(	do_succulence	);
DECLARE_DO_FUN(	do_summon	);
DECLARE_DO_FUN(	do_summoning	);
DECLARE_DO_FUN( do_sunbath      );
DECLARE_DO_FUN(	do_surge	);
DECLARE_DO_FUN(	do_switch	);
DECLARE_DO_FUN(	do_teachdisc	);
DECLARE_DO_FUN(do_tasteblood);
DECLARE_DO_FUN(do_bloodrage);
DECLARE_DO_FUN(do_potency);
DECLARE_DO_FUN(do_theftvitae);
DECLARE_DO_FUN(do_cauldronblood);
long get_blood_potency args((CHAR_DATA *ch));
void vampire_clear_offer args((CHAR_DATA *ch));
DECLARE_DO_FUN( do_telekinetics );
DECLARE_DO_FUN(	do_tell		);
DECLARE_DO_FUN( do_telepathy	);
DECLARE_DO_FUN( do_tempest	);
DECLARE_DO_FUN( do_tenebrous	);
DECLARE_DO_FUN(	do_thanatopsis	);
DECLARE_DO_FUN(	do_tie		);
DECLARE_DO_FUN(	do_time		);
DECLARE_DO_FUN(	do_watertravel	);
DECLARE_DO_FUN(	do_title	);
DECLARE_DO_FUN(	do_token	);
DECLARE_DO_FUN(	do_tongue	);
DECLARE_DO_FUN(	do_torment	);
DECLARE_DO_FUN(	do_totalscore	);
DECLARE_DO_FUN(	do_touch	);
DECLARE_DO_FUN(	do_tradition	);
DECLARE_DO_FUN(	do_train	);
DECLARE_DO_FUN(	do_transfer	);
DECLARE_DO_FUN(	do_transport	);
DECLARE_DO_FUN(	do_travel	);
DECLARE_DO_FUN(	do_tribegifts	);
DECLARE_DO_FUN(	do_truedisguise	);
DECLARE_DO_FUN(	do_truesight	);
DECLARE_DO_FUN(	do_turn		);
DECLARE_DO_FUN( do_ultimawave   );
DECLARE_DO_FUN(	do_unlock	);
DECLARE_DO_FUN(	do_unpolymorph	);
DECLARE_DO_FUN(	do_untie	);
DECLARE_DO_FUN(	do_unveil	);
DECLARE_DO_FUN(	do_unwerewolf	);
DECLARE_DO_FUN(	do_up		);
DECLARE_DO_FUN(	do_upkeep	);
DECLARE_DO_FUN(	do_users	);
DECLARE_DO_FUN(	do_value	);
DECLARE_DO_FUN(	do_vampire	);
DECLARE_DO_FUN(	do_vamptalk	);
DECLARE_DO_FUN(	do_vanish	);
DECLARE_DO_FUN( do_vamparmor    );
DECLARE_DO_FUN( do_wolfarmor    );
DECLARE_DO_FUN( do_exlist       );
DECLARE_DO_FUN( do_for          );
DECLARE_DO_FUN( do_vlist        );
DECLARE_DO_FUN( do_delevel      );
DECLARE_DO_FUN( do_ooc          );
DECLARE_DO_FUN( do_vopen        );
DECLARE_DO_FUN( do_clearreply   );
DECLARE_DO_FUN(	do_version	);
DECLARE_DO_FUN(	do_visible	);
DECLARE_DO_FUN( do_watercontrol );
DECLARE_DO_FUN(	do_weakness	);
DECLARE_DO_FUN(	do_web		);
DECLARE_DO_FUN(	do_wake		);
DECLARE_DO_FUN(	do_wallgranite	);
DECLARE_DO_FUN(	do_wear		);
DECLARE_DO_FUN(	do_wearaffect	);
DECLARE_DO_FUN(	do_weather	);
DECLARE_DO_FUN(	do_werewolf	);
DECLARE_DO_FUN(	do_west		);
DECLARE_DO_FUN( do_whelp	);
DECLARE_DO_FUN(	do_where	);
DECLARE_DO_FUN(	do_whisper	);
DECLARE_DO_FUN(	do_who		);
DECLARE_DO_FUN(	do_willpower	);
DECLARE_DO_FUN( do_windblades   );
DECLARE_DO_FUN( do_windwings    );
DECLARE_DO_FUN(	do_wings	);
DECLARE_DO_FUN(	do_wizhelp	);
DECLARE_DO_FUN( do_wizdox	);
DECLARE_DO_FUN(	do_wizlist	);
DECLARE_DO_FUN(	do_wizlock	);
DECLARE_DO_FUN( do_worldaff	);
DECLARE_DO_FUN(	do_wwpowers	);
DECLARE_DO_FUN(	do_wyld		);
DECLARE_DO_FUN( do_wyrmcall	);
DECLARE_DO_FUN(	do_yell		);
DECLARE_DO_FUN(	do_zap		);
DECLARE_DO_FUN(	do_zombie	);
DECLARE_DO_FUN(	do_zuloform	);
DECLARE_DO_FUN( do_chiroptean	);
DECLARE_DO_FUN( do_monk         );
DECLARE_DO_FUN( do_bridgewalk	);
DECLARE_DO_FUN( do_grasp	);

/* Drow functions */
DECLARE_DO_FUN(	do_attune	);
DECLARE_DO_FUN(	do_acidbolt	);
DECLARE_DO_FUN(	do_faeriefire	);
DECLARE_DO_FUN(	do_summonspider	);
DECLARE_DO_FUN(	do_drowarmor	);
DECLARE_DO_FUN(	do_door		);
DECLARE_DO_FUN(	do_prayer	);
DECLARE_DO_FUN(	do_skipdisc	);
DECLARE_DO_FUN(	do_drowupkeep	);
DECLARE_DO_FUN(	do_sign		);
DECLARE_DO_FUN(	do_globedark	);

/* Mage functions */
DECLARE_DO_FUN(	do_brittle_bones	);
DECLARE_DO_FUN(	do_double_cast	);
DECLARE_DO_FUN(	do_electrical_chaos	);
DECLARE_DO_FUN(	do_hearts_blood	);
DECLARE_DO_FUN(	do_inferno	);
DECLARE_DO_FUN(	do_mageenchant	);
DECLARE_DO_FUN(	do_magearmor	);
DECLARE_DO_FUN(	do_rift		);
DECLARE_DO_FUN(	do_polymorph_other	);

/* Ninja functions */
DECLARE_DO_FUN(	do_bomuzite	);
DECLARE_DO_FUN(	do_hakunetsu	);
DECLARE_DO_FUN(	do_harakiri	);
DECLARE_DO_FUN(	do_kakusu	);
DECLARE_DO_FUN(	do_michi	);
DECLARE_DO_FUN(	do_mienaku	);
DECLARE_DO_FUN(	do_miktalk	);
DECLARE_DO_FUN(	do_mizotamouyo	);
DECLARE_DO_FUN(	do_ninjaarmor	);
DECLARE_DO_FUN(	do_orimasu	);
DECLARE_DO_FUN(	do_principles	);
DECLARE_DO_FUN(	do_shuurai	);
DECLARE_DO_FUN(	do_tsumazuku	);
DECLARE_DO_FUN(	do_tsume	);

/* Sorcerer Functions */
DECLARE_DO_FUN(	do_chant	);
DECLARE_DO_FUN(	do_colors	);
DECLARE_DO_FUN( do_enchant	);
DECLARE_DO_FUN(	do_sorccreate	);
DECLARE_DO_FUN(	do_vision	);
DECLARE_DO_FUN(	do_write	);


DECLARE_DO_FUN(	do_blah		);
/*
 * Online creation commands  OLC
 */
/*DECLARE_DO_FUN( do_ashow        );
DECLARE_DO_FUN( do_rshow        );
DECLARE_DO_FUN( do_oshow        );
DECLARE_DO_FUN( do_mshow        );
*/
DECLARE_DO_FUN( do_aedit        );
DECLARE_DO_FUN( do_redit        );
DECLARE_DO_FUN( do_oedit        );
DECLARE_DO_FUN( do_medit        );

DECLARE_DO_FUN( do_resets       );

DECLARE_DO_FUN( do_asave        );

DECLARE_DO_FUN( do_arealist     );

/*
 * Imm Commands
 */
DECLARE_DO_FUN(	do_bamfin	);
DECLARE_DO_FUN(	do_bamfout	);
DECLARE_DO_FUN(	do_ban		);
DECLARE_DO_FUN(	do_goto		);
DECLARE_DO_FUN( do_tick		);


/*
 *  Oucast declarations
 */
DECLARE_DO_FUN(do_outcast	);
DECLARE_DO_FUN(do_noutcast	);
DECLARE_DO_FUN(do_moutcast	);

/*
 * Spell functions.
 * Defined in magic.c.
 */
DECLARE_SPELL_FUN(	spell_null		);
DECLARE_SPELL_FUN(	spell_acid_blast	);
DECLARE_SPELL_FUN(	spell_armor		);
DECLARE_SPELL_FUN(	spell_bless		);
DECLARE_SPELL_FUN(	spell_blindness		);
DECLARE_SPELL_FUN(	spell_burning_hands	);
DECLARE_SPELL_FUN(	spell_call_lightning	);
DECLARE_SPELL_FUN(	spell_cause_critical	);
DECLARE_SPELL_FUN(	spell_cause_light	);
DECLARE_SPELL_FUN(	spell_cause_serious	);
DECLARE_SPELL_FUN(	spell_change_sex	);
DECLARE_SPELL_FUN(	spell_charm_person	);
DECLARE_SPELL_FUN(	spell_chill_touch	);
DECLARE_SPELL_FUN(	spell_colour_spray	);
DECLARE_SPELL_FUN(	spell_continual_light	);
DECLARE_SPELL_FUN(	spell_control_weather	);
DECLARE_SPELL_FUN(	spell_create_food	);
DECLARE_SPELL_FUN(	spell_create_spring	);
DECLARE_SPELL_FUN(	spell_create_water	);
DECLARE_SPELL_FUN(	spell_cure_blindness	);
DECLARE_SPELL_FUN(	spell_cure_critical	);
DECLARE_SPELL_FUN(	spell_cure_light	);
DECLARE_SPELL_FUN(	spell_healing_circle	);
DECLARE_SPELL_FUN(	spell_cure_poison	);
DECLARE_SPELL_FUN(	spell_cure_serious	);
DECLARE_SPELL_FUN(	spell_curse		);
DECLARE_SPELL_FUN(	spell_detect_evil	);
DECLARE_SPELL_FUN(	spell_detect_hidden	);
DECLARE_SPELL_FUN(	spell_detect_invis	);
DECLARE_SPELL_FUN(	spell_detect_magic	);
DECLARE_SPELL_FUN(	spell_detect_poison	);
DECLARE_SPELL_FUN(	spell_dispel_evil	);
DECLARE_SPELL_FUN(	spell_dispel_magic	);
DECLARE_SPELL_FUN(	spell_earthquake	);
DECLARE_SPELL_FUN(	spell_enchant_weapon	);
DECLARE_SPELL_FUN(	spell_enchant_armor	);
DECLARE_SPELL_FUN(	spell_energy_drain	);
DECLARE_SPELL_FUN(	spell_faerie_fire	);
DECLARE_SPELL_FUN(	spell_faerie_fog	);
DECLARE_SPELL_FUN(	spell_fear		);
DECLARE_SPELL_FUN(	spell_fireball		);
DECLARE_SPELL_FUN(	spell_flamestrike	);
DECLARE_SPELL_FUN(	spell_fly		);
DECLARE_SPELL_FUN(	spell_gate		);
DECLARE_SPELL_FUN(	spell_general_purpose	);
DECLARE_SPELL_FUN(	spell_giant_strength	);
DECLARE_SPELL_FUN(	spell_harm		);
DECLARE_SPELL_FUN(	spell_heal		);
DECLARE_SPELL_FUN(	spell_high_explosive	);
DECLARE_SPELL_FUN(	spell_identify		);
DECLARE_SPELL_FUN(	spell_infravision	);
DECLARE_SPELL_FUN(	spell_invis		);
DECLARE_SPELL_FUN(	spell_know_alignment	);
DECLARE_SPELL_FUN(	spell_lightning_bolt	);
DECLARE_SPELL_FUN(	spell_locate_object	);
DECLARE_SPELL_FUN(	spell_magic_missile	);
DECLARE_SPELL_FUN(	spell_mass_invis	);
DECLARE_SPELL_FUN(	spell_pass_door		);
DECLARE_SPELL_FUN(	spell_poison		);
DECLARE_SPELL_FUN(	spell_protection	);
DECLARE_SPELL_FUN(	spell_refresh		);
DECLARE_SPELL_FUN(	spell_remove_curse	);
DECLARE_SPELL_FUN(	spell_sanctuary		);
DECLARE_SPELL_FUN(	spell_shocking_grasp	);
DECLARE_SPELL_FUN(	spell_shield		);
DECLARE_SPELL_FUN(	spell_sleep		);
DECLARE_SPELL_FUN(	spell_stone_skin	);
DECLARE_SPELL_FUN(	spell_summon		);
DECLARE_SPELL_FUN(	spell_teleport		);
DECLARE_SPELL_FUN(	spell_ventriloquate	);
DECLARE_SPELL_FUN(	spell_weaken		);
DECLARE_SPELL_FUN(	spell_word_of_recall	);
DECLARE_SPELL_FUN(	spell_acid_breath	);
DECLARE_SPELL_FUN(	spell_fire_breath	);
DECLARE_SPELL_FUN(	spell_frost_breath	);
DECLARE_SPELL_FUN(	spell_gas_breath	);
DECLARE_SPELL_FUN(	spell_lightning_breath	);

DECLARE_SPELL_FUN(	spell_guardian		);
DECLARE_SPELL_FUN(	spell_soulblade		);
DECLARE_SPELL_FUN(	spell_mana		);
DECLARE_SPELL_FUN(	spell_frenzy		);
DECLARE_SPELL_FUN(	spell_darkblessing	);
DECLARE_SPELL_FUN(	spell_transport		);
DECLARE_SPELL_FUN(	spell_quest		);
DECLARE_SPELL_FUN(	spell_mount		);
DECLARE_SPELL_FUN(	spell_insert_page	);
DECLARE_SPELL_FUN(	spell_chaos_blast	);
DECLARE_SPELL_FUN(	spell_web		);
DECLARE_SPELL_FUN(	spell_remove_page	);
DECLARE_SPELL_FUN(	spell_find_familiar	);
DECLARE_SPELL_FUN(	spell_improve		);
DECLARE_SPELL_FUN(	spell_tendrils		);

/*
 * OS-dependent declarations.
 * These are all very standard library functions,
 *   but some systems have incomplete or non-ansi header files.
 */
#if	defined(_AIX)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(apollo)
long	atoi		args( ( const char *string ) );
void *	calloc		args( ( unsigned nelem, size_t size ) );
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(hpux)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(interactive)
#endif
	
#if	defined(linux)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(macintosh)
#define NOCRYPT
#if	defined(unix)
#undef	unix
#endif
#endif

#if	defined(MIPS_OS)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(MSDOS)
#define NOCRYPT
#if	defined(unix)
#undef	unix
#endif
#endif

#if	defined(NeXT)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(sequent)
char *	crypt		args( ( const char *key, const char *salt ) );
long	fclose		args( ( FILE *stream ) );
long	fprintf		args( ( FILE *stream, const char *format, ... ) );
long	fread		args( ( void *ptr, long size, long n, FILE *stream ) );
long	fseek		args( ( FILE *stream, long offset, long ptrname ) );
void	perror		args( ( const char *s ) );
long	ungetc		args( ( long c, FILE *stream ) );
#endif

#if	defined(sun)
char *	crypt		args( ( const char *key, const char *salt ) );
long	fclose		args( ( FILE *stream ) );
long	fprintf		args( ( FILE *stream, const char *format, ... ) );
#if 	defined(SYSV)
size_t 	fread		args( ( void *ptr, size_t size, size_t n, 
				FILE *stream ) );
#else
long	fread		args( ( void *ptr, long size, long n, FILE *stream ) );
#endif
long	fseek		args( ( FILE *stream, long offset, long ptrname ) );
void	perror		args( ( const char *s ) );
long	ungetc		args( ( long c, FILE *stream ) );
#endif

#if	defined(ultrix)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif



/* Password storage requires libxcrypt; never build a plaintext fallback. */
#if	defined(NOCRYPT)
#error "NOCRYPT is unsupported: install libxcrypt with yescrypt support."
#endif



/*
 * Data files used by the server.
 *
 * AREA_LIST contains a list of areas to boot.
 * All files are read in completely at bootup.
 * Most output files (bug, idea, typo, shutdown) are append-only.
 *
 * The NULL_FILE is held open so that we have a stream handle in reserve,
 *   so players can go ahead and telnet to all the other descriptors.
 * Then we close it whenever we need to open a file (e.g. a save file).
 */
#define PLAYER_DIR	"../player/"	/* Player files			*/
#define BACKUP_DIR	"../backup/"    /* Backup Directory		*/
#define NULL_FILE	"/dev/null"	/* To reserve one stream	*/

#define AREA_LIST	"area.lst"	/* List of areas		*/
#define SHUTDOWN_FILE	"shutdown.txt"	/* For 'shutdown'		*/
#define DISABLED_FILE	"disabled.txt"	/* For 'disable'		*/
#define EXE_FILE        "../src/merc"   /* For signals */
#define COPYOVER_FILE   "copyover.data" /* For signals */


/*
 * Our function prototypes.
 * One big lump ... this is every function in Merc.
 */
#define CD	CHAR_DATA
#define MID	MOB_INDEX_DATA
#define OD	OBJ_DATA
#define OID	OBJ_INDEX_DATA
#define RID	ROOM_INDEX_DATA
#define SF	SPEC_FUN

/* act_comm.c */
void	add_follower	args( ( CHAR_DATA *ch, CHAR_DATA *master ) );
void	stop_follower	args( ( CHAR_DATA *ch ) );
void	die_follower	args( ( CHAR_DATA *ch ) );
bool	is_same_group	args( ( CHAR_DATA *ach, CHAR_DATA *bch ) );
void	room_text	args( ( CHAR_DATA *ch, char *argument ) );
char    *strlower       args( ( char * ip ) );
void	excessive_cpu	args( ( long blx ) );
bool	check_parse_name args( ( char *name ) );
bool	is_in		args( (char *, char *) );
bool	all_in		args( (char *, char *) );

/* act_info.c */
void	set_title	args( ( CHAR_DATA *ch, char *title ) );
void	show_list_to_char args( ( OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing ) );
long    get_speed	args( ( CHAR_DATA *ch ) );
long	char_hitroll	args( ( CHAR_DATA *ch ) );
long	char_damroll	args( ( CHAR_DATA *ch ) );
long	char_ac		args( ( CHAR_DATA *ch ) );
int	raw_stat	args( ( CHAR_DATA *ch, int stat ) );
int	char_stat	args( ( CHAR_DATA *ch, int stat ) );
int	stat_max	args( ( CHAR_DATA *ch, int stat ) );
long	stat_cost	args( ( CHAR_DATA *ch, int stat ) );
long	get_celerity	args( ( CHAR_DATA *ch ) );
long	get_disc	args( ( CHAR_DATA *ch, long disc ) );
long	get_truedisc	args( ( CHAR_DATA *ch, long disc ) );
long	get_breed	args( ( CHAR_DATA *ch, long gift ) );
long	get_auspice	args( ( CHAR_DATA *ch, long gift ) );
long	get_tribe	args( ( CHAR_DATA *ch, long gift ) );
long	get_color	args( ( CHAR_DATA *ch, long color ) );
long	get_highland	args( ( CHAR_DATA *ch, long power ) );
long	get_ninja	args( ( CHAR_DATA *ch, long power ) );
long	get_monk	args( ( CHAR_DATA *ch, long power ) );
bool	get_drow	args( ( CHAR_DATA *ch, long power ) );
long	get_style	args( ( CHAR_DATA *ch, long style ) );

/* act_move.c */
void	move_char	args( ( CHAR_DATA *ch, long door ) );
void	open_lift	args( ( CHAR_DATA *ch ) );
void	close_lift	args( ( CHAR_DATA *ch ) );
void	move_lift	args( ( CHAR_DATA *ch, long to_room ) );
void	move_door	args( ( CHAR_DATA *ch ) );
void	thru_door	args( ( CHAR_DATA *ch, long doorexit ) );
void	open_door	args( ( CHAR_DATA *ch, bool be_open ) );
bool	is_open		args( ( CHAR_DATA *ch ) );
bool	same_floor	args( ( CHAR_DATA *ch, long cmp_room ) );

/* act_obj.c */
bool	is_ok_to_wear	args( ( CHAR_DATA *ch, bool wolf_ok, char *argument ) );
void	quest_object	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
bool	remove_obj	args( ( CHAR_DATA *ch, long iWear, bool fReplace ) );
void	wear_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace ) );
bool	objects_in_room	args( ( CHAR_DATA *ch ) );
bool	objects_in_container	args( ( CHAR_DATA *ch, OBJ_DATA *container ) );
bool	objects_in_hand	args( ( CHAR_DATA *ch ) );
void	mortal_vamp	args( ( CHAR_DATA *ch ) );

/* act_wiz.c */
void	bind_char	args( ( CHAR_DATA *ch ) );
void	logchan		args( ( char *argument ) );
void	sysmes		args( ( char *argument ) );
void    copyover_recover args( ( void ) );
ROOM_INDEX_DATA *find_location( CHAR_DATA *ch, char *arg );	/* OLC */

/* Signals.c */
void init_signals( void );
#define SIGPFILE      77
#define stc send_to_char

void attempt_copyover( void );

/* random.c */
void	mageupkeep		args( ( CHAR_DATA *ch ) );
void	check_leaderboard	args( ( CHAR_DATA *ch, char *argument ) );
void	load_leaderboard	args( ( void ) );
void	save_leaderboard	args( ( void ) );
long	strlen2			args( ( const char *s ) );

/* comm.c */
void	close_socket	args( ( DESCRIPTOR_DATA *dclose ) );
void	close_socket2	args( ( DESCRIPTOR_DATA *dclose ) );
void	write_to_buffer	args( ( DESCRIPTOR_DATA *d, const char *txt, long length ) );
void	send_to_char	args( ( const char *txt, CHAR_DATA *ch ) );
void	page_to_char	args( ( const char *txt, CHAR_DATA *ch ) );
void	send_to_all_char args( ( const char *text ) );
void	act		args( ( const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2, long type ) );
void	act2		args( ( const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2, long type ) );
void	kavitem		args( ( const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2, long type ) );
void    mud_logf        args( ( char *fmt, ... ) );
bool password_matches(const char *argument, const char *stored, bool allow_plaintext);

/* prototypes from db.c */
void  load_disabled   args( ( void ) );
void  save_disabled   args( ( void ) );
void  load_bans       args( ( void ) );
void  save_bans       args( ( void ) );

/* db.c */
char *  print_flags	args( ( long flag ));
void	boot_db		args( ( void ) );
void	area_update	args( ( void ) );
CD *	create_mobile	args( ( MOB_INDEX_DATA *pMobIndex ) );
OD *	create_object	args( ( OBJ_INDEX_DATA *pObjIndex, long level ) );
void	clear_char	args( ( CHAR_DATA *ch ) );
void	free_char	args( ( CHAR_DATA *ch ) );
char *	get_extra_descr	args( ( const char *name, EXTRA_DESCR_DATA *ed ) );
char *	get_roomtext	args( ( const char *name, ROOMTEXT_DATA    *rt ) );
MID *	get_mob_index	args( ( long vnum ) );
OID *	get_obj_index	args( ( long vnum ) );
RID *	get_room_index	args( ( long vnum ) );
char	fread_letter	args( ( FILE *fp ) );
long	fread_number	args( ( FILE *fp ) );
long	fread_flag	args( ( FILE *fp ) );
char *	fread_string	args( ( FILE *fp ) );
void	fread_to_eol	args( ( FILE *fp ) );
char *	fread_word	args( ( FILE *fp ) );
void *	alloc_mem	args( ( long sMem ) );
void *	alloc_perm	args( ( long sMem ) );
long	flag_convert	args( ( char letter ) );
void	free_mem	args( ( void *pMem, long sMem ) );
char *	str_dup		args( ( const char *str ) );
void	free_string	args( ( char *pstr ) );
int	number_fuzzy	args( ( int number ) );
int	number_range	args( ( int from, int to ) );
int	number_percent	args( ( void ) );
int	number_door	args( ( void ) );
int	number_bits	args( ( int width ) );
int	number_mm	args( ( void ) );
int	dice		args( ( int number, int size ) );
int	interpolate	args( ( int level, int value_00, int value_32 ) );
void	smash_tilde	args( ( char *str ) );
bool	str_cmp		args( ( const char *astr, const char *bstr ) );
bool	str_prefix	args( ( const char *astr, const char *bstr ) );
bool	str_infix	args( ( const char *astr, const char *bstr ) );
bool	str_suffix	args( ( const char *astr, const char *bstr ) );
char *	capitalize	args( ( const char *str ) );
char *	upper_cap	args( ( const char *str ) );
void	append_file	args( ( CHAR_DATA *ch, char *file, char *str ) );
void	bug		args( ( const char *str, long param ) );
void	log_string	args( ( const char *str ) );
void	tail_chain	args( ( void ) );
void	reset_area      args( ( AREA_DATA * pArea ) );	/* OLC */

extern LEADER_BOARD leader_board;

/* fight.c */
void    combat_status_update args( ( void ) );
void	violence_update	args( ( void ) );
void	multi_hit	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
long	get_speed	args( ( CHAR_DATA *ch ) );
void	one_hit		args( ( CHAR_DATA *ch, CHAR_DATA *victim, long dt, char *message ) );
void	damage		args( ( CHAR_DATA *ch, CHAR_DATA *victim, long dam, long dt, char *message ) );
void	group_gain	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	update_pos	args( ( CHAR_DATA *victim ) );
void	set_fighting	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	stop_fighting	args( ( CHAR_DATA *ch, bool fBoth ) );
bool	is_safe		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	auto_safe	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	hurt_fighting	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	hurt_person	args( ( CHAR_DATA *ch, CHAR_DATA *victim, long dam ) );
void	killperson	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	death_message	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
long	dambonus	args( ( CHAR_DATA *ch, CHAR_DATA *victim, long dam ) );
void	dp_gain		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
long	get_speed	args( ( CHAR_DATA *ch ) );
long	mod_damcap	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
long	get_damcap	args( ( CHAR_DATA *ch ) );
long	get_soak	args( ( CHAR_DATA *victim ) );
bool	check_avoid	args( ( CHAR_DATA *ch, CHAR_DATA *victim, bool weap ) );
long	avoidblow	args( ( CHAR_DATA *ch ) );
long	breakavoid	args( ( CHAR_DATA *ch ) );
void	make_head	args( ( CHAR_DATA *ch ) );
void	raw_kill	args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void	make_corpse	args( ( CHAR_DATA *ch, CHAR_DATA *maker ) );
void	behead		args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void	paradox		args( ( CHAR_DATA *ch ) );
long	xp_compute	args( ( CHAR_DATA *victim ) );

/* handler.c */
long	get_trust	args( ( CHAR_DATA *ch ) );
long	get_hours	args( ( CHAR_DATA *ch ) );
bool	is_name		args( ( const char *str, char *namelist ) );
bool	is_abbr		args( ( const char *str, char *namelist ) );
long	loc_name	args( ( const char *str, char *namelist ) );
void	affect_to_char	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	affect_remove	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	affect_strip	args( ( CHAR_DATA *ch, long sn ) );
bool	is_affected	args( ( CHAR_DATA *ch, long sn ) );
void	affect_join	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	char_from_room	args( ( CHAR_DATA *ch ) );
void	char_to_room	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex ) );
void	obj_to_char	args( ( OBJ_DATA *obj, CHAR_DATA *ch ) );
void	obj_from_char	args( ( OBJ_DATA *obj ) );
long	apply_ac	args( ( OBJ_DATA *obj, long iWear ) );
OD *	get_eq_char	args( ( CHAR_DATA *ch, long iWear ) );
void	equip_char	args( ( CHAR_DATA *ch, OBJ_DATA *obj, long iWear ) );
void	unequip_char	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
long	count_obj_list	args( ( OBJ_INDEX_DATA *obj, OBJ_DATA *list ) );
void	obj_from_room	args( ( OBJ_DATA *obj ) );
void	obj_to_room	args( ( OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex ) );
void	obj_to_obj	args( ( OBJ_DATA *obj, OBJ_DATA *obj_to ) );
void	obj_from_obj	args( ( OBJ_DATA *obj ) );
void	extract_obj	args( ( OBJ_DATA *obj ) );
void	extract_char	args( ( CHAR_DATA *ch, bool fPull ) );
CD *	get_char	args( ( CHAR_DATA *ch ) );
CD *	get_char_room	args( ( CHAR_DATA *ch, char *argument ) );
CD *	get_char_world	args( ( CHAR_DATA *ch, char *argument ) );
CD *	get_char_world2	args( ( CHAR_DATA *ch, char *argument ) );
CD *	get_char_finger	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_type	args( ( OBJ_INDEX_DATA *pObjIndexData ) );
OD *	get_obj_list	args( ( CHAR_DATA *ch, char *argument,
			    OBJ_DATA *list ) );
OD *	get_obj_in_obj	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_carry	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_wear	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_here	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_room	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_world	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_world2	args( ( CHAR_DATA *ch, char *argument ) );
OD *	create_money	args( ( long amount ) );
long	get_obj_number	args( ( OBJ_DATA *obj ) );
long	get_obj_weight	args( ( OBJ_DATA *obj ) );
bool	room_is_dark	args( ( ROOM_INDEX_DATA *pRoomIndex ) );
bool	room_is_private	args( ( ROOM_INDEX_DATA *pRoomIndex ) );
bool	can_see		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	can_see_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
char *	affect_loc_name	args( ( long location ) );
char *	affect_bit_name	args( ( long vector ) );
void	affect_modify	args( ( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd ) );
long 	str_close	args( ( char *a, char *b ) );

/* one disabled command */
struct disabled_data
{
      DISABLED_DATA *next; /* pointer to next node */
      struct cmd_type const *command; /* pointer to the command struct*/
      char *disabled_by; /* name of disabler */
      long level; /* level of disabler */
};

extern                        DISABLED_DATA     *disabled_first;
extern                        BAN_DATA          *ban_first;

/* interp.c */
void	interpret	args( ( CHAR_DATA *ch, char *argument ) );
bool	is_number	args( ( char *arg ) );
long	number_argument	args( ( char *argument, char *arg ) );
char *	one_argument	args( ( char *argument, char *arg_first ) );

/* magic.c */
long	skill_lookup	args( ( const char *name ) );
long	slot_lookup	args( ( long slot ) );
long	sn_lookup	args( ( long sn ) );
bool	saves_spell	args( ( long level, CHAR_DATA *victim ) );
void	obj_cast_spell	args( ( long sn, long level, CHAR_DATA *ch,
				    CHAR_DATA *victim, OBJ_DATA *obj ) );
void	enhance_stat	args( ( long sn, long level, CHAR_DATA *ch, 
				    CHAR_DATA *victim, long apply_bit,
				    long bonuses, long affect_bit ) );

/* random.c */
long	get_ratio	args( ( CHAR_DATA *ch ) );

/* save.c */
bool	save_char_obj_checked	args( ( CHAR_DATA *ch ) );
void	save_char_obj		args( ( CHAR_DATA *ch ) );
void	save_char_obj_backup	args( ( CHAR_DATA *ch ) );
bool	load_char_obj		args( ( DESCRIPTOR_DATA *d, char *name ) );
bool	load_char_short		args( ( DESCRIPTOR_DATA *d, char *name ) );

/* special.c */
SF *	spec_lookup	args( ( const char *name ) );

/* style.c */
void	update_style	args( ( CHAR_DATA *ch ) );
void	improve_stance	args( ( CHAR_DATA *ch ) );
void	improve_style	args( ( CHAR_DATA *ch ) );
void	improve_weapon	args( ( CHAR_DATA *ch, long skill ) );

/* update.c */
void	gain_condition	args( ( CHAR_DATA *ch, long iCond, long value ) );
void	update_handler	args( ( void ) );

/* kav_fight.c */
void	beast_rage	args( ( CHAR_DATA *ch ) );
void	autostance	args( ( CHAR_DATA *ch ) );

/* kav_info.c */
long	get_totalscore	args( ( CHAR_DATA *ch ) );
bool	play_yet	args( ( CHAR_DATA *ch ) );
long	date_to_int	args( ( char *argument ) );

/* kav_wiz.c */
void	oset_affect	args( ( CHAR_DATA *ch, OBJ_DATA *obj, long value, long affect, bool is_quest) );
void	clear_stats	args( ( CHAR_DATA *ch ) );

/* clan.c */
void	vamp_rage	args( ( CHAR_DATA *ch ) );
bool	char_exists	args( ( char *name ) );
OD *	get_page	args( ( OBJ_DATA *book, long page_num ) );
void	update_gen	args( ( CHAR_DATA *ch ) );
long	true_generation	args( ( CHAR_DATA *ch, char *argument ) );
void	mummify_corpse	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
void	poison_weapon	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
char *	disc_name	args( ( long disc_no ) );

/* more.c */
void	guardian_message args( ( CHAR_DATA *ch ) );

/* string.c */
char *  format_string   args( ( char *oldstring /*, bool fSpace */ ) );	/* OLC */
void	string_edit	args( ( CHAR_DATA *ch, char **pString ) );	/* OLC */
void    string_add      args( ( CHAR_DATA *ch, char *argument ) );	/* OLC */
void    string_append   args( ( CHAR_DATA *ch, char **pString ) );	/* OLC */
char *  first_arg       args( ( char *argument, char *arg_first, bool fCase ) );	/* OLC */
char *	string_replace	args( ( char * orig, char * old, char * new ) );	/* OLC */
char *	string_proper	args( ( char * argument ) );	/* OLC */
char *	string_unpad	args( ( char * argument ) );	/* OLC */
long	arg_count	args( ( char *argument ) );	/* OLC */

/* trap.c */
void	trigger_trap	args( ( CHAR_DATA *ch, TRAP_DATA *trap ) );
void	tdamage		args( ( CHAR_DATA *ch, TRAP_DATA *trap ) );
void	tspell		args( ( CHAR_DATA *ch, TRAP_DATA *trap ) );
void	tmove		args( ( CHAR_DATA *ch, TRAP_DATA *trap ) );
void	tsummon		args( ( CHAR_DATA *ch, TRAP_DATA *trap ) );

/* olc.c */
void    aedit           args( ( CHAR_DATA *ch, char *argument ) );	/* OLC */
void    redit           args( ( CHAR_DATA *ch, char *argument ) );	/* OLC */
void    medit           args( ( CHAR_DATA *ch, char *argument ) );	/* OLC */
void    oedit           args( ( CHAR_DATA *ch, char *argument ) );	/* OLC */
/*void	add_reset	args( ( AREA_DATA *area, RESET_DATA *pReset, long index ) );	
OLC */

/* bit.c */
char *	room_bit_name		args( ( long room_flags ) );	/* OLC */
long	room_name_bit		args( ( char *buf ) );		/* OLC */
long	sector_number		args( ( char *argument ) );	/* OLC */
char *	sector_name		args( ( long sect ) );		/* OLC */
long	item_name_type		args( ( char *name ) ); 	/* OLC */
char *	item_type_name		args( ( long item_type ) );	/* OLC */
long	extra_name_bit		args( ( char* buf ) );		/* OLC */
char *	extra_bit_name		args( ( long extra_flags ) ); 	/* OLC */
long	wear_name_bit		args( ( char* buf ) );		/* OLC */
char *	wear_bit_name		args( ( long wear ) );		/* OLC */
long	act_name_bit		args( ( char* buf ) );		/* OLC */
long	affect_name_bit		args( ( char* buf ) );		/* OLC */
long	affect_name_loc		args( ( char* name ) );		/* OLC */
long     wear_name_loc   	args( ( char *buf ) );		/* OLC */
char *	wear_loc_name		args( ( long wearloc ) );	/* OLC */
char *	act_bit_name		args( ( long act ) );		/* OLC */
long	get_weapon_type		args( ( char *arg ) );		/* OLC */
char *	get_type_weapon		args( ( long arg ) );		/* OLC */
long	get_container_flags	args( ( char *arg ) );		/* OLC */
char *	get_flags_container	args( ( long arg ) );		/* OLC */
long	get_liquid_type		args( ( char *arg ) );		/* OLC */
char *	get_type_liquid		args( ( long arg ) );		/* OLC */

#undef	CD
#undef	MID
#undef	OD
#undef	OID
#undef	RID
#undef	SF
