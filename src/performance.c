//
// Created by enrico on 21/03/21.
//

#include <unistd.h>
#include <float.h>
#include "performance.h"

#define MAX_COORD 10000

double ** generate_points(int seed, int size){
    srand(seed);
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

void set_instance(instance *user_inst, instance *inst, int seed, int size){
    char iname[BUFLEN];
    char cname[BUFLEN];

    inst->nnodes = size;
    double **points = generate_points(seed, size);
    inst->xcoord = points[0];
    inst->ycoord = points[1];

    snprintf(iname, BUFLEN, "instSize%dSeed%d", inst->nnodes, seed);
    snprintf(cname, BUFLEN, "perf test");
    inst->name[0] = strdup(iname);
    inst->comment[0] = strdup(cname);

    inst->verbose = 0;
    inst->gui = false;
    // set time-limit if not set
    inst->time_limit = (user_inst->time_limit < CPX_INFBOUND) ? user_inst->time_limit : 3600; // 1h time limit!
    inst->do_plot = user_inst->do_plot;
    inst->integer_costs = user_inst->integer_costs;
}

void start_perf_test(instance *user_inst){
    int max = user_inst->perfr;
    if(max < 4){
        printerr(user_inst, "<max> must be >4!");
    }
    if(max > 88){
        if(max > 10000){
            printerr(user_inst, "Too much nodes!");
        }
        printf(BOLDRED "[WARN] %d points may be too much for your computer!\n", max);
        printf(BOLDRED "[WARN] Press CTR+C or wait 10s...\n" RESET);
        sleep(10); // script-friendly timer
    }

    print(user_inst, 'I', 1, "Starting performance mode...");
    instance dummy_inst;
    init_instance(&dummy_inst);

    // open comma separated value file for storing values!
    char filename[BUFLEN];
    snprintf(filename, BUFLEN, "times%d.csv", user_inst->seed);
    FILE *values = fopen(filename,"w");
    fprintf(values, "%d,", (FLAST-1) * 2);
    for(int i = BENDERS + 1; i < FLAST; i++)
        for(int lazy = 0; lazy < ((i >= MTZ)?2:1); lazy++)
            fprintf(values, "%s %s,", formulation_names[i], lazy?"lazy":"");

    for(int set = 0; set < user_inst->runs; set++) { // change set
        print(user_inst, 'I', 1, "==== Entering set %d/%d of size %d ====", set + 1, user_inst->runs, user_inst->perfr);
        set_instance(user_inst, &dummy_inst, user_inst->seeds[set], user_inst->perfr);
        save_instance_to_tsp_file(&dummy_inst);
        fprintf(values, "\n%d,", dummy_inst.nnodes);
        fclose(values);
        values = fopen(filename, "a");
        for (enum formulation_t form = 0; form < FLAST; form++) {
            bool compact = false;
            if(form == MTZ || form == GG || form == GGi)
                compact = true;
            for (int lazy = 0; lazy < (compact?2:1); lazy++) { // add lazy constraints
                set_instance_formulation(&dummy_inst, form, lazy);
                if(user_inst->verbose >= 2) save_instance_to_tsp_file(&dummy_inst);
                TSPOpt(&dummy_inst);
                fprintf(values, "%ld,", dummy_inst.runtime);
                print(user_inst, 'I', 1, "%7s %4s: %ld seconds",
                       formulation_names[form], lazy?"lazy":"", dummy_inst.runtime);

                // cleaning resources
                free(dummy_inst.xstar);

                // requested by TSPOpt
                dummy_inst.xstar = NULL;
            }
        }
        //free(dummy_inst.name[0]);
        //free(dummy_inst.comment[0]);
        free_instance(&dummy_inst);
    }

    fclose(values);
    printf(BOLDGREEN "[INFO] Test finished. Times saved to %s\n" RESET, filename);
    // freeing memory
    //free_points(points, max, step);
    // do not call free_instance(): double free!
}