#include <string.h>
#include "tsp.h"

#define USAGE   "./tsp [options]\n"\
                "--file <input-file>                file in tsp format\n"\
                "--time-limit <time>                max overall time in seconds\n"

void debug(const char *err) { printf("\nDEBUG: %s \n", err); }
void print_error(const char *err) { printf("\n\n ERROR: %s \n\n", err); exit(1); }

void parse_command_line(int argc, char** argv, instance *inst){
    // put default instance value
    inst->input_file_name = NULL;
    inst->time_limit = -1;

    // parse cli
    char help = (argc > 2) ? 0 : 1;
    for(int i = 1; i < argc; i++){
        if ( strcmp(argv[i],"--file") == 0 ) { inst->input_file_name = argv[++i]; continue; }
        if ( strcmp(argv[i],"--time-limit") == 0 ) { inst->time_limit = atof(argv[++i]); continue; }
        if ( strcmp(argv[i],"--help") == 0 ) { help = 1; continue; }
        printf("Unknown option %s\n", argv[i]);
        help = 1; // need to show help if came here
    }
    if(help) { printf(USAGE); exit(1); }

    if(VERBOSE > 10){
        printf("Command line arguments found (include defaults):\n");
        printf("--file          %s\n", inst->input_file_name);
        printf("--time-limit    %f\n", inst->time_limit);
    }
}

void parse_input_file(instance *inst){
    // put default instance values
    inst->xcoord = inst->ycoord = NULL;

}

void free_instance(instance *inst){
    free(inst->xcoord);
    free(inst->ycoord);
}

int main(int argc, char **argv){
    instance inst;
    parse_command_line(argc, argv, &inst);
    parse_input_file(&inst);
    free_instance(&inst); // release occupied memory!
}
