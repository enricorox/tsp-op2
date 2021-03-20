//
// Created by enrico on 19/03/21.
//

#include "standard_formulation.h"

void build_model(instance *inst, CPXENVptr env, CPXLPptr lp) {
    char binary = 'B';
    int err;

    // allocate array for variable's name with trick
    //char **cname = (char **) calloc(1, sizeof(char *)); // don't need to dynamically allocate 1 element
    char *cname[1]; // string array required by cplex for batch insertions
    cname[0] = (char *) calloc(BUFLEN, sizeof(char));

    // add binary vars x(i,j) for i < j
    // one for each edge
    for(int i = 0; i < inst->tot_nodes; i++){
        for(int j = i+1; j < inst->tot_nodes; j++){
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
            // check xpos on the fly (can be removed if I'm sure it's ok?)
            if (CPXgetnumcols(env,lp)-1 != xpos(i, j, inst)) {
                printf(BOLDRED "[ERROR] xpos() got a bad index!\n" RESET);
                free(cname[0]);
                free_instance(inst);
                exit(1);
            }
        }
    }

    // add the 2 degree constraints
    for(int h = 0; h < inst->tot_nodes; h++){
        // define right hand side
        double rhs = 2.0;
        // define the type of constraint (array) ('E' for equality)
        char sense = 'E';
        // define constraint name
        sprintf(cname[0], "degree(%d)", h+1);
        if((err = CPXnewrows(env, lp, 1, &rhs, &sense, NULL, cname))){
            printf(BOLDRED "[ERROR] CPXnewrows() error code %d\n" RESET, err);
            exit(1);
        }
        int lastrow_idx = CPXgetnumrows(env, lp) - 1; // row index starts from 0
        // change last row coefficients from 0 to 1
        for(int i = 0; i < inst->tot_nodes; i++){
            if ( i == h ) continue; // skip auto-loops
            if((err = CPXchgcoef(env, lp, lastrow_idx, xpos(i, h, inst), 1.0))) {
                printf(BOLDRED "[ERROR] Cannot change coefficient: error code %d", err);
            }
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
    if(inst->verbose >=2) printf("Solution %s found:\n", inst->status == CPX_STAT_OPTIMAL ? "optimal":"");

    // deal with numeric errors
    double *rxstar = (double *) calloc(tot_cols, sizeof(double));
    for(int i = 0; i < inst->tot_nodes; i++){
        for ( int j = i+1; j < inst->tot_nodes; j++ ){
            int idx = xpos(i,j,inst);
            if(xstar[idx] > 0.5) {
                if(inst->verbose >= 2) printf("x(%3d,%3d) = 1\n", i + 1, j + 1);
                rxstar[idx] = 1;
            }
        }
    }

    inst->xstar = rxstar;
    free(xstar);
}