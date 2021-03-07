#ifndef TSP_H_ // begin ifndef
#define TSP_H_ // block multiple tsp.h includes

#include <stdio.h>
#include <stdlib.h>

// define a general instance of the problem
typedef struct{
    // input data
    int tot_nodes;
    double *xcoord, *ycoord;
    char *input_file_name; // can be very large if it contains parent directories!
    double time_limit;
    int verbose;
} instance;



#endif // end ifndef
