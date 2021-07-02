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

    if(inst.test != 0)
        test(&inst);
    else {
        // parse tsp file
        parse_file(&inst, inst.input_tsp_file_name);
        // parse .opt.tsp file
        if (!inst.no_opt){
            if(find_opt_file(&inst) != NULL) {
                parse_file(&inst, inst.input_opt_file_name);
            }
        }
        if(inst.cons_heuristic != CHLAST){
            print(&inst, 'I', 1, "Solving %s with %s constructive and %s refinement heuristic",
                  inst.name[0], cons_heuristic_names[inst.cons_heuristic],
                  (inst.ref_heuristic != RHLAST)?ref_heuristic_names[inst.ref_heuristic]:"none");
            heuristic(&inst);
        }else {
            print(&inst, 'I', 1, "Solving %s with %s formulation", inst.name[0], formulation_names[inst.formulation]);
            // start optimization
            TSPOpt(&inst);
        }

    }

    // release memory!
    free_instance(&inst);
    return 0;
}
