#include <assert.h>
#include <sys/wait.h>
#include <unistd.h>
#include "../Genesis/src/db.c"

time_t current_time;
FILE *fpReserve;
void logchan(char *text) { (void)text; }

static FILE *input(const char *text)
{
    FILE *fp = tmpfile();
    assert(fp);
    fputs(text, fp);
    rewind(fp);
    return fp;
}

static void expect_failure(int kind)
{
    pid_t pid = fork();
    int status;
    assert(pid >= 0);
    if (pid == 0) {
        FILE *fp = input(kind == 0 ? "'unterminated" : "unterminated");
        fpReserve = fopen(NULL_FILE, "r");
        fpArea = fp; /* Exercise error reporting at EOF without a newline. */
        alarm(3);
        if (kind == 0) fread_word(fp);
        else fread_string(fp);
        _exit(0);
    }
    assert(waitpid(pid, &status, 0) == pid);
    assert(WIFEXITED(status) && WEXITSTATUS(status) == 1);
}

int main(void)
{
    FILE *fp;
    char cwd[4096], temp[] = "/tmp/anathema-db-XXXXXX";
    assert(getcwd(cwd, sizeof(cwd)));
    assert(mkdtemp(temp));
    assert(chdir(temp) == 0);
    string_space = calloc(1, MAX_STRING);
    assert(string_space);
    top_string = string_space;
    fBootDb = TRUE;
    fp = input("  'two words' last");
    assert(!strcmp(fread_word(fp), "two words"));
    assert(!strcmp(fread_word(fp), "last"));
    assert(!strcmp(fread_word(fp), ""));
    fclose(fp);
    fp = input("hello\nworld~");
    assert(!strcmp(fread_string(fp), "hello\n\rworld"));
    fclose(fp);
    fp = input("\377~");
    assert((unsigned char)fread_string(fp)[0] == 255);
    fclose(fp);
    expect_failure(0);
    expect_failure(1);
    unlink("shutdown.txt");
    assert(chdir(cwd) == 0);
    assert(rmdir(temp) == 0);
    free(string_space);
    puts("db regressions passed");
    return 0;
}
