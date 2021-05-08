//
// Gravish and Graves compact formulation of TSP
//

#include "formulation_GG.h"

/**
 * y position.
 * N.B. Need to add edge binary variable first!
 *
 * @param i start point
 * @param j end point
 * @param inst general instance
 * @return flow variable column index on CPLEX LP
 */
int ypos(int i, int j, instance *inst){
    if((j < 0) || (i < 0) || (j >= inst->nnodes) || (i >= inst->nnodes)){
        printf(BOLDRED"[ERROR] ypos(): unexpected i = %d, j = %d\n" RESET, i, j);
        free_instance(inst);
        exit(1);
    }
    int pos = inst->nnodes * inst->nnodes + i * inst->nnodes + j;
    return pos;
}

/**
 * Add flow variables.
 * N.B. Need to add edge binary variables first!
 *
 * @param inst general instance
 */
void add_flow_vars(instance *inst){
    char *cname[] = {(char *) malloc(BUFLEN)};

    char integer = 'I';

    // add y flow variables
    for(int i = 0; i < inst->nnodes; i++){
        for(int j = 0; j < inst->nnodes; j++) {
            snprintf(cname[0], BUFLEN, "y(%d,%d)", i + 1, j + 1);
            //printf("y(%d,%d) = %s\n", i+1, j+1, cname[0]);
            double obj = 0;
            double lb = 0;
            double ub = ((i == j) || (j == 0)) ? 0 : inst->nnodes - 1; // was 2
            if (CPXnewcols(inst->CPXenv, inst->CPXlp, 1, &obj, &lb, &ub, &integer, cname)) {
                free(cname[0]);
                printerr(inst, "CPXnewcols() error.");
            }
            // check ypos on the fly
            if (CPXgetnumcols(inst->CPXenv, inst->CPXlp) - 1 != ypos(i, j, inst)) {
                free(cname[0]);
                printerr(inst, "ypos() got a bad index!");
            }
        }
    }
    free(cname[0]);
}

/**
 * Add flow constraints
 *
 * @param inst general instance
 */
void add_flow_constraints(instance *inst){
    char *rname[] = {(char *) malloc(BUFLEN)};

    // add flow constraints in(h) = out(h) +1 for h!=1
    int nnz = 2 * inst->nnodes;
    int index[nnz];
    double value[nnz];
    double rhs = 1;
    char sense = 'E';
    int izero = 0;
    for(int h = 1; h < inst->nnodes; h++){
        snprintf(rname[0], BUFLEN, "flow(%d)", h + 1);
        // build index value array
        int idx = 0;
        for(int i = 0; i < inst->nnodes; i++) {
            index[idx] = ypos(i, h, inst);
            value[idx] = (h!=i)?1:0;
            index[inst->nnodes + idx] = ypos(h, i, inst);
            value[inst->nnodes + idx] = (h != i) ? -1 : 0;
            idx++;
        }
        if(inst->lazy) {
            if(CPXaddlazyconstraints(inst->CPXenv, inst->CPXlp, 1, nnz, &rhs, &sense, &izero, index, value, rname)) {
                free(rname[0]);
                printerr(inst, "CPXaddlazyconstraints() error.");
            }
        }else
            if (CPXaddrows(inst->CPXenv, inst->CPXlp, 0, 1, nnz, &rhs, &sense, &izero, index, value, NULL, rname)) {
                free(rname[0]);
                printerr(inst, "CPXaddrows() error!");
            }

    }

    free(rname[0]);
}

/**
 * Add linking constraints.
 *
 * N.B. Need to add edge binary variables first!
 * @param inst general instance
 * @param env CPLEX envinronment
 * @param lp CPLEX LP
 */
void add_linking_constraints(instance *inst){
    char *rname[] = {(char *) calloc(BUFLEN, sizeof(char))};

    int nnz = 2;
    int index[nnz];
    double value[nnz];
    double rhs = 0;
    char sense = 'L';
    int izero = 0;
    // linking constraints: y_ij <= (n-2) * x_ij, for each i != 1 != j
    for(int i = 1; i < inst->nnodes; i++){
        for(int j = 1; j < inst->nnodes; j++) {
            index[0] = ypos(i, j, inst);
            value[0] = 1;
            index[1] = xpos_directed(i, j, inst);
            value[1] = -inst->nnodes + 2;
            snprintf(rname[0], BUFLEN, "link(%d,%d)", i + 1, j + 1);
            if(inst->lazy){
                if(CPXaddlazyconstraints(inst->CPXenv, inst->CPXlp, 1, nnz, &rhs, &sense, &izero, index, value, rname)) {
                    free(rname[0]);
                    printerr(inst, "CPXaddlazyconstraints() error!");
                }
            }else
                if(CPXaddrows(inst->CPXenv, inst->CPXlp, 0, 1, nnz, &rhs, &sense, &izero, index, value, NULL, rname)){
                    free(rname[0]);
                    printerr(inst, "CPXaddrows() error!");
                }
        }
    }

    // linking for 1: y_1j = (<=) (n-1)x_1j
    rhs = 0;
    sense = 'L';
    if(inst->formulation == GGi) sense = 'E';
    nnz = 2; // we can reuse previous arrays!
    for(int j = 1; j < inst->nnodes; j++){
        snprintf(rname[0], BUFLEN, "link(1, %d)", j + 1);
        index[0] = ypos(0, j, inst);
        value[0] = 1;
        index[1] = xpos_directed(0, j, inst);
        value[1] = -inst->nnodes + 1;
        if(inst->lazy) {
            if(CPXaddlazyconstraints(inst->CPXenv, inst->CPXlp, 1, nnz, &rhs, &sense, &izero, index, value, rname)) {
                free(rname[0]);
                printerr(inst, "CPXaddlazyconstraints() error!");
            }
        }else
        if(CPXaddrows(inst->CPXenv, inst->CPXlp, 0, 1, nnz, &rhs, &sense, &izero, index, value, NULL, rname)) {
            free(rname[0]);
            printerr(inst, "CPXaddrows() error!");
        }
    }
    free(rname[0]);
}

/**
 * Build Gravish and Graves TSP (improved) model on CPLEX
 *
 * @param inst general instance
 * @param env CPLEX envinroment
 * @param lp CPLEX LP
 */
void build_model_GG(instance *inst) {
    build_model_base_directed(inst);

    add_flow_vars(inst);

    add_flow_constraints(inst);

    add_linking_constraints(inst);
}

/**
 * Get (and print) solution
 * @param inst general instance
 * @param env CPLEX environment
 * @param lp CPLEX LP
 */
void get_solution_GG(instance *inst){
    // get solution from CPLEX
    int tot_cols = CPXgetnumcols(inst->CPXenv, inst->CPXlp);
    double *xstar = (double *) calloc(tot_cols, sizeof(double));
    if (CPXgetx(inst->CPXenv, inst->CPXlp, xstar, 0, tot_cols - 1)) {
        printf(BOLDRED "[ERROR] CPXgetx(): error retrieving xstar!\n" RESET);
        free(xstar);
        free_instance(inst);
        exit(1);
    }

    // scan adjacency matrix induced by xstar and print values
    if(inst->verbose >=2) printf("Solution found:\n");
    // deal with numeric errors
    double *rxstar = (double *) calloc(tot_cols, sizeof(double));
    for(int i = 0; i < inst->nnodes; i++){
        for (int j = 0; j < inst->nnodes; j++ ){
            int idx = xpos_directed(i, j, inst);
            if(xstar[idx] > 0.5) {
                if(inst->verbose >= 2) printf("x(%3d,%3d) = 1\n", i + 1, j + 1);
                rxstar[idx] = 1;
            }
        }
    }

    inst->xstar = rxstar;

    for(int i = 0; i < inst->nnodes; i++){
        for(int j = 0; j < inst->nnodes; j++) {
            int idx = ypos(i, j, inst);
            if(xstar[idx] > 0.001 && inst->verbose >= 2)
                printf("y(%3d,%3d) = %d\n", i+1, j+1, (int) round(xstar[idx]));
        }
    }

    free(xstar);
}
