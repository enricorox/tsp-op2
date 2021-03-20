#ifndef TSP_H_ // begin ifndef
#define TSP_H_ // block multiple tsp.h includes

#include <sys/time.h>
#include <cplex.h>

#include "utils.h"
#include "plot.h"

// formulations
#include "standard_formulation.h"
#include "MTZ_formulation.h"
#include "GG_formulation.h"

void TSPOpt(instance *inst);

void save_model(instance *inst, CPXENVptr env, CPXLPptr lp);

#endif // end ifndef
