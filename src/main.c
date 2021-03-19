#include <unistd.h>
#include "utils.h"
#include "tsp.h"

int main(int argc, char **argv){
    instance inst;
    init_instance(&inst);

    parse_cli(argc, argv, &inst);
    parse_file(&inst, inst.input_tsp_file_name);
    if(inst.input_opt_file_name != NULL) parse_file(&inst, inst.input_opt_file_name);

    char *rxstar = TSPOpt(&inst);

    if(inst.do_plot) plot(&inst, rxstar);

    // release memory!
    free(rxstar);
    free_instance(&inst);
    return 0;
}
