#include <jia.h>
#include "stdio.h"

#include "stdio.h"
#include "comments.h"	/* moves leading comments to another file for now */

#include "parameters.h"
#include "mdvar.h"
#include "water.h"
#include "wwpot.h"
#include "cnst.h"
#include "mddata.h"
#include "fileio.h"
#include "split.h"
#include "global.h"
#include <sys/time.h>

extern FILE *dump;

/************************************************************************/
double  MDMAIN(NFSV,NFRST,NSTEP,NRST,NPRINT,NSAVE,LKT,NORD1)

    /* routine that implements the time-steps. Called by main routine 
        and calls others */

int NFSV,NFRST,NSTEP,NRST,NPRINT,NSAVE,LKT,NORD1;
{
    double XTT;
    int i, ii, II,mol, func, dir;
    double POTA,POTR,POTRF;
    double XVIR,AVGT,TEN;
    struct timeval start, finish;

        /* wait till everyone gets to beginning; not necessary */

    /* MOLECULAR DYNAMICS LOOP OVER ALL TIME-STEPS */

    for (i=1;i <= NSTEP; i++) {
       	TTMV=TTMV+1.00;

            /* initialize various shared sums */
	if (jiapid == 0) {
	    int dir;
	    gl->VIR = 0.0;
	    gl->POTA = 0.0;
	    gl->POTR = 0.0;
	    gl->POTRF = 0.0;
	    for (dir = XDIR; dir <= ZDIR; dir++)
		gl->SUM[dir] = 0.0;
	}
        jia_barrier();
/********
	{ int mol, i, j, k, l;
	  fprintf(dump,"\n\n----------------------------------------------\n\n");
	  for (mol =StartMol[jiapid]; mol < StartMol[jiapid+1]; mol++)
	    {
	      for (i = 0; i < 3; i++)
		fprintf(dump,"%d, VM[%d] = %.12f\n",mol, i, VAR[mol].VM[i]);
	      for (l = 0; l< FORCES; l++)
		for (j = 0; j < NDIR; j++)
		  for (k = 0; k < NATOM; k++)
		    fprintf(dump,"%d, F[%d][%d][%d] = %.8f\n",mol, l, j, k, 
			    VAR[mol].F[l][j][k]);
	    }
	  fflush(dump);
	}
*********/
    	PREDIC(TLC,NORD1);

     	INTRAF(&gl->VIR);

        jia_barrier();    	

	INTERF(FORCES,&gl->VIR);
        CORREC(PCC,NORD1);
    	BNDRY();
    	KINETI(NMOL,gl->SUM,HMAS,OMAS);
        jia_barrier();

    	TKIN=TKIN+gl->SUM[0]+gl->SUM[1]+gl->SUM[2];
        TVIR=TVIR-gl->VIR;

        /*  check if potential energy is to be computed, and if
            printing and/or saving is to be done, this time step.
            Note that potential energy is computed once every NPRINT
            time-steps */

        if (((i % NPRINT) == 0) || ( (NSAVE > 0) && ((i % NSAVE) == 0))){

                /*  call potential energy computing routine */
	    POTENG(&gl->POTA,&gl->POTR,&gl->POTRF);
	    jia_barrier();

                /* modify computed sums */
	    POTA=gl->POTA*FPOT;
	    POTR=gl->POTR*FPOT;
	    POTRF=gl->POTRF*FPOT;

                /* compute some values to print */
	    XVIR=TVIR*FPOT*0.50/TTMV;
	    AVGT=TKIN*FKIN*TEMP*2.00/(3.00*TTMV);
	    TEN=(gl->SUM[0]+gl->SUM[1]+gl->SUM[2])*FKIN;
	    XTT=POTA+POTR+POTRF+TEN;
	    if ((i % NPRINT) == 0 && jiapid == 0) {
	  	fprintf(six,"     %5d %14.5lf %12.5lf %12.5lf %12.5lf\n %16.3lf %16.5lf %16.5lf\n",
                    i,TEN,POTA,POTR,POTRF,XTT,AVGT,XVIR);
	    }
        }

            /* wait for everyone to finish time-step */
	jia_barrier();

    } /* for i */
    return(XTT);
} /* end of subroutine MDMAIN */
