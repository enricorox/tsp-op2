//
// Created by enrico on 20/03/21.
//

#include "GG_formulation.h"

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
            sprintf(cname[0], "y(%d,%d)", i + 1, j + 1);
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
    int err;
    char *cname[1];
    cname[0] = (char *) calloc(BUFLEN, sizeof(char));

    for(int h = 1; h < inst->tot_nodes; h++){
        double rhs = 1;
        char sense = 'E';
        sprintf(cname[0], "flow(%d)", h + 1);
        if((err = CPXnewrows(env, lp, 1, &rhs, &sense, NULL, cname)) ){
            printf(BOLDRED "[ERROR] CPXnewrows() error code %d\n" RESET, err);
            exit(1);
        }
        int lastrow_idx = CPXgetnumrows(env, lp) - 1; // constraint index starts from 0

        int err1, err2;
        // change last row coefficients
        for(int i = 0; i < inst->tot_nodes; i++) {
            if(h == i) continue; // coefficients remains 0
            err1 = CPXchgcoef(env, lp, lastrow_idx, ypos(i, h, inst), 1.0);
            err2 = CPXchgcoef(env, lp, lastrow_idx, ypos(h, i, inst), -1.0);
        }
        if (err1 || err2) {
            printf(BOLDRED "[ERROR] Cannot change coefficient\n");
        }
    }

    // flow for 1: y_1j = (n-1)x_1j
    for(int j = 1; j < inst->tot_nodes; j++){
        double rhs = 0;
        char sense = 'E';
        sprintf(cname[0], "flow_one(%d)", j + 1);
        if((err = CPXnewrows(env, lp, 1, &rhs, &sense, NULL, cname)) ){
            printf(BOLDRED "[ERROR] CPXnewrows() error code %d\n" RESET, err);
            exit(1);
        }
        int lastrow_idx = CPXgetnumrows(env, lp) - 1; // constraint index starts from 0

        int err1, err2;
        // change last row coefficients
        err1 = CPXchgcoef(env, lp, lastrow_idx, ypos(0, j, inst), 1.0);
        err2 = CPXchgcoef(env, lp, lastrow_idx, xpos_compact(0, j, inst), -inst->tot_nodes + 1);

        if (err1 || err2) {
            printf(BOLDRED "[ERROR] Cannot change coefficient\n");
        }
    }

    free(cname[0]);
}

void add_link_constraints(instance *inst, CPXENVptr env, CPXLPptr lp){
    int err;
    char *cname[1];
    cname[0] = (char *) calloc(BUFLEN, sizeof(char));

    // linking constraints: y_ij <= (n-2)x_ij, i!=1!=j
    for(int i = 1; i < inst->tot_nodes; i++){
        for(int j = 1; j < inst->tot_nodes; j++){
            double rhs = 0;
            char sense = 'L';
            sprintf(cname[0], "link(%d,%d)", i + 1, j + 1);
            if((err = CPXnewrows(env, lp, 1, &rhs, &sense, NULL, cname)) ){
                printf(BOLDRED "[ERROR] CPXnewrows() error code %d\n" RESET, err);
                exit(1);
            }
            int lastrow_idx = CPXgetnumrows(env, lp) - 1; // constraint index starts from 0

            int err1, err2;
            // change last row coefficients
            err1 = CPXchgcoef(env, lp, lastrow_idx, ypos(i, j, inst), 1.0);
            err2 = CPXchgcoef(env, lp, lastrow_idx, xpos_compact(i, j, inst), -inst->tot_nodes + 2);

            if (err1 || err2) {
                printf(BOLDRED "[ERROR] Cannot change coefficient\n");
            }
        }
    }

    free(cname[0]);
}

void build_model_GG(instance *inst, CPXENVptr env, CPXLPptr lp){
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
