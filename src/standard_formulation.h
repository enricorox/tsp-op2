//
// Created by enrico on 19/03/21.
//

#ifndef TSP_OP2_STANDARD_FORMULATION_H
#define TSP_OP2_STANDARD_FORMULATION_H

#include <cplex.h>

#include "tsp_commons.h"

void build_model(instance *inst, CPXENVptr env, CPXLPptr lp);

#endif //TSP_OP2_STANDARD_FORMULATION_H
