//
// Created by enrico on 12/03/21.
//
#include "plot.h"

void plot(instance *inst, char const *rxstar){
    // write points to file
    char data_file[BUFLEN];
    sprintf(data_file, "%s-gnuplot-data.dat",inst->name[0]);
    FILE *fdata = fopen(data_file, "w");
    fprintf(fdata, "# Autogenerated by tsp\n");
    for(int i = 0; i < inst->tot_nodes; i++)
        fprintf(fdata, "%f %f\n", inst->xcoord[i], inst->ycoord[i]);
    fclose(fdata);

    if(inst->verbose >= 1) printf(BOLDGREEN "[INFO] Points written to file %s\n" RESET, data_file);

    // write gnuplot script
    char commands_file[BUFLEN];
    sprintf(commands_file, "%s-gnuplot-commands-model-%s.plt", inst->name[0],
            formulation_names[inst->formulation]);
    FILE *fcom = fopen(commands_file,"w");

    char image_name[BUFLEN];
    sprintf(image_name,"%s-graph-model-%s.svg", inst->name[0],
            formulation_names[inst->formulation]);

    // write graph properties
    fprintf(fcom,   "# Autogenerated by tsp\n"
                           "set title '%s - %s'\n"
                           "set style circle radius graph 0.004\n"
                           "set terminal svg\n"
                           "set output '%s'\n"
                           "unset key\n"
                           "unset xtics\n"
                           "unset ytics\n"
                           "unset border\n",
                           inst->name[0], inst->comment[0], image_name);

    // defining edges
    for(int i = 0; i < inst->tot_nodes; i++)
        for(int j = 0; j < inst->tot_nodes; j++)// TODO fix j = i+1
            if(rxstar[xpos_compact(i, j, inst)]) // TODO fix
                fprintf(fcom, "set arrow from %f,%f to %f,%f lc rgb 'red'\n", // TODO fix nohead
                        inst->xcoord[i], inst->ycoord[i],
                        inst->xcoord[j], inst->ycoord[j]);

    // define labels
    for(int i = 0; i < inst->tot_nodes; i++) {
        fprintf(fcom, "set label '%d' at %f,%f front offset 0.2,0.2 font ',4'\n",
                i + 1, inst->xcoord[i], inst->ycoord[i]);
        if(inst->opt_tour != NULL) {
                int curr = inst->opt_tour[i] - 1;
                int next = inst->opt_tour[(i + 1 == inst->tot_nodes) ? 0 : i + 1] - 1;
                fprintf(fcom, "set arrow from %f,%f to %f,%f nohead lc rgb 'green' dt '-'\n",
                    inst->xcoord[curr], inst->ycoord[curr],
                    inst->xcoord[next], inst->ycoord[next]);
            }
    }

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

    double xoffset = (xmax - xmin)/15;
    double yoffset = (ymax - ymin)/15;
    fprintf(fcom, "plot [%f:%f][%f:%f] '%s' with circles fillcolor 'cyan' fillstyle solid border linecolor 'blue'\n",
            xmin-xoffset, xmax+xoffset,
            ymin-yoffset, ymax+yoffset,
            data_file);

    fclose(fcom);

    if(inst->verbose >= 1) printf(BOLDGREEN "[INFO] Gnuplot script written to file %s\n" RESET, commands_file);

    // run gnuplot
    char command[BUFLEN];
    sprintf(command, "/usr/bin/gnuplot -c %s", commands_file);
    if(system(command)) {
        printf(BOLDRED "[ERROR] Please install gnuplot!\n" RESET);
        free_instance(inst);
        exit(1);
    }
    if(inst->verbose) printf(BOLDGREEN "[INFO] Tour drawn in %s\n" RESET, image_name);
    if(inst->verbose >=1 && inst->gui) {
        sprintf(command, "/usr/bin/eog %s", image_name);
        if (system(command))
            printf(BOLDRED "[WARN] Sorry, Eye Of Gnome not found: can't show graph\n" RESET);
    }
}