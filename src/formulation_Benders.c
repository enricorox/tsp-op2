//
// Created by enrico on 19/03/21.
//

#include <unistd.h>
#include "formulation_Benders.h"
#include "plot.h"

void find_conn_comp(instance *inst, const double *xstar, int *ncomp, int *succ, int *comp){
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
                if(curr != next && xstar[xpos(curr, next, inst)] > 0.5 && comp[next] == -1){
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

CPXLPptr update_constraints(instance *inst, CPXENVptr env, CPXLPptr lp, int ncomp, const int *comp){
    if(ncomp == 1) printerr(inst, "Illegal state: must be ncomp > 1");
    int status;
    CPXLPptr lp_clone = CPXcloneprob(env, lp, &status);
    if(status) printerr(inst, "Can't copy problem!");
    if(CPXfreeprob(env, &lp)) printerr(inst, "Can't free problem!");

    int tot_cols = CPXgetnumcols(env, lp_clone);
    printf("tot_cols = %d\n", tot_cols);
    char *rname[] = {(char *) malloc(BUFLEN)};
    double *value = (double *) malloc(tot_cols * sizeof(double));
    int *index = (int *) malloc(tot_cols * sizeof(int));
    // initialize value vector that doesn't change!
    for(int i = 0; i < tot_cols; i++) value[i] = 1;
    char sense = 'L';
    int izero = 0;

    for(int curr_comp = 1; curr_comp <= ncomp; curr_comp++){
        int comp_size = 0;
        int tot_edges = 0;

        for(int i = 0; i < inst->tot_nodes; i++){
            if(comp[i] != curr_comp) continue;
            comp_size++;
            for(int j = i + 1; j < inst->tot_nodes; j++){
                if(comp[j] != curr_comp) continue;

                // here `i` and `j` must belong to `curr_comp`
                if(tot_edges >= tot_cols) printerr(inst, "Illegal state: must be tot_edges < tot_cols!");
                //printf("nnz = %d\n", comp_size);
                index[tot_edges++] = xpos(i, j, inst);
            }
        }
        double rhs = comp_size - 1;
        int nnz = tot_edges;
        snprintf(rname[0], BUFLEN, "SEC(%d, %d)", curr_comp, comp_size);
        if(CPXaddrows(env, lp_clone, 0, 1, nnz, &rhs, &sense, &izero, index, value, NULL, rname))
            printerr(inst, "Can't add rows!");
        printf("Added constraints for component %d of size %d\n", curr_comp, comp_size);
    }
    free(rname[0]);
    free(index);
    free(value);
    return lp_clone;
}

void loop_benders(instance *inst, CPXENVptr env, CPXLPptr lp) {
    build_model_base_undirected(env, lp, inst);

    int tot_cols = CPXgetnumcols(env, lp);
    int ncomp = inst->tot_nodes;
    double *xstar = (double *) malloc(tot_cols * sizeof(double));
    int *succ = (int *) malloc(inst->tot_nodes * sizeof(int));
    int *comp = (int *) malloc(inst->tot_nodes * sizeof(int));

    while(true) {
        CPXmipopt(env, lp);

        if(CPXgetx(env, lp, xstar, 0, tot_cols - 1)) {
            printerr(inst, "CPXgetx(): error retrieving xstar!");
        }
        inst->xstar = xstar;
        find_conn_comp(inst, xstar, &ncomp, succ, comp);
        printf("Found %d connected components\n", ncomp);
        //plot(inst, inst->xstar);
        if(ncomp > 1)
            lp = update_constraints(inst, env, lp, ncomp, comp);
        else
            break;
    }
    if(ncomp != 1)
        printerr(inst, "Illegal state: must be ncomp = 1");

    free(succ);
    free(comp);
    // TODO set CPX_PARAM_OBJLLIM lower bound before adding new constraints!
}

void get_solution_Benders(instance *inst, CPXENVptr env, CPXLPptr lp){
    // get solution from CPLEX
    int tot_cols = CPXgetnumcols(env, lp);
    double *xstar = (double *) calloc(tot_cols, sizeof(double));
    if (CPXgetx(env, lp, xstar, 0, tot_cols - 1)) {
        free(xstar);
        printerr(inst, "CPXgetx(): error retrieving xstar!");
    }

    // scan adjacency matrix induced by xstar and print values
    if(inst->verbose >=2) printf("Solution %s found:\n", inst->status == CPX_STAT_OPTIMAL ? "optimal":"");

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

    inst->xstar = rxstar;
    free(xstar);
}