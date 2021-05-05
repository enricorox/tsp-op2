#include "tsp.h"

double get_zstar_opt(instance *inst){
    if(inst->opt_tour == NULL){
        printf(BOLDRED "[ERROR] get_zstar_opt(): IllegalState: optimal tour needed!\n");
        free_instance(inst);
        exit(1);
    }

    // compute optimal known cost
    double z = 0;
    for(int i = 0; i < inst->nnodes; i++){
        int curr = inst->opt_tour[i] - 1; // nodes indexes start from 1!
        int next = inst->opt_tour[(i < inst->nnodes - 1) ? i + 1 : 0] - 1;
        z += cost(curr, next, inst);
    }
    return z;
}

void TSPOpt(instance *inst){
    int err;
    // define cplex envinroment
    inst->CPXenv = CPXopenCPLEX(&err);
    if(err) printerr(inst, "Can't create CPLEX enviroment");

    // activate cplex log file
    if(inst->verbose >= 2) {
        char log_name[BUFLEN];
        snprintf(log_name, BUFLEN, "%s.%s%s%d.cplex.log",
                 inst->name[0], formulation_names[inst->formulation], inst->lazy?".lazy.":".", inst->seed);
        CPXsetlogfilename(inst->CPXenv, log_name, "w");
    }

    // ===== CPLEX parameters =====
    // set random seed
    if(CPXsetintparam(inst->CPXenv, CPXPARAM_RandomSeed, inst->seed))
        print(inst, 'W', 1, "Error setting random seed.\n");
    // set tree memory limit
    if(CPXsetdblparam(inst->CPXenv, CPXPARAM_MIP_Limits_TreeMemory, inst->mem_limit))
        print(inst, 'W', 1, "Error setting tree memory limit.\n");
    // set time limit
    if(CPXsetdblparam(inst->CPXenv, CPXPARAM_TimeLimit, (double) inst->time_limit))
        print(inst, 'W', 1, "Error setting time limit.");
    // log on screen
    if(inst->verbose >= 3)
        CPXsetintparam(inst->CPXenv, CPX_PARAM_SCRIND, CPX_ON);

    // create empty lp problem
    inst->CPXlp = CPXcreateprob(inst->CPXenv, &err, "TSP");
    if(err) printerr(inst, "Can't create LP problem");

    // performance measure
    gettimeofday(&inst->tstart, NULL);

    // choose formulation
    switch(inst->formulation) {
        // ============== matheuristics ==============
        case SFIXING:
            inst->directed = false;
            build_model_sfixing(inst);
            solve_sfixing(inst);
            break;
        case HFIXING:
            inst->directed = false;
            break;
        // ============== directed graphs ==============
        case MTZ:
            inst->directed = true;
            build_model_MTZ(inst);
            break;
        case GG:
        case GGi:
            inst->directed = true;
            build_model_GG(inst);
            break;
        // ============== undirected graphs ==============
        case CUTS:
            inst->directed = false;
            build_model_cuts(inst);
            break;
        case BENDERS:
        default:
            inst->directed = false;
            loop_benders(inst);
    }

    // save model to file
    if(inst->verbose > 0)
        save_model(inst);

    // optimize!
    if(inst->xstar == NULL) {
        if (inst->verbose >= 1) printf(BOLDGREEN "[INFO] Optimization started! Please wait...\n" RESET);
        if (CPXmipopt(inst->CPXenv, inst->CPXlp))
            printerr(inst, "CPXmipopt() error!");
    }

    // get solution status
    if(inst->status == -1)
        inst->status = CPXgetstat(inst->CPXenv, inst->CPXlp);

    struct timeval stop;
    gettimeofday(&stop, NULL);
    inst->runtime = stop.tv_sec - inst->tstart.tv_sec;

    char *status;
    switch (inst->status) {
        case CPXMIP_OPTIMAL:
            status = "optimal";
            break;
        case CPXMIP_INFEASIBLE:
            status = BOLDRED "infeasible" RESET;
            break;
        case CPXMIP_TIME_LIM_INFEAS:
            status = "infeasible (time limit)";
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
        case CPXMIP_NODE_LIM_FEAS:
            status = "feasible (node limit reached)";
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

    if(inst->verbose >=1)
        printf(BOLDGREEN "[INFO] Optimization finished in %ld seconds!\n" RESET, inst->runtime);

    // get solution
    switch(inst->formulation){
        // ============== matheuristics ==============
        case SFIXING:
            get_solution_sfixing(inst);
            break;
        case HFIXING:
            break;
        // ============== directed graphs ==============
        case MTZ:
            get_solution_MTZ(inst);
            break;
        case GG:
        case GGi:
            get_solution_GG(inst);
            break;
        // ============== undirected graphs ==============
        case CUTS:
            get_solution_cuts(inst);
            break;
        case BENDERS:
        default:
            get_solution_Benders(inst);
    }

    // show cost
    if(inst->status < 0 && CPXgetobjval(inst->CPXenv, inst->CPXlp, &inst->zstar)) // A solution must exist here!
        printerr(inst, "Cannot get solution status");
    if(inst->verbose >= 1) {
        printf(BOLDGREEN "[INFO] Found z* = %f\n" RESET, inst->zstar);
        if(inst->opt_tour != NULL )
            printf(BOLDGREEN "[INFO] Known solution z* = %f\n" RESET, get_zstar_opt(inst));
    }
}

// write model to file
void save_model(instance *inst){
    char *file_template = "%s.%s-model.lp";
    char file_name[BUFLEN];
    snprintf(file_name, BUFLEN, file_template, inst->name[0], formulation_names[inst->formulation]);

    if(CPXwriteprob(inst->CPXenv, inst->CPXlp, file_name, "lp"))
        print(inst, 'W', 1, "Can't save LP to file");

    if(inst->verbose >=1) printf(BOLDGREEN "[INFO] Model saved to %s\n" RESET, file_name);
}