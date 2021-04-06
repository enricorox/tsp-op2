//
// Created by enrico on 19/03/21.
//

#ifndef TSP_OP2_FORMULATION_COMMONS_H
#define TSP_OP2_FORMULATION_COMMONS_H

#include <math.h>
#include <cplex.h>

#include "utils.h"
#include "distances.h"

// ===== DIRECTED GRAPH =====
int xpos_compact(int i, int j, instance *inst);

void add_x_vars_directed(instance *inst);

void add_degree_constraints_directed(instance *inst);

void add_SEC2_constraints_directed(instance *inst);

void build_model_base_directed(instance *inst);

// ===== UNDIRECTED GRAPH =====
int xpos(int i, int j, instance *inst);

void build_model_base_undirected(instance *inst);

#endif //TSP_OP2_FORMULATION_COMMONS_H
