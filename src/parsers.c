//
// Created by enrico on 23/03/21.
//

#include "parsers.h"

void parse_cli(int argc, char **argv, instance *inst){
    // parse cli
    bool help = false;
    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i],"--file") == 0){
            if(argv[++i] != NULL)
                inst->input_tsp_file_name = strdup(argv[i]) ;
            continue;
        }
        if(strcmp(argv[i],"--opt-tour") == 0){
            if(argv[++i] != NULL)
                inst->input_opt_file_name = strdup(argv[i]);
            continue;
        }
        if(strncmp(argv[i],"--formulation", 6) == 0){
            if(argv[++i] != NULL) {
                bool found = false;
                for(int k = 0; k < FLAST; k++)
                    if (strcasecmp(argv[i], formulation_names[k]) == 0) {
                        inst->formulation = k;
                        found = true;
                        break;
                    }
                if(!found)
                    printf(BOLDRED "[WARN] Unknown formulation: using default\n" RESET);

            }
            continue;
        }
        if(strncmp(argv[i],"--constructive-heuristic", 3) == 0){
            if(argv[++i] != NULL) {
                bool found = false;
                for(int k = 0; k < CHLAST; k++)
                    if (strcasecmp(argv[i], cons_heuristic_names[k]) == 0) {
                        inst->cons_heuristic = k;
                        found = true;
                        break;
                    }
                if(!found)
                    printerr(inst, "Unknown constructive heuristic");
            }
            continue;
        }
        if(strncmp(argv[i],"--refinement-heuristic", 3) == 0){
            if(argv[++i] != NULL) {
                bool found = false;
                for(int k = 0; k < RHLAST; k++)
                    if (strcasecmp(argv[i], ref_heuristic_names[k]) == 0) {
                        inst->ref_heuristic = k;
                        found = true;
                        break;
                    }
                if(!found)
                    printerr(inst, "Unknown refinement heuristic");

            }
            continue;
        }
        if(strcmp(argv[i],"--seed") == 0){
            if(argv[++i] != NULL)
                inst->seed = atoi(argv[i]);
            continue;
        }
        if(strcmp(argv[i],"--test") == 0){
            if(argv[++i] != NULL)
                inst->test = atoi(argv[i]);
            continue;
        }
        if(strcmp(argv[i],"--lazy")  == 0){ inst->lazy = true; continue;}
        if(strcmp(argv[i],"--time-limit") == 0){
            if(argv[++i] != NULL){
                inst->time_limit = atof(argv[i]);
                if(inst->time_limit < 10) {
                    printf(BOLDRED "[WARN] Time limit may be too low!\n" RESET);
                    sleep(5);
                }
            }
            continue;
        }
        if(strcmp(argv[i],"--mem-limit") == 0){
            if(argv[++i] != NULL) {
                inst->mem_limit = atoi(argv[i]);
                if (inst->mem_limit <= 0) {
                    printerr(inst, "Memory limit must be positive!");
                }
            }
            continue;
        }
        if(strcmp(argv[i],"--no-gui") == 0){ inst->gui = false; continue;}
        if(strcmp(argv[i],"--no-plot") == 0){ inst->do_plot = false; continue;}
        if(strcmp(argv[i],"--no-int-costs") == 0){ inst->integer_costs = false; continue;}
        if(strcmp(argv[i], "--verbose") == 0){
            if(argv[++i] != NULL)
                inst->verbose = atoi(argv[i]);
            continue;
        }
        if(strcmp(argv[i],"--perfr") == 0){
            if(argv[++i] != NULL)
                inst->perfr = atoi(argv[i]);
            continue;
        }
        if(strcmp(argv[i],"--perfl") == 0){
            if(argv[++i] != NULL)
                inst->perfl = strdup(argv[i]);
            continue;
        }
        if(strcmp(argv[i],"--size") == 0){
            if(argv[++i] != NULL)
                inst->size = atoi(argv[i]);
            continue;
        }
        if(strcmp(argv[i],"--seeds") == 0){
            if(inst->size < 0)
                printerr(inst, "You need to specify --runs before --seeds!");
            inst->seeds = (int *) malloc(inst->size * sizeof(int));
            int k;
            for(k = 0; k < inst->size; k++)
                if(argv[++i] != NULL)
                    inst->seeds[k] = atoi(argv[i]);
                else
                    printerr(inst, "Got %d seeds, expected %d", k, inst->size);
            continue;
        }

        if(strcmp(argv[i],"--help") == 0) { help = 1; break; }

        printf(BOLDRED "[ERROR] Unknown option: %s\n" RESET, argv[i]);
        help = true; // need to show help if came here
    }

    if(inst->verbose >=1) printf(BOLDGREEN "[INFO] Command line arguments parsed.\n" RESET);

    // print arguments
    if(inst->verbose >= 2){
        if(inst->verbose >= 3 && !help)
            printf(USAGE);

        printf("Command line arguments found (include defaults):\n");
        printf("--file                      %s\n", inst->input_tsp_file_name);
        printf("--opt-tour                  %s\n", inst->input_opt_file_name);
        printf("--formulation               %s\n", formulation_names[inst->formulation]);
        printf("--constructive-heuristic    %s\n", cons_heuristic_names[inst->cons_heuristic]);
        printf("--refinement-heuristic      %s\n", ref_heuristic_names[inst->ref_heuristic]);
        printf("--seed                      %d\n", inst->seed);
        printf("--lazy                      %s\n", inst->lazy?"true":"false");
        printf("--time-limit                %f\n", inst->time_limit);
        printf("--mem-limit                 %f\n", inst->mem_limit);
        printf("--no-gui                    %s\n", inst->gui?"false":"true");
        printf("--no-plot                   %s\n", inst->do_plot?"false":"true");
        printf("--no-int-costs              %s\n", inst->integer_costs?"false":"true");
        printf("--perfr                     %d\n", inst->perfr);
        printf("--perfl                     %s\n", inst->perfl);
        printf("--size                      %d\n", inst->size);
        printf("--verbose                   %d\n", inst->verbose);
    }

    if(help){
        printf(USAGE);
        exit(1);
    }

    if(inst->input_tsp_file_name == NULL && !inst->perfr && !inst->perfl && !inst->test){
        printf(BOLDRED "[ERROR] Check mandatory arguments: ./tsp --help\n" RESET);
        free_instance(inst);
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
/*
    else{
        printf(BOLDRED "[ERROR] parse_file(): Illegal argument\n" RESET);
        free_instance(inst);
        exit(1);
    }
*/
    // open file
    FILE *fin = fopen(file_name, "r");
    if (fin == NULL ){
        printf(BOLDRED "[ERROR] input file %s not found!\n" RESET, opt?inst->input_opt_file_name:inst->input_tsp_file_name);
        free_instance(inst);
        exit(1);
    }

    // parse file
    char line[BUFLEN];
    char *param_name, *param;
    while(1){
        // check for error & read line
        if(fgets(line, BUFLEN, fin) == NULL){
            printf(BOLDRED "[WARN] EOF not found!\n" RESET);
            break;
        }

        // debug output
        if(inst->verbose >= 3) printf("[DEBUG] Reading line: %s", line);

        // tokenize line
        param_name = strtok(line, ": \n\r");
        param = strtok(NULL, ": \n\r");

        // --------- compare strings --------- //
        if(strcmp(param_name, "") == 0) continue; // ignore empty lines

        if(strncmp(param_name, "NAME", 4) == 0) {
            int idx = opt?1:0;
            inst->name[idx] = strdup(param);
            if(inst->verbose >=2) printf("NAME = %s\n", inst->name[idx]);
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
            int idx = opt?1:0;
            // revert tokenization!
            if(strtok(NULL,"\n") != NULL) param[strlen(param)] = ' ';
            inst->comment[idx] = strdup(param);
            if(inst->verbose >=2) printf("COMMENT = %s\n", inst->comment[idx]);
            continue;
        }

        if(strncmp(param_name, "DIMENSION", 9) == 0){
            int tot_nodes = atoi(param);
            if(inst->nnodes > 0 && tot_nodes != inst->nnodes){
                printf(BOLDRED "[ERROR] Different dimensions found!\n" RESET);
                free_instance(inst);
                exit(1);
            }

            inst->nnodes = tot_nodes;

            if(inst->verbose >=2) printf("DIMENSION = %d\n", inst->nnodes);
            if(inst->nnodes <= 0) {
                printf("[ERROR] Cannot allocate %d nodes!\n", inst->nnodes);
                exit(1);
            }
            continue;
        }

        if(strncmp(param_name, "EDGE_WEIGHT_TYPE", 16) == 0){ // Assuming it is EUC_2D
            if(inst->verbose >=2) printf("EDGE_WEIGHT_TYPE = %s\n", param);

            if(strncmp(param, "EUC_2D", 6) == 0){ inst->dist = EUC_2D; continue; }

            if(strncmp(param, "ATT", 3) == 0) { inst->dist = ATT; continue; }

            if(strncmp(param, "GEO", 3) == 0) { inst->dist = GEO; continue; }

            printf(BOLDRED "[WARN] EDGE_WEIGHT_TYPE = %s is not supported yet: using default EU_2D.\n" RESET, param);
            continue;
        }

        if(strncmp(param_name, "NODE_COORD_SECTION", 18) == 0) {
            // check if nnodes is given
            if (inst->nnodes <= 0) {
                printf("[ERROR] DIMENSION must be before NODE_COORD_SECTION!\n");
                free_instance(inst);
                exit(1);
            }
            if (inst->verbose >= 2) printf("NODE_COORD_SECTION:\n");
            // allocate arrays
            inst->xcoord = (double *) calloc(inst->nnodes, sizeof(double));
            inst->ycoord = (double *) calloc(inst->nnodes, sizeof(double));
            if (inst->xcoord == NULL || inst->ycoord == NULL){
                printf(BOLDRED "[ERROR] Can't allocate memory: too many nodes (nnodes = %d)!\n" RESET, inst->nnodes);
                free_instance(inst);
                exit(1);
            }
            int node_number;
            // read nodes
            for(int n = 0; n < inst->nnodes; n++){
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
            // check if nnodes is given
            if (inst->nnodes <= 0) {
                printf(BOLDRED "[ERROR] DIMENSION must be before TOUR_SECTION!\n" RESET);
                free_instance(inst);
                exit(1);
            }
            if (inst->verbose >= 2) printf("TOUR_SECTION:\n");
            // allocate arrays
            inst->opt_tour = (int *) calloc(inst->nnodes, sizeof(double));
            bool *duplicates = (bool *) calloc(inst->nnodes, sizeof(bool));
            if (inst->xcoord == NULL || inst->ycoord == NULL){
                printf(BOLDRED "[ERROR] Can't allocate memory: too many nodes (nnodes = %d)!\n" RESET, inst->nnodes);
                free_instance(inst);
                exit(1);
            }
            // read nodes
            int node;
            for(int n = 0; n < inst->nnodes; n++){
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
                if(node > inst->nnodes){
                    printf(BOLDRED "[ERROR] Illegal node: %d\n" RESET, node);
                    free(duplicates);
                    free_instance(inst);
                    exit(1);
                }
                if(duplicates[node-1]){
                    printf(BOLDRED "[ERROR] That's not a tour: node %d is not unique!\n" RESET, node);
                    free(duplicates);
                    free_instance(inst);
                    exit(1);
                }
                duplicates[node-1] = true;
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
        printf(BOLDRED "[WARN] param_name = %s is unknown: ignored.\n" RESET, param_name);
    }
    if(inst->verbose >=1) printf(BOLDGREEN "[INFO] File %s parsed.\n" RESET, file_name);

    fclose(fin);
}

/**
 * Don't trust name field, compute from filename
 * @param inst
 * @return inst->input_opt_file_name or NULL if fail
 */
char * find_opt_file(instance *inst){
    // respect user choice
    if(inst->input_opt_file_name != NULL) return inst->input_opt_file_name;

    char *tsp_filename = strdup(inst->input_tsp_file_name);
    if(strlen(tsp_filename) < 4) return NULL; // ".tsp" cannot appear

    // allocate memory for new name
    char *opt_filename = (char *) malloc(BUFLEN);

    // find prefix
    for(int i = 0; tsp_filename[i] != '\0'; i++){
        if(tsp_filename[i] == '.' && tsp_filename[i + 1] == 't' &&
           tsp_filename[i + 2] == 's' && tsp_filename[i + 3] == 'p'){
                tsp_filename[i] = '\0';
                break;
        }
    }

    // build name
    snprintf(opt_filename, BUFLEN,"%s.opt.tour", tsp_filename);

    // free memory
    free(tsp_filename);
    if(!exist(opt_filename)){
        if(inst->verbose >=2) printf(BOLDRED "[WARN] %s file not found!\n", opt_filename);
        free(opt_filename);
        return NULL;
    }
    inst->input_opt_file_name = opt_filename;
    return opt_filename;
}

/**
 * Read a list file
 * @param filename
 * @param tspfiles return a list of tsp filename
 * @param nfiles length of above array
 * @param seeds return a list of seeds
 * @param nseeds length of above array
 *
 * File template:
 *
 * NSEEDS=5
 * 1
 * 2
 * 3
 * 4
 * 5
 * NFILES=2
 * a.tsp
 * b.tsp
 */
void parse_file_list(const char *filename, char **tspfiles, int *nfiles, int **seeds, int *nseeds){
    if(!exist(filename)){
        printf("[ERROR] Can't open file %s\n", filename);
        exit(1);
    }

    FILE *fin = fopen(filename, "r");
    char line[BUFLEN];
    char section=0; // 1=SEEDS, 2=FILES
    while(fgets(line, BUFLEN, fin)){
        if(strncmp(line, "NSEEDS=", 7) == 0){
            int len = atoi(line + 7);
            int *s = calloc(len, sizeof(int));
            for(int i = 0; i < len; i++){
                fgets(line, BUFLEN, fin);
                s[i] = atoi(line);
            }
            *nseeds = len;
        }
        if(strncmp(line, "NFILES=", 7) == 0){
            int len = atoi(line + 7);
            char **s = malloc(len);
            for(int i = 0; i < len; i++){
                fgets(line, BUFLEN, fin);
                s[i] = strdup(line);
            }
            *nfiles = len;
        }
    }
    fclose(fin);
}

bool exist(const char *file){
    if(file == NULL) return false;
    FILE * f = fopen(file, "r");
    if(f == NULL) return false;
    fclose(f);
    return true;
}