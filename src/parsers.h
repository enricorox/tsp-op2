//
// Created by enrico on 23/03/21.
//

#ifndef TSP_OP2_PARSERS_H
#define TSP_OP2_PARSERS_H

#include <unistd.h>

#include "utils.h"

#define USAGE   "TSP solver 0.x, a solver for Travelling Salesman Problem with CPLEX\n" \
                "Author: Enrico Rossignolo - 1218951 - enrico.rossignolo@studenti.unipd.it\n\n" \
                "Usage: ./tsp (--file <file-tsp> | --perf <max>) [options]\n" \
                "Options:\n"\
                "--opt-tour <file-opt-tsp>          tsp file with optimal tour\n" \
                "--formulation <form>               standard, MTZ or GG\n" \
                "--lazy                             use lazy constraints\n"\
                "--time-limit <time>                max overall time in seconds\n" \
                "--no-gui                           don't use GUI\n" \
                "--no-plot                          don't plot\n" \
                "--no-int-costs                     don't force integer costs (apply to EUC_2D)\n" \
                "--perf <max>                       do performance test with max size\n"\
                "--verbose <n>                      0=quiet, 1=default, 2=verbose, 3=debug\n" \
                "--help                             show this help\n\n"


void parse_cli(int argc, char **argv, instance *inst);

void parse_file(instance *inst, char *file_name);

char * find_opt_file(instance *inst);

#endif //TSP_OP2_PARSERS_H
