//
// Created by enrico on 12/03/21.
//
#ifndef TSP_OP2_PLOT_H
#define TSP_OP2_PLOT_H

#include "utils.h"
#include "tsp.h"

#define GPDATA "gnuplot-data.dat"
#define GPCOMM "gnuplot-command.plt"
#define FILESVG "tsp-graph-solution.svg"

void plot(instance *inst, char const *rxstar);

#endif //TSP_OP2_PLOT_H
