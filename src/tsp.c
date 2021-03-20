#include "tsp.h"

void TSPOpt(instance *inst){
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

    // save model to file
    save_model(inst, env, lp);

    // set CPLEX parameters
    CPXsetdblparam(env, CPX_PARAM_TILIM, inst->time_limit);

    // optimize!
    if(inst->verbose >=1) printf(BOLDGREEN "[INFO] Optimization started! Wait please...\n" RESET);
    if((err = CPXmipopt(env, lp))){
        printf(BOLDRED "[ERROR] CPXmipopt(): error code %d\n" RESET, err);
        free_instance(inst);
        exit(1);
    }

    // get solution status
    inst->status = CPXgetstat(env, lp);
    if(inst->status == CPX_STAT_INForUNBD){
        printf(BOLDRED "[ERROR] TSP cannot be unfeasible or unbounded!\n" RESET);
        free_instance(inst);
        exit(1);
    }

    gettimeofday(&stop, NULL);
    inst->time = stop.tv_sec-start.tv_sec;
    if(inst->verbose >=1) printf(BOLDGREEN "[INFO] Optimization finished in %ld seconds!\n" RESET,
                                 inst->time);

    // get solution
    get_solution(inst, env, lp);

    // free and close cplex model
    CPXfreeprob(env, &lp);
    CPXcloseCPLEX(&env);
}

// write model to file
void save_model(instance *inst, CPXENVptr env, CPXLPptr lp){
    char *file_template = "%s.%s-model.lp";
    char file_name[strlen(file_template) + strlen(inst->name[0]) +
            + strlen(formulation_names[inst->formulation]) + 1];
    sprintf(file_name, file_template, inst->name[0], formulation_names[inst->formulation]);
    CPXwriteprob(env, lp, file_name, "lp");

    if(inst->verbose >=1) printf(BOLDGREEN "[INFO] Model saved to %s\n" RESET, file_name);
}