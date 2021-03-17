#ifndef TSP_H_ // begin ifndef
#define TSP_H_ // block multiple tsp.h includes

#include "utils.h"
#include "plot.h"

#include <cplex.h>




char * TSPOpt(instance *inst);
double dist(int i, int j, instance *inst);
int xpos(int i, int j, instance *inst);
void build_model(instance *inst, CPXENVptr env, CPXLPptr lp);

#endif // end ifndef
