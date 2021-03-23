//
// Created by enrico on 19/03/21.
//

#ifndef TSP_OP2_FORMULATION_STANDARD_H
#define TSP_OP2_FORMULATION_STANDARD_H

#include <cplex.h>

#include "formulation_commons.h"

void build_model(instance *inst, CPXENVptr env, CPXLPptr lp);

void get_solution(instance *inst, CPXENVptr env, CPXLPptr lp);

#endif //TSP_OP2_FORMULATION_STANDARD_H
