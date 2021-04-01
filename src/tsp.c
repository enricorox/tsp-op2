#include "tsp.h"

double get_zstar_opt(instance *inst){
    if(inst->opt_tour == NULL){
        printf(BOLDRED "[ERROR] get_zstar_opt(): optimal tour needed!\n");
        free_instance(inst);
        exit(1);
    }
    double z = 0;
    //save_to_tsp_file(inst);
    for(int i = 0; i < inst->tot_nodes; i++){
        int curr = inst->opt_tour[i] - 1; // nodes indexes start from 1!
        int next = inst->opt_tour[(i < inst->tot_nodes - 1) ? i + 1 : 0] - 1;
        z += cost(curr, next, inst);
    }
    return z;
}

void TSPOpt(instance *inst){
    // performance check
    struct timeval start, stop;
    gettimeofday(&start, NULL);

    int err;
    // define cplex envinroment
    CPXENVptr env = CPXopenCPLEX(&err);

    // activate cplex log file
    if(inst->verbose >= 2) {
        char log_name[BUFLEN];
        snprintf(log_name, BUFLEN, "%s.%s%s%d.cplex.log",
                 inst->name[0], formulation_names[inst->formulation], inst->lazy?".lazy.":".", inst->seed);
        CPXsetlogfilename(env, log_name, "w");
    }

    // set cplex random seed
    CPXsetintparam(env, CPX_PARAM_RANDOMSEED, inst->seed);

    // create lp problem
    CPXLPptr lp = CPXcreateprob(env, &err, "TSP");
    if(err){
        printf(BOLDRED "[ERROR] Cannot create problem: error code %d\n" RESET, err);
        free_instance(inst);
        exit(1);
    }

    // choose formulation
    switch(inst->formulation) {
        // ============== directed graphs ==============
        case MTZ:
            inst->directed = true;
            build_model_MTZ(inst, env, lp);
            break;
        case GG:
            inst->directed = true;
            build_model_GG(inst, env, lp);
            break;
        // ============== undirected graphs ==============
        default:
            build_model(inst, env, lp);
    }

    // save model to file
    if(inst->verbose > 0)
        save_model(inst, env, lp);

    // set CPLEX parameters
    CPXsetdblparam(env, CPX_PARAM_TILIM, inst->time_limit);

    // optimize!
    if(inst->verbose >=1) printf(BOLDGREEN "[INFO] Optimization started! Wait please...\n" RESET);
    if((err = CPXmipopt(env, lp))){
        printf(BOLDRED "[ERROR] CPXmipopt(): error code %d %s\n" RESET, err, (err == 1217)?"infeasible!":"");
        free_instance(inst);
        exit(1);
    }

    // get solution status
    inst->status = CPXgetstat(env, lp);
    char *status;
    switch (inst->status) {
        case CPXMIP_OPTIMAL:
            status = "optimal";
            break;
        case CPXMIP_INFEASIBLE:
            status = BOLDRED "infeasible";
            break;
        case CPXMIP_UNBOUNDED:
            status = BOLDRED "unbounded";
            break;
        case CPXMIP_OPTIMAL_TOL:
            status = "optimal (within tolerance)";
            break;
        case CPXMIP_TIME_LIM_FEAS:
            status = "feasible (time limit reached)";
            break;
        default:
            status = "unknown";
    }
    if(inst->verbose >=1) printf(BOLDGREEN "[INFO] Solution status: %s (%d)\n" RESET, status, inst->status);

    if((inst->status == CPXMIP_INFEASIBLE) || (inst->status == CPXMIP_UNBOUNDED)){
        printf(BOLDRED "[ERROR] Solution cannot be %s!\n" RESET,
               (inst->status == CPXMIP_INFEASIBLE)?"infeasible":"unbounded");
        free_instance(inst);
        exit(1);
    }

    gettimeofday(&stop, NULL);
    inst->runtime = stop.tv_sec - start.tv_sec;
    if(inst->verbose >=1)
        printf(BOLDGREEN "[INFO] Optimization finished in %ld seconds!\n" RESET, inst->runtime);

    // get solution
    switch(inst->formulation){
        // ============== directed graphs ==============
        case MTZ:
            get_solution_MTZ(inst, env, lp);
            break;
        case GG:
            get_solution_GG(inst, env, lp);
            break;
        // ============== undirected graphs ==============
        default:
            get_solution(inst, env, lp);
    }

    // show cost
    double obj;
    CPXgetobjval(env, lp, &obj); // A solution must exist here!
    if(inst->verbose >= 1) {
        printf(BOLDGREEN "[INFO] Found z* = %f\n" RESET, obj);
        if(inst->opt_tour != NULL )
            printf(BOLDGREEN "[INFO] Known solution z* = %f\n" RESET, get_zstar_opt(inst));
    }

    // free and close cplex model
    CPXfreeprob(env, &lp);
    CPXcloseCPLEX(&env);
}

// write model to file
void save_model(instance *inst, CPXENVptr env, CPXLPptr lp){
    char *file_template = "%s.%s-model.lp";
    char file_name[BUFLEN];
    snprintf(file_name, BUFLEN, file_template, inst->name[0], formulation_names[inst->formulation]);

    CPXwriteprob(env, lp, file_name, "lp");

    if(inst->verbose >=1) printf(BOLDGREEN "[INFO] Model saved to %s\n" RESET, file_name);
}