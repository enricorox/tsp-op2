#include <string.h>
#include "tsp.h"
#include "plot.h"

#define BUFLEN 256
#define USAGE   "Usage: ./tsp --file <file-tsp> [options]\n"\
                "Options:" \
                "--time-limit <time>                max overall time in seconds\n" \
                "--verbose <n>                      0=quiet, 1=default, 2=verbose, 3=debug\n" \
                "--help                             show this help\n\n"

void parse_command_line(int argc, char **argv, instance *inst){
    // parse cli
    char help = (argc >= 2) ? 0 : 1;
    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i],"--file") == 0){
            i++;
            inst->input_file_name = malloc(strlen(argv[i])+1);
            strcpy(inst->input_file_name, argv[i]);
            continue;
        }
        if(strcmp(argv[i],"--time-limit") == 0){ inst->time_limit = atof(argv[++i]); continue;}
        if(strcmp(argv[i], "--verbose") == 0){ inst->verbose = atoi(argv[++i]); continue;}
        if (strcmp(argv[i],"--help") == 0) { help = 1; continue; }

        printf(BOLDRED "[ERROR] Unknown option: %s\n" RESET, argv[i]);
        help = 1; // need to show help if came here
    }

    if(inst->verbose >=1) printf(BOLDGREEN "[INFO] Command line arguments parsed.\n" RESET);

    // print arguments
    if(inst->verbose >= 2){
        if(inst->verbose >= 3){ printf(USAGE);}
        printf("Command line arguments found (include defaults):\n");
        printf("--file          %s\n", inst->input_file_name);
        printf("--time-limit    %f\n", inst->time_limit);
        printf("--verbose       %d\n", inst->verbose);
    }

    if(help){
        printf(USAGE);
        exit(1);
    }

    if(inst->input_file_name == NULL){
        printf(BOLDRED "[ERROR] Check mandatory arguments!\n" RESET);
        exit(1);
    }
}

void parse_tsp_file(instance *inst){
    // open file
    FILE *fin = fopen(inst->input_file_name, "r");
    if (fin == NULL ){
        printf(BOLDRED "[ERROR] input file %s not found!\n" RESET, inst->input_file_name);
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

        // verbose output
        if(inst->verbose >= 3) printf("Reading line: %s", line);

        // tokenize line
        param_name = strtok(line, ": \n");
        param = strtok(NULL, ": \n");

        // --------- compare strings --------- //
        if(strcmp(param_name, "") == 0) continue; // ignore empty lines

        if(strncmp(param_name, "NAME", 4) == 0) {
            inst->name = malloc(sizeof(param)+1);
            strcpy(inst->name, param);
            if(inst->verbose >=2) printf("NAME = %s\n", inst->name);
            continue;
        }

        if(strncmp(param_name, "TYPE", 4) == 0){
            if(inst->verbose >=2) printf("TYPE = %s\n", param);
            if(strncmp(param, "TSP",3) != 0){
                printf(BOLDRED "[ERROR] TYPE = %s not supported yet.\n" RESET, param);
                free_instance(inst);
                exit(1);
            }
            continue;
        }

        if(strncmp(param_name, "COMMENT", 7) == 0) {
            inst->comment = malloc(sizeof(line)+1);
            // revert tokenization!
            if(strtok(NULL,"\n") != NULL) param[strlen(param)] = ' ';
            strcpy(inst->comment, param);
            if(inst->verbose >=2) printf("COMMENT = %s\n", inst->comment);
            continue;
        }

        if(strncmp(param_name, "DIMENSION", 9) == 0){
            inst->tot_nodes = atoi(param);
            if(inst->verbose >=2) printf("DIMENSION = %d\n", inst->tot_nodes);
            if(inst->tot_nodes <= 0) {
                printf("[ERROR] Cannot allocate %d nodes!\n", inst->tot_nodes);
                exit(1);
            }
            continue;
        }

        if(strncmp(param_name, "EDGE_WEIGHT_TYPE", 16) == 0){
            inst->integer_costs = 0;
            if(inst->verbose >=2) printf("EDGE_WEIGHT_TYPE = %s\n", param);
            if(strncmp(param, "EUC_2D", 3) != 0){
                printf(BOLDRED "[ERROR] EDGE_WEIGHT_TYPE = %s is not supported yet." RESET, param);
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
                printf(BOLDRED "[ERROR] Can't allocate memory: too many nodes!\n" RESET);
                free_instance(inst);
                exit(1);
            }
            int node_number;
            // find nodes
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

        if(strncmp(param_name, "EOF", 3) == 0) {
            if(inst->verbose >=2) printf("End Of File\n");
            break;
        }

        // default case
        printf(BOLDRED "[ERROR] param_name = %s is unknown\n" RESET, param_name);
        exit(1);
    }
    if(inst->verbose >=1) printf(BOLDGREEN "[INFO] Tsp file parsed.\n" RESET);
    fclose(fin);
}

int main(int argc, char **argv){
    instance inst;
    init_instance(&inst);

    parse_command_line(argc, argv, &inst);
    parse_tsp_file(&inst);

    char *rxstar = TSPOpt(&inst);
    plot(&inst, rxstar);

    // release memory!
    free(rxstar);
    free_instance(&inst);
    return 0;
}
