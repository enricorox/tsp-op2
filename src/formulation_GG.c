//
// Gravish and Graves compact formulation of TSP
//

#include "formulation_GG.h"

int ypos(int i, int j, instance *inst){
    if((j < 0) || (i < 0) || (j >= inst->tot_nodes) || (i >= inst->tot_nodes)){
        printf(BOLDRED"[ERROR] ypos(): unexpected i = %d, j = %d\n" RESET, i, j);
        free_instance(inst);
        exit(1);
    }
    int pos = inst->tot_nodes * inst->tot_nodes + i * inst->tot_nodes + j;
    return pos;
}

void add_flow_vars(instance *inst, CPXENVptr env, CPXLPptr lp){
    char *cname[1];
    cname[0] = (char *) calloc(BUFLEN, sizeof(char));

    char integer = 'I';
    int err;

    // add y flow variables
    for(int i = 0; i < inst->tot_nodes; i++){
        for(int j = 0; j < inst->tot_nodes; j++) {
            snprintf(cname[0], BUFLEN, "y(%d,%d)", i + 1, j + 1);
            //printf("y(%d,%d) = %s\n", i+1, j+1, cname[0]);
            double obj = 0;
            double lb = 0;
            double ub = ((i == j) || (j == 0)) ? 0 : inst->tot_nodes - 1; // was 2
            if ((err = CPXnewcols(env, lp, 1, &obj, &lb, &ub, &integer, cname))) {
                printf(BOLDRED "[ERROR] CPXnewcols(): error code %d\n" RESET, err);
                free_instance(inst);
                free(cname[0]);
                exit(1);
            }
            // check ypos on the fly
            if (CPXgetnumcols(env, lp) - 1 != ypos(i, j, inst)) {
                printf(BOLDRED "[ERROR] ypos() got a bad index!\n" RESET);
                free_instance(inst);
                free(cname[0]);
                exit(1);
            }
        }
    }
    free(cname[0]);
}

void add_flow_constraints(instance *inst, CPXENVptr env, CPXLPptr lp){
    char *rname[] = {(char *) calloc(BUFLEN, sizeof(char))};

    // add flow constraints in(h) = out(h) +1 for h!=1
    int nnz = 2 * inst->tot_nodes;
    int index[nnz];
    double value[nnz];
    double rhs = 1;
    char sense = 'E';
    int izero = 0;
    for(int h = 1; h < inst->tot_nodes; h++){
        sprintf(rname[0], "flow(%d)", h + 1);
        // build index value array
        int idx = 0;
        for(int i = 0; i < inst->tot_nodes; i++) {
            index[idx] = ypos(i, h, inst);
            value[idx] = (h!=i)?1:0;
            index[inst->tot_nodes + idx] = ypos(h, i, inst);
            value[inst->tot_nodes + idx] = (h!=i)?-1:0;
            idx++;
        }
        if(inst->lazy) {
            if (CPXaddlazyconstraints(env, lp, 1, nnz, &rhs, &sense, &izero, index, value, rname)) {
                printf(BOLDRED "[ERROR] CPXaddlazyconstraints() error!\n");
                free_instance(inst);
                exit(1);
            }
        }else
            if (CPXaddrows(env, lp, 0, 1, nnz, &rhs, &sense, &izero, index, value, NULL, rname)) {
                printf(BOLDRED "[ERROR] CPXaddrows() error!\n");
                free_instance(inst);
                exit(1);
            }

    }

    // flow for 1: y_1j = (n-1)x_1j
    rhs = 0;
    sense = 'E';
    nnz = 2; // can reuse previous arrays!
    for(int j = 1; j < inst->tot_nodes; j++){
        sprintf(rname[0], "flow_one(%d)", j + 1);
        index[0] = ypos(0, j, inst);
        value[0] = 1;
        index[1] = xpos_compact(0, j, inst);
        value[1] = -inst->tot_nodes + 1;
        if(CPXaddlazyconstraints(env, lp, 1, nnz, &rhs, &sense, &izero, index, value, rname)){
            printf(BOLDRED "[ERROR] CPXaddlazyconstraints() error!\n" RESET);
            exit(1);
        }
    }

    free(rname[0]);
}

void add_link_constraints(instance *inst, CPXENVptr env, CPXLPptr lp){
    int err;
    char *rname[] = {(char *) calloc(BUFLEN, sizeof(char))};

    int nnz = 2;
    int index[nnz];
    double value[nnz];
    double rhs = 0;
    char sense = 'L';
    int izero = 0;
    // linking constraints: y_ij <= (n-2)x_ij, i!=1!=j
    for(int i = 1; i < inst->tot_nodes; i++){
        for(int j = 1; j < inst->tot_nodes; j++) {
            index[0] = ypos(i, j, inst);
            value[0] = 1;
            index[1] = xpos_compact(i, j, inst);
            value[1] = -inst->tot_nodes + 2;
            snprintf(rname[0], BUFLEN, "link(%d,%d)", i + 1, j + 1);
            if(inst->lazy){
                if ((err = CPXaddlazyconstraints(env, lp, 1, nnz, &rhs, &sense, &izero, index, value, rname))) {
                    printf(BOLDRED "[ERROR] CPXaddlazyconstraints() error code %d\n" RESET, err);
                    exit(1);
                }
            }else
                if((err = CPXaddrows(env, lp, 0, 1, nnz, &rhs, &sense, &izero, index, value, NULL, rname)) ){
                printf(BOLDRED "[ERROR] CPXaddrows() error code %d\n" RESET, err);
                exit(1);
                }
        }
    }

    free(rname[0]);
}

void build_model_GG(instance *inst, CPXENVptr env, CPXLPptr lp) {
    build_model_base_directed(env, lp, inst);

    add_flow_vars(inst, env, lp);

    add_flow_constraints(inst, env, lp);

    add_link_constraints(inst, env, lp);
}

void get_solution_GG(instance *inst, CPXENVptr env, CPXLPptr lp){
    // get solution from CPLEX
    int tot_cols = CPXgetnumcols(env, lp);
    double *xstar = (double *) calloc(tot_cols, sizeof(double));
    if (CPXgetx(env, lp, xstar, 0, tot_cols - 1)) {
        printf(BOLDRED "[ERROR] CPXgetx(): error retrieving xstar!\n" RESET);
        free(xstar);
        free_instance(inst);
        exit(1);
    }

    // scan adjacency matrix induced by xstar and print values
    if(inst->verbose >=2) printf("Solution found:\n");
    // deal with numeric errors
    double *rxstar = (double *) calloc(tot_cols, sizeof(double));
    for(int i = 0; i < inst->tot_nodes; i++){
        for ( int j = 0; j < inst->tot_nodes; j++ ){
            int idx = xpos_compact(i,j,inst);
            if(xstar[idx] > 0.5) {
                if(inst->verbose >= 2) printf("x(%3d,%3d) = 1\n", i + 1, j + 1);
                rxstar[idx] = 1;
            }
        }
    }

    inst->xstar = rxstar;

    for(int i = 0; i < inst->tot_nodes; i++){
        for(int j = 0; j < inst->tot_nodes; j++) {
            int idx = ypos(i, j, inst);
            if(xstar[idx] > 0.001 && inst->verbose >= 2)
                printf("y(%3d,%3d) = %d\n", i+1, j+1, (int) round(xstar[idx]));
        }
    }

    free(xstar);
}
