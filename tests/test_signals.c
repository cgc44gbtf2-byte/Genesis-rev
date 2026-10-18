/*
 * Isolated stop-signal regressions: no server, accounts or player files.
 * From mudfix:
 * cc -std=gnu17 -DMERC -Dunix -ffunction-sections -fdata-sections \
 *   tests/test_signals.c Genesis/src/signals.c \
 *   -Wl,--gc-sections -lcrypt -o /tmp/test_signals
 * /tmp/test_signals
 */
#include <assert.h>
#include <sys/wait.h>

#define main genesis_server_main
#include "../Genesis/src/comm.c"
#undef main

CHAR_DATA *char_list;
static CHAR_DATA *saved[32];
static int save_count;
static unsigned int save_result = PLAYER_SAVE_ALL;
static int save_warnings;

void log_string(const char *text)
{
    assert(strstr(text,"shutdown primary save failed") != NULL);
    ++save_warnings;
}

void save_char_obj(CHAR_DATA *ch)
{
    assert(ch != NULL && !IS_NPC(ch));
    assert(ch->pcdata != NULL && ch->in_room != NULL);
    assert(ch->desc == NULL); /* Editor/note guards cannot suppress the save. */
    assert(save_count < (int)(sizeof(saved) / sizeof(saved[0])));
    saved[save_count++] = ch;
}

unsigned int player_save_state(CHAR_DATA *ch, unsigned int destinations)
{
    assert(destinations == PLAYER_SAVE_ALL);
    save_char_obj(ch);
    return save_result;
}

static void test_shutdown_failure_reporting(void)
{
    CHAR_DATA ch = {0}; PC_DATA pc = {0};
    ROOM_INDEX_DATA room = {0}; DESCRIPTOR_DATA d = {0};
    ch.name="Failure"; ch.level=LEV_AWA; ch.pcdata=&pc; ch.in_room=&room;
    ch.desc=&d; d.character=&ch; d.connected=CON_NOTE_TEXT;
    d.password_save_pending=TRUE;
    char_list=&ch; save_count=0; save_warnings=0;
    save_result=PLAYER_SAVE_BACKUP;
    save_shutdown_players();
    assert(save_count==1 && save_warnings==1 && d.password_save_pending &&
           ch.desc==&d && d.connected==CON_NOTE_TEXT);
    save_result=PLAYER_SAVE_PRIMARY;
    save_shutdown_players();
    assert(save_count==2 && save_warnings==1 && !d.password_save_pending &&
           ch.desc==&d);
    save_result=PLAYER_SAVE_ALL; char_list=NULL;
}
static void test_save_eligibility(void)
{
    CHAR_DATA players[15] = {{0}};
    PC_DATA pc[15] = {{0}};
    DESCRIPTOR_DATA descriptors[15] = {{0}};
    ROOM_INDEX_DATA room = {0};
    int states[] = {
        CON_PLAYING, CON_AEDITOR, CON_REDITOR, CON_OEDITOR, CON_MEDITOR,
        CON_NOTE_TO, CON_NOTE_FINISH, CON_PLAYING, CON_PLAYING,
        CON_GET_OLD_PASSWORD, CON_READ_MOTD, CON_PLAYING, CON_PLAYING,
        CON_PLAYING, CON_PLAYING
    };
    int i;

    for (i = 0; i < 15; ++i)
    {
        players[i].name = "Signaltest";
        players[i].level = 2;
        players[i].pcdata = &pc[i];
        players[i].in_room = &room;
        players[i].desc = &descriptors[i];
        players[i].next = i < 14 ? &players[i + 1] : NULL;
        descriptors[i].character = &players[i];
        descriptors[i].connected = states[i];
    }
    players[7].desc = NULL; /* Fully loaded link-dead player. */
    players[8].desc = NULL; /* Switched original, still in char_list. */
    descriptors[11].original = &players[8];
    SET_BIT(players[11].act, ACT_IS_NPC); /* Switched NPC body. */
    players[12].in_room = NULL;
    players[13].pcdata = NULL;
    players[14].level = 1;
    char_list = players;

    save_count = 0;
    save_shutdown_players();
    assert(save_count == 9);
    for (i = 0; i < 9; ++i)
        assert(saved[i] == &players[i]);
    for (i = 0; i < 15; ++i)
    {
        assert(players[i].desc ==
               ((i == 7 || i == 8) ? NULL : &descriptors[i]));
        assert(descriptors[i].connected == states[i]);
    }
    char_list = NULL;
}

static void test_stop_handlers(void)
{
    struct sigaction action;
    int before = save_count;

    init_signals();
    assert(sigaction(SIGTERM, NULL, &action) == 0);
    assert(action.sa_handler != SIG_DFL && action.sa_handler != SIG_IGN);
    assert(!(action.sa_flags & SA_RESTART));
    shutdown_requested = 0;
    errno = EDOM;
    assert(raise(SIGTERM) == 0);
    assert(shutdown_requested == 1 && errno == EDOM);
    assert(save_count == before); /* The handler must never walk/save players. */
    assert(raise(SIGTERM) == 0);
    assert(shutdown_requested == 1 && save_count == before);
    shutdown_requested = 0;
    assert(raise(SIGINT) == 0);
    assert(shutdown_requested == 1 && save_count == before);
    shutdown_requested = 0;
    assert(raise(SIGPIPE) == 0);
    assert(shutdown_requested == 0);
}

static void test_fatal_defaults(void)
{
    int fatal[] = {SIGBUS, SIGSEGV, SIGILL, SIGFPE, SIGTRAP, SIGABRT};
    unsigned int i;

    for (i = 0; i < sizeof(fatal) / sizeof(fatal[0]); ++i)
    {
        struct sigaction action;
        pid_t pid;
        int status;

        assert(sigaction(fatal[i], NULL, &action) == 0);
        assert(action.sa_handler == SIG_DFL);
        pid = fork();
        assert(pid >= 0);
        if (pid == 0)
        {
            struct rlimit no_core = {0, 0};
            setrlimit(RLIMIT_CORE, &no_core);
            raise(fatal[i]);
            _exit(99);
        }
        assert(waitpid(pid, &status, 0) == pid);
        assert(WIFSIGNALED(status) && WTERMSIG(status) == fatal[i]);
    }
}

static void test_select_interrupt(int with_descriptors, int sig)
{
    int fds[2], status, result;
    fd_set reads;
    struct timeval delay = {2, 0};
    pid_t pid;

    assert(pipe(fds) == 0);
    shutdown_requested = 0;
    pid = fork();
    assert(pid >= 0);
    if (pid == 0)
    {
        usleep(20000);
        kill(getppid(), sig);
        _exit(0);
    }
    FD_ZERO(&reads);
    FD_SET(fds[0], &reads);
    /* Exercise both select shapes used by the game: poll and stall. */
    result = select(with_descriptors ? fds[0] + 1 : 0,
                    with_descriptors ? &reads : NULL, NULL, NULL, &delay);
    assert(result == -1 && errno == EINTR);
    assert(shutdown_requested == 1);
    assert(waitpid(pid, &status, 0) == pid);
    assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
    close(fds[0]);
    close(fds[1]);
}

static void test_socket_reuse(void)
{
    int fd = init_socket(0);
    int enabled = 0;
    socklen_t size = sizeof(enabled);

    assert(getsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enabled, &size) == 0);
    assert(enabled == 1);
    close(fd);
}

int main(void)
{
    test_save_eligibility();
    test_shutdown_failure_reporting();
    test_stop_handlers();
    test_select_interrupt(1, SIGTERM);
    test_select_interrupt(0, SIGINT);
    test_fatal_defaults();
    test_socket_reuse();
    puts("signal regressions passed");
    return 0;
}
