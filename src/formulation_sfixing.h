//
// Created by enrico on 03/05/21.
//

#ifndef TSP_OP2_FORMULATION_SFIXING_H
#define TSP_OP2_FORMULATION_SFIXING_H

#include "utils.h"
#include "formulation_cuts.h"

void build_model_sfixing(instance *);

void get_solution_sfixing(instance *inst);

void solve_sfixing(instance *inst);

#endif //TSP_OP2_FORMULATION_SFIXING_H
