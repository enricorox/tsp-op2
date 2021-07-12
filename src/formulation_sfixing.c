//
// Created by enrico on 03/05/21.
//

#include <sys/time.h>
#include "formulation_sfixing.h"
#include "plot.h"

void build_model_sfixing(instance *inst){
    build_model_cuts(inst);
    inst->ncols = CPXgetnumcols(inst->CPXenv, inst->CPXlp);
    inst->nrows = CPXgetnumrows(inst->CPXenv, inst->CPXlp);
}

void get_solution_sfixing(instance *inst){
    // xstar is already populated!
    //get_solution_cuts(inst);
}

// add sum_{e:x_e^h = 1}(x_e) >= n - k
void addcnstr(instance *inst, int k){
    if(k < 2 || k > inst->nnodes) printerr(inst, "Cannot use k = %d", k);
    if(inst->xbest == NULL) printerr(inst, "xbest must be not null!");

    char *rname[] = {(char *) calloc(BUFLEN, sizeof(char))};

    int nnz = inst->nnodes;
    int index[nnz];
    double value[nnz];
    for(int i = 0; i < nnz; i++) value[i] = 1;

    // build left hand side
    int j = 0;
    for(int i = 0; i < inst->ncols; i++)
        if(inst->xbest[i] > 0.5) index[j++] = i;
    if(j != nnz) {
        print(inst, 'D', 0, "ncols = %d, nnz = %d, j = %d", inst->ncols, nnz, j);

        for(int i = 0; i < inst->ncols; i++)
            printf("x%d = %1.1f%s", i, inst->xbest[i], ((i+1)%10)?"\t":"\n");
        printf("\n");

        printerr(inst, "Ooops! That should not happen: j != nnz");
    }
    // define right hand side
    double rhs = inst->nnodes - k;
    // define the type of constraint (array) ('E' for equality)
    char sense = 'G';
    // define starting index
    int izero = 0;
    // define constraint name
    snprintf(rname[0], BUFLEN, "local_branching");
    if(CPXaddrows(inst->CPXenv, inst->CPXlp, 0, 1, nnz, &rhs, &sense, &izero, index, value, NULL, rname)) {
        free(rname[0]);
        printerr(inst, "Cannot add rows!");
    }

    free(rname[0]);
}

void solve_sfixing(instance *inst){
    // set short time limit
    double timelim = inst->time_limit / 20;
    double zbest;
    double *xbest;

    // local branching parameters
    int min_k = (int) (0.1 * inst->nnodes);
    if(min_k < 2) min_k = 2;
    int max_k = 2 * min_k;

    CPXsetintparam(inst->CPXenv, CPXPARAM_MIP_Limits_Solutions, 1);
    //CPXsetintparam(inst->CPXenv, CPXPARAM_Emphasis_MIP, CPX_MIPEMPHASIS_FEASIBILITY);
    CPXsetintparam(inst->CPXenv, CPXPARAM_Emphasis_MIP, CPX_MIPEMPHASIS_OPTIMALITY);

    // allocate arrays and variables
    xbest = (double *) calloc(inst->ncols, sizeof(double));
    inst->xbest = (double *) calloc(inst->ncols, sizeof(double));
    struct timeval now;

    // indices for warm start
    int varindices[inst->ncols];
    for(int i = 0; i < inst->ncols; i++) varindices[i] = i;
    int beg[] = {0};

    // local branching parameter
    int k = min_k;//2;
    // need initialization on first iteration
    bool init = true;
    // max number of integer solution per sub-problem
    long nsol = 1;

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
            //CPXsetdblparam(inst->CPXenv, CPXPARAM_TimeLimit, (left < timelim)? left:timelim);

            // changed approach
            CPXsetdblparam(inst->CPXenv, CPXPARAM_TimeLimit, left);
            CPXsetlongparam(inst->CPXenv, CPXPARAM_MIP_Limits_Solutions, nsol);

            // add local branching constraints
            addcnstr(inst, k);

            // add warm start
            CPXaddmipstarts(inst->CPXenv, inst->CPXlp, 1, inst->ncols, beg, varindices, inst->xbest,
                            CPX_MIPSTART_AUTO, NULL);
        }


        // save model
        save_model(inst);

        // solve until (short) time limit or node limit expires
        CPXmipopt(inst->CPXenv, inst->CPXlp);

        // get solution
        int status = CPXgetx(inst->CPXenv, inst->CPXlp, xbest, 0, inst->ncols - 1);
        if(status) {
            if(init)
                printerr(inst, "Not enough time to find a starting solution! (code %d)", status);
            else {
                timelim +=  0.75 * timelim;
                print(inst, 'W', 1, "Not enough time to find an incumbent solution! (code %d)\n\tIncreasing individual time-limit to %f", status, timelim);
                if(CPXdelrows(inst->CPXenv, inst->CPXlp, inst->nrows, inst->nrows))
                    printerr(inst, "Cannot delete row %d", inst->nrows);
                else
                    print(inst, 'D', 1, "Deleted row %d", inst->nrows);
                continue;
            }
        }
        CPXgetobjval(inst->CPXenv, inst->CPXlp, &zbest);
        if(zbest < inst->zbest){
            print(inst, 'D', 1, "Found better solution value zbest = %f", zbest);
            inst->zbest = zbest;
            inst->status = CPXgetstat(inst->CPXenv, inst->CPXlp);
            k = min_k;

            // swap array pointers
            double *temp = xbest;
            xbest = inst->xbest;
            inst->xbest = temp;
        }else {
            k++;
            nsol++;
            print(inst, 'D', 1, "Increased k = %d", k);
            if(k > max_k) {
                // go back instead of wasting time!
                k = min_k;

                //print(inst, 'W', 1, "k exceeded limit");
                //break;
            }
        }
        // plot
        //plot(inst, inst->xbest);

        if(!init) { // remove last constraint
            if (CPXdelrows(inst->CPXenv, inst->CPXlp, inst->nrows, inst->nrows))
                printerr(inst, "Cannot delete row %d", inst->nrows);
            else
                print(inst, 'D', 1, "Deleted row %d", inst->nrows);
        }else {
            // unset initialization
            init = false;
            nsol = 2;

            // unset node limit
            //CPXsetlongparam(inst->CPXenv, CPX_PARAM_NODELIM, 9223372036800000000L);
            //CPXsetlongparam(inst->CPXenv, CPXPARAM_MIP_Limits_Solutions, 9223372036800000000L);
        }
    }
    free(xbest);

    // copy solution
    inst->xstar = (double *) calloc(inst->ncols, sizeof(double)); // TODO that's not "xstar"
    memcpy(inst->xstar, inst->xbest, inst->ncols * sizeof(double));

    inst->zstar = inst->zbest;
}