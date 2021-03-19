#include "tsp.h"

char * TSPOpt(instance *inst){
    // performance check
    struct timeval start, stop;
    gettimeofday(&start, NULL);

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

    // choose formulation
    switch(inst->formulation) {
        case MTZ:
            inst->directed = true;
            build_model_MTZ(inst, env, lp);
            break;
        case GG:
            inst->directed = true;
            break;
        default:
            build_model(inst, env, lp);
    }

    save_model(inst, env, lp);

    // Set CPLEX parameters
    CPXsetdblparam(env, CPX_PARAM_TILIM, inst->time_limit);

    // optimize!
    if(inst->verbose >=1) printf(BOLDGREEN "[INFO] Optimization started! Wait please...\n" RESET);
    if((err = CPXmipopt(env, lp))){
        printf(BOLDRED "[ERROR] CPXmipopt(env, lp): error code %d\n" RESET, err);
        free_instance(inst);
        exit(1);
    }

    // get solution
    int tot_cols = CPXgetnumcols(env, lp);
    double *xstar = (double *) calloc(tot_cols, sizeof(double));
    if((err = CPXgetx(env, lp, xstar, 0, tot_cols-1))){
        printf(BOLDRED "[ERROR] CPXgetx(env, lp, xstar, 0, tot_cols-1): error code %d\n" RESET, err);
        exit(1);
    }

    gettimeofday(&stop, NULL);
    if(inst->verbose >=1) printf(BOLDGREEN "[INFO] Optimization finished in %ld seconds!\n" RESET,
                                 stop.tv_sec-start.tv_sec);

    // scan adjacency matrix and print values
    if(inst->verbose >=2) printf("Solution found:\n");
    // rounded solution
    char *rxstar = (char *) calloc(tot_cols, sizeof(char));
    for(int i = 0; i < inst->tot_nodes; i++){
        for ( int j = 0; j < inst->tot_nodes; j++ ){ // TODO j = i+1
            if (xstar[xpos_compact(i,j,inst)] > 0.5) { // deal with numeric errors TODO fix
                if(inst->verbose >= 2) printf("x(%3d,%3d) = 1\n", i + 1, j + 1);
                rxstar[xpos_compact(i, j, inst)] = 1; // TODO fix
            }
        }
    }

    free(xstar);

    // free and close cplex model
    CPXfreeprob(env, &lp);
    CPXcloseCPLEX(&env);
    return rxstar;
}

// write model to file
void save_model(instance *inst, CPXENVptr env, CPXLPptr lp){
    char file_name[BUFLEN];
    sprintf(file_name, "%s-model-%s.lp", inst->name[0], formulation_names[inst->formulation]);
    CPXwriteprob(env, lp, file_name, "lp");

    if(inst->verbose >=1) printf(BOLDGREEN "[INFO] Model saved to %s\n" RESET, file_name);
}