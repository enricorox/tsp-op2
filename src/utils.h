//
// Created by enrico on 17/03/21.
//

#ifndef TSP_OP2_UTILS_H
#define TSP_OP2_UTILS_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// ANSI escape sequences
#define RESET   "\033[0m"
#define BOLDRED     "\033[1m\033[31m"
#define BOLDGREEN   "\033[1m\033[32m"

#define BUFLEN 1024

enum formulation_t {STANDARD, MTZ, GG, FLAST};
enum distance_t {EUC_2D, ATT, GEO};
const char *formulation_names[3];

// define a general instance of the problem
typedef struct{
    // from cli
    char *input_tsp_file_name; // can be very large if it contains parent directories!
    char *input_opt_file_name;
    enum formulation_t formulation;
    bool lazy;
    double time_limit;
    bool gui;
    bool do_plot;
    int perf;
    int verbose;

    // from file (2nd cell for opt tour)
    char *name[2];
    char *comment[2];
    int tot_nodes;
    enum distance_t dist;
    double *xcoord, *ycoord;
    int *opt_tour;

    // other parameters
    bool integer_costs;
    bool directed;

    // results
    long time;
    double *xstar;
    int status;
} instance;

void init_instance(instance * inst);

void free_instance(instance *inst);

void save_instance_to_tsp_file(instance *inst);

#endif //TSP_OP2_UTILS_H

