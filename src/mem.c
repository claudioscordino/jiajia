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
 *             Author: Weiwu Hu, Weisong Shi, Zhimin Tang              *      
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
#include "mem.h"
#include "init.h"
#include "comm.h"
#include "syn.h"

#ifdef IRIX62
#include <sys/sbd.h>
#endif /* IRIX62 */

extern void assert(int cond, char *errstr);
extern jia_msg_t *newmsg();
extern void freemsg();
extern void asendmsg(jia_msg_t *req);
extern void savepage(int cachei);
extern void newtwin(address_t *twin);
extern void freetwin(address_t *twin);
extern void enable_sigio();
extern void disable_sigio();
extern void jia_barrier();
extern float jia_clock();

void initmem();
void getpage(address_t addr,int flag);
int xor(address_t addr);
void flushpage(int cachei);
int replacei(int cachei);
int findposition(address_t addr);
#ifdef SOLARIS
void sigsegv_handler(int signo, siginfo_t *sip, ucontext_t *uap);
#endif /* SOLARIS */

#if defined AIX41 || defined IRIX62
void sigsegv_handler();
#endif /* AIX41 || IRIX62 */

#ifdef LINUX 
void sigsegv_handler(int, struct sigcontext_struct);
#endif

void getpserver(jia_msg_t *req);
void getpgrantserver(jia_msg_t *req);
void addwtvect(int homei,wtvect_t wv,int from);
void setwtvect(int homei,wtvect_t wv);
unsigned long s2l(unsigned char *str);
void diffserver(jia_msg_t *req);
void diffgrantserver(jia_msg_t *req);
int  encodediff(int diffi, unsigned char* diff);
void senddiffs();
void savediff(int cachei);

extern int         jia_pid;
extern host_t      hosts[Maxhosts];
extern int         hostc;
extern char        errstr[Linesize];
extern jiastack_t  lockstack[Maxstacksize];
extern jialock_t   locks[Maxlocks+1];
extern int         stackptr;
extern int         H_MIG,B_CAST,W_VEC;

#ifdef DOSTAT
extern jiastat_t jiastat;
extern int statflag;
#endif

jiahome_t       home[Homepages+1];
jiacache_t      cache[Cachepages+1];
jiapage_t       page[Maxmempages];

unsigned long   globaladdr;
long            jiamapfd;
volatile int getpwait;
volatile int diffwait;
int repcnt[Setnum];
jia_msg_t *diffmsg[Maxhosts];


void initmem()
{int i,j;

 for (i=0;i<Maxhosts;i++){
   diffmsg[i]=DIFFNULL;
 }
 diffwait=0;
 getpwait=0;

 for (i=0;i<=hostc;i++){
   hosts[i].homesize=0;
 }

 for (i=0;i<=Homepages;i++){
   home[i].wtnt=0;
   home[i].rdnt=0;
   home[i].addr=(address_t)0;
   home[i].twin=NULL;
 }

 for (i=0;i<Maxmempages;i++){
   page[i].cachei=(unsigned short)Cachepages;
   page[i].homei=(unsigned short)Homepages;
   page[i].homepid=(unsigned short)Maxhosts;
 }

 for (i=0;i<=Cachepages;i++){
   cache[i].state=UNMAP;
   cache[i].addr=0;
   cache[i].twin=NULL;
   cache[i].wtnt=0;
 }

 globaladdr=0;

#if defined SOLARIS || defined IRIX62
 jiamapfd=open("/dev/zero", O_RDWR,0);

 { struct sigaction act;

   act.sa_handler = (void_func_handler)sigsegv_handler;
   sigemptyset(&act.sa_mask);
   act.sa_flags = SA_SIGINFO;
   if (sigaction(SIGSEGV, &act, NULL))
     assert0(0,"segv sigaction problem");
 }
#endif 

#ifdef LINUX
 jiamapfd=open("/dev/zero", O_RDWR,0);
 { struct sigaction act;
   act.sa_handler = (void_func_handler)sigsegv_handler;
   sigemptyset(&act.sa_mask);
   act.sa_flags = SA_NOMASK;
   if (sigaction(SIGSEGV, &act, NULL))
     assert0(0,"segv sigaction problem");
 }
#endif 

#ifdef AIX41
  { struct sigvec vec;

    vec.sv_handler = (void_func_handler)sigsegv_handler;
    vec.sv_flags = SV_INTERRUPT;
    sigvec(SIGSEGV, &vec, NULL);
  }
#endif /* SOLARIS */
 
 for (i=0;i<Setnum;i++) repcnt[i]=0;
 srand(1);
}


void memprotect(caddr_t addr,size_t len, int prot)                               
{int protyes;                                            

 protyes=mprotect(addr,len,prot);  
 if (protyes!=0) {                                        
   sprintf(errstr,"mprotect failed! addr=0x%x, errno=%d",(unsigned long)addr,errno); 
   assert(0,errstr);                          
 }                                                       
}


void memmap(caddr_t addr,size_t len,int prot)                               
{caddr_t mapad;                                                     

#if defined SOLARIS || defined IRIX62 || defined LINUX
  mapad=mmap(addr,len,prot,MAP_PRIVATE|MAP_FIXED,jiamapfd,0);
#endif 
#ifdef AIX41
  mapad=mmap(addr,len,prot,MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS,-1,0);
#endif 

  if (mapad != addr){                                      
    sprintf(errstr,"mmap failed! addr=0x%x, errno=%d",(unsigned long)(addr),errno);
    assert(0,errstr);                                                
  }                                                                  
}


void memunmap(caddr_t addr,size_t len)                               
{int unmapyes;                                            

 unmapyes=munmap(addr,len);  
 if (unmapyes!=0) {                                        
   sprintf(errstr,"munmap failed! addr=0x%x, errno=%d",(unsigned long)addr,errno); 
   assert(0,errstr);                          
 }                                                       
}


unsigned long jia_alloc3(int size,int block, int starthost)
{int homepid;
 int mapsize;
 int allocsize;
 int originaddr;
 int homei,pagei,i;
 int protect;
 
 sprintf(errstr, 
         "Insufficient shared space! --> Max=0x%x Left=0x%x Need=0x%x\n",
         Maxmemsize, Maxmemsize-globaladdr,size);

 assert(((globaladdr+size)<=Maxmemsize), errstr);

 originaddr=globaladdr;
 allocsize=((size%Pagesize)==0)?(size):((size/Pagesize+1)*Pagesize); 
 mapsize=((block%Pagesize)==0)?(block):((block/Pagesize+1)*Pagesize); 
 homepid=starthost;

 while (allocsize>0){
  if (jia_pid==homepid){
     assert((hosts[homepid].homesize+mapsize)<(Homepages*Pagesize),"Too many home pages");

     protect=(hostc==1) ? PROT_READ|PROT_WRITE : PROT_READ;
     memmap((caddr_t)(Startaddr+globaladdr),(size_t)mapsize,protect);

     for (i=0;i<mapsize;i+=Pagesize){
       pagei=(globaladdr+i)/Pagesize;
       homei=(hosts[homepid].homesize+i)/Pagesize;
       home[homei].addr=(address_t)(Startaddr+globaladdr+i);
       page[pagei].homei=homei;
     }
   }

   for (i=0;i<mapsize;i+=Pagesize){
     pagei=(globaladdr+i)/Pagesize;
     page[pagei].homepid=homepid;
   }

#ifdef JIA_DEBUG
#endif 
   printf("Map 0x%x bytes in home %4d! globaladdr = 0x%x\n",mapsize,homepid,globaladdr);

   hosts[homepid].homesize+=mapsize;
   globaladdr+=mapsize;
   allocsize-=mapsize;   
   homepid=(homepid+1)%hostc;
 }

 return(Startaddr+originaddr);
}


unsigned long jia_alloc3b(int size,int *block, int starthost)
{int homepid;
 int mapsize;
 int allocsize;
 int originaddr;
 int homei,pagei,i;
 int blocki;
 int protect;
 
 sprintf(errstr, 
         "Insufficient shared space! --> Max=0x%x Left=0x%x Need=0x%x\n",
         Maxmemsize, Maxmemsize-globaladdr,size);
 
 assert(((globaladdr+size)<=Maxmemsize), errstr);

 blocki=0; 
 originaddr=globaladdr;
 allocsize=((size%Pagesize)==0)?(size):((size/Pagesize+1)*Pagesize);
 homepid=starthost;
 
 while (allocsize>0){
   mapsize=((block[blocki]%Pagesize)==0)?(block[blocki]):((block[blocki]/Pagesize+1)*Pagesize);
   if (jia_pid==homepid){
     assert((hosts[homepid].homesize+mapsize)<(Homepages*Pagesize),"Too many home pages");

     protect=(hostc==1) ? PROT_READ|PROT_WRITE : PROT_READ;
     memmap((caddr_t)(Startaddr+globaladdr),(size_t)mapsize,protect);

     for (i=0;i<mapsize;i+=Pagesize){
       pagei=(globaladdr+i)/Pagesize;
       homei=(hosts[homepid].homesize+i)/Pagesize;
       home[homei].addr=(address_t)(Startaddr+globaladdr+i);
       page[pagei].homei=homei;
     }
   }
     
   for (i=0;i<mapsize;i+=Pagesize){
     pagei=(globaladdr+i)/Pagesize;
     page[pagei].homepid=homepid;
   }
 
#ifdef JIA_DEBUG
#endif 
   printf("Map 0x%x bytes in home %4d! globaladdr = 0x%x\n",mapsize,homepid,globaladdr);
      
   hosts[homepid].homesize+=mapsize;
   globaladdr+=mapsize;
   allocsize-=mapsize;
   homepid=(homepid+1)%hostc;
   if (homepid==0) blocki++;
 }
 
 return(Startaddr+originaddr);
}


unsigned long jia_alloc(int size)
{static int starthost=-1;

  starthost=(starthost+1)%hostc;
  return(jia_alloc3(size,size,starthost));
}

unsigned long jia_alloc1(int size)
{
  return(jia_alloc3(size,size,0));
}

unsigned long jia_alloc2(int size, int block)
{
  return(jia_alloc3(size,block,0));
}

unsigned long jia_alloc2p(int size, int proc)
{
  return(jia_alloc3(size,size,proc));
}


int xor(address_t addr)
{
 return((((unsigned long)(addr-Startaddr)/Pagesize)%Setnum)*Setpages);
}


int replacei(int cachei)
{int seti;

 if (REPSCHEME==0)
   return((random()>>8)%Setpages);
 else{
   seti=cachei/Setpages;
   repcnt[seti]=(repcnt[seti]+1)%Setpages;
   return(repcnt[seti]);
 } 
}


void flushpage(int cachei)
{
 memunmap((caddr_t)cache[cachei].addr,Pagesize);

 page[((unsigned long)cache[cachei].addr-Startaddr)/Pagesize].cachei=Cachepages;

 if (cache[cachei].state==RW) freetwin(&(cache[cachei].twin));
 cache[cachei].state=UNMAP;
 cache[cachei].wtnt=0;
 cache[cachei].addr=0;
}


int findposition(address_t addr)
{int cachei;             /*index of cache*/
 int seti;               /*index in a set*/
 int invi;
 int i;
 
   cachei=xor(addr);
   seti=replacei(cachei);
   invi=-1;
   for (i=0;(cache[cachei+seti].state!=UNMAP)&&(i<Setpages);i++){

     if ((invi==(-1))&&(cache[cachei+seti].state==INV))
       invi=seti;

     seti=(seti+1)%Setpages;
   }

   if ((cache[cachei+seti].state!=UNMAP)&&(invi!=(-1))){
     seti=invi;
   }   

   if ((cache[cachei+seti].state==INV)||(cache[cachei+seti].state==RO)){
     flushpage(cachei+seti);
#ifdef DOSTAT
if (statflag==1){
     if (cache[cachei+seti].state==RO) jiastat.repROcnt++;
}
#endif
   }else if (cache[cachei+seti].state==RW){
     savepage(cachei+seti);
     senddiffs();
     while(diffwait);
     flushpage(cachei+seti);
#ifdef DOSTAT
if (statflag==1){
     jiastat.repRWcnt++;
}
#endif
   }
   page[((unsigned long)addr-Startaddr)/Pagesize].cachei=(unsigned short)(cachei+seti);
   return(cachei+seti);
}


#ifdef SOLARIS 
void sigsegv_handler(int signo, siginfo_t *sip, ucontext_t *uap)
#endif 

#if defined AIX41 || defined IRIX62
void sigsegv_handler(int signo, int code, struct sigcontext *scp, char *addr) 
#endif 

#ifdef LINUX
void sigsegv_handler(int signo, struct sigcontext_struct sigctx)
#endif 
{address_t faultaddr;
 int writefault;
 int cachei,homei;

 sigset_t set;
	
#ifdef DOSTAT
 register unsigned int begin = get_usecs();
if (statflag==1){
 jiastat.kernelflag=2;
}
#endif

 sigemptyset(&set);
 sigaddset(&set,SIGIO);
 sigprocmask(SIG_UNBLOCK, &set, NULL);

#ifdef SOLARIS
 faultaddr=(address_t)sip->si_addr;
 faultaddr=(address_t)((unsigned long)faultaddr/Pagesize*Pagesize);
 writefault=(int)(*(unsigned *)uap->uc_mcontext.gregs[REG_PC] & (1<<21));
#endif 

#ifdef AIX41 
 faultaddr = (char *)scp->sc_jmpbuf.jmp_context.o_vaddr;
 faultaddr = (address_t)((unsigned long)faultaddr/Pagesize*Pagesize);
 writefault = (scp->sc_jmpbuf.jmp_context.except[1] & DSISR_ST) >> 25;
#endif 

#ifdef IRIX62
 faultaddr=(address_t)scp->sc_badvaddr;
 faultaddr=(address_t)((unsigned long)faultaddr/Pagesize*Pagesize);
 writefault=(int)(scp->sc_cause & EXC_CODE(1));
#endif 

#ifdef LINUX 
 faultaddr = (address_t) sigctx.cr2;
 faultaddr = (address_t) ((unsigned long) faultaddr/Pagesize*Pagesize);
 writefault = (int) sigctx.err &2;
#endif 

 sprintf(errstr,"Access shared memory out of range from 0x%x to 0x%x!, 
                 faultaddr=0x%x, writefault=0x%x", 
                 Startaddr,Startaddr+globaladdr,faultaddr, writefault);
 assert((((unsigned long)faultaddr<(Startaddr+globaladdr))&& 
         ((unsigned long)faultaddr>=Startaddr)), errstr);


 if (homehost(faultaddr)==jia_pid){
   memprotect((caddr_t)faultaddr,Pagesize,PROT_READ|PROT_WRITE);
   homei=homepage(faultaddr);
   home[homei].wtnt|=3;
   if ((W_VEC==ON)&&(home[homei].wvfull==0)){
     newtwin(&(home[homei].twin));
     memcpy(home[homei].twin,home[homei].addr,Pagesize);
   }
#ifdef DOSTAT
if (statflag==1){
 jiastat.segvLtime += get_usecs() - begin;
 jiastat.kernelflag=0;
 jiastat.segvLcnt++;
}
#endif

 }else{
   writefault=(writefault==0) ? 0 : 1;
   cachei=(int)page[((unsigned long)faultaddr-Startaddr)/Pagesize].cachei;
   if (cachei<Cachepages){
     memprotect((caddr_t)faultaddr,Pagesize,PROT_READ|PROT_WRITE);
     if (!((writefault)&&(cache[cachei].state==RO))){
       getpage(faultaddr,1);
     }
   }else{
     cachei=findposition(faultaddr);
     memmap((caddr_t)faultaddr,Pagesize,PROT_READ|PROT_WRITE);
     getpage(faultaddr,0);
   }

   if (writefault){
     cache[cachei].addr=faultaddr;
     cache[cachei].state=RW;
     cache[cachei].wtnt=1;
     newtwin(&(cache[cachei].twin));
     while(getpwait) ;
     memcpy(cache[cachei].twin,faultaddr,Pagesize);
   }else{
     cache[cachei].addr=faultaddr;
     cache[cachei].state=RO;
     while(getpwait) ;
     memprotect((caddr_t)faultaddr,(size_t)Pagesize,PROT_READ);
   }

#ifdef DOSTAT
if (statflag==1){
 jiastat.segvRcnt++;
 jiastat.segvRtime += get_usecs() - begin;
 jiastat.kernelflag=0;
}
#endif
 }
}


void getpage(address_t addr,int flag)
{int homeid;
 jia_msg_t *req;

 homeid=homehost(addr); 
 assert((homeid!=jia_pid),"This should not have happened 2!");
 req=newmsg();

 req->op=GETP;
 req->frompid=jia_pid;
 req->topid=homeid;
 req->temp=flag;       /*0:read request,1:write request*/
 req->size=0;
 appendmsg(req,ltos(addr),Intbytes);

 getpwait=1;
 asendmsg(req);

 freemsg(req);
/*
 while(getpwait) ;
*/
#ifdef DOSTAT
if (statflag==1){
 jiastat.getpcnt++;
}
#endif
}


void getpserver(jia_msg_t *req)
{address_t paddr; 
 int homei;
 jia_msg_t *rep;

 assert((req->op==GETP)&&(req->topid==jia_pid),"Incorrect GETP Message!");

 paddr=(address_t)stol(req->data);
/*
 printf("getpage=0x%x from host %d\n",(unsigned long) paddr,req->frompid);
*/
 if ((H_MIG==ON)&&(homehost(paddr)!=jia_pid)){
   /*This is a new home, the home[] data structure may
     not be updated, but the page has already been here
     the rdnt item of new home is set to 1 in migpage()*/
 }else{
   assert((homehost(paddr)==jia_pid),"This should have not been happened! 4");
   homei=homepage(paddr);

   if ((W_VEC==ON)&&(home[homei].wvfull==1)){
     home[homei].wvfull=0;
     newtwin(&(home[homei].twin));
     memcpy(home[homei].twin,home[homei].addr,Pagesize);
   }

   home[homei].rdnt=1;
 }
 rep=newmsg();
 rep->op=GETPGRANT;
 rep->frompid=jia_pid;
 rep->topid=req->frompid;
 rep->temp=0;
 rep->size=0;
 appendmsg(rep,req->data,Intbytes);
 
 if ((W_VEC==ON)&&(req->temp==1)){int i;
   for (i=0;i<Wvbits;i++){
     if (((home[homei].wtvect[req->frompid]) & (((wtvect_t)1)<<i))!=0){
       appendmsg(rep,paddr+i*Blocksize,Blocksize);
     }
   }
   rep->temp=home[homei].wtvect[req->frompid];
 }else{
   appendmsg(rep,paddr,Pagesize);
   rep->temp=WVFULL;
 }

 if (W_VEC==ON){
   home[homei].wtvect[req->frompid]=WVNULL;
/*
   printf("0x%x\n",rep->temp);
*/
 }

 asendmsg(rep);
 freemsg(rep);
}


void getpgrantserver(jia_msg_t *rep)
{address_t addr;
 unsigned int datai;
 unsigned long wv;
 int i;
	
 assert((rep->op==GETPGRANT),"Incorrect returned message!");
 
 wv=rep->temp;

 datai=0;
 addr=(address_t)stol(rep->data+datai);
 datai+=Intbytes;

 if ((W_VEC==ON)&&(wv!=WVFULL)){
   for (i=0;i<Wvbits;i++){
     if ((wv & (((wtvect_t)1)<<i))!=0){
       memcpy(addr+i*Blocksize,rep->data+datai,Blocksize);
       datai+=Blocksize;
     }
   }
 }else{
   memcpy(addr,rep->data+datai,Pagesize);
 }

 getpwait=0;
}


void addwtvect(int homei,wtvect_t wv,int from)
{int i;

 home[homei].wvfull=1;
 for (i=0;i<hostc;i++){
   if (i!=from) home[homei].wtvect[i] |= wv;

   if (home[homei].wtvect[i]!=WVFULL) home[homei].wvfull=0;
 }
}


void setwtvect(int homei,wtvect_t wv)
{int i;

 home[homei].wvfull=1;
 for (i=0;i<hostc;i++){
   home[homei].wtvect[i] = wv;
   if (home[homei].wtvect[i]!=WVFULL) home[homei].wvfull=0;
 }
}


unsigned long s2l(unsigned char *str)
{union {
   unsigned long l;
   unsigned char c[Intbytes];
       } notype;
 
 notype.c[0]=str[0];
 notype.c[1]=str[1];
 notype.c[2]=str[2];
 notype.c[3]=str[3];

 return(notype.l);
}


void diffserver(jia_msg_t *req)
{int datai;
 int homei;
 unsigned long pstop,doffset,dsize;
 unsigned long paddr;
 jia_msg_t *rep;
 wtvect_t wv;

#ifdef DOSTAT
 register unsigned int begin = get_usecs();
#endif

 assert((req->op==DIFF)&&(req->topid==jia_pid),"Incorrect DIFF Message!");

 datai=0;
 while(datai<req->size){
   paddr=s2l(req->data+datai);
   datai+=Intbytes;
   wv=WVNULL;

   homei=homepage(paddr);
   /* In the case of H_MIG==ON, homei may be the index of 
      the new home[] which has not be updated due to difference
      of barrier arrival. In this case, homei==Homepages and
      home[Homepages].wtnt==0*/

   if ((home[homei].wtnt&1)!=1)
     memprotect((caddr_t)paddr,Pagesize,PROT_READ|PROT_WRITE);

   pstop=s2l(req->data+datai)+datai-Intbytes;
   datai+=Intbytes;
   while(datai<pstop){
     dsize=s2l(req->data+datai) & 0xffff;
     doffset=(s2l(req->data+datai)>>16) & 0xffff;
     datai+=Intbytes;
     memcpy((address_t)(paddr+doffset),req->data+datai,dsize);
     datai+=dsize;

     if ((W_VEC==ON)&&(dsize>0)) {int i;
       for(i=doffset/Blocksize*Blocksize;i<(doffset+dsize);i+=Blocksize)
         wv |= ((wtvect_t)1)<<(i/Blocksize);
     }
   }

   if (W_VEC==ON) addwtvect(homei,wv,req->frompid);

   if ((home[homei].wtnt&1)!=1)
     memprotect((caddr_t)paddr,(size_t)Pagesize,(int)PROT_READ);

#ifdef DOSTAT
   if (statflag==1){
     jiastat.mwdiffcnt++;
   }
#endif
 }

#ifdef DOSTAT
if (statflag==1){
 jiastat.dedifftime += get_usecs() - begin;
 jiastat.diffcnt++;
}
#endif

 rep=newmsg();
 rep->op=DIFFGRANT;
 rep->frompid=jia_pid;
 rep->topid=req->frompid;
 rep->size=0;

 asendmsg(rep);
 freemsg(rep);
}


int encodediff(int cachei, unsigned char* diff)
{int size;
 int bytei;
 unsigned long cnt;
 unsigned long start;
 unsigned long header;

#ifdef DOSTAT
 register unsigned int begin = get_usecs();
#endif

   size=0;
   memcpy(diff+size,ltos(cache[cachei].addr),Intbytes);
   size+=Intbytes;
   size+=Intbytes;                       /*leave space for size*/

   bytei=0;
   while (bytei<Pagesize){
     for (; (bytei<Pagesize)&&(memcmp(cache[cachei].addr+bytei,
           cache[cachei].twin+bytei,Diffunit)==0); bytei+=Diffunit);
     if (bytei<Pagesize){
       cnt=(unsigned long) 0;
       start=(unsigned long) bytei;
       for (; (bytei<Pagesize)&&(memcmp(cache[cachei].addr+bytei,
             cache[cachei].twin+bytei,Diffunit)!=0); bytei+=Diffunit) 
         cnt+=Diffunit;
       header=((start & 0xffff)<<16)|(cnt & 0xffff);
       memcpy(diff+size,ltos(header),Intbytes);
       size+=Intbytes;
       memcpy(diff+size,cache[cachei].addr+start,cnt);
       size+=cnt;   
     }
   }
   memcpy(diff+Intbytes,ltos(size),Intbytes);    /*fill size*/

#ifdef DOSTAT
if (statflag==1){
 jiastat.endifftime += get_usecs() - begin;
}
#endif

 return(size);
}


void savediff(int cachei)
{unsigned char  diff[Maxmsgsize];
 int   diffsize;
 int hosti;
  
 hosti=homehost(cache[cachei].addr);
 if (diffmsg[hosti]==DIFFNULL){
   diffmsg[hosti]=newmsg();
   diffmsg[hosti]->op=DIFF; 
   diffmsg[hosti]->frompid=jia_pid; 
   diffmsg[hosti]->topid=hosti; 
   diffmsg[hosti]->size=0; 
 }
 diffsize=encodediff(cachei,diff);
 if ((diffmsg[hosti]->size+diffsize)>Maxmsgsize){
   diffwait++;
   asendmsg(diffmsg[hosti]);
   diffmsg[hosti]->size=0;
   appendmsg(diffmsg[hosti],diff,diffsize);
   while(diffwait);
 }else{
   appendmsg(diffmsg[hosti],diff,diffsize);
 }
}


void senddiffs()
{int hosti;
 
 for (hosti=0;hosti<hostc;hosti++){
   if (diffmsg[hosti]!=DIFFNULL){
     if (diffmsg[hosti]->size>0){
       diffwait++;
       asendmsg(diffmsg[hosti]);
     }
     freemsg(diffmsg[hosti]);
     diffmsg[hosti]=DIFFNULL;
   }
 }
/*
 while(diffwait);
*/
}


void diffgrantserver(jia_msg_t *rep)
{
 assert((rep->op==DIFFGRANT)&&(rep->size==0),"Incorrect returned message!");

 diffwait--;
}


#else  /* NULL_LIB */

unsigned long jia_alloc(int size)
{
   return (unsigned long) valloc(size);
}
#endif /* NULL_LIB */
