//
// Created by enrico on 17/03/21.
//

#include <stdlib.h>
#include <cplex.h>
#include <unistd.h>
#include <stdarg.h>
#include "utils.h"

const char *formulation_names[] = {"cuts", "Benders", "MTZ", "GG", "GGi"};

void init_instance(instance *inst){
    // ===== from cli =====
    inst->input_tsp_file_name = NULL;
    inst->input_opt_file_name = NULL;
    inst->formulation = BENDERS;
    inst->lazy = false;
    inst->seed = DEFAULT_CPLEX_SEED;
    inst->integer_costs = true;
    inst->time_limit = CPX_INFBOUND;
    inst->mem_limit = 15000;
    inst->gui = true;
    inst->do_plot = true;
    inst->no_opt = false;
    inst->perfr = 0;
    inst->perfl = NULL;
    inst->verbose = 1;

    // ===== from file =====
    inst->name[0] = inst->name[1] = NULL;
    inst->comment[0] = inst->comment[1] = NULL;
    inst->nnodes = -1;
    inst->dist = EUC_2D;
    inst->xcoord = inst->ycoord = NULL;
    inst->opt_tour = NULL;

    // ===== CPLEX =====
    inst->CPXenv = NULL;
    inst->CPXlp = NULL;
    inst->ncols = 0;

    // ===== other parameters =====
    inst->directed = false;
    inst->tstart.tv_sec = inst->tstart.tv_usec = 0;

    // ===== results =====
    inst->runtime = -1;
    inst->xstar = NULL;
    inst->status = -1; // to be set to >0 by CPLEX
}
void free_instance(instance *inst){
    free(inst->input_tsp_file_name);
    free(inst->input_opt_file_name);
    free(inst->perfl);

    free(inst->name[0]);
    free(inst->name[1]);

    free(inst->comment[0]);
    free(inst->comment[1]);

    free(inst->xcoord);
    free(inst->ycoord);

    free(inst->opt_tour);

    free(inst->xstar);

    CPXfreeprob(inst->CPXenv, &inst->CPXlp);
    CPXcloseCPLEX(&inst->CPXenv);
}

void save_instance_to_tsp_file(instance *inst){
    char name[BUFLEN];
    snprintf(name,BUFLEN, "%s.save.tsp", inst->name[0]);
    FILE *fout = fopen(name, "w");
    fprintf(fout,"NAME : %s\n", inst->name[0]);
    fprintf(fout,"COMMENT : %s\n", inst->comment[0]);
    fprintf(fout,"TYPE : TSP\n");
    fprintf(fout,"DIMENSION : %d\n", inst->nnodes);
    fprintf(fout,"EDGE_WEIGHT_TYPE : %s\n", "EUC_2D"); // can be changed
    fprintf(fout,"NODE_COORD_SECTION\n");
    for(int i = 0; i < inst->nnodes; i++)
        fprintf(fout, "%d %f %f\n", i + 1, inst->xcoord[i], inst->ycoord[i]);
    fprintf(fout, "EOF\n");
    fclose(fout);
}

/**
 * Print error, free memory and exit
 * @param inst instance to free
 * @param err explicative string
 */
void printerr(instance *inst, const char *err, ...){
    va_list args;
    va_start(args, err);
    char buf[BUFLEN];
    snprintf(buf, BUFLEN, BOLDRED "[ERR] %s\n" RESET, err);
    vprintf(buf, args);
    va_end(args);
    free_instance(inst);
    exit(1);
}

void print(instance *inst, char type, int lv, const char *msg, ...){
    if(inst->verbose < lv) return;

    va_list args;
    va_start(args, msg);
    char buf[BUFLEN];
    switch(type){
    case 'E':
        snprintf(buf, BUFLEN, BOLDRED "[ERR] %s\n" RESET, msg);
        vprintf(buf, args);
        break;
    case 'W':
        snprintf(buf, BUFLEN, BOLDYELLOW "[WARN] %s\n" RESET, msg);
        vprintf(buf, args);
        break;
    case 'D':
        snprintf(buf, BUFLEN, "[DEBUG] %s\n", msg);
        vprintf(buf, args);
        break;
    case 'I':
    default:
        snprintf(buf, BUFLEN, BOLDGREEN "[INFO] %s\n" RESET, msg);
        vprintf(buf, args);
        break;
    }
    va_end(args);
}
