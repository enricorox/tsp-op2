//
// Created by enrico on 19/03/21.
//

#include "tsp_commons.h"

double dist(int i, int j, instance *inst) {
    double dx = inst->xcoord[i] - inst->xcoord[j];
    double dy = inst->ycoord[i] - inst->ycoord[j];
    if ( !inst->integer_costs ) return sqrt(dx*dx+dy*dy);
    int dis = (int) (sqrt(dx*dx+dy*dy) + 0.499999999); // nearest integer
    return dis+0.0;
}

// return CPLEX column position given the coordinates of a cell in adjacency matrix
int xpos(int i, int j, instance *inst) {
    if((i == j) || (j < 0) || (i < 0) || (j >= inst->tot_nodes) || (i >= inst->tot_nodes)){
        printf(BOLDRED"[ERROR] xpos(): unexpected i = %d, j = %d\n" RESET, i, j);
        free_instance(inst);
        exit(1);
    }
    // Check if it's a cell above diagonal
    if(i > j) return xpos(j, i, inst);

    // Below formula derives from
    // pos = j - 1 + i*n - sum_1^i(k+1)
    int pos = i * inst->tot_nodes + j - (( i + 1 ) * ( i + 2 )) / 2;
    return pos;
}

int xpos_compact(int i, int j, instance *inst){
    if((j < 0) || (i < 0) || (j >= inst->tot_nodes) || (i >= inst->tot_nodes)){
        printf(BOLDRED"[ERROR] xpos_compact(): unexpected i = %d, j = %d\n" RESET, i, j);
        free_instance(inst);
        exit(1);
    }
    int pos = i * inst->tot_nodes + j;
    return pos;
}

int upos(int i, instance *inst){
    if(i < 0 || i >= inst->tot_nodes){
        printf(BOLDRED"[ERROR] xpos(): unexpected i = %d\n" RESET, i);
        exit(1);
    }
    int upos = inst->tot_nodes * inst->tot_nodes + i;
    return upos;
}