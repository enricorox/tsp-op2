//
// Created by enrico on 21/06/21.
//

#include "formulation_hfixing.h"

//
// Created by enrico on 03/05/21.
//

#include <sys/time.h>
#include "formulation_sfixing.h"
#include "plot.h"

void build_model_hfixing(instance *inst){
    build_model_cuts(inst);
    inst->ncols = CPXgetnumcols(inst->CPXenv, inst->CPXlp);
    inst->nrows = CPXgetnumrows(inst->CPXenv, inst->CPXlp);
}

void get_solution_hfixing(instance *inst){
    // xstar is already populated!
    //get_solution_cuts(inst);
}

void fix_edges(instance *inst, int perc){
    if(perc < 0 || perc > 100) printerr(inst, "Cannot use perc = %d", perc);
    if(inst->xbest == NULL) printerr(inst, "xbest must be not null!");

    double *bd = calloc(inst->ncols, sizeof(double));

    int *indices = calloc(inst->ncols, sizeof(int));
    for(int i = 0; i < inst->ncols; i++) indices[i] = i;

    char *lu = calloc(inst->ncols, sizeof(char));
    for(int i = 0; i < inst->ncols; i++) lu[i] = 'L';

    // choose edges
    int counter = 0;
    for(int i = 0; i < inst->nnodes; i++)
        for(int j = i + 1; j < inst->nnodes; j++){
            int k = xpos_undirected(i, j, inst);
            if(inst->xbest[k] > 0.5) // if previously selected
                if((bd[k] = (uprob(perc) ? 1 : 0))) { // put to 1 with 90% probability
                    print(inst, 'D', 3, "Edge x(%d, %d) fixed", i, j);
                    counter++;
                }
        }
    print(inst, 'D', 1, "fixed %d edges", counter);

    // change lower bounds
    int status = CPXchgbds(inst->CPXenv, inst->CPXlp, inst->ncols, indices, lu, bd);
    if(status)
        print(inst, 'D', 1, "Change bound error %d", status);

    free(bd);
    free(indices);
    free(lu);
}

void solve_hfixing(instance *inst){
    // set short time limit
    double timelim = inst->time_limit / 20;
    double zbest;
    double *xbest;
    //CPXsetlongparam(inst->CPXenv, CPX_PARAM_NODELIM, 0L);
    CPXsetintparam(inst->CPXenv, CPXPARAM_MIP_Limits_Solutions, 1); // exit after first feasible solution
    CPXsetintparam(inst->CPXenv, CPXPARAM_Emphasis_MIP, CPX_MIPEMPHASIS_FEASIBILITY);

    print(inst, 'D', 1, "ncols = %d", inst->ncols);

    // allocate arrays and variables
    xbest = (double *) calloc(inst->ncols, sizeof(double));
    inst->xbest = (double *) calloc(inst->ncols, sizeof(double));
    struct timeval now;

    // indices for warm start
    int varindices[inst->ncols];
    for(int i = 0; i < inst->ncols; i++) varindices[i] = i;
    int beg[] = {0};

    bool init = true;
    while(true){
        // check time limit
        gettimeofday(&now, NULL);
        double elapsed = (double) (now.tv_sec - inst->tstart.tv_sec) ;
        if(elapsed >= inst->time_limit) {
            print(inst, 'D', 1, "Reached time limit: %f", elapsed);
            break;
        }
        double left = inst->time_limit - elapsed;

        if(!init) {
            // set short time limit
            CPXsetdblparam(inst->CPXenv, CPXPARAM_TimeLimit, (left < timelim)?left:timelim);

            // add local branching constraints
            fix_edges(inst, 90);

            // add warm start
            CPXaddmipstarts(inst->CPXenv, inst->CPXlp, 1, inst->ncols, beg, varindices, inst->xbest,
                            CPX_MIPSTART_AUTO, NULL);
        }


        // save model
        // save_model(inst);

        // solve until (short) time limit or node limit expires
        CPXmipopt(inst->CPXenv, inst->CPXlp);

        // get solution
        int status = CPXgetx(inst->CPXenv, inst->CPXlp, xbest, 0, inst->ncols - 1);
        if(status != 0) {
            if(init) {
                print(inst, 'W', 1, "Writing last LP model...");
                save_model(inst);
                printerr(inst, "Not enough time to find a starting solution! (error %d)", status);
            }
            else {
                print(inst, 'W', 1, "Not enough time to find an incumbent solution! Increasing individual time-limit");
                timelim +=  0.5 * timelim;
                CPXdelrows(inst->CPXenv, inst->CPXlp, inst->nrows, inst->nrows);
                continue;
            }
        }
        CPXgetobjval(inst->CPXenv, inst->CPXlp, &zbest);
        if(zbest < inst->zbest){
            print(inst, 'D', 1, "Found better solution value zbest = %f", zbest);
            inst->zbest = zbest;
            inst->status = CPXgetstat(inst->CPXenv, inst->CPXlp);

            // swap array pointers
            double *temp = xbest;
            xbest = inst->xbest;
            inst->xbest = temp;
        }
        // plot
        //plot(inst, inst->xbest);

        if(init){
            // unset initialization
            init = false;

            // unset node limit
            //CPXsetlongparam(inst->CPXenv, CPX_PARAM_NODELIM, 9223372036800000000L);
            // unset number of solution limit
            CPXsetlongparam(inst->CPXenv, CPXPARAM_MIP_Limits_Solutions, 9223372036800000000L);
        }
    }
    free(xbest);

    // copy solution
    inst->xstar = (double *) calloc(inst->ncols, sizeof(double)); // TODO that's not "xstar"
    memcpy(inst->xstar, inst->xbest, inst->ncols * sizeof(double));

    inst->zstar = inst->zbest;
}