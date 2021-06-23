//
// Created by enrico on 16/06/21.
//

#include <float.h>
#include "heuristic_tabu_search.h"
#include "heuristic_kopt.h"
#include "distances.h"

double search(instance *inst, int *succ, bool findmin, long *tabu, long tenure){
    // initialize local minimum
    int *xbest = calloc(inst->nnodes, sizeof(int));
    memcpy(xbest, succ, inst->nnodes * sizeof(int));
    double zbest = cost_succ(inst, succ);

    long now = 0; // iteration counter

    while(!timeout(inst)){
        now++;
        bool found = false;
        double min = DBL_MAX;
        int a, b;

        // select node pairs (2-opt neighbourhood), skip tabu
        for(int i = 0; i < inst->nnodes; i++){
            if((found && !findmin) || timeout(inst)) break;

            // check tabu nodes
            if(now - tabu[i] <= tenure) {
                print(inst, 'D', 2, "Node %d is tabu!", i);
                continue;
            }

            for(int j = 0; j < inst->nnodes; j++){
                if((found && !findmin) || timeout(inst)) break;
                if(i == j) continue;

                // check tabu
                if(now - tabu[j] <= tenure) {
                    print(inst, 'D', 2, "Node %d is tabu!", j);
                    continue;
                }

                double delta = cost(i, j, inst) + cost(succ[i], succ[j], inst)
                               - cost(i, succ[i], inst) - cost(j, succ[j], inst);
                if(delta < min) {
                    min = delta;
                    a = i;
                    b = j;
                    found = true;
                    if(!findmin) break;
                }
            }
        }

        if(!found){
            print(inst, 'W', 1, "Cannot find other neighbours!");
            break;
        }

        print(inst, 'D', 2, "Making a two-opt move...");
        two_opt_move(succ, a, b);

        // update tabu counters
        tabu[a] = tabu[b] = now;

        // update minimum
        double z = cost_succ(inst, succ);
        if(z < zbest){
            zbest = z;
            memcpy(xbest, succ, inst->nnodes * sizeof(int));
        }
    }
    free(xbest);
    return zbest;
}

double tabu_search(instance *inst, int *succ){
    long *tabu = calloc(inst->nnodes, sizeof(long));
    bool findmin;
    long tenure;

    switch(inst->ref_heuristic){
        case TABU_SEARCH1:
            findmin = true;
            tenure = 20;
            break;
        case TABU_SEARCH2:
            findmin = true;
            tenure = inst->nnodes / 10;
            break;
        case TABU_SEARCH3:
            findmin = true;
            tenure = inst->nnodes / 15;
            break;
        default:
            printerr(inst, "tabu_search(): illegal heuristic");
    }

    // initialize tabu (all nodes are unlocked)
    for(int i = 0; i < inst->nnodes; i++) tabu[i] = -tenure;

    // search on 2-opt neighbours
    double zbest = search(inst, succ, findmin, tabu, tenure);

    free(tabu);
    return zbest;
}