/*
 * Isolated tests of production helpers (no server or game data).
 * From mudfix:
 * cc -std=gnu17 -O1 -g -ffunction-sections -fdata-sections \
 *   tests/test_buffers.c Genesis/src/string.c Genesis/src/interp.c \
 *   -Wl,--gc-sections -o /tmp/test_buffers
 * Add -fsanitize=address,undefined -fno-omit-frame-pointer to instrument.
 * With GCC ASan, compile interp.c separately with --param=asan-globals=0:
 * ASan's global registration otherwise retains its unused command table
 * and, in turn, every game command. Other translation units need no override.
 */
#include <assert.h>
#include <unistd.h>
#include "../Genesis/src/db.c"

time_t current_time;
FILE *fpReserve;
static char logged[MAX_STRING_LENGTH];
static char sent[MAX_STRING_LENGTH];

void logchan(char *text)
{
    snprintf(logged, sizeof(logged), "%s", text);
}

void send_to_char(const char *text, CHAR_DATA *ch)
{
    (void)ch;
    snprintf(sent, sizeof(sent), "%s", text);
}

static char *filled(size_t length, char c)
{
    char *text = malloc(length + 1);
    assert(text);
    memset(text, c, length);
    text[length] = '\0';
    return text;
}

static void test_logging(void)
{
    char output[MAX_STRING_LENGTH];
    char *large = filled(2 * MAX_STRING_LENGTH, 'a');

    bug("number %li", 123456789L);
    assert(!strcmp(logged, "[*****] BUG: number 123456789"));
    bug("character %c", 'Q');
    assert(!strcmp(logged, "[*****] BUG: character Q"));
    bug("integer %d", -7);
    assert(!strcmp(logged, "[*****] BUG: integer -7"));
    bug("literal %n %s %p %*d %", 0);
    assert(!strcmp(logged, "[*****] BUG: literal %n %s %p %*d %"));
    format_bug(output, sizeof(output), "%li %li %%", 42);
    assert(!strcmp(output, "42 %li %"));
    format_bug(output, sizeof(output), large, 0);
    assert(strlen(output) == sizeof(output) - 1);
    assert(output[sizeof(output) - 2] == 'a');
    free(large);
}

static void test_arguments(void)
{
    struct {
        char word[MAX_INPUT_LENGTH];
        unsigned char guard[8];
    } result;
    char *large = filled(3 * MAX_INPUT_LENGTH, 'A');
    char *input = malloc(strlen(large) + 16);
    char *rest;
    assert(input);
    sprintf(input, "'%s' tail", large);
    memset(&result, 0xa5, sizeof(result));
    rest = first_arg(input, result.word, TRUE);
    assert(!strcmp(rest, "tail"));
    assert(strlen(result.word) == MAX_INPUT_LENGTH - 1);
    assert(result.word[0] == 'a');
    for (size_t i = 0; i < sizeof(result.guard); ++i)
        assert(result.guard[i] == 0xa5);
    memset(&result, 0xa5, sizeof(result));
    rest = one_argument(input, result.word);
    assert(!strcmp(rest, "tail"));
    assert(strlen(result.word) == MAX_INPUT_LENGTH - 1);
    assert(result.word[0] == 'a');
    for (size_t i = 0; i < sizeof(result.guard); ++i)
        assert(result.guard[i] == 0xa5);
    assert(!strcmp(first_arg("(Two words) rest", result.word, FALSE), "rest"));
    assert(!strcmp(result.word, "Two words"));
    free(input);
    free(large);
}

static void test_editor(void)
{
    CHAR_DATA ch = {0};
    DESCRIPTOR_DATA descriptor = {0};
    char *text = str_dup("");
    char *large = filled(2 * MAX_STRING_LENGTH, 'x');
    char *original;

    ch.desc = &descriptor;
    string_append(&ch, &text); /* Empty text must not read before its start. */
    assert(descriptor.pString == &text);
    string_add(&ch, "hello");
    assert(!strcmp(text, "hello\n\r"));
    string_add(&ch, large);
    assert(strlen(text) == MAX_STRING_LENGTH - 1);
    assert(!strncmp(text, "hello\n\r", 7));
    assert(descriptor.pString == NULL);
    free_string(text);

    text = str_dup(large);
    descriptor.pString = &text;
    original = text;
    string_add(&ch, "more");
    assert(text == original && !strcmp(text, large));
    assert(strstr(sent, "Existing string too long"));
    string_add(&ch, "@");
    assert(descriptor.pString == NULL);
    free_string(text);

    text = str_dup("one two");
    descriptor.pString = &text;
    string_add(&ch, ".r 'two' 'three'");
    assert(!strcmp(text, "one three"));
    assert(strstr(sent, "replaced"));
    string_add(&ch, ".f");
    assert(!strcmp(text, "One three\n\r"));
    assert(!strcmp(sent, "String formatted.\n\r"));
    free_string(text);
    text = str_dup(large);
    descriptor.pString = &text;
    original = text;
    string_add(&ch, ".f");
    assert(text == original);
    assert(!strcmp(sent, "String too long to format.\n\r"));
    free_string(text);
    free(large);
}

static void test_strings(void)
{
    char *large = filled(2 * MAX_STRING_LENGTH, 'x');
    char *original;
    char *text = str_dup("hello world");

    text = string_replace(text, "world", "mud");
    assert(!strcmp(text, "hello mud"));
    original = text;
    assert(string_replace(text, "missing", "new") == original);
    assert(string_replace(text, "", "new") == original);
    assert(string_replace(text, "mud", large) == original);
    assert(!strcmp(text, "hello mud"));
    free_string(text);

    text = str_dup(large);
    original = text;
    assert(string_replace(text, "absent", "new") == original);
    assert(string_replace(text, "x", "xx") == original);
    free_string(text);
    assert(strlen(capitalize(large)) == MAX_STRING_LENGTH - 1);
    assert(capitalize(large)[0] == 'X');
    assert(strlen(upper_cap(large)) == MAX_STRING_LENGTH - 1);
    text = string_unpad(str_dup(large));
    assert(!strcmp(text, large));
    free_string(text);
    text = string_unpad(str_dup("   hello world   "));
    assert(!strcmp(text, "hello world"));
    free_string(text);
    text = string_unpad(str_dup("    "));
    assert(!strcmp(text, ""));
    free_string(text);
    {
        char *boundary = filled(MAX_STRING_LENGTH - 2, 'x');
        text = string_replace(str_dup(boundary), "x", "xx");
        assert(strlen(text) == MAX_STRING_LENGTH - 1);
        original = text;
        assert(string_replace(text, "x", "xx") == original);
        free_string(text);
        free(boundary);
    }
    free(large);
}

static void test_formatting(void)
{
    const char *inputs[] = {"", " ", "\n", "\r", ")", ".", "!", "?", ".\"",
                            "  hello\nworld. next!  "};
    const char *expected[] = {"\n\r", "\n\r", "\n\r", "\n\r", ")\n\r",
                             ".\n\r", "!\n\r", "?\n\r", ".\"\n\r",
                             "Hello world.  Next!\n\r"};
    for (size_t i = 0; i < sizeof(inputs) / sizeof(inputs[0]); ++i) {
        char *text = format_string(str_dup(inputs[i]));
        assert(!strcmp(text, expected[i]));
        free_string(text);
    }
    {
        char *word = filled(300, 'a');
        char *text = format_string(str_dup(word));
        size_t letters = 0;
        for (char *p = text; *p; ++p)
            if (*p == 'A' || *p == 'a') ++letters;
        assert(letters == 300); /* Hard wrapping must not drop split letters. */
        free_string(text);
        free(word);
    }
    {
        char *punctuation = filled(MAX_STRING_LENGTH - 1, '.');
        char *text = str_dup(punctuation);
        char *original = text;
        assert(format_string(text) == original);
        assert(!strcmp(text, punctuation));
        free_string(text);
        free(punctuation);
    }
    {
        char *word = filled(MAX_STRING_LENGTH - 10, 'a');
        for (size_t i = 9; i < strlen(word); i += 10)
            word[i] = ' ';
        char *text = str_dup(word);
        char *original = text;
        /* Normalization fits, but adding wrap newlines does not. */
        assert(format_string(text) == original);
        assert(!strcmp(text, word));
        free_string(text);
        free(word);
    }
}

static void test_eof(void)
{
    FILE *fp = tmpfile();
    assert(fp);
    fputs("comment without newline", fp);
    rewind(fp);
    alarm(3);
    fread_to_eol(fp);
    assert(feof(fp));
    rewind(fp);
    fputs("comment\n\rnext", fp);
    fflush(fp);
    rewind(fp);
    fread_to_eol(fp);
    assert(getc(fp) == 'n');
    fclose(fp);
    alarm(0);
}

int main(void)
{
    string_space = calloc(1, MAX_STRING);
    assert(string_space);
    top_string = string_space;
    fpReserve = fopen(NULL_FILE, "r");
    assert(fpReserve);
    test_logging();
    test_arguments();
    test_editor();
    test_strings();
    test_formatting();
    test_eof();
    fclose(fpReserve);
    free(string_space);
    puts("buffer regressions passed");
    return 0;
}
