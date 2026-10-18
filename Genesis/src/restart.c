/* Checked manual restart and versioned, private, atomic socket handoff.
 * No live world mutation or connection close before a successful exec. */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <time.h>
#include "merc.h"
#include "restart.h"
#include "atomic_save.h"

#define HANDOFF_MAGIC "ANATHEMA_COPYOVER 1\n"
extern int port, control;
extern bool write_to_descriptor(long fd, char *text, long length);
struct handoff_row { int fd; char name[256], host[256]; };
struct fd_flags { int fd, flags; };
struct handoff {
    size_t count, flag_count;
    struct handoff_row rows[FD_SETSIZE];
    struct fd_flags flags[FD_SETSIZE+2];
};

#ifdef RESTART_TESTING
int restart_test_exec_errno=ENOEXEC, restart_test_exec_count;
int restart_test_manifest_fault;
void (*restart_test_exec_probe)(void);
#endif

static bool number(const char *text, long low, long high, long *value)
{
    if (text==NULL || *text=='\0') return FALSE;
    for (const unsigned char *p=(const unsigned char *)text; *p; ++p)
        if (*p<'0' || *p>'9') return FALSE;
    errno=0; char *end;
    long parsed=strtol(text,&end,10);
    if (errno || *end || parsed<low || parsed>high) return FALSE;
    *value=parsed; return TRUE;
}

bool restart_arguments(int argc, char **argv, int *p, int *listener, bool *copyover)
{
    *p=4000; *listener=-1; *copyover=FALSE;
    if (argv==NULL || argc<1) return FALSE;
    if (argc==1) return TRUE;
    if (argc!=2 && argc!=4) return FALSE;
    long parsed_port,parsed_fd;
    if (!number(argv[1],1025,65535,&parsed_port)) return FALSE;
    *p=(int)parsed_port;
    if (argc==2) return TRUE;
    if (argv[2]==NULL || strcmp(argv[2],"copyover") ||
        !number(argv[3],3,FD_SETSIZE-1,&parsed_fd)) return FALSE;
    *listener=(int)parsed_fd;
    *copyover=TRUE; return TRUE;
}

static bool connected_socket(int fd)
{
    int type; socklen_t n=sizeof(type);
    struct sockaddr_storage peer; socklen_t pn=sizeof(peer);
    return fd>2 && fd<FD_SETSIZE &&
        getsockopt(fd,SOL_SOCKET,SO_TYPE,&type,&n)==0 && type==SOCK_STREAM &&
        getpeername(fd,(struct sockaddr *)&peer,&pn)==0;
}

bool restart_listener_valid(int fd, int expected_port)
{
    int listening; socklen_t n=sizeof(listening);
    struct sockaddr_storage address; socklen_t size=sizeof(address);
    if (fd<3 || fd>=FD_SETSIZE ||
        getsockopt((int)fd,SOL_SOCKET,SO_ACCEPTCONN,&listening,&n)<0 || !listening ||
        getsockname((int)fd,(struct sockaddr *)&address,&size)<0) return FALSE;
    if (address.ss_family==AF_INET)
        return ntohs(((struct sockaddr_in *)&address)->sin_port)==expected_port;
    if (address.ss_family==AF_INET6)
        return ntohs(((struct sockaddr_in6 *)&address)->sin6_port)==expected_port;
    return FALSE;
}

static bool host_valid(const char *host)
{
    if (host==NULL || *host=='\0' || strlen(host)>255) return FALSE;
    for (const unsigned char *p=(const unsigned char *)host; *p; ++p)
        if (*p<33 || *p>126) return FALSE;
    return TRUE;
}

static bool add_flags(struct handoff *h, int fd)
{
    if (fd<3 || fd>=FD_SETSIZE || h->flag_count>=FD_SETSIZE+2) return FALSE;
    for (size_t i=0; i<h->flag_count; ++i)
        if (h->flags[i].fd==fd) return FALSE;
    int flags=fcntl(fd,F_GETFD);
    if (flags<0) return FALSE;
    h->flags[h->flag_count++]=(struct fd_flags){fd,flags};
    return TRUE;
}

static bool row_unique(struct handoff *h, int fd, const char *name)
{
    for (size_t i=0; i<h->count; ++i)
        if (h->rows[i].fd==fd || !str_cmp(h->rows[i].name,name)) return FALSE;
    return TRUE;
}

static int write_handoff(FILE *fp, void *context)
{
    struct handoff *h=context;
    if (fputs(HANDOFF_MAGIC,fp)==EOF) return -1;
    for (size_t i=0; i<h->count; ++i)
        if (fprintf(fp,"%d %s %s\n",h->rows[i].fd,h->rows[i].name,
                    h->rows[i].host)<0) return -1;
    return fputs("-1\n",fp)==EOF ? -1 : 0;
}

void restart_copyover(CHAR_DATA *ch)
{
    struct handoff *h=calloc(1,sizeof(*h));
    bool published=FALSE;
    if (ch==NULL) { free(h); return; }
    if (h==NULL) { stc("Copyover cancelled: out of memory.\n\r",ch); return; }
    if (!restart_listener_valid(control,port) || access(EXE_FILE,X_OK)<0)
        goto aborted;
    if (!player_restart_checkpoint()) goto aborted;
    if (!add_flags(h,(int)control) || fpReserve==NULL ||
        !add_flags(h,fileno(fpReserve))) goto aborted;
    for (DESCRIPTOR_DATA *d=descriptor_list; d; d=d->next) {
        if (d->descriptor<3 || d->descriptor>=FD_SETSIZE ||
            !connected_socket((int)d->descriptor) ||
            !add_flags(h,(int)d->descriptor)) goto aborted;
        if (!descriptor_is_in_game(d)) continue; /* Close only on successful exec. */
        CHAR_DATA *owner=player_save_owner(d->original ? d->original : d->character);
        if (owner==NULL || !host_valid(d->host) ||
            !row_unique(h,(int)d->descriptor,owner->name)) goto aborted;
        struct handoff_row *row=&h->rows[h->count++];
        row->fd=(int)d->descriptor;
        strcpy(row->name,owner->name); strcpy(row->host,d->host);
    }
    struct atomic_save_error error;
#ifdef RESTART_TESTING
    enum atomic_save_fault previous=atomic_save_test_fault;
    atomic_save_test_fault=(enum atomic_save_fault)restart_test_manifest_fault;
#endif
    enum atomic_save_result result=atomic_save_copyover(".",write_handoff,h,&error);
#ifdef RESTART_TESTING
    atomic_save_test_fault=previous;
#endif
    published=result!=ATOMIC_SAVE_FAILED;
    if (result!=ATOMIC_SAVE_OK) {
        char message[256];
        snprintf(message,sizeof(message),"Copyover manifest: %s: %s",
                 error.stage,strerror(error.number));
        log_string(message);
        goto aborted;
    }
    for (size_t i=0; i<h->flag_count; ++i) {
        bool keep=h->flags[i].fd==control;
        for (size_t j=0; j<h->count; ++j)
            if (h->flags[i].fd==h->rows[j].fd) keep=TRUE;
        int flags=h->flags[i].flags;
        if (fcntl(h->flags[i].fd,F_SETFD,
                  keep ? flags&~FD_CLOEXEC : flags|FD_CLOEXEC)<0) goto aborted;
    }
    char p[32], fd[32];
    snprintf(p,sizeof(p),"%d",port); snprintf(fd,sizeof(fd),"%d",control);
#ifdef RESTART_TESTING
    ++restart_test_exec_count;
    if (restart_test_exec_probe) restart_test_exec_probe();
    errno=restart_test_exec_errno;
#else
    execl(EXE_FILE,"merc",p,"copyover",fd,(char *)NULL);
#endif
    {
        char message[256];
        snprintf(message,sizeof(message),"Copyover exec failed: %s",strerror(errno));
        log_string(message);
    }
aborted:
    /* This also restores flags after a partial flag-setting failure. No
     * descriptor/control/body/reserve file was closed or detached. */
    for (size_t i=0; i<h->flag_count; ++i)
        if (fcntl(h->flags[i].fd,F_SETFD,h->flags[i].flags)<0)
            log_string("WARNING: failed to restore descriptor flags after cancelled copyover.");
    if (published && unlink(COPYOVER_FILE)<0)
        log_string("WARNING: could not remove cancelled copyover manifest.");
    stc("Copyover cancelled. Check primary saves, drafts/editors, dirty areas, sockets and executable. Sessions remain running.\n\r",ch);
    log_string("Copyover cancelled before exec; player sessions retained.");
    free(h);
}

static bool read_handoff(FILE *fp, struct handoff *h)
{
    char line[1024];
    if (!fgets(line,sizeof(line),fp) || strcmp(line,HANDOFF_MAGIC)) return FALSE;
    while (fgets(line,sizeof(line),fp)) {
        if (!strchr(line,'\n')) return FALSE;
        if (!strcmp(line,"-1\n")) return getc(fp)==EOF && !ferror(fp);
        char token[32], name[256], host[256], extra;
        long fd;
        if (h->count>=FD_SETSIZE ||
            sscanf(line,"%31s %255s %255s %c",token,name,host,&extra)!=3 ||
            !number(token,3,FD_SETSIZE-1,&fd) || fd==control ||
            !atomic_save_name_valid(name) || !host_valid(host) ||
            !connected_socket((int)fd) || !row_unique(h,(int)fd,name)) return FALSE;
        struct handoff_row *row=&h->rows[h->count++];
        row->fd=(int)fd; strcpy(row->name,name); strcpy(row->host,host);
    }
    return FALSE; /* Missing sentinel or I/O error, never loop on stale input. */
}

static DESCRIPTOR_DATA *recovered_descriptor(void)
{
    static const DESCRIPTOR_DATA zero;
    DESCRIPTOR_DATA *d;
    if (descriptor_free==NULL) d=alloc_perm(sizeof(*d));
    else { d=descriptor_free; descriptor_free=d->next; }
    *d=zero; d->connected=CON_COPYOVER_RECOVER;
    d->outsize=2000; d->outbuf=alloc_mem(d->outsize);
    return d;
}

bool copyover_recover(void)
{
    struct handoff *h=calloc(1,sizeof(*h));
    if (h==NULL) return FALSE;
    int fd=open(COPYOVER_FILE,O_RDONLY|O_NOFOLLOW|O_NONBLOCK|O_CLOEXEC);
    struct stat status;
    if (fd<0) { free(h); return FALSE; }
    if (fstat(fd,&status)<0 || !S_ISREG(status.st_mode) ||
        status.st_uid!=geteuid() || (status.st_mode&077)!=0 || status.st_nlink!=1) {
        close(fd); free(h); return FALSE;
    }
    FILE *fp=fdopen(fd,"r");
    if (!fp) { close(fd); free(h); return FALSE; }
    bool valid=read_handoff(fp,h);
    if (fclose(fp)!=0) valid=FALSE;
    if (!valid || unlink(COPYOVER_FILE)<0) { free(h); return FALSE; }
    /* Complete syntax/socket validation precedes any player restoration. */
    for (size_t i=0; i<h->count; ++i) {
        struct handoff_row *row=&h->rows[i];
        DESCRIPTOR_DATA *d=recovered_descriptor();
        d->descriptor=row->fd; d->host=str_dup(row->host);
        d->next=descriptor_list; descriptor_list=d;
        if (!load_char_obj(d,row->name) || IS_NPC(d->character) ||
            d->character->pcdata==NULL || d->character->level<LEV_AWA ||
            str_cmp(d->character->name,row->name)) {
            close_socket(d); continue;
        }
        CHAR_DATA *ch=d->character;
        if (ch->in_room==NULL) ch->in_room=get_room_index(ROOM_VNUM_TEMPLE);
        if (ch->in_room==NULL) { close_socket(d); continue; }
        ch->next=char_list; char_list=ch;
        char_to_room(ch,ch->in_room); d->connected=CON_PLAYING;
        send_to_char("Copyover recovery complete.\n\r",ch);
        do_look(ch,"");
    }
    free(h); log_string("Copyover recovery complete.");
    return TRUE;
}
