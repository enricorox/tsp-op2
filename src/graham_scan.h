/**
 * Header file for Graham scan algorithm program
 * 
 * Author: Jakub Wlodek
 * Created on: Nov 30, 2018
 */


// include guard
#ifndef GRAHAM_SCAN_H
#define GRAHAM_SCAN_H

typedef enum{
    GS_RightTurn,
    GS_LeftTurn,
    GS_Inline
} GS_Turn;

/* Structures used to represent a point and a set of points */
typedef struct Point {
    double xCoord;
    double yCoord;
    double angle;
    int id;
} Point;

typedef struct PointSet {
    int num_points;
    Point* points;
} PointSet;

/* function definitions */
PointSet* parse_input_file(char* file_name);

void print_points(PointSet* ps);

void print_stack(Point* p, int stack_size);

void print_points_to_file(PointSet* ps, char* file_name);

int compare_point_sets(PointSet* ps1, PointSet* ps2);

GS_Turn find_turn_type(Point* p1, Point* p2, Point* p3);

Point* find_lowest_point(PointSet* ps);

void sort_by_angle(Point* points, int left, int right);

PointSet* compute_convex_hull(PointSet* ps);

PointSet* remove_degeneracy(PointSet* convexHull);

void compute_angles(PointSet* ps, Point* lowest);

double distance_between(Point* p1, Point* p2);

void merge_halves(Point* points, int left, int center, int right);

#endif