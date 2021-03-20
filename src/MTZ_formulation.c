//
// Created by enrico on 19/03/21.
//

#include "MTZ_formulation.h"

void build_model_MTZ(instance *inst, CPXENVptr env, CPXLPptr lp) {
    char binary = 'B';
    char integer = 'I';
    int err;

    // allocate array for variable's name with trick
    //char **cname = (char **) calloc(1, sizeof(char *)); // don't need to dynamically allocate 1 element
    char *cname[1]; // string array required by cplex for batch insertions
    cname[0] = (char *) calloc(BUFLEN, sizeof(char));

    // add binary vars x(i,j) for i != j
    // one for each edge without auto-loops
    for ( int i = 0; i < inst->tot_nodes; i++ ){
        for ( int j = 0; j < inst->tot_nodes; j++ ){
            // define variable name
            sprintf(cname[0], "x(%d,%d)", i+1,j+1);
            // define its cost
            double obj = dist(i,j,inst); // cost == distance
            // define its lower bound
            double lb = 0.0;
            // define its upper bound
            double ub = 1.0;
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

    // add integer vars u(i) for each node i
    for(int i = 0; i < inst->tot_nodes; i++){
        sprintf(cname[0], "u(%d)", i+1);
        double obj = 0;
        double lb = 0;
        double ub = i?(inst->tot_nodes - 2):0;
        if((err = CPXnewcols(env, lp, 1, &obj, &lb, &ub, &integer, cname))) {
            printf(BOLDRED "[ERROR] CPXnewcols(): error code %d\n" RESET, err);
            free_instance(inst); free(cname[0]);
            exit(1);
        }
        // check xpos on the fly (can be removed if I'm sure it's ok)
        if(CPXgetnumcols(env,lp)-1 != upos(i, inst)) {
            printf(BOLDRED "[ERROR] upos() got a bad index!\n" RESET);
            free_instance(inst); free(cname[0]);
            exit(1);
        }
    }

    // add the 1 degree in and out constraints
    for(char out = 0; out < 2; out++)
        for(int h = 0; h < inst->tot_nodes; h++){
            // define right hand side
            double rhs = 1.0;
            // define the type of constraint (array) ('E' for equality)
            char sense = 'E';
            // define constraint name
            sprintf(cname[0], "degree_%s(%d)", out?"out":"in", h+1);
            if ((err = CPXnewrows(env, lp, 1, &rhs, &sense, NULL, cname)) ){
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

    // position big M constraints
    int big_M = inst->tot_nodes - 1; // use big M trick
    for(int i = 1; i < inst->tot_nodes; i++)
        for(int j = 1; j < inst->tot_nodes; j++){
            double rhs = big_M - 1;
            char sense = 'L';
            sprintf(cname[0], "position(%d,%d)", i + 1, j + 1);
            if ((err = CPXnewrows(env, lp, 1, &rhs, &sense, NULL, cname)) ){
                printf(BOLDRED "[ERROR] CPXnewrows() error code %d\n" RESET, err);
                exit(1);
            }
            int lastrow_idx = CPXgetnumrows(env, lp) - 1; // constraint index starts from 0

            int err1, err2, err3;
            // change last row coefficients
            err3 = err2 = err1 = 0;
            if(i != j) {
                err1 = CPXchgcoef(env, lp, lastrow_idx, upos(i, inst), 1.0);
                err2 = CPXchgcoef(env, lp, lastrow_idx, upos(j, inst), -1.0);
            }
            err3 = CPXchgcoef(env, lp, lastrow_idx, xpos_compact(i, j, inst), big_M);
            if (err1 || err2 || err3) {
                printf(BOLDRED "[ERROR] Cannot change coefficient\n");
            }
        }

    free(cname[0]);
}

void get_solution(instance *inst, CPXENVptr env, CPXLPptr lp){
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
    // rounded solution
    double *rxstar = (double *) calloc(tot_cols, sizeof(double));
    for(int i = 0; i < inst->tot_nodes; i++){
        for ( int j = 0; j < inst->tot_nodes; j++ ){ // TODO j = i+1
            int idx = xpos_compact(i,j,inst);
            if (xstar[idx] > 0.5) { // deal with numeric errors TODO fix
                if(inst->verbose >= 2) printf("x(%3d,%3d) = 1\n", i + 1, j + 1);
                rxstar[idx] = 1; // TODO fix
            }
        }
    }

    inst->xstar = rxstar;
    free(xstar);
}