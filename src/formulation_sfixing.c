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
void addconstr(instance *inst, int k){
    if(k < 0 || k > inst->nnodes) printerr(inst, "Cannot use k = %d", k);
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

    print(inst, 'D', 2, "j = %d", j);

    // define right hand side
    double rhs = inst->nnodes - k;
    // define the type of constraint (array) ('E' for equality)
    char sense = 'L';
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
    CPXsetlongparam(inst->CPXenv, CPX_PARAM_NODELIM, 0L);

    xbest = (double *) calloc(inst->ncols, sizeof(double));
    inst->xbest = (double *) calloc(inst->ncols, sizeof(double));
    struct timeval now;

    int k = 5;
    bool init = true;
    while(true){
        // check time limit
        gettimeofday(&now, NULL);
        double elapsed = (double) (now.tv_sec - inst->tstart.tv_sec) + timelim;
        if(elapsed >= inst->time_limit) {
            print(inst, 'D', 1, "Reached time limit: %f", elapsed);
            break;
        }

        // add local branching constraints
        if(!init) addconstr(inst, k);

        // save model
        save_model(inst);

        // solve until (short) time limit or node limit expires
        CPXmipopt(inst->CPXenv, inst->CPXlp);

        // get solution
        int status = CPXgetx(inst->CPXenv, inst->CPXlp, xbest, 0, inst->ncols - 1);
        if(status) {
            if(init)
                printerr(inst, "Not enough time to find a starting solution!", status);
            else {
                print(inst, 'W', 1, "Not enough time to find an incumbent solution!");
                break;
            }
        }
        CPXgetobjval(inst->CPXenv, inst->CPXlp, &zbest);
        if(zbest < inst->zbest){
            print(inst, 'D', 1, "Found better solution value zbest = %f", zbest);
            inst->zbest = zbest;
            inst->status = CPXgetstat(inst->CPXenv, inst->CPXlp);

            // switch array pointers
            double *temp = xbest;
            xbest = inst->xbest;
            inst->xbest = temp;
        }else {
            k += 2;
            print(inst, 'D', 1, "Increased k = %d", k);
            if(k > 20) {
                print(inst, 'W', 1, "k exceeded limit");
                break;
            }
        }
        // plot
        plot(inst, inst->xbest);

        if(!init) // keep in mind we added a constraint after model definition
            CPXdelrows(inst->CPXenv, inst->CPXlp, inst->nrows, inst->nrows);
        else {
            // initialization done
            init = false;

            // set short time limit
            //CPXsetdblparam(inst->CPXenv, CPXPARAM_TimeLimit, timelim);

            // unset node limit
            //CPXsetlongparam(inst->CPXenv, CPX_PARAM_NODELIM, 9223372036800000000L);
        }
    }
    free(xbest);

    // copy solution
    inst->xstar = (double *) calloc(inst->ncols, sizeof(double)); // TODO that's not "xstar"
    memcpy(inst->xstar, inst->xbest, inst->ncols * sizeof(double));

    inst->zstar = inst->zbest;
}