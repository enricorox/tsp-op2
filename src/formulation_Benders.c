//
// Created by enrico on 19/03/21.
//

#include <unistd.h>
#include "formulation_Benders.h"
#include "plot.h"

/**
 * Update constraints.
 * Use subtours elimination on connected components.
 *
 * @param inst general instance
 * @param env CPLEX environment
 * @param lp CPLEX linear problem
 * @param ncomp number of connected components
 * @param comp array specifying components
 * @return a new problem with updated constraints
 */
void updconstr(instance *inst, int ncomp, const int *comp, int it){
    if(ncomp <= 1) printerr(inst, "Illegal state: must be ncomp > 1");

/*
    // clone problem
    int status;
    CPXLPptr lp_clone = CPXcloneprob(inst->CPXenv, inst->CPXlp, &status);
    if(status) printerr(inst, "Can't clone problem!");
    // free old problem
    if(CPXfreeprob(inst->CPXenv, &inst->CPXlp)) printerr(inst, "Can't free problem!");

    inst->CPXlp = lp_clone;
*/
    int tot_cols = CPXgetnumcols(inst->CPXenv, inst->CPXlp);
    char *rname[] = {(char *) malloc(BUFLEN)};
    double *value = (double *) malloc(tot_cols * sizeof(double));
    int *index = (int *) malloc(tot_cols * sizeof(int));
    // initialize value vector that doesn't change!
    for(int i = 0; i < tot_cols; i++) value[i] = 1;
    char sense = 'L';
    int izero = 0;

    for(int curr_comp = 1; curr_comp <= ncomp; curr_comp++){
        int csize = 0; // component size
        int nedges = 0; // all possible edges in the component

        for(int i = 0; i < inst->nnodes; i++){
            if(comp[i] != curr_comp) continue;
            csize++;
            for(int j = i + 1; j < inst->nnodes; j++){
                if(comp[j] != curr_comp) continue;

                // here `i` and `j` must belong to `curr_comp`
                if(nedges >= tot_cols) printerr(inst, "Illegal state: must be tot_edges < tot_cols!");
                // save index
                index[nedges++] = xpos(i, j, inst);
            }
        }
        double rhs = csize - 1;
        int nnz = nedges;
        snprintf(rname[0], BUFLEN, "SEC(%d,%d,%d)", it, curr_comp, csize);
        if(CPXaddrows(inst->CPXenv, inst->CPXlp, 0, 1, nnz, &rhs, &sense, &izero, index, value, NULL, rname))
            printerr(inst, "Can't add rows!");
        if(inst->verbose >= 3)
            printf("Added constraints for component %d of size %d\n", curr_comp, csize);
    }
    free(rname[0]);
    free(index);
    free(value);
}

/**
 * Benders loop method.
 * Solve iteratively TSP generating on the fly SECs.
 * Use directed graphs.
 *
 * @param inst general instance
 * @param env CPLEX environment
 * @param lp CPLEX linear problem
 */
void loop_benders(instance *inst) {
    build_model_base_undirected(inst);

    int tot_cols = CPXgetnumcols(inst->CPXenv, inst->CPXlp);
    int ncomp = inst->nnodes;
    double *xstar = (double *) malloc(tot_cols * sizeof(double));
    int *succ = (int *) malloc(inst->nnodes * sizeof(int));
    int *comp = (int *) malloc(inst->nnodes * sizeof(int));

    print(inst, 'I', 1, "Optimization started! Please wait...");

    struct timeval ctime;
    long eltime;

    int it = 0;
    while(true) {
        it++;
        if(inst->verbose >= 2) printf(BOLDGREEN "[Benders] Iteration #%d...\n" RESET, it);
        CPXmipopt(inst->CPXenv, inst->CPXlp);

        // get xstar
        if(CPXgetx(inst->CPXenv, inst->CPXlp, xstar, 0, tot_cols - 1)) {
            printerr(inst, "CPXgetx(): error retrieving xstar!");
        }
        inst->xstar = xstar;

        // find connected components
        findccomp(inst, inst->xstar, &ncomp, succ, comp);

        if(inst->verbose >= 2)
            printf(BOLDGREEN "[Benders] Found %d connected components\n" RESET, ncomp);

        if(ncomp == 1)
            break;

        // set new lower bound
        double zstar;
        CPXgetobjval(inst->CPXenv, inst->CPXlp, &zstar);
        print(inst, 'D', 2, "Found z* = %f", zstar);
        CPXsetdblparam(inst->CPXenv, CPX_PARAM_OBJLLIM, zstar);

        // check and update time limit
        gettimeofday(&ctime, NULL);
        eltime = ctime.tv_sec - inst->tstart.tv_sec;
        if((double) eltime >= inst->time_limit) {
            print(inst, 'W', 1, "Time limit reached!");
            inst->status = CPXMIP_TIME_LIM_INFEAS;
            break;
        }
        print(inst, 'D', 3, "New time limit: %ld", inst->time_limit - (double) eltime);
        if(CPXsetdblparam(inst->CPXenv, CPXPARAM_TimeLimit, (double) (inst->time_limit - (double) eltime)))
            print(inst, 'W', 1, "Error setting time limit.");

        // add sec for connected components
        updconstr(inst, ncomp, comp, it);
    }

    free(succ);
    free(comp);
}

void get_solution_Benders(instance *inst){
    get_solution_base_undirected(inst);
}