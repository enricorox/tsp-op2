//
// Created by enrico on 21/03/21.
//

#include <unistd.h>
#include <float.h>
#include "performance.h"
#include "heuristics.h"
#include "parsers.h"

#define MAX_COORD 10000

double ** generate_points(int seed, int size){
    if(seed != 0) srand(seed);
    double **points = (double **) malloc(2 * sizeof(double *));
    points[0] = (double *) malloc(size * sizeof(double));
    points[1] = (double *) malloc(size * sizeof(double));

    for(int i = 0; i < size; i++){
        points[0][i] = rand() % MAX_COORD;
        points[1][i] = rand() % MAX_COORD;
    }

    return points;
}

double *** generate_points_(int stop, int step){
    srand(16); // assure reproducibility
    int num_p = 2 * (1 + stop / step) * stop / step;
    printf(BOLDGREEN "[INFO] Generating %d random points...\n" RESET, num_p);
    double ***points;

    // allocate memory for pointers
    int start = step;
    int sets = stop / start;
    points = (double ***) malloc(sets * sizeof(double **));
    for(int i = 0; i < sets; i++) {
        // allocate memory for pointers again (xcoord and ycoord)
        points[i] = malloc(2 * sizeof(double *));
        points[i][0] = malloc((start + step * i) * sizeof(double));
        points[i][1] = malloc((start + step * i) * sizeof(double));
        for(int k = 0; k < start + step * i ; k++) {
            points[i][0][k] = rand();
            points[i][1][k] = rand();
        }
    }
    return points;
}

void free_points_(double ***points, int stop, int step){
    int start = step;
    int sets = stop/start;
    for(int i = 0; i < sets; i++){
        //printf("free(): points[%d][0] = %p\n", i, points[i][0]);
        free(points[i][0]);
        //printf("free(): points[%d][1] = %p\n", i, points[i][1]);
        free(points[i][1]);
        free(points[i]);
    }
    free(points);
}

void print_points_(double ***points, int stop, int step){
    int start = step;
    int sets = stop / start;
    for(int i = 0; i < sets; i++) {
        printf("======== begin of set %d ========\n", i + 1);
        for (int k = 0; k < start + step * i; k++)
            printf("p(%2d) = (%10.0f,%10.0f)\n", k + 1, points[i][0][k], points[i][1][k]);
    }
}

void set_instance_formulation(instance *inst, enum formulation_t form, bool lazy){
    inst->formulation = form;
    inst->lazy = lazy;
}

void set_instance_(instance *user_inst, instance *inst, double ***points, int stop, int step, int set){
    int start = step;
    if(set < 0 || set > stop / start){
        printf(BOLDRED "[ERROR] set_instance(): illegal argument\n");
        free_points_(points, start, stop);
        exit(1);
    }
    char iname[BUFLEN];
    char cname[BUFLEN];

    snprintf(iname, BUFLEN, "inst%d", inst->nnodes);
    snprintf(cname, BUFLEN, "perf test");
    inst->name[0] = strdup(iname);
    inst->comment[0] = strdup(cname);

    inst->nnodes = start + step * set;
    inst->xcoord = points[set][0];
    inst->ycoord = points[set][1];

    inst->verbose = 0;
    inst->gui = false;
    // set time-limit if not set
    inst->time_limit = (user_inst->time_limit < CPX_INFBOUND) ? user_inst->time_limit : 3600; // 1h time limit!
    inst->do_plot = user_inst->do_plot;
    inst->integer_costs = user_inst->integer_costs;
}

void set_instance(instance *inst, int size, double time_limit, int ntest, int ninst){
    inst->nnodes = size;
    double **points = generate_points(0, size);
    inst->xcoord = points[0];
    inst->ycoord = points[1];

    char name[BUFLEN];
    char comment[BUFLEN];
    snprintf(name, BUFLEN, "t%d-s%d-n%d", ntest, size, ninst);
    snprintf(comment, BUFLEN, "perf test");
    inst->name[0] = strdup(name);
    inst->comment[0] = strdup(comment);

    inst->verbose = 0;
    inst->gui = false;
    inst->do_plot = false;
    inst->time_limit = time_limit;
    inst->mem_limit = 12000;
}

void reset_instance(instance *dummy_inst){
    CPXfreeprob(dummy_inst->CPXenv, &dummy_inst->CPXlp);
    CPXcloseCPLEX(&dummy_inst->CPXenv);

    free(dummy_inst->xstar);
    dummy_inst->xstar = NULL;

    free(dummy_inst->xbest);
    dummy_inst->xbest = NULL;

    dummy_inst->zstar = CPX_INFBOUND;

    dummy_inst->zbest = CPX_INFBOUND;

    dummy_inst->runtime = -1;

    dummy_inst->status = -1;
}

// number of random instances
#define NINSTANCES 2//20

// compact models
#define NNODES1 60
#define TLIMIT1 1000

// cuts & benders
#define NNODES2 500
#define TLIMIT2 1000

// math-heuristic
#define NNODES3 1000
#define TLIMIT3 1000

// heuristics
#define NNODES4 2000
#define TLIMIT4 1000

void test(instance *user_inst){
    // the instance to solve
    instance dummy_inst;
    init_instance(&dummy_inst);

    // use the random seed that coincide with test number:
    // thus different tests have different instances!
    srand(user_inst->test);

    // open comma separated value file for storing times or approximations!
    char filename[BUFLEN];
    snprintf(filename, BUFLEN, "../test/times-%d.csv", user_inst->test);
    FILE *times = fopen(filename, "a");

    if(times == NULL)
        printerr(user_inst, "Cannot find %s", filename);

    switch(user_inst->test){
        case 1: // compact models
        {
            // print first line for performance profile
            bool lazy[] = {false, true};
            enum formulation_t formulations[] = {GG, MTZ};
            fprintf(times, "4,");
            for (int l = 0; l < 2; l++)
                for (int f = 0; f < 2; f++) {
                    fprintf(times, "%s%s,", formulation_names[formulations[f]], lazy[l] ? "-lazy" : "");
                }
            fprintf(times, "\n");

            for (int i = 0; i < NINSTANCES; i++) {
                // generate and assign points and parameters
                set_instance(&dummy_inst, NNODES1, TLIMIT1, user_inst->test, i + 1);
                print(user_inst, 'I', 1, "Generating instance #%d with %d nodes", i + 1, dummy_inst.nnodes);
                fprintf(times, "%s,", dummy_inst.name[0]);

                for (int l = 0; l < 2; l++)
                    for (int f = 0; f < 2; f++) {
                        print(user_inst, 'I', 1, "Executing %s%s...", formulation_names[formulations[f]],
                              lazy[l] ? " lazy" : "");
                        dummy_inst.lazy = lazy[l];
                        dummy_inst.formulation = formulations[f];

                        TSPOpt(&dummy_inst);
                        fprintf(times, "%ld,", dummy_inst.runtime);
                        print(user_inst, 'I', 1, "Runtime = %ld", dummy_inst.runtime);
                        reset_instance(&dummy_inst);
                    }
                fprintf(times, "\n");
                free_instance(&dummy_inst);
            }
        }
            break;
        case 2: // cuts & benders
        {
            // print first line for performance profile
            enum formulation_t formulations[] = {CUTS1, BENDERS};
            fprintf(times, "2,");
            for (int l = 0; l < 2; l++)
                for (int f = 0; f < 2; f++) {
                    fprintf(times, "%s,", formulation_names[formulations[f]]);
                }
            fprintf(times, "\n");

            for (int i = 0; i < NINSTANCES; i++) {
                // generate and assign points and parameters
                set_instance(&dummy_inst, NNODES2, TLIMIT2, user_inst->test, i + 1);
                print(user_inst, 'I', 1, "Generating instance #%d with %d nodes", i + 1, dummy_inst.nnodes);
                fprintf(times, "#%d,", i + 1);

                for (int f = 0; f < 2; f++) {
                    print(user_inst, 'I', 1, "Executing %s...", formulation_names[formulations[f]]);
                    dummy_inst.formulation = formulations[f];

                    TSPOpt(&dummy_inst);
                    fprintf(times, "%ld,", dummy_inst.runtime);
                    print(user_inst, 'I', 1, "Runtime = %ld", dummy_inst.runtime);
                    reset_instance(&dummy_inst);
                }
                fprintf(times, "\n");
                free_instance(&dummy_inst);
            }
        }
            break;
        case 3: // math-heuristic
        {
            // print first line for performance profile
            enum formulation_t formulations[] = {HFIXING1, SFIXING1};
            fprintf(times, "2,");
            for (int l = 0; l < 2; l++)
                for (int f = 0; f < 2; f++) {
                    fprintf(times, "%s,", formulation_names[formulations[f]]);
                }
            fprintf(times, "\n");

            for (int i = 0; i < NINSTANCES; i++) {
                // generate and assign points and parameters
                set_instance(&dummy_inst, NNODES3, TLIMIT3, user_inst->test, i + 1);
                save_instance_to_tsp_file(&dummy_inst);
                print(user_inst, 'I', 1, "Generating instance #%d with %d nodes", i + 1, dummy_inst.nnodes);
                fprintf(times, "#%d,", i + 1);

                for (int f = 0; f < 2; f++) {
                    print(user_inst, 'I', 1, "Executing %s...", formulation_names[formulations[f]]);
                    dummy_inst.formulation = formulations[f];
                    TSPOpt(&dummy_inst);
                    fprintf(times, "%f,", dummy_inst.zbest);
                    print(user_inst, 'I', 1, "zbest = %f", dummy_inst.zbest);
                    reset_instance(&dummy_inst);
                }
                fprintf(times, "\n");
                free_instance(&dummy_inst);
            }
        }
            break;
        case 4: // heuristics
        {
            // print first line for performace profile
            fprintf(times,
                    "4, greedy+vns, greedy+tabu, extra-mileage+vns, extra-mileage+tabu\n"); // TODO add variations

            // customize the instance
            dummy_inst.time_limit = TLIMIT4;
            dummy_inst.nnodes = NNODES4;
        }
            break;
        default: // custom test
            if(user_inst->formulation == FLAST && user_inst->cons_heuristic == CHLAST)
                printerr(user_inst, "Heuristic or formulation not defined!");

            // set random instance seed
            srand(user_inst->perfr);

            // set instance solution models
            dummy_inst.formulation = user_inst->formulation;
            dummy_inst.cons_heuristic = user_inst->cons_heuristic;
            dummy_inst.ref_heuristic = user_inst->ref_heuristic;
            // generate random points, set options...
            set_instance(&dummy_inst, user_inst->size, user_inst->time_limit, user_inst->test, user_inst->perfr);
            if(user_inst->input_tsp_file_name != NULL)
                parse_file(&dummy_inst, user_inst->input_tsp_file_name);
            if(user_inst->formulation != FLAST)
                TSPOpt(&dummy_inst);
            else
                heuristic(&dummy_inst);


            if(user_inst->formulation < HFIXING1){ // print time
                print(user_inst, 'I', 1, "runtime = %ld", dummy_inst.runtime);
                fprintf(times, "%ld, ", dummy_inst.runtime);
            }else{ // print approx
                fprintf(times, "%f, ", dummy_inst.zbest);
                print(user_inst, 'I', 1, "zbest = %f", dummy_inst.zbest);
            }
    }

    // close file
    fclose(times);
}

void perprof(instance *dummy_inst, instance *user_inst){
    // use performance profile TODO
    char command[BUFLEN];
    snprintf(command, BUFLEN, "python2 ../perfprof.py -D , -T %f -S 2 -M 20 -P \"test %d, shift 2 sec.s\" ../test/times-%d.csv ../test/test-%d.pdf",
             dummy_inst->time_limit, user_inst->test, user_inst->test, user_inst->test);
    print(user_inst, 'I', 1, "Executing %s", command);
    system(command);
}