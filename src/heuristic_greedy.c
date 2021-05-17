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
            // skip used nodes
            if(visited[i] || selected[i]) {
                print(inst, 'D', 3, "Node %d skipped because already %s", i + 1, visited[i]?"visited":"selected");
                continue;
            }
            double c = cost(node, i, inst);
            print(inst, 'D', 3, "cost(%d, %d) = %f", node + 1, i + 1, c);

            // update the minimum
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

    // reset arrays
    bzero(visited, inst->nnodes * sizeof(bool));
    bzero(result, inst->nnodes * inst->nnodes * sizeof(double));

    double z = 0;
    int curr = nstart; // current node
    visited[curr] = true; // flag as used

    // find nnodes edges
    for(int i = 0; i < inst->nnodes; i++){
        //print_visited(inst, visited);

        int order = 1; // find the nearest node

        // use randomness with probability of 15%
        if(inst->cons_heuristic == GREEDYGRASP && nrand() > 85){
            int t = nrand();
            if(t > 80) order = 2; // find the 2nd nearest node with probability of 15%
            if(t > 95) order = 3; // find the 3rd nearest node with probability of 5%
        }

        int next = findnearest(inst, visited, curr, order);

        // close the circuit if there are no nodes
        if(next == -1) next = nstart;

        // flag the node
        visited[next] = true;

        print(inst, 'D', 3, "curr = %d, next = %d", curr + 1, next + 1);

        // select (curr, next) edge
        result[xpos_directed(curr, next, inst)] = 1;

        // accumulate cost
        z += cost(curr, next, inst);

        // update current node
        curr = next;

        // handling time-limit
        if(timeout(inst))
            return DBL_MAX;
    }
    print(inst, 'D', 3, "Cost = %f", z);
    return z;
}

void greedy(instance *inst, double timelimit){
    // set graph as undirected
    inst->directed = true;

    // visited nodes (eaten bananas)
    bool *visited = (bool *) calloc(inst->nnodes, sizeof(bool));

    // initialize vectors
    double *x = (double *) calloc(inst->nnodes * inst->nnodes, sizeof(double));
    inst->xbest =  (double *) calloc(inst->nnodes * inst->nnodes, sizeof(double));

    // initialize cost
    double z = inst->zbest = DBL_MAX;

    while(!timeouts(inst, timelimit)) {
        // use every nodes as initial node
        for (int start = 0; (start < inst->nnodes) && !timeouts(inst, timelimit); start++) {
            // compute gorilla's path
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
        if(inst->cons_heuristic == GREEDY) break;
    }

    free(visited);
    free(x);

    if(inst->zbest == DBL_MAX)
        printerr(inst, "Time-limit is too short!");
}