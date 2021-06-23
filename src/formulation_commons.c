//
// TSP shared functions among formulations
//

#include "formulation_commons.h"

// ===== DIRECTED GRAPH FUNCTIONS =====

int xpos_directed(int i, int j, instance *inst){
    if((j < 0) || (i < 0) || (j >= inst->nnodes) || (i >= inst->nnodes)){
        printf(BOLDRED"[ERROR] xpos_directed(): unexpected i = %d, j = %d\n" RESET, i, j);
        free_instance(inst);
        exit(1);
    }
    int pos = i * inst->nnodes + j;
    return pos;
}

void add_x_vars_directed(instance *inst){
    char binary = 'B';
    int err;

    // allocate array for variable's name with trick
    char *cname[] = { (char *) calloc(BUFLEN, sizeof(char))};

    // add binary vars x(i,j) for i != j
    // one for each edge without auto-loops
    for(int i = 0; i < inst->nnodes; i++ ){
        for (int j = 0; j < inst->nnodes; j++ ){
            // define variable name
            snprintf(cname[0], BUFLEN, "x(%d,%d)", i+1,j+1);
            // define its cost
            double obj = cost(i, j, inst); // cost == distance
            // define its lower bound
            double lb = 0.0;
            // define its upper bound
            double ub = (i != j)?1:0;
            if((err = CPXnewcols(inst->CPXenv, inst->CPXlp, 1, &obj, &lb, &ub, &binary, cname))) {
                printf(BOLDRED "[ERROR] CPXnewcols(): error code %d\n" RESET, err);
                exit(1);
            }
            // check xpos_undirected on the fly (can be removed if I'm sure it's ok)
            if (CPXgetnumcols(inst->CPXenv,inst->CPXlp)-1 != xpos_directed(i, j, inst)) {
                printf(BOLDRED "[ERROR] xpos_directed() got a bad index!\n" RESET);
                free(cname[0]);
                free_instance(inst);
                exit(1);
            }
        }
    }
    free(cname[0]);
}

void add_degree_constraints_directed(instance *inst){
    char *rname[] = { (char *) calloc(BUFLEN, sizeof(char))};
    int err;

    // define right hand side
    double rhs = 1.0;
    // define the type of constraint (array) ('E' for equality)
    char sense = 'E';

    // add the 1 degree in and out constraints
    for(char out = 0; out < 2; out++)
        for(int h = 0; h < inst->nnodes; h++){
            // define constraint name
            snprintf(rname[0], BUFLEN, "degree_%s(%d)", out?"out":"in", h+1);
            if ((err = CPXnewrows(inst->CPXenv, inst->CPXlp, 1, &rhs, &sense, NULL, rname)) ){
                printf(BOLDRED "[ERROR] CPXnewrows() error code %d\n" RESET, err);
                exit(1);
            }
            int lastrow_idx = CPXgetnumrows(inst->CPXenv, inst->CPXlp) - 1; // constraint index starts from 0
            // change last row coefficients from 0 to 1
            for(int i = 0; i < inst->nnodes; i++){
                if(i == h) continue;
                if ((err = CPXchgcoef(inst->CPXenv, inst->CPXlp, lastrow_idx,
                                      xpos_directed(out ? h : i, out ? i : h, inst), 1.0))) {
                    printf(BOLDRED "[ERROR] Cannot change coefficient: error code %d", err);
                }
            }
        }
    free(rname[0]);
}

void add_SEC2_constraints_directed(instance *inst){
    char *rname[] = { (char *) calloc(BUFLEN, sizeof(char))};

    int err, err1, err2;
    // add Subtour Elimination Constraints for 2 nodes
    for(int i = 0; i < inst->nnodes; i++) {
        for (int j = 0; j < inst->nnodes; j++) {
            if (j == i) continue;
            // define right hand side
            double rhs = 1.0;
            // define the type of constraint (array) ('E' for equality)
            char sense = 'L';
            // define constraint name
            snprintf(rname[0], BUFLEN, "SEC2(%d,%d)", i + 1, j + 1);
            if((err = CPXnewrows(inst->CPXenv, inst->CPXlp, 1, &rhs, &sense, NULL, rname))) {
                printerr(inst, "[ERROR] CPXnewrows() error.");
            }
            int lastrow_idx = CPXgetnumrows(inst->CPXenv, inst->CPXlp) - 1; // constraint index starts from 0
            // change last row coefficients from 0 to 1 or -1
            err1 = CPXchgcoef(inst->CPXenv, inst->CPXlp, lastrow_idx, xpos_directed(i, j, inst), 1.0);
            err2 = CPXchgcoef(inst->CPXenv, inst->CPXlp, lastrow_idx, xpos_directed(j, i, inst), 1.0);
            if (err1 || err2) {
                printf(BOLDRED "[ERROR] Cannot change coefficient: error code %d", err);
            }
        }
    }

    free(rname[0]);
}

void build_model_base_directed(instance *inst){
    add_x_vars_directed(inst);

    add_degree_constraints_directed(inst);

    add_SEC2_constraints_directed(inst);
}


// ===== UNDIRECTED GRAPH FUNCTIONS ====

// return CPLEX column position given an arc (i,j)
int xpos_undirected(int i, int j, instance *inst) {
    if((i == j) || (j < 0) || (i < 0) || (j >= inst->nnodes) || (i >= inst->nnodes)){
        printf(BOLDRED"[ERROR] xpos_undirected(): unexpected i = %d, j = %d\n" RESET, i, j);
        free_instance(inst);
        exit(1);
    }
    // Check if it's a cell above diagonal
    if(i > j) return xpos_undirected(j, i, inst);

    // Below formula derives from
    // pos = j - 1 + i*n - sum_1^i(k+1)
    int pos = i * inst->nnodes + j - ((i + 1 ) * (i + 2 )) / 2;
    return pos;
}

void add_x_vars_undirected(instance *inst){
    char binary = 'B';

    // allocate array for variable's name with trick
    char *cname[] = {(char *) calloc(BUFLEN, sizeof(char))}; // string array required by cplex for batch insertions

    // add binary vars x(i,j) for i < j
    // one for each edge
    for(int i = 0; i < inst->nnodes; i++){
        for(int j = i+1; j < inst->nnodes; j++){
            // define variable name
            snprintf(cname[0], BUFLEN, "x(%d,%d)", i+1,j+1);
            // define its cost
            double obj = cost(i, j, inst); // cost == distance
            // define its lower bound
            double lb = 0.0;
            // define its upper bound
            double ub = 1.0;
            if(CPXnewcols(inst->CPXenv, inst->CPXlp, 1, &obj, &lb, &ub, &binary, cname)) {
                free(cname[0]);
                printerr(inst, "Cannot add columns!");
            }
            // check xpos_undirected on the fly (can be removed if I'm sure it's ok?)
            if (CPXgetnumcols(inst->CPXenv,inst->CPXlp)-1 != xpos_undirected(i, j, inst)) {
                free(cname[0]);
                printerr(inst, "xpos_undirected() got a bad index!");
            }
        }
    }
    free(cname[0]);
}

void add_degree_constraints_undirected(instance *inst){
    char *rname[] = {(char *) calloc(BUFLEN, sizeof(char))};

    int nnz = inst->nnodes - 1;
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
    for(int h = 0; h < inst->nnodes; h++){
        // build index array
        int k = 0;
        for(int i = 0; i < inst->nnodes; i++)
            if(i != h) index[k++] = xpos_undirected(h, i, inst);
        // define constraint name
        snprintf(rname[0], BUFLEN, "degree(%d)", h+1);
        if(CPXaddrows(inst->CPXenv, inst->CPXlp, 0, 1, nnz, &rhs, &sense, &izero, index, value, NULL, rname)) {
            free(rname[0]);
            printerr(inst, "Cannot add rows!");
        }
    }
    free(rname[0]);
}

void build_model_base_undirected(instance *inst){
    inst->directed = false;

    add_x_vars_undirected(inst);

    add_degree_constraints_undirected(inst);
}

void get_solution_base_undirected(instance *inst){
    // get solution from CPLEX
    int tot_cols = CPXgetnumcols(inst->CPXenv, inst->CPXlp);
    double *xstar;
    if(inst->xstar == NULL) {
        xstar = (double *) calloc(tot_cols, sizeof(double));
        if (CPXgetx(inst->CPXenv, inst->CPXlp, xstar, 0, tot_cols - 1)) {
            free(xstar);
            printerr(inst, "CPXgetx(): error retrieving xstar!");
        }
    }
    else
        xstar = inst->xstar;

    // scan adjacency matrix induced by xstar and print values
    // deal with numeric errors
    double *rxstar = (double *) calloc(tot_cols, sizeof(double));
    for(int i = 0; i < inst->nnodes; i++){
        for (int j = i+1; j < inst->nnodes; j++ ){
            int idx = xpos_undirected(i, j, inst);
            if(xstar[idx] > 0.5) {
                if(inst->verbose >= 2) printf("x(%3d,%3d) = 1\n", i + 1, j + 1);
                rxstar[idx] = 1;
            }else rxstar[idx] = 0;
        }
    }
    free(xstar);
    inst->xstar = rxstar;

}

/**
 * Find connected components.
 *
 * @param inst general instance
 * @param xstar best solution found
 * @param ncomp returned number of components
 * @param succ returned array of successors (initialized by caller)
 * @param comp returned array specifying components (initialized by caller)
 */
void findccomp(instance *inst, const double *xstar, int *ncomp, int *succ, int *comp){
    // initialize data structures
    *ncomp = 0;
    for(int i = 0; i < inst->nnodes; i++)
        succ[i] = comp[i] = -1;

    // choose a node `start` and visit its connected component
    for(int start = 0; start < inst->nnodes; start++){
        // skip visited nodes
        if(comp[start] >= 0) continue;

        // a new component is found
        (*ncomp)++;
        int curr = start;
        bool done = false;
        while(!done){ // go and visit the current component
            // assign component to node `i`
            comp[curr] = *ncomp;
            done = true;
            for(int next = 0; next < inst->nnodes; next++){
                // the edge [curr, next] is selected in xstar and j was not visited before
                if(curr != next && xstar[xpos_undirected(curr, next, inst)] > 0.5 && comp[next] == -1){
                    // set `i` successor
                    succ[curr] = next;
                    // change current node
                    curr = next;
                    done = false;
                    break;
                }
            }
        }
        // last arc to close the cycle
        succ[curr] = start;
    } // go to the next component...
}
