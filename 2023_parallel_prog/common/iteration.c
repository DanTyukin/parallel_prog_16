#include "iteration.h"


double calculate_second_step(
    double prev_phi1, double current_phi1, double next_phi1, double phi2)
{
  return (1 - c2) * current_phi1 + c2 / 2 * (prev_phi1 + next_phi1) + pars.tau * phi2;
}

double calculate_iteration(double u11, double u01, double u10, double u12)
{
  return (2 - 2 * c2) * u11  - u01 + c2 * (u10 + u12);
}

double calculate_c2(Parameters * pars) 
{
    return pars->tau * pars->tau * pars->a * pars->a / pars->h / pars->h;
}