#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

#include "header.h"

#ifdef JOIN
    void (*solve)(Parameters*, EdgeConditions*, double*, int) = solve_join;
#else 
    void (*solve)(Parameters*, EdgeConditions*, double*, int) = solve_barrier;
#endif

Parameters pars;
double c2;

int main(int argc, char** argv) {
    // get number of threads
    int number_of_threads = get_number_of_threads(argc, argv);

    // read parameters from stdin
    EdgeConditions edge;
    int err = read_from_stdin(&edge, &pars);
    if (err) {
        report_read_error(err);
        return err;
    }
    c2 = calculate_c2(&pars);

    // solve 
    double * result = malloc(pars.M * pars.P * sizeof(double));
    solve(&pars, &edge, result, number_of_threads);

    // write to stdout
    write_to_std(result, &pars, 0);
    
    free(result);
    free_edge(&edge);

    return 0;
}