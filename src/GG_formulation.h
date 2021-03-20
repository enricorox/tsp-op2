//
// Created by enrico on 20/03/21.
//

#ifndef TSP_OP2_GG_FORMULATION_H
#define TSP_OP2_GG_FORMULATION_H

#include <cplex.h>

#include "utils.h"
#include "tsp_commons.h"

int ypos(int i, int j, instance *inst);

void build_model_GG(instance *inst, CPXENVptr env, CPXLPptr lp);

void get_solution_GG(instance *inst, CPXENVptr env, CPXLPptr lp);

#endif //TSP_OP2_GG_FORMULATION_H
