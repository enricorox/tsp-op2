//
// Created by enrico on 19/03/21.
//

#include "formulation_Benders.h"

void build_model_Benders(instance *inst, CPXENVptr env, CPXLPptr lp) {
    build_model_base_undirected(env, lp, inst);

    // TODO set CPX_PARAM_OBJLLIM lower bound before adding new constraints!
}

void get_solution(instance *inst, CPXENVptr env, CPXLPptr lp){
    // get solution from CPLEX
    int tot_cols = CPXgetnumcols(env, lp);
    double *xstar = (double *) calloc(tot_cols, sizeof(double));
    if (CPXgetx(env, lp, xstar, 0, tot_cols - 1)) {
        free(xstar);
        printerr(inst, "CPXgetx(): error retrieving xstar!");
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