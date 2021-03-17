//
// Created by enrico on 17/03/21.
//

#ifndef TSP_OP2_UTILS_H
#define TSP_OP2_UTILS_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// ANSI escape sequences (from stackoverflow)
#define RESET   "\033[0m"
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */


#define BUFLEN 256
#define USAGE   "Usage: ./tsp --file <file-tsp> [options]\n"\
                "Options:\n"                                  \
                "--opt-tour <file-opt-tsp>          tsp file with optimal tour\n"\
                "--time-limit <time>                max overall time in seconds\n" \
                "--verbose <n>                      0=quiet, 1=default, 2=verbose, 3=debug\n" \
                "--help                             show this help\n\n"

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

void parse_command_line(int argc, char **argv, instance *inst);

void parse_tsp_file(instance *inst, char opt);

void init_instance(instance * inst);

void free_instance(instance *inst);

#endif //TSP_OP2_UTILS_H

