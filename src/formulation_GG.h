//
// Created by enrico on 20/03/21.
//

#ifndef TSP_OP2_FORMULATION_GG_H
#define TSP_OP2_FORMULATION_GG_H

#include <cplex.h>

#include "utils.h"
#include "formulation_commons.h"

int ypos(int i, int j, instance *inst);

void add_flow_vars(instance *inst, CPXENVptr env, CPXLPptr lp);

void add_flow_constraints(instance *inst, CPXENVptr env, CPXLPptr lp);

void add_flow_constraints_lazy(instance *inst, CPXENVptr env, CPXLPptr lp);

void add_link_constraints(instance *inst, CPXENVptr env, CPXLPptr lp);

void add_link_constraints_lazy(instance *inst, CPXENVptr env, CPXLPptr lp);

void build_model_GG(instance *inst, CPXENVptr env, CPXLPptr lp);

void get_solution_GG(instance *inst, CPXENVptr env, CPXLPptr lp);

#endif //TSP_OP2_FORMULATION_GG_H
