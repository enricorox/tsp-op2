//
// Created by enrico on 21/03/21.
//

#include <stdlib.h>

#include "performance.h"

#define START 5
#define STOP 80
#define STEP 5

double *** generate_points(int start, int stop, int step){
    double ***points;
    printf("Malloc-ing points...\n");

    // allocate memory for pointers
    int sets = stop / start;
    points = (double ***) malloc(sets * sizeof(double **));
    for(int i = 0; i < sets; i++) {
        // allocate memory for pointers again (xcoord and ycoord)
        points[i] = malloc(2 * sizeof(double *));
        // allocate memory for integers
        points[i][0] = malloc((start + sets * i) * sizeof(double));
        //printf("malloc(): points[%d][0] = %p\n", i, points[i][0]);
        points[i][1] = malloc((start + sets * i) * sizeof(double));
        //printf("malloc(): points[%d][1] = %p\n", i, points[i][1]);
        for(int k = 0; k < start + step * i ; k++) {
            points[i][0][k] = rand();
            points[i][1][k] = rand();
        }
    }
    return points;
}

void free_points(double ***points, int start, int stop){
    //printf("Free-ing points...\n");
    int sets = stop/start;
    for(int i = 0; i < sets; i++){
        //printf("free(): points[%d][0] = %p\n", i, points[i][0]);
        free(points[i][0]);
        //printf("free(): points[%d][1] = %p\n", i, points[i][1]);
        free(points[i][1]);
        free(points[i]);
    }
    free(points);
    printf("Points freed!\n");
}

void print_points(double ***points, int start, int stop, int step){
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
}

void set_instance_points(instance *inst, double ***points, int start, int stop, int step, int batch){
    if(batch < 0 || batch > stop / start){
        printf(BOLDRED "[ERROR] set_instance_points(): illegal argument\n");
        free_points(points, start, stop);
        exit(1);
    }
    inst->tot_nodes = start + step * batch;
    inst->xcoord = points[batch][0];
    inst->ycoord = points[batch][1];
}

void start_perf_test(){
    instance inst;
    init_instance(&inst);

    int start = START; int stop = STOP;
    int step = STEP;

    double ***points = generate_points(start, stop, step);
    print_points(points, start, stop, step);

    free_points(points, start, stop);
    free_instance(&inst);
}