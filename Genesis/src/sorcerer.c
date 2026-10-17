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
 * Local Functions.
 */
void	adv_spell_damage	args( ( CHAR_DATA *ch, OBJ_DATA *book, OBJ_DATA *page, char *argument, int lastcolor ) );
void	adv_spell_affect	args( ( CHAR_DATA *ch, OBJ_DATA *book, OBJ_DATA *page, char *argument, int lastcolor ) );
void	adv_spell_action	args( ( CHAR_DATA *ch, OBJ_DATA *book, OBJ_DATA *page, char *argument, int lastcolor ) );


OBJ_DATA *get_page( OBJ_DATA *book, long page_num )
{
    OBJ_DATA *page;
    OBJ_DATA *page_next;

    if (page_num < 1) return NULL;
    for (page = book->contains; page != NULL; page = page_next)
    {
	page_next = page->next_content;
	if (page->value[0] == page_num) return page;
    }
    return NULL;
}

void show_page( CHAR_DATA *ch, OBJ_DATA *book, long pnum, bool pagefalse )
{
    OBJ_DATA *page;
    OBJ_DATA *page_next;
    char buf[MSL];
    bool found = FALSE;

    for (page = book->contains; page != NULL; page = page_next)
    {
	page_next = page->next_content;
	if (page->value[0] == pnum)
	{
	    found = TRUE;
	    if (!str_cmp(page->victpoweruse, "(null)")) sprintf(buf, "Untitled page.\n\r");
	    else sprintf(buf, "%s.\n\r", page->victpoweruse);
	    buf[0] = UPPER(buf[0]);
	    stc(buf, ch);
	    if (!pagefalse)
	    {
if (page->chpoweruse == NULL || page->chpoweruse[0] == '\0' || !str_cmp(page->chpoweruse, "(null)"))
		{
		    stc("This page is blank.\n\r", ch);
		    return;
		}
		stc("--------------------------------------------------------------------------------\n\r", ch);
		stc(page->chpoweruse, ch);
		stc("\n\r--------------------------------------------------------------------------------\n\r", ch);
	    }
	}
    }
    if (!found) stc("This page has been torn out.\n\r", ch);
    return;
}


void do_colors( CHAR_DATA *ch, char *argument )
{
    char buf[MSL];
    int color, cost;
    if (!IS_SORCERER(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (argument[0] == '\0')
    {
	sprintf(buf, "Purple: %li, Red: %li, Blue: %li, Green: %li, Yellow: %li, Black: %li.\n\r",
	 get_color(ch, PURPLE_MAGIC), get_color(ch, RED_MAGIC), get_color(ch, BLUE_MAGIC),
	 get_color(ch, GREEN_MAGIC), get_color(ch, YELLOW_MAGIC), get_color(ch, BLACK_MAGIC));
	stc(buf, ch);
	stc("To improve type \"colors <color>\".\n\r", ch);
	return;
    }

    if (!str_cmp(argument, "purple")) color = PURPLE_MAGIC;
    else if (!str_cmp(argument, "red")) color = RED_MAGIC;
    else if (!str_cmp(argument, "blue")) color = BLUE_MAGIC;
    else if (!str_cmp(argument, "green")) color = GREEN_MAGIC;
    else if (!str_cmp(argument, "yellow")) color = YELLOW_MAGIC;
    else if (!str_cmp(argument, "black")) color = BLACK_MAGIC;
    else
    {
	do_colors(ch, "");
	return;
    }

    if (get_color(ch, color) >= 5)
    {
	sprintf(buf, "You already have the maximum %s magic.\n\r", argument);
	stc(buf, ch);
	return;
    }

    cost = 10 * (get_color(ch, color) + 1);
    if (ch->primal < cost)
    {
	sprintf(buf, "You need %i primal to improve your %s magic.\n\r", cost, argument);
	stc(buf, ch);
	return;
    }

    ch->primal -= cost;
    ch->pcdata->colors[color]++;
    sprintf(buf, "You use %i primal to improve your %s magic!\n\r", cost, argument);
    stc(buf, ch);
    return;
}


void do_sorccreate( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    char arg[MIL];
    char buf[MIL];
    long itemtype;

    one_argument(argument, arg);

    if (!IS_SORCERER(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (arg[0] == '\0')
    {
	stc("Item can be one of: Book, Page or Pen.\n\r", ch);
	return;
    }
    else if (!str_cmp(arg, "book")) itemtype = ITEM_BOOK;
    else if (!str_cmp(arg, "page")) itemtype = ITEM_PAGE;
    else if (!str_cmp(arg, "pen")) itemtype = ITEM_PEN;
    else
    {
	do_sorccreate(ch, "");
	return;
    }

    if (itemtype == ITEM_PEN || itemtype == ITEM_BOOK)
    {
	if (ch->primal < 10)
	{
	    stc("Creating a pen or book requires 10 primal.\n\r", ch);
	    return;
	}
	ch->primal -= 10;
    }

    obj = create_object(get_obj_index(OBJ_VNUM_PROTOPLASM), 0);
    obj->item_type = itemtype;
    if (itemtype == ITEM_PEN || itemtype == ITEM_BOOK) obj->wear_flags = ITEM_TAKE + ITEM_WEAR_HOLD;

    sprintf(buf, "%s", arg);
    free_string(obj->name);
    obj->name = str_dup(buf);

    sprintf(buf, "a %s", arg);
    free_string(obj->short_descr);
    obj->short_descr = str_dup(buf);

    sprintf(buf, "A %s lies here.", arg);
    free_string(obj->description);
    obj->description = str_dup(buf);

    free_string(obj->questmaker);
    obj->questmaker = str_dup(ch->name);

    obj_to_char(obj, ch);
    act("$p suddenly appears in your hands.", ch, obj, NULL, TO_CHAR);
    act("$p suddenly appears in $n's hands.", ch, obj, NULL, TO_ROOM);
    return;
}


void do_vision( CHAR_DATA *ch, char *argument )
{
    if (!IS_SORCERER(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (!IS_SET(ch->act, PLR_HOLYLIGHT))
    {
	SET_BIT(ch->act, PLR_HOLYLIGHT);
	SET_BIT(ch->affected_by, AFF_SHADOWSIGHT);
	SET_BIT(ch->vampaff, VAM_NIGHTSIGHT);
	stc("You manipulate magical fields about yourself to enhance your vision.\n\r", ch);
    }
    else
    {
	REMOVE_BIT(ch->act, PLR_HOLYLIGHT);
	REMOVE_BIT(ch->affected_by, AFF_SHADOWSIGHT);
	REMOVE_BIT(ch->vampaff, VAM_NIGHTSIGHT);
	stc("You allow your magically enhanced vision to fade.\n\r", ch);
    }

    return;
}


void do_enchant( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA *paf;
    OBJ_DATA *obj;
    char arg[MIL];

    one_argument(argument, arg);

    if (!IS_SORCERER(ch))
    {
	stc("Huh?\n\r", ch);
	return;
    }

    if (arg[0] == '\0')
    {
	stc("Which weapon do you wish to enchant?\n\r", ch);
	return;
    }

    if ((obj = get_obj_carry(ch, arg)) == NULL)
    {
	stc("You are not carrying that object.\n\r", ch);
	return;
    }

    if (obj->item_type != ITEM_WEAPON && obj->item_type != ITEM_ARMOR)
    {
	act("$p is not a weapon or armor.", ch, obj, NULL, TO_CHAR);
	return;
    }

    if (IS_SET(obj->quest, QUEST_SORCENCHANT) || IS_SET(obj->quest, QUEST_ARTIFACT))
    {
	act("$p has already been enchanted.", ch, obj, NULL, TO_CHAR);
	return;
    }

    act("You weave enchantments into $p.", ch, obj, NULL, TO_CHAR);
    act("$p glows blue as $n casts a spell of enchantment.", ch, obj, NULL, TO_ROOM);
    if (affect_free == NULL)
	paf		= alloc_perm(sizeof(*paf));
    else
    {
	paf		= affect_free;
	affect_free	= affect_free->next;
    }
    paf->type		= skill_lookup("enchant weapon");
    paf->duration	= -1;
    paf->location	= APPLY_HITROLL;
    paf->modifier	= 5;
    paf->bitvector	= 0;
    paf->next		= obj->affected;
    obj->affected	= paf;

    if (affect_free == NULL)
	paf		= alloc_perm(sizeof(*paf));
    else
    {
	paf		= affect_free;
	affect_free	= affect_free->next;
    }
    paf->type		= skill_lookup("enchant weapon");
    paf->duration	= -1;
    paf->location	= APPLY_DAMROLL;
    paf->modifier	= 5;
    paf->bitvector	= 0;
    paf->next		= obj->affected;
    obj->affected	= paf;

    SET_BIT(obj->quest, QUEST_SORCENCHANT);
    return;
}


void do_write( CHAR_DATA *ch, char *argument )
{
    char arg1[MIL];
    char arg2[MIL];
    char arg3[MIL];
    char buf[MSL];
    OBJ_DATA *obj;

    smash_tilde(argument);
    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    strcpy(arg3, argument);

    if (IS_NPC(ch)) return;
    if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
    {
	stc("Syntax: Write <page> <title/line> <text>.\n\r", ch);
	return;
    }

    if ((obj = get_eq_char(ch, WEAR_HOLD)) == NULL || obj->item_type != ITEM_PEN)
    {
	stc("You are not holding a pen.\n\r", ch);
	return;
    }

    if ((obj = get_obj_carry(ch, arg1)) == NULL || (obj->item_type != ITEM_PAGE && obj->item_type != ITEM_BOOK))
    {
	stc("You may only write on books and pages.\n\r", ch);
	return;
    }

    if (!str_cmp(arg2, "title"))
    {
	free_string(obj->victpoweruse);
	obj->victpoweruse = str_dup(arg3);
	stc("Ok.\n\r", ch);
	act("$n writes something on $p.", ch, obj, NULL, TO_ROOM);
	return;
    }

    if (str_cmp(arg2, "line"))
    {
	stc("You can write a TITLE or a LINE.\n\r", ch);
	return;
    }
    else if (obj->item_type == ITEM_BOOK)
    {
	stc("You can only write a title on the book.\n\r", ch);
	return;
    }

    if (obj->chpoweruse != NULL) strcpy(buf, obj->chpoweruse);
    else return;

    if (!str_cmp(buf, "(null)"))
    {
	arg3[0] = UPPER(arg3[0]);
	free_string(obj->chpoweruse);
	obj->chpoweruse = str_dup(arg3);
	stc("Ok.\n\r", ch);
	act("$n writes something on $p.", ch, obj, NULL, TO_ROOM);
	if (!IS_SORCERER(ch) && !IS_IMM(ch))
	    SET_BIT(obj->spectype, ADV_FAILED);
	else if (!str_cmp(arg3, "start.damage.spell") && obj->spectype == 0)
	    {SET_BIT(obj->spectype, ADV_STARTED); SET_BIT(obj->spectype, ADV_DAMAGE);}
	else if (!str_cmp(arg3, "start.affect.spell") && obj->spectype == 0)
	    {SET_BIT(obj->spectype, ADV_STARTED); SET_BIT(obj->spectype, ADV_AFFECT);}
	else if (!str_cmp(arg3, "start.action.spell") && obj->spectype == 0)
	    {SET_BIT(obj->spectype, ADV_STARTED); SET_BIT(obj->spectype, ADV_ACTION);}
	else SET_BIT(obj->spectype, ADV_FAILED);
	return;
    }

    if (obj->chpoweruse != NULL && buf[0] != '\0')
    {
	if (strlen(buf) + strlen(arg3) >= MAX_STRING_LENGTH - 4)
	{
	    stc("Line too long.\n\r", ch);
	    return;
	}
	else
	{
	    free_string(obj->chpoweruse);
	    strcat(buf, "\n\r");
	    strcat(buf, arg3);
	    obj->chpoweruse = str_dup(buf);

	    argument = one_argument(argument, arg1);
	    strcpy(arg2, argument);

	    if (!IS_SORCERER(ch) && !IS_IMM(ch) && !IS_SET(obj->spectype, ADV_FINISHED))
		SET_BIT(obj->spectype, ADV_FAILED);
	    else if (!str_cmp(arg1, "start.damage.spell") && obj->spectype == 0)
		{SET_BIT(obj->spectype, ADV_STARTED); SET_BIT(obj->spectype, ADV_DAMAGE);}
	    else if (!str_cmp(arg1, "start.affect.spell") && obj->spectype == 0)
		{SET_BIT(obj->spectype, ADV_STARTED); SET_BIT(obj->spectype, ADV_AFFECT);}
	    else if (!str_cmp(arg1, "start.action.spell") && obj->spectype == 0)
		{SET_BIT(obj->spectype, ADV_STARTED); SET_BIT(obj->spectype, ADV_ACTION);}
	    else if (!IS_SET(obj->spectype, ADV_FINISHED))
	    {
		if (!IS_SET(obj->spectype, ADV_STARTED)) SET_BIT(obj->spectype, ADV_FAILED);
		else if (!str_cmp(arg1, "end.blue"))
		{
		    SET_BIT(obj->spectype, ADV_FINISHED);
		    obj->toughness = BLUE_MAGIC;
		}
		else if (!str_cmp(arg1, "end.red"))
		{
		    SET_BIT(obj->spectype, ADV_FINISHED);
		    obj->toughness = RED_MAGIC;
		}
		else if (!str_cmp(arg1, "end.green"))
		{
		    SET_BIT(obj->spectype, ADV_FINISHED);
		    obj->toughness = GREEN_MAGIC;
		}
		else if (!str_cmp(arg1, "end.yellow"))
		{
		    SET_BIT(obj->spectype, ADV_FINISHED);
		    obj->toughness = YELLOW_MAGIC;
		}
	        else if (!str_cmp(arg1, "end.purple"))
	        {
		    SET_BIT(obj->spectype, ADV_FINISHED);
		    obj->toughness = PURPLE_MAGIC;
		}
		else if (!str_cmp(arg1, "end.black"))
		{
		    SET_BIT(obj->spectype, ADV_FINISHED);
		    obj->toughness = BLACK_MAGIC;
		}
		else if (!str_cmp(arg1, "end.random"))
		{
		    SET_BIT(obj->spectype, ADV_FINISHED);
		    obj->toughness = RANDOM_MAGIC;
		}
	 	else if (!str_cmp(arg1, "next.page") && !IS_SET(obj->spectype, ADV_NEXT_PAGE))
		    SET_BIT(obj->spectype, ADV_NEXT_PAGE);
		else if (!str_cmp(arg1, "reversed") && !IS_SET(obj->spectype, ADV_REVERSED) && IS_SET(obj->spectype, ADV_AFFECT))
		    SET_BIT(obj->spectype, ADV_REVERSED);
		else if (!str_cmp(arg1, "apply:") && IS_SET(obj->spectype, ADV_AFFECT))
		{
		    if (arg2[0] == '\0') SET_BIT(obj->spectype, ADV_FAILED);
		    else if (!str_cmp(arg2, "hp") && !IS_SET(obj->value[1], ADV_HIT))
			SET_BIT(obj->value[1], ADV_HIT);
		    else if (!str_cmp(arg2, "mana") && !IS_SET(obj->value[1], ADV_MANA))
			SET_BIT(obj->value[1], ADV_MANA);
		    else if (!str_cmp(arg2, "armor") && !IS_SET(obj->value[1], ADV_AC))
			SET_BIT(obj->value[1], ADV_AC);
		    else if (!str_cmp(arg2, "hitroll") && !IS_SET(obj->value[1], ADV_HITROLL))
			SET_BIT(obj->value[1], ADV_HITROLL);
		    else if (!str_cmp(arg2, "damroll") && !IS_SET(obj->value[1], ADV_DAMROLL))
			SET_BIT(obj->value[1], ADV_DAMROLL);
		    else if (!str_cmp(arg2, "save") && !IS_SET(obj->value[1], ADV_SAVING_SPELL))
			SET_BIT(obj->value[1], ADV_SAVING_SPELL);
		    else {SET_BIT(obj->spectype, ADV_FAILED); return;}
		}
		else if (!str_cmp(arg1, "affect:") && IS_SET(obj->spectype, ADV_AFFECT))
		{
		    if (arg2[0] == '\0') SET_BIT(obj->spectype, ADV_FAILED);
		    else if (!str_cmp(arg2, "blind") && !IS_SET(obj->value[3], AFF_BLIND))
			SET_BIT(obj->value[3], AFF_BLIND);
		    else if (!str_cmp(arg2, "umbra") && !IS_SET(obj->value[3], AFF_SHADOWPLANE))
			SET_BIT(obj->value[3], AFF_SHADOWPLANE);
		    else if (!str_cmp(arg2, "sanct") && !IS_SET(obj->value[3], AFF_SANCTUARY))
			SET_BIT(obj->value[3], AFF_SANCTUARY);
		    else if (!str_cmp(arg2, "faerie.fire") && !IS_SET(obj->value[3], AFF_FAERIE_FIRE))
			SET_BIT(obj->value[3], AFF_FAERIE_FIRE);
		    else if (!str_cmp(arg2, "curse") && !IS_SET(obj->value[3], AFF_CURSE))
			SET_BIT(obj->value[3], AFF_CURSE);
		    else if (!str_cmp(arg2, "flaming") && !IS_SET(obj->value[3], AFF_FLAMING))
			SET_BIT(obj->value[3], AFF_FLAMING);
		    else if (!str_cmp(arg2, "poison") && !IS_SET(obj->value[3], AFF_POISON))
			SET_BIT(obj->value[3], AFF_POISON);
		    else if (!str_cmp(arg2, "ethereal") && !IS_SET(obj->value[3], AFF_ETHEREAL))
			SET_BIT(obj->value[3], AFF_ETHEREAL);
		    else if (!str_cmp(arg2, "sleep") && !IS_SET(obj->value[3], AFF_SLEEP))
			SET_BIT(obj->value[3], AFF_SLEEP);
		    else if (!str_cmp(arg2, "web") && !IS_SET(obj->value[3], AFF_WEBBED))
			SET_BIT(obj->value[3], AFF_WEBBED);
		    else {SET_BIT(obj->spectype, ADV_FAILED); return;}
		}
		else if (!str_cmp(arg1, "duration:") && IS_SET(obj->spectype, ADV_AFFECT) && obj->level == 0)
		{
		    if (arg2[0] == '\0' || !is_number(arg2) || atoi(arg2) < 0 || atoi(arg2) > 50)
			SET_BIT(obj->spectype, ADV_FAILED);
		    else obj->level = atoi(arg2);
		}
		else if (!str_cmp(arg1, "message.1:") && !IS_SET(obj->spectype, ADV_MESSAGE_1))
		{
		    if (arg2[0] == '\0') SET_BIT(obj->spectype, ADV_FAILED);
		    else
		    {
			SET_BIT(obj->spectype, ADV_MESSAGE_1);
			free_string(obj->chpoweroff);
			obj->chpoweroff = str_dup(arg2);
		    }
		}
		else if (!str_cmp(arg1, "message.2:") && !IS_SET(obj->spectype, ADV_MESSAGE_2))
		{
		    if (arg2[0] == '\0') SET_BIT(obj->spectype, ADV_FAILED);
		    else
		    {
			SET_BIT(obj->spectype, ADV_MESSAGE_2);
			free_string(obj->victpoweron);
			obj->victpoweron = str_dup(arg2);
		    }
		}
		else if (!str_cmp(arg1, "message.3:") && !IS_SET(obj->spectype, ADV_MESSAGE_3))
		{
		    if (arg2[0] == '\0') SET_BIT(obj->spectype, ADV_FAILED);
		    else
		    {
			SET_BIT(obj->spectype, ADV_MESSAGE_3);
			free_string(obj->victpoweroff);
			obj->victpoweroff = str_dup(arg2);
		    }
		}
		else SET_BIT(obj->spectype, ADV_FAILED);
	    }
	}
    }
    else return;
    act("$n writes something on $p.", ch, obj, NULL, TO_ROOM);
    stc("Ok.\n\r", ch);
    return;
}


void do_chant( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *book;
    OBJ_DATA *page;
    char arg[MIL];
    extern long chainspell;

    chainspell = 0;

    one_argument(argument, arg);

    if (ch->in_room != NULL && IS_SET(ch->in_room->added_flags, ROOM2_SILENCE) && !IS_BUI(ch))
    {
	stc("Your words make no sound!\n\r", ch);
	return;
    }

    if ((book = get_eq_char(ch, WEAR_HOLD)) == NULL || book->item_type != ITEM_BOOK)
    {
	stc("First you must hold a spellbook.\n\r", ch);
	return;
    }

    if (IS_SET(book->value[1], CONT_CLOSED))
    {
	stc("First you better open the book.\n\r", ch);
	return;
    }

    if (book->value[2] < 1)
    {
	stc("There are no spells on the index page!\n\r", ch);
	return;
    }

    if ((page = get_page(book, book->value[2])) == NULL)
    {
	stc("The current page seems to have been torn out!\n\r", ch);
	return;
    }

    act("You chant the arcane words from $p.", ch, book, NULL, TO_CHAR);
    act("$n chants some arcane words from $p.", ch, book, NULL, TO_ROOM);

    if (IS_SET(page->spectype, ADV_FAILED) || !IS_SET(page->spectype, ADV_FINISHED))
	stc("Spell Failure: Bad first page.\n\r", ch);
    else if (IS_SET(page->spectype, ADV_DAMAGE)) adv_spell_damage(ch, book, page, argument, -1);
    else if (IS_SET(page->spectype, ADV_AFFECT)) adv_spell_affect(ch, book, page, argument, -1);
    else if (IS_SET(page->spectype, ADV_ACTION)) adv_spell_action(ch, book, page, argument, -1);
    else stc("Spell Failure: Bad first page.\n\r", ch);

    return;
}

void adv_spell_damage( CHAR_DATA *ch, OBJ_DATA *book, OBJ_DATA *page, char *argument, int lastcolor)
{
    OBJ_DATA *page_next;
    char arg[MIL];
    char buf[MSL];
    int cost, dam, color = page->toughness;
    bool aggy = FALSE, area = FALSE, heal = FALSE, spec = FALSE;
    extern long chainspell;

    if (++chainspell > 10) return;

    one_argument(argument, arg);

    if (color == RANDOM_MAGIC) color = number_range(0, 5);

    if (color == PURPLE_MAGIC)
    {
	dam = number_range(100, 140);
	area = TRUE;
	cost = 800;
    }
    else if (color == RED_MAGIC)
    {
	dam = number_range(140, 180);
	cost = 600;
    }
    else if (color == BLUE_MAGIC)
    {
	dam = number_range(100, 140);
	heal = TRUE;
	cost = 400;
    }
    else if (color == GREEN_MAGIC)
    {
	dam = number_range(80, 120);
	spec = TRUE;
	heal = TRUE;
	cost = 500;
    }
    else if (color == YELLOW_MAGIC)
    {
	dam = number_range(40, 200);
	cost = 200;
    }
    else if (color == BLACK_MAGIC)
    {
	dam = number_range(80, 120);
	aggy = TRUE;
	cost = 700;
    }
    else
    {
	stc("Spell Failure: Undefined color.\n\r", ch);
	return;
    }

    dam *= get_color(ch, color);

    if (lastcolor == color)
	dam += 10 * get_color(ch, color);
    else if (lastcolor == PURPLE_MAGIC)
	area = TRUE;
    else if (lastcolor == RED_MAGIC)
	dam += 20 * get_color(ch, RED_MAGIC);
    else if (lastcolor == BLUE_MAGIC)
    {
	if (heal) dam += 30 * get_color(ch, BLUE_MAGIC);
	else heal = TRUE;
    }
    else if (lastcolor == GREEN_MAGIC)
	spec = TRUE;
    else if (lastcolor == YELLOW_MAGIC)
    {
	cost -= 200;
	dam += number_range(10, -10) * get_color(ch, YELLOW_MAGIC);
    }
    else if (lastcolor == BLACK_MAGIC)
    {
	aggy = TRUE;
	dam -= 20 * get_color(ch, BLACK_MAGIC);
    }

    if ((strlen(page->victpoweron) < 1 || !str_cmp(page->victpoweron, "(null)"))
     || (strlen(page->victpoweroff) < 1 || !str_cmp(page->victpoweroff, "(null)"))
     || (strlen(page->chpoweroff) < 1 || !str_cmp(page->chpoweroff, "(null)")))
    {
	stc("Spell Failure: Missing message.\n\r", ch);
	return;
    }

    if (ch->mana < cost)
    {
	stc("Your mana reserves are exhausted.\n\r", ch);
	return;
    }

    if (!area)
    {
	CHAR_DATA *victim;

	if (spec)
	{
	    if (heal) victim = ch;
	    else victim = ch->fighting;
	}
	else victim = get_char_room(ch, arg);

	if (victim == NULL)
	{
	    stc("Your target is not present.\n\r", ch);
	    return;
	}

	act2(page->victpoweron, ch, NULL, victim, TO_CHAR);
	act2(page->victpoweroff, ch, NULL, victim, TO_ROOM);

	if (heal)
	{
	    victim->hit += dam;
	    if (victim->hit > victim->max_hit) victim->hit = victim->max_hit;
	}
	else
	{
	    if (!hurt_fighting(ch, victim)) return;
	    if (!IS_NPC(victim) && victim->pcdata->atm > 0)
		dam *= (100 - victim->pcdata->atm) * .01;
	    if (!IS_NPC(victim) && victim->pcdata->atm >= 100)
		act( "$n's spell is unable to penetrate your antimagic shield.", ch, NULL, victim, TO_VICT);
	    else
	    {
		sprintf(buf, "Your %s strikes $N incredibly hard! [#g%i#n]", page->chpoweroff, dam);
		act2(buf, ch, NULL, victim, TO_CHAR);
		sprintf(buf, "$n's %s strikes $N incredibly hard! [#0%i#n]", page->chpoweroff, dam);
		act2(buf, ch, NULL, victim, TO_NOTVICT);
		sprintf(buf, "$n's %s strikes you incredibly hard! [#r%i#n]", page->chpoweroff, dam);
		act2(buf, ch, NULL, victim, TO_VICT);
		hurt_person(ch, victim, dam);
	    }
	}
    }
    else
    {
	CHAR_DATA *vch = ch;
	CHAR_DATA *vch_next;

	act2(page->victpoweron, ch, NULL, vch, TO_CHAR);
	act2(page->victpoweroff, ch, NULL, vch, TO_ROOM);

	if (spec) dam *= 3;

	for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
	{
	    vch_next = vch->next_in_room;
	    if (vch->in_room != ch->in_room) continue;
	    if (IS_BUI(vch)) continue;
	    if (heal && spec && !is_same_group(ch, vch)) continue;
	    if (!heal && is_same_group(ch, vch)) continue;
	    if (!heal && !IS_NPC(vch))
	    {
		if (spec) continue;
		if (!CAN_PK(vch)) continue;
	    }
	    if (!heal && is_safe(ch, vch)) continue;

	    if (heal)
	    {
		vch->hit += dam;
		if (vch->hit > vch->max_hit) vch->hit = vch->max_hit;
	    }
	    else
	    {
		if (!hurt_fighting(ch, vch)) continue;
		if (!IS_NPC(vch) && vch->pcdata->atm > 0)
		    dam *= (100 - vch->pcdata->atm) * .01;
		if (!IS_NPC(vch) && vch->pcdata->atm >= 100)
		    act("$n's spell is unable to penetrate your antimagic shield.", vch, NULL, NULL, TO_VICT);
		else
		{
		    sprintf(buf, "Your %s strikes $N incredibly hard! [#g%i#n]", page->chpoweroff, dam);
		    act2(buf, ch, NULL, vch, TO_CHAR);
		    sprintf(buf, "$n's %s strikes $N incredibly hard! [#0%i#n]", page->chpoweroff, dam);
		    act2(buf, ch, NULL, vch, TO_NOTVICT);
		    sprintf(buf, "$n's %s strikes you incredibly hard! [#r%i#n]", page->chpoweroff, dam);
		    act2(buf, ch, NULL, vch, TO_VICT);
		    hurt_person(ch, vch, dam);
		}
	    }
	}
    }

    WAIT_STATE(ch, 15);
    ch->mana -= cost;

    if (IS_SET(page->spectype, ADV_NEXT_PAGE))
    {
	if (page->specpower < page->value[0])
	{
	    stc("Spell Failure: Pages out of order.\n\r", ch);
	    return;
	}
	if ((page_next = get_page(book, page->specpower)) == NULL)
	{
	    stc("Spell Failure: No next page.\n\r", ch);
	    return;
	}

	if (IS_SET(page_next->spectype, ADV_DAMAGE)) adv_spell_damage(ch, book, page_next, argument, color);
	else if (IS_SET(page_next->spectype, ADV_AFFECT)) adv_spell_affect(ch, book, page_next, argument, color);
	else if (IS_SET(page_next->spectype, ADV_ACTION)) adv_spell_action(ch, book, page_next, argument, color);
	else stc("Spell Failure: Bad next page.\n\r", ch);
    }
    return;
}

void adv_spell_affect( CHAR_DATA *ch, OBJ_DATA *book, OBJ_DATA *page, char *argument, int lastcolor)
{
    CHAR_DATA *victim = ch;
    OBJ_DATA *page_next;
    char arg[MSL];
    long bonuses, sn, color = page->toughness, duration = page->level, cost = 1500 + 30*duration;
    long affect_bit = page->value[3], apply_bit = page->value[1];
    bool any_affects = FALSE;
    extern long chainspell;

    if (++chainspell > 10) return;

    one_argument(argument, arg);

    if ((strlen(page->victpoweron) < 1 || !str_cmp(page->victpoweron, "(null)"))
     || (strlen(page->victpoweroff) < 1 || !str_cmp(page->victpoweroff, "(null)"))
     || (strlen(page->chpoweroff) < 1 || !str_cmp(page->chpoweroff, "(null)")))
    {
	stc("Spell Failure: Missing message.\n\r", ch);
	return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL || victim->in_room == NULL || victim->in_room->area != ch->in_room->area)
    {
	stc("Your target is not present.\n\r", ch);
	return;
    }

    if (color == RANDOM_MAGIC)
    {
	cost *= 1.5;
	color = number_range(0, 5);
    }

    if (color == PURPLE_MAGIC)
    {
	if ((sn = skill_lookup("purple sorcery")) < 0)
	{
	    stc("Spell Failure: Purple doesn't exist...Inform Akuma.\n\r", ch);
	    return;
	}
    }
    else if (color == RED_MAGIC)
    {
	if ((sn = skill_lookup("red sorcery")) < 0)
	{
	    stc("Spell Failure: Red doesn't exist...Inform Akuma.\n\r", ch);
	    return;
	}
    }
    else if (color == BLUE_MAGIC)
    {
	if ((sn = skill_lookup("blue sorcery")) < 0)
	{
	    stc("Spell Failure: Blue doesn't exist...Inform Akuma.\n\r", ch);
	    return;
	}
    }
    else if (color == GREEN_MAGIC)
    {
	if ((sn = skill_lookup("green sorcery")) < 0)
	{
	    stc("Spell Failure: Green doesn't exist...Inform Akuma.\n\r", ch);
	    return;
	}
    }
    else if (color == YELLOW_MAGIC)
    {
	if ((sn = skill_lookup("yellow sorcery")) < 0)
	{
	    stc("Spell Failure: Yellow doesn't exist...Inform Akuma.\n\r", ch);
	    return;
	}
    }
    else if (color == BLACK_MAGIC)
    {
	if ((sn = skill_lookup("black sorcery")) < 0)
	{
	    stc("Spell Failure: Black doesn't exist...Inform Akuma.\n\r", ch);
	    return;
	}
    }
    else
    {
	stc("Spell Failure: Bad end.<color>.\n\r", ch);
	return;
    }

    bonuses = get_color(ch, color);

    if (ch->mana < cost)
    {
	stc("You have insufficient mana to chant this spell.\n\r", ch);
	return;
    }

    if (IS_SET(page->spectype, ADV_REVERSED)) bonuses = 0 - bonuses;

    if (!IS_NPC(victim) && number_range(1, 150) <= victim->pcdata->atm)
    {
	stc("Your spell fizzles.\n\r", ch);
	return;
    }

    if (is_affected(victim, sn))
    {
	stc("They are already affected by a spell of that colour.\n\r", ch);
	return;
    }

    if (apply_bit == 0)
    {
	enhance_stat(sn, duration, ch, victim, APPLY_NONE, bonuses, affect_bit);
	affect_bit = 0;
	any_affects = TRUE;
    }
    if (IS_SET(apply_bit, ADV_HIT))
    {
	enhance_stat(sn, duration, ch, victim, APPLY_HIT, bonuses * 250, affect_bit);
	affect_bit = 0;
	any_affects = TRUE;
    }
    if (IS_SET(apply_bit, ADV_MANA))
    {
	enhance_stat(sn, duration, ch, victim, APPLY_MANA, bonuses * 250, affect_bit);
	affect_bit = 0;
	any_affects = TRUE;
    }
    if (IS_SET(apply_bit, ADV_AC))
    {
	enhance_stat(sn, duration, ch, victim, APPLY_AC, bonuses * 10, affect_bit);
	affect_bit = 0;
	any_affects = TRUE;
    }
    if (IS_SET(apply_bit, ADV_HITROLL))
    {
	enhance_stat(sn, duration, ch, victim, APPLY_HITROLL, bonuses * 10, affect_bit);
	affect_bit = 0;
	any_affects = TRUE;
    }
    if (IS_SET(apply_bit, ADV_DAMROLL))
    {
	enhance_stat(sn, duration, ch, victim, APPLY_DAMROLL, bonuses * 10, affect_bit);
	affect_bit = 0;
	any_affects = TRUE;
    }
    if (IS_SET(apply_bit, ADV_SAVING_SPELL))
    {
	enhance_stat(sn, duration, ch, victim, APPLY_SAVING_SPELL, bonuses * 1, affect_bit);
	affect_bit = 0;
	any_affects = TRUE;
    }
    if (!any_affects)
    {
	stc("Spell Failure: Bad apply.\n\r",ch);
	return;
    }

    act2(page->chpoweroff, ch, NULL, victim, TO_CHAR);
    act2(page->victpoweron, ch, NULL, victim, TO_VICT);
    act2(page->victpoweroff, ch, NULL, victim, TO_NOTVICT);

    WAIT_STATE(ch, 18);
    ch->mana -= cost;

    if (IS_SET(page->spectype, ADV_NEXT_PAGE))
    {
	if (page->specpower < page->value[0])
	{
	    stc("Spell Failure: Pages out of order.\n\r", ch);
	    return;
	}
	if ((page_next = get_page(book, page->specpower)) == NULL)
	{
	    stc("Spell Failure: No next page.\n\r", ch);
	    return;
	}

	if (IS_SET(page_next->spectype, ADV_DAMAGE)) adv_spell_damage(ch, book, page_next, argument, color);
	else if (IS_SET(page_next->spectype, ADV_AFFECT)) adv_spell_affect(ch, book, page_next, argument, color);
	else if (IS_SET(page_next->spectype, ADV_ACTION)) adv_spell_action(ch, book, page_next, argument, color);
	else stc("Spell Failure: Bad next page.\n\r", ch);
    }

    return;
}

void adv_spell_action( CHAR_DATA *ch, OBJ_DATA *book, OBJ_DATA *page, char *argument, int lastcolor)
{
    ROOM_INDEX_DATA *old_room = ch->in_room;
    CHAR_DATA *victim = NULL;
    OBJ_DATA *obj = NULL;
    OBJ_DATA *page_next;
    char arg[MSL];
    char buf[MSL];
    long color = page->toughness, cost = 3000;
    extern long chainspell;

    if (++chainspell > 10) return;

    if (ch->mana < cost)
    {
	stc("You have insufficient mana to chant this spell.\n\r", ch);
	return;
    }

    argument = one_argument(argument, arg);

    if ((strlen(page->chpoweroff) < 1 || !str_cmp(page->chpoweroff, "(null)"))
     || (strlen(page->victpoweron) < 1 || !str_cmp(page->victpoweron, "(null)"))
     || (strlen(page->victpoweroff) < 1 || !str_cmp(page->victpoweroff, "(null)")))
    {
	stc("Spell Failure: Missing message.\n\r", ch);
	return;
    }

    if (ch->position == POS_FIGHTING)
    {
	stc("Combat interrupts your concentration!.\n\r", ch);
	return;
    }

    if (color == RANDOM_MAGIC) color = number_range(0, 5);

    if (color == PURPLE_MAGIC) /* Object Summon */
    {
	if ((obj = get_obj_world(ch, arg)) == NULL)
	{
	    stc("No such object.\n\r", ch);
	    return;
	}

	if (obj->in_room == NULL || obj->carried_by != NULL || IS_SET(obj->quest, QUEST_RELIC)
	 || IS_SET(obj->quest, QUEST_ARTIFACT) || obj->item_type == ITEM_QUEST)
	{
	    stc("You are unable to grasp the object.\n\r", ch);
	    return;
	}

	act2(page->chpoweroff, ch, obj, NULL, TO_CHAR); /* Message.1 - Character */
	act2(page->victpoweron, ch, obj, NULL, TO_ROOM); /* Message.2 - Character's Room */
	ch->in_room = obj->in_room;
	act2(page->victpoweroff, ch, obj, NULL, TO_ROOM); /* Message.3 - Object's Room */
	ch->in_room = old_room;

	obj_from_room(obj);
	obj_to_room(obj, ch->in_room);

	if (obj->questmaker == NULL || strlen(obj->questmaker) > 1)
	{
	    free_string(obj->questmaker);
	    obj->questmaker = str_dup(ch->name);
	}
    }
    else if (color == RED_MAGIC) /* Summoning */
    {
	if ((victim = get_char_world(ch, arg)) == NULL)
	{
	    stc("No such person.\n\r", ch);
	    return;
	}
	if ((!IS_NPC(victim) && !IS_EXTRA(victim, EXTRA_SUMMON))
	 || (IS_NPC(victim) && victim->level > (char_stat(ch, STAT_INT) * 20 + 30))
	 || victim == ch)
	{
	    stc("Targeting failed.\n\r", ch);
	    return;
	}

	ch->in_room = victim->in_room;
	act2(page->chpoweroff, ch, NULL, victim, TO_VICT); /* Message.1 - Victim */
	act2(page->victpoweron, ch, NULL, victim, TO_NOTVICT); /* Message.2 - Victim's Room */
	ch->in_room = old_room;
	act2(page->victpoweroff, ch, NULL, victim, TO_CHAR); /* Message.3 - Char and... */
	act2(page->victpoweroff, ch, NULL, victim, TO_ROOM); /* Message.3 - Char's Room */

	char_from_room(victim);
	char_to_room(victim, ch->in_room);
	do_look(victim, "auto");
	if (IS_NPC(victim)) SET_BIT(victim->act, ACT_NOEXP);
    }
    else if (color == BLUE_MAGIC) /* Traveling */
    {
	if ((victim = get_char_world(ch, arg)) == NULL)
	{
	    stc("No such person.\n\r", ch);
	    return;
	}
	if ((!IS_NPC(victim) && !IS_EXTRA(victim, EXTRA_SUMMON))
	 || (IS_NPC(victim) && victim->level > (char_stat(ch, STAT_INT) * 90 + 10))
	 || victim == ch)
	{
	    stc("Targeting failed.\n\r", ch);
	    return;
	}

	act2(page->chpoweroff, ch, NULL, victim, TO_CHAR); /* Message.1 - Char */
	act2(page->victpoweron, ch, NULL, victim, TO_ROOM); /* Message.2 - Char's Room */
	ch->in_room = victim->in_room;
	act2(page->victpoweroff, ch, NULL, victim, TO_ROOM); /* Message.3 - Victim's Room */
	ch->in_room = old_room;

	char_from_room(ch);
	char_to_room(ch, victim->in_room);
	do_look(ch, "auto");
    }
    else if (color == GREEN_MAGIC) /* Readaura */
    {
	if ((victim = get_char_world(ch, arg)) == NULL)
	{
	    stc("No such person.\n\r", ch);
	    return;
	}
	if (!IS_NPC(victim) && IS_MORE(victim, MORE_SHIELDED) && ch->in_room->area != victim->in_room->area)
	{
	    stc("Targeting failed.\n\r", ch);
	    return;
	}

	ch->in_room = victim->in_room;
	act2(page->chpoweroff, ch, NULL, victim, TO_VICT); /* Message.1 - Victim */
	ch->in_room = old_room;
	act2(page->victpoweron, ch, NULL, victim, TO_CHAR); /* Message.2 - Char */
	act2(page->victpoweroff, ch, NULL, victim, TO_ROOM); /* Message.3 - Char's Room */

	if (IS_NPC(victim)) sprintf(buf, "%s is an NPC.\n\r", victim->short_descr);
	else
	{
	    if (IS_ST(victim)) sprintf(buf, "%s is a Storyteller.\n\r", victim->name);
	    else if (IS_IMP(victim)) sprintf(buf, "%s is an Implementor.\n\r", victim->name);
	    else if (IS_IMM(victim)) sprintf(buf, "%s is an Immortal.\n\r", victim->name);
	    else if (IS_BUI(victim)) sprintf(buf, "%s is a Builder.\n\r", victim->name);
	    else if (IS_AWA(victim)) sprintf(buf, "%s is an Awakened.\n\r", victim->name);
	    else sprintf(buf, "%s is a Sleeper.\n\r", victim->name);
	}
	stc(buf, ch);
	sprintf(buf, "Str: %i.  Dex: %i.  Con: %i.  Int: %i.  Wis: %i.\n\r",
	 char_stat(victim, STAT_STR), char_stat(victim, STAT_DEX), char_stat(victim, STAT_CON),
	 char_stat(victim, STAT_INT), char_stat(victim, STAT_WIS));
	stc(buf, ch);
	sprintf(buf, "Hp: %li/%li, Mana: %li/%li.\n\r", victim->hit, victim->max_hit, victim->mana, victim->max_mana);
	stc(buf, ch);
	sprintf(buf, "Hitroll: %li, Damroll: %li, AC: %li.\n\r", char_hitroll(victim), char_damroll(victim), char_ac(victim));
	stc(buf, ch);
    }
    else if (color == YELLOW_MAGIC) /* Object Travel */
    {
	if ((obj = get_obj_world(ch, arg)) == NULL)
	{
	    stc("No such object.\n\r", ch);
	    return;
	}

	if (obj->in_room == NULL || obj->carried_by != NULL || IS_SET(obj->quest, QUEST_RELIC)
	 || IS_SET(obj->quest, QUEST_ARTIFACT) || obj->item_type == ITEM_QUEST)
	{
	    stc("You are unable to grasp the object.\n\r", ch);
	    return;
	}

	act2(page->chpoweroff, ch, obj, NULL, TO_CHAR); /* Message.1 - Character */
	act2(page->victpoweron, ch, obj, NULL, TO_ROOM); /* Message.2 - Character's Room */
	ch->in_room = obj->in_room;
	act2(page->victpoweroff, ch, obj, NULL, TO_ROOM); /* Message.3 - Object's Room */
	ch->in_room = old_room;

	char_from_room(ch);
	char_to_room(ch, obj->in_room);
    }
    else if (color == BLACK_MAGIC) /* Scry */
    {
	if ((victim = get_char_world(ch, arg)) == NULL)
	{
	    stc("No such person.\n\r", ch);
	    return;
	}
	if (!IS_NPC(victim) && IS_MORE(victim, MORE_SHIELDED) && ch->in_room->area != victim->in_room->area)
	{
	    stc("Targeting failed.\n\r", ch);
	    return;
	}

	ch->in_room = victim->in_room;
	act2(page->chpoweroff, ch, NULL, victim, TO_VICT); /* Message.1 - Victim */
	ch->in_room = old_room;
	act2(page->victpoweron, ch, NULL, victim, TO_CHAR); /* Message.2 - Char */
	act2(page->victpoweroff, ch, NULL, victim, TO_ROOM); /* Message.3 - Char's Room */

	ch->in_room = victim->in_room;
	do_look(ch, "auto");
	ch->in_room = old_room;
    }
    else
    {
	stc("Spell Failure: Bad Color.\n\r", ch);
	return;
    }

    ch->mana -= cost;

    if (IS_SET(page->spectype, ADV_NEXT_PAGE))
    {
	if (page->specpower < page->value[0])
	{
	    stc("Spell Failure: Pages out of order.\n\r", ch);
	    return;
	}
	if ((page_next = get_page(book, page->specpower)) == NULL)
	{
	    stc("Spell Failure: No next page.\n\r", ch);
	    return;
	}

	if (IS_SET(page_next->spectype, ADV_DAMAGE)) adv_spell_damage(ch, book, page_next, argument, color);
	else if (IS_SET(page_next->spectype, ADV_AFFECT)) adv_spell_affect(ch, book, page_next, argument, color);
	else if (IS_SET(page_next->spectype, ADV_ACTION)) adv_spell_action(ch, book, page_next, argument, color);
	else stc("Spell Failure: Bad next page.\n\r", ch);
    }
    return;
}
