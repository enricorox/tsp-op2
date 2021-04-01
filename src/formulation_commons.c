//
// TSP shared functions among formulations
//

#include "formulation_commons.h"

// ===== DIRECTED GRAPH FUNCTIONS =====

int xpos_compact(int i, int j, instance *inst){
    if((j < 0) || (i < 0) || (j >= inst->tot_nodes) || (i >= inst->tot_nodes)){
        printf(BOLDRED"[ERROR] xpos_compact(): unexpected i = %d, j = %d\n" RESET, i, j);
        free_instance(inst);
        exit(1);
    }
    int pos = i * inst->tot_nodes + j;
    return pos;
}

void add_x_vars_directed(CPXENVptr env, CPXLPptr lp, instance *inst){
    char binary = 'B';
    int err;

    // allocate array for variable's name with trick
    char *cname[] = { (char *) calloc(BUFLEN, sizeof(char))};

    // add binary vars x(i,j) for i != j
    // one for each edge without auto-loops
    for( int i = 0; i < inst->tot_nodes; i++ ){
        for ( int j = 0; j < inst->tot_nodes; j++ ){
            // define variable name
            snprintf(cname[0], BUFLEN, "x(%d,%d)", i+1,j+1);
            // define its cost
            double obj = cost(i, j, inst); // cost == distance
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
}

void add_degree_constraints_directed(CPXENVptr env, CPXLPptr lp, instance *inst){
    char *rname[] = { (char *) calloc(BUFLEN, sizeof(char))};
    int err;

    // define right hand side
    double rhs = 1.0;
    // define the type of constraint (array) ('E' for equality)
    char sense = 'E';

    // add the 1 degree in and out constraints
    for(char out = 0; out < 2; out++)
        for(int h = 0; h < inst->tot_nodes; h++){
            // define constraint name
            snprintf(rname[0], BUFLEN, "degree_%s(%d)", out?"out":"in", h+1);
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
    free(rname[0]);
}

void add_SEC2_constraints_directed(CPXENVptr env, CPXLPptr lp, instance *inst){
    char *rname[] = { (char *) calloc(BUFLEN, sizeof(char))};

    int err, err1, err2;
    // add Subtour Elimination Constraints for 2 nodes
    for(int i = 0; i < inst->tot_nodes; i++) {
        for (int j = 0; j < inst->tot_nodes; j++) {
            if (j == i) continue;
            // define right hand side
            double rhs = 1.0;
            // define the type of constraint (array) ('E' for equality)
            char sense = 'L';
            // define constraint name
            snprintf(rname[0], BUFLEN, "SEC2(%d,%d)", i + 1, j + 1);
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

void build_model_base_directed(CPXENVptr env, CPXLPptr lp, instance *inst){
    add_x_vars_directed(env, lp, inst);

    add_degree_constraints_directed(env, lp, inst);

    add_SEC2_constraints_directed(env, lp, inst);
}


// ===== UNDIRECTED GRAPH FUNCTIONS ====

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

void add_x_vars_undirected(CPXENVptr env, CPXLPptr lp, instance *inst){
    char binary = 'B';
    int err;

    // allocate array for variable's name with trick
    char *cname[] = {(char *) calloc(BUFLEN, sizeof(char))}; // string array required by cplex for batch insertions

    // add binary vars x(i,j) for i < j
    // one for each edge
    for(int i = 0; i < inst->tot_nodes; i++){
        for(int j = i+1; j < inst->tot_nodes; j++){
            // define variable name
            snprintf(cname[0], BUFLEN, "x(%d,%d)", i+1,j+1);
            // define its cost
            double obj = cost(i, j, inst); // cost == distance
            // define its lower bound
            double lb = 0.0;
            // define its upper bound
            double ub = 1.0;
            if(CPXnewcols(env, lp, 1, &obj, &lb, &ub, &binary, cname)) {
                free(cname[0]);
                printerr(inst, "Cannot add columns!");
            }
            // check xpos on the fly (can be removed if I'm sure it's ok?)
            if (CPXgetnumcols(env,lp)-1 != xpos(i, j, inst)) {
                free(cname[0]);
                printerr(inst, "xpos() got a bad index!");
            }
        }
    }
    free(cname[0]);
}

void add_degree_constraints_undirected(CPXENVptr env, CPXLPptr lp, instance *inst){
    char *rname[] = {(char *) calloc(BUFLEN, sizeof(char))};

    int nnz = inst->tot_nodes - 1;
    int index[nnz];
    double value[nnz];
    for(int i = 0; i < nnz; i++) value[i] = 1;

    // define right hand side
    double rhs = 2.0;
    // define the type of constraint (array) ('E' for equality)
    char sense = 'E';
    // define starting index
    int izero = 0;
    // add the 2 degree constraints
    for(int h = 0; h < inst->tot_nodes; h++){
        // build index array
        int k = 0;
        for(int i = 0; i < inst->tot_nodes; i++)
            if(i != h) index[k++] = xpos(h, i, inst);
        // define constraint name
        snprintf(rname[0], BUFLEN, "degree(%d)", h+1);
        if(CPXaddrows(env, lp, 0, 1, nnz, &rhs, &sense, &izero, index, value, NULL, rname)) {
            free(rname[0]);
            printerr(inst, "Cannot add rows!");
        }
    }
    free(rname[0]);
}

void build_model_base_undirected(CPXENVptr env, CPXLPptr lp, instance *inst){
    add_x_vars_undirected(env, lp, inst);

    add_degree_constraints_undirected(env, lp, inst);
}