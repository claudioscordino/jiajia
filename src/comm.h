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
 *           Author: Weiwu Hu, Weisong Shi, Zhimin Tang                * 
 * =================================================================== *
 *   This software is ported to SP2 by                                 *
 *                                                                     *
 *         M. Rasit Eskicioglu                                         *
 *         University of Alberta                                       *
 *         Dept. of Computing Science                                  *
 *         Edmonton, Alberta T6G 2H1 CANADA                            *
 * =================================================================== *
 **********************************************************************/

#ifndef	JIACOMM_H
#define	JIACOMM_H

#include "global.h"
#include "init.h"

#define TIMEOUT      1000
#define MAX_RETRIES  64

#define Maxmsgsize   (40960-Msgheadsize) 
#define Msgheadsize  32
#define Maxmsgs      8 
#define Maxqueue     32

#define  DIFF      0
#define  DIFFGRANT 1     
#define  GETP      2  
#define  GETPGRANT 3     
#define  ACQ       4
#define  ACQGRANT  5    
#define  INVLD     6
#define  BARR      7
#define  BARRGRANT 8
#define  REL       9
#define  WTNT      10 
#define  JIAEXIT   11
#define  WAIT      12
#define  WAITGRANT 13
#define  STAT      14
#define  STATGRANT 15
#define  ERRMSG    16

#define  SETCV     17
#define  RESETCV   18
#define  WAITCV    19
#define  CVGRANT   20
#define  MSGBODY   21
#define  MSGTAIL   22
#define  LOADREQ   23
#define  LOADGRANT 24

#define  BCAST     100

#define  inqh    inqueue[inhead]
#define  inqt    inqueue[intail]
#define  outqh   outqueue[outhead]
#define  outqt   outqueue[outtail]


 
typedef struct Jia_Msg {
	unsigned int op;
	unsigned int frompid;
	unsigned int topid;
        unsigned int temp;      /*Useless*/
	unsigned int seqno;
        unsigned int index; 
        unsigned int scope;     /*Incar. no.  used as tag in msg. passing*/
	unsigned int size;
	unsigned char data[Maxmsgsize];
} jia_msg_t;

typedef  jia_msg_t* msgp_t;

typedef struct CommManager{
    	int                 snd_fds[Maxhosts];
   	fd_set              snd_set;
    	int                 snd_maxfd;
    	unsigned            snd_seq[Maxhosts];

    	int                 rcv_fds[Maxhosts];
    	fd_set              rcv_set;
    	int                 rcv_maxfd;
    	unsigned            rcv_seq[Maxhosts];
} CommManager;


#endif	/* JIACOMM_H */
