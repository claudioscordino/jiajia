#include <stdio.h>

#include "tsp.h"

char TOUR_STR[16][256];
extern int jia_pid;
/*
 * new_tour():
 *
 *    Create a new tour structure given an existing tour structure and
 *  the next edge to add to the tour.  Returns the index of the new structure.
 *
 */



new_tour(prev_index, move)
int prev_index, move;
{
    int index, i;
    TourElement *curr, *prev;

    /* get index of a blank tour */
    if (glob->TourStackTop >= 0) index = glob->TourStack[glob->TourStackTop--];
    else {
		fprintf(stderr, "TourStackTop: %d\n", glob->TourStackTop);
		fflush(stderr);
		exit(-1);
    }

    curr = &glob->Tours[index];
    prev = &glob->Tours[prev_index];

    for (i = 0; i < TspSize; i++) {
		curr->prefix[i] = prev->prefix[i];
                jia_wtntw(&(curr->prefix[i]));
		curr->conn = prev->conn;
                jia_wtntw(&(curr->conn));
    }

    curr->last = prev->last;
    jia_wtntw(&(curr->last));
    curr->prefix_weight = prev->prefix_weight + 
		glob->weights[curr->prefix[curr->last]][move];
    jia_wtntw(&(curr->prefix_weight));
    curr->prefix[++(curr->last)] = move;
    jia_wtntw(&(curr->last));
    jia_wtntw(&(curr->prefix[(curr->last)]));
    if (debug) {
		MakeTourString(curr->last, curr->prefix);
    }

    /* add our new move */
    curr->conn |= 1 << move;
    jia_wtntw(&(curr->conn));

    /* calculate and return the new lower bound */
    return calc_bound(index);
}


/*
 * set_best():
 *
 *  Set the global `best' value.
 *
 */
set_best(best, path)
int best;
int *path;
{
    int i;

    if (best >= glob->MinTourLen) {
		if (debug)
			fprintf(stderr, "\nset_best: %d <-> %d\n", best, glob->MinTourLen);
		return(0);	/* MinTourLen monotonically decreases */
    }

    /* Ensure that changes to the minimum are serialized. */

    jia_lock(glob->MinLock);

    if (best < glob->MinTourLen) {
		if (debug || debugPrioQ) {
			MakeTourString(TspSize, path);
		}
		fprintf(stderr, "MinTourLen: %d (old: %d): ", best, glob->MinTourLen, glob->MinTour[0]);
		glob->MinTourLen = best;
                jia_wtntw(&(glob->MinTourLen));
		for (i = 0; i < TspSize; i++) {
			glob->MinTour[i] = path[i];
		/*	jia_wtntw(&(glob->MinTour[i]));*/
		}
		jia_wtnt(&(glob->MinTour[i]),4*TspSize);
		fprintf(stderr, "\n");
    }
    jia_unlock(glob->MinLock);
}


/*
 *  MakeTourString(len, path):
 *
 *  Make a string for printing that describes the tour passed via len and path.
 *
 */
MakeTourString(len, path)
int   	len;
int 	*path;
{
    int i, j;

    for (i = j = 0; i < len; i++) {
		sprintf((_tour_str + j), "%1d - ", (int)path[i]);
		if ((int)path[i] >= 10) j += 5;	/* Two digit number. */
		else j += 4;
    }
    sprintf((_tour_str + j), "%1d\0", (int) path[i]);
}
