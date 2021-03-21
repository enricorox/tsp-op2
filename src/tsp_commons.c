//
// TSP shared functions among formulations
//

#include "tsp_commons.h"

double dist(int i, int j, instance *inst) {
    double dx = inst->xcoord[i] - inst->xcoord[j];
    double dy = inst->ycoord[i] - inst->ycoord[j];
    if ( !inst->integer_costs ) return sqrt(dx*dx+dy*dy);
    int dis = (int) (sqrt(dx*dx+dy*dy) + 0.499999999); // nearest integer
    return dis+0.0;
}

// return CPLEX column position given an arc (i,j)
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

void build_model_base_directed(CPXENVptr env, CPXLPptr lp, instance *inst){
    char binary = 'B';
    int err;

    // allocate array for variable's name with trick
    char *cname[] = { (char *) calloc(BUFLEN, sizeof(char))};

    // add binary vars x(i,j) for i != j
    // one for each edge without auto-loops
    for( int i = 0; i < inst->tot_nodes; i++ ){
        for ( int j = 0; j < inst->tot_nodes; j++ ){
            // define variable name
            sprintf(cname[0], "x(%d,%d)", i+1,j+1);
            // define its cost
            double obj = dist(i,j,inst); // cost == distance
            // define its lower bound
            double lb = 0.0;
            // define its upper bound
            double ub = (i != j)?1:0;
            if((err = CPXnewcols(env, lp, 1, &obj, &lb, &ub, &binary, cname))) {
                printf(BOLDRED "[ERROR] CPXnewcols(): error code %d\n" RESET, err);
                exit(1);
            }
            // check xpos on the fly (can be removed if I'm sure it's ok)
            if (CPXgetnumcols(env,lp)-1 != xpos_compact(i,j, inst)) {
                printf(BOLDRED "[ERROR] xpos_compact() got a bad index!\n" RESET);
                free(cname[0]);
                free_instance(inst);
                exit(1);
            }
        }
    }
    free(cname[0]);
    char *rname[] = { (char *) calloc(BUFLEN, sizeof(char))};

    // add the 1 degree in and out constraints
    for(char out = 0; out < 2; out++)
        for(int h = 0; h < inst->tot_nodes; h++){
            // define right hand side
            double rhs = 1.0;
            // define the type of constraint (array) ('E' for equality)
            char sense = 'E';
            // define constraint name
            sprintf(rname[0], "degree_%s(%d)", out?"out":"in", h+1);
            if ((err = CPXnewrows(env, lp, 1, &rhs, &sense, NULL, rname)) ){
                printf(BOLDRED "[ERROR] CPXnewrows() error code %d\n" RESET, err);
                exit(1);
            }
            int lastrow_idx = CPXgetnumrows(env, lp) - 1; // constraint index starts from 0
            // change last row coefficients from 0 to 1
            for(int i = 0; i < inst->tot_nodes; i++){
                if(i == h) continue;
                if ((err = CPXchgcoef(env, lp, lastrow_idx, xpos_compact(out?h:i, out?i:h, inst), 1.0))) {
                    printf(BOLDRED "[ERROR] Cannot change coefficient: error code %d", err);
                }
            }
        }

    int err1, err2;
    // add "in or out" constraints
    for(int i = 0; i < inst->tot_nodes; i++) {
        for (int j = 0; j < inst->tot_nodes; j++) {
            if (j == i) continue;
            // define right hand side
            double rhs = 1.0;
            // define the type of constraint (array) ('E' for equality)
            char sense = 'L';
            // define constraint name
            sprintf(rname[0], "in_or_out(%d,%d)", i + 1, j + 1);
            if((err = CPXnewrows(env, lp, 1, &rhs, &sense, NULL, rname))) {
                printf(BOLDRED "[ERROR] CPXnewrows() error code %d\n" RESET, err);
                exit(1);
            }
            int lastrow_idx = CPXgetnumrows(env, lp) - 1; // constraint index starts from 0
            // change last row coefficients from 0 to 1 or -1
            err1 = CPXchgcoef(env, lp, lastrow_idx, xpos_compact(i, j, inst), 1.0);
            err2 = CPXchgcoef(env, lp, lastrow_idx, xpos_compact(j, i, inst), 1.0);
            if (err1 || err2) {
                printf(BOLDRED "[ERROR] Cannot change coefficient: error code %d", err);
            }
        }
    }

    free(rname[0]);
}