//
// Created by enrico on 07/05/21.
//

#include "heuristic_greedy.h"
#include "distances.h"
#include "formulation_commons.h"

#define NTHREAD 4

int findnearest(instance *inst, const bool * visited, int node, int order){
    // flag for used nodes
    bool *selected = calloc(inst->nnodes, sizeof(bool));
    selected[node] = true;

    // latest node found
    int latest = -1;

    // find kth smallest edge
    for(int k = 0; k < order; k++) {
        // search minimum cost edge
        double min = DBL_MAX;
        for(int i = 0; i < inst->nnodes; i++) {
            if(visited[i] || selected[i]) {
                print(inst, 'D', 3, "Node %d skipped because already %s", i + 1, visited[i]?"visited":"selected");
                continue;
            }
            double c = cost(node, i, inst);
            print(inst, 'D', 3, "cost(%d, %d) = %f", node + 1, i + 1, c);
            if (c < min) {
                latest = i;
                min = c;
            }
        }
        if(min == DBL_MAX) break; // nothing found
        selected[latest] = true; // flag node as used
    }
    free(selected);
    return latest;
}

void print_visited(instance *inst, const bool * visited){
    printf("Visited nodes: ");
    for(int i = 0; i < inst->nnodes; i++)
        if(visited[i]) printf("%d ", i + 1);
    printf("\n");
}

double gorilla(instance *inst, int nstart, bool *visited, double *result){
    print(inst, 'D', 3, "*** Starting from %d ***", nstart + 1);
    bzero(visited, inst->nnodes * sizeof(bool));
    bzero(result, inst->nnodes * inst->nnodes * sizeof(double));
    int nedges = inst->nnodes;

    double zbest = 0;
    int curr = nstart;
    visited[curr] = true;
    for(int i = 0; i < nedges; i++){
        //print_visited(inst, visited);
        int order = 1;
        if(inst->heuristic == GREEDYGRASP && nrand() > 85){
            int t = nrand();
            if(t > 80) order = 2;
            if(t > 95) order = 3;
        }
        int next = findnearest(inst, visited, curr, order);
        if(next == -1) next = nstart; // close the circuit
        visited[next] = true;

        print(inst, 'D', 3, "curr = %d, next = %d", curr + 1, next + 1);
        result[xpos_directed(curr, next, inst)] = 1;
        zbest += cost(curr, next, inst);
        curr = next;
    }
    print(inst, 'D', 3, "Cost = %f", zbest);
    return zbest;
}

void gorilla_init(instance *inst){
    // set graph as undirected
    inst->directed = true;

    // visited nodes (eaten bananas)
    bool *visited = (bool *) calloc(inst->nnodes, sizeof(bool));

    // initialize vectors
    double *x = (double *) calloc(inst->nnodes * inst->nnodes, sizeof(double));
    inst->xbest =  (double *) calloc(inst->nnodes * inst->nnodes, sizeof(double));

    // initialize cost
    double z = inst->zbest = DBL_MAX;

    while(!timeout(inst)) {
        // use every nodes as initial node
        for (int start = 0; (start < inst->nnodes) && !timeout(inst); start++) {
            z = gorilla(inst, start, visited, x);

            // update the minimum
            if (z < inst->zbest) {
                inst->zbest = z;

                // swap vectors
                double *t = x;
                x = inst->xbest;
                inst->xbest = t;
            }
        }

        // break if deterministic cycle
        if(inst->heuristic == GREEDY) break;
    }

    free(visited);
    free(x);
}