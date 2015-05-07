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

#ifndef JIAMEM_H
#define JIAMEM_H

/*
*/
#define   RESERVE_TWIN_SPACE
#define   REPSCHEME   0
#define   Maxdiffs    64
#define   SWvalve     1 /*must be less than 15*/
#define   Diffunit    4
#define   Dirtysize   (Pagesize/(Diffunit*8))
#define   Homepages   16384
#define   Homesize    (Homepages*Pagesize)
#define   Cachesize   (Pagesize*Cachepages) 
#define   Setpages    Cachepages
#define   Setnum      (Cachepages/Setpages) 

#define   DIFFNULL    ((jia_msg_t*) NULL)
  
#define   homehost(addr) page[((unsigned long)(addr)-Startaddr)/Pagesize].homepid
#define   homepage(addr) page[((unsigned long)(addr)-Startaddr)/Pagesize].homei

typedef unsigned char* address_t; 

typedef unsigned long  wtvect_t;
#define   Wvbits       32
#define   WVNULL       0x0
#define   WVFULL       0xffffffff
#define   Blocksize    (Pagesize/Wvbits)

typedef struct{
           char               wtnt; 
                  /*bit0:written by home host in an interval*/
                  /*bit1:written by home host in a barrier interval*/
                  /*bit2:written by other host in an barrier interval*/ 
                  /*bit7~4:single write counter*/ 
           char               rdnt;   
                  /*bit0:somebody has a valid copy*/
           address_t          addr;
           wtvect_t           *wtvect;  /*used only for write vector*/
           address_t          twin;     /*used only for write vector*/
           char               wvfull;   /*used only for write vector*/
              } jiahome_t;

typedef enum {UNMAP,INV,RO,RW} pagestate_t;

typedef struct{
           pagestate_t        state;
           address_t          addr;
           address_t          twin;
           char               wtnt; 
              } jiacache_t;

typedef struct{
           unsigned short int cachei;
           unsigned short int homei;
           unsigned short int homepid;
              } jiapage_t;

#endif /*JIAMEM_H*/
