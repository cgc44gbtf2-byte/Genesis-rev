/*
 * Synthetic wallgranite regressions; no server or game data required.
 * From mudfix, compile sequentially with -O0:
 * cc -std=gnu17 -O0 -ffunction-sections -fdata-sections \
 *   tests/test_wall.c -Wl,--gc-sections -o /tmp/test_wall
 * /tmp/test_wall
 * Add -fsanitize=address,undefined -fno-omit-frame-pointer for sanitizers.
 */
#include "../Genesis/src/classes/werewolf/wwauspice.c"
#include <assert.h>
#include <ctype.h>
#include <strings.h>

static ROOM_INDEX_DATA origin, destination, elsewhere;
static OBJ_INDEX_DATA wall_index;
static OBJ_DATA walls[2];
static int wall_count, moves;
static long forward, backward;

char *one_argument(char *argument, char *word)
{
    while (isspace((unsigned char)*argument))
        ++argument;
    while (*argument && !isspace((unsigned char)*argument))
        *word++ = tolower((unsigned char)*argument++);
    *word = '\0';
    return argument;
}

bool str_cmp(const char *a, const char *b)
{
    return strcasecmp(a, b) != 0;
}

void send_to_char(const char *message, CHAR_DATA *ch)
{
    (void)message;
    assert(ch->in_room == &origin || ch->in_room == &destination ||
           ch->in_room == NULL);
}

void act(const char *message, CHAR_DATA *ch, const void *a,
         const void *b, long type)
{
    (void)message;
    (void)a;
    (void)b;
    (void)type;
    assert(ch->in_room == &origin || ch->in_room == &destination);
}

long get_auspice(CHAR_DATA *ch, long gift)
{
    assert(gift == AUSPICE_PHILODOX);
    return ch->pcdata->auspice[gift];
}

int number_range(int from, int to)
{
    assert(from == 30 && to == 90);
    return 60;
}

char *str_dup(const char *text)
{
    char *copy = malloc(strlen(text) + 1);
    assert(copy != NULL);
    strcpy(copy, text);
    return copy;
}

void free_string(char *text)
{
    free(text);
}

OBJ_INDEX_DATA *get_obj_index(long vnum)
{
    assert(vnum == OBJ_VNUM_PORTAL);
    return &wall_index;
}

OBJ_DATA *create_object(OBJ_INDEX_DATA *index, long level)
{
    OBJ_DATA *obj;
    assert(index == &wall_index && level == 0);
    assert(wall_count < 2);
    obj = &walls[wall_count++];
    memset(obj, 0, sizeof(*obj));
    obj->name = str_dup("old name");
    obj->short_descr = str_dup("old short description");
    obj->description = str_dup("old description");
    return obj;
}

void obj_to_room(OBJ_DATA *obj, ROOM_INDEX_DATA *room)
{
    assert(obj->in_room == NULL);
    assert(room == &origin || room == &destination);
    obj->in_room = room;
}

void char_from_room(CHAR_DATA *ch)
{
    assert(ch->in_room == &origin || ch->in_room == &destination);
    if (ch->in_room == &origin)
    {
        EXIT_DATA *reverse = destination.exit[backward];
        const long blockers = EX_WALL_WATER | EX_WALL_GRANITE |
                              EX_WALL_WIND | EX_WALL_FLAME;
        /* No temporary movement is allowed until mirroring is validated. */
        assert(reverse != NULL && reverse->to_room == &origin);
        assert((reverse->exit_info & blockers) == 0);
    }
    ch->in_room = NULL;
    ++moves;
}

void char_to_room(CHAR_DATA *ch, ROOM_INDEX_DATA *room)
{
    assert(ch->in_room == NULL);
    assert(room == &origin || room == &destination);
    ch->in_room = room;
    ++moves;
}

enum reverse_case { ABSENT, BLOCKED, WRONG_DESTINATION, PAIRED, SELF_LOOP };

static void run_case(int direction, enum reverse_case kind, long blocker)
{
    static char *names[] = {"north", "east", "south", "west", "up", "down"};
    static const int reverse_dirs[] = {
        DIR_SOUTH, DIR_WEST, DIR_NORTH, DIR_EAST, DIR_DOWN, DIR_UP
    };
    CHAR_DATA ch;
    PC_DATA pc;
    EXIT_DATA source, reverse, unrelated_source, unrelated_destination;
    EXIT_DATA before_source, before_reverse, before_us, before_ud;
    EXIT_DATA *origin_exits[6], *destination_exits[6];
    int i;

    memset(&origin, 0, sizeof(origin));
    memset(&destination, 0, sizeof(destination));
    memset(&elsewhere, 0, sizeof(elsewhere));
    memset(&source, 0, sizeof(source));
    memset(&reverse, 0, sizeof(reverse));
    memset(&unrelated_source, 0, sizeof(unrelated_source));
    memset(&unrelated_destination, 0, sizeof(unrelated_destination));
    memset(&ch, 0, sizeof(ch));
    memset(&pc, 0, sizeof(pc));
    forward = direction;
    backward = reverse_dirs[direction];
    wall_count = moves = 0;
    ch.class = CLASS_WEREWOLF;
    ch.pcdata = &pc;
    ch.in_room = &origin;
    pc.auspice[AUSPICE_PHILODOX] = 4;

    source.to_room = kind == SELF_LOOP ? &origin : &destination;
    source.exit_info = EX_ISDOOR | EX_CLOSED;
    reverse.to_room = kind == WRONG_DESTINATION ? &elsewhere : &origin;
    reverse.exit_info = EX_ISDOOR | EX_LOCKED;
    if (kind == BLOCKED)
        reverse.exit_info |= blocker;
    origin.exit[forward] = &source;
    destination.exit[backward] = kind == ABSENT ? NULL : &reverse;
    unrelated_source.to_room = unrelated_destination.to_room = &elsewhere;
    unrelated_source.exit_info = EX_PICKPROOF;
    unrelated_destination.exit_info = EX_BASHPROOF;
    origin.exit[(forward + 1) % 6] = &unrelated_source;
    destination.exit[(backward + 1) % 6] = &unrelated_destination;
    memcpy(origin_exits, origin.exit, sizeof(origin_exits));
    memcpy(destination_exits, destination.exit, sizeof(destination_exits));
    before_source = source;
    before_reverse = reverse;
    before_us = unrelated_source;
    before_ud = unrelated_destination;

    do_wallgranite(&ch, names[direction]);

    assert(ch.in_room == &origin);
    assert(ch.wait == 12);
    assert(moves == (kind == PAIRED ? 4 : 0));
    assert(wall_count == (kind == PAIRED ? 2 : 1));
    before_source.exit_info |= EX_WALL_GRANITE;
    assert(memcmp(&source, &before_source, sizeof(source)) == 0);
    if (kind == PAIRED)
        before_reverse.exit_info |= EX_WALL_GRANITE;
    assert(memcmp(&reverse, &before_reverse, sizeof(reverse)) == 0);
    assert(memcmp(&unrelated_source, &before_us, sizeof(before_us)) == 0);
    assert(memcmp(&unrelated_destination, &before_ud, sizeof(before_ud)) == 0);
    assert(memcmp(origin.exit, origin_exits, sizeof(origin_exits)) == 0);
    assert(memcmp(destination.exit, destination_exits, sizeof(destination_exits)) == 0);
    for (i = 0; i < wall_count; ++i)
    {
        char description[MAX_INPUT_LENGTH];
        long wall_direction = i == 0 ? forward : backward;
        assert(walls[i].in_room == (i == 0 ? &origin : &destination));
        assert(walls[i].item_type == ITEM_WALL);
        assert(walls[i].value[0] == wall_direction);
        assert(walls[i].timer == 60);
        assert(strcmp(walls[i].name, "wall stone granite") == 0);
        assert(strcmp(walls[i].short_descr, "a wall of granite") == 0);
        snprintf(description, sizeof(description),
                 "A wall of solid granite blocks your exit %s.", names[wall_direction]);
        assert(strcmp(walls[i].description, description) == 0);
        free_string(walls[i].name);
        free_string(walls[i].short_descr);
        free_string(walls[i].description);
    }
}

static void test_missing_location(void)
{
    CHAR_DATA ch;
    PC_DATA pc;
    memset(&ch, 0, sizeof(ch));
    memset(&pc, 0, sizeof(pc));
    ch.class = CLASS_WEREWOLF;
    ch.pcdata = &pc;
    pc.auspice[AUSPICE_PHILODOX] = 4;
    wall_count = moves = 0;
    do_wallgranite(&ch, "north");
    assert(ch.in_room == NULL && wall_count == 0 && moves == 0);
}

int main(void)
{
    static const long blockers[] = {
        EX_WALL_WATER, EX_WALL_GRANITE, EX_WALL_WIND, EX_WALL_FLAME
    };
    int dir;
    size_t i;
    for (dir = 0; dir < 6; ++dir)
    {
        run_case(dir, ABSENT, 0);
        run_case(dir, WRONG_DESTINATION, 0);
        run_case(dir, PAIRED, 0);
        run_case(dir, SELF_LOOP, 0);
        for (i = 0; i < sizeof(blockers) / sizeof(blockers[0]); ++i)
            run_case(dir, BLOCKED, blockers[i]);
    }
    test_missing_location();
    puts("granite wall regressions passed");
    return 0;
}
