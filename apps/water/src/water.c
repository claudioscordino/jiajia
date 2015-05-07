#include <jia.h>

#include "stdio.h"
#include <sys/time.h>
#include "comments.h"	/* moves leading comments to another file */
#include "split.h"

/*  include files for declarations  */
#define extern
#include "parameters.h"
#include "mdvar.h"
#include "water.h"
#include "wwpot.h"
#include "cnst.h"
#include "mddata.h"
#include "fileio.h"
#include "frcnst.h"
#include "randno.h"
#include "global.h"
#undef extern
struct GlobalMemory *gl;        /* pointer to the Global Memory
                                structure, which contains the lock,
                                barrier, and some scalar variables */
FILE *dump;                                

int NSTEP, NSAVE, NRST, NPRINT,NFMC;
int NORD1;
int II;                         /*  variables explained in common.h */
int i;
int NDATA;
int   NFRST=11;
int  NFSV=10;
int  LKT=0;

int StartMol[MAXPROCS+1];       /* number of the first molecule
                                   to be handled by this process; used
                                   for static scheduling     */ 
int MolsPerProc;                /* number of mols per processor */ 

extern char	*optarg;
int errout;
main(argc, argv)
char **argv;
{
    FILE *fp;
    char *input_file = "waterfiles/sample.in";
    int mol, pid, func, c, dir, atom, tsteps = 0;
    double XTT, MDMAIN();
    double VIR;
    struct timeval start, finish;
	int kk;
	unsigned mol_size = sizeof(molecule_type);
	unsigned gmem_size = sizeof(struct GlobalMemory);

    /* default values for the control parameters of the driver */
    /* are in parameters.h */
/***
    if ((errout = creat("/tmp/errout", 0644)) <= 0) {
      fprintf(stderr, "Can't open file errout\n");
      exit();
    }
    close(2);
    dup(errout);
*******/     
    six = stderr;
    nfmc = fopen("waterfiles/LWI12","r"); /* input file for particle
                                  displacements */

    TEMP  =298.0;
    RHO   =0.9980;
    CUTOFF=0.0;

    while ((c = getopt(argc, argv, "i:t:")) != -1)
	switch (c) {
	case 'i':
	    input_file = optarg;
	    break;
	  case 't':
	    tsteps = atoi(optarg);
	    break;
	}

   /* READ INPUT */

    /*
     *   TSTEP = time interval between steps
     *   NMOL  = number of molecules to be simulated
     *   NSTEP = number of time-steps to be simulated
     *   NORDER = order of the predictor-corrector method. 6 by default
     *   NSAVE = frequency of data saving.  -1 by default
     *   NRST  = no longer used
     *   NPRINT = frequency (in time-steps) of computing pot. energy and
     *            writing output file.  setting it larger than NSTEP
     *            means not doing any I/O until the very end.
     *   NFMC = file number to read initial displacements from.  set to 
     *          0 if program should generate a regular lattice initially.
     *   jiahosts = number of processors to be used.
     */
    if (!(fp = fopen(input_file,"r"))) {
	fprintf(stderr, "Unable to open '%s'\n", input_file);
	exit(-1);
    }
    fscanf(fp, "%lf%d%d%d%d%d%d%d%d",&TSTEP, &NMOL, &NSTEP, &NORDER, 
	   &NSAVE, &NRST, &NPRINT, &NFMC);
    if (tsteps) NSTEP = tsteps;
    if (NMOL > MAXMOLS) {
	fprintf(stderr, "Lock array in global.H has size %d < %d (NMOL)\n",
							MAXMOLS, NMOL);
	exit(-1);
    }
    jia_init(argc, argv);
    jia_config(WVEC,ON2);
    

    printf("Using %d procs on %d steps of %d mols\n", jiahosts, NSTEP, NMOL);

    /* SET UP SCALING FACTORS AND CONSTANTS */

    NORD1=NORDER+1;

    CNSTNT(NORD1,TLC);  /* sub. call to set up constants */

    fprintf(six,"\nTEMPERATURE                = %8.2f K\n",TEMP);
    fprintf(six,"DENSITY                    = %8.5f G/C.C.\n",RHO);
    fprintf(six,"NUMBER OF MOLECULES        = %8d\n",NMOL);
    fprintf(six,"NUMBER OF PROCESSORS       = %8d\n",jiahosts);
    fprintf(six,"TIME STEP                  = %8.2e SEC\n",TSTEP);
    fprintf(six,"ORDER USED TO SOLVE F=MA   = %8d \n",NORDER);
    fprintf(six,"NO. OF TIME STEPS          = %8d \n",NSTEP);
    fprintf(six,"FREQUENCY OF DATA SAVING   = %8d \n",NSAVE);
    fprintf(six,"FREQUENCY TO WRITE RST FILE= %8d \n",NRST);

      /* allocate space for main (VAR) data structure as well as
           synchronization variables */
    sleep(jiapid);
	gl = (struct GlobalMemory *) jia_alloc2p(gmem_size,0);
	VAR = (molecule_type *) jia_alloc2p(mol_size*NMOL,0);
	VAR_DISPVM = (shared_type *) jia_alloc2p(sizeof(shared_type)*NMOL,0);
	VAR_FORCE = (force_type *) jia_alloc2p(sizeof(force_type)*NMOL,0); 

    jia_barrier();	
    if (jiapid == 0) { /* Do memory initializations */

	/* Initialize pointers in VAR */
	for(mol=0; mol<NMOL; mol++) {
	  VAR[mol].VM = (double *)VAR_DISPVM[mol].Vm;
	  for (dir=0; dir<3; dir++) {
	    VAR[mol].F[DISP][dir] = VAR_DISPVM[mol].Disp[dir];
	    VAR[mol].F[FORCES][dir] = VAR_FORCE[mol].Force[dir];
	  }
	}

             /* macro calls to initialize synch varibles  */

	gl->start = 0;
	gl->InterfBar = 1;
	gl->PotengBar = 2;
        gl->IntrafVirLock = 1;
        gl->InterfVirLock = 2;
        gl->FXLock = 3;
        gl->FYLock = 4;
        gl->FZLock = 5;

	for (kk = 0; kk < jiahosts; kk++)
		gl->MolLock[kk] = kk + 8;

	gl->KinetiSumLock = 6;
	gl->PotengSumLock = 7;


      }
    else {
	freopen("/tmp/err.1", "a", stderr); setbuf(stderr, NULL);
      }
    jia_barrier();
    dump = fopen("/tmp/dump.dsm", "w");

    /* set up control for static scheduling */

    MolsPerProc = NMOL/jiahosts;
    StartMol[0] = 0;
    for (pid = 1; pid < jiahosts; pid += 1) {
      StartMol[pid] = StartMol[pid-1] + MolsPerProc;
    }
    StartMol[jiahosts] = NMOL;

    c = StartMol[jiapid+1] - StartMol[jiapid];
    VAR_PRIVATE = (private_type *) malloc(sizeof(private_type) * c);
    bzero(VAR_PRIVATE, sizeof(private_type) * c); 
    i = 0;
    for (mol=StartMol[jiapid]; mol<StartMol[jiapid+1]; mol++) {
      II = 0;
      for (func = VEL; func<MAXODR; func++) {
	for (dir=0; dir<3; dir++)
	      VAR[mol].F[func][dir] = VAR_PRIVATE[i].Der[II][dir];
	II++;
      }
      i++;
    }

    SYSCNS();    /* sub. call to initialize system constants  */
    fprintf(six,"SPHERICAL CUTOFF RADIUS    = %8.4f ANGSTROM\n",CUTOFF);
    fflush(six);
    IRST=0;

            /* if there is no input displacement file, and we are to
               initialize to a regular lattice */
    if (NFMC == 0) {
	fclose(nfmc);
	nfmc = NULL;
    }

            /* initialization routine; also reads displacements and
             sets up random velocities*/
    if (jiapid == 0)
      INITIA(nfmc);

    /*.......ESTIMATE ACCELERATION FROM F/M */
    /* note that these initial calls to the force-computing 
       routines  (INTRAF and INTERF) use only 1 process since 
       others haven't been created yet */

    jia_barrier();
    for (mol=StartMol[jiapid]; mol<StartMol[jiapid+1]; mol++)
      for (dir=0; dir<3; dir++)
	for (atom=0; atom<NATOM; atom++)
	  VAR[mol].F[VEL][dir][atom] = VAR[mol].F[FORCES][dir][atom];

    INTRAF(&gl->VIR);
    jia_barrier();
    INTERF(FORCES,&gl->VIR);
    jia_barrier();
    NFMC= -1;
    for (mol=StartMol[jiapid]; mol<StartMol[jiapid+1]; mol++)
      for (dir=0; dir<3; dir++)
	for (atom=0; atom<NATOM; atom++) {
	  VAR[mol].F[ACC][dir][atom] = VAR[mol].F[FORCES][dir][atom];
	  VAR[mol].F[FORCES][dir][atom] = 0.0;
	}
    
    /*.....START MOLECULAR DYNAMIC LOOP */
    if (NFMC < 0) {
  	ELPST=0.00;
        TKIN=0.00;
        TVIR=0.00;
        TTMV=0.00;
    };
    if (NSAVE > 0)  /* not true for input decks provided */
      fprintf(six,"COLLECTING X AND V DATA AT EVERY %4d TIME STEPS \n",NSAVE);

            /* call routine to do the timesteps */
    jia_startstat();
/*
    jia_config(HMIG,ON);
    jia_config(ADWD,ON);
*/

    gettimeofday(&start, NULL);
    XTT = MDMAIN(NFSV,NFRST,NSTEP,NRST,NPRINT,NSAVE,LKT,NORD1); 
    gettimeofday(&finish, NULL);

    fprintf(stderr, "\nExited Happily with XTT %g\n", XTT);
    fprintf(stderr, "Elapsed time: %.2f seconds\n",
	    (((finish.tv_sec * 1000000.0) + finish.tv_usec) -
	     ((start.tv_sec * 1000000.0) + start.tv_usec)) / 1000000.0);
    jia_exit();
    exit(((XTT > 9.21674) && (XTT < 9.21676)) ? (0) : (1));

} /* main.c */
