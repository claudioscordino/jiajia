/**********************************************************************
 *                                                                     *
 *   The JIAJIA Software Distributed Shared Memory System              * 
 *                                                                     *
 *   Copyright (C) 1997 the Center of High Performance Computing       * 
 *   of Institute of Computing Technology, Chinese Academy of          *      
 *   Sciences.  All rights reserved.                                   * 
 *                                                                     *
 *   Permission to use, copy, modify and distribute this software      *
 *   is hereby granted provided that (1) source code retains these     *
 *   copyright, permission, and disclaimer notices, and (2) redistri-  *
 *   butions including binaries reproduce the notices in supporting    *
 *   documentation, and (3) all advertising materials mentioning       *
 *   features or use of this software display the following            *
 *   acknowledgement: ``This product includes software developed by    *
 *   the Center of High Performance Computing, Institute of Computing  *
 *   Technology, Chinese Academy of Sciences."                         * 
 *                                                                     *
 *   This program is distributed in the hope that it will be useful,   *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of    *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.              * 
 *                                                                     *
 *   Center of High Performance Computing requests users of this       *
 *   software to return to dsm@water.chpc.ict.ac.cn any                * 
 *   improvements that they make and grant CHPC redistribution rights. *    
 *                                                                     * 
 *         Author: Weiwu Hu, Weisong Shi, Zhimin Tang                  *      
 * =================================================================== *
 *   This software is ported to SP2 by                                 *
 *                                                                     *
 *         M. Rasit Eskicioglu                                         *
 *         University of Alberta                                       * 
 *         Dept. of Computing Science                                  *
 *         Edmonton, Alberta T6G 2H1 CANADA                            *
 * =================================================================== *
 **********************************************************************/

#ifndef NULL_LIB
#include "comm.h"
#include "mem.h"

#define  BEGINCS  {sigset_t newmask, oldmask;\
                   sigemptyset(&newmask);\
                   sigaddset(&newmask, SIGIO);\
                   sigprocmask(SIG_BLOCK, &newmask, &oldmask);\
                   oldsigiomask=sigismember(&oldmask,SIGIO);\
                   printf("Enter CS");\
                  }
#define  ENDCS    {if (oldsigiomask==0) enable_sigio();\
                   printf("Exit CS\n");\
                  }

#ifndef JIA_DEBUG 
#define  msgprint  0 
#define  printf   emptyprintf
#else  /* JIA_DEBUG */
#define msgprint  1
#endif  /* JIA_DEBUG */

/*----------------------------------------------------------*/
/* following definitions are defined by Hu */ 
extern  host_t  hosts[Maxhosts];
extern  int     jia_pid; 
extern  int     hostc;
extern  char    errstr[Linesize];
extern  int     msgbusy[Maxmsgs];
extern  jia_msg_t   msgarray[Maxmsgs];
extern  int     msgcnt;

/* servers used by asynchronous */
extern void   diffserver(jia_msg_t *);
extern void   getpserver(jia_msg_t *);
extern void   acqserver(jia_msg_t * );
extern void   invserver(jia_msg_t *); 
extern void   relserver(jia_msg_t *);
extern void   jiaexitserver(jia_msg_t *);
extern void   wtntserver(jia_msg_t *);
extern void   barrserver(jia_msg_t *); 
extern void   barrgrantserver(jia_msg_t * );
extern void   acqgrantserver(jia_msg_t *);
extern void   waitgrantserver(jia_msg_t *);
extern void   waitserver(jia_msg_t *);
extern void   diffgrantserver(jia_msg_t *);
extern void   getpgrantserver(jia_msg_t *);
extern void   loadserver(jia_msg_t *);
extern void   loadgrantserver(jia_msg_t *);
extern void   emptyprintf();

extern void   printmsg(jia_msg_t *, int);
extern jia_msg_t  *newmsg();

#ifdef DOSTAT
extern int statflag;
extern jiastat_t jiastat;
unsigned int interruptflag=0;
#endif

/* following definitions are defined by Shi */
unsigned long reqports[Maxhosts][Maxhosts],repports[Maxhosts][Maxhosts];
CommManager commreq,commrep;
unsigned long timeout_time;
static struct timeval   polltime = { 0, 0 };
jia_msg_t inqueue[Maxqueue],outqueue[Maxqueue];
volatile int inhead, intail,incount;
volatile int outhead, outtail,outcount;
long   Startport;


void    initcomm();
int     req_fdcreate(int, int);
int     rep_fdcreate(int, int);
#if defined SOLARIS || defined IRIX62
void    sigio_handler(int sig, siginfo_t *sip, ucontext_t *uap);
#endif /* SOLARIS */
#ifdef LINUX
void    sigio_handler();
#endif
#ifdef AIX41
void    sigio_handler();
#endif /* AIX41 */
void    sigint_handler();
void    asendmsg(jia_msg_t *);
void    msgserver();
void    outsend();
void bcastserver(jia_msg_t *msg);

extern void assert(int,char *);
extern void assert0(int,char *);
extern unsigned long jia_current_time();
extern void disable_sigio();
extern void enable_sigio();

extern sigset_t oldset;
int oldsigiomask;

/*---------------------------------------------------------*/
int req_fdcreate(int i, int flag)
{   
  int         fd,res;
  int         size;
  struct      sockaddr_in addr;

  fd = socket(AF_INET, SOCK_DGRAM, 0);
  assert0((fd!=-1),"req_fdcreate()-->socket()");

#ifdef SOLARIS 
  size=Maxmsgsize+Msgheadsize+128;
  res=setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char *)&size, sizeof(size));
  assert0((res==0),"req_fdcreate()-->setsockopt():SO_RCVBUF");

  size=Maxmsgsize+Msgheadsize+128;
  res=setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char *)&size, sizeof(size));
  assert0((res==0),"req_fdcreate()-->setsockopt():SO_SNDBUF");
#endif

  addr.sin_family      = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port        = (flag)? htons(0) : htons(reqports[jia_pid][i]);

  res=bind(fd, (struct sockaddr*)&addr, sizeof(addr));
  assert0((res==0),"req_fdcreate()-->bind()");
  return fd;
}

/*------------------------------------------------------------*/
int rep_fdcreate(int i, int flag)
{
  int         fd,res;
#ifdef SOLARIS
  int         size;
#endif /* SOLARIS */
  struct      sockaddr_in addr;

  fd = socket(AF_INET, SOCK_DGRAM, 0);
  assert0((fd!=-1),"rep_fdcreate()-->socket()");

#ifdef SOLARIS
  size=Intbytes;
  res=setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char *)&size, sizeof(size));
  assert0((res==0),"rep_fdcreate()-->setsockopt():SO_RCVBUF");

  size=Intbytes;
  res=setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char *)&size, sizeof(size));
  assert0((res==0),"rep_fdcreate()-->setsockopt():SO_SNDBUF");
#endif /* SOLARIS */

  addr.sin_family      = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port        = (flag)? htons(0):htons(repports[jia_pid][i]);

  res=bind(fd, (struct sockaddr *)&addr, sizeof(addr));
  assert0((res==0),"rep_fdcreate()-->bind()");
  return fd;
}

/*------------------------------------------------------------*/
void initcomm()
{ int i,j,fd;

  if (jia_pid==0){
    printf("************Initialize Communication!*******\n");
  }
  printf(" Startport = %d \n", Startport);


  msgcnt=0;	
  for (i=0;i<Maxmsgs;i++) {
		msgbusy[i]=0;
		msgarray[i].index = i;
  }

  inhead   =0;
  intail   =0;
  incount  =0;
  outhead   =0;
  outtail   =0;
  outcount  =0;

#if defined SOLARIS || defined IRIX62
  { struct sigaction act;

    act.sa_handler = (void_func_handler)sigio_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    if (sigaction(SIGIO, &act, NULL))
      assert0(0,"initcomm()-->sigaction()");

    act.sa_handler = (void_func_handler)sigint_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    if (sigaction(SIGINT, &act, NULL)){
      assert0(0,"segv sigaction problem");  
    }

  }
#endif
#ifdef LINUX
  { struct sigaction act;

    act.sa_handler = (void_func_handler)sigio_handler;
    sigemptyset(&act.sa_mask);
    if (sigaction(SIGIO, &act, NULL))
      assert0(0,"initcomm()-->sigaction()");

    act.sa_handler = (void_func_handler)sigint_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_NOMASK;
    if (sigaction(SIGINT, &act, NULL)){
      assert0(0,"segv sigaction problem");  
    }

  }
#endif
#ifdef AIX41
  { struct sigvec vec;

    vec.sv_handler = (void_func_handler)sigio_handler;
    vec.sv_flags = SV_INTERRUPT;
    sigvec(SIGIO, &vec, NULL);

    vec.sv_handler = (void_func_handler)sigint_handler;
    vec.sv_flags = 0;
    sigvec(SIGINT, &vec, NULL);
  }
#endif 

 /***********Initialize comm ports********************/

  for (i =0; i<Maxhosts; i++ )
    for (j=0 ; j<Maxhosts; j++) {
      reqports[i][j]=Startport+i*Maxhosts+j;
      repports[i][j]= Startport+Maxhosts*Maxhosts+i*Maxhosts+j;
    }
 
#ifdef JIA_DEBUG
  for(i=0; i<Maxhosts; i++) 
     for (j=0; j<Maxhosts; j++) {
        if (j==0) printf("\nREQ[%02d][] = ", i);
           else if (j%5) printf("%d  ", reqports[i][j]);
                   else printf("%d  \n            ", reqports[i][j]);
     }
 
  for(i=0; i<Maxhosts; i++) 
     for (j=0; j<Maxhosts; j++) {
        if (j==0) printf("\nREP[%02d][] = ", i);
           else if (j%5) printf("%d  ", repports[i][j]);
                   else printf("%d  \n            ", reqports[i][j]);
     }
#endif /* JIA_DEBUG */
 
 /***********Initialize commreq ********************/

  commreq.rcv_maxfd =0;
  commreq.snd_maxfd =0; 
  FD_ZERO(&(commreq.snd_set));
  FD_ZERO(&(commreq.rcv_set));
  for(i=0; i<Maxhosts; i++) { 
    fd= req_fdcreate(i,0);
    commreq.rcv_fds[i]= fd;
    FD_SET(fd, &commreq.rcv_set);
    commreq.rcv_maxfd = MAX(fd+1, commreq.rcv_maxfd);

    if (0>fcntl(commreq.rcv_fds[i], F_SETOWN, getpid()))
       assert0(0,"initcomm()-->fcntl(..F_SETOWN..)");

    if (0>fcntl(commreq.rcv_fds[i], F_SETFL, FASYNC|FNDELAY))
       assert0(0,"initcomm()-->fcntl(..F_SETFL..)");

    fd= req_fdcreate(i,1);
    commreq.snd_fds[i] = fd;
    FD_SET(fd, &commreq.snd_set);
    commreq.snd_maxfd = MAX(fd+1, commreq.snd_maxfd);
  }
  for (i=0; i<Maxhosts; i++ ) {
    commreq.snd_seq[i]=0;
    commreq.rcv_seq[i]=0;
  }

 /***********Initialize commrep ********************/

  commrep.rcv_maxfd =0;
  commrep.snd_maxfd =0; 
  FD_ZERO(&(commrep.snd_set));
  FD_ZERO(&(commrep.rcv_set));

  for(i=0; i<Maxhosts; i++) { 
    fd= rep_fdcreate(i,0);
    commrep.rcv_fds[i]= fd;
    FD_SET(fd, &(commrep.rcv_set));
    commrep.rcv_maxfd = MAX(fd+1, commrep.rcv_maxfd);
 
    fd= rep_fdcreate(i,1);
    commrep.snd_fds[i] = fd;
    FD_SET(fd, &commrep.snd_set);
    commrep.snd_maxfd = MAX(fd+1, commrep.snd_maxfd);
  }
}


/*----------------------------------------------------------*/
void msgserver()
{
     SPACE(1);printf("Enterservermsg[%d],inc=%d,inh=%d,int=%d!\n",inqh.op, incount,inhead,intail);
     switch (inqh.op) {
             case REL:      relserver(&inqh);      break;
             case JIAEXIT:  jiaexitserver(&inqh);  break;
             case BARR:     barrserver(&inqh);     break;
             case WTNT:     wtntserver(&inqh);     break;
             case BARRGRANT:barrgrantserver(&inqh);break;
             case ACQGRANT: acqgrantserver(&inqh); break;
             case ACQ :     acqserver(&inqh);      break;
             case INVLD:    invserver(&inqh);      break;
             case WAIT:     waitserver(&inqh);     break;
             case WAITGRANT:waitgrantserver(&inqh);break;
             case GETP:     getpserver(&inqh);     break;
             case GETPGRANT:getpgrantserver(&inqh);break;  
             case DIFF:     diffserver(&inqh);     break;
             case DIFFGRANT:diffgrantserver(&inqh);break;
             case SETCV:    setcvserver(&inqh);    break;
             case RESETCV:  resetcvserver(&inqh);  break;
             case WAITCV:   waitcvserver(&inqh);   break;
             case CVGRANT:  cvgrantserver(&inqh);  break;
             case MSGBODY:  
             case MSGTAIL:  msgrecvserver(&inqh);  break;
             case LOADREQ:  loadserver(&inqh);     break;
             case LOADGRANT:loadgrantserver(&inqh);break;
#ifdef DOSTAT
             case STAT:     statserver(&inqh);     break;
             case STATGRANT:statgrantserver(&inqh);break;
#endif 

             default:       if (inqh.op>=BCAST) {
                               bcastserver(&inqh);
                            } else  { 
                              printmsg(&inqh,1);
                              assert0(0,"msgserver(): Incorrect Message!");
                            }
                            break;   
     }
     SPACE(1);printf("Out servermsg!\n");
}

/*----------------------------------------------------------*/
void sigint_handler()
{
  	assert(0,"Exit by user!!\n");
}
/*----------------------------------------------------------*/
#if defined SOLARIS || defined IRIX62
void  sigio_handler(int sig, siginfo_t *sip, ucontext_t *uap)
#endif 
#ifdef LINUX
void sigio_handler()
#endif
#ifdef AIX41 
void sigio_handler()
#endif
{
  int res, len,oldindex;
  int i, s;
  fd_set  readfds;
  struct sockaddr_in  from,to; 
  sigset_t set,oldset;
  int servemsg;
  int testresult;

  
#ifdef DOSTAT
register unsigned int begin;
if (statflag==1){
    jiastat.sigiocnt++;
  if (interruptflag==0) {
    begin = get_usecs();
    if (jiastat.kernelflag==0)  { 
      jiastat.usersigiocnt++;  
    }else if (jiastat.kernelflag==1){ 
      jiastat.synsigiocnt++; 
    }else if (jiastat.kernelflag==2){ 
      jiastat.segvsigiocnt++; 
    }
  } 
  interruptflag++;	
}
#endif

  SPACE(1); printf("Enter sigio_handler!\n");

  servemsg=0;
  readfds = commreq.rcv_set;	
  polltime.tv_sec=0;
  polltime.tv_usec=0;
  res = select(commreq.rcv_maxfd, &readfds, NULL, NULL, &polltime);
  while (res>0) {
    for (i=0;i<hostc;i++) if (i!=jia_pid) if (FD_ISSET(commreq.rcv_fds[i], &readfds)){
      assert0((incount<Maxqueue), "sigio_handler(): Inqueue exceeded!");

      s=sizeof(from);
      res=recvfrom(commreq.rcv_fds[i],(char *)&(inqt),Maxmsgsize+Msgheadsize,0,
                                 (struct sockaddr *)&from, &s);
      assert0((res>=0), "sigio_handler()-->recvfrom()");

      to.sin_family = AF_INET;
      memcpy(&to.sin_addr,hosts[inqt.frompid].addr,hosts[inqt.frompid].addrlen);
      to.sin_port = htons(repports[inqt.frompid][inqt.topid]);
      
      res = sendto(commrep.snd_fds[i],(char *)&(inqt.seqno),sizeof(inqt.seqno),0,
                         (struct sockaddr *)&to, sizeof(to));
      assert0((res!=-1),"sigio_handler()-->sendto() ACK");

      if (inqt.seqno>commreq.rcv_seq[i]) {

#ifdef DOSTAT
if (statflag==1){
        if (inqt.frompid != inqt.topid) {
          jiastat.msgrcvcnt++;
          jiastat.msgrcvbytes+=(inqt.size+Msgheadsize);
        }
}
#endif

        commreq.rcv_seq[i] = inqt.seqno;
        if (msgprint==1) printmsg(&inqt,1);
        BEGINCS;
        intail=(intail+1)%Maxqueue;
        incount++;
        if (incount==1) servemsg=1;
        ENDCS;
      }else {
        if (msgprint==1) printmsg(&inqt,1);
        printf("Receive resend message!\n");
        jiastat.resentcnt++;
      }
    }
    readfds = commreq.rcv_set;	
    polltime.tv_sec=0;
    polltime.tv_usec=0;
    res = select(commreq.rcv_maxfd, &readfds, NULL, NULL, &polltime);
  }

  SPACE(1);printf("Finishrecvmsg!inc=%d,inh=%d,int=%d\n",incount,inhead,intail);

  enable_sigio();
  while (servemsg==1){
        msgserver();
        BEGINCS;
        inqh.op=ERRMSG;
        inhead=(inhead+1)%Maxqueue;
        incount--;
        servemsg=(incount>0)? 1 : 0;
        ENDCS;
  }	

  SPACE(1);printf("Out sigio_handler!\n");
#ifdef DOSTAT
if (statflag==1){
  interruptflag--;
  if (interruptflag ==0) { 
    if (jiastat.kernelflag==0) { 
      jiastat.usersigiotime+= get_usecs()-begin;
    } else if (jiastat.kernelflag==1) {
      jiastat.synsigiotime += get_usecs()-begin;
    } else if (jiastat.kernelflag==2) {
      jiastat.segvsigiotime += get_usecs()-begin;
    }
  } 
}
#endif DOSTAT
}



/*----------------------------------------------------------*/

void asendmsg(jia_msg_t *msg)
{int outsendmsg;

#ifdef DOSTAT
 register unsigned int begin = get_usecs();
if (statflag==1){
 if (msg->size>4096) jiastat.largecnt++;
 if (msg->size<128)  jiastat.smallcnt++;
}
#endif

  printf("Enter asendmsg! outc=%d, outh=%d, outt=%d\n",
               outcount,outhead, outtail);
  
  BEGINCS;
  assert0((outcount<Maxqueue), "asendmsg(): Outqueue exceeded!");
  memcpy(&(outqt),msg,Msgheadsize+msg->size);
  commreq.snd_seq[msg->topid]++;
  outqt.seqno= commreq.snd_seq[msg->topid];
  outcount++;
  outtail=(outtail+1)%Maxqueue;
  outsendmsg=(outcount==1)? 1 : 0;
  ENDCS;
  
  while(outsendmsg==1){
    outsend();
    BEGINCS;
    outhead=(outhead+1)%Maxqueue;
    outcount--;
    outsendmsg=(outcount>0)? 1 : 0;
    ENDCS;
  }
  printf("Out asendmsg! outc=%d, outh=%d, outt=%d\n",outcount,outhead,outtail);

#ifdef DOSTAT 
if (statflag==1){
  jiastat.asendtime += get_usecs()-begin;
}
#endif 
}


void outsend()
{
  int res, toproc, fromproc;
  struct sockaddr_in  to,from;
  int  rep;
  int retries_num;
  unsigned long start, end;
  int msgsize;
  int s;
  int sendsuccess,arrived;
  fd_set readfds;
  int servemsg;
#ifdef DOSTAT
 register unsigned int begin;
#endif
  
  printf("Enter outsend!\n");

  if (msgprint==1) printmsg(&outqh,0);

  toproc = outqh.topid;
  fromproc = outqh.frompid;
  
  if (toproc == fromproc) {
    BEGINCS;
    assert0((incount<=Maxqueue), "outsend(): Inqueue exceeded!");
    commreq.rcv_seq[toproc] = outqh.seqno;
    memcpy(&(inqt),&(outqh), Msgheadsize+outqh.size);
    if (msgprint==1) printmsg(&(inqt),1);
    incount++;
    intail=(intail+1)%Maxqueue;
    servemsg=(incount==1)? 1 : 0;
    ENDCS;   
    SPACE(1);printf("Finishcopymsg,inc=%d,inh=%d,int=%d!\n",incount,inhead,intail);


    while (servemsg==1){
      msgserver();
      BEGINCS;
      inqh.op=ERRMSG;
      inhead=(inhead+1)%Maxqueue;
      incount--;
      servemsg=(incount>0)? 1 : 0;
      ENDCS;
    }
  }else{
    msgsize=outqh.size+Msgheadsize;

#ifdef DOSTAT
if (statflag==1){
    jiastat.msgsndcnt++;
    jiastat.msgsndbytes+=msgsize;
}
#endif

    to.sin_family = AF_INET;
    memcpy(&to.sin_addr,hosts[toproc].addr,hosts[toproc].addrlen);
    to.sin_port = htons(reqports[toproc][fromproc]);

    retries_num=0;
    sendsuccess =0;

    while ((retries_num<MAX_RETRIES)&&(sendsuccess!=1)) {
      BEGINCS;
      res=sendto(commreq.snd_fds[toproc], (char *)&(outqh),msgsize, 0,
                    (struct sockaddr *)&to, sizeof(to));
      assert0((res!=-1),"outsend()-->sendto()");
      ENDCS;

      arrived=0;
      start= jia_current_time();
      end  = start+TIMEOUT;

      while ((jia_current_time()<end)&&(arrived!=1)){
        FD_ZERO(&readfds);
        FD_SET(commrep.rcv_fds[toproc],&readfds);
        polltime.tv_sec=0;
        polltime.tv_usec=0;
        res = select(commrep.rcv_maxfd, &readfds,NULL,NULL,&polltime);
        if (FD_ISSET(commrep.rcv_fds[toproc],&readfds)!=0)
          arrived=1;
      }

      if (arrived==1) {
recv_again:
        s= sizeof(from);
        res = recvfrom(commrep.rcv_fds[toproc], (char *)&rep, Intbytes,0,
                        (struct sockaddr *)&from, &s);
        if ((res < 0) && (errno == EINTR)) goto recv_again;
        if ((res!=-1)&&(rep==outqh.seqno)) sendsuccess=1;
      }
         
      retries_num++;
    }

    if (sendsuccess!=1){
      sprintf(errstr,"Can't asend message(%d,%d) to host %d!",outqh.op, outqh.seqno, toproc); 
      printf("BUFFER SIZE %d(%d)\n", outqh.size, msgsize);
      assert0((sendsuccess==1),errstr);
    }
  }

  printf("Out outsend!\n");
} 
/*-------------------------------------------*/


void bsendmsg(jia_msg_t *msg)
{unsigned long root,level;

 msg->op+=BCAST;

 root=jia_pid;

 if (hostc==1){
   level=1;
 }else{
   for (level=0;(1<<level)<hostc;level++);
 }

 msg->temp=((root & 0xffff) <<16) | (level & 0xffff);

 bcastserver(msg); 
}


void bcastserver(jia_msg_t *msg)
{int mypid,child1,child2;
 int rootlevel, root, level;

 rootlevel=msg->temp;
 root= (rootlevel >> 16) & 0xffff;
 level=rootlevel & 0xffff; 
 level--;

 mypid=((jia_pid-root)>=0) ? (jia_pid-root) : (jia_pid-root+hostc);
 child1=mypid;
 child2=mypid+(1<<level);

 if (level==0) msg->op-=BCAST;
 msg->temp=((root & 0xffff) <<16) | (level & 0xffff);
 msg->frompid=jia_pid;

 if (child2<hostc){
   msg->topid=(child2+root)%hostc;
   asendmsg(msg); 
 }

 msg->topid=(child1+root)%hostc;
 asendmsg(msg);
}

#else  /* NULL_LIB */
#endif /* NULL_LIB */
