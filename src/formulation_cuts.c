//
// Created by enrico on 08/04/21.
//

#include "formulation_cuts.h"

static int CPXPUBLIC subtourcuts(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void *userhandle ){
    instance *inst = (instance *) userhandle;

    // retrieve xstar
    double* xstar = (double*) malloc(inst->ncols * sizeof(double));
    double objval = CPX_INFBOUND;
    if(CPXcallbackgetcandidatepoint(context, xstar, 0, inst->ncols - 1, &objval))
        printerr(inst, "CPXcallbackgetcandidatepoint error");

    // find connected components
    int ncomp;
    int *succ = (int *) malloc(inst->nnodes * sizeof(int));
    int *comp = (int *) malloc(inst->nnodes * sizeof(int));
    findccomp(inst, xstar, &ncomp, succ, comp);

    // skip if solution found!
    if(ncomp == 1)
        return 0;

    // ===== add cuts to the common pools =====
    double *value = (double *) malloc(inst->ncols * sizeof(double));
    int *index = (int *) malloc(inst->ncols * sizeof(int));
    // initialize value vector that doesn't change!
    for(int i = 0; i < inst->ncols; i++) value[i] = 1;
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
                if(nedges >= inst->ncols) printerr(inst, "Illegal state: must be tot_edges < tot_cols!");
                // save index
                index[nedges++] = xpos(i, j, inst);
            }
        }
        double rhs = csize - 1;
        int nnz = nedges;
        if(CPXcallbackrejectcandidate(context, 1, nnz, &rhs, &sense, &izero, index, value))
            printerr(inst, "Can't add cut!");
        if(inst->verbose >= 3)
            printf("Added cut for component %d of size %d\n", curr_comp, csize);
    }

    free(xstar);
    free(index);
    free(value);
    free(succ);
    free(comp);
}

void build_model_cuts(instance *inst){
    // write base model
    build_model_base_undirected(inst);

    inst->ncols = CPXgetnumcols(inst->CPXenv, inst->CPXlp);

    // install callback
    CPXLONG contextid = CPX_CALLBACKCONTEXT_CANDIDATE;
    if(CPXcallbacksetfunc(inst->CPXenv, inst->CPXlp, contextid, subtourcuts, inst))
        printerr(inst,"CPXcallbacksetfunc() error");
}

void get_solution_cuts(instance *inst){
    get_solution_base_undirected(inst);
}