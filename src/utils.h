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

#define USAGE   "TSP solver 0.x, a solver for Travelling Salesman Problem with CPLEX\n" \
                "Author: Enrico Rossignolo - 1218951 - enrico.rossignolo@studenti.unipd.it\n\n" \
                "Usage: ./tsp (--file <file-tsp> | --perf) [options]\n" \
                "Options:\n"\
                "--opt-tour <file-opt-tsp>          tsp file with optimal tour\n" \
                "--formulation <form>               standard, MTZ or GG\n" \
                "--lazy                             use lazy constraints\n"\
                "--time-limit <time>                max overall time in seconds\n" \
                "--no-gui                           don't use GUI\n" \
                "--no-plot                          don't plot\n" \
                "--perf <max>                       do performance test with max size\n"\
                "--verbose <n>                      0=quiet, 1=default, 2=verbose, 3=debug\n" \
                "--help                             show this help\n\n"

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

void parse_cli(int argc, char **argv, instance *inst);

void parse_file(instance *inst, char *file_name);

void init_instance(instance * inst);

void free_instance(instance *inst);

void save_to_tsp_file(instance *inst);

#endif //TSP_OP2_UTILS_H

