//
// Created by enrico on 23/03/21.
//

#include "distances.h"

// nearest integer
int nint(double x){
    return  (int) (x + 0.499999999);
}

double dist_euc2d(int i, int j, instance *inst){
    double dx = inst->xcoord[i] - inst->xcoord[j];
    double dy = inst->ycoord[i] - inst->ycoord[j];
    if ( !inst->integer_costs ) return sqrt(dx*dx+dy*dy);
    int dis = nint(sqrt(dx*dx+dy*dy));
    return dis+0.0;
}

double dist_att(int i, int j, instance *inst){
    double dx = inst->xcoord[i] - inst->xcoord[j];
    double dy = inst->ycoord[i] - inst->ycoord[j];
    double rij = sqrt( (dx*dx + dy*dy) / 10.0 );
    int tij = nint(rij);
    return (tij < rij) ? tij + 1 : tij;
}

double dist_geo(int i, int j, instance *inst){
    // convert to latitude in radians
    int deg = nint( inst->xcoord[i] );
    double min = inst->xcoord[i] - deg;
    double latitude = PI * (deg + 5.0 * min / 3.0 ) / 180.0;

    // convert to longitude in radians
    deg = nint( inst->ycoord[i] );
    min = inst->ycoord[i] - deg;
    double longitude = PI * (deg + 5.0 * min / 3.0 ) / 180.0;

    // convert to km
    double q1 = cos( longitude - longitude );
    double q2 = cos( latitude - latitude );
    double q3 = cos( latitude + latitude );
    int dij = (int) ( RRR * acos( 0.5*((1.0+q1)*q2 - (1.0-q1)*q3) ) + 1.0);
    return dij;
}

double cost(int i, int j, instance *inst) {
    switch(inst->dist){
        case EUC_2D: return dist_euc2d(i, j, inst);
        case ATT: return dist_att(i, j, inst);
        case GEO: return dist_geo(i, j, inst);
        default:
            printf(BOLDRED "[ERROR] Unknown distance type!\n" RESET);
            free_instance(inst);
            exit(1);
    }
}