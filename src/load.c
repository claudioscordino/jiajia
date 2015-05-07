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
 **********************************************************************/

#ifndef NULL_LIB
#include "global.h"
#include "init.h"
#include "mem.h"
#include "syn.h"
#include "comm.h"
#include "load.h"

extern void assert(int cond, char *errstr);
extern jia_msg_t *newmsg();
extern void freemsg(jia_msg_t*);
extern void asendmsg(jia_msg_t *msg);
extern void broadcast(jia_msg_t *msg);
extern float jia_clock();

extern int jia_pid;
extern int hostc;
extern int LOAD_BAL;

float caltime,starttime,endtime;
int   firsttime;
int   loadstart,loadend;
volatile int loadwait,loadcnt;

jiaload_t loadstat[Maxhosts];


void initload()
{int i,j,k;

 loadwait=0;
 loadcnt=0;
 firsttime=1;
 caltime=0.0;
 
}


void jia_loadbalance()
{
 jia_msg_t *req;

 req=newmsg();
 req->frompid=jia_pid;
 req->topid=0;
 req->op=LOADREQ;
 req->size=0;
 appendmsg(req,(unsigned char*)(&caltime),Intbytes);

 loadwait=1;
 asendmsg(req);

 while(loadwait);
 
 freemsg(req);
}


void jia_newload()
{float Ptotal=0.0; 
 float ex, ex2, sigma;
 int i;

 ex2 = 0.0;
 ex  = 0.0;
 for(i=0; i<hostc;i++){
   ex  += loadstat[i].time;
   ex2 += loadstat[i].time*loadstat[i].time;
 }

 ex  = ex/hostc;
 ex2 = ex2/hostc;
 sigma = ex2-ex*ex;
 
 if (sigma/(ex*ex)<(Delta*Delta)){
 }else{
   Ptotal=0.0;
   for(i=0; i<hostc;i++){
     loadstat[i].power /= loadstat[i].time;
     Ptotal+=loadstat[i].power;
   }
   for(i=0; i<hostc;i++){
     loadstat[i].power /= Ptotal;
     printf(" loadstat[%d].power = %.2f\n", i, loadstat[i].power);
   }
 }
}


void loadserver(jia_msg_t *req)
{int datai,hosti;
 jia_msg_t *grant;

 assert((req->op==LOADREQ),"Incorrect LOADREQ msg");

 datai=0;
 hosti=req->frompid;
 loadstat[hosti].time=*((float*)(req->data+datai));
 
 loadcnt++;
 
 if (loadcnt==hostc){
   loadcnt=0;
   jia_newload();
   grant=newmsg();
   grant->frompid=jia_pid;
   grant->op=LOADGRANT;
   grant->size=0;

   for (hosti=0;hosti<hostc;hosti++){
     appendmsg(grant,(unsigned char*)(&(loadstat[hosti].power)),
                     sizeof(loadstat[hosti].power));
   }

   broadcast(grant);

   freemsg(grant);
 }
}


void loadgrantserver(jia_msg_t *grant)
{int datai;
 int i;

 assert((grant->op==LOADGRANT),"Incorrect LOADGRANT msg");

 datai=0;
 for (i=0;i<hostc;i++){
   loadstat[i].power=*((float *)(grant->data+datai));
   datai+=sizeof(loadstat[i].power);
 }

 loadwait=0;
}


void jia_loadcheck()
{
 if (LOAD_BAL==ON){
   endtime=jia_clock();
   caltime+=(endtime-starttime);
   jia_loadbalance();
   caltime=0.0;  
   starttime=jia_clock();  
 }
}

void jia_divtask(int *begin, int *end)
{int i;
 int iternum;

 if (hostc==1) return;

 loadstart=(*begin);
 loadend=(*end);
 iternum=loadend-loadstart+1;

 if (LOAD_BAL==ON){
   if (firsttime){
     firsttime=0;
     for (i=0;i<hostc;i++){
       loadstat[i].power=1.0/hostc;
     }
     caltime=0.0;
     starttime=jia_clock();  
   }

   loadstat[0].begin=loadstart;
   for (i=0;i<hostc-1; i++){
     loadstat[i].end=loadstat[i].begin+loadstat[i].power*iternum;
     if (loadstat[i].end>loadend) loadstat[i].end=loadend;
     loadstat[i+1].begin=loadstat[i].end+1;
   }
   loadstat[hostc-1].end=loadend;

   *begin=loadstat[jia_pid].begin;
   *end=loadstat[jia_pid].end;
 }else{
   *begin=loadstart+(loadend-loadstart+1)/hostc*jia_pid;
   *end=*begin+(loadend-loadstart+1)/hostc-1;
   if (jia_pid==(hostc-1)) *end=loadend;
 }
}



#else  /* NULL_LIB */
void jia_divtask(int *begin, int *end)
{
}
#endif /* NULL_LIB */



