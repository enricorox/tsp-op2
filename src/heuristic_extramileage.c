#include "utils.h"
#include "distances.h"
#include "formulation_commons.h"

//
// Created by enrico on 08/05/21.
//
double diameter(instance *inst, int *a, int *b){
    double d = 0;
    for(int i = 0; i < inst->nnodes; i++){
        for(int j = i + 1; j < inst->nnodes; j++){
            //printf("cost(%d,%d) = ", i+1, j+1);
            double c = cost(i, j, inst);
            //printf("%f\n", c);
            if(c > d){
                d = c;
                *a = i;
                *b = j;
            }
        }
    }
    return d;
}

void init_extramileage(instance *inst, bool *visited){
    // find the set diameter
    int a, b;
    diameter(inst, &a, &b);
    visited[a] = visited[b] = true;

    // select (a,b) and (b,a) edges
    inst->xbest[xpos_directed(a, b, inst)] = 1;
    inst->xbest[xpos_directed(b, a, inst)] = 1;

}

void extramileage(instance *inst){
    inst->directed = true;
    bool *visited = (bool *) calloc(inst->nnodes, sizeof(bool));
    inst->xbest = (double *) calloc(inst->nnodes * inst->nnodes, sizeof(double));

    init_extramileage(inst, visited);

    // select one edge at each iteration
    for(int it = 0; it < inst->nnodes - 2; it++){
        double min = DBL_MAX;   // minimum extra-mileage
        int im, jm, hm;         // minimum corresponding nodes

        // scan selected edges
        for(int i = 0; i < inst->nnodes; i++){
            if(!visited[i]) continue; // skip not used nodes
            for(int j = 0; j < inst->nnodes; j++){
                if(!visited[j]) continue; // skip not used nodes

                // select a free node
                for(int h = 0; h < inst->nnodes; h++){
                    if(visited[h]) continue; // skip used nodes

                    // compute extra-mileage
                    double extram = cost(i, h, inst) + cost(h, i, inst) - cost(i, j, inst);
                    if(extram < min){
                        min = extram;
                        im = i;
                        jm = j;
                        hm = h;
                    }
                }
            }
        }
        // unselect (im,jm)
        inst->xbest[xpos_directed(im, jm, inst)] = 0;
        // select (im, hm) and (hm, jm)
        inst->xbest[xpos_directed(im, hm, inst)] = 1;
        inst->xbest[xpos_directed(hm, jm, inst)] = 1;
        // visit the node
        visited[hm] = true;

        if(timeout(inst))
            printerr(inst,"Time-limit too short!");
    }
}