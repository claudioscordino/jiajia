#include <stdio.h>
#include 	"tsp.h"

char *read_int(char_ptr, wt)
    char *char_ptr;
    int *wt;
{
    char *temp_ptr;

    temp_ptr = char_ptr;

    while (*temp_ptr == ' ' || *temp_ptr == '\t')
        temp_ptr++;

    if (*temp_ptr == '\n' || *temp_ptr == '\0')
        return(NULL);

    while (*temp_ptr != ' ' && *temp_ptr != '\t' && *temp_ptr != '\n')
        temp_ptr++;

    *temp_ptr = '\0';

    *wt = atoi(char_ptr);

    return(temp_ptr + 1);
}


read_tsp(file)
    char *file;
{
    FILE *fp;
    char *scan_ptr, line[81], fname[80];
    int wt, i ,j, fully_connected = 1;

    strcpy(fname,"tspfiles/tspfile");
    strcat(fname,file);
    if ((fp = fopen(fname, "r")) == NULL) {
        printf("Cannot open TSP file: %s\n", fname);
        exit(0);
    }

    fgets(line, 80, fp);

    TspSize = atoi(line);
    if (TspSize > MAX_TOUR_SIZE) {
	fprintf(stderr,"Error: Problem size (%d) larger than maximum (%d).\n",
		TspSize, MAX_TOUR_SIZE);
	fprintf(stderr,"ABORTING.\n");
	exit(-1);
    }

    for (i = 0; i < TspSize; i++) {
	scan_ptr = fgets(line, 80, fp);
	j = 0;
	while (scan_ptr = read_int(scan_ptr, &wt)) {
                jia_wtntw(&(glob->weights[i][j]));
		glob->weights[i][j++] = wt;
        }
    }

    if (debug) for (i = 0; i < TspSize; i++)
	for (j = 0; j < TspSize; j++)
	    printf("%2d%s", glob->weights[i][j], j == TspSize - 1 ? "\n" : " ");

    return(TspSize);
}
