#include <jia.h>
/*
#define jia_wtnt(x) 
*/
#define	BROAD_TOUR
#define register	...

#define MAX_TOUR_SIZE	32	
#define MAX_NUM_TOURS	5000
#define BIGINT		2000000
#define END_TOUR	(-1)
#define ALL_DONE	(-1)

#define BEEP		7

typedef struct {
    int prefix[MAX_TOUR_SIZE]; 	/* List of nodes in cur tour. */
/*  char prefix[MAX_TOUR_SIZE];	/* List of nodes in cur tour. */
    int conn;			/* conn & 1<<i => in cur tour. */
    int last;			/* Index of last node in cur tour. */
    int prefix_weight;		/* Weight of edges in cur tour. */
    int lower_bound;		/* Lower bound of edges not in cur tour. */
    int	mst_weight;		/* minimum spanning tree cost for rest. */
} TourElement;

typedef struct {
    int  index;			/* Index into tours[] of this element. */
    int  priority;		/* Priority of this element. */
} PrioQElement;

typedef struct 
{
    int 		weights[MAX_TOUR_SIZE][MAX_TOUR_SIZE];
    int			TourStackTop;	/* Index of the last entry. */
    int 		Done;		/* 1 iff no more tours to evaluate. */
    int 		PrioQLast;	/* Last slot filled in PrioQ.	*/
    int 		MinTourLen;	/* Minimum tour length so far.	  */
    int 		MinTour[MAX_TOUR_SIZE];/* Tour with min length.  */
/*  char 		MinTour[MAX_TOUR_SIZE];/* Tour with min length.  */

    int			TourLock;	/* Lock protecting the TourStack and PrioQ */
    int			MinLock;	/* Lock that protext the min tour length. */
    int			barrier;	/* Global barrier to detect termination.  */
    PrioQElement 	PrioQ[MAX_NUM_TOURS];
    int			TourStack[MAX_NUM_TOURS];
    TourElement 	Tours[MAX_NUM_TOURS]; /* Partially evaluated tours. */
} GlobalMemory;

extern GlobalMemory	*glob;

extern read_tsp(), set_best();
extern char *fgets();

extern int TspSize, StartNode;

extern int NodesFromEnd, debug, debugPrioQ;

extern char TOUR_STR[16][256];

#define _tour_str       TOUR_STR[jia_pid]
