#ifndef TSP_H_ // begin ifndef
#define TSP_H_ // block multiple tsp.h includes

#include <stdio.h>
#include <stdlib.h>

#include <cplex.h>

// ANSI escape sequences (from stackoverflow)
#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */


// define a general instance of the problem
typedef struct{
    // from cli
    char *input_tsp_file_name; // can be very large if it contains parent directories!
    char *input_opt_file_name;
    double time_limit;
    int verbose;

    // from file (2nd cell for opt tour)
    char * name[2];
    char * comment[2];
    int tot_nodes;
    double *xcoord, *ycoord;
    int *opt_tour;

    // other parameters
    char integer_costs;
} instance;

void init_instance(instance * inst);
void free_instance(instance *inst);
char * TSPOpt(instance *inst);
double dist(int i, int j, instance *inst);
int xpos(int i, int j, instance *inst);
void build_model(instance *inst, CPXENVptr env, CPXLPptr lp);

#endif // end ifndef
