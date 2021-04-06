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
#define DEFAULT_CPLEX_SEED 202009243 // from cplex> display settings all

enum formulation_t {BENDERS, MTZ, GG, GGi, FLAST}; // FLAST is enum guard
enum distance_t {EUC_2D, ATT, GEO};
const char *formulation_names[4];

// define a general instance of the problem
typedef struct{
    // ===== from cli =====
    char *input_tsp_file_name;      // input file in TSPLIB format http://comopt.ifi.uni-heidelberg.de/software/TSPLIB95/tsp95.pdf
    char *input_opt_file_name;      // input file in TSPLIB format needed only to check correctness
    enum formulation_t formulation; // formulation type
    bool lazy;                      // add formulation-specific constraints in lazy way
    int seed;                       // cplex random seed
    bool integer_costs;             // force to use integer cost (only for EUC_2D)
    double time_limit;              // overall time limit - CPLEX parameter
    double mem_limit;               // memory limit for CPLEX decision tree
    bool gui;                       // show image to the user
    bool do_plot;                   // use gnu-plot
    bool no_opt;                    // don't show known optimal solution
    int perfr;                      // run performance test on random points up to <max> size
    char *perfl;                    // run performance test on a list written to file
    int verbose;                    // print level

    // ===== from file =====
    char *name[2];                  // name field (2nd cell for opt.tour)
    char *comment[2];               // comment field (2nd cell for opt.tour)
    int tot_nodes;                  // dimension field
    enum distance_t dist;           // distance type
    double *xcoord, *ycoord;        // points
    int *opt_tour;                  // optimal tour from .opt.tour file

    // ===== CPLEX =====
    CPXENVptr CPXenv;
    CPXLPptr CPXlp;

    // ===== other parameters =====
    bool directed;                  // use directed graph (for plot purpose)

    // ===== results =====
    long runtime;                   // overall runtime
    double *xstar;                  // (rounded) optimal solution
    int status;                     // cplex status
} instance;

void init_instance(instance * inst);

void free_instance(instance *inst);

void save_instance_to_tsp_file(instance *inst);

void printerr(instance *inst, const char *err, ...) __attribute__ ((__noreturn__));

void print(instance *inst, char type, int lv, const char *msg, ...);

#endif //TSP_OP2_UTILS_H

