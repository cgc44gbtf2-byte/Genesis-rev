/*
 * Isolated comm.c regressions (no server or player files are loaded).
 * From mudfix:
 * cc -std=gnu17 -DMERC -Dunix -ffunction-sections -fdata-sections \
 *   tests/test_comm.c -Wl,--gc-sections \
 *   -lcrypt -o /tmp/test_comm
 * /tmp/test_comm
 */
#include <assert.h>
#include <unistd.h>
#include <crypt.h>

static ssize_t comm_test_read(int fd, void *buf, size_t count);
static ssize_t comm_test_write(int fd, const void *buf, size_t count);
static char *comm_test_crypt(const char *key, const char *salt);
static char *comm_test_gensalt(const char *prefix, unsigned long cost,
                              const char *entropy, int entropy_size,
                              char *output, int output_size);

#define main genesis_server_main
#define read comm_test_read
#define write comm_test_write
#define crypt comm_test_crypt
#define crypt_gensalt_rn comm_test_gensalt
#include "../Genesis/src/comm.c"
#undef main
#undef read
#undef write
#undef crypt
#undef crypt_gensalt_rn

static int read_mode, read_calls, write_mode, write_calls, fail_crypt;
static int fail_gensalt, saved_passwords, fail_save;
static char saved_password[CRYPT_OUTPUT_SIZE];
static int reported_bugs;
char log_buf[MAX_STRING_LENGTH];
CHAR_DATA *char_list;

static ssize_t comm_test_read(int fd, void *buf, size_t count)
{
    (void)fd;
    assert(count > 0);
    ++read_calls;
    if (read_mode == 0)
    {
        memset(buf, 'x', count);
        return count;
    }
    if (read_calls == 1)
    {
        errno = EINTR;
        return -1;
    }
    errno = EAGAIN;
    return -1;
}

static ssize_t comm_test_write(int fd, const void *buf, size_t count)
{
    (void)fd;
    (void)buf;
    assert(count > 0);
    ++write_calls;
    if (write_mode == 1)
        return 0;
    if (write_mode == 2)
        return count > 3 ? 3 : count;
    if (write_calls == 1)
    {
        errno = EINTR;
        return -1;
    }
    if (write_calls == 2)
        return 2;
    errno = EAGAIN;
    return -1;
}

static char *comm_test_crypt(const char *key, const char *salt)
{
    if (fail_crypt == 1)
        return NULL;
    if (fail_crypt == 2)
        return "*0";
    if (fail_crypt == 3 && !strncmp(salt, "$y$", 3))
        return "xy12345678901"; /* A provider must never silently downgrade. */
    if (fail_crypt == 4 && !strncmp(salt, "$y$", 3))
        return "$y$invalid~hash";
    return crypt(key, salt);
}

static char *comm_test_gensalt(const char *prefix, unsigned long cost,
                              const char *entropy, int entropy_size,
                              char *output, int output_size)
{
    assert(!strcmp(prefix, "$y$"));
    assert(cost == 5);
    assert(entropy == NULL && entropy_size == 0); /* OS randomness, not game RNG. */
    if (fail_gensalt == 1)
        return NULL;
    if (fail_gensalt == 2)
    {
        strcpy(output, "xy");
        return output;
    }
    return crypt_gensalt_rn(prefix, cost, entropy, entropy_size, output, output_size);
}

void *alloc_mem(long size)
{
    void *p = malloc(size);
    assert(p != NULL);
    return p;
}

void free_mem(void *p, long size)
{
    (void)size;
    free(p);
}

void bug(const char *message, long value)
{
    (void)message;
    (void)value;
    ++reported_bugs;
}

void log_string(const char *message)
{
    (void)message;
}

int number_range(int from, int to)
{
    (void)to;
    return from;
}

char *str_dup(const char *text)
{
    return strdup(text);
}

void free_string(char *text)
{
    free(text);
}

void do_autosave(CHAR_DATA *ch, char *argument)
{
    (void)argument;
    assert(ch->desc != NULL && ch->desc->connected == CON_PLAYING);
    assert(IS_EXTRA(ch, EXTRA_NEWPASS));
    assert(!strncmp(ch->pcdata->pwd, "$y$", 3));
    assert(strlen(ch->pcdata->pwd) < sizeof(saved_password));
    strcpy(saved_password, ch->pcdata->pwd);
    ++saved_passwords;
}

bool save_char_obj_checked(CHAR_DATA *ch)
{
    if (fail_save || ch->level < 2)
        return FALSE;
    assert(!IS_NPC(ch));
    assert(!strncmp(ch->pcdata->pwd, "$y$", 3));
    strcpy(saved_password, ch->pcdata->pwd);
    ++saved_passwords;
    return TRUE;
}

bool str_cmp(const char *a, const char *b)
{
    return strcasecmp(a, b) != 0;
}

void free_char(CHAR_DATA *ch)
{
    (void)ch; /* Synthetic stack login shell. */
}

void do_return(CHAR_DATA *ch, char *argument)
{
    (void)ch;
    (void)argument;
    assert(!"takeover must detach the old descriptor before closing it");
}

void act(const char *format, CHAR_DATA *ch, const void *a,
         const void *b, long type)
{
    (void)format; (void)ch; (void)a; (void)b; (void)type;
}

long get_trust(CHAR_DATA *ch)
{
    (void)ch;
    return 0;
}

long char_ac(CHAR_DATA *ch)
{
    (void)ch;
    assert(!"unexpected custom prompt");
    return 0;
}

long char_hitroll(CHAR_DATA *ch)
{
    return char_ac(ch);
}

long char_damroll(CHAR_DATA *ch)
{
    return char_ac(ch);
}

static void test_input(void)
{
    DESCRIPTOR_DATA d;
    memset(&d, 0, sizeof(d));
    read_calls = 0;
    read_mode = 0;
    assert(read_from_descriptor(&d));
    assert(read_calls == 1);
    assert(strlen(d.inbuf) == sizeof(d.inbuf) - 10);

    strcpy(d.inbuf, "abc");
    read_calls = 0;
    read_mode = 1;
    assert(read_from_descriptor(&d));
    assert(read_calls == 2);
    assert(!strcmp(d.inbuf, "abc"));
}

static void test_output(void)
{
    DESCRIPTOR_DATA d;
    CHAR_DATA ch;
    char single_hash = '#';
    char colors[MAX_STRING_LENGTH - 1];
    long i;
    memset(&d, 0, sizeof(d));
    memset(&ch, 0, sizeof(ch));
    d.outsize = 2000;
    d.outbuf = alloc_mem(d.outsize);
    d.fcommand = TRUE;
    write_to_buffer(&d, &single_hash, 1);
    assert(d.outbuf[0] == '#');
    assert(d.outbuf[d.outtop] == '\0');
    d.outtop = 0;
    d.character = &ch;
    SET_BIT(ch.act, PLR_ANSI);
    for (i = 0; i < sizeof(colors); ++i)
        colors[i] = i % 2 ? 'n' : '#';
    write_to_buffer(&d, colors, sizeof(colors));
    assert(d.outtop > sizeof(colors));
    assert(d.outtop < d.outsize);
    assert(d.outbuf[d.outtop] == '\0');
    free_mem(d.outbuf, d.outsize);

    d.outsize = 128000;
    d.outtop = d.outsize - 2;
    d.outbuf = alloc_mem(d.outsize);
    write_to_buffer(&d, "x", 1);
    assert(reported_bugs == 1);
    assert(d.outtop == d.outsize);
    write_to_buffer(&d, "ignored", 7);
    assert(reported_bugs == 1);
    free_mem(d.outbuf, d.outsize);

    write_mode = 0;
    write_calls = 0;
    assert(write_descriptor_bytes(1, "abcdef", 6) == 2);
    assert(write_calls == 3);
    write_mode = 1;
    write_calls = 0;
    assert(!write_to_descriptor(1, "abcdef", 6));
    assert(write_calls == 1);
    write_mode = 2;
    write_calls = 0;
    assert(write_to_descriptor(1, "abcdef", 6));
    assert(write_calls == 2);
}

static void test_passwords(void)
{
    CHAR_DATA ch;
    char oversized[2 * MAX_INPUT_LENGTH + 8];
    char stored[256];
    char *encoded = crypt("synthetic-test-password", "xy");
    assert(encoded != NULL && encoded[0] != '*');
    strcpy(stored, encoded);
    assert(password_matches("synthetic-test-password", stored, FALSE));
    assert(!password_matches("incorrect", stored, FALSE));
    assert(!password_matches(stored, stored, FALSE));
    assert(!password_matches(stored, stored, TRUE));
    assert(password_matches("legacy-test", "legacy-test", TRUE));
    assert(!password_matches("legacy-test", "legacy-test", FALSE));
    assert(!password_matches("", "", TRUE));
    assert(!password_matches("test", NULL, TRUE));
    assert(!password_matches(NULL, stored, TRUE));
    fail_crypt = 1;
    assert(!password_matches("synthetic-test-password", stored, FALSE));
    fail_crypt = 2;
    assert(!password_matches("synthetic-test-password", stored, FALSE));
    assert(!password_matches("*0", "*0", TRUE));
    assert(!password_matches("$y$invalid", "$y$invalid", TRUE));
    assert(!password_matches("_invalid", "_invalid", TRUE));
    assert(!password_matches("!locked", "!locked", TRUE));
    fail_crypt = 0;

    memset(&ch, 0, sizeof(ch));
    memset(oversized, 'x', sizeof(oversized) - 1);
    oversized[sizeof(oversized) - 1] = '\0';
    do_password(&ch, oversized);
    memcpy(oversized, "old ", 4);
    do_password(&ch, oversized);
}

static void test_modern_passwords(void)
{
    CHAR_DATA ch = {0};
    PC_DATA pc = {0};
    DESCRIPTOR_DATA d = {0};
    char first[CRYPT_OUTPUT_SIZE];
    char old[CRYPT_OUTPUT_SIZE];
    int i;

    ch.pcdata = &pc;
    ch.desc = &d;
    d.character = &ch;
    d.connected = CON_GET_NEW_PASSWORD;
    pc.pwd = str_dup("");

    /* This is the shared new-character/change-password creation path. */
    assert(password_set(&ch, "modern-test-password"));
    assert(!strncmp(pc.pwd, "$y$", 3));
    assert(IS_EXTRA(&ch, EXTRA_NEWPASS));
    assert(password_matches("modern-test-password", pc.pwd, FALSE));
    assert(password_matches("modern-test-password", pc.pwd, TRUE));
    assert(!password_matches("modern-test-password-wrong", pc.pwd, FALSE));
    assert(!password_matches(pc.pwd, pc.pwd, TRUE));
    strcpy(first, pc.pwd);
    assert(password_set(&ch, "modern-test-password"));
    assert(strcmp(first, pc.pwd)); /* Same input must get a fresh random salt. */
    assert(password_matches("modern-test-password", pc.pwd, FALSE));
    assert(saved_passwords == 0);

    /* New yescrypt hashes distinguish characters after DES's eight-byte limit. */
    assert(password_set(&ch, "12345678suffix-one"));
    assert(!password_matches("12345678suffix-two", pc.pwd, FALSE));
    assert(password_set(&ch, "case ~ ' \" sensitive"));
    assert(password_matches("case ~ ' \" sensitive", pc.pwd, FALSE));
    strcpy(old, pc.pwd);
    for (i = 1; i <= 4; ++i)
    {
        fail_crypt = i;
        assert(!password_set(&ch, "replacement-test"));
        assert(!strcmp(pc.pwd, old));
    }
    fail_crypt = 0;
    for (i = 1; i <= 2; ++i)
    {
        fail_gensalt = i;
        assert(!password_set(&ch, "replacement-test"));
        assert(!strcmp(pc.pwd, old));
    }
    fail_gensalt = 0;
    assert(!password_set(&ch, NULL));
    assert(!password_set(&ch, ""));
    free_string(pc.pwd);
}

static void test_password_migration(void)
{
    CHAR_DATA ch = {0};
    PC_DATA pc = {0};
    DESCRIPTOR_DATA d = {0};
    char legacy[CRYPT_OUTPUT_SIZE];
    char migrated[CRYPT_OUTPUT_SIZE];
    int i, flag;

    ch.level = 2;
    ch.pcdata = &pc;
    ch.desc = &d;
    d.character = &ch;
    strcpy(legacy, crypt("legacy-test-password", "xy"));

    /* DES migrates with or without EXTRA_NEWPASS. Do not hash the stored hash. */
    for (flag = 0; flag <= 1; ++flag)
    {
        ch.extra = flag ? EXTRA_NEWPASS : 0;
        pc.pwd = str_dup(legacy);
        d.connected = CON_GET_OLD_PASSWORD;
        assert(!password_upgrade(&d, legacy)); /* Pass-the-hash remains rejected. */
        assert(!password_upgrade(&d, "wrong"));
        assert(!strcmp(pc.pwd, legacy));
        assert(!d.password_save_pending);
        assert(password_upgrade(&d, "legacy-test-password"));
        assert(strncmp(pc.pwd, "$y$", 3) == 0);
        assert(password_matches("legacy-test-password", pc.pwd, FALSE));
        assert(d.password_save_pending);
        password_save_if_ready(&d);
        assert(saved_passwords == flag); /* Never save the short-login shell. */
        d.connected = CON_READ_MOTD;
        password_save_if_ready(&d);
        assert(saved_passwords == flag);
        d.connected = CON_PLAYING;
        ch.level = LEV_SLE; /* This tree's sleeper is level 1, not level 2. */
        password_save_if_ready(&d);
        assert(d.password_save_pending);
        assert(saved_passwords == flag);
        ch.level = 2;
        fail_save = 1;
        password_save_if_ready(&d);
        assert(d.password_save_pending);
        assert(saved_passwords == flag);
        fail_save = 0;
        password_save_if_ready(&d);
        assert(saved_passwords == flag + 1);
        assert(!d.password_save_pending);
        assert(!strcmp(saved_password, pc.pwd));
        strcpy(migrated, pc.pwd);
        assert(password_upgrade(&d, "legacy-test-password"));
        assert(!d.password_save_pending); /* Modern hashes are not re-salted on login. */
        assert(!strcmp(migrated, pc.pwd));
        free_string(pc.pwd);
    }

    /* Failure keeps the original hash and migration flag intact; no save. */
    ch.extra = 0;
    pc.pwd = str_dup(legacy);
    for (i = 1; i <= 4; ++i)
    {
        fail_crypt = i;
        assert(!password_upgrade(&d, "legacy-test-password"));
        assert(!strcmp(pc.pwd, legacy));
        assert(!IS_EXTRA(&ch, EXTRA_NEWPASS));
        assert(!d.password_save_pending);
    }
    fail_crypt = 0;
    fail_gensalt = 1;
    assert(!password_upgrade(&d, "legacy-test-password"));
    assert(!strcmp(pc.pwd, legacy));
    assert(!d.password_save_pending);
    fail_gensalt = 0;
    free_string(pc.pwd);

    /* Preserve the explicit plaintext legacy gate, including short passwords. */
    pc.pwd = str_dup("x");
    ch.extra = EXTRA_NEWPASS;
    assert(!password_upgrade(&d, "x"));
    ch.extra = 0;
    assert(password_upgrade(&d, "x"));
    assert(password_matches("x", pc.pwd, FALSE));
    password_save_if_ready(&d);
    assert(saved_passwords == 3);
    free_string(pc.pwd);
}

static void test_switched_takeover(void)
{
    CHAR_DATA player = {0}, npc = {0}, shell = {0};
    PC_DATA pc = {0};
    DESCRIPTOR_DATA old = {0}, incoming = {0};
    int before = saved_passwords;

    player.name = shell.name = "Synthetic";
    player.level = 2;
    player.pcdata = &pc;
    player.extra = EXTRA_SWITCH;
    pc.pwd = str_dup(crypt("live-password", "xy"));
    npc.name = "Syntheticmob";
    npc.act = ACT_IS_NPC;
    npc.desc = &old;
    old.character = &npc;
    old.original = &player;
    old.connected = CON_PLAYING;
    old.descriptor = dup(STDERR_FILENO);
    old.host = str_dup("synthetic");
    old.outsize = 4096;
    old.outbuf = alloc_mem(old.outsize);
    old.next = &incoming;
    incoming.character = &shell;
    incoming.host = "synthetic";
    incoming.connected = CON_GET_OLD_PASSWORD;
    incoming.outsize = 4096;
    incoming.outbuf = alloc_mem(incoming.outsize);
    descriptor_list = &old;
    char_list = &player;

    assert(!password_prepare_live(&incoming, "stale-password"));
    assert(old.character == &npc && npc.desc == &old);
    fail_gensalt = 1;
    assert(!password_prepare_live(&incoming, "live-password"));
    assert(old.original == &player && descriptor_list == &old);
    fail_gensalt = 0;
    assert(password_prepare_live(&incoming, "live-password"));
    assert(check_playing(&incoming, shell.name));
    assert(incoming.character == &npc && incoming.original == &player);
    assert(npc.desc == &incoming && player.desc == NULL);
    assert(old.character == NULL && old.original == NULL);
    assert(password_upgrade(&incoming, "live-password"));
    fail_save = 1;
    password_save_if_ready(&incoming);
    assert(incoming.password_save_pending && saved_passwords == before);
    fail_save = 0;
    password_save_if_ready(&incoming);
    assert(!incoming.password_save_pending && saved_passwords == before + 1);
    free(pc.pwd);
    free(incoming.outbuf);
    descriptor_list = descriptor_free = NULL;
    char_list = NULL;
}

static void test_password_change(void)
{
    CHAR_DATA ch = {0};
    PC_DATA pc = {0};
    DESCRIPTOR_DATA d = {0};
    char old[CRYPT_OUTPUT_SIZE];
    int before = saved_passwords;

    ch.pcdata = &pc;
    ch.desc = &d;
    d.character = &ch;
    d.connected = CON_PLAYING;
    d.outsize = 2000;
    d.outbuf = alloc_mem(d.outsize);
    d.fcommand = TRUE;
    pc.pwd = str_dup(crypt("old-test", "xy"));
    strcpy(old, pc.pwd);
    fail_gensalt = 1;
    do_password(&ch, "old-test new-test-password");
    assert(!strcmp(pc.pwd, old));
    assert(!IS_EXTRA(&ch, EXTRA_NEWPASS));
    assert(saved_passwords == before);
    assert(strstr(d.outbuf, "encryption failed"));
    assert(!strstr(d.outbuf, "old-test") && !strstr(d.outbuf, "new-test-password"));
    fail_gensalt = 0;
    do_password(&ch, "old-test new-test-password");
    assert(password_matches("new-test-password", pc.pwd, FALSE));
    assert(!password_matches("old-test", pc.pwd, FALSE));
    strcpy(old, pc.pwd);
    do_password(&ch, "new-test-password another-test-password");
    assert(strcmp(pc.pwd, old));
    assert(password_matches("another-test-password", pc.pwd, FALSE));
    assert(!password_matches("new-test-password", pc.pwd, FALSE));
    assert(saved_passwords == before + 2);
    assert(!strcmp(saved_password, pc.pwd));
    do_password(&ch, "wrong-test another-test-password");
    assert(saved_passwords == before + 2);
    free_string(pc.pwd);
    free_mem(d.outbuf, d.outsize);
}

static void test_prompt_drain(void)
{
    DESCRIPTOR_DATA d;
    CHAR_DATA ch;
    long pending;

    memset(&d, 0, sizeof(d));
    memset(&ch, 0, sizeof(ch));
    d.character = &ch;
    d.connected = CON_PLAYING;
    d.outsize = 2000;
    d.outbuf = alloc_mem(d.outsize);
    d.fcommand = TRUE;
    SET_BIT(ch.act, PLR_TELNET_GA);
    write_to_buffer(&d, "reply", 5);

    write_mode = 0;
    write_calls = 0;
    assert(process_output(&d, TRUE));
    assert(!d.fcommand);
    pending = d.outtop;
    assert(pending > 0);
    /* The synthetic writer remains blocked after its first two bytes. */
    assert(process_output(&d, TRUE));
    assert(d.outtop == pending);
    assert(process_output(&d, FALSE));
    assert(d.outtop == pending);
    write_mode = 2;
    assert(process_output(&d, TRUE));
    assert(d.outtop == 0);

    /* OLC prompts obey the same requested/fresh-command rules. */
    d.pString = (char **)&d.pEdit;
    d.fcommand = TRUE;
    write_mode = 0;
    write_calls = 0;
    assert(process_output(&d, TRUE));
    pending = d.outtop;
    assert(pending > 0);
    assert(process_output(&d, TRUE));
    assert(d.outtop == pending);
    assert(process_output(&d, FALSE));
    assert(d.outtop == pending);
    free_mem(d.outbuf, d.outsize);
}

int main(void)
{
    test_input();
    test_output();
    test_passwords();
    test_modern_passwords();
    test_password_migration();
    test_password_change();
    test_prompt_drain();
    test_switched_takeover();
    puts("comm regressions passed");
    return 0;
}
