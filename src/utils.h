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
                "Usage: ./tsp --file <file-tsp> [options]\n" \
                "Options:\n"\
                "--opt-tour <file-opt-tsp>          tsp file with optimal tour\n" \
                "--time-limit <time>                max overall time in seconds\n" \
                "--no-gui                           don't use GUI\n" \
                "--no-plot                          don't plot\n" \
                "--verbose <n>                      0=quiet, 1=default, 2=verbose, 3=debug\n" \
                "--help                             show this help\n\n"

enum formulation_t {STANDARD, MTZ, GG};
char *formulation_names[3];

// define a general instance of the problem
typedef struct{
    // from cli
    char *input_tsp_file_name; // can be very large if it contains parent directories!
    char *input_opt_file_name;
    enum formulation_t formulation;
    double time_limit;
    bool gui;
    bool do_plot;
    int verbose;

    // from file (2nd cell for opt tour)
    char *name[2];
    char *comment[2];
    int tot_nodes;
    double *xcoord, *ycoord;
    int *opt_tour;

    // other parameters
    bool integer_costs;
    bool directed;
} instance;

void parse_cli(int argc, char **argv, instance *inst);

void parse_file(instance *inst, char *file_name);

void init_instance(instance * inst);

void free_instance(instance *inst);

#endif //TSP_OP2_UTILS_H

