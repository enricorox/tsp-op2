//
// Created by enrico on 19/03/21.
//

#ifndef TSP_OP2_MTZ_FORMULATION_H
#define TSP_OP2_MTZ_FORMULATION_H

#include "tsp.h"

int xpos_compact(int i, int j, instance *inst);

int upos(int i, instance *inst);

void build_model_MTZ(instance *inst, CPXENVptr env, CPXLPptr lp);


#endif //TSP_OP2_MTZ_FORMULATION_H
