//
// Created by enrico on 12/03/21.
//
#include "plot.h"

void plot(instance *inst, double const *rxstar){
    // write points to file
    char *data_template = "%s-gnuplot-data.dat";
    char data_file[strlen(data_template) + strlen(inst->name[0]) + 1];
    sprintf(data_file, data_template, inst->name[0]);
    FILE *fdata = fopen(data_file, "w");
    fprintf(fdata, "# Autogenerated by tsp\n");
    for(int i = 0; i < inst->tot_nodes; i++)
        fprintf(fdata, "%f %f\n", inst->xcoord[i], inst->ycoord[i]);
    fclose(fdata);

    if(inst->verbose >= 1) printf(BOLDGREEN "[INFO] Points written to file %s\n" RESET, data_file);

    // write gnuplot script
    char *script_template = "%s.%s%s-gnuplot-script.plt";
    char script_name[strlen(script_template) + strlen(inst->name[0]) +
            + strlen(formulation_names[inst->formulation]) + 1];
    sprintf(script_name, script_template, inst->name[0],
            formulation_names[inst->formulation], inst->lazy?"-lazy":"");
    FILE *fcom = fopen(script_name, "w");

    char *image_template = "%s.%s%s-graph.svg";
    char image_name[strlen(image_template) + strlen(inst->name[0]) +
            + strlen(formulation_names[inst->formulation]) + 1];
    sprintf(image_name, image_template, inst->name[0],
            formulation_names[inst->formulation], inst->lazy?"-lazy":"");

    // write graph properties
    fprintf(fcom,   "# Autogenerated by tsp\n"
                           "set title '%s - %s'\n"
                           "set style circle radius graph 0.004\n"
                           "set style line 1 lt 1 lw 1\n"
                           "set style arrow 1 head filled size graph 0.01,13,45 ls 1 lc rgb 'red'\n"
                           "set style arrow 2 nohead filled size graph 0.01,13,45 ls 1 lc rgb 'red'\n"
                           "set style arrow 3 head filled size graph 0.01,13,45 ls 1 lc rgb 'green' dt '-'\n"
                           "set style arrow 4 nohead filled size graph 0.01,13,45 ls 1 lc rgb 'green' dt '-'\n"
                           "set terminal svg\n"
                           "set output '%s'\n"
                           "unset key\n"
                           "unset xtics\n"
                           "unset ytics\n"
                           "unset border\n",
                           inst->name[0], inst->comment[0], image_name);

    // defining edges
    for(int i = 0; i < inst->tot_nodes; i++) {
        int s = inst->directed?0:i+1;
        for (int j = s; j < inst->tot_nodes; j++) {
            int idx = inst->directed?xpos_compact(i, j, inst):xpos(i, j, inst);
            if (rxstar[idx])
                fprintf(fcom, "set arrow arrowstyle %d from %f,%f to %f,%f\n",
                        inst->directed?1:2, // choose right arrow style
                        inst->xcoord[i], inst->ycoord[i],
                        inst->xcoord[j], inst->ycoord[j]);
        }
    }
    // define labels
    for(int i = 0; i < inst->tot_nodes; i++) {
        fprintf(fcom, "set label '%d' at %f,%f front offset 0.2,0.2 font ',4'\n",
                i + 1, inst->xcoord[i], inst->ycoord[i]);
        if(inst->opt_tour != NULL) {
                int curr = inst->opt_tour[i] - 1;
                int next = inst->opt_tour[(i + 1 == inst->tot_nodes) ? 0 : i + 1] - 1;
                fprintf(fcom, "set arrow arrowstyle %d from %f,%f to %f,%f\n",
                        inst->directed?3:4, // choose right arrow style
                        inst->xcoord[curr], inst->ycoord[curr],
                        inst->xcoord[next], inst->ycoord[next]);
            }
    }

    // find max and min for nice margin
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

    if(inst->verbose >= 1) printf(BOLDGREEN "[INFO] Gnuplot script written to file %s\n" RESET, script_name);

    // run gnuplot
    char *command_template = "/usr/bin/gnuplot -c %s";
    char command[strlen(command_template) + strlen(script_name) + 1];
    sprintf(command, command_template, script_name);
    if(system(command)) {
        printf(BOLDRED "[ERROR] Please install gnuplot!\n" RESET);
        free_instance(inst);
        exit(1);
    }
    if(inst->verbose) printf(BOLDGREEN "[INFO] Tour drawn in %s\n" RESET, image_name);

    // show image
    if(inst->verbose >=1 && inst->gui) {
        sprintf(command, "/usr/bin/eog %s", image_name);
        if (system(command))
            printf(BOLDRED "[WARN] Sorry, Eye Of Gnome not found: can't show graph\n" RESET);
    }
}