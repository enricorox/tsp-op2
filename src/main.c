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

void parse_command_line(int argc, char** argv, instance *inst){
    // put default instance value
    inst->input_file_name = NULL;
    inst->time_limit = -1;
    inst->verbose = 1;

    // parse cli
    char help = (argc > 2) ? 0 : 1;
    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i],"--file") == 0){ inst->input_file_name = argv[++i]; continue;}
        if(strcmp(argv[i],"--time-limit") == 0){ inst->time_limit = atof(argv[++i]); continue;}
        if(strcmp(argv[i], "--verbose") == 0){ inst->verbose = atoi(argv[++i]); continue;}
        if (strcmp(argv[i],"--help") == 0) { help = 1; continue; }

        printf("ERROR: Unknown option: %s\n", argv[i]);
        help = 1; // need to show help if came here
    }

    if(inst->verbose >=2) printf("[*] Command line arguments parsed.\n");

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
        printf("ERROR: Check mandatory arguments!\n");
        exit(1);
    }
}

void parse_tsp_file(instance *inst){
    if(inst->verbose >=2) printf("[*] Parsing tsp file...\n");
    // put default instance values
    inst->tot_nodes = -1;
    inst->xcoord = inst->ycoord = NULL;

    // open file
    FILE *fin = fopen(inst->input_file_name, "r");
    if (fin == NULL ){
        printf("ERROR: input file %s not found!", inst->input_file_name);
        free_instance(inst);
        exit(1);}

    // parse file
    char line[256];
    char *param_name, *param;
    char done = 0;
    while(!done){
        if(fgets(line, sizeof(line), fin) == NULL){
            printf("ERROR: EOF not found!\n");
            free_instance(inst);
            exit(1);
        }
        if(inst->verbose >= 3) printf("line: %s", line);
        param_name = strtok(line, ": \n");
        if(inst->verbose >= 3) printf("param_name: %s\n", param_name);

        if(strncmp(param_name, "NAME", 4) == 0) continue;

        if(strncmp(param_name, "TYPE", 4) == 0){
            param = strtok(NULL, ": \n");
            if(strncmp(param, "TSP",3) != 0){
                printf("ERROR: TYPE %s not supported yet.", param);
                free_instance(inst);
                exit(1);
            }
            continue;
        }

        if(strncmp(param_name, "COMMENT", 7) == 0) continue;

        if(strncmp(param_name, "DIMENSION", 9) == 0){
            param = strtok(NULL, ": \n");
            inst->tot_nodes = atoi(param);
            continue;
        }

        if(strncmp(param_name, "EDGE_WEIGHT_TYPE", 16) == 0){
            param = strtok(NULL, ": \n");
            if(strncmp(param, "ATT", 3) != 0){
                printf("ERROR: EDGE_WEIGHT_TYPE %s not supported yet.", param);
                free_instance(inst);
                exit(1);
            }
            continue;
        }

        if(strncmp(param_name, "NODE_COORD_SECTION", 18) == 0) {
            if(inst->tot_nodes <= 0){
                printf("ERROR: Can't allocate %d nodes!", inst->tot_nodes);
                free_instance(inst);
                exit(1);
            }
            inst->xcoord = (double *) calloc(inst->tot_nodes, sizeof(double));
            inst->ycoord = (double *) calloc(inst->tot_nodes, sizeof(double));

            for(int n = 0; n < inst->tot_nodes; n++){
                if((fgets(line, sizeof(line), fin) == NULL) || (strncmp(line, "EOF", 3) == 0)){
                    printf("ERROR: Too few nodes! Expected %d, got %d\n", inst->tot_nodes, n);
                    free_instance(inst);
                    exit(1);
                }
                if(atoi(strtok(line, " ")) != n+1){
                    printf("ERROR: Nodes must be ordered!\n");
                    free_instance(inst);
                    exit(1);
                }
                inst->xcoord[n] = atof(strtok(NULL, " "));
                inst->ycoord[n] = atof(strtok(NULL, " "));
            }
            continue;
        }

        if(strncmp(param_name, "EOF", 3) == 0) done = 1;
    }
    if(inst->verbose >=2) printf("[*] Tsp file parsed.\n");
    if(inst->verbose >= 3){
        printf("Nodes found:\n");
        for(int i=0; i < inst->tot_nodes; i++)
            printf("n%d = (%f, %f)\n", i+1, inst->xcoord[i], inst->ycoord[i]);
    }
    fclose(fin);
}

int main(int argc, char **argv){
    instance inst;
    parse_command_line(argc, argv, &inst);
    parse_tsp_file(&inst);
    free_instance(&inst); // release memory!
}
