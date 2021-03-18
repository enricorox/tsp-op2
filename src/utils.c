//
// Created by enrico on 17/03/21.
//

#include <stdlib.h>
#include "utils.h"
void init_instance(instance *inst){
    // from cli
    inst->input_tsp_file_name = NULL;
    inst->input_opt_file_name = NULL;
    inst->time_limit = -1;
    inst->gui = true;
    inst->do_plot = true;
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

void parse_cli(int argc, char **argv, instance *inst){
    // parse cli
    bool help = false;
    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i],"--file") == 0){
            if(argv[++i] != NULL) {
                inst->input_tsp_file_name = malloc(strlen(argv[i]) + 1);
                strcpy(inst->input_tsp_file_name, argv[i]);
            }
            continue;
        }
        if(strcmp(argv[i],"--opt-tour") == 0){
            if(argv[++i] != NULL) {
                inst->input_opt_file_name = malloc(strlen(argv[i]) + 1);
                strcpy(inst->input_opt_file_name, argv[i]);
            }
            continue;
        }
        if(strcmp(argv[i],"--time-limit") == 0){
            if(argv[++i] != NULL)
                inst->time_limit = atof(argv[i]);
            continue;
        }
        if(strcmp(argv[i],"--no-gui") == 0){ inst->gui = false; continue;}
        if(strcmp(argv[i],"--no-plot") == 0){ inst->do_plot = false; continue;}
        if(strcmp(argv[i], "--verbose") == 0){
            if(argv[++i] != NULL)
                inst->verbose = atoi(argv[i]);
            continue;
        }
        if(strcmp(argv[i],"--help") == 0) { help = 1; continue; }

        printf(BOLDRED "[ERROR] Unknown option: %s\n" RESET, argv[i]);
        help = true; // need to show help if came here
    }

    if(inst->verbose >=1) printf(BOLDGREEN "[INFO] Command line arguments parsed.\n" RESET);

    // print arguments
    if(inst->verbose >= 2){
        if(inst->verbose >= 3 && !help)
            printf(USAGE);

        printf("Command line arguments found (include defaults):\n");
        printf("--file          %s\n", inst->input_tsp_file_name);
        printf("--opt-tour      %s\n", inst->input_opt_file_name);
        printf("--time-limit    %f\n", inst->time_limit);
        printf("--no-gui        %s\n", inst->gui?"false":"true");
        printf("--no-plot       %s\n", inst->do_plot?"false":"true");
        printf("--verbose       %d\n", inst->verbose);
    }

    if(help){
        printf(USAGE);
        exit(1);
    }

    if(inst->input_tsp_file_name == NULL){
        printf(BOLDRED "[ERROR] Check mandatory arguments!\n" RESET);
        exit(1);
    }
}

void parse_file(instance *inst, char *file_name){
    // reading optimal tour file?
    bool opt;
    if(file_name == inst->input_tsp_file_name)
        opt = false;
    else if(file_name == inst->input_opt_file_name)
        opt = true;
    else{
        printf(BOLDRED "[ERROR] parse_file(): Illegal argument" RESET);
        exit(1);
    }
    // open file
    FILE *fin = fopen(file_name, "r");
    if (fin == NULL ){
        printf(BOLDRED "[ERROR] input file %s not found!\n" RESET, inst->input_tsp_file_name);
        free_instance(inst);
        exit(1);}

    // parse file
    char line[BUFLEN];
    char *param_name, *param;
    while(1){
        // check for error & read line
        if(fgets(line, sizeof(line), fin) == NULL){
            printf(BOLDRED "[WARN] EOF not found!\n" RESET);
        }

        // debug output
        if(inst->verbose >= 3) printf("[DEBUG] Reading line: %s", line);

        // tokenize line
        param_name = strtok(line, ": \n");
        param = strtok(NULL, ": \n");

        // --------- compare strings --------- //
        if(strcmp(param_name, "") == 0) continue; // ignore empty lines

        if(strncmp(param_name, "NAME", 4) == 0) {
            inst->name[opt?1:0] = malloc(strlen(param)+1);
            strcpy(inst->name[opt?1:0], param);
            if(inst->verbose >=2) printf("NAME = %s\n", inst->name[opt?1:0]);
            continue;
        }

        if(strncmp(param_name, "TYPE", 4) == 0){
            if(inst->verbose >=2) printf("TYPE = %s\n", param);
            if(strncmp(param, "TSP",3) != 0 && strncmp(param, "TOUR", 4) != 0){
                printf(BOLDRED "[ERROR] TYPE = %s not supported yet.\n" RESET, param);
                free_instance(inst);
                exit(1);
            }
            continue;
        }

        if(strncmp(param_name, "COMMENT", 7) == 0) {
            inst->comment[opt?1:0] = malloc(sizeof(line)+1);
            // revert tokenization!
            if(strtok(NULL,"\n") != NULL) param[strlen(param)] = ' ';
            strcpy(inst->comment[opt?1:0], param);
            if(inst->verbose >=2) printf("COMMENT = %s\n", inst->comment[opt?1:0]);
            continue;
        }

        if(strncmp(param_name, "DIMENSION", 9) == 0){
            int tot_nodes = atoi(param);
            if(inst->tot_nodes > 0 && tot_nodes != inst->tot_nodes){
                printf(BOLDRED "[ERROR] Different dimensions found!\n" RESET);
                free_instance(inst);
                exit(1);
            }

            inst->tot_nodes = tot_nodes;

            if(inst->verbose >=2) printf("DIMENSION = %d\n", inst->tot_nodes);
            if(inst->tot_nodes <= 0) {
                printf("[ERROR] Cannot allocate %d nodes!\n", inst->tot_nodes);
                exit(1);
            }
            continue;
        }

        if(strncmp(param_name, "EDGE_WEIGHT_TYPE", 16) == 0){
            inst->integer_costs = false;
            if(inst->verbose >=2) printf("EDGE_WEIGHT_TYPE = %s\n", param);
            if(strncmp(param, "EUC_2D", 3) != 0){
                printf(BOLDRED "[ERROR] EDGE_WEIGHT_TYPE = %s is not supported yet.\n" RESET, param);
                free_instance(inst);
                exit(1);
            }
            continue;
        }

        if(strncmp(param_name, "NODE_COORD_SECTION", 18) == 0) {
            // check if tot_nodes is given
            if (inst->tot_nodes <= 0) {
                printf("[ERROR] DIMENSION must be before NODE_COORD_SECTION!\n");
                free_instance(inst);
                exit(1);
            }
            if (inst->verbose >= 2) printf("NODE_COORD_SECTION:\n");
            // allocate arrays
            inst->xcoord = (double *) calloc(inst->tot_nodes, sizeof(double));
            inst->ycoord = (double *) calloc(inst->tot_nodes, sizeof(double));
            if (inst->xcoord == NULL || inst->ycoord == NULL){
                printf(BOLDRED "[ERROR] Can't allocate memory: too many nodes (tot_nodes = %d)!\n" RESET, inst->tot_nodes);
                free_instance(inst);
                exit(1);
            }
            int node_number;
            // read nodes
            for(int n = 0; n < inst->tot_nodes; n++){
                if(fgets(line, sizeof(line), fin) == NULL){
                    printf(BOLDRED "[ERROR] I/O error" RESET);
                    free_instance(inst);
                    exit(1);
                }
                if(inst->verbose >= 3) printf("Reading line: %s", line);
                node_number = atoi(strtok(line, " "));
                if(node_number != n+1){
                    printf((node_number == 0) ? BOLDRED "[ERROR] Too few nodes!\n" RESET:
                           BOLDRED "[ERROR] Nodes must be ordered!\n" RESET);
                    free_instance(inst);
                    exit(1);
                }
                inst->xcoord[n] = atof(strtok(NULL, " "));
                inst->ycoord[n] = atof(strtok(NULL, " "));

                // show node
                if(inst->verbose >=2) printf("n%d = (%f, %f)\n", n+1, inst->xcoord[n], inst->ycoord[n]);
            }
            continue;
        }

        if(strncmp(param_name, "TOUR_SECTION", 12) == 0){
            // check if tot_nodes is given
            if (inst->tot_nodes <= 0) {
                printf(BOLDRED "[ERROR] DIMENSION must be before TOUR_SECTION!\n" RESET);
                free_instance(inst);
                exit(1);
            }
            if (inst->verbose >= 2) printf("TOUR_SECTION:\n");
            // allocate arrays
            inst->opt_tour = (int *) calloc(inst->tot_nodes, sizeof(double));
            char *duplicates = (char *) calloc(inst->tot_nodes, sizeof(char));
            if (inst->xcoord == NULL || inst->ycoord == NULL){
                printf(BOLDRED "[ERROR] Can't allocate memory: too many nodes (tot_nodes = %d)!\n" RESET, inst->tot_nodes);
                free_instance(inst);
                exit(1);
            }
            // read nodes
            int node;
            for(int n = 0; n < inst->tot_nodes; n++){
                if(fgets(line, sizeof(line), fin) == NULL){
                    printf(BOLDRED "[ERROR] I/O error" RESET);
                    free_instance(inst);
                    exit(1);
                }
                if(inst->verbose >= 3) printf("[DEBUG] Reading line: %s", line);
                node = atoi(line);
                if(node <= 0){
                    printf((node == 0) ? BOLDRED "[ERROR] Too few nodes!\n" RESET:
                           BOLDRED "[ERROR] Nodes must be positive!\n" RESET);
                    free(duplicates);
                    free_instance(inst);
                    exit(1);
                }
                if(node > inst->tot_nodes){
                    printf(BOLDRED "[ERROR] Illegal node: %d\n" RESET, node);
                    free(duplicates);
                    free_instance(inst);
                    exit(1);
                }
                if(duplicates[node-1] != 0){
                    printf(BOLDRED "[ERROR] That's not a tour: node %d is not unique!\n" RESET, node);
                    free(duplicates);
                    free_instance(inst);
                    exit(1);
                }
                duplicates[node-1]++;
                inst->opt_tour[n] = node;

                // show node
                if(inst->verbose >=2) printf("n%d = %d\n", n+1, inst->opt_tour[n]);
            }
            free(duplicates);
            continue;
        }

        if(strncmp(param_name, "EOF", 3) == 0 || (strncmp(param_name, "-1", 2) == 0 && opt)) {
            if(inst->verbose >=2) printf("End Of File\n");
            break;
        }

        // default case
        printf(BOLDRED "[ERROR] param_name = %s is unknown\n" RESET, param_name);
        exit(1);
    }
    if(inst->verbose >=1) printf(BOLDGREEN "[INFO] File %s parsed.\n" RESET, file_name);
    fclose(fin);
}