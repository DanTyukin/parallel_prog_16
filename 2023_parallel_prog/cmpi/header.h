#include <mpi.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "../common/io.h"
#include "../common/iteration.h"

#define V 0
#define LOG if (V) printf
#define TAG 1

void exchange(double* left, double* right, int rank, int np);

/**
 * @param arr initial array
 * @param x_dim allocated size for x axis
 * @param m quant in x axis
 * @param p quant in t axis
 * @param value value to be set
*/
void set_to_local_result(double* arr, int x_dim, int m, int p, double value);

double get_from_local_result(double* arr, int x_dim, int m, int p);

