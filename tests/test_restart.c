#define _POSIX_C_SOURCE 200809L
#define RESTART_TESTING
#define ATOMIC_SAVE_TESTING
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <time.h>
#include <crypt.h>
#include "merc.h"
#include "atomic_save.h"
#include "restart.h"

extern char *string_space, *top_string;
extern void init_mm(void);
extern int port, control;
extern bool merc_down;
extern bool fBootDb;
static ROOM_INDEX_DATA room;
static AREA_DATA area;
static int peers[32], peer_count, login_fd, active_fd, reserve_fd;

static void globals(void)
{
    string_space=malloc(4*1024*1024); assert(string_space); top_string=string_space;
    current_time=time(NULL); init_mm();
    fpReserve=fopen(NULL_FILE,"r"); assert(fpReserve);
    leader_board.bestpk_number=leader_board.tt_number=LONG_MAX;
    leader_board.worstpk_number=LONG_MIN;
    room.vnum=ROOM_VNUM_TEMPLE; room.name="Private room"; room.description="";
    room.area=&area; room.light=1; area.name="Private area";
    room_index_hash[room.vnum%MAX_KEY_HASH]=&room;
}

static int pair(void)
{
    int fds[2]; assert(socketpair(AF_UNIX,SOCK_STREAM,0,fds)==0);
    assert(peer_count<32); peers[peer_count++]=fds[1]; return fds[0];
}

static DESCRIPTOR_DATA *session(int fd, CHAR_DATA *ch, int state)
{
    DESCRIPTOR_DATA *d=alloc_perm(sizeof(*d)); memset(d,0,sizeof(*d));
    d->descriptor=fd; d->character=ch; if(ch) ch->desc=d;
    d->host=str_dup("private-test"); d->connected=state;
    d->outsize=16384; d->outbuf=alloc_mem(d->outsize);
    d->next=descriptor_list; descriptor_list=d; return d;
}

static CHAR_DATA *player(char *name, bool online)
{
    DESCRIPTOR_DATA shell={0}; load_char_obj(&shell,name);
    CHAR_DATA *ch=shell.character; ch->desc=NULL; ch->level=LEV_IMP;
    ch->next=char_list; char_list=ch;
    char_to_room(ch,&room);
    if(online) session(pair(),ch,CON_PLAYING);
    return ch;
}

static void clean(void)
{
    restart_test_exec_probe=NULL; restart_test_manifest_fault=SAVE_FAULT_NONE;
    atomic_save_test_fault=SAVE_FAULT_NONE; merc_down=FALSE; area_first=NULL;
    while(descriptor_list) close_socket(descriptor_list);
    while(char_list) extract_char(char_list,TRUE);
    while(peer_count) assert(close(peers[--peer_count])==0);
    unlink(COPYOVER_FILE); unlink(SHUTDOWN_FILE);
}

static void arguments(void)
{
    int p,fd; bool c;
    char *normal[]={"merc","4000",NULL};
    assert(restart_arguments(2,normal,&p,&fd,&c) && p==4000 && !c);
    assert(!restart_arguments(3,normal,&p,&fd,&c));
    char *bad[]={"merc","999999999999999999999999999999999999999",NULL};
    assert(!restart_arguments(2,bad,&p,&fd,&c));
    char *partial[]={"merc","4000","copyover",NULL};
    assert(!restart_arguments(3,partial,&p,&fd,&c));
    char *wrong[]={"merc","4000","anything","4",NULL};
    assert(!restart_arguments(4,wrong,&p,&fd,&c));
    char *zero[]={"merc","4000","copyover","0",NULL};
    assert(!restart_arguments(4,zero,&p,&fd,&c));
    char buf[32],pbuf[32];
    snprintf(buf,sizeof(buf),"%d",control); snprintf(pbuf,sizeof(pbuf),"%d",port);
    char *valid[]={"merc",pbuf,"copyover",buf,NULL};
    assert(restart_arguments(4,valid,&p,&fd,&c) && c && fd==control && p==port);
    assert(restart_listener_valid(control,port) && !restart_listener_valid(0,port));
}

static void shutdown_failure(void)
{
    CHAR_DATA *ch=player("Shutdown",TRUE);
    atomic_save_test_fault=SAVE_FAULT_RENAME;
    do_shutdown(ch,"");
    assert(!merc_down && ch->desc && descriptor_list==ch->desc &&
           access(SHUTDOWN_FILE,F_OK)!=0);
    atomic_save_test_fault=SAVE_FAULT_NONE;
    do_shutdown(ch,"");
    assert(merc_down && access("../player/Shutdown",F_OK)==0);
    clean();
}

static void blockers(void)
{
    CHAR_DATA *ch=player("Blocker",TRUE);
    ch->level=LEV_SLE; assert(!player_restart_checkpoint());
    ch->level=LEV_IMP; ch->desc->connected=CON_NOTE_TEXT;
    assert(!player_restart_checkpoint());
    ch->desc->connected=CON_PLAYING;
    char *draft=NULL; ch->desc->pString=&draft;
    assert(!player_restart_checkpoint());
    ch->desc->pString=NULL; area_first=&area; area.area_flags=AREA_CHANGED;
    assert(!player_restart_checkpoint());
    area.area_flags=AREA_ADDED; assert(player_restart_checkpoint());
    /* ADDED is informational after asave; CHANGED is the unsaved marker. */
    area.area_flags=0;
    CHAR_DATA *saved_list=char_list; char_list=NULL;
    assert(!player_restart_checkpoint()); char_list=saved_list;
    clean();
}

static void probe(void)
{
    assert(!(fcntl((int)control,F_GETFD)&FD_CLOEXEC));
    assert(!(fcntl(active_fd,F_GETFD)&FD_CLOEXEC));
    assert(fcntl(login_fd,F_GETFD)&FD_CLOEXEC);
    assert(fcntl(reserve_fd,F_GETFD)&FD_CLOEXEC);
    struct stat st; assert(stat(COPYOVER_FILE,&st)==0 && (st.st_mode&0777)==0600);
}

static void copyover_failures(void)
{
    CHAR_DATA *ch=player("Copyover",TRUE);
    DESCRIPTOR_DATA *d=ch->desc;
    active_fd=(int)d->descriptor; login_fd=pair(); session(login_fd,NULL,CON_GET_NAME);
    reserve_fd=fileno(fpReserve);
    assert(fcntl((int)control,F_SETFD,FD_CLOEXEC)==0);
    assert(fcntl(active_fd,F_SETFD,FD_CLOEXEC)==0);
    int before_control=fcntl((int)control,F_GETFD),before_active=fcntl(active_fd,F_GETFD);
    int before_login=fcntl(login_fd,F_GETFD),before_reserve=fcntl(reserve_fd,F_GETFD);
    restart_test_exec_count=0; restart_test_exec_probe=probe;
    atomic_save_test_fault=SAVE_FAULT_RENAME;
    do_copyover(ch,"");
    assert(restart_test_exec_count==0 && ch->desc==d && descriptor_list &&
           access(COPYOVER_FILE,F_OK)!=0);
    atomic_save_test_fault=SAVE_FAULT_NONE;
    int faults[]={SAVE_FAULT_CREATE,SAVE_FAULT_FLUSH,SAVE_FAULT_SYNC,
                  SAVE_FAULT_CLOSE,SAVE_FAULT_RENAME,SAVE_FAULT_DIRSYNC};
    for(size_t i=0;i<sizeof(faults)/sizeof(faults[0]);++i) {
        restart_test_manifest_fault=faults[i]; do_copyover(ch,"");
        assert(restart_test_exec_count==0 && ch->desc==d &&
               fcntl(active_fd,F_GETFD)==before_active &&
               access(COPYOVER_FILE,F_OK)!=0);
    }
    restart_test_manifest_fault=SAVE_FAULT_NONE;
    do_copyover(ch,""); /* exec probe returns ENOEXEC, not a real restart. */
    assert(restart_test_exec_count==1 && ch->desc==d && fileno(fpReserve)==reserve_fd &&
           fcntl((int)control,F_GETFD)==before_control &&
           fcntl(active_fd,F_GETFD)==before_active &&
           fcntl(login_fd,F_GETFD)==before_login &&
           fcntl(reserve_fd,F_GETFD)==before_reserve && access(COPYOVER_FILE,F_OK)!=0);
    MOB_INDEX_DATA index={0};
    FILE *words=tmpfile(); assert(words);
    assert(fputs("familiar~",words)!=EOF); rewind(words);
    fBootDb=TRUE; index.player_name=index.short_descr=fread_string(words);
    fBootDb=FALSE; assert(fclose(words)==0);
    index.long_descr=index.description=str_empty; index.act=ACT_IS_NPC;
    index.vnum=92003; index.level=1;
    CHAR_DATA *body=create_mobile(&index); char_to_room(body,&room);
    ch->pcdata->familiar=body; body->wizard=ch;
    do_switch(ch,"familiar");
    assert(d->original==ch && d->character==body && ch->desc==NULL);
    do_copyover(body,"");
    assert(restart_test_exec_count==2 && d->original==ch && d->character==body &&
           ch->desc==NULL && ch->pcdata->familiar==body && body->wizard==ch &&
           IS_EXTRA(ch,EXTRA_SWITCH) && fileno(fpReserve)==reserve_fd);
    clean();
}

static void manifest(const char *text)
{
    FILE *fp=fopen(COPYOVER_FILE,"w"); assert(fp);
    assert(fchmod(fileno(fp),0600)==0 && fputs(text,fp)!=EOF && fclose(fp)==0);
}

static void recover(void)
{
    CHAR_DATA *ch=player("Recover",FALSE); ch->gold=987;
    assert(player_save_state(ch,PLAYER_SAVE_ALL)==PLAYER_SAVE_ALL); clean();
    int fd=pair(); char text[2048];
    const char *bad[]={"","4 Recover host\n-1\n","ANATHEMA_COPYOVER 1\n",
        "ANATHEMA_COPYOVER 1\n-1\njunk\n",
        "ANATHEMA_COPYOVER 1\n999999999999999999999999 Recover host\n-1\n",
        "ANATHEMA_COPYOVER 1\n0 Recover host\n-1\n"};
    for(size_t i=0;i<sizeof(bad)/sizeof(bad[0]);++i) {
        manifest(bad[i]); assert(!copyover_recover() && !char_list && !descriptor_list);
    }
    snprintf(text,sizeof(text),"ANATHEMA_COPYOVER 1\n%d Recover host\n%d Duplicate host\n-1\n",fd,fd);
    manifest(text); assert(!copyover_recover() && !char_list && !descriptor_list);
    int second=pair();
    snprintf(text,sizeof(text),"ANATHEMA_COPYOVER 1\n%d Recover host\n%d recover host\n-1\n",fd,second);
    manifest(text); assert(!copyover_recover() && !char_list && !descriptor_list);
    assert(close(second)==0);
    char oversized[301]; memset(oversized,'a',300); oversized[300]='\0';
    snprintf(text,sizeof(text),"ANATHEMA_COPYOVER 1\n%d %s host\n-1\n",fd,oversized);
    manifest(text); assert(!copyover_recover());
    snprintf(text,sizeof(text),"ANATHEMA_COPYOVER 1\n%d Recover %s\n-1\n",fd,oversized);
    manifest(text); assert(!copyover_recover());
    unlink(COPYOVER_FILE);
    assert(symlink("../player/Recover",COPYOVER_FILE)==0);
    assert(!copyover_recover() && access("../player/Recover",F_OK)==0);
    assert(unlink(COPYOVER_FILE)==0 && mkfifo(COPYOVER_FILE,0600)==0);
    assert(!copyover_recover()); assert(unlink(COPYOVER_FILE)==0);
    snprintf(text,sizeof(text),"ANATHEMA_COPYOVER 1\n%d Recover host\n-1\n",fd);
    manifest(text); assert(copyover_recover());
    assert(char_list && char_list->gold==987 && char_list->desc &&
           char_list->desc->descriptor==fd && char_list->desc->connected==CON_PLAYING &&
           access(COPYOVER_FILE,F_OK)!=0);
    clean();
}

static void remove_directory(const char *name)
{
    DIR *dir=opendir(name); assert(dir); struct dirent *entry;
    while((entry=readdir(dir))) {
        if(entry->d_name[0]=='.') continue;
        char path[1024]; snprintf(path,sizeof(path),"%s/%s",name,entry->d_name);
        assert(unlink(path)==0);
    }
    assert(closedir(dir)==0 && rmdir(name)==0);
}

int main(int argc,char **argv)
{
    if(argc==3 && !strcmp(argv[1],"--seed")) {
        assert(chdir(argv[2])==0); globals();
        CHAR_DATA *ch=player("Restarter",FALSE);
        char *hash=crypt("test-only-passphrase","$6$fixture$");
        assert(hash && hash[0]!='*');
        free_string(ch->pcdata->pwd); ch->pcdata->pwd=str_dup(hash);
        SET_BIT(ch->extra,EXTRA_NEWPASS); ch->gold=777;
        assert(player_save_state(ch,PLAYER_SAVE_ALL)==PLAYER_SAVE_ALL);
        clean(); assert(fclose(fpReserve)==0); return 0;
    }
    char root[]="/tmp/anathema-restart-XXXXXX",cwd[4096];
    assert(getcwd(cwd,sizeof(cwd)) && mkdtemp(root) && chdir(root)==0);
    assert(mkdir("player",0700)==0 && mkdir("backup",0700)==0 &&
           mkdir("run",0700)==0 && mkdir("src",0700)==0 && chdir("run")==0);
    globals();
    FILE *exe=fopen("../src/merc","w"); assert(exe);
    assert(fputs("#!/bin/sh\nexit 99\n",exe)!=EOF && fclose(exe)==0 &&
           chmod("../src/merc",0700)==0);
    control=socket(AF_INET,SOCK_STREAM,0); assert(control>=3);
    struct sockaddr_in address={0}; address.sin_family=AF_INET;
    address.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
    assert(bind((int)control,(struct sockaddr *)&address,sizeof(address))==0 &&
           listen((int)control,8)==0);
    socklen_t size=sizeof(address);
    assert(getsockname((int)control,(struct sockaddr *)&address,&size)==0);
    port=ntohs(address.sin_port);
    arguments(); shutdown_failure(); blockers(); copyover_failures(); recover();
    assert(close((int)control)==0 && fclose(fpReserve)==0);
    assert(unlink("../src/merc")==0 && chdir("..")==0);
    remove_directory("player"); remove_directory("backup");
    assert(rmdir("run")==0 && rmdir("src")==0 && chdir(cwd)==0 && rmdir(root)==0);
    puts("restart/checkpoint/manifest/descriptor recovery regressions passed");
    return 0;
}
