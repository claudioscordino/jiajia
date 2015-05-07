#include <stdio.h>
#include <string.h>

#include "tsp.h"

typedef struct {
unsigned 	CurDist, PathLen;
int 		Visit[MAX_TOUR_SIZE], Path[MAX_TOUR_SIZE];
int		visitNodes;
} EXPANDED;
EXPANDED solve[16];

#define _CurDist	solve[1].CurDist
#define _PathLen	solve[1].PathLen
#define _Visit		solve[1].Visit
#define _Path		solve[1].Path
#define _visitNodes	solve[1].visitNodes

/*
 *   recursive_solve(curr_ind)
 *
 *	We're now supposed to recursively try all possible solutions
 *	starting with the current tour.  We do this by copying the
 *	state to local variables (to avoid unneeded conflicts) and
 *	calling visit_nodes to do the actual recursive solution.
 *
 */
recursive_solve(index)
    int index;
{
    unsigned i, j;
    TourElement *curr = &glob->Tours[index];

    _CurDist = curr->prefix_weight;
    _PathLen = curr->last + 1;

    for (i = 0; i < TspSize; i++) _Visit[i] = 0;

    for (i = 0; i < _PathLen; i++) {
	_Path[i] = curr->prefix[i];
	_Visit[_Path[i]] = 1;
    }

    if (_PathLen > TspSize) {
	fprintf(stderr, "Pathlen: %d\n", _PathLen);
	fflush(stderr);
	exit(0);
    }

    if (_CurDist == 0 || debugPrioQ) {
	sprintf(_tour_str, "\t%d: Tour %d is ", jia_pid, index);
	j = strlen(_tour_str);
	for (i = 0; i < _PathLen-1; i++) {
	    sprintf((_tour_str + j), "%1d - ", (int)_Path[i]);
	    if ((int)_Path[i] >= 10) j += 5;	/* Two digit number. */
	    else j += 4;
	}
	sprintf((_tour_str + j), "%1d\n", (int)_Path[i]);
	j = strlen(_tour_str);
	sprintf((_tour_str+j), "\t%d: Cur: %d, Min %d, Len: %d, Sz: %d.\n",
		jia_pid, _CurDist, glob->MinTourLen, _PathLen-1, TspSize);
    }

    visit_nodes(_Path[_PathLen-1]);
}


/*
 *   visit_nodes()
 *
 *       Exhaustively visits each node to find Hamilton cycle.
 *       Assumes that search started at node from.
 *
 */
visit_nodes(from)
    int from;
{
    int i, j;
    int dist, last;

#ifdef	DEBUG
    _visitNodes++;
#endif	DEBUG    
    for (i = 1; i < TspSize; i++) {
	if (_Visit[i]) continue;	/* Already visited. */
	if ((dist = glob->weights[from][i]) == 0) continue; /* Not connected. */
	if (_CurDist + dist > glob->MinTourLen) continue; /* Path too long. */

	/* Try next node. */
	_Visit[i] = 1;
	_Path[_PathLen++] = i;
	_CurDist += dist;

	if (_PathLen == TspSize) {
	    /* _Visiting last node - determine if path is min length. */
	    if ((last = glob->weights[i][StartNode]) != 0 &&
		(_CurDist += last) < glob->MinTourLen) {
		set_best(_CurDist, _Path);
	    }
	    _CurDist -= last;
	} /* if visiting last node */
	else if (_CurDist < glob->MinTourLen) visit_nodes(i);	/* _Visit on. */

	/* Remove current try and loop again at this level. */
	_CurDist -= dist;
	_PathLen--;
	_Visit[i] = 0;
    }
}


/*
 *  Add up min edges connecting all unconnected vertixes (AHU p. 331-335)
 *  At some point, we may want to use MST to get a lower bound. This
 *  bound should be higher (and therefore more accurate) but will take
 *  longer to compute. 
 */
calc_bound(curr_index)
    int curr_index;
{
    int i, j, wt, wt1, wt2;
    TourElement *curr = &glob->Tours[curr_index];

    /*
     * wt1: the value of the edge with the lowest weight from the node
     *	    we're testing to another unconnected node.
     * wt2: the value of the edge with the second lowest weight
     */

    /* if we have one unconnected node */
    if (curr->last == (TspSize - 2)) {
	for (i = 0; i < TspSize; i++) {
	    if (!(curr->conn & (1<<i))) {
		/* we have found the one unconnected node */
		curr->prefix[TspSize-1] = i;
                jia_wtntw(&(curr->prefix[TspSize-1]));
		curr->prefix[TspSize] = StartNode;
                jia_wtntw(&(curr->prefix[TspSize]));
		
		/* add edges to and from the last node */
		curr->prefix_weight+= glob->weights[curr->prefix[TspSize-2]][i] +
		                      glob->weights[i][curr->prefix[StartNode]];
                jia_wtntw(&(curr->prefix_weight));

		if (curr->prefix_weight < glob->MinTourLen) {
		    /* Store our new best path and its weight. */
		    set_best(curr->prefix_weight, curr->prefix);
		}

		/* De-allocate this tour so someone else can use it */
		curr->lower_bound = BIGINT;
                jia_wtntw(&(curr->lower_bound));
#ifdef	LOCK_PREFETCH
		PREFETCH2(glob->TourStack, glob->PrioQ);
#endif	LOCK_PREFETCH
		jia_lock(glob->TourLock);
		glob->TourStack[++glob->TourStackTop] = curr_index; /* Free tour. */
                jia_wtntw(&(glob->TourStackTop));
                jia_wtntw(&(glob->TourStack[glob->TourStackTop]));
		jia_unlock(glob->TourLock);
		return(END_TOUR);
	    }
	}
    }

    curr->mst_weight = 0;
    jia_wtntw(&(curr->mst_weight));

    /*
     * Add up the lowest weights for edges connected to vertices
     * not yet used or at the ends of the current tour, and divide by two.
     * This could be tweaked quite a bit.  For example:
     *   (1) Check to make sure that using an edge would not make it
     *       impossible for a vertex to have degree two.
     *   (2) Check to make sure that the edge doesn't give some
     *       vertex degree 3.
     */

    if (curr->last != TspSize - 1) {
	for (i = 0; (i < TspSize); i++) {
	    if (curr->conn & 1<<i) continue;
	    for (j = 0, wt1 = wt2 = BIGINT; j < TspSize; j++) {
		/* Ignore j's that are not connected to i (glob->weights[i][j]==0), */
		/* or that are already in the tour and aren't either the      */
		/* first or last node in the current tour.		      */
		wt = glob->weights[i][j];
		if (!wt || (curr->conn&(1<<j)&&(j != curr->prefix[0]) &&
					       (j != curr->prefix[curr->last])))
		    continue;

		/* Might want to check that edges go to unused vertices */
		if (wt < wt1) {
		    wt2 = wt1;
		    wt1 = wt;
		}
		else if (wt < wt2) wt2 = wt;
	    }
  
	    /* At least three unconnected nodes? */
	    if (wt2 != BIGINT) {
                    curr->mst_weight += (wt1 + wt2) >> 1;
                    jia_wtntw(&(curr->mst_weight));
            }
	    /* Exactly two unconnected nodes? */
	    else if (wt1 != BIGINT) {
		curr->mst_weight += wt1;
 		jia_wtntw(&(curr->mst_weight));
            }
	}
  	curr->mst_weight += 1;
        jia_wtntw(&(curr->mst_weight));
    }

    curr->lower_bound = curr->mst_weight + curr->prefix_weight;
    jia_wtntw(&(curr->lower_bound));

    return(curr_index);
}
