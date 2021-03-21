#include <unistd.h>

#include "utils.h"
#include "tsp.h"
#include "performance.h"

int main(int argc, char **argv){
    // define and initialize general instance
    instance inst;
    init_instance(&inst);
    // parse command line
    parse_cli(argc, argv, &inst);

    if(inst.perf)
        start_perf_test(); // start formulations performance test
    else {
        // parse .tsp file
        parse_file(&inst, inst.input_tsp_file_name);
        // parse .opt.tsp file
        if (inst.input_opt_file_name != NULL) parse_file(&inst, inst.input_opt_file_name);
        // start optimization
        TSPOpt(&inst);
        // plot
        if (inst.do_plot && inst.xstar != NULL) plot(&inst, inst.xstar);
    }

    // release memory!
    free_instance(&inst);
    return 0;
}
