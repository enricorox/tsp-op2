#ifndef TSP_H_ // begin ifndef
#define TSP_H_ // block multiple tsp.h includes

#include <sys/time.h>
#include <cplex.h>

#include "utils.h"
#include "formulation_commons.h"
#include "plot.h"

// formulations
#include "formulation_Benders.h"
#include "formulation_cuts.h"
#include "formulation_MTZ.h"
#include "formulation_GG.h"

#include "formulation_sfixing.h"

void TSPOpt(instance *inst);

void save_model(instance *inst);

double get_zstar_opt(instance *inst);

#endif // end ifndef
