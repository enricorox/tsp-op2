//
// Created by enrico on 19/03/21.
//

#ifndef TSP_OP2_FORMULATION_MTZ_H
#define TSP_OP2_FORMULATION_MTZ_H

#include <cplex.h>

#include "utils.h"
#include "formulation_commons.h"

int upos(int i, instance *inst);

void add_uconsistency_vars(instance *inst, CPXENVptr env, CPXLPptr lp);

void add_uconsistency_constraints(instance *inst, CPXENVptr env, CPXLPptr lp);

void add_uconsistency_constraints_lazy(instance *inst, CPXENVptr env, CPXLPptr lp);

void build_model_MTZ(instance *inst, CPXENVptr env, CPXLPptr lp);

void build_model_MTZ(instance *inst, CPXENVptr env, CPXLPptr lp);

void get_solution_MTZ(instance *inst, CPXENVptr env, CPXLPptr lp);


#endif //TSP_OP2_FORMULATION_MTZ_H
