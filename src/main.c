#include <stdio.h>
#include <stdlib.h>
#include <cplex.h>
#include "tsp.h"

// Debug functions
void debug(const char *err) { printf("\nDEBUG: %s \n", err); }
void print_error(const char *err) { printf("\n\n ERROR: %s \n\n", err); exit(1); }

int main(int argc, char** argv){
    printf("It works!");
}
