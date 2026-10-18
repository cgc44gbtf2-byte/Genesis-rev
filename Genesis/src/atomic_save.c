#define _POSIX_C_SOURCE 200809L
#include "atomic_save.h"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef ATOMIC_SAVE_TESTING
enum atomic_save_fault atomic_save_test_fault;
#define FAULT(which) (atomic_save_test_fault == (which) ? (errno = EIO, 1) : 0)
#else
#define FAULT(which) 0
#endif

int atomic_save_name_valid(const char *name)
{
    size_t n;
    if (name == NULL || name[0] == '\0')
        return 0;
    for (n = 0; name[n] != '\0'; ++n) {
        unsigned char c = (unsigned char)name[n];
        if (n >= 255 || !((c >= 'A' && c <= 'Z') ||
                         (c >= 'a' && c <= 'z')))
            return 0;
    }
    return 1;
}

static enum atomic_save_result atomic_save_named(const char *directory, const char *name,
                                    atomic_save_writer writer, void *context,
                                    struct atomic_save_error *error, int valid_name)
{
    static unsigned long sequence;
    struct atomic_save_error local_error = { NULL, 0, 0 };
    struct stat st;
    int dirfd = -1, fd = -1, saved_errno, attempt, temp_exists = 0;
    FILE *stream = NULL;
    char temporary[96];
    char directory_path[PATH_MAX];
    size_t directory_length;
    const char *stage = "arguments";
    enum atomic_save_result result = ATOMIC_SAVE_FAILED;

    if (error == NULL)
        error = &local_error;
    *error = local_error;
    if (directory == NULL || directory[0] == '\0' ||
        !valid_name || writer == NULL) {
        errno = EINVAL;
        goto failed;
    }
    directory_length = strlen(directory);
    if (directory_length >= sizeof(directory_path)) {
        errno = ENAMETOOLONG;
        goto failed;
    }
    memcpy(directory_path, directory, directory_length + 1);
    /* A trailing slash would make O_NOFOLLOW follow the final symlink. */
    while (directory_length > 1 && directory_path[directory_length - 1] == '/')
        directory_path[--directory_length] = '\0';
    stage = "open directory";
    dirfd = open(directory_path, O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
    if (dirfd < 0)
        goto failed;
    stage = "directory permissions";
    if (fstat(dirfd, &st) < 0)
        goto failed;
    if (st.st_uid != geteuid() || (st.st_mode & (S_IWGRP | S_IWOTH))) {
        errno = EPERM;
        goto failed;
    }
    stage = "destination type";
    if (fstatat(dirfd, name, &st, AT_SYMLINK_NOFOLLOW) == 0) {
        if (!S_ISREG(st.st_mode)) {
            errno = EINVAL;
            goto failed;
        }
    } else if (errno != ENOENT) {
        goto failed;
    }
    stage = "create temporary";
    /* O_EXCL, not secrecy of the name, prevents symlink/hardlink attacks.
     * All operations stay anchored to the checked directory descriptor.
     * Collisions never cause an existing entry to be opened or removed. */
    for (attempt = 0; attempt < 128; ++attempt) {
        snprintf(temporary, sizeof(temporary), ".save-%ld-%lu",
                 (long)getpid(), ++sequence);
        if (FAULT(SAVE_FAULT_CREATE))
            goto failed;
        fd = openat(dirfd, temporary,
                    O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0600);
        if (fd >= 0)
            break;
        if (errno != EEXIST)
            goto failed;
    }
    if (fd < 0)
        goto failed;
    temp_exists = 1;
    stage = "temporary permissions";
    if (fchmod(fd, 0600) < 0)
        goto failed;
    stage = "open stream";
    stream = fdopen(fd, "w");
    if (stream == NULL)
        goto failed;
    fd = -1; /* stream owns it now */
    stage = "serialize";
    errno = 0;
    if (writer(stream, context) != 0 || ferror(stream)) {
        if (errno == 0)
            errno = EIO;
        goto failed;
    }
    stage = "flush";
    if (FAULT(SAVE_FAULT_FLUSH) || fflush(stream) != 0 || ferror(stream))
        goto failed;
    stage = "file fsync";
    if (FAULT(SAVE_FAULT_SYNC) || fsync(fileno(stream)) < 0)
        goto failed;
    stage = "close";
    /* Even on error fclose consumes the FILE; never retry it. */
    saved_errno = fclose(stream);
    stream = NULL;
    if (FAULT(SAVE_FAULT_CLOSE) || saved_errno != 0)
        goto failed;
    stage = "rename";
    if (FAULT(SAVE_FAULT_RENAME) ||
        renameat(dirfd, temporary, dirfd, name) < 0)
        goto failed;
    temp_exists = 0;
    result = ATOMIC_SAVE_OK;
    stage = "directory fsync (new save visible; durability uncertain)";
    if (FAULT(SAVE_FAULT_DIRSYNC) || fsync(dirfd) < 0) {
        result = ATOMIC_SAVE_UNCERTAIN;
        error->stage = stage;
        error->number = errno ? errno : EIO;
    }
    close(dirfd);
    return result;

failed:
    saved_errno = errno ? errno : EIO;
    error->stage = stage;
    error->number = saved_errno;
    if (stream != NULL)
        fclose(stream);
    if (fd >= 0)
        close(fd);
    if (temp_exists &&
        (FAULT(SAVE_FAULT_CLEANUP) || unlinkat(dirfd, temporary, 0) < 0))
        error->cleanup_number = errno ? errno : EIO;
    if (dirfd >= 0)
        close(dirfd);
    errno = saved_errno;
    return result;
}

enum atomic_save_result atomic_save(const char *directory, const char *name,
    atomic_save_writer writer, void *context, struct atomic_save_error *error)
{
    return atomic_save_named(directory,name,writer,context,error,
                             atomic_save_name_valid(name));
}

enum atomic_save_result atomic_save_copyover(const char *directory,
    atomic_save_writer writer, void *context, struct atomic_save_error *error)
{
    return atomic_save_named(directory,"copyover.data",writer,context,error,1);
}
