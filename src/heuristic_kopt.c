//
// Created by enrico on 13/05/21.
//

#include <float.h>
#include <unistd.h>
#include "heuristic_kopt.h"
#include "distances.h"
#include "plot.h"

void reverse_chain(int *succ, int start, int stop){
    // initialize nodes
    int curr = start;
    int next = succ[curr];

    while(curr != stop){
        int temp = succ[next];
        succ[next] = curr;
        curr = next;
        next = temp;

        // check errors
        if(curr == start){
            printf("ERROR reversing chain\n");
            exit(1);
        }

    }
}

void two_opt_move(int *succ, int a, int b){
    int aprime = succ[a];
    int bprime = succ[b];

    reverse_chain(succ, aprime, b);

    succ[aprime] = bprime;
    succ[a] = b;
}

double two_opt(instance *inst, int *succ, bool findmin){
    while(!timeout(inst)){
        bool done = false;
        double min = DBL_MAX;
        int a, b;

        // select (all possible) node pairs
        for(int i = 0; i < inst->nnodes; i++){
            if((done && !findmin) || timeout(inst)) break;
            for(int j = 0; j < inst->nnodes; j++){
                if(i == j) continue;
                double delta = cost(i, j, inst) + cost(succ[i], succ[j], inst)
                        - cost(i, succ[i], inst) - cost(j, succ[j], inst);
                if((delta < 0 - EPSILON) && (delta < min)) {
                    print(inst, 'D', 2, "Shortcut found! i = %d, j = %d, delta = %f", i + 1, j + 1, delta);
                    min = delta;
                    a = i;
                    b = j;
                    done = true;
                    if(!findmin) break;
                }
            }
        }

        // exit if no shortcut
        if(min >= 0)
            break;

        // swap edges
        if(inst->verbose >= 3)
            printsucc(inst, succ);

        print(inst, 'D', 2, "Making a two-opt move...");

        two_opt_move(succ, a, b);

        if(inst->verbose >= 3)
            printsucc(inst, succ);
    }
    return cost_succ(inst, succ);
}