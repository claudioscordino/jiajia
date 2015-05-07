#include <jia.h>

#include "mdvar.h"
#include "parameters.h"
#include "mddata.h"
#include "split.h"

BNDRY()     /* this routine puts the molecules back inside the box if
            they are  out */
{
    int mol, dir; 
    
    /* for each molecule */
    for (mol = StartMol[jiapid]; mol < StartMol[jiapid+1]; mol++) {
/*
    for (mol = 0; mol < NMOL; mol++) {
*/
	/* for each direction */
  	for ( dir = XDIR; dir <= ZDIR; dir++ ) {
	    /* if the oxygen atom is out of the box */
    	    if (VAR[mol].F[DISP][dir][O] > BOXL) {
		/* move all three atoms back in the box */
		VAR[mol].F[DISP][dir][H1] -= BOXL;
		VAR[mol].F[DISP][dir][O]  -= BOXL;
		VAR[mol].F[DISP][dir][H2] -= BOXL;
            }
     	    else if (VAR[mol].F[DISP][dir][O] < 0.00) {
		VAR[mol].F[DISP][dir][H1] += BOXL;
		VAR[mol].F[DISP][dir][O]  += BOXL;
		VAR[mol].F[DISP][dir][H2] += BOXL;
            } 
    	} /* for dir */
    } /* for mol */
} /* end of subroutine BNDRY */

