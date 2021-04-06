//
// Created by enrico on 20/03/21.
//

#ifndef TSP_OP2_FORMULATION_GG_H
#define TSP_OP2_FORMULATION_GG_H

#include <cplex.h>

#include "utils.h"
#include "formulation_commons.h"

int ypos(int i, int j, instance *inst);

void add_flow_vars(instance *inst);

void add_flow_constraints(instance *inst);

void add_linking_constraints(instance *inst);

void build_model_GG(instance *inst);

void get_solution_GG(instance *inst);

#endif //TSP_OP2_FORMULATION_GG_H
