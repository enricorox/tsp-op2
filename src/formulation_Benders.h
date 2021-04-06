//
// Created by enrico on 19/03/21.
//

#ifndef TSP_OP2_FORMULATION_BENDERS_H
#define TSP_OP2_FORMULATION_BENDERS_H

#include <cplex.h>

#include "formulation_commons.h"

void loop_benders(instance *inst);

void get_solution_Benders(instance *inst);

#endif //TSP_OP2_FORMULATION_BENDERS_H
