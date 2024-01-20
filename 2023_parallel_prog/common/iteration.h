#ifndef ITERATION_H
#define ITERATION_H

#include "io.h"

extern Parameters pars;
extern double c2;

double calculate_second_step(
    double prev_phi1, double current_phi1, double next_phi1, double phi2);

/**
 * @param u11 U[p][m]
 * @param u01 U[p-1][m]
 * @param u10 U[p][m-1]
 * @param u12 U[p][m+1]
*/
double calculate_iteration(double u11, double u01, double u10, double u12);

double calculate_c2(Parameters * pars);

#endif