#include "tsp.h"

int build_model(instance inst, CPXENVptr env, CPXLPptr lp) {

}

// return CPLEX column position given the coordinates of an adjacency cell
int xpos(int i, int j, instance inst) {
    if((i == j) || (j < 1) || (i < 1)){
        printf(BOLDRED"[ERROR] xpos(): unexpected i = %d, j = %d\n" RESET, i, j);
        exit(1);
    }
    // Check if it's a cell above diagonal
    if(i > j) return xpos(j, i, inst);
    // int pos = i * inst.tot_nodes + j - (( i + 1 ) * ( i + 2 )) / 2;
    int pos = (i-1) * inst.tot_nodes + j - (i * (i + 1))/2;
    return pos;
}

int TSPOpt(instance inst){
    // open CPLEX model
    int err;
    CPXENVptr env = CPXopenCPLEX(&err);
    if(err){
        printf(BOLDRED "[ERROR] CPLEX error code: %d" RESET, err);
        exit(1);
    }
    CPXLPptr lp = CPXcreateprob(env, &err, "TSP");
    if(err){
        printf(BOLDRED "[ERROR] CPLEX error code: %d\n" RESET, err);
        exit(1);
    }

    build_model(inst, env, lp);

    // optimize!
    if((err = CPXmipopt(env, lp))){
        printf(BOLDRED "[ERROR] CPXmipopt(env, lp): error code %d\n" RESET, err);
        exit(1);
    }

    // get solution
    int tot_cols = CPXgetnumcols(env, lp);
    double *xstar = (double *) calloc(tot_cols, sizeof(double));
    if((err = CPXgetx(env, lp, xstar, 0, tot_cols-1))){
        printf(BOLDRED "[ERROR] CPXgetx(env, lp, xstar, 0, tot_cols-1): error code %d\n" RESET, err);
        exit(1);
    }

    // scan adjacency matrix and print values
    for(int i = 0; i < inst.tot_nodes; i++){
        for ( int j = i+1; j < inst.tot_nodes; j++ ){
            if (xstar[xpos(i,j,inst)] > 0.5) // deal with numeric errors
                printf("x(%3d,%3d) = 1\n", i+1,j+1);
        }
    }

    free(xstar);

    // free and close cplex model
    CPXfreeprob(env, &lp);
    CPXcloseCPLEX(&env);

}