#ifndef TSP_H_ // begin ifndef
#define TSP_H_ // block multiple tsp.h includes

#include <sys/time.h>
#include <cplex.h>

#include "utils.h"
#include "formulation_commons.h"
#include "plot.h"

// formulations
#include "formulation_standard.h"
#include "formulation_MTZ.h"
#include "formulation_GG.h"

double cost(int i, int j, instance *inst);

void TSPOpt(instance *inst);

void save_model(instance *inst, CPXENVptr env, CPXLPptr lp);

#endif // end ifndef
