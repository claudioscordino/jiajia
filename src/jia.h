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
 *          Author: Weiwu Hu, Weisong Shi, Zhimin Tang                 * 
 * =================================================================== *
 *   This software is ported to SP2 by                                 *
 *                                                                     *
 *         M. Rasit Eskicioglu                                         *
 *         University of Alberta                                       *
 *         Dept. of Computing Science                                  *
 *         Edmonton, Alberta T6G 2H1 CANADA                            *
 * =================================================================== *
 **********************************************************************/
 
#ifndef JIA_PUBLIC
#define	JIA_PUBLIC

#define jiahosts  hostc
#define jiapid    jia_pid

extern int	jia_pid;
extern int      hostc;
 
void           jia_init(int, char **);
void           jia_exit();

unsigned long  jia_alloc3(int,int,int);
unsigned long  jia_alloc2(int,int);
unsigned long  jia_alloc2p(int, int);
unsigned long  jia_alloc1(int);
unsigned long  jia_alloc(int);

void           jia_lock(int);
void           jia_unlock(int);
void           jia_barrier();
void           jia_wait();
void           jia_setcv(int);
void           jia_resetcv(int);
void           jia_waitcv(int);

void           jia_error(char*, ...);
unsigned int   jia_startstat();
unsigned int   jia_stopstat();
float          jia_clock();

void           jia_send(char*, int, int, int);
int            jia_recv(char*, int, int, int);
void           jia_reduce(char*, char*, int, int, int);
void           jia_bcast(char*, int, int);

void           jia_config(int,int);

void           jia_divtask(int*,int*);
void           jia_loadcheck();

#ifndef ON
#define OFF    0
#define ON     1
#endif

#define HMIG       0
#define ADWD       1
#define BROADCAST  2
#define LOADBAL    3 
#define WVEC       4 

#define jia_wtnt(a,b) 
#define jia_wtntw(a) 
#define jia_wtntd(a) 
#define jia_errexit jia_error

/* argonne (ANL) macros */

extern int      jia_lock_index;

#define Maxlocks   64
#define	LOCKDEC(x)	int	x;
#ifndef NULL_LIB
#define	LOCKINIT(x)     (x)=(jia_lock_index++)%Maxlocks;
#else /* NULL_LIB */
#define	LOCKINIT(x)     
#endif /* NULL_LIB */
#define	LOCK(x)		jia_lock(x);
#define	UNLOCK(x)	jia_unlock(x);

#define	ALOCKDEC(x,y)	int x[y];
#define	ALOCKINIT(x,y)	{						\
			int	i;	 			\
			for (i=0;i<(y);i++){ 			\
				(x)[i] = (jia_lock_index++)%Maxlocks;	\
			}					\
			}
#define	ALOCK(x,y)	jia_lock((x)[y]);
#define	AULOCK(x,y)	jia_unlock((x)[y]);

#define	BARDEC(x)	int	x;
#define	BARINIT(x)
#define	BARRIER(x,y)	jia_barrier();

#define	EXTERN_ENV		
#define	MAIN_ENV		
#define	MAIN_INITENV(x,y)			
#define	MAIN_END 
#define	WAIT_FOR_END(x)	jia_barrier();

#define	PAUSEDEC(x)	int	x;
#define	PAUSEINIT(x)	
#define	PAUSE_POLL(x)
#define	PAUSE_SET(x,y)	
#define	PAUSE_RESET(x)	
#define	PAUSE_ACCEPT(x)	

#define	GSDEC(x)	int	x;
#define	GSINIT(x)	
#define	GETSUB(x, y, z, zz)	
/*
#define	G_MALLOC(x)	jia_alloc3((x),(x)/jiahosts,0);
*/
#define	G_MALLOC(x)	jia_alloc1((x));
#define	G2_MALLOC(x,y)	jia_alloc3(x);

#define	CLOCK(x)	x=jia_clock();

#endif /* JIA_PUBLIC */
