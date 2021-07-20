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
#include "heuristics.h"
#include "heuristic_greedy.h"

void build_model_hfixing(instance *inst){
    build_model_cuts(inst);
    inst->ncols = CPXgetnumcols(inst->CPXenv, inst->CPXlp);
    inst->nrows = CPXgetnumrows(inst->CPXenv, inst->CPXlp);
}

void get_solution_hfixing(instance *inst){
    // xstar is already populated!
    //get_solution_cuts(inst);
}

double lin_func(double x, double m, double q){
    double y = m * x + q;
    //printf("[DEBUG] %f --> %f\n", x, y);
    return y;
}

void fix_edges(instance *inst, double m, double q){
    // if(perc < 0 || perc > 100) printerr(inst, "Cannot use perc = %d", perc);
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
            double perc = lin_func(cost(i, j, inst), m, q);
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

void find_min_max(instance *inst, double *min, double *max){
    *min = DBL_MAX; *max = DBL_MIN;
    for(int i = 0; i < inst->nnodes; i++){
        for(int j = 0; j < inst->nnodes; j++){
            double c = cost(i, j, inst);
            if(c < *min)
                *min = c;
            if(c > *max)
                *max = c;
        }
    }
}

void comp_lin_func(double x0, double x1, double y0, double y1, double *m, double *q){
    *m = (y1 - y0)/(x1 - x0);
    *q = (x0*y1 - x1*y0)/(x0 - x1);
}

void solve_hfixing(instance *inst){
    // set short time limit
    double timelim = inst->time_limit / 20;
    double zbest;
    double *xbest;

    double m, q;
    if(inst->formulation == HFIXING3 || inst->formulation == HFIXING4 || inst->formulation == HFIXING5){
        double min, max;
        find_min_max(inst, &min, &max);
        comp_lin_func(min, max, 90, 10, &m, &q);
    }else{
        m = 0; q = 90;
    }

    CPXsetintparam(inst->CPXenv, CPXPARAM_MIP_Limits_Solutions, 1); // exit after first feasible solution
    CPXsetintparam(inst->CPXenv, CPXPARAM_Emphasis_MIP,
                   (inst->formulation == HFIXING1)?CPX_MIPEMPHASIS_FEASIBILITY:CPX_MIPEMPHASIS_OPTIMALITY);

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

    if(inst->formulation == HFIXING4 || inst->formulation == HFIXING5) {
        init = false;
        free(inst->xbest);
        if(inst->formulation == HFIXING5)
            inst->cons_heuristic = GREEDYGRASP;
        greedy(inst, inst->time_limit/10);
        inst->directed = false;
        for(int i = 0; i < inst->nnodes; i++){
            for(int j = 0; j < inst->nnodes; j++){
                if(inst->xbest[xpos_directed(i,j,inst)] > 0.5)
                    xbest[xpos_undirected(i,j,inst)] = 1;
            }
        }
        free(inst->xbest);
        inst->xbest = xbest;
        xbest = (double *) calloc(inst->ncols, sizeof(double));
        CPXsetlongparam(inst->CPXenv, CPXPARAM_MIP_Limits_Solutions,2);
        //plot(inst, inst->xbest);
    }

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
            if(inst->formulation == HFIXING1)
                CPXsetdblparam(inst->CPXenv, CPXPARAM_TimeLimit, (left < timelim)?left:timelim);
            else
                CPXsetdblparam(inst->CPXenv, CPXPARAM_TimeLimit, left);

            // add local branching constraints
            fix_edges(inst, m, q);

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
                print(inst, 'W', 1, "Not enough time to find a starting solution! (error %d)", status);
                inst->zbest = inst->zstar = DBL_MAX;
                break;
            }else {
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
            // unset number of solution limit
            CPXsetlongparam(inst->CPXenv, CPXPARAM_MIP_Limits_Solutions,
                            (inst->formulation == HFIXING1)?9223372036800000000L:2);
        }
    }
    free(xbest);

    // copy solution
    inst->xstar = (double *) calloc(inst->ncols, sizeof(double)); // TODO that's not "xstar"
    memcpy(inst->xstar, inst->xbest, inst->ncols * sizeof(double));

    inst->zstar = inst->zbest;
}