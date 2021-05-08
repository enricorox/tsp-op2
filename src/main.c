#include <unistd.h>

#include "utils.h"
#include "tsp.h"
#include "performance.h"
#include "parsers.h"
#include "heuristics.h"

int main(int argc, char **argv){
    // define and initialize general instance
    instance inst;
    init_instance(&inst);

    // parse command line
    parse_cli(argc, argv, &inst);

    if(inst.perfr || inst.perfl != NULL)
        start_perf_test(&inst); // start formulations performance test
    else {
        // parse tsp file
        parse_file(&inst, inst.input_tsp_file_name);
        // parse .opt.tsp file
        if (!inst.no_opt){
            if(find_opt_file(&inst) != NULL) {
                parse_file(&inst, inst.input_opt_file_name);
            }
        }
        if(inst.heuristic != HLAST){
            print(&inst, 'I', 1, "Solving %s with %s heuristic", inst.name[0], heuristic_names[inst.heuristic]);
            heuristic(&inst);
        }else {
            print(&inst, 'I', 1, "Solving %s with %s formulation", inst.name[0], formulation_names[inst.formulation]);
            // start optimization
            TSPOpt(&inst);
        }
        // plot
        if (inst.do_plot) {
            if(inst.xstar != NULL)
                plot(&inst, inst.xstar);
            else if(inst.xbest != NULL)
                plot(&inst, inst.xbest);
        }
    }

    // release memory!
    free_instance(&inst);
    return 0;
}
