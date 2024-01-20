#include <pthread.h>
#include <stdlib.h>

#include "../common/io.h"
#include "../common/iteration.h"

void solve_join(Parameters* pars, EdgeConditions* edge, double *result, int n_threads);
void solve_barrier(Parameters* pars, EdgeConditions* edge, double *result, int n_threads);
