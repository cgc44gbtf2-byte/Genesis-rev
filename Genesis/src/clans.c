/* Shared class-clan governance. Class power/generation fields are not roles.
 * The registry is authoritative; player-file clan strings are reconciled
 * caches. One atomic registry update covers offline members and transfers.
 */
#define _DEFAULT_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "atomic_save.h"

#define CLAN_LIMIT 64
#define MEMBER_LIMIT 1024
#define INVITE_LIMIT 256
#define NAME_LIMIT 12
#define LABEL_LIMIT 25
#define CLAN_DIRECTORY "../clans"
#define MEMBER_ROLE 1
#define OFFICER_ROLE 2
#define LEADER_ROLE 3

struct clan_entry { long id, class_id; char label[LABEL_LIMIT + 1]; };
struct clan_member {
    long id, class_id, role;
    char name[NAME_LIMIT + 1];
};
struct clan_registry {
    long next_id;
    struct clan_entry clans[CLAN_LIMIT];
    struct clan_member members[MEMBER_LIMIT];
};
struct clan_invite {
    long id, class_id, transfer;
    time_t created;
    char from[NAME_LIMIT + 1], to[NAME_LIMIT + 1];
};
static struct clan_registry registry;
static struct clan_invite invites[INVITE_LIMIT];
static bool loaded, healthy, uncertain;
static bool fresh_lock;
static int lock_fd = -1;
static dev_t lock_device;
static ino_t lock_directory;
static int write_registry(FILE *fp, void *context);

static bool supported(long id)
{
    switch (id) {
    case CLASS_VAMPIRE: case CLASS_WEREWOLF: case CLASS_SORCERER:
    case CLASS_DROW: case CLASS_HIGHLANDER: case CLASS_MAGE:
    case CLASS_DRAGON: case CLASS_NINJA: case CLASS_MONK: case CLASS_ELEM:
        return TRUE;
    default: return FALSE;
    }
}

static bool canonical_name(const char *src, char out[NAME_LIMIT + 1])
{
    size_t n;
    if (src == NULL || (n = strlen(src)) < 1 || n > NAME_LIMIT) return FALSE;
    for (size_t i = 0; i < n; ++i) {
        unsigned char c = src[i];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')))
            return FALSE;
        out[i] = i == 0 ? UPPER(c) : LOWER(c);
    }
    out[n] = '\0';
    return TRUE;
}

static bool valid_label(const char *s)
{
    size_t n = s == NULL ? 0 : strlen(s);
    if (n < 2 || n > LABEL_LIMIT || s[0] == ' ' || s[n-1] == ' ') return FALSE;
    for (size_t i = 0; i < n; ++i) {
        unsigned char c = s[i];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == ' ' || c == '-' || c == '\''))
            return FALSE;
        if (c == ' ' && i > 0 && s[i-1] == ' ') return FALSE;
    }
    return TRUE;
}

static int member_index(const struct clan_registry *r, const char *name)
{
    if (name == NULL || !*name) return -1;
    for (int i = 0; i < MEMBER_LIMIT; ++i)
        if (r->members[i].name[0] && !str_cmp(name, r->members[i].name)) return i;
    return -1;
}

static int clan_index(const struct clan_registry *r, long id)
{
    if (id <= 0) return -1;
    for (int i = 0; i < CLAN_LIMIT; ++i)
        if (r->clans[i].id == id) return i;
    return -1;
}

static bool valid_registry(const struct clan_registry *r)
{
    long highest = 0;
    char name[NAME_LIMIT + 1];
    if (r->next_id < 1) return FALSE;
    for (int i = 0; i < CLAN_LIMIT; ++i) {
        const struct clan_entry *c = &r->clans[i];
        int leaders = 0;
        if (!c->id) continue;
        if (c->id < 1 || !supported(c->class_id) || !valid_label(c->label))
            return FALSE;
        if (c->id > highest) highest = c->id;
        for (int j = i + 1; j < CLAN_LIMIT; ++j)
            if (r->clans[j].id && (r->clans[j].id == c->id ||
                (r->clans[j].class_id == c->class_id &&
                 !str_cmp(r->clans[j].label, c->label)))) return FALSE;
        for (int j = 0; j < MEMBER_LIMIT; ++j)
            if (r->members[j].id == c->id &&
                r->members[j].role == LEADER_ROLE) ++leaders;
        if (leaders != 1) return FALSE;
    }
    if (r->next_id <= highest) return FALSE;
    for (int i = 0; i < MEMBER_LIMIT; ++i) {
        const struct clan_member *m = &r->members[i];
        int c;
        if (!m->name[0]) continue;
        if (!canonical_name(m->name, name) || strcmp(name, m->name) ||
            !supported(m->class_id)) return FALSE;
        if (m->id == 0) {
            if (m->role != 0) return FALSE; /* Persistent removal tombstone. */
        } else {
            c = clan_index(r, m->id);
            if (c < 0 || r->clans[c].class_id != m->class_id ||
                m->role < MEMBER_ROLE || m->role > LEADER_ROLE) return FALSE;
        }
        for (int j = i + 1; j < MEMBER_LIMIT; ++j)
            if (r->members[j].name[0] &&
                !str_cmp(m->name, r->members[j].name)) return FALSE;
    }
    return TRUE;
}

static bool acquire_lock(int directory, const struct stat *ds)
{
    struct stat st, path;
    if (lock_fd >= 0) {
        return ds->st_dev == lock_device && ds->st_ino == lock_directory &&
            fstat(lock_fd, &st) == 0 &&
            fstatat(directory, "Lock", &path, AT_SYMLINK_NOFOLLOW) == 0 &&
            st.st_dev == path.st_dev && st.st_ino == path.st_ino;
    }
    lock_fd = openat(directory, "Lock",
                    O_RDWR | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0600);
    fresh_lock = lock_fd >= 0;
    if (lock_fd < 0 && errno == EEXIST)
        lock_fd = openat(directory, "Lock", O_RDWR | O_NOFOLLOW | O_CLOEXEC);
    if (lock_fd < 0) return FALSE;
    if (fstat(lock_fd, &st) != 0 || !S_ISREG(st.st_mode) ||
        st.st_uid != geteuid() || (st.st_mode & 0022) ||
        st.st_nlink != 1 || flock(lock_fd, LOCK_EX | LOCK_NB) != 0) {
        close(lock_fd); lock_fd = -1; return FALSE;
    }
    lock_device = ds->st_dev;
    lock_directory = ds->st_ino;
    return TRUE;
}

static int open_directory(bool create)
{
    int fd;
    struct stat st;
    if (create) {
        if (mkdir(CLAN_DIRECTORY, 0700) == 0) {
            int parent = open("..", O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
            if (parent < 0) return -1;
            int result = fsync(parent), error = errno;
            close(parent);
            if (result != 0) { errno = error; return -1; }
        } else if (errno != EEXIST) return -1;
    }
    fd = open(CLAN_DIRECTORY, O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
    if (fd < 0) return -1;
    if (fstat(fd, &st) != 0 || st.st_uid != geteuid() ||
        (st.st_mode & 0022) || !acquire_lock(fd, &st)) {
        close(fd); errno = EACCES; return -1;
    }
    return fd;
}

static bool number(const char *s, long *value)
{
    char *end;
    if (s == NULL || !*s) return FALSE;
    for (const char *p = s; *p; ++p) if (*p < '0' || *p > '9') return FALSE;
    errno = 0;
    *value = strtol(s, &end, 10);
    return errno != ERANGE && *end == '\0';
}

static bool decode_label(const char *hex, char out[LABEL_LIMIT + 1])
{
    size_t n = strlen(hex);
    if (n == 0 || n % 2 || n > LABEL_LIMIT * 2) return FALSE;
    for (size_t i = 0; i < n; i += 2) {
        unsigned value = 0;
        for (int j = 0; j < 2; ++j) {
            char c = hex[i+j];
            if (c >= '0' && c <= '9') value = value * 16 + c - '0';
            else if (c >= 'a' && c <= 'f') value = value * 16 + c - 'a' + 10;
            else return FALSE;
        }
        if (value == 0) return FALSE;
        out[i/2] = (char)value;
    }
    out[n/2] = '\0';
    return valid_label(out);
}

void clan_registry_close(void)
{
    if (lock_fd >= 0) close(lock_fd);
    lock_fd = -1;
    loaded = healthy = uncertain = FALSE;
    fresh_lock = FALSE;
    memset(&registry, 0, sizeof(registry));
    memset(invites, 0, sizeof(invites));
}

bool clan_load_registry(void)
{
    struct clan_registry candidate = {0};
    struct stat st;
    FILE *fp = NULL;
    int dir, fd, nc = 0, nm = 0, line_no = 0;
    bool ended = FALSE, good = TRUE;
    char line[256];
    loaded = TRUE; healthy = FALSE; uncertain = FALSE;
    memset(invites, 0, sizeof(invites));
    candidate.next_id = 1;
    dir = open_directory(FALSE);
    if (dir < 0 && errno == ENOENT && lock_fd < 0) {
        registry = candidate; healthy = TRUE; return TRUE;
    }
    if (dir < 0) goto failed;
    fd = openat(dir, "Registry", O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
    close(dir);
    if (fd < 0 && errno == ENOENT && fresh_lock) {
        struct atomic_save_error error;
        enum atomic_save_result result = atomic_save(CLAN_DIRECTORY, "Registry",
            write_registry, &candidate, &error);
        if (result == ATOMIC_SAVE_FAILED) goto failed;
        registry = candidate; healthy = TRUE; fresh_lock = FALSE;
        uncertain = result == ATOMIC_SAVE_UNCERTAIN;
        return TRUE;
    }
    if (fd < 0) goto failed;
    if (fstat(fd, &st) != 0 || !S_ISREG(st.st_mode) || st.st_uid != geteuid() ||
        (st.st_mode & 0022) || st.st_nlink != 1 || st.st_size > 262144) {
        close(fd); goto failed;
    }
    fp = fdopen(fd, "r");
    if (fp == NULL) { close(fd); goto failed; }
    while (fgets(line, sizeof(line), fp) != NULL) {
        char *words[7], *save = NULL, *w;
        int n = 0;
        if (strchr(line, '\n') == NULL) { good = FALSE; break; }
        w = strtok_r(line, " \t\r\n", &save);
        while (w != NULL && n < 7) { words[n++] = w; w = strtok_r(NULL, " \t\r\n", &save); }
        if (n == 0) continue;
        if (ended || w != NULL) { good = FALSE; break; }
        if (line_no++ == 0) {
            if (n != 3 || strcmp(words[0], "AnathemaClans") ||
                strcmp(words[1], "1") || !number(words[2], &candidate.next_id))
                { good = FALSE; break; }
        } else if (!strcmp(words[0], "C") && n == 4 && nc < CLAN_LIMIT) {
            struct clan_entry *c = &candidate.clans[nc++];
            if (!number(words[1], &c->id) || c->id < 1 ||
                !number(words[2], &c->class_id) || !decode_label(words[3], c->label))
                { good = FALSE; break; }
        } else if (!strcmp(words[0], "M") && n == 5 && nm < MEMBER_LIMIT) {
            struct clan_member *m = &candidate.members[nm++];
            if (!number(words[1], &m->id) || !number(words[2], &m->class_id) ||
                !number(words[3], &m->role) ||
                !canonical_name(words[4], m->name) || strcmp(words[4], m->name))
                { good = FALSE; break; }
        } else if (!strcmp(words[0], "End") && n == 1) ended = TRUE;
        else { good = FALSE; break; }
    }
    if (ferror(fp)) good = FALSE;
    if (fclose(fp) != 0) good = FALSE;
    fp = NULL;
    if (!good || !ended || !valid_registry(&candidate)) goto failed;
    registry = candidate; healthy = TRUE; return TRUE;
failed:
    if (fp != NULL) fclose(fp);
    bug("Clan registry unavailable or invalid; governance disabled", 0);
    return FALSE;
}

static bool available(CHAR_DATA *ch, bool writing)
{
    if (!loaded) clan_load_registry();
    if (!healthy || (writing && uncertain)) {
        stc("Clan governance is unavailable; ask staff to check the registry.\n\r", ch);
        return FALSE;
    }
    return TRUE;
}

static bool player_ready(CHAR_DATA *ch)
{
    char name[NAME_LIMIT + 1];
    return ch != NULL && !IS_NPC(ch) && ch->pcdata != NULL &&
        supported(ch->class) && ch->level >= LEV_AWA &&
        canonical_name(ch->name, name) && ch->desc != NULL &&
        ch->desc->original == NULL && ch->desc->connected == CON_PLAYING &&
        ch->position >= POS_STANDING && ch->in_room != NULL &&
        ch->fighting == NULL && ch->combat_pause == 0 && ch->wait == 0;
}

void clan_sync_player(CHAR_DATA *ch)
{
    int m, c;
    const char *label;
    if (ch == NULL || IS_NPC(ch) || ch->pcdata == NULL || !supported(ch->class))
        return;
    if (!loaded) clan_load_registry();
    if (!healthy) return; /* Never grant permissions from cached clan strings. */
    m = member_index(&registry, ch->name);
    if (m < 0 || registry.members[m].class_id != ch->class) return;
    c = clan_index(&registry, registry.members[m].id);
    label = c < 0 ? "" : registry.clans[c].label;
    if (ch->clan == NULL || strcmp(ch->clan, label)) {
        free_string(ch->clan);
        ch->clan = str_dup(label);
    }
}

static void sync_online(void)
{
    for (CHAR_DATA *p = char_list; p != NULL; p = p->next) clan_sync_player(p);
}

static int write_registry(FILE *fp, void *context)
{
    const struct clan_registry *r = context;
    static const char hex[] = "0123456789abcdef";
    fprintf(fp, "AnathemaClans 1 %ld\n", r->next_id);
    for (int i = 0; i < CLAN_LIMIT; ++i) if (r->clans[i].id) {
        const struct clan_entry *c = &r->clans[i];
        fprintf(fp, "C %ld %ld ", c->id, c->class_id);
        for (const unsigned char *p = (const unsigned char *)c->label; *p; ++p) {
            fputc(hex[*p >> 4], fp); fputc(hex[*p & 15], fp);
        }
        fputc('\n', fp);
    }
    for (int i = 0; i < MEMBER_LIMIT; ++i) if (r->members[i].name[0]) {
        const struct clan_member *m = &r->members[i];
        fprintf(fp, "M %ld %ld %ld %s\n", m->id, m->class_id, m->role, m->name);
    }
    return fprintf(fp, "End\n") < 0 || ferror(fp) ? -1 : 0;
}

static bool commit_registry(CHAR_DATA *ch, const struct clan_registry *next)
{
    struct atomic_save_error error;
    enum atomic_save_result result;
    int directory;
    if (!valid_registry(next)) {
        bug("Clan update rejected: invalid proposed registry", 0);
        stc("Clan update rejected; no changes saved.\n\r", ch); return FALSE;
    }
    directory = open_directory(TRUE);
    if (directory < 0) {
        stc("Clan update could not be saved; no changes applied.\n\r", ch);
        return FALSE;
    }
    close(directory);
    result = atomic_save(CLAN_DIRECTORY, "Registry", write_registry,
                         (void *)next, &error);
    if (result == ATOMIC_SAVE_FAILED) {
        bug("Clan registry save failed; previous state retained", 0);
        stc("Clan update could not be saved; no changes applied.\n\r", ch);
        return FALSE;
    }
    registry = *next;
    fresh_lock = FALSE;
    uncertain = result == ATOMIC_SAVE_UNCERTAIN;
    sync_online();
    if (uncertain) {
        bug("Clan registry visible but durability uncertain; writes frozen", 0);
        stc("Update visible, durability uncertain. Staff must use clanadmin retry.\n\r", ch);
    } else stc("Clan update saved.\n\r", ch);
    return TRUE;
}

bool clan_block_class_change(CHAR_DATA *ch)
{
    int m;
    if (ch == NULL || IS_NPC(ch)) return FALSE;
    if (!loaded) clan_load_registry();
    if (!healthy || uncertain) {
        stc("Class changes blocked while clan governance is unavailable.\n\r", ch);
        return TRUE;
    }
    m = member_index(&registry, ch->name);
    if (m >= 0 && registry.members[m].id != 0) {
        stc("Leave the managed clan first; leaders must transfer or disband it.\n\r", ch);
        return TRUE;
    }
    return FALSE;
}

bool clan_identity_reserved(const char *name)
{
    int m;
    if (!loaded) clan_load_registry();
    if (!healthy || uncertain) return TRUE;
    m = member_index(&registry, name);
    return m >= 0 && registry.members[m].id != 0;
}

bool clan_block_name_change(CHAR_DATA *ch, const char *label)
{
    if (!loaded) clan_load_registry();
    if (!healthy || uncertain) return TRUE;
    if (member_index(&registry, ch->name) >= 0) return TRUE;
    for (int i = 0; i < CLAN_LIMIT; ++i)
        if (registry.clans[i].id && registry.clans[i].class_id == ch->class &&
            !str_cmp(registry.clans[i].label, label)) return TRUE;
    return FALSE;
}

static int active_member(CHAR_DATA *ch)
{
    int m = member_index(&registry, ch->name);
    return m >= 0 && registry.members[m].id != 0 &&
        registry.members[m].class_id == ch->class ? m : -1;
}

static const char *role_name(long role)
{
    return role == LEADER_ROLE ? "Leader" : role == OFFICER_ROLE ? "Officer" : "Member";
}

static bool together(CHAR_DATA *a, CHAR_DATA *b)
{
    const long planes = AFF_ETHEREAL | AFF_DEEPUMBRA | AFF_SHADOWPLANE;
    return player_ready(a) && player_ready(b) && a != b &&
        a->class == b->class && a->in_room == b->in_room &&
        (a->affected_by & planes) == (b->affected_by & planes) &&
        can_see(a, b) && can_see(b, a);
}

static void offer(CHAR_DATA *ch, int m, const char *name, bool transfer)
{
    CHAR_DATA *target = get_char_room(ch, (char *)name);
    int t, slot = -1, c = clan_index(&registry, registry.members[m].id);
    char who[NAME_LIMIT + 1], from[NAME_LIMIT + 1], buf[MAX_STRING_LENGTH];
    if (registry.members[m].role < (transfer ? LEADER_ROLE : OFFICER_ROLE) ||
        !together(ch, target) || !canonical_name(target->name, who) ||
        !canonical_name(ch->name, from)) {
        stc("You lack authority, or the eligible player is not here and available.\n\r", ch);
        return;
    }
    t = member_index(&registry, who);
    if (transfer ? (t < 0 || registry.members[t].id != registry.members[m].id ||
                   registry.members[t].class_id != ch->class) :
        ((t >= 0 && registry.members[t].id != 0) ||
         (target->clan != NULL && target->clan[0] &&
          str_cmp(target->clan, registry.clans[c].label)))) {
        stc("That player is not eligible for this offer.\n\r", ch); return;
    }
    for (int i = 0; i < INVITE_LIMIT; ++i) {
        if (!str_cmp(invites[i].to, who)) { slot = i; break; }
        if (slot < 0 && (invites[i].id == 0 ||
            current_time < invites[i].created ||
            difftime(current_time, invites[i].created) > 120)) slot = i;
    }
    if (slot < 0) { stc("Too many pending clan offers; try later.\n\r", ch); return; }
    struct clan_invite *p = &invites[slot];
    memset(p, 0, sizeof(*p));
    p->id = registry.members[m].id; p->class_id = ch->class;
    p->created = current_time; p->transfer = transfer;
    strcpy(p->from, from); strcpy(p->to, who);
    snprintf(buf, sizeof(buf), "%s offers %s in %s. Type 'clan accept %s' within 120 seconds.\n\r",
             from, transfer ? "leadership" : "membership", registry.clans[c].label, from);
    stc(buf, target);
    stc("Named clan offer sent; nothing changes until they accept.\n\r", ch);
}

static void accept_offer(CHAR_DATA *ch, const char *from)
{
    struct clan_registry next = registry;
    struct clan_invite *p = NULL;
    CHAR_DATA *leader;
    int m, t, c;
    for (int i = 0; i < INVITE_LIMIT; ++i)
        if (invites[i].id && !str_cmp(invites[i].to, ch->name) &&
            !str_cmp(invites[i].from, from)) { p = &invites[i]; break; }
    if (p == NULL || current_time < p->created ||
        difftime(current_time, p->created) > 120 ||
        p->class_id != ch->class || (c = clan_index(&registry, p->id)) < 0 ||
        (m = member_index(&registry, p->from)) < 0 ||
        registry.members[m].id != p->id ||
        registry.members[m].role < (p->transfer ? LEADER_ROLE : OFFICER_ROLE) ||
        (leader = get_char_room(ch, p->from)) == NULL || !together(ch, leader)) {
        stc("No valid offer from that player. Ask for a new one.\n\r", ch); return;
    }
    t = member_index(&registry, ch->name);
    if (p->transfer) {
        if (t < 0 || registry.members[t].id != p->id ||
            registry.members[t].class_id != ch->class || t == m) return;
        next.members[m].role = OFFICER_ROLE;
        next.members[t].role = LEADER_ROLE;
    } else {
        if ((t >= 0 && registry.members[t].id != 0) ||
            (ch->clan != NULL && ch->clan[0] &&
             str_cmp(ch->clan, registry.clans[c].label))) {
            stc("Leave your current clan before accepting.\n\r", ch); return;
        }
        if (t < 0) for (int i = 0; i < MEMBER_LIMIT; ++i)
            if (!next.members[i].name[0]) { t = i; break; }
        if (t < 0) { stc("Clan registry member capacity reached.\n\r", ch); return; }
        canonical_name(ch->name, next.members[t].name);
        next.members[t].id = p->id; next.members[t].class_id = ch->class;
        next.members[t].role = MEMBER_ROLE;
    }
    if (commit_registry(ch, &next)) {
        p->id = 0;
        stc("Your named clan offer was accepted.\n\r", leader);
    }
}

static void disband(struct clan_registry *next, long id)
{
    int c = clan_index(next, id);
    if (c >= 0) memset(&next->clans[c], 0, sizeof(next->clans[c]));
    for (int i = 0; i < MEMBER_LIMIT; ++i) if (next->members[i].id == id) {
        next->members[i].id = 0; next->members[i].role = 0;
    }
}

void do_clan(CHAR_DATA *ch, char *argument)
{
    char action[MAX_INPUT_LENGTH], target[MAX_INPUT_LENGTH], extra[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int m, c, t;
    if (ch == NULL || IS_NPC(ch) || ch->pcdata == NULL) return;
    argument = one_argument(argument, action);
    argument = one_argument(argument, target);
    one_argument(argument, extra);
    if (!available(ch, FALSE)) return;
    m = active_member(ch);
    if (!action[0] || !str_cmp(action, "info") || !str_cmp(action, "roster")) {
        if (m < 0) {
            stc("No managed clan membership. Staff can create/adopt a class clan.\n\r", ch);
        } else {
            c = clan_index(&registry, registry.members[m].id);
            snprintf(buf, sizeof(buf), "Clan ID %ld: %s (class %ld); your role: %s.\n\r",
                     registry.clans[c].id, registry.clans[c].label, ch->class,
                     role_name(registry.members[m].role));
            stc(buf, ch);
            if (!str_cmp(action, "roster")) {
                long page = 1;
                int count = 0, shown = 0;
                if (extra[0] || (target[0] && (!number(target, &page) ||
                    page < 1 || page > (MEMBER_LIMIT + 19) / 20))) {
                    stc("Syntax: clan roster [page].\n\r", ch); return;
                }
                for (int i = 0; i < MEMBER_LIMIT; ++i)
                    if (registry.members[i].id == registry.members[m].id) ++count;
                if (page > (count + 19) / 20) {
                    stc("No such roster page.\n\r", ch); return;
                }
                snprintf(buf, sizeof(buf), "Roster page %ld of %d (%d registered members, including offline).\n\r",
                         page, (count + 19) / 20, count);
                stc(buf, ch);
                count = 0;
                for (int i = 0; i < MEMBER_LIMIT && shown < 20; ++i)
                    if (registry.members[i].id == registry.members[m].id &&
                        count++ >= (page - 1) * 20) {
                    snprintf(buf, sizeof(buf), "  %-12s %s\n\r",
                             registry.members[i].name, role_name(registry.members[i].role));
                    stc(buf, ch);
                    ++shown;
                }
            }
        }
        stc("Clan: info, roster [page], invite <player>, accept <inviter>, decline,\n\r"
            "promote/demote/kick <member>, transfer <member>, leave, disband confirm.\n\r", ch);
        return;
    }
    if (!player_ready(ch) || !available(ch, TRUE)) {
        stc("Use your own awake, out-of-combat player session for clan changes.\n\r", ch);
        return;
    }
    if (extra[0]) { stc("Too many clan arguments.\n\r", ch); return; }
    if (!str_cmp(action, "decline") && !target[0]) {
        for (int i = 0; i < INVITE_LIMIT; ++i)
            if (!str_cmp(invites[i].to, ch->name)) invites[i].id = 0;
        stc("Pending clan offer declined.\n\r", ch); return;
    }
    if (!str_cmp(action, "accept") && target[0]) { accept_offer(ch, target); return; }
    if (m < 0) { stc("You are not a registered clan member.\n\r", ch); return; }
    if ((!str_cmp(action, "invite") || !str_cmp(action, "transfer")) && target[0]) {
        offer(ch, m, target, !str_cmp(action, "transfer")); return;
    }
    struct clan_registry next = registry;
    if (!str_cmp(action, "leave") && !target[0]) {
        if (registry.members[m].role == LEADER_ROLE) {
            stc("Transfer leadership or use 'clan disband confirm' first.\n\r", ch); return;
        }
        next.members[m].id = next.members[m].role = 0;
    } else if (!str_cmp(action, "disband") && !str_cmp(target, "confirm")) {
        if (registry.members[m].role != LEADER_ROLE) {
            stc("Only the clan leader may disband it.\n\r", ch); return;
        }
        disband(&next, registry.members[m].id);
    } else if ((!str_cmp(action, "promote") || !str_cmp(action, "demote") ||
                !str_cmp(action, "kick")) && target[0]) {
        t = member_index(&registry, target);
        if (t < 0 || t == m || registry.members[t].id != registry.members[m].id ||
            registry.members[t].class_id != ch->class ||
            registry.members[m].role <= registry.members[t].role ||
            registry.members[m].role < OFFICER_ROLE) {
            stc("You may only manage lower-ranked members of your own clan.\n\r", ch); return;
        }
        if (!str_cmp(action, "kick")) next.members[t].id = next.members[t].role = 0;
        else {
            if (registry.members[m].role != LEADER_ROLE) {
                stc("Only the leader appoints or removes officers.\n\r", ch); return;
            }
            next.members[t].role = !str_cmp(action, "promote") ? OFFICER_ROLE : MEMBER_ROLE;
        }
    } else {
        stc("Unknown/incomplete clan command. Type 'clan' for syntax.\n\r", ch); return;
    }
    commit_registry(ch, &next);
}

void clan_legacy(CHAR_DATA *ch, const char *action, char *argument)
{
    char target[MAX_INPUT_LENGTH], extra[MAX_INPUT_LENGTH], command[MAX_INPUT_LENGTH * 2];
    argument = one_argument(argument, target);
    one_argument(argument, extra);
    if (!target[0] || extra[0]) { stc("Specify one clan target.\n\r", ch); return; }
    snprintf(command, sizeof(command), "%s %s", action, target);
    do_clan(ch, command);
}

void do_clanadmin(CHAR_DATA *ch, char *argument)
{
    char action[MAX_INPUT_LENGTH], who[MAX_INPUT_LENGTH], rest[MAX_INPUT_LENGTH];
    char name[NAME_LIMIT + 1], buf[MAX_STRING_LENGTH];
    int m, c = -1;
    long id;
    CHAR_DATA *leader;
    if (ch == NULL || IS_NPC(ch) || ch->pcdata == NULL || ch->desc == NULL ||
        ch->desc->original != NULL || ch->desc->connected != CON_PLAYING ||
        get_trust(ch) < LEV_IMM) return;
    argument = one_argument(argument, action);
    if (!available(ch, FALSE)) return;
    if (!str_cmp(action, "retry") && argument[0] == '\0') {
        commit_registry(ch, &registry); return;
    }
    if (!str_cmp(action, "list") && argument[0] == '\0') {
        for (int i = 0; i < CLAN_LIMIT; ++i) if (registry.clans[i].id) {
            snprintf(buf, sizeof(buf), "ID %ld class %ld: %s\n\r",
                     registry.clans[i].id, registry.clans[i].class_id, registry.clans[i].label);
            stc(buf, ch);
        }
        return;
    }
    if (!available(ch, TRUE)) return;
    argument = one_argument(argument, who);
    if (!str_cmp(action, "disband")) {
        argument = one_argument(argument, rest);
        if (!number(who, &id) || clan_index(&registry, id) < 0 ||
            str_cmp(rest, "confirm") || argument[0]) {
            stc("Syntax: clanadmin disband <id> confirm.\n\r", ch); return;
        }
        struct clan_registry next = registry;
        disband(&next, id); commit_registry(ch, &next); return;
    }
    if (str_cmp(action, "create") || !valid_label(argument) ||
        (leader = get_char_world(ch, who)) == NULL || !player_ready(leader) ||
        !canonical_name(leader->name, name)) {
        stc("Syntax: clanadmin create <online-leader> <2-25 character clan name>.\n\r", ch);
        return;
    }
    if ((leader->clan != NULL && leader->clan[0] && str_cmp(leader->clan, argument)) ||
        ((m = member_index(&registry, name)) >= 0 && registry.members[m].id != 0)) {
        stc("Adopt their existing clan name, or have them leave their clan first.\n\r", ch);
        return;
    }
    for (int i = 0; i < CLAN_LIMIT; ++i) {
        if (registry.clans[i].id && registry.clans[i].class_id == leader->class &&
            !str_cmp(registry.clans[i].label, argument)) {
            stc("That class clan already exists.\n\r", ch); return;
        }
        if (c < 0 && registry.clans[i].id == 0) c = i;
    }
    struct clan_registry next = registry;
    if (m < 0) for (int i = 0; i < MEMBER_LIMIT; ++i)
        if (!next.members[i].name[0]) { m = i; break; }
    if (m < 0 || c < 0 || next.next_id == LONG_MAX) {
        stc("Clan registry capacity reached.\n\r", ch); return;
    }
    next.clans[c].id = next.next_id++;
    next.clans[c].class_id = leader->class;
    strcpy(next.clans[c].label, argument);
    strcpy(next.members[m].name, name);
    next.members[m].id = next.clans[c].id;
    next.members[m].class_id = leader->class;
    next.members[m].role = LEADER_ROLE;
    commit_registry(ch, &next);
}
