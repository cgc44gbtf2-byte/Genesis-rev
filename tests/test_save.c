/* Synthetic fixtures only; never opens any real player directory or data.
 * cc -std=gnu17 -O1 -g -ffunction-sections -fdata-sections \
 *   tests/test_save.c -Wl,--gc-sections -o /tmp/test_save
 * Add -fsanitize=address,undefined -fno-omit-frame-pointer for sanitizers.
 */
#define ATOMIC_SAVE_TESTING
#include "../Genesis/src/atomic_save.c"
#include "../Genesis/src/save.c"
#include <assert.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/wait.h>

time_t current_time = 12345;
FILE *fpReserve;
const struct skill_type skill_table[MAX_SKILL] = {{0}};
BOARD_DATA boards[MAX_BOARD];
static ROOM_INDEX_DATA test_room;
static unsigned int bugs;
static char last_bug[MAX_STRING_LENGTH];

void bug(const char *text, long param)
{
    (void)param;
    ++bugs;
    snprintf(last_bug, sizeof(last_bug), "%s", text);
}

ROOM_INDEX_DATA *get_room_index(long vnum)
{
    (void)vnum;
    return &test_room;
}

bool str_cmp(const char *a, const char *b)
{
    return strcmp(a, b) != 0;
}

static const char old_data[] =
    "#PLAYER\nName         Fixture~\nLevel        2\nEnd\n\n"
    "#OBJECT\nName         oldtoken~\nEnd\n\n#END\n";
static const char new_data[] =
    "#PLAYER\nName         Fixture~\nLevel        3\nEnd\n\n"
    "#OBJECT\nName         newtoken~\nEnd\n\n#END\n";

static void put_fixture(const char *path, const char *text)
{
    FILE *fp = fopen(path, "w");
    assert(fp != NULL);
    assert(fputs(text, fp) >= 0);
    assert(fclose(fp) == 0);
}

static void expect_fixture(const char *path, const char *text)
{
    char data[16384];
    FILE *fp = fopen(path, "r");
    size_t n;
    assert(fp != NULL);
    n = fread(data, 1, sizeof(data) - 1, fp);
    assert(!ferror(fp) && feof(fp));
    data[n] = '\0';
    assert(fclose(fp) == 0);
    assert(strcmp(data, text) == 0);
}

static int count_temporaries(const char *directory, int remove_files)
{
    DIR *dir = opendir(directory);
    struct dirent *entry;
    int n = 0;
    assert(dir != NULL);
    while ((entry = readdir(dir)) != NULL) {
        if (strncmp(entry->d_name, ".save-", 6) == 0) {
            ++n;
            if (remove_files)
                assert(unlinkat(dirfd(dir), entry->d_name, 0) == 0);
        }
    }
    assert(closedir(dir) == 0);
    return n;
}

static int writer(FILE *fp, void *context)
{
    (void)context;
    return fputs(new_data, fp) < 0 ? -1 : 0;
}

static int partial_writer(FILE *fp, void *context)
{
    (void)context;
    assert(fputs("#PLAYER\nName         Incomplete~\n", fp) >= 0);
    errno = ENOSPC;
    return -1;
}

static int stream_error_writer(FILE *fp, void *context)
{
    int readonly = open("/dev/null", O_RDONLY);
    (void)context;
    assert(readonly >= 0);
    assert(setvbuf(fp, NULL, _IONBF, 0) == 0);
    /* Cause a real stdio write failure, even though callback returns success. */
    assert(dup2(readonly, fileno(fp)) == fileno(fp));
    assert(close(readonly) == 0);
    assert(fputs(new_data, fp) == EOF);
    assert(ferror(fp));
    return 0;
}

static int flush_error_writer(FILE *fp, void *context)
{
    int readonly = open("/dev/null", O_RDONLY);
    (void)context;
    assert(readonly >= 0);
    assert(fputs(new_data, fp) >= 0); /* Still buffered. */
    assert(dup2(readonly, fileno(fp)) == fileno(fp));
    assert(close(readonly) == 0);
    return 0; /* Real fflush failure occurs in production helper. */
}

static int visibility_writer(FILE *fp, void *context)
{
    expect_fixture(context, old_data); /* Old target while writing new temp. */
    return writer(fp, NULL);
}

static int observe_temporary_writer(FILE *fp, void *context)
{
    DIR *dir = opendir("../player");
    struct dirent *entry;
    struct stat st;
    int found = 0;
    assert(dir != NULL);
    while ((entry = readdir(dir)) != NULL) {
        if (strncmp(entry->d_name, ".save-", 6) == 0) {
            const char *counter = strrchr(entry->d_name, '-');
            *(unsigned long *)context = strtoul(counter + 1, NULL, 10);
            assert(fstatat(dirfd(dir), entry->d_name, &st,
                           AT_SYMLINK_NOFOLLOW) == 0);
            assert(S_ISREG(st.st_mode) && (st.st_mode & 0777) == 0600);
            ++found;
        }
    }
    assert(found == 1 && closedir(dir) == 0);
    return writer(fp, NULL);
}

static void test_faults(const char *directory)
{
    struct atomic_save_error error;
    char target[256];
    int existing;
    size_t i;
    const enum atomic_save_fault failures[] = {
        SAVE_FAULT_CREATE, SAVE_FAULT_FLUSH, SAVE_FAULT_SYNC,
        SAVE_FAULT_CLOSE, SAVE_FAULT_RENAME
    };
    const atomic_save_writer failed_writers[] = {
        partial_writer, stream_error_writer, flush_error_writer
    };
    snprintf(target, sizeof(target), "%s/Fixture", directory);
    for (existing = 0; existing <= 1; ++existing) {
        for (i = 0; i < sizeof(failures) / sizeof(failures[0]); ++i) {
            if (existing)
                put_fixture(target, old_data);
            atomic_save_test_fault = failures[i];
            assert(atomic_save(directory, "Fixture", writer, NULL, &error)
                   == ATOMIC_SAVE_FAILED);
            assert(error.stage != NULL && error.number == EIO);
            assert(error.cleanup_number == 0);
            if (existing)
                expect_fixture(target, old_data);
            else
                assert(access(target, F_OK) < 0 && errno == ENOENT);
            assert(count_temporaries(directory, 0) == 0);
        }
        atomic_save_test_fault = SAVE_FAULT_NONE;
        for (i = 0; i < sizeof(failed_writers) / sizeof(failed_writers[0]); ++i) {
            assert(atomic_save(directory, "Fixture", failed_writers[i],
                               NULL, &error) == ATOMIC_SAVE_FAILED);
            assert(error.number != 0 && error.cleanup_number == 0);
            if (existing)
                expect_fixture(target, old_data);
            else
                assert(access(target, F_OK) < 0 && errno == ENOENT);
            assert(count_temporaries(directory, 0) == 0);
        }
    }
    atomic_save_test_fault = SAVE_FAULT_NONE;
    put_fixture(target, old_data);
    assert(atomic_save(directory, "Fixture", partial_writer, NULL, &error)
           == ATOMIC_SAVE_FAILED);
    assert(error.number == ENOSPC);
    expect_fixture(target, old_data);
    assert(atomic_save(directory, "Fixture", stream_error_writer, NULL, &error)
           == ATOMIC_SAVE_FAILED);
    assert(strcmp(error.stage, "serialize") == 0);
    expect_fixture(target, old_data);
    assert(atomic_save(directory, "Fixture", flush_error_writer, NULL, &error)
           == ATOMIC_SAVE_FAILED);
    assert(strcmp(error.stage, "flush") == 0);
    expect_fixture(target, old_data);
    assert(count_temporaries(directory, 0) == 0);

    atomic_save_test_fault = SAVE_FAULT_CLEANUP;
    assert(atomic_save(directory, "Fixture", partial_writer, NULL, &error)
           == ATOMIC_SAVE_FAILED);
    assert(error.number == ENOSPC && error.cleanup_number == EIO);
    expect_fixture(target, old_data);
    assert(count_temporaries(directory, 1) == 1);

    atomic_save_test_fault = SAVE_FAULT_NONE;
    assert(atomic_save(directory, "Fixture", visibility_writer, target, &error)
           == ATOMIC_SAVE_OK);
    expect_fixture(target, new_data);
    {
        struct stat st;
        assert(stat(target, &st) == 0 && (st.st_mode & 0777) == 0600);
    }
    put_fixture(target, old_data);
    atomic_save_test_fault = SAVE_FAULT_DIRSYNC;
    assert(atomic_save(directory, "Fixture", writer, NULL, &error)
           == ATOMIC_SAVE_UNCERTAIN);
    assert(error.number == EIO && strstr(error.stage, "uncertain"));
    expect_fixture(target, new_data);
    assert(count_temporaries(directory, 0) == 0);
    assert(unlink(target) == 0);
    atomic_save_test_fault = SAVE_FAULT_NONE;
    assert(atomic_save(directory, "Fixture", writer, NULL, &error)
           == ATOMIC_SAVE_OK);
    expect_fixture(target, new_data);
    assert(unlink(target) == 0);
    atomic_save_test_fault = SAVE_FAULT_DIRSYNC;
    assert(atomic_save(directory, "Fixture", writer, NULL, &error)
           == ATOMIC_SAVE_UNCERTAIN);
    expect_fixture(target, new_data);
    assert(unlink(target) == 0);
    atomic_save_test_fault = SAVE_FAULT_NONE;
}

static int crash_writer(FILE *fp, void *context)
{
    (void)context;
    assert(fputs("#PLAYER\nIncomplete\n", fp) >= 0);
    assert(fflush(fp) == 0);
    _exit(0); /* Simulated process loss, no application cleanup. */
}

static void test_process_loss(void)
{
    int existing, status;
    pid_t child;
    for (existing = 0; existing <= 1; ++existing) {
        if (existing)
            put_fixture("../player/Fixture", old_data);
        child = fork();
        assert(child >= 0);
        if (child == 0) {
            struct atomic_save_error error;
            atomic_save("../player", "Fixture", crash_writer, NULL, &error);
            _exit(1);
        }
        assert(waitpid(child, &status, 0) == child);
        assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
        if (existing) {
            expect_fixture("../player/Fixture", old_data);
            assert(unlink("../player/Fixture") == 0);
        } else {
            assert(access("../player/Fixture", F_OK) < 0 && errno == ENOENT);
        }
        /* Abrupt loss can leave an orphan; it is never a loadable player
         * filename. A subsequent save must succeed without reusing it. */
        assert(count_temporaries("../player", 0) == 1);
        {
            struct atomic_save_error error;
            assert(atomic_save("../player", "Fixture", writer, NULL, &error)
                   == ATOMIC_SAVE_OK);
        }
        expect_fixture("../player/Fixture", new_data);
        assert(unlink("../player/Fixture") == 0);
        assert(count_temporaries("../player", 1) == 1);
    }
}

static void test_security(void)
{
    struct atomic_save_error error;
    const char *invalid[] = {"", ".", "..", "../Escape", "A/B", "/tmp/X",
                            "A\\B", "A\nB", "A B", "A1", "\xff"};
    char overlong[257];
    char collision[128];
    size_t i;
    unsigned long next, counter = 0;
    assert(!atomic_save_name_valid(NULL));
    memset(overlong, 'A', sizeof(overlong) - 1);
    overlong[sizeof(overlong) - 1] = '\0';
    assert(!atomic_save_name_valid(overlong));
    for (i = 0; i < sizeof(invalid) / sizeof(invalid[0]); ++i)
        assert(atomic_save("../player", invalid[i], writer, NULL, &error)
               == ATOMIC_SAVE_FAILED);
    assert(atomic_save("../missing", "Fixture", writer, NULL, &error)
           == ATOMIC_SAVE_FAILED);
    put_fixture("../outside", old_data);
    assert(symlink("../outside", "../player/Fixture") == 0);
    assert(atomic_save("../player", "Fixture", writer, NULL, &error)
           == ATOMIC_SAVE_FAILED);
    expect_fixture("../outside", old_data);
    assert(unlink("../player/Fixture") == 0);
    assert(mkdir("../player/Fixture", 0700) == 0);
    assert(atomic_save("../player", "Fixture", writer, NULL, &error)
           == ATOMIC_SAVE_FAILED);
    assert(rmdir("../player/Fixture") == 0);
    assert(symlink("player", "../alias") == 0);
    assert(atomic_save("../alias/", "Fixture", writer, NULL, &error)
           == ATOMIC_SAVE_FAILED);
    assert(unlink("../alias") == 0);
    assert(chmod("../player", 0777) == 0);
    assert(atomic_save("../player", "Fixture", writer, NULL, &error)
           == ATOMIC_SAVE_FAILED);
    assert(error.number == EPERM);
    assert(chmod("../player", 0700) == 0);

    /* Occupy all prospective temp names with symlinks: none may be followed
     * or removed, and the helper must fail after its bounded retry limit. */
    assert(atomic_save("../player", "Fixture", observe_temporary_writer,
                       &counter, &error) == ATOMIC_SAVE_OK);
    assert(unlink("../player/Fixture") == 0);
    for (next = 1; next <= 128; ++next) {
        snprintf(collision, sizeof(collision), "../player/.save-%ld-%lu",
                 (long)getpid(), counter + next);
        assert(symlink("../outside", collision) == 0);
    }
    assert(atomic_save("../player", "Fixture", writer, NULL, &error)
           == ATOMIC_SAVE_FAILED);
    assert(error.number == EEXIST);
    assert(count_temporaries("../player", 0) == 128);
    expect_fixture("../outside", old_data);
    for (next = 1; next <= 128; ++next) {
        struct stat st;
        snprintf(collision, sizeof(collision), "../player/.save-%ld-%lu",
                 (long)getpid(), counter + next);
        assert(lstat(collision, &st) == 0 && S_ISLNK(st.st_mode));
        assert(unlink(collision) == 0);
    }
    assert(unlink("../outside") == 0);
    assert(count_temporaries("../player", 0) == 0);
}

static void test_game_saves(void)
{
    CHAR_DATA ch = {0}, switched = {0};
    PC_DATA pc = {0};
    DESCRIPTOR_DATA desc = {0};
    OBJ_DATA obj = {0};
    OBJ_DATA nested = {0}, sibling = {0};
    OBJ_INDEX_DATA index = {0};
    FILE *fp;
    char expected[16384];
    size_t n;
    int i;
    unsigned int before;
    ch.name = "fIXTURE";
    ch.short_descr = ch.long_descr = ch.description = ch.lord = "";
    ch.clan = ch.morph = ch.createtime = ch.lasttime = "";
    ch.lasthost = ch.prompt = ch.cprompt = ch.contingency = "";
    ch.pcdata = &pc;
    pc.pwd = pc.bamfin = pc.bamfout = pc.cdest = pc.title = "";
    pc.decap_char = pc.marriage = pc.email = pc.conding = pc.love = "";
    pc.covenant = pc.cov_time = pc.leader = pc.vleader = "";
    for (i = 0; i < MAX_BOARD; ++i)
        boards[i].short_name = "synthetic";
    ch.in_room = &test_room;
    ch.level = 2;
    obj.name = "synthetic token";
    obj.short_descr = obj.description = "";
    obj.wear_loc = WEAR_NONE;
    obj.pIndexData = &index;
    nested = sibling = obj;
    nested.name = "synthetic nested token";
    sibling.name = "synthetic sibling token";
    obj.contains = &nested;
    obj.next_content = &sibling;
    ch.carrying = &obj;
    ch.save_time = current_time;
    fp = tmpfile();
    assert(fp != NULL);
    assert(write_player_save(fp, &ch) == 0);
    rewind(fp);
    n = fread(expected, 1, sizeof(expected) - 1, fp);
    assert(feof(fp) && !ferror(fp));
    expected[n] = '\0';
    assert(fclose(fp) == 0);
    assert(strstr(expected, "#PLAYER\n") == expected);
    assert(strstr(expected, "#OBJECT\n") != NULL);
    assert(strstr(expected, "synthetic nested token") != NULL);
    assert(strstr(expected, "synthetic sibling token") != NULL);
    assert(strstr(expected, "Nest         1\n") != NULL);
    assert(n >= 5 && strcmp(expected + n - 5, "#END\n") == 0);

    fpReserve = fopen(NULL_FILE, "r");
    assert(fpReserve);
    save_char_obj(&ch);
    expect_fixture("../player/Fixture", expected);
    save_char_obj_backup(&ch);
    expect_fixture("../backup/Fixture", expected);
    assert(fpReserve != NULL && ch.save_time == current_time);
    before = bugs;
    atomic_save_test_fault = SAVE_FAULT_RENAME;
    save_char_obj(&ch);
    save_char_obj_backup(&ch);
    assert(bugs == before + 2);
    expect_fixture("../player/Fixture", expected);
    expect_fixture("../backup/Fixture", expected);
    atomic_save_test_fault = SAVE_FAULT_DIRSYNC;
    save_char_obj(&ch);
    assert(strstr(last_bug, "uncertain") != NULL);
    atomic_save_test_fault = SAVE_FAULT_NONE;
    assert(unlink("../player/Fixture") == 0);
    assert(unlink("../backup/Fixture") == 0);
    ch.level = 1;
    save_char_obj(&ch);
    assert(access("../player/Fixture", F_OK) < 0);
    ch.level = 2;
    ch.act = ACT_IS_NPC;
    save_char_obj(&ch);
    assert(access("../player/Fixture", F_OK) < 0);
    ch.act = 0;
    ch.desc = &desc;
    desc.connected = CON_GET_NAME;
    save_char_obj_backup(&ch);
    assert(access("../backup/Fixture", F_OK) < 0);
    desc.connected = CON_PLAYING;
    switched = ch;
    desc.original = &ch;
    switched.name = "Notoriginal";
    save_char_obj(&switched);
    expect_fixture("../player/Fixture", expected);
    assert(access("../player/Notoriginal", F_OK) < 0);
    assert(unlink("../player/Fixture") == 0);
    ch.name = "../Escape";
    save_char_obj(&ch);
    assert(strstr(last_bug, "invalid player filename"));
    assert(fclose(fpReserve) == 0);
    fpReserve = NULL;
}

int main(void)
{
    char root[] = "/tmp/genesis-save-test-XXXXXX";
    int original = open(".", O_RDONLY | O_DIRECTORY);
    int probe, next_probe;
    assert(original >= 0);
    assert(mkdtemp(root) != NULL);
    assert(chdir(root) == 0);
    assert(mkdir("player", 0700) == 0);
    assert(mkdir("backup", 0700) == 0);
    assert(mkdir("run", 0700) == 0);
    assert(chdir("run") == 0);
    probe = dup(STDERR_FILENO);
    assert(probe >= 0 && close(probe) == 0);
    test_faults("../player");
    test_faults("../backup");
    test_security();
    test_process_loss();
    test_game_saves();
    next_probe = dup(STDERR_FILENO);
    assert(next_probe == probe && close(next_probe) == 0);
    assert(count_temporaries("../player", 0) == 0);
    assert(count_temporaries("../backup", 0) == 0);
    assert(chdir("..") == 0);
    assert(rmdir("player") == 0 && rmdir("backup") == 0);
    assert(rmdir("run") == 0);
    assert(fchdir(original) == 0 && close(original) == 0);
    assert(rmdir(root) == 0);
    puts("save tests passed (synthetic fixtures only)");
    return 0;
}
