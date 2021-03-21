//
// Miller-Tucker-Zemlin compact formulation of TSP
//

#include "MTZ_formulation.h"

int upos(int i, instance *inst){
    if(i < 0 || i >= inst->tot_nodes){
        printf(BOLDRED"[ERROR] xpos(): unexpected i = %d\n" RESET, i);
        exit(1);
    }
    int upos = inst->tot_nodes * inst->tot_nodes + i;
    return upos;
}

void add_uconsistency_vars(instance *inst, CPXENVptr env, CPXLPptr lp){
    char *cname[1];
    cname[0] = calloc(BUFLEN, sizeof(char));

    char integer = 'I';
    int err;

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
    free(cname[0]);
}

void add_uconsistency_constraints(instance *inst, CPXENVptr env, CPXLPptr lp){
    int err;
    char *rname[1];
    rname[0] = calloc(BUFLEN, sizeof(char));

    // position big M constraints
    int big_M = inst->tot_nodes - 1; // use big M trick
    for(int i = 1; i < inst->tot_nodes; i++)
        for(int j = 1; j < inst->tot_nodes; j++){
            double rhs = big_M - 1;
            char sense = 'L';
            sprintf(rname[0], "u_consistency(%d,%d)", i + 1, j + 1);
            if((err = CPXnewrows(env, lp, 1, &rhs, &sense, NULL, rname)) ){
                printf(BOLDRED "[ERROR] CPXnewrows() error code %d\n" RESET, err);
                exit(1);
            }
            int lastrow_idx = CPXgetnumrows(env, lp) - 1; // constraint index starts from 0

            int err1, err2, err3;
            // change last row coefficients
            err2 = err1 = 0;
            if(i != j) {
                err1 = CPXchgcoef(env, lp, lastrow_idx, upos(i, inst), 1.0);
                err2 = CPXchgcoef(env, lp, lastrow_idx, upos(j, inst), -1.0);
            }
            err3 = CPXchgcoef(env, lp, lastrow_idx, xpos_compact(i, j, inst), big_M);
            if (err1 || err2 || err3) {
                printf(BOLDRED "[ERROR] Cannot change coefficient\n");
            }
        }

    free(rname[0]);
}

void add_uconsistency_constraints_lazy(instance *inst, CPXENVptr env, CPXLPptr lp){
    int err;
    char *rname[1];
    rname[0] = calloc(BUFLEN, sizeof(char));
    int index[3];
    double value[3];
    int izero = 0;

    int big_M = inst->tot_nodes - 1; // use big M trick
    double rhs = big_M - 1;
    char sense = 'L';
    int nnz = 3;
    for(int i = 1; i < inst->tot_nodes; i++)
        for(int j = 1; j < inst->tot_nodes; j++){
            if(i == j) continue;
            sprintf(rname[0], "u_consistency(%d,%d)", i + 1, j + 1);
            index[0] = upos(i, inst);
            value[0] = 1;
            index[1] = upos(j, inst);
            value[1] = -1;
            index[2] = xpos_compact(i, j, inst);
            value[2] = big_M;

            if((err = CPXaddlazyconstraints(env, lp, 1, nnz, &rhs, &sense, &izero, index, value, rname)) ){
                printf(BOLDRED "[ERROR] CPXaddlazyconstraints() error code %d\n" RESET, err);
                exit(1);
            }
        }

    free(rname[0]);
}

void build_model_MTZ(instance *inst, CPXENVptr env, CPXLPptr lp) {
    build_model_base_directed(env, lp, inst);

    add_uconsistency_vars(inst, env, lp);

    if(inst->lazy)
        add_uconsistency_constraints(inst, env, lp);
    else
        add_uconsistency_constraints_lazy(inst, env, lp);
}

void get_solution_MTZ(instance *inst, CPXENVptr env, CPXLPptr lp){
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
            if (xstar[idx] > 0.5) {
                if(inst->verbose >= 2) printf("x(%3d,%3d) = 1\n", i + 1, j + 1);
                rxstar[idx] = 1;
            }
        }
    }

    inst->xstar = rxstar;

    for(int i = 0; i < inst->tot_nodes; i++){
        int idx = upos(i, inst);
        if(xstar[idx] > 0.001 && inst->verbose >= 2)
            printf("u(%3d) = %d\n", i+1, (int) round(xstar[idx]));
    }

    free(xstar);
}