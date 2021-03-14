#include "tsp.h"

void free_instance(instance *inst){
    free(inst->input_tsp_file_name);
    free(inst->input_opt_file_name);
    free(inst->name[0]);
    free(inst->name[1]);
    free(inst->comment[0]);
    free(inst->comment[1]);

    free(inst->xcoord);
    free(inst->ycoord);
    free(inst->opt_tour);
}

void init_instance(instance *inst){
    // from cli
    inst->input_tsp_file_name = NULL;
    inst->input_opt_file_name = NULL;
    inst->time_limit = -1;
    inst->verbose = 1;

    // from file
    inst->name[0] = inst->name[1] = NULL;
    inst->comment[0] = inst->comment[1] = NULL;
    inst->tot_nodes = -1;
    inst->xcoord = inst->ycoord = NULL;
    inst->opt_tour = NULL;

    // other parameters
    inst->integer_costs = 0;
}

// return CPLEX column position given the coordinates of a cell in adjacency matrix
int xpos(int i, int j, instance *inst) {
    if((i == j) || (j < 0) || (i < 0) || (j >= inst->tot_nodes) || (i >= inst->tot_nodes)){
        printf(BOLDRED"[ERROR] xpos(): unexpected i = %d, j = %d\n" RESET, i, j);
        exit(1);
    }
    // Check if it's a cell above diagonal
    if(i > j) return xpos(j, i, inst);
    // Below formula derives from
    // pos = j - 1 + i*n - sum_1^i(k+1)
    int pos = i * inst->tot_nodes + j - (( i + 1 ) * ( i + 2 )) / 2;
    return pos;
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
    char *cname[1]; // string array required by cplex
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
            }
        }
    }

    // add the 2 degree constraints
    for ( int h = 0; h < inst->tot_nodes; h++ ){
        // define right hand side
        double rhs = 2.0;
        // define the type of constraint (array) ('E' for equality)
        char sense = 'E';
        // define constraint name (array)
        sprintf(cname[0], "degree(%d)", h+1);
        if ( (err = CPXnewrows(env, lp, 1, &rhs, &sense, NULL, cname)) ){
            printf(BOLDRED "[ERROR] CPXnewrows() error code %d\n" RESET, err);
            exit(1);
        }
        int lastrow = CPXgetnumrows(env,lp) - 1;
        // change last row coefficients from 0 to 1
        for ( int i = 0; i < inst->tot_nodes; i++ ){
            if ( i == h ) continue; // skip auto-loops
            if ( (err = CPXchgcoef(env, lp, lastrow, xpos(i,h, inst), 1.0)) ) {
                printf(BOLDRED "[ERROR] Cannot change coefficient: error code %d", err);
            }
        }
    }

    if (inst->verbose >= 0) CPXwriteprob(env, lp, "model.lp", NULL);

    free(cname[0]);

    if(inst->verbose >=1) printf(BOLDGREEN "[INFO] Model built.\n" RESET);
}

char * TSPOpt(instance *inst){
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

    build_model(inst, env, lp);

    // optimize!
    if(inst->verbose >=1) printf(BOLDGREEN "[INFO] Optimization started! Wait please...\n" RESET);
    if((err = CPXmipopt(env, lp))){
        printf(BOLDRED "[ERROR] CPXmipopt(env, lp): error code %d\n" RESET, err);
        exit(1);
    }
    if(inst->verbose >=1) printf(BOLDGREEN "[INFO] Optimization finished!\n" RESET);

    // get solution
    int tot_cols = CPXgetnumcols(env, lp);
    double *xstar = (double *) calloc(tot_cols, sizeof(double));
    if((err = CPXgetx(env, lp, xstar, 0, tot_cols-1))){
        printf(BOLDRED "[ERROR] CPXgetx(env, lp, xstar, 0, tot_cols-1): error code %d\n" RESET, err);
        exit(1);
    }

    // scan adjacency matrix and print values
    if(inst->verbose >=2) printf("Solution found:\n");
    // rounded solution
    char *rxstar = (char *) calloc(tot_cols, sizeof(char));
    for(int i = 0; i < inst->tot_nodes; i++){
        for ( int j = i+1; j < inst->tot_nodes; j++ ){
            if (xstar[xpos(i,j,inst)] > 0.5) { // deal with numeric errors
                if(inst->verbose >= 2) printf("x(%3d,%3d) = 1\n", i + 1, j + 1);
                rxstar[xpos(i, j, inst)] = 1;
            }
        }
    }

    free(xstar);

    // free and close cplex model
    CPXfreeprob(env, &lp);
    CPXcloseCPLEX(&env);
    return rxstar;
}