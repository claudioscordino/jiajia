/*
 * pq.c:
 *
 *	Contains the routines for inserting an item into the priority
 * queue and removing the lowest priority item from the queue.
 *
 */
#include <stdio.h>

#include "tsp.h"

#define LEFT_CHILD(x)	((x)<<1)
#define RIGHT_CHILD(x)	(((x)<<1)+1)

#define less_than(x,y)	(((x)->priority  < (y)->priority) || \
			 ((x)->priority == (y)->priority) && \
			  (glob->Tours[(x)->index].last > glob->Tours[(y)->index].last))

/*
 * DumpPrioQ():
 *
 * Dump the contents of PrioQ in some user-readable format (for debugging).
 *
 */
DumpPrioQ()
{
    int pq, ind;

    for (pq = 1; pq <= glob->PrioQLast; pq++) {
	ind = glob->PrioQ[pq].index;
	MakeTourString(glob->Tours[ind].last, glob->Tours[ind].prefix);
    }
}


/*
 * split_tour():
 *
 *  Break current tour into subproblems, and stick them all in the priority
 *  queue for later evaluation.
 *
 */
split_tour(curr_ind)
    int curr_ind;
{
    int n_ind, last_node, wt;
    int i, pq, parent, index, priority;
    TourElement *curr;
    PrioQElement *cptr, *pptr;

    curr = &glob->Tours[curr_ind];

    if (debug) {
	MakeTourString(curr->last, curr->prefix);
    }

    /* Create a tour and add it to the priority Q for each possible
       path that can be derived from the current path by adding a single
       node while staying under the current minimum tour length. */
    if (curr->last != (TspSize - 1)) {
	int t1, t2, t3;
	TourElement *new;
	
	last_node = curr->prefix[curr->last];
	for (i = 0; i < TspSize; i++) {
	    /*
	     * Check: 1. Not already in tour
	     *	      2. Edge from last entry to node in graph
	     *	and   3. Weight of new partial tour is less than cur min.
             */
	    wt = glob->weights[last_node][i];
	    t1 = ((curr->conn & (1<<i)) == 0);
	    t2 = (wt != 0);
	    t3 = (curr->lower_bound + wt) <= glob->MinTourLen;
	    if (t1 && t2 && t3) {
		if ((n_ind = new_tour(curr_ind, i)) == END_TOUR) {
		    continue;
		}
		/*
		 * If it's shorter than the current best tour, or equal
		 * to the current best tour and we're reporting all min
		 * length tours, put it on the priority q.
		 */
		new = &glob->Tours[n_ind];

		if (glob->PrioQLast >= MAX_NUM_TOURS-1) {
		    fprintf(stderr, "pqLast %d\n", glob->PrioQLast);
		    fflush(stderr);
		    exit(-1);
		}

		if (debugPrioQ) {
		    MakeTourString(new->last, new->prefix);
		}

		pq = ++glob->PrioQLast;
                jia_wtntw(&(glob->PrioQLast));
		cptr = &(glob->PrioQ[pq]);
		cptr->index = n_ind;
                jia_wtntw(&(cptr->index));
		cptr->priority = new->lower_bound;
                jia_wtntw(&(cptr->priority));

		/* Bubble the entry up to the appropriate level to maintain
		   the invariant that a parent is less than either of it's
		   children. */
		for (parent = pq >> 1, pptr = &(glob->PrioQ[parent]);
		     (pq > 1) && less_than(cptr,pptr);
		     pq = parent, cptr = pptr,
		     parent = pq >> 1, pptr = &(glob->PrioQ[parent])) {

		    /* PrioQ[pq] lower priority than parent -> SWITCH THEM. */
		    index = cptr->index;
		    priority = cptr->priority;
		    cptr->index = pptr->index;
 		    jia_wtntw(&(cptr->index));
		    cptr->priority = pptr->priority;
                    jia_wtntw(&(cptr->priority));
		    pptr->index = index;
                    jia_wtntw(&(pptr->index));
		    pptr->priority = priority;
                    jia_wtntw(&(pptr->priority));
		}
	    }
	    else if (debug) {
		/* Failed. */
		sprintf(_tour_str, " [%d + %d > %d]",
			curr->lower_bound, wt, glob->MinTourLen);
	    }
	}
    }
}


/*
 * find_solvable_tour():
 *
 * Used by both the normal TSP program (called by get_tour()) and
 * the RPC server (called by RPCserver()) to return the next solvable
 * (sufficiently short) tour.
 *
 */
find_solvable_tour()
{
    int curr, i, left, right, child, index;
    int priority, last;
    PrioQElement *pptr, *cptr, *lptr, *rptr;

    if (glob->Done) return(-1);

    for (; glob->PrioQLast != 0; ) {
	pptr = &(glob->PrioQ[1]);
	curr = pptr->index;
	if (pptr->priority >= glob->MinTourLen) {
	    /* We're done -- there's no way a better tour could be found. */
	    MakeTourString(glob->Tours[curr].last, glob->Tours[curr].prefix);
	    glob->Done = 1;
            jia_wtntw(&(glob->Done));
	    return(-1);
	}

	/* Bubble everything maintain the priority queue's heap invariant. */
	/* Move last element to root position. */
	cptr = &(glob->PrioQ[glob->PrioQLast]);
	pptr->index    = cptr->index;
        jia_wtntw(&(pptr->index));
	pptr->priority = cptr->priority;
        jia_wtntw(&(pptr->priority));
        jia_wtntw(&(glob->PrioQLast));
	glob->PrioQLast--;

	/* Push previous last element down tree to restore heap structure. */
	for (i = 1; i <= (glob->PrioQLast >> 1); ) {
	    /* Find child with lowest priority. */
	    left  = LEFT_CHILD(i);
	    right = RIGHT_CHILD(i);

	    lptr = &(glob->PrioQ[left]);
	    rptr = &(glob->PrioQ[right]);

	    if (left == glob->PrioQLast || less_than(lptr,rptr)) {
		child = left;
		cptr = lptr;
	    }
	    else {
		child = right;
		cptr = rptr;
	    }

	    /* Exchange parent and child, if necessary. */
	    if (less_than(cptr,pptr)) {
		/* glob->PrioQ[child] has lower prio than its parent - switch 'em. */
		index = pptr->index;
		priority = pptr->priority;
		pptr->index = cptr->index;
		jia_wtntw(&(pptr->index ));
		pptr->priority = cptr->priority;
                jia_wtntw(&(pptr->priority));
		cptr->index = index;
                jia_wtntw(&(cptr->index));
		cptr->priority = priority;
                jia_wtntw(&(cptr->priority));
		i = child;
		pptr = cptr;
	    }
	    else break;
	}

	last = glob->Tours[curr].last;
	
	if (debug) {
	}

	/* If we're within `NodesFromEnd' nodes of a complete tour, find
	   minimum solutions recursively.  Else, split the tour. */
	if (last < TspSize || last < 1) {
	    if (last >= (TspSize - NodesFromEnd - 1)) return(curr);
	    else split_tour(curr);	/* The current tour is too long, */
	}				/* to solve now, break it up.	 */
	else {
	    /* Bogus tour index. */
	    MakeTourString(TspSize, glob->Tours[curr].prefix);
	    fprintf(stderr, "\t%d: %s\n", jia_pid, _tour_str);
	}
	glob->TourStack[++glob->TourStackTop] = curr; /* Free tour. */
        jia_wtntw(&(glob->TourStackTop));
        jia_wtntw(&(glob->TourStack[glob->TourStackTop]));
    }
    /* Ran out of candidates - DONE! */
    glob->Done = 1;
    jia_wtntw(&(glob->Done));
    return(-1);
}


get_tour(curr)
    int curr;
{
#ifdef	LOCK_PREFETCH
    PREFETCH2(glob, glob + 1);
#endif	LOCK_PREFETCH
    jia_lock(glob->TourLock);
    if (curr != -1){
       glob->TourStack[++glob->TourStackTop] = curr;
       jia_wtntw(&(glob->TourStackTop)); 
       jia_wtntw(&(glob->TourStack[glob->TourStackTop]));
    }

    curr = find_solvable_tour();
    jia_unlock(glob->TourLock);

    return(curr);
}
