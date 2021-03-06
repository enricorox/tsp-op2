//
// Created by enrico on 12/03/21.
//
#include "plot.h"

void plot_succ(instance *inst, int *succ){
    double *rxstar = succtox(inst, succ, inst->directed);
    plot(inst, rxstar);
    free(rxstar);
}

void plot(instance *inst, const double *rxstar){
    if(rxstar == NULL)
        printerr(inst, "plot() argument is NULL");

    // write points to file
    char *data_template = "%s-gnuplot-data.dat";
    char data_file[BUFLEN];
    snprintf(data_file, BUFLEN, data_template, inst->name[0]);
    FILE *fdata = fopen(data_file, "w");
    fprintf(fdata, "# Autogenerated by tsp\n");
    for(int i = 0; i < inst->nnodes; i++)
        fprintf(fdata, "%f %f\n", inst->xcoord[i], inst->ycoord[i]);
    fclose(fdata);

    print(inst, 'D', 3, "Points written to file %s", data_file);

    // write gnuplot script
    char *script_template = "%s.%s%s.gnuplot-script.plt";
    char script_name[BUFLEN];
    snprintf(script_name, BUFLEN, script_template, inst->name[0],
             (inst->cons_heuristic != CHLAST) ? cons_heuristic_names[inst->cons_heuristic] : formulation_names[inst->formulation],
             inst->lazy?".lazy":"");
    FILE *fcom = fopen(script_name, "w");

    char *image_template = "%s.%s%s.graph.svg";
    char image_name[BUFLEN];
    snprintf(image_name, BUFLEN, image_template, inst->name[0],
             (inst->cons_heuristic != CHLAST) ? cons_heuristic_names[inst->cons_heuristic] : formulation_names[inst->formulation],
             inst->lazy?".lazy":"");

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
    for(int i = 0; i < inst->nnodes; i++) {
        int s = inst->directed?0:i+1;
        for (int j = s; j < inst->nnodes; j++) {
            int idx = inst->directed ? xpos_directed(i, j, inst) : xpos_undirected(i, j, inst);
            if (rxstar[idx] > 0.5)
                fprintf(fcom, "set arrow arrowstyle %d from %f,%f to %f,%f\n",
                        inst->directed?1:2, // choose right arrow style
                        inst->xcoord[i], inst->ycoord[i],
                        inst->xcoord[j], inst->ycoord[j]);
        }
    }
    // define labels and optimal tour
    for(int i = 0; i < inst->nnodes; i++) {
        fprintf(fcom, "set label '%d' at %f,%f front offset 0.2,0.2 font ',4'\n",
                i + 1, inst->xcoord[i], inst->ycoord[i]);
        if(inst->opt_tour != NULL) {
                int curr = inst->opt_tour[i] - 1;
                int next = inst->opt_tour[(i + 1 == inst->nnodes) ? 0 : i + 1] - 1;
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
    for(int i = 1; i < inst->nnodes; i++){
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

    print(inst, 'D', 3, "Gnuplot script written to file %s", script_name);

    // size gnuplot
    char *command_template = "/usr/bin/gnuplot -p -c %s";
    char command[BUFLEN];
    snprintf(command, BUFLEN, command_template, script_name);
    if(system(command))
        printerr(inst, "Please install gnuplot!");

    print(inst, 'D', 3, "Tour drawn in %s", image_name);

    // show image
    if(inst->verbose >=1 && inst->gui) {
        command_template = "/usr/bin/eog %s";
        snprintf(command, BUFLEN, command_template, image_name);
        if (system(command))
            print(inst, 'W', 1, "Sorry, error opening Eye Of Gnome: can't show graph");
    }
}