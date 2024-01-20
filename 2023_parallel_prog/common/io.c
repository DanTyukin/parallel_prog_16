#include "io.h"

enum ReadError {
    ERR_M,
    ERR_P,
    ERR_h,
    ERR_tau,
    ERR_a,
    ERR_phi1,
    ERR_phi2,
    ERR_psi1,
    ERR_psi2,
    ERR_c,
    ERR_OUT_OF_DOMAIN = 128
};

const char *parameter_name[] = {"M", "P", "h", "tau", "a", "phi1", "phi2",
                                "psi1", "psi2"};

int read_edge_conditions(EdgeConditions* edge, Parameters *pars);
int generate_edge_conditions(EdgeConditions *edge, Parameters *pars);

int read_from_stdin(EdgeConditions *edge, Parameters *pars)
{
    if (!pars || !edge)
        return -1;
    //    read M
    if (!scanf("%d", &pars->M))
        return ERR_M;
    if (pars->M < 3)
        return ERR_M | ERR_OUT_OF_DOMAIN;
    //    read P
    if (!scanf("%d", &pars->P))
        return ERR_P;
    if (pars->P < 2)
        return ERR_P | ERR_OUT_OF_DOMAIN;
    //    read tau
    if (!scanf("%lf", &pars->tau))
        return ERR_tau;
    if (pars->tau <= 0)
        return ERR_tau | ERR_OUT_OF_DOMAIN;
    //    read h
    if (!scanf("%lf", &pars->h))
        return ERR_h;
    if (pars->h <= 0)
        return ERR_h | ERR_OUT_OF_DOMAIN;
    //    read a
    if (!scanf("%lf", &pars->a))
        return ERR_a;
    if (pars->a <= 0)
        return ERR_a | ERR_OUT_OF_DOMAIN;

    // |tau*a/h| should be < 1 for algorithm to converge
    if (abs(pars->tau * pars->a / pars->h) >= 1)
        return ERR_c;

    if (SPEED_TEST) 
        return generate_edge_conditions(edge, pars);
    else 
        return read_edge_conditions(edge, pars);
}

int read_edge_conditions(EdgeConditions *edge, Parameters *pars)
{
    int err = 0;
    edge->phi1 = edge->phi2 = edge->psi1 = edge->psi2 = 0;

    //    for 1..M read phi1
    edge->phi1 = malloc(pars->M * sizeof(double));
    for (int i = 0; i < pars->M; i++)
    {
        if (!scanf("%lf", edge->phi1 + i))
        {
            err = ERR_phi1;
            goto cleanup;
        }
    }

    //    for 1..M read phi2
    edge->phi2 = malloc(pars->M * sizeof(double));
    for (int i = 0; i < pars->M; i++)
    {
        if (!scanf("%lf", edge->phi2 + i))
        {
            err = ERR_phi2;
            goto cleanup;
        }
    }

    //    for 1..P read psi1
    edge->psi1 = malloc(pars->P * sizeof(double));
    for (int i = 0; i < pars->P; i++)
    {
        if (!scanf("%lf", edge->psi1 + i))
        {
            err = ERR_psi1;
            goto cleanup;
        }
    }

    //    for 1..P read psi1
    edge->psi2 = malloc(pars->P * sizeof(double));
    for (int i = 0; i < pars->P; i++)
    {
        if (!scanf("%lf", edge->psi2 + i))
        {
            err = ERR_psi2;
            goto cleanup;
        }
    }
    return 0;
cleanup:
    free(edge->phi1);
    free(edge->phi2);
    free(edge->psi1);
    free(edge->psi2);
    return err;
}

int generate_edge_conditions(EdgeConditions *edge, Parameters *pars)
{
    double l = pars->h * pars->M;
    size_t phi_size = sizeof(double) * pars->M;
    size_t psi_size = sizeof(double) * pars->P;
    edge->phi1 = malloc(phi_size);
    edge->phi2 = malloc(phi_size);
    edge->psi1 = malloc(psi_size);
    edge->psi2 = malloc(psi_size);
    for (size_t i = 0; i < pars->M; i++)
    {
        edge->phi1[i] = 0;
        edge->phi2[i] = 0;
    }
    for (size_t i = 0; i < pars->P; i++)
    {
        edge->psi1[i] = sin(6*M_PI*i / (pars->P + 1));
        edge->psi2[i] = 0;
    }
    return 0;
}

void report_read_error(int error) 
{
    const char* err_type = error < ERR_OUT_OF_DOMAIN ?
        "value out of domain" : "cannot read value";
    error &= ~ERR_OUT_OF_DOMAIN;
    const char* parameter = 0;
    if (error < sizeof(parameter_name) / sizeof(parameter_name[0]))
    {
        fprintf(stderr, "Error reading %s: %s.\n",
                parameter_name[error], err_type);
    }
    else if (error == ERR_c) 
    {
        fprintf(stderr, "Error reading parameters: invalid values of tau, h and a, algorithm does not converge.\n");
    }
    else
    {
        fprintf(stderr, "An error occurred while reading parameters.\n");
    }
}

void write_to_std(double * result, Parameters * pars, int transpose)
{
    if (SPEED_TEST || !result || !pars) return;
    for (int p = 0; p < pars->P; p++)
    {
        for (int m = 0; m < pars->M; m++)
            printf("%lf ", transpose ? result[p + pars->P*m] : result[m + pars->M * p]);
        printf("\n");
    }
}

void free_edge(EdgeConditions* edge) 
{
    if (!edge) return;
    if (edge->phi1) free(edge->phi1);
    if (edge->phi2) free(edge->phi2);
    if (edge->psi1) free(edge->psi1);
    if (edge->psi2) free(edge->psi2);
}

int get_number_of_threads(int argc, char ** argv)
{
    int number_of_threads = N_THREADS;
    if (argc == 2) {
        int nt = atoi(argv[1]);
        if (nt > 0) number_of_threads = nt;
    }
    return number_of_threads;
}