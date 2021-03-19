#include "tsp.h"

// return CPLEX column position given the coordinates of a cell in adjacency matrix
int xpos(int i, int j, instance *inst) {
    if((i == j) || (j < 0) || (i < 0) || (j >= inst->tot_nodes) || (i >= inst->tot_nodes)){
        printf(BOLDRED"[ERROR] xpos(): unexpected i = %d, j = %d\n" RESET, i, j);
        free_instance(inst);
        exit(1);
    }
    // Check if it's a cell above diagonal
    if(i > j) return xpos(j, i, inst);
    // Below formula derives from
    // pos = j - 1 + i*n - sum_1^i(k+1)
    int pos = i * inst->tot_nodes + j - (( i + 1 ) * ( i + 2 )) / 2;
    return pos;
}

int xpos_compact(int i, int j, instance *inst){
    if((j < 0) || (i < 0) || (j >= inst->tot_nodes) || (i >= inst->tot_nodes)){
        printf(BOLDRED"[ERROR] xpos_compact(): unexpected i = %d, j = %d\n" RESET, i, j);
        free_instance(inst);
        exit(1);
    }
    int pos = i * inst->tot_nodes + j;
    return pos;
}

int upos(int i, instance *inst){
    if(i < 0 || i >= inst->tot_nodes){
        printf(BOLDRED"[ERROR] xpos(): unexpected i = %d\n" RESET, i);
        exit(1);
    }
    int upos = inst->tot_nodes * inst->tot_nodes + i;
    return upos;
}

double dist(int i, int j, instance *inst) {
    double dx = inst->xcoord[i] - inst->xcoord[j];
    double dy = inst->ycoord[i] - inst->ycoord[j];
    if ( !inst->integer_costs ) return sqrt(dx*dx+dy*dy);
    int dis = (int) (sqrt(dx*dx+dy*dy) + 0.499999999); // nearest integer
    return dis+0.0;
}

void build_model(instance *inst, CPXENVptr env, CPXLPptr lp) {
    char binary = 'B';
    int err;

    // allocate array for variable's name with trick
    //char **cname = (char **) calloc(1, sizeof(char *)); // don't need to dynamically allocate 1 element
    char *cname[1]; // string array required by cplex for batch insertions
    cname[0] = (char *) calloc(128, sizeof(char));

    // add binary vars x(i,j) for i < j
    // one for each edge
    for ( int i = 0; i < inst->tot_nodes; i++ ){
        for ( int j = i+1; j < inst->tot_nodes; j++ ){
            // define variable name
            sprintf(cname[0], "x(%d,%d)", i+1,j+1);
            // define its cost
            double obj = dist(i,j,inst); // cost == distance
            // define its lower bound
            double lb = 0.0;
            // define its upper bound
            double ub = 1.0;
            if ( (err = CPXnewcols(env, lp, 1, &obj, &lb, &ub, &binary, cname)) ) {
                printf(BOLDRED "[ERROR] CPXnewcols(): error code %d\n" RESET, err);
                exit(1);
            }
            // check xpos on the fly (can be removed if I'm sure it's ok?)
            if (CPXgetnumcols(env,lp)-1 != xpos(i,j, inst)) {
                printf(BOLDRED "[ERROR] xpos() got a bad index!\n" RESET);
                free(cname[0]);
                free_instance(inst);
                exit(1);
            }
        }
    }

    // add the 2 degree constraints
    for ( int h = 0; h < inst->tot_nodes; h++ ){
        // define right hand side
        double rhs = 2.0;
        // define the type of constraint (array) ('E' for equality)
        char sense = 'E';
        // define constraint name
        sprintf(cname[0], "degree(%d)", h+1);
        if ((err = CPXnewrows(env, lp, 1, &rhs, &sense, NULL, cname)) ){
            printf(BOLDRED "[ERROR] CPXnewrows() error code %d\n" RESET, err);
            exit(1);
        }
        int lastrow_idx = CPXgetnumrows(env, lp) - 1; // row index starts from 0
        // change last row coefficients from 0 to 1
        for ( int i = 0; i < inst->tot_nodes; i++ ){
            if ( i == h ) continue; // skip auto-loops
            if ( (err = CPXchgcoef(env, lp, lastrow_idx, xpos(i, h, inst), 1.0)) ) {
                printf(BOLDRED "[ERROR] Cannot change coefficient: error code %d", err);
            }
        }
    }
    free(cname[0]);
}

void build_model_MTZ(instance *inst, CPXENVptr env, CPXLPptr lp) {
    char binary = 'B';
    char integer = 'I';
    int err;

    // allocate array for variable's name with trick
    //char **cname = (char **) calloc(1, sizeof(char *)); // don't need to dynamically allocate 1 element
    char *cname[1]; // string array required by cplex for batch insertions
    cname[0] = (char *) calloc(BUFLEN, sizeof(char));

    // add binary vars x(i,j) for i != j
    // one for each edge without auto-loops
    for ( int i = 0; i < inst->tot_nodes; i++ ){
        for ( int j = 0; j < inst->tot_nodes; j++ ){
            // define variable name
            sprintf(cname[0], "x(%d,%d)", i+1,j+1);
            // define its cost
            double obj = dist(i,j,inst); // cost == distance
            // define its lower bound
            double lb = 0.0;
            // define its upper bound
            double ub = 1.0;
            if((err = CPXnewcols(env, lp, 1, &obj, &lb, &ub, &binary, cname))) {
                printf(BOLDRED "[ERROR] CPXnewcols(): error code %d\n" RESET, err);
                exit(1);
            }
            // check xpos on the fly (can be removed if I'm sure it's ok)
            if (CPXgetnumcols(env,lp)-1 != xpos_compact(i,j, inst)) {
                printf(BOLDRED "[ERROR] xpos_compact() got a bad index!\n" RESET);
                free(cname[0]);
                free_instance(inst);
                exit(1);
            }
        }
    }

    // add integer vars u(i) for each node i
    for(int i = 0; i < inst->tot_nodes; i++){
        sprintf(cname[0], "u(%d)", i+1);
        double obj = 0;
        double lb = 0;
        double ub = i?(inst->tot_nodes - 2):0;
        if((err = CPXnewcols(env, lp, 1, &obj, &lb, &ub, &integer, cname))) {
            printf(BOLDRED "[ERROR] CPXnewcols(): error code %d\n" RESET, err);
            free_instance(inst); free(cname[0]);
            exit(1);
        }
        // check xpos on the fly (can be removed if I'm sure it's ok)
        if(CPXgetnumcols(env,lp)-1 != upos(i, inst)) {
            printf(BOLDRED "[ERROR] upos() got a bad index!\n" RESET);
            free_instance(inst); free(cname[0]);
            exit(1);
        }
    }

    // add the 1 degree in and out constraints
    for(char out = 0; out < 2; out++)
        for(int h = 0; h < inst->tot_nodes; h++){
            // define right hand side
            double rhs = 1.0;
            // define the type of constraint (array) ('E' for equality)
            char sense = 'E';
            // define constraint name
            sprintf(cname[0], "degree_%s(%d)", out?"out":"in", h+1);
            if ((err = CPXnewrows(env, lp, 1, &rhs, &sense, NULL, cname)) ){
                printf(BOLDRED "[ERROR] CPXnewrows() error code %d\n" RESET, err);
                exit(1);
            }
            int lastrow_idx = CPXgetnumrows(env, lp) - 1; // constraint index starts from 0
            // change last row coefficients from 0 to 1
            for(int i = 0; i < inst->tot_nodes; i++){
                if(i == h) continue;
                if ((err = CPXchgcoef(env, lp, lastrow_idx, xpos_compact(out?h:i, out?i:h, inst), 1.0))) {
                    printf(BOLDRED "[ERROR] Cannot change coefficient: error code %d", err);
                }
            }
        }

    // position big M constraints
    int big_M = inst->tot_nodes - 1; // use big M trick
    for(int i = 1; i < inst->tot_nodes; i++)
        for(int j = 1; j < inst->tot_nodes; j++){
            double rhs = big_M - 1;
            char sense = 'L';
            sprintf(cname[0], "position(%d,%d)", i + 1, j + 1);
            if ((err = CPXnewrows(env, lp, 1, &rhs, &sense, NULL, cname)) ){
                printf(BOLDRED "[ERROR] CPXnewrows() error code %d\n" RESET, err);
                exit(1);
            }
            int lastrow_idx = CPXgetnumrows(env, lp) - 1; // constraint index starts from 0

            int err1, err2, err3;
            // change last row coefficients
            err3 = err2 = err1 = 0;
            if(i != j) {
                err1 = CPXchgcoef(env, lp, lastrow_idx, upos(i, inst), 1.0);
                err2 = CPXchgcoef(env, lp, lastrow_idx, upos(j, inst), -1.0);
            }
            err3 = CPXchgcoef(env, lp, lastrow_idx, xpos_compact(i, j, inst), big_M);
            if (err1 || err2 || err3) {
                printf(BOLDRED "[ERROR] Cannot change coefficient\n");
            }
        }

    free(cname[0]);
}

char * TSPOpt(instance *inst){
    // performance check
    struct timeval start, stop;
    gettimeofday(&start, NULL);

    // open CPLEX model
    int err;
    CPXENVptr env = CPXopenCPLEX(&err);
    if(err){
        printf(BOLDRED "[ERROR] CPLEX error code: %d" RESET, err);
        exit(1);
    }
    CPXLPptr lp = CPXcreateprob(env, &err, "TSP");
    if(err){
        printf(BOLDRED "[ERROR] CPLEX error code: %d\n" RESET, err);
        exit(1);
    }

    // choose formulation
    switch(inst->formulation) {
        case MTZ: build_model_MTZ(inst, env, lp); break;
        default: build_model(inst, env, lp);
    }

    // write model to file
    char file_name[BUFLEN];
    sprintf(file_name, "%s-model-%s.lp", inst->name[0], formulation_names[inst->formulation]);
    CPXwriteprob(env, lp, file_name, "lp");

    if(inst->verbose >=1) printf(BOLDGREEN "[INFO] Model saved to %s\n" RESET, file_name);

    // Set CPLEX parameters
    CPXsetdblparam(env, CPX_PARAM_TILIM, inst->time_limit);

    // optimize!
    if(inst->verbose >=1) printf(BOLDGREEN "[INFO] Optimization started! Wait please...\n" RESET);
    if((err = CPXmipopt(env, lp))){
        printf(BOLDRED "[ERROR] CPXmipopt(env, lp): error code %d\n" RESET, err);
        free_instance(inst);
        exit(1);
    }

    // get solution
    int tot_cols = CPXgetnumcols(env, lp);
    double *xstar = (double *) calloc(tot_cols, sizeof(double));
    if((err = CPXgetx(env, lp, xstar, 0, tot_cols-1))){
        printf(BOLDRED "[ERROR] CPXgetx(env, lp, xstar, 0, tot_cols-1): error code %d\n" RESET, err);
        exit(1);
    }

    gettimeofday(&stop, NULL);
    if(inst->verbose >=1) printf(BOLDGREEN "[INFO] Optimization finished in %ld seconds!\n" RESET,
                                 stop.tv_sec-start.tv_sec);

    // scan adjacency matrix and print values
    if(inst->verbose >=2) printf("Solution found:\n");
    // rounded solution
    char *rxstar = (char *) calloc(tot_cols, sizeof(char));
    for(int i = 0; i < inst->tot_nodes; i++){
        for ( int j = 0; j < inst->tot_nodes; j++ ){ // TODO j = i+1
            if (xstar[xpos_compact(i,j,inst)] > 0.5) { // deal with numeric errors TODO fix
                if(inst->verbose >= 2) printf("x(%3d,%3d) = 1\n", i + 1, j + 1);
                rxstar[xpos_compact(i, j, inst)] = 1; // TODO fix
            }
        }
    }

    free(xstar);

    // free and close cplex model
    CPXfreeprob(env, &lp);
    CPXcloseCPLEX(&env);
    return rxstar;
}