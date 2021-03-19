//
// Created by enrico on 19/03/21.
//

#ifndef TSP_OP2_STANDARD_FORMULATION_H
#define TSP_OP2_STANDARD_FORMULATION_H

#include "tsp.h"

int xpos(int i, int j, instance *inst);

double dist(int i, int j, instance *inst);

void build_model(instance *inst, CPXENVptr env, CPXLPptr lp);

#endif //TSP_OP2_STANDARD_FORMULATION_H
