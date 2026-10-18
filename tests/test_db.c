#include <assert.h>
#include <limits.h>
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

static int old_small_range(int from,int to)
{
    int power=2,number;
    if ((to=to-from+1)<=1) return from;
    while (power<to) power<<=1;
    do { number=number_mm()&(power-1); } while(number>=to);
    return from+number;
}

static void random_and_mobile_bounds(void)
{
    int before[57],after[57];
    current_time=1792252800; init_mm();
    const int ordinary[][2]={{1,100},{-1000,1000},{0,16777215},
        {INT_MIN,INT_MIN+100},{INT_MAX-100,INT_MAX},{10,-10},{7,7}};
    for (int i=0;i<100;++i)
        for (size_t j=0;j<sizeof(ordinary)/sizeof(ordinary[0]);++j) {
            memcpy(before,rgiState,sizeof(before));
            int expected=old_small_range(ordinary[j][0],ordinary[j][1]);
            memcpy(after,rgiState,sizeof(after));
            memcpy(rgiState,before,sizeof(before));
            assert(number_range(ordinary[j][0],ordinary[j][1])==expected);
            assert(memcmp(rgiState,after,sizeof(after))==0);
        }
    bool upper=FALSE,negative=FALSE,positive=FALSE;
    for (int i=0;i<2048;++i) {
        int large=number_range(0,40000000);
        assert(large>=0 && large<=40000000);
        if (large>16777215) upper=TRUE;
        int full=number_range(INT_MIN,INT_MAX);
        if (full<0) negative=TRUE; else positive=TRUE;
        int cross=number_range(-1900000000,900000000);
        assert(cross>=-1900000000 && cross<=900000000);
    }
    assert(upper && negative && positive);
    for (int i=0;i<64;++i) {
        long hp=mobile_hit_points(1000);
        assert(hp>=258000 && hp<=1008000);
        hp=mobile_hit_points(6308);
        assert(hp>=9998180 && hp<=10000000);
    }
    assert(mobile_hit_points(6309)==10000000);
    assert(mobile_hit_points(500003)==10000000);
    assert(mobile_hit_points(INT_MAX)==10000000);
    assert(mobile_hit_points(LONG_MAX)==10000000);
}

int main(void)
{
    random_and_mobile_bounds();
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
