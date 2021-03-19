//
// Created by enrico on 19/03/21.
//

#ifndef TSP_OP2_TSP_FUNCTIONS_H
#define TSP_OP2_TSP_FUNCTIONS_H

#include <math.h>
#include "utils.h"

double dist(int i, int j, instance *inst);

int xpos(int i, int j, instance *inst);

int xpos_compact(int i, int j, instance *inst);

int upos(int i, instance *inst);

#endif //TSP_OP2_TSP_FUNCTIONS_H
