#include <unistd.h>
#include "utils.h"
#include "distances.h"
#include "formulation_commons.h"
#include "graham_scan.h"
#include "plot.h"
//
// Created by enrico on 08/05/21.
//

/**
 * Convert the instance points into a PointSet
 * @param inst instance pointer
 * @return a PointSet containing all the instance points
 */
PointSet * insttopointset(instance *inst){
    PointSet *ps = calloc(1, sizeof(PointSet));
    ps->num_points = inst->nnodes;
    ps->points = calloc(inst->nnodes, sizeof(Point));

    for(int i = 0; i < inst->nnodes; i++){
        ps->points[i].xCoord = inst->xcoord[i];
        ps->points[i].yCoord = inst->ycoord[i];
        ps->points[i].id = i;
    }
    return ps;
}

/**
 * Select a convex hull using the library https://github.com/jwlodek/Graham-Scan
 * @param inst instance pointer
 * @param visited array of visited nodes
 */
int selecthull(instance *inst, bool *visited){
    PointSet *ps = insttopointset(inst);
    PointSet *sol = remove_degeneracy(compute_convex_hull(ps));

    for(int i = 0; i < sol->num_points - 1; i++){
        int a = sol->points[i].id;
        int b = sol->points[i + 1].id;
        //printf("a = %d, b = %d\n", a + 1, b + 1);
        printf("p%d = (%f,%f)\n", a+1, sol->points[i].xCoord, sol->points[i].yCoord);
        inst->xbest[xpos_directed(a, b, inst)] = 1;
        visited[a] = true;
    }

    // close the circuit
    int a = sol->points[sol->num_points - 1].id;
    int b = sol->points[0].id;
    //printf("a = %d, b = %d\n", a + 1, b + 1);
    printf("p%d = (%f,%f)\n", a+1, sol->points[sol->num_points-1].xCoord, sol->points[sol->num_points-1].yCoord);
    visited[a] = true;
    inst->xbest[xpos_directed(a, b, inst)] = 1;

    printf("Hull selected!\n");
    if(inst->verbose >= 3)
        plot(inst, inst->xbest);
    return sol->num_points;
}

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

int init_extramileage(instance *inst, bool *visited){
    if(inst->cons_heuristic == EXTRAMILEAGE) { // find set diameter
        int a, b;
        diameter(inst, &a, &b);
        visited[a] = visited[b] = true;

        // select (a,b) and (b,a) edges
        inst->xbest[xpos_directed(a, b, inst)] = 1;
        inst->xbest[xpos_directed(b, a, inst)] = 1;
        if(inst->verbose >= 3)
            plot(inst, inst->xbest);

        return 2;
    }else // find convex hull
        return selecthull(inst, visited);
}

void extramileage(instance *inst){
    // use directed graph
    inst->directed = true;

    // initialize vectors
    bool *visited = (bool *) calloc(inst->nnodes, sizeof(bool));
    inst->xbest = (double *) calloc(inst->nnodes * inst->nnodes, sizeof(double));

    // find diameter or convex hull
    int selected = init_extramileage(inst, visited);

    // add one edge at each iteration
    for(int it = 0; it < inst->nnodes - selected; it++){
        double min = DBL_MAX;   // minimum extra-mileage
        int im, jm, hm;         // minimum corresponding nodes

        // scan selected edges
        for(int i = 0; i < inst->nnodes; i++){
            if(!visited[i]) continue; // skip not used nodes
            if(timeout(inst)) break;

            for(int j = 0; j < inst->nnodes; j++){
                if(i == j) continue; // skip auto-loops
                if(!visited[j]) continue; // skip not used nodes
                if(timeout(inst)) break;

                // select a free node with minimum extra-mileage
                for(int h = 0; h < inst->nnodes; h++){
                    if(visited[h]) continue; // skip used nodes
                    if(inst->xbest[xpos_directed(i, j, inst)] == 0) continue; // skip unselected edges
                    // compute extra-mileage
                    double extram = cost(i, h, inst) + cost(h, i, inst) - cost(i, j, inst);

                    // update the minimum
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

        //plot(inst, inst->xbest);

        if(timeout(inst))
            printerr(inst,"Time-limit too short!");
    }

    free(visited);
}