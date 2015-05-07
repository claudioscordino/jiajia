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

#ifndef JIASYN_H
#define JIASYN_H

#define Maxwtnts     511     /*(4096-8)/4/2*/
#define Maxstacksize 8
#define hidelock     Maxlocks
#define top          lockstack[stackptr]
#define stol(x)      (*((unsigned long *) (x)))
#define ltos(x)      ((unsigned char *) (&(x)))
#define sbit(s,n)    ((s[(n)/8])|=((unsigned char)(0x1<<((n)%8))))
#define cbit(s,n)    ((s[(n)/8])&=(~((unsigned char)(0x1<<((n)%8)))))
#define tbit(s,n)    (((s[(n)/8])&((unsigned char)(0x1<<((n)%8))))!=0)   
#define WNULL        ((wtnt_t*)NULL)
#define Maxcvs         16


typedef struct wtnttype {
        unsigned char*  wtnts[Maxwtnts];
        int             from[Maxwtnts];   /*from pid or from scope*/
        int             wtntc;
        struct wtnttype *more;
               } wtnt_t;

typedef struct locktype {
        int         acqs[Maxhosts];
        int         acqscope[Maxhosts];
        int         acqc;
        int         scope;
        int         myscope;
        wtnt_t      *wtntp;
               } jialock_t;

typedef struct stacktype {
        int         lockid;
        wtnt_t      *wtntp;
               } jiastack_t;

typedef struct cvtype {
        int         waits[Maxhosts];  /*hosts waiting on cv*/
        int         waitc;
        int         value;
               } jiacv_t;

#endif /*JIASYN_H*/
