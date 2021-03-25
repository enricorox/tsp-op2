//
// Created by enrico on 17/03/21.
//

#include <stdlib.h>
#include <cplex.h>
#include <unistd.h>
#include "utils.h"

const char *formulation_names[] = {"standard", "MTZ", "GG"};

void init_instance(instance *inst){
    // ===== from cli =====
    inst->input_tsp_file_name = NULL;
    inst->input_opt_file_name = NULL;
    inst->formulation = STANDARD;
    inst->lazy = false;
    inst->seed = 0;
    inst->integer_costs = true;
    inst->time_limit = CPX_INFBOUND;
    inst->gui = true;
    inst->do_plot = true;
    inst->no_opt = false;
    inst->perfr = 0;
    inst->perfl = NULL;
    inst->runs = 1;
    inst->verbose = 1;

    // ===== from file =====
    inst->name[0] = inst->name[1] = NULL;
    inst->comment[0] = inst->comment[1] = NULL;
    inst->tot_nodes = -1;
    inst->dist = EUC_2D;
    inst->xcoord = inst->ycoord = NULL;
    inst->opt_tour = NULL;

    // ===== other parameters =====
    inst->directed = false;

    // ===== results =====
    inst->runtime = -1;
    inst->xstar = NULL;
    inst->status = -1; // to be set to >0 by CPLEX
}
void free_instance(instance *inst){
    free(inst->input_tsp_file_name);
    //printf("Freeing input_opt_filename %p...\n", inst->input_opt_file_name);
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
}

void save_instance_to_tsp_file(instance *inst){
    char name[BUFLEN];
    snprintf(name,BUFLEN, "%s.save.tsp", inst->name[0]);
    FILE *fout = fopen(name, "w");
    fprintf(fout,"NAME : %s\n", inst->name[0]);
    fprintf(fout,"COMMENT : %s\n", inst->comment[0]);
    fprintf(fout,"TYPE : TSP\n");
    fprintf(fout,"DIMENSION : %d\n", inst->tot_nodes);
    fprintf(fout,"EDGE_WEIGHT_TYPE : %s\n", "EUC_2D"); // can be changed
    fprintf(fout,"NODE_COORD_SECTION\n");
    for(int i = 0; i < inst->tot_nodes; i++)
        fprintf(fout, "%d %f %f\n", i + 1, inst->xcoord[i], inst->ycoord[i]);
    fprintf(fout, "EOF\n");
    fclose(fout);
}
