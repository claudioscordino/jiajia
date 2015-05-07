#include <jia.h>

#include "math.h"
#include "mdvar.h"
#include "parameters.h"
#include "mddata.h"
#include "split.h"
#include "global.h"

KINETI(NMOL,SUM,HMAS,OMAS)
int NMOL;
double HMAS,OMAS;
double SUM[];

        /* this routine computes kinetic energy in each of the three
            spatial dimensions, and puts the computed values in the
            SUM array */ 
{
    int dir, mol;
    double S;
    double *tempptr; 

    /* loop over the three directions */
    for (dir = XDIR; dir <= ZDIR; dir++) {
	S=0.0;
        /* loop over the molecules */
        for (mol = StartMol[jiapid]; mol < StartMol[jiapid+1]; mol++) {
            tempptr = VAR[mol].F[VEL][dir]; 
            S += ( tempptr[H1] * tempptr[H1] +
                  tempptr[H2] * tempptr[H2] ) * HMAS
                      + (tempptr[O] * tempptr[O]) * OMAS;
        }
	jia_lock(gl->KinetiSumLock);
	SUM[dir]+=S;
	jia_unlock(gl->KinetiSumLock);
    } /* for */
} /* end of subroutine KINETI */

