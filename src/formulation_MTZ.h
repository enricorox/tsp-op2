//
// Created by enrico on 19/03/21.
//

#ifndef TSP_OP2_FORMULATION_MTZ_H
#define TSP_OP2_FORMULATION_MTZ_H

#include <cplex.h>

#include "utils.h"
#include "formulation_commons.h"

int upos(int i, instance *inst);

void add_uconsistency_vars(instance *inst);

void add_uconsistency_constraints(instance *inst);

void build_model_MTZ(instance *inst);

void get_solution_MTZ(instance *inst);


#endif //TSP_OP2_FORMULATION_MTZ_H
