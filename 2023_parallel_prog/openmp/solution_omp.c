#include "header.h"

Parameters pars;
double c2;

int main(int argc, char** argv) 
{
    int number_of_threads = get_number_of_threads(argc, argv);
    omp_set_dynamic(0);
    omp_set_num_threads(number_of_threads);

    // read from stdin 
    EdgeConditions edge;
    int err = read_from_stdin(&edge, &pars);
    if (err)
    {
        report_read_error(err);
        return err;
    }
    c2 = calculate_c2(&pars);

    // create buffer
    double * result = malloc(pars.P * pars.M * sizeof(double));

    // fill first row

    #pragma omp parallel for num_threads(number_of_threads) 
    for (int m = 0; m < pars.M; m++)
    {
        result[m] = edge.phi1[m];
    }

    // calculate second row

    #pragma omp parallel for num_threads(number_of_threads) 
    for (int m = 1; m < pars.M - 1; m++)
    {
        result[pars.M + m] = calculate_second_step(
            edge.phi1[m - 1],
            edge.phi1[m],
            edge.phi1[m + 1],
            edge.phi2[m]
        );
    }

    // fill edges

    #pragma omp parallel for num_threads(number_of_threads) 
    for (int p = 1; p < pars.P; p++)
    {
        result[pars.M * p] = edge.psi1[p];
        result[pars.M * (p+1) - 1] = edge.psi2[p];
    }

    // iterate for 3..P rows

    for (int p = 2; p < pars.P; p++)
    {
        #pragma omp parallel for num_threads(number_of_threads) 
        for (int m = 1; m < pars.M - 1; m++)
        {
            result[p * pars.M + m] = calculate_iteration(
                result[(p-1)*pars.M + m],
                result[(p-2)*pars.M + m],
                result[(p-1)*pars.M + m - 1], 
                result[(p-1)*pars.M + m + 1]
            );
        }
    }

    // write to stdout 
    write_to_std(result, &pars, 0);

    // free memory
    free(result); 
    free_edge(&edge);

    return 0;
}