#include <stdio.h>
 
#include <sys/time.h>


#include "tsp.h"

#define SILENT

/* BEGIN SHARED DATA */
GlobalMemory	*glob = NULL;
/* END SHARED DATA */


int 		StartNode, TspSize, NodesFromEnd;
int 		dump, debug, debugPrioQ;
int		performance;
extern int	visitNodes;



usage()
{
	fprintf(stderr, "tsp:\n\t%s\n\t%s\n\t%s\n\t%s\n\t%s\n\t%s\n\t%s\n",
			"-n: number of hosts", "-s: start node",
			"-f: size (filename)", "-r: size to recursively solve",
			"-d: debug mode", "-D: debug the priority queue",
			"-S: run silently");
}

extern char    *optarg;

void Worker()
{
    int curr = -1;

    /* Wait for signal to start. */
    jia_barrier();

    for (;;) {
		/* Continuously get a tour and split. */
		curr = get_tour(curr);
		if (curr < 0) {
			break;
		}
		recursive_solve(curr);
    }
    jia_barrier();	/* Signal completion. */
}


main(argc,argv)
unsigned argc;
char **argv;
{
    int 		c, i, j, silent;
    char 	       *fname;
    struct timeval	start, end;

    TspSize = MAX_TOUR_SIZE;
    NodesFromEnd = 12;
    dump = 0;
    fname = "19b";
#ifdef SILENT
    silent = 1;
#endif SILENT
/*
    jia_init(argc, argv);
*/
    if (argc == 1) usage();

    while ((c = getopt(argc, argv, "df:pr:s:DSh")) != -1)
		switch (c) {
		  case 'd':
			debug++;
			printf("Debugging on.\n");
			break;
		  case 'f':
			fname = optarg;
			break;
		  case 'p':
			performance = 1;
			break;
		  case 'r':
			sscanf(optarg, "%d", &NodesFromEnd);
			break;
		  case 's':
			StartNode = atoi(optarg);
			printf("Starting at node %d\n",StartNode);
			break;
		  case 'D':
			debugPrioQ++;
			printf("Debugging prioQ.\n");
			break;
		  case 'S':
			silent++;
			printf("Running silent.\n");
			break;
/*
		  case '?':
*/
		  case 'h':
			usage();
			exit(0);
		}

    jia_init(argc, argv);
/*
*/
    jia_config(WVEC,OFF); 

 
	if ((glob = (GlobalMemory *) jia_alloc(sizeof(GlobalMemory))) == 0) {
		fprintf(stderr, "Unable to alloc shared memory\n");
		exit(-1);
	}
        jia_barrier();
        if(jiapid ==0) {
	bzero((char *)glob, sizeof(*glob));
        jia_wtnt(glob,sizeof(*glob));
	glob->TourStackTop = -1;
        jia_wtntw(&(glob->TourStackTop));
	glob->MinTourLen = BIGINT;
        jia_wtntw(&(glob->MinTourLen));
	glob->TourLock = 1;
        jia_wtntw(&(glob->TourLock));
	glob->MinLock = 2;
        jia_wtntw(&(glob->MinLock));
	glob->barrier = 0;
        jia_wtntw(&(glob->barrier));
        }
	/* Read in the data file with graph description. */
	TspSize = read_tsp(fname);

	/* Initialize the first tour. */
        if (jiapid ==0) {
	glob->Tours[0].prefix[0] = StartNode;
        jia_wtntw(&(glob->Tours[0].prefix[0]));
	glob->Tours[0].conn = 1;
        jia_wtntw(&(glob->Tours[0].conn));
	glob->Tours[0].last = 0;
        jia_wtntw(&(glob->Tours[0].last));
	glob->Tours[0].prefix_weight = 0;
        jia_wtntw(&(glob->Tours[0].prefix_weight));
        }
	calc_bound(0);			/* Sets lower_bound. */

	/* Initialize the priority queue structures. */
        if (jiapid ==0) {
	glob->PrioQ[1].index = 0;	/* The first PrioQ entry is Tour 0. */
        jia_wtntw(&(glob->PrioQ[1].index));
	glob->PrioQ[1].priority = glob->Tours[0].lower_bound;
        jia_wtntw(&(glob->PrioQ[1].priority));
	glob->PrioQLast = 1;
        jia_wtntw(&(glob->PrioQLast));

	/* Put all the unused tours in the free tour stack. */
	for (i = MAX_NUM_TOURS - 1; i > 0; i--)
		glob->TourStack[++glob->TourStackTop] = i;
                jia_wtntw(&(glob->TourStackTop));
                jia_wtntw(&(glob->TourStack[glob->TourStackTop]));
        }
    jia_barrier();			
    jia_startstat();
    gettimeofday(&start, NULL);

    Worker();

    gettimeofday(&end, NULL);

    fprintf(stdout, "Elapsed time: %.2f seconds\n",
			(((end.tv_sec * 1000000.0) + end.tv_usec) -
			 ((start.tv_sec * 1000000.0) + start.tv_usec)) / 1000000.0);
    fprintf(stderr, "Elapsed time: %.2f seconds\n",
			(((end.tv_sec * 1000000.0) + end.tv_usec) -
			 ((start.tv_sec * 1000000.0) + start.tv_usec)) / 1000000.0);

    fprintf(stdout, "\n-----------------\n\n");
    fprintf(stderr, "\n-----------------\n\n");

    fprintf(stdout, "\t[MINIMUM TOUR LENGTH: %d]\n\n", glob->MinTourLen);
    fprintf(stdout, "MINIMUM TOUR:\n");
	for (i = 0; i < TspSize; i++) {
	    if (i % 10) fprintf(stdout, " - "); 
	    else fprintf(stdout, "\n\t");
	    fprintf(stdout, "%2d", (int) glob->MinTour[i]);
	}
    fprintf(stdout, " - %2d\n\n", StartNode);

    jia_exit();
}


