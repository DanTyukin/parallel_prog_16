#include "header.h"

typedef struct {
    const Parameters* pars;
    const EdgeConditions* edge;
    int n_threads;
    double *u;
    int rank;
    int* iteration;   
} ThreadArg;

void * init(void * data) {
    ThreadArg* arg = (ThreadArg*) data;
    int M = arg->pars->M;
    double* phi1 = arg->edge->phi1;
    double c = arg->pars->tau * arg->pars->a / arg->pars->h;
    for (int i = arg->rank + 1; i < M - 1; i += arg->n_threads) {
        arg->u[i] = phi1[i];
        arg->u[i + M] = calculate_second_step(phi1[i-1], phi1[i], phi1[i+1], arg->edge->phi2[i]);
    }
    for (int i = arg->rank; i < arg->pars->P; i += arg->n_threads) {
        arg->u[i*M] = arg->edge->psi1[i];
        arg->u[i*M + M - 1] = arg->edge->psi2[i];
    }
    pthread_exit(0);
}

void * iterate(void * data) {
    ThreadArg* arg = (ThreadArg*) data;
    double c = arg->pars->tau * arg->pars->a / arg->pars->h;
    int M = arg->pars->M;
    int d = *(arg->iteration) * M;
    double* u = arg->u;
    for (int i = arg->rank + 1; i < M - 1; i += arg->n_threads) {
        u[d + M + i] = calculate_iteration(u[d + i], u[d - M + i], u[d + i - 1], u[d + i + 1]);
    }
    pthread_exit(0);
}

void solve_join(Parameters* pars, EdgeConditions* edge, double *result, int n_threads) {
    pthread_t * ids = malloc(n_threads * sizeof(pthread_t));
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    ThreadArg * args = malloc(n_threads * sizeof(ThreadArg));

    int iteration = 1;

    for (int i = 0; i < n_threads; i++) {
        args[i].pars = pars;
        args[i].edge = edge;
        args[i].u = result;
        args[i].iteration = &iteration;
        args[i].rank = i;
        args[i].n_threads = n_threads;
    }
    for (int i = 0; i < n_threads; i++) {
        pthread_create(ids + i, &attr, init, args + i);
    }
    for (int i = 0; i < n_threads; i++) {
        pthread_join(ids[i], NULL);
    }
    while(iteration < pars->P - 1) {
        for (int i = 0; i < n_threads; i++) {
            pthread_create(ids + i, &attr, iterate, args + i);
        }
        for (int i = 0; i < n_threads; i++) {
            pthread_join(ids[i], NULL);
        }
        iteration++;
    }
    free(ids);
    free(args);
}