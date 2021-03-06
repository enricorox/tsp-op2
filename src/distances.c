//
// Created by enrico on 23/03/21.
//

#include "distances.h"

// nearest integer
long nint(double x){
    return  (long) (x + 0.499999999);
    //return (long) x; // see http://comopt.ifi.uni-heidelberg.de/software/TSPLIB95/TSPFAQ.html
                    // problem whit berlin52 but still optimal
}

double dist_euc2d(int i, int j, instance *inst){
    double dx = inst->xcoord[i] - inst->xcoord[j];
    double dy = inst->ycoord[i] - inst->ycoord[j];
    double dis = sqrt(dx*dx+dy*dy);
    if ( !inst->integer_costs ) return dis;
    //long dis = nint(sqrt(dx*dx+dy*dy));
    return round(dis);
}

double dist_att(int i, int j, instance *inst){
    double dx = inst->xcoord[i] - inst->xcoord[j];
    double dy = inst->ycoord[i] - inst->ycoord[j];
    double rij = sqrt( (dx*dx + dy*dy) / 10.0 );
    double tij = (double) nint(rij);
    return (tij < rij) ? tij + 1 : tij;
}

void real_to_geo_coords(int i, double *lat, double *lon, const instance *inst){
    // convert to latitude in radians
    double deg = (int) inst->xcoord[i];
    double min = inst->xcoord[i] - deg;
    *lat = PI * (deg + 5.0 * min / 3.0 ) / 180.0;

    // convert to longitude in radians
    deg = (int) inst->ycoord[i];
    min = inst->ycoord[i] - deg;
    *lon = PI * (deg + 5.0 * min / 3.0 ) / 180.0;
}

double dist_geo(int i, int j, const instance *inst){
    // see http://comopt.ifi.uni-heidelberg.de/software/TSPLIB95/TSPFAQ.html
    double lat_i, lon_i, lat_j, lon_j;
    real_to_geo_coords(i, &lat_i, &lon_i, inst);
    real_to_geo_coords(j, &lat_j, &lon_j, inst);

    //printf("lat_%d = %f | lon_%d = % f\n", i+1, lat_i, i+1, lon_i);
    //printf("lat_%d = %f | lon_%d = % f\n", j+1, lat_j, j+1, lon_j);

    double q1 = cos( lon_i - lon_j );
    double q2 = cos( lat_i - lat_j );
    double q3 = cos( lat_i + lat_j );
    int dij = (int) (REARTH * acos(0.5 * ((1.0 + q1) * q2 - (1.0 - q1) * q3) ) + 1.0);
    //printf("d(%2d,%2d) = %d\n", i + 1, j + 1, dij);
    return dij;
}

double cost(int i, int j, instance *inst) {
    double dist;
    switch(inst->dist){
        case EUC_2D: dist = dist_euc2d(i, j, inst); break;
        case ATT: dist = dist_att(i, j, inst); break;
        case GEO: dist = dist_geo(i, j, inst); break;
        default:
            printf(BOLDRED "[ERROR] Unknown distance type!\n" RESET);
            free_instance(inst);
            exit(1);
    }
    if(dist < 0) {
        print(inst, 'W', 1, "Buffer Overflow detected while computing cost(%d,%d)!\n"
                            "Using DBL_MAX instead.", i + 1, j + 1);
        dist = DBL_MAX;
    }
    return dist;
}