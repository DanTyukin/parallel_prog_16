#ifndef IO_H
#define IO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef SPEED_TEST
#define SPEED_TEST 0
#endif

#ifndef N_THREADS
#define N_THREADS 8
#endif

typedef struct {
    int M;
    int P;
    double h;
    double tau;
    double a;
} Parameters;

typedef struct {
    double *phi1;
    double *phi2;
    double *psi1;
    double *psi2;
} EdgeConditions;

int read_from_stdin(EdgeConditions* edge, Parameters* pars);
void report_read_error(int error);
/**
 * @param result flat array P*M to be written in stdout
 * @param transpose if 0 every row represents M axis, if 1 every row represents P 
 * axis
*/
void write_to_std(double * result, Parameters * pars, int transpose);

void free_edge(EdgeConditions* edge);

int get_number_of_threads(int argc, char ** argv);

#endif