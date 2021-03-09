#include <string.h>
#include "tsp.h"

#define USAGE   "Usage: ./tsp --file <input-file> --time-limit <time> [options]\n" \
                "--file <input-file>                file in tsp format\n"\
                "--time-limit <time>                max overall time in seconds\n" \
                "--verbose <n>                      0=quiet, 1=default, 2=verbose, 3=very verbose\n" \
                "--help                             show this help\n\n"

void free_instance(instance *inst){
    free(inst->xcoord);
    free(inst->ycoord);
}

void parse_command_line(int argc, char **argv, instance *inst){
    // put default instance value
    inst->input_file_name = NULL;
    inst->time_limit = -1;
    inst->verbose = 1;

    // parse cli
    char help = (argc >= 3) ? 0 : 1;
    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i],"--file") == 0){ inst->input_file_name = argv[++i]; continue;}
        if(strcmp(argv[i],"--time-limit") == 0){ inst->time_limit = atof(argv[++i]); continue;}
        if(strcmp(argv[i], "--verbose") == 0){ inst->verbose = atoi(argv[++i]); continue;}
        if (strcmp(argv[i],"--help") == 0) { help = 1; continue; }

        printf(BOLDRED "[ERROR] Unknown option: %s\n" RESET, argv[i]);
        help = 1; // need to show help if came here
    }

    if(inst->verbose >=1) printf(BOLDGREEN "[*] Command line arguments parsed.\n" RESET);

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

    if((inst->input_file_name == NULL) || (inst->time_limit == -1)){
        printf(BOLDRED "[ERROR] Check mandatory arguments!\n" RESET);
        exit(1);
    }
}

void parse_tsp_file(instance *inst){
    if(inst->verbose >=1) printf(BOLDGREEN "[*] Parsing tsp file...\n" RESET);
    // put default instance values
    inst->tot_nodes = -1;
    inst->xcoord = inst->ycoord = NULL;

    // open file
    FILE *fin = fopen(inst->input_file_name, "r");
    if (fin == NULL ){
        printf(BOLDRED "[ERROR] input file %s not found!\n" RESET, inst->input_file_name);
        free_instance(inst);
        exit(1);}

    // parse file
    char line[256];
    char *param_name, *param;
    while(1){
        // check for error & read line
        if(fgets(line, sizeof(line), fin) == NULL){
            printf(BOLDRED "[ERROR] EOF not found!\n" RESET);
            free_instance(inst);
            exit(1);
        }

        // verbose output
        if(inst->verbose >= 3) printf("Reading line: %s", line);

        // tokenize line
        param_name = strtok(line, ": \n");
        param = strtok(NULL, ": \n");

        // show tokenized line
        if(inst->verbose >= 2) printf("param_name = %s, param = %s\n", param_name, param);

        // --------- compare strings --------- //
        if(strcmp(param_name, "") == 0) continue; // ignore empty lines

        if(strncmp(param_name, "NAME", 4) == 0) continue;

        if(strncmp(param_name, "TYPE", 4) == 0){
            if(strncmp(param, "TSP",3) != 0){
                printf(BOLDRED "[ERROR] TYPE %s not supported yet.\n" RESET, param);
                free_instance(inst);
                exit(1);
            }
            continue;
        }

        if(strncmp(param_name, "COMMENT", 7) == 0) continue;

        if(strncmp(param_name, "DIMENSION", 9) == 0){
            inst->tot_nodes = atoi(param);
            if(inst->tot_nodes <= 0) {
                printf("[ERROR] Cannot allocate %d nodes!\n", inst->tot_nodes);
                exit(1);
            }
            continue;
        }

        if(strncmp(param_name, "EDGE_WEIGHT_TYPE", 16) == 0){
            if(strncmp(param, "ATT", 3) != 0){
                printf(BOLDRED "[ERROR] EDGE_WEIGHT_TYPE %s is not supported yet." RESET, param);
                free_instance(inst);
                exit(1);
            }
            continue;
        }

        if(strncmp(param_name, "NODE_COORD_SECTION", 18) == 0) {
            // check if tot_nodes is given
            if(inst->tot_nodes <= 0){
                printf("[ERROR] DIMENSION must be before NODE_COORD_SECTION!\n");
                free_instance(inst);
                exit(1);
            }

            // allocate arrays
            inst->xcoord = (double *) calloc(inst->tot_nodes, sizeof(double));
            inst->ycoord = (double *) calloc(inst->tot_nodes, sizeof(double));
            int node_number = 0;
            // find nodes
            for(int n = 0; n < inst->tot_nodes; n++){
                if(fgets(line, sizeof(line), fin) == NULL){
                    printf(BOLDRED "[ERROR] Format error" RESET);
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

        if(strncmp(param_name, "EOF", 3) == 0) break;

        // default case
        printf(BOLDRED "[ERROR] param_name = %s is unknown\n" RESET, param_name);
        exit(1);
    }
    if(inst->verbose >=1) printf(BOLDGREEN "[*] Tsp file parsed.\n" BOLDGREEN);
    fclose(fin);
}

int main(int argc, char **argv){
    instance inst;
    parse_command_line(argc, argv, &inst);
    parse_tsp_file(&inst);
    free_instance(&inst); // release memory!
}
