//
// Created by enrico on 17/03/21.
//

#ifndef TSP_OP2_UTILS_H
#define TSP_OP2_UTILS_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <cplex.h>
// ANSI escape sequences
#define RESET       "\033[0m"
#define BOLDRED     "\033[1m\033[31m"
#define BOLDGREEN   "\033[1m\033[32m"
#define BOLDYELLOW  "\033[1m\033[33m"

#define BUFLEN 1024

// NB it changes with each CPLEX release!
#define DEFAULT_CPLEX_SEED 202009243 // from cplex (display settings all)

enum formulation_t {CUTS, BENDERS, MTZ, GG, GGi, HFIXING, SFIXING, FLAST}; // FLAST is enum guard
enum cons_heuristic_t {GREEDY, GREEDYGRASP, EXTRAMILEAGE, EXTRAMILEAGECONVEXHULL, CHLAST}; // CHLAST is enum guard
enum ref_heuristic_t {TWO_OPT, TWO_OPT_MIN, VNS1, VNS2, TABU_SEARCH1, TABU_SEARCH2, TABU_SEARCH3, RHLAST};
enum distance_t {EUC_2D, ATT, GEO};

const char *formulation_names[8];
const char *cons_heuristic_names[5];
const char *ref_heuristic_names[8];

// define a general instance of the problem
typedef struct{
    // ===== from cli =====
    char *input_tsp_file_name;              // input file in TSPLIB format http://comopt.ifi.uni-heidelberg.de/software/TSPLIB95/tsp95.pdf
    char *input_opt_file_name;              // input file in TSPLIB format needed only to check correctness
    enum formulation_t formulation;         // formulation type
    enum cons_heuristic_t cons_heuristic;   // cons_heuristic type
    enum ref_heuristic_t ref_heuristic;
    bool lazy;                              // add formulation-specific constraints in lazy way
    int seed;                               // cplex random seed
    bool integer_costs;             // force to use integer cost (only for EUC_2D)
    double time_limit;              // overall time limit - CPLEX parameter
    double mem_limit;               // memory limit for CPLEX decision tree
    bool gui;                       // show image to the user
    bool do_plot;                   // use gnu-plot
    bool no_opt;                    // don't show known optimal solution
    int perfr;                      // random instance seed
    int size;                       // size of random instance
    int *seeds;                     // list of `runs` seeds
    char *perfl;                    // size performance test on a list written to file
    int test;                       // test number
    int verbose;                    // print level

    // ===== from file =====
    char *name[2];                  // name field (2nd cell for opt.tour)
    char *comment[2];               // comment field (2nd cell for opt.tour)
    int nnodes;                     // number of nodes i.e. dimension field
    enum distance_t dist;           // distance type
    double *xcoord, *ycoord;        // points
    int *opt_tour;                  // optimal tour from .opt.tour file. Format: 4, 7, 2, ...

    // ===== CPLEX =====
    CPXENVptr CPXenv;               // CPLEX environment
    CPXLPptr CPXlp;                 // CPLEX linear problem
    int ncols;                      // number of columns in the tableau
    int nrows;                      // number of rows in the tableau

    // ===== other parameters =====
    bool directed;                  // use directed graph (for plot purpose)
    struct timeval tstart;

    // ===== results =====
    long runtime;                   // overall runtime
    double *xstar;                  // (rounded) optimal solution
    double zstar;                   // optimal solution value
    double zbest;                   // best solution value found for euristics
    double *xbest;                  // best solution found for euristics
    int status;                     // cplex status
    int *succ;                      // array of successor (i,j) => succ[i] = j
} instance;

void init_instance(instance *inst);

void free_instance(instance *inst);

void save_instance_to_tsp_file(instance *inst);

void printerr(instance *inst, const char *err, ...) __attribute__ ((__noreturn__));

void print(instance *inst, char type, int lv, const char *msg, ...);

bool uprob(double perc);

int nrand();

void start(instance *inst);

bool timeout(instance *inst);

bool timeouts(instance *inst, double s);

int * xtosucc(instance *inst, const double *x);

double * succtox(instance *inst, const int *succ);

void printsucc(instance *inst, const int *succ);

double cost_succ(instance *inst, const int *succ);

#endif //TSP_OP2_UTILS_H
