//
// Created by enrico on 19/03/21.
//

#ifndef TSP_OP2_MTZ_FORMULATION_H
#define TSP_OP2_MTZ_FORMULATION_H

#include <cplex.h>

#include "utils.h"
#include "tsp_functions.h"

void build_model_MTZ(instance *inst, CPXENVptr env, CPXLPptr lp);


#endif //TSP_OP2_MTZ_FORMULATION_H
