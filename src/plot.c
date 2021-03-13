//
// Created by enrico on 12/03/21.
//

#include "plot.h"
#include "tsp.h"

void plot(instance *inst, char const *rxstar){
    // write points to file
    FILE *fdata = fopen(GPDATA, "w");
    for(int i = 0; i < inst->tot_nodes; i++)
        fprintf(fdata, "%f %f\n", inst->xcoord[i], inst->ycoord[i]);
    fclose(fdata);

    if(inst->verbose >= 1) printf(BOLDGREEN "[INFO] Points written to file" GPDATA "\n" RESET);

    // write gnuplot script
    FILE *fcom = fopen(GPCOMM,"w");

    // write graph properties
    fprintf(fcom,   "set title 'TSP solution'\n"
                           "set style circle radius graph 0.005\n"
                           "set autoscale\n"
                           "set terminal svg\n"
                           "set output '" FILESVG "'\n"
                           "unset key\n"
                           "unset xtics\n"
                           "unset ytics\n"
                           "unset border\n"
                           );

    // defining edges
    for(int i = 0; i < inst->tot_nodes; i++)
        for(int j = i+1; j < inst->tot_nodes; j++)
            if(rxstar[xpos(i, j, inst)])
                fprintf(fcom, "set arrow from %f,%f to %f,%f nohead lc rgb 'red'\n",
                        inst->xcoord[i], inst->ycoord[i],
                        inst->xcoord[j], inst->ycoord[j]);

    // define labels
    for(int i = 0; i < inst->tot_nodes; i++)
        fprintf(fcom, "set label '%d' at %f,%f front offset 0.2,0.2 font 'Symbol,4'\n",
                i+1, inst->xcoord[i], inst->ycoord[i]);

    // find max and min for border
    double xmax = inst->xcoord[0];
    double xmin = inst->xcoord[0];
    double ymax = inst->ycoord[0];
    double ymin = inst->ycoord[0];
    for(int i = 1; i < inst->tot_nodes; i++){
        if(xmax < inst->xcoord[i])
            xmax = inst->xcoord[i];
        if(xmin > inst->xcoord[i])
            xmin = inst->xcoord[i];
        if(ymax < inst->ycoord[i])
            ymax = inst->ycoord[i];
        if(ymin > inst->ycoord[i])
            ymin = inst->ycoord[i];
    }

    fprintf(fcom, "plot [%f:%f][%f:%f] '" GPDATA "' with circles\n",
            xmin-5, xmax+5,
            ymin-5, ymax+5);

    fclose(fcom);

    if(inst->verbose >= 1) printf(BOLDGREEN "[INFO] Gnuplot script written to file " GPCOMM "\n" RESET);

    // run gnuplot
    if(system("gnuplot -p -c " GPCOMM)) {
        printf(BOLDRED "[ERROR] Please install gnuplot!\n" RESET);
        exit(1);
    }
    if(inst->verbose) printf(BOLDGREEN "[INFO] Tour drawn to " FILESVG "\n" RESET);
    if(inst->verbose >=1) system("eog " FILESVG);
}