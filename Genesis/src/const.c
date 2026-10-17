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
#include <stdio.h>
#include <time.h>
#include "merc.h"

/*
* Liquid properties.
* Used in world.obj.
*/
const	struct	liq_type	liq_table	[LIQ_MAX]	=
{
    { "water",			"clear",	{  0, 1, 10 }	},  /*  0 */
    { "beer",			"amber",	{  3, 2,  5 }	},
    { "wine",			"rose",		{  5, 2,  5 }	},
    { "ale",			"brown",	{  2, 2,  5 }	},
    { "dark ale",		"dark",		{  1, 2,  5 }	},

    { "whisky",			"golden",	{  6, 1,  4 }	},  /*  5 */
    { "lemonade",		"pink",		{  0, 1,  8 }	},
    { "firebreather",		"boiling",	{ 10, 0,  0 }	},
    { "local specialty",	"everclear",	{  3, 3,  3 }	},
    { "slime mold juice",	"green",	{  0, 4, -8 }	},

    { "milk",			"white",	{  0, 3,  6 }	},  /* 10 */
    { "tea",			"tan",		{  0, 1,  6 }	},
    { "coffee",			"black",	{  0, 1,  6 }	},
    { "blood",			"red",		{  0, 0,  5 }	},
    { "salt water",		"clear",	{  0, 1, -2 }	},

    { "cola",			"cherry",	{  0, 1,  5 }	}   /* 15 */
};



/*
* The skill and spell table.
* Slot numbers must never be changed as they appear in #OBJECTS sections.
*/
#define SLOT(n)	n

const	struct	skill_type	skill_table	[MAX_SKILL]	=
{

/*
* Magic spells.
*/

    {
	"reserved",		99,
	0,			TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT( 0),	 0,	 0,
	"",			""
    },

    {
	"acid blast",		LEV_SLE,
	spell_acid_blast,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(70),	20,	12,
	"acid blast",		"!Acid Blast!"
    },

    {
	"armor",		LEV_AWA,
	spell_armor,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT( 1),	 5,	12,
	"",			"You feel less protected."
    },

    {
	"bless",		LEV_AWA,
	spell_bless,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT( 3),	 5,	12,
	"",			"You feel less righteous."
    },

    {
	"blindness",		LEV_SLE,
	spell_blindness,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_blindness,		SLOT( 4),	 5,	12,
	"",			"You can see again."
    },

    {
	"burning hands",	LEV_SLE,
	spell_burning_hands,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT( 5),	15,	12,
	"burning hands",	"!Burning Hands!"
    },

    {
	"call lightning",	LEV_SLE,
	spell_call_lightning,	TAR_IGNORE,		POS_FIGHTING,
	NULL,			SLOT( 6),	15,	12,
	"lightning bolt",	"!Call Lightning!"
    },

    {
	"cause critical",	LEV_SLE,
	spell_cause_critical,	TAR_DEATH,      	POS_FIGHTING,
	NULL,			SLOT(63),	20,	12,
	"spell",		"!Cause Critical!"
    },

    {
	"cause light",		LEV_SLE,
	spell_cause_light,	TAR_DEATH,      	POS_FIGHTING,
	NULL,			SLOT(62),	15,	12,
	"spell",		"!Cause Light!"
    },

    {
	"cause serious",	LEV_SLE,
	spell_cause_serious,	TAR_DEATH,      	POS_FIGHTING,
	NULL,			SLOT(64),	17,	12,
	"spell",		"!Cause Serious!"
    },

    {
	"change sex",		99,
	spell_change_sex,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(82),	15,	12,
	"",			"Your body feels familiar again."
    },

    {
	"charm person",		LEV_SOR,
	spell_charm_person,	TAR_CHAR_OFFENSIVE,	POS_STANDING,
	&gsn_charm_person,	SLOT( 7),	 5,	12,
	"",			"You feel more self-confident."
    },

    {
	"chill touch",		LEV_SLE,
	spell_chill_touch,	TAR_DEATH,      	POS_FIGHTING,
	NULL,			SLOT( 8),	15,	12,
	"chilling touch",	"You feel less cold."
    },

    {
	"colour spray",		LEV_SLE,
	spell_colour_spray,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(10),	15,	12,
	"colour spray",		"!Colour Spray!"
    },

    {
	"continual light",	LEV_SLE,
	spell_continual_light,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(57),	 7,	2,
	"",			"!Continual Light!"
    },

    {
	"control weather",	LEV_AWA,
	spell_control_weather,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(11),	25,	12,
	"",			"!Control Weather!"
    },

    {
	"create food",		LEV_SLE,
	spell_create_food,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(12),	 5,	2,
	"",			"!Create Food!"
    },

    {
	"create spring",	LEV_SLE,
	spell_create_spring,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(80),	20,	2,
	"",			"!Create Spring!"
    },

    {
	"create water",		99,
	spell_create_water,	TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(13),	 5,	2,
	"",			"!Create Water!"
    },

    {
	"cure blindness",	LEV_SLE,
	spell_cure_blindness,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(14),	 5,	12,
	"",			"!Cure Blindness!"
    },

    {
	"cure critical",	LEV_SLE,
	spell_cure_critical,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(15),	20,	12,
	"",			"!Cure Critical!"
    },

    {
	"cure light",		LEV_SLE,
	spell_cure_light,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(16),	10,	12,
	"",			"!Cure Light!"
    },

    {
	"cure poison",		LEV_SLE,
	spell_cure_poison,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(43),	 5,	12,
	"",			"!Cure Poison!"
    },

    {
	"cure serious",		LEV_SLE,
	spell_cure_serious,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(61),	15,	12,
	"",			"!Cure Serious!"
    },

    {
	"curse",		LEV_SLE,
	spell_curse,		TAR_CHAR_OFFENSIVE,	POS_STANDING,
	&gsn_curse,		SLOT(17),	20,	12,
	"curse",		"The curse wears off."
    },

    {
	"detect evil",		99,
	spell_detect_evil,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(18),	 5,	2,
	"",			"The red in your vision disappears."
    },

    {
	"detect hidden",	99,
	spell_detect_hidden,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(44),	 5,	2,
	"",			"You feel less aware of your suroundings."
    },

    {
	"detect invis",		LEV_SLE,
	spell_detect_invis,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(19),	 5,	2,
	"",			"You no longer see invisible objects."
    },

    {
	"detect magic",		99,
	spell_detect_magic,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(20),	 5,	2,
	"",			"The detect magic wears off."
    },

    {
	"detect poison",	99,
	spell_detect_poison,	TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(21),	 5,	2,
	"",			"!Detect Poison!"
    },

    {
	"dispel evil",		99,
	spell_dispel_evil,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(22),	15,	12,
	"dispel evil",		"!Dispel Evil!"
    },

    {
	"dispel magic",		LEV_SLE,
	spell_dispel_magic,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(59),	1000,	12,
	"",			"!Dispel Magic!"
    },

    {
	"earthquake",		LEV_SLE,
	spell_earthquake,	TAR_IGNORE,		POS_FIGHTING,
	NULL,			SLOT(23),	15,	12,
	"earthquake",		"!Earthquake!"
    },

    {
	"enchant weapon",	LEV_SLE,
	spell_enchant_weapon,	TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(24),	100,	2,
	"",			"!Enchant Weapon!"
    },

    {
	"energy drain",		LEV_AWA,
	spell_energy_drain,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(25),	35,	12,
	"energy drain",		"!Energy Drain!"
    },

    {
	"faerie fire",		LEV_SLE,
	spell_faerie_fire,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(72),	 5,	12,
	"faerie fire",		"The pink aura around you fades away."
    },

    {
	"faerie fog",		LEV_SLE,
	spell_faerie_fog,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(73),	250,	8,
	"faerie fog",		"!Faerie Fog!"
    },

    {
	"fireball",		LEV_SLE,
	spell_fireball,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(26),	15,	12,
	"fireball",		"!Fireball!"
    },

    {
	"flamestrike",		LEV_SLE,
	spell_flamestrike,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(65),	20,	12,
	"flamestrike",		"!Flamestrike!"
    },

    {
	"fly",			LEV_SLE,
	spell_fly,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(56),	10,	18,
	"",			"You slowly float to the ground."
    },

    {
	"gate",			99,
	spell_gate,		TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(83),	50,	12,
	"",			"!Gate!"
    },

    {
	"giant strength",	99,
	spell_giant_strength,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(39),	20,	12,
	"",			"You feel weaker."
    },

    {
	"harm",			LEV_SLE,
	spell_harm,		TAR_DEATH,		POS_FIGHTING,
	NULL,			SLOT(27),	750,	12,
	"harm spell",		"!Harm!"
    },

    {
	"heal",			LEV_SLE,
	spell_heal,		TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(28),	750,	12,
	"",			"!Heal!"
    },

    {
	"identify",		LEV_SLE,
	spell_identify,		TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(53),	12,	2,
	"",			"!Identify!"
    },

    {
	"infravision",		LEV_SLE,
	spell_infravision,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(77),	 5,	2,
	"",			"You no longer see in the dark."
    },

    {
	"invis",		LEV_SLE,
	spell_invis,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	&gsn_invis,		SLOT(29),	 5,	12,
	"",			"You are no longer invisible."
    },

    {
	"know alignment",	99,
	spell_know_alignment,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(58),	 9,	2,
	"",			"!Know Alignment!"
    },

    {
	"lightning bolt",	LEV_SLE,
	spell_lightning_bolt,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(30),	15,	12,
	"lightning bolt",	"!Lightning Bolt!"
    },

    {
	"locate object",	LEV_SLE,
	spell_locate_object,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(31),	20,	2,
	"",			"!Locate Object!"
    },

    {
	"magic missile",	LEV_SLE,
	spell_magic_missile,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(32),	15,	4,
	"magic missile",	"!Magic Missile!"
    },

    {
	"mass invis",		LEV_SOR,
	spell_mass_invis,	TAR_IGNORE,		POS_STANDING,
	&gsn_mass_invis,	SLOT(69),	20,	24,
	"",			"!Mass Invis!"
    },

    {
	"pass door",		99,
	spell_pass_door,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(74),	20,	2,
	"",			"You feel solid again."
    },

    {
	"poison",		LEV_SLE,
	spell_poison,		TAR_CHAR_OFFENSIVE,	POS_STANDING,
	&gsn_poison,		SLOT(33),	10,	12,
	"poison",		"You feel less sick."
    },

    {
	"protection",		99,
	spell_protection,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(34),	 5,	12,
	"",			"You feel less protected."
    },

    {
	"refresh",		99,
	spell_refresh,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(81),	12,	2,
	"refresh",		"!Refresh!"
    },

    {
	"remove curse",		LEV_SLE,
	spell_remove_curse,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(35),	 5,	2,
	"",			"!Remove Curse!"
    },

    {
	"sanctuary",		LEV_SLE,
	spell_sanctuary,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(36),	500,	12,
	"",			"The white aura around your body fades."
    },

    {
	"shield",		LEV_AWA,
	spell_shield,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(67),	12,	18,
	"",			"Your force shield shimmers then fades away."
    },

    {
	"shocking grasp",	LEV_SLE,
	spell_shocking_grasp,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(37),	15,	12,
	"shocking grasp",	"!Shocking Grasp!"
    },

    {
	"sleep",		99,
	spell_sleep,		TAR_CHAR_OFFENSIVE,	POS_STANDING,
	&gsn_sleep,		SLOT(38),	15,	12,
	"",			"You feel less tired."
    },

    {
	"stone skin",		LEV_AWA,
	spell_stone_skin,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(66),	12,	18,
	"",			"Your skin feels soft again."
    },

    {
	"summon",		LEV_SOR,
	spell_summon,		TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(40),	500,	12,
	"",			"!Summon!"
    },

    {
	"teleport",		LEV_SLE,
	spell_teleport,		TAR_CHAR_SELF,		POS_FIGHTING,
	NULL,	 		SLOT( 2),	500,	24,
	"",			"!Teleport!"
    },

    {
	"ventriloquate",	99,
	spell_ventriloquate,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(41),	 5,	2,
	"",			"!Ventriloquate!"
    },

    {
	"weaken",		99,
	spell_weaken,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(68),	20,	12,
	"spell",		"You feel stronger."
    },

    {
	"word of recall",	99,
	spell_word_of_recall,	TAR_CHAR_SELF,		POS_RESTING,
	NULL,			SLOT(42),	 5,	2,
	"",			"!Word of Recall!"
    },

/*
* Dragon breath
*/
    {
	"acid breath",		LEV_SLE,
	spell_acid_breath,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(200),	 300,	 12,
	"blast of acid",	"!Acid Breath!"
    },

    {
	"fire breath",		LEV_SLE,
	spell_fire_breath,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(201),	 200,	 12,
	"blast of flame",	"!Fire Breath!"
    },

    {
	"frost breath",		LEV_SLE,
	spell_frost_breath,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(202),	 300,	 12,
	"blast of frost",	"!Frost Breath!"
    },

    {
	"gas breath",		LEV_SLE,
	spell_gas_breath,	TAR_IGNORE,		POS_FIGHTING,
	NULL,			SLOT(203),	 500,	 15,
	"blast of gas",		"!Gas Breath!"
    },

    {
	"lightning breath",	LEV_SLE,
	spell_lightning_breath,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(204),	 300,	 12,
	"blast of lightning",	"!Lightning Breath!"
    },

/*
* Fighter and thief skills.
*/
    {
	"hide",			99,
	spell_null,		TAR_IGNORE,		POS_RESTING,
	&gsn_hide,		SLOT( 0),	 0,	12,
	"",			"!Hide!"
    },

    {
	"sneak",		99,
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_sneak,		SLOT( 0),	 0,	12,
	"",			"Your footsteps are no longer so quiet."
    },

/*
* Spells for mega1.are from Glop/Erkenbrand.
*/
    {
	"general purpose",	99,
	spell_general_purpose,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(501),	0,	12,
	"general purpose ammo",	"!General Purpose Ammo!"
    },

    {
	"high explosive",	99,
	spell_high_explosive,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(502),	0,	12,
	"high explosive ammo",	"!High Explosive Ammo!"
    },

/*
* Spells added by KaVir.
*/
    {
	"guardian",		LEV_SLE,
	spell_guardian,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(600),	100,	12,
	"",			"!Guardian!"
    },

    {
	"soulblade",		LEV_SLE,
	spell_soulblade,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(601),	100,	12,
	"",			"!Soulblade!"
    },

    {
	"mana",			LEV_SLE,
	spell_mana,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(602),	 0,	12,
	"",			"!Mana!"
    },

    {
	"frenzy",		99,
	spell_frenzy,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(603),	 20,	12,
	"",			"Your bloodlust subsides."
    },

    {
	"darkblessing",		99,
	spell_darkblessing,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(604),	 20,	12,
	"",			"You feel less wicked."
    },

    {
	"transport",		LEV_SOR,
	spell_transport,	TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(607),	250,	2,
	"",			"!Transport!"
    },

    {
	"quest",		LEV_SLE,
	spell_quest,		TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(611),	 500,	2,
	"",			"!Quest!"
    },

    {
	"mount",		LEV_SLE,
	spell_mount,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(618),	100,	12,
	"",			"!Mount!"
    },

    {
	"insert page",		LEV_SOR,
	spell_insert_page,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(553),	 10,	2,
	"",			"!InsertPage!"
    },

    {
	"purple sorcery",	99,
	spell_null,		TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(554),	 100,	12,
	"",			"The purple spell on you fades away."
    },

    {
	"red sorcery",		99,
	spell_null,		TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(555),	 100,	12,
	"",			"The red spell on you fades away."
    },

    {
	"blue sorcery",		99,
	spell_null,		TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(556),	 100,	12,
	"",			"The blue spell on you fades away."
    },

    {
	"green sorcery",	99,
	spell_null,		TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(557),	 100,	12,
	"",			"The green spell on you fades away."
    },

    {
	"yellow sorcery",	99,
	spell_null,		TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(558),	 100,	12,
	"",			"The yellow spell on you fades away."
    },

    {
	"black sorcery",	99,
	spell_null,		TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(96),	 100,	12,
	"",			"The black spell on you fades away."
    },

    {
	"chaos blast",		99,
	spell_chaos_blast,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(559),	20,	12,
	"chaos blast",		"!Chaos Blast!"
    },

    {
	"web",			99,
	spell_web,		TAR_CHAR_OFFENSIVE,	POS_STANDING,
	&gsn_web,		SLOT(561),	 100,	12,
	"",			"The web surrounding you breaks away."
    },

    {
	"remove page",		LEV_SOR,
	spell_remove_page,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(564),	 10,	2,
	"",			"!RemovePage!"
    },

    {
	"find familiar",	LEV_SOR,
	spell_find_familiar,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(565),	100,	2,
	"",			""
    },

    {
	"improve",		LEV_SLE,
	spell_improve,		TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(566),	1500,	2,
	"",			"!Improve!"
    },

    {
	"fear",			99,
	spell_fear,		TAR_CHAR_OFFENSIVE,	POS_STANDING,
	&gsn_fear,		SLOT(567),	 5,	12,
	"",			"You no longer feel so scared."
    },

    {
	"weakness",		99,
	spell_null,		TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(573),	100,	12,
	"",			"You feel stronger."
    },

    {
	"slowness",		99,
	spell_null,		TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(574),	100,	12,
	"",			"You feel faster."
    },

    {
	"infirmity",		99,
	spell_null,		TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(575),	100,	12,
	"",			"You feel healthier."
    },

    {
	"tendrils",		99,
	spell_tendrils,		TAR_CHAR_OFFENSIVE,	POS_STANDING,
	&gsn_tendrils,		SLOT(576),	 100,	12,
	"tendrils of darkness",	"The tendrils of darkness surrounding you fall away."
    },

    {
	"rotting flesh",	99,
	spell_null,	        TAR_CHAR_OFFENSIVE,	POS_STANDING,
	&gsn_rot,		SLOT(577),	10,	12,
	"flesh rot",		"Your flesh stops rotting."
    },

    {
	"enchant armor",	LEV_SLE,
	spell_enchant_armor,	TAR_OBJ_INV,		POS_STANDING,
	NULL, 			SLOT(94), 	50,	2,
	"",			"!Enchant Armor!"
    },

    {
	"whelpsbody",		99,
	spell_null,		TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(0),	 0,	24,
	"",			"Your body returns to normal."
    },

    {
	"atoningwind",		99,
	spell_null,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_atoningwind,	SLOT(0),	0,	24,
	"Atoning Wind",		"The howling winds around you calms down."
    },

    {
	"spiralmoon",		99,
	spell_null,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_spiralmoon,	SLOT(0),	0,	24,
	"Spiral Moon",		"The dark energy affecting you subsides."
    },

    {
	"slow",			99,
	spell_null,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_slow,		SLOT(0),	0,	24,
	"Slow",			"You speed back up."
    },

    {
	"liquifyearth",		99,
	spell_null,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_liquifyearth,	SLOT(0),	0,	24,
	"Liquify Earth",	"The molasses-like earth crumbles away."
    },

    {    "friction curse",        99,
         spell_null,          TAR_CHAR_OFFENSIVE,      POS_FIGHTING,
         &gsn_friction_curse,     SLOT( 0),        0,      24,
         "Friction Curse",        "Your motions are no longer being transfered into heat."
    },

    {    "brittle bones",        99,
         spell_null,          TAR_CHAR_OFFENSIVE,      POS_FIGHTING,
         &gsn_brittle_bones,     SLOT( 0),        0,      24,
         "brittle bones",        "Your bones return to their normal strength."
    },

    {    "alter weight",        99,
         spell_null,          TAR_CHAR_OFFENSIVE,      POS_FIGHTING,
         &gsn_alter_weight,     SLOT( 0),        0,      24,
         "Alter Weight",        "Gravity is no longer increased around you."
    },

    {    "mage invis",        99,
         spell_null,          TAR_IGNORE,      POS_STANDING,
         &gsn_mage_invis,     SLOT( 0),        0,      24,
         "mage invis",        "You no longer block your image from other's minds."
    },

    {    "blood potency",     99,
         spell_null,          TAR_CHAR_SELF,          POS_STANDING,
         &gsn_potency,         SLOT(0),               0, 12,
         "blood potency",     "Your concentrated blood returns to normal."
    },

};
