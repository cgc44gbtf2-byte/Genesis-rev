/*
 * Isolated comm.c regressions (no server or player files are loaded).
 * From mudfix:
 * cc -std=gnu17 -DMERC -Dunix -ffunction-sections -fdata-sections \
 *   tests/test_comm.c -Wl,--gc-sections \
 *   /usr/lib/x86_64-linux-gnu/libcrypt.so.1 -o /tmp/test_comm
 * /tmp/test_comm
 */
#include <assert.h>
#include <unistd.h>
extern char *crypt(const char *key, const char *salt);

static ssize_t comm_test_read(int fd, void *buf, size_t count);
static ssize_t comm_test_write(int fd, const void *buf, size_t count);
static char *comm_test_crypt(const char *key, const char *salt);

#define main genesis_server_main
#define read comm_test_read
#define write comm_test_write
#define crypt comm_test_crypt
#include "../Genesis/src/comm.c"
#undef main
#undef read
#undef write
#undef crypt

static int read_mode, read_calls, write_mode, write_calls, fail_crypt;
static int reported_bugs;
char log_buf[MAX_STRING_LENGTH];

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
    return crypt(key, salt);
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
    (void)ch;
    (void)argument;
    assert(!"unexpected password change");
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

    memset(&ch, 0, sizeof(ch));
    memset(oversized, 'x', sizeof(oversized) - 1);
    oversized[sizeof(oversized) - 1] = '\0';
    do_password(&ch, oversized);
    memcpy(oversized, "old ", 4);
    do_password(&ch, oversized);
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
    test_prompt_drain();
    puts("comm regressions passed");
    return 0;
}
