//
// Created by enrico on 07/05/21.
//

#include "heuristics.h"
#include "heuristic_greedy.h"

void heuristic(instance * inst){
    switch(inst->heuristic){
        case EXTRAMILEAGE:
            if(inst->dist != EUC_2D)
                printerr(inst, "You need EUC_2D costs to use this heuristic!");
        case GREEDY:
        case GREEDYGRASP:
        default:
            gorilla_init(inst);
            break;
    }
    print(inst, 'I', 1, "Found zbest = %f", inst->zbest);
}