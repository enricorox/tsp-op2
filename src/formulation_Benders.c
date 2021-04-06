//
// Created by enrico on 19/03/21.
//

#include <unistd.h>
#include "formulation_Benders.h"
#include "plot.h"

/**
 * Find connected components.
 *
 * @param inst general instance
 * @param ncomp returned number of components
 * @param succ returned array of successors
 * @param comp returned array specifying components
 */
void findccomp(instance *inst, int *ncomp, int *succ, int *comp){
    // initialize data structures
    *ncomp = 0;
    for(int i = 0; i < inst->tot_nodes; i++)
        succ[i] = comp[i] = -1;

    // choose a node `start` and visit its connected component
    for(int start = 0; start < inst->tot_nodes; start++){
        // skip visited nodes
        if(comp[start] >= 0) continue;

        // a new component is found
        (*ncomp)++;
        int curr = start;
        bool done = false;
        while(!done){ // go and visit the current component
            // assign component to node `i`
            comp[curr] = *ncomp;
            done = true;
            for(int next = 0; next < inst->tot_nodes; next++){
                // the edge [curr, next] is selected in xstar and j was not visited before
                if(curr != next && inst->xstar[xpos(curr, next, inst)] > 0.5 && comp[next] == -1){
                    // set `i` successor
                    succ[curr] = next;
                    // change current node
                    curr = next;
                    done = false;
                    break;
                }
            }
        }
        // last arc to close the cycle
        succ[curr] = start;
    } // go to the next component...
}

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
void updconstr(instance *inst, int ncomp, const int *comp){
    if(ncomp <= 1) printerr(inst, "Illegal state: must be ncomp > 1");

    // clone problem
    int status;
    CPXLPptr lp_clone = CPXcloneprob(inst->CPXenv, inst->CPXlp, &status);
    if(status) printerr(inst, "Can't clone problem!");
    // free old problem
    if(CPXfreeprob(inst->CPXenv, &inst->CPXlp)) printerr(inst, "Can't free problem!");

    inst->CPXlp = lp_clone;

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

        for(int i = 0; i < inst->tot_nodes; i++){
            if(comp[i] != curr_comp) continue;
            csize++;
            for(int j = i + 1; j < inst->tot_nodes; j++){
                if(comp[j] != curr_comp) continue;

                // here `i` and `j` must belong to `curr_comp`
                if(nedges >= tot_cols) printerr(inst, "Illegal state: must be tot_edges < tot_cols!");
                // save index
                index[nedges++] = xpos(i, j, inst);
            }
        }
        double rhs = csize - 1;
        int nnz = nedges;
        snprintf(rname[0], BUFLEN, "SEC(%d,%d)", curr_comp, csize);
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
    int ncomp = inst->tot_nodes;
    double *xstar = (double *) malloc(tot_cols * sizeof(double));
    int *succ = (int *) malloc(inst->tot_nodes * sizeof(int));
    int *comp = (int *) malloc(inst->tot_nodes * sizeof(int));

    print(inst, 'I', 1, "Optimization started! Please wait...");

    struct timeval start, stop;
    long otime = 0;
    int it = 0;
    while(true) {
        if(inst->verbose >= 2) printf(BOLDGREEN "[Benders] Iteration #%d...\n" RESET, ++it);
        CPXmipopt(inst->CPXenv, inst->CPXlp);

        // get xstar
        if(CPXgetx(inst->CPXenv, inst->CPXlp, xstar, 0, tot_cols - 1)) {
            printerr(inst, "CPXgetx(): error retrieving xstar!");
        }
        inst->xstar = xstar;

        gettimeofday(&start, NULL);
        // find connected components
        findccomp(inst, &ncomp, succ, comp);

        if(inst->verbose >= 2)
            printf(BOLDGREEN "[Benders] Found %d connected components\n" RESET, ncomp);

        if(ncomp == 1)
            break;


        // set new lower bound
        double zstar;
        CPXgetobjval(inst->CPXenv, inst->CPXlp, &zstar);
        CPXsetdblparam(inst->CPXenv, CPX_PARAM_OBJLLIM, zstar);


        // add sec for connected components
        updconstr(inst, ncomp, comp);
        gettimeofday(&stop, NULL);
        otime += stop.tv_sec - start.tv_sec;
    }

    print(inst, 'I', 2, "Overhead finding connected components: %ld seconds", otime);
    free(succ);
    free(comp);
}

void get_solution_Benders(instance *inst){
    // get solution from CPLEX
    int tot_cols = CPXgetnumcols(inst->CPXenv, inst->CPXlp);
    double *xstar;
    if(inst->xstar == NULL) {
        xstar = (double *) calloc(tot_cols, sizeof(double));
        if (CPXgetx(inst->CPXenv, inst->CPXlp, xstar, 0, tot_cols - 1)) {
            free(xstar);
            printerr(inst, "CPXgetx(): error retrieving xstar!");
        }
    }
    else
        xstar = inst->xstar;


    // scan adjacency matrix induced by xstar and print values
    // deal with numeric errors
    double *rxstar = (double *) calloc(tot_cols, sizeof(double));
    for(int i = 0; i < inst->tot_nodes; i++){
        for ( int j = i+1; j < inst->tot_nodes; j++ ){
            int idx = xpos(i,j,inst);
            if(xstar[idx] > 0.5) {
                if(inst->verbose >= 2) printf("x(%3d,%3d) = 1\n", i + 1, j + 1);
                rxstar[idx] = 1;
            }
        }
    }
    free(xstar);
    inst->xstar = rxstar;

}