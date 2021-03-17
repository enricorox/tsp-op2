#include "utils.h"
#include "tsp.h"

int main(int argc, char **argv){
    instance inst;
    init_instance(&inst);

    parse_command_line(argc, argv, &inst);
    parse_tsp_file(&inst,0);
    if(inst.input_opt_file_name != NULL) parse_tsp_file(&inst, 1);

    char *rxstar = TSPOpt(&inst);
    plot(&inst, rxstar);

    // release memory!
    free(rxstar);
    free_instance(&inst);
    return 0;
}
