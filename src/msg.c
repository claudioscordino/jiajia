/***********************************************************************
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
#include "global.h"
#include "init.h"
#include "mem.h"
#include "syn.h"
#include "comm.h"
#include "msg.h"

extern void assert(int cond, char *errstr);
extern jia_msg_t *newmsg();
extern void freemsg(jia_msg_t*);
extern void printmsg(jia_msg_t *msg,int right);
extern void asendmsg(jia_msg_t *msg);
extern void broadcast(jia_msg_t *msg);
extern float jia_clock();

void jia_send(char *buf, int len, int toproc, int tag);
void msgrecvserver(jia_msg_t *req);
int nextpacket(int fromproc,int tag);
int nextmsg(char *buf, int len, int fromproc, int tag);
int jia_recv(char *buf, int len, int fromproc, int tag);
int thesizeof(int op);
void reduce(char* dest, char* source, int count, int op);
void jia_reduce(char* sendbuf, char* recvbuf, int count, int op, int root);

extern char errstr[Linesize];
extern int jia_pid;
extern int hostc;

volatile int recvwait,endofmsg;
jia_msg_t msgbuf[Maxmsgbufs];
unsigned long msgseqno;



void initmsg()
{int i,j,k;

 endofmsg=0;
 recvwait=0;
 msgseqno=0;
 for (i=0;i<Maxmsgbufs;i++){
   msgbuf[i].op=ERRMSG;
   msgbuf[i].frompid=0;
   msgbuf[i].topid=0;
   msgbuf[i].scope=0;
   msgbuf[i].temp=0;
   msgbuf[i].index=0;
   msgbuf[i].size=0;
   msgbuf[i].seqno=0;
 }
}



/******************** Message Passing Part*****************/

void jia_send(char *buf, int len, int toproc, int tag)
{jia_msg_t *req;
 int msgsize;
 char *msgptr;

 assert(((toproc<hostc)&&(toproc>=0)),"Incorrect message destination");

 msgsize=len;
 msgptr=buf;

 req=newmsg();
 req->frompid=jia_pid;
 req->topid=toproc;
 req->scope=tag; 

 while (msgsize > Maxmsgsize){
   req->op=MSGBODY;
   req->size=0;
   appendmsg(req,msgptr,Maxmsgsize);
   asendmsg(req);
   msgptr+=Maxmsgsize;
   msgsize-=Maxmsgsize;
 }

 req->op=MSGTAIL;
 req->size=0;
 appendmsg(req,msgptr,msgsize);
 asendmsg(req);

 freemsg(req);
}


void msgrecvserver(jia_msg_t *req)
{int i;
 int empty;

 msgseqno++;

 i=0;
 while ((msgbuf[i].op!=ERRMSG)&&(i<Maxmsgbufs)) i++;

 if (i<Maxmsgbufs){         /*This is empty place in the buf*/
   msgbuf[i].op=req->op;
   msgbuf[i].frompid=req->frompid;
   msgbuf[i].topid=req->topid;
   msgbuf[i].scope=req->scope;             /*This is tag*/
   msgbuf[i].seqno=msgseqno;
   msgbuf[i].size=req->size;
   memcpy(msgbuf[i].data,req->data,req->size);
   recvwait=0;
 }else{
   assert0(0,"Message Buffer Overflow!");
 }
}


int nextpacket(int fromproc,int tag)
/*Find next packet in the buf that meet fromproc and tag condition*/
{int i,index;
 unsigned long next;

 index=Maxmsgbufs;
 next=Maxmsgno;
 for (i=0;i<Maxmsgbufs;i++){
   if (((msgbuf[i].op==MSGTAIL)||(msgbuf[i].op==MSGBODY))&&
       ((fromproc==MSG_PROC_ALL)||(fromproc==msgbuf[i].frompid))&&
       ((tag==MSG_TAG_ALL)||(tag==msgbuf[i].scope))&&
       (msgbuf[i].seqno<next)){
     next=msgbuf[i].seqno;
     index=i;
   }
 }
 return(index);
}


int nextmsg(char *buf, int len, int fromproc, int tag)
{int i;
 int msgsize,size;
 
 msgsize=0;
 i=nextpacket(fromproc,tag);
 
 while ((endofmsg==0)&&(i!=Maxmsgbufs)){
   size=MIN(msgbuf[i].size,len-msgsize);
   if (size<0) size=0;
   memcpy(buf+msgsize,msgbuf[i].data,size);

   msgsize+=size;

   if (msgbuf[i].op==MSGTAIL) endofmsg=1;

   msgbuf[i].op=ERRMSG;

   if (endofmsg==0)
     i=nextpacket(fromproc,tag);
 }

 return(msgsize);
}


int jia_recv(char *buf, int len, int fromproc, int tag)
{int msgsize;

 endofmsg=0;

 disable_sigio();
 msgsize=nextmsg(buf, len, fromproc, tag);

 while (endofmsg==0){
   recvwait=1;
   enable_sigio();
   while(recvwait);
   disable_sigio();
   msgsize+=nextmsg(buf+msgsize, len-msgsize, fromproc, tag);
 }

 enable_sigio();

 return(msgsize);
}


int thesizeof(int op)
{int len;

 switch (op) {
   case SUM_INT: 
   case MAX_INT:
   case MIN_INT: len=sizeof(int);
                 break;
   case SUM_FLOAT: 
   case MAX_FLOAT:
   case MIN_FLOAT: len=sizeof(float);
                   break;
   case SUM_DOUBLE: 
   case MAX_DOUBLE:
   case MIN_DOUBLE: len=sizeof(double);
                    break;
   default: assert(0,"Incorrect operation in jia_reduce");
 }  
 return(len);
}


void reduce(char* dest, char* source, int count, int op)
{int k;

 switch (op){
   case SUM_INT: 
     for (k=0;k<count;k++){
       ((int *)dest)[k]+=((int *)source)[k];
     }
     break;
   case SUM_FLOAT: 
     for (k=0;k<count;k++){
       ((float *)dest)[k]+=((float *)source)[k];
     }
     break;
   case SUM_DOUBLE: 
     for (k=0;k<count;k++){
       ((double *)dest)[k]+=((double *)source)[k];
     }
     break;
   case MAX_INT: 
     for (k=0;k<count;k++){
       ((int *)dest)[k]=MAX(((int *)source)[k],((int *)dest)[k]);
     }
     break;
   case MAX_FLOAT: 
     for (k=0;k<count;k++){
       ((float *)dest)[k]=MAX(((float *)source)[k],((float *)dest)[k]);
     }
     break;
   case MAX_DOUBLE: 
     for (k=0;k<count;k++){
       ((double *)dest)[k]=MAX(((double *)source)[k],((double *)dest)[k]);
     }
     break;
   case MIN_INT: 
     for (k=0;k<count;k++){
       ((int *)dest)[k]=MIN(((int *)source)[k],((int *)dest)[k]);
     }
     break;
   case MIN_FLOAT: 
     for (k=0;k<count;k++){
       ((float *)dest)[k]=MIN(((float *)source)[k],((float *)dest)[k]);
     }
     break;
   case MIN_DOUBLE: 
     for (k=0;k<count;k++){
       ((double *)dest)[k]=MIN(((double *)source)[k],((double *)dest)[k]);
     }
     break;

   default: assert(0,"Incorrect operation in jia_reduce");
 }
}


void jia_reduce(char* sendbuf, char* recvbuf, int count, int op, int root)
{int j,hosts;
 int len,recvlen;
 int mypid,fromproc,toproc;
 char *tempbuf;

 assert((root<hostc)&&(root>=0),"Incorrect root in reduce");

 len=count*thesizeof(op);

 memcpy(recvbuf,sendbuf,len);
 tempbuf=malloc(len);

 mypid=((jia_pid-root)>=0) ? (jia_pid-root) : (jia_pid-root+hostc);

 for (j=0;(1<<j)<hostc;j++){
   if (mypid%(1<<j)==0){

     if ((mypid%(1<<(j+1)))!=0){
       toproc=(mypid-(1<<j)+root)%hostc;
       jia_send(recvbuf,len,toproc,REDUCE_TAG);
     }else if ((mypid+(1<<j))<hostc){
       fromproc=(mypid+(1<<j)+root)%hostc;
       recvlen=jia_recv(tempbuf,len,fromproc,REDUCE_TAG);
       assert((len==recvlen),"Unmatched length in jia_reduce");

       reduce(recvbuf, tempbuf, count, op); 
     }
   }
 }
 free(tempbuf);
}


void jia_bcast(char* buf, int len, int root)
{jia_msg_t *req;
 int msgsize;
 char *msgptr;

 if (jia_pid==root){
   msgsize=len;
   msgptr=buf;

   req=newmsg();
   req->frompid=jia_pid;
   req->topid=jia_pid;
   req->scope=BCAST_TAG; 

   while (msgsize > Maxmsgsize){
     req->op=MSGBODY;
     req->size=0;
     appendmsg(req,msgptr,Maxmsgsize);
     bsendmsg(req);
     msgptr+=Maxmsgsize;
     msgsize-=Maxmsgsize;
   }

   req->op=MSGTAIL;
   req->size=0;
   appendmsg(req,msgptr,msgsize);
   bsendmsg(req);

   freemsg(req);
 }

 jia_recv(buf,len,MSG_PROC_ALL,BCAST_TAG);
 
}

#else  /* NULL_LIB */

#endif /* NULL_LIB */



