//
// Created by enrico on 13/05/21.
//

#ifndef TSP_OP2_HEURISTIC_KOPT_H
#define TSP_OP2_HEURISTIC_KOPT_H

#include "utils.h"

#define EPSILON 0.0000001

void reverse_chain(int *succ, int start, int stop);

double two_opt(instance *inst, int *succ, bool findmin);

#endif //TSP_OP2_HEURISTIC_KOPT_H
