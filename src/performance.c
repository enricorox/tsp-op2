//
// Created by enrico on 21/03/21.
//

#include "performance.h"

double *** generate_points(int stop, int step){
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

void set_instance_options(instance *inst, enum formulation_t form, bool lazy){
    inst->formulation = form;
    inst->lazy = lazy;

    inst->verbose = 0;
    inst->gui = false;
    inst->time_limit = 3600; // 1h time limit!
}

void set_instance_points(instance *inst, double ***points, int stop, int step, int set){
    int start = step;
    if(set < 0 || set > stop / start){
        printf(BOLDRED "[ERROR] set_instance_points(): illegal argument\n");
        free_points(points, start, stop);
        exit(1);
    }
    inst->tot_nodes = start + step * set;
    inst->xcoord = points[set][0];
    inst->ycoord = points[set][1];
}

void start_perf_test(){
    instance inst;
    init_instance(&inst);

    int stop = STOP;
    int step = STEP;
    int sets = stop / step;

    double ***points = generate_points(stop, step);
    //print_points(points, stop, step);

    // open comma separated value file for storing values!
    FILE *values = fopen("times.csv","w");
    fprintf(values, "size,");
    for(int i = MTZ; i < FLAST; i++)
        for(char lazy = 0; lazy < 2; lazy++)
            fprintf(values, "%s %s,", formulation_names[i], lazy?"lazy":"");

    char iname[BUFLEN];
    char cname[BUFLEN];
    for(int set = 0; set < sets; set++) { // change set
        set_instance_points(&inst, points, stop, step, set);
        sprintf(iname, "inst%d", inst.tot_nodes);
        sprintf(cname, "perf test");
        inst.name[0] = iname;
        inst.comment[0] = cname;
        fprintf(values, "\n%d,", inst.tot_nodes);
        for (enum formulation_t form = STANDARD+1; form < FLAST; form++) { // change formulation
            for (char lazy = 0; lazy < 2; lazy++) { // add lazy constraints
                set_instance_options(&inst, form, lazy);
                TSPOpt(&inst);
                fprintf(values,"%ld,", inst.time);
                plot(&inst, inst.xstar);
                free(inst.xstar);
                printf(BOLDGREEN "[INFO] %s %s with %d points finished in %ld seconds\n",
                       formulation_names[form], lazy?"lazy":"", inst.tot_nodes, inst.time);
            }
        }
    }

    fclose(values);
    // freeing memory
    free_points(points, stop, step);
}