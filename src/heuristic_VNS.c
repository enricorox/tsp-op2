//
// Created by enrico on 19/05/21.
//

#include <float.h>
#include "heuristic_VNS.h"
#include "heuristic_kopt.h"

void reord(instance *inst, int *succ, int *a, int *b, int *c){
    int curr = succ[*a];
    int count = inst->nnodes;
    while(curr != *a){
        if(curr == *b) break; // already ordered
        if(curr == *c){
            // swap & break
            int t = *c;
            *c = *b;
            *b = t;
            break;
        }
        // update curr
        curr = succ[curr];

        if(count == 0) printerr(inst, "Error finding 3 chains!");
        count--;
    }
}

void kick(instance *inst, int *succ, int k){
    // define random nodes
    int a, b, c, d;
    switch(k){
        case 3:
            // choose 1st node
            a = rand() % inst->nnodes;

            // choose 2nd node
            do{b = rand() % inst->nnodes;}while(b == a);

            // choose 3rd node
            do{c = rand() % inst->nnodes;}while((c == a) || (c == b));

            // reorder based on successors
            reord(inst, succ, &a, &b, &c);

            // find successors
            int aprime = succ[a];
            int bprime = succ[b];
            int cprime = succ[c];

            // reverse chain
            reverse_chain(succ, bprime, c);

            // reconnect chains
            succ[a] = c;
            succ[bprime] = aprime;
            succ[b] = cprime;
            break;
        case 4: // TODO k = 4
        default:
            printerr(inst,"kick(): k = %d not implemented", k);
    }
}

double VNS(instance *inst){
    // build a temporary solution vector (successors)
    int *sol = calloc(inst->nnodes, sizeof(int));

    double zbest = DBL_MAX;

    bool findmin = true;

    switch(inst->ref_heuristic){
        case VNS1: // two-opt(-min) and jump on three-opt neighbourhood
            findmin = false;
        case VNS2:
            while(!timeout(inst)){
                // copy best solution
                memcpy(sol, inst->succ, inst->nnodes * sizeof(int));

                // perturb it
                kick(inst, sol, 3);

                // find local optimum in 2-opt neighborhood
                double z = two_opt(inst, sol, findmin);

                // update minimum
                if(z < zbest){
                    zbest = z;

                    // swap vectors
                    int *t = sol;
                    sol = inst->succ;
                    inst->succ = t;
                }
            }
            break;
        default:
            printerr(inst, "VNS(): heuristic not implemented");
    }

    free(sol);
    return zbest;
}