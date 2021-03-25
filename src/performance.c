//
// Created by enrico on 21/03/21.
//

#include <unistd.h>
#include "performance.h"

double *** generate_points(int stop, int step){
    srand(1); // assure reproducibility
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

void free_points(double ***points, int stop, int step){
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

void print_points(double ***points, int stop, int step){
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

void set_instance(instance *user_inst, instance *inst, double ***points, int stop, int step, int set){
    int start = step;
    if(set < 0 || set > stop / start){
        printf(BOLDRED "[ERROR] set_instance(): illegal argument\n");
        free_points(points, start, stop);
        exit(1);
    }
    char iname[BUFLEN];
    char cname[BUFLEN];

    snprintf(iname, BUFLEN, "inst%d", inst->tot_nodes);
    snprintf(cname, BUFLEN, "perf test");
    inst->name[0] = strdup(iname);
    inst->comment[0] = strdup(cname);

    inst->tot_nodes = start + step * set;
    inst->xcoord = points[set][0];
    inst->ycoord = points[set][1];

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
        printf(BOLDRED "[ERROR] <max> must be >4!\n" RESET);
        return;
    }
    if(max > 88){
        if(max > 10000){
            printf(BOLDRED "[ERROR] Too much nodes!\n" RESET);
            return;
        }
        printf(BOLDRED "[WARN] %d points may be too much for your computer!\n", max);
        printf(BOLDRED "[WARN] Press CTR+C or wait 10s...\n" RESET);
        sleep(10); // script-friendly timer
    }

    printf(BOLDGREEN "[INFO] Starting performance mode...\n" RESET);
    instance inst;
    init_instance(&inst);

    while(max % 4 != 0) max--;
    int step = STEP;
    int sets = max / step;

    double ***points = generate_points(max, step);
    //print_points(points, stop, step);

    // open comma separated value file for storing values!
    char *filename = "times.csv";
    FILE *values = fopen(filename,"w");
    fprintf(values, "%d,", (FLAST-1) * 2); // TODO change to include standard formulation
    for(int i = STANDARD + 1; i < FLAST; i++)
        for(char lazy = 0; lazy < 2; lazy++)
            fprintf(values, "%s %s,", formulation_names[i], lazy?"lazy":"");

    for(int set = 0; set < sets; set++) { // change set
        printf(BOLDGREEN "[INFO] ==== Entering set %d/%d of size %d ====\n" RESET, set + 1, sets, (set + 1) * step);
        set_instance(user_inst, &inst, points, max, step, set);
        fprintf(values, "\n%d,", inst.tot_nodes);
        fclose(values);
        values = fopen(filename, "a");
        for (enum formulation_t form = STANDARD+1; form < FLAST; form++) { // change formulation // TODO change to include standard formulation
            for (char lazy = 0; lazy < 2; lazy++) { // add lazy constraints
                set_instance_formulation(&inst, form, lazy);
                if(user_inst->verbose >= 2) save_instance_to_tsp_file(&inst);
                TSPOpt(&inst);
                fprintf(values,"%ld,", inst.runtime);
                //plot(&inst, inst.xstar);
                printf(BOLDGREEN "[INFO] %3s %4s: %ld seconds\n",
                       formulation_names[form], lazy?"lazy":"", inst.runtime);
                free(inst.xstar);
            }
        }
        free(inst.name[0]);
        free(inst.comment[0]);
    }

    fclose(values);
    printf(BOLDGREEN "[INFO] Test finished. Times saved to %s\n" RESET, filename);
    // freeing memory
    free_points(points, max, step);
    // do not call free_instance(): double free!
}