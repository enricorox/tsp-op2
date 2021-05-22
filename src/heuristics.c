//
// Created by enrico on 07/05/21.
//

#include <unistd.h>
#include "heuristics.h"
#include "heuristic_greedy.h"
#include "heuristic_extramileage.h"
#include "tsp.h"
#include "heuristic_kopt.h"
#include "heuristic_VNS.h"

void heuristic(instance * inst){
    start(inst);

    // define time limit for construction heuristics
    double timelimit;
    if(inst->ref_heuristic != RHLAST)
        timelimit = inst->time_limit/2;
    else
        timelimit = inst->time_limit;

    // choose constructive heuristic
    switch(inst->cons_heuristic){
        case EXTRAMILEAGE:
        case EXTRAMILEAGECONVEXHULL:
            if(inst->dist != EUC_2D)
                printerr(inst, "You need EUC_2D distance to use this cons_heuristic!");
            extramileage(inst);
            break;
        case GREEDY:
        case GREEDYGRASP:
            greedy(inst, timelimit);
            break;
        default:
            printerr(inst, "Heuristic not found (internal error)");
    }

    if(inst->verbose >= 3)
        plot(inst, inst->xbest);

    // convert xbest to successors vector
    inst->succ = xtosucc(inst, inst->xbest);

    // choose refinement heuristic
    switch(inst->ref_heuristic) {
        case TWO_OPT:
            inst->zbest = two_opt(inst, inst->succ, false);
            break;
        case TWO_OPT_MIN:
            inst->zbest = two_opt(inst, inst->succ, true);
            break;
        case VNS1:
            inst->zbest = VNS(inst);
        default:
            print(inst, 'D', 3, "No refinement heuristic used");
            break;
    }

    print(inst, 'I', 1, "Found zbest = %f", inst->zbest);

    if(inst->opt_tour != NULL ) {
        double zopt = get_zstar_opt(inst);
        double ratio = inst->zbest / zopt;
        double error = (ratio - 1) * 100;
        print(inst, 'I', 1, "Known solution z* = %f, ratio = %f, error = %f%", zopt, ratio, error);
    }

    plot_succ(inst, inst->succ);
}