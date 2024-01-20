#include "header.h"

typedef struct {
    const Parameters* pars;
    const EdgeConditions* edge;
    int n_threads;
    double *u;
    int rank;
    int* iteration;   
} ThreadArg;

static pthread_barrier_t barrier;

void* thread_routine(void* data) {
    ThreadArg* arg = (ThreadArg*) data;
    int M = arg->pars->M;
    double* phi1 = arg->edge->phi1;
    double c = arg->pars->tau * arg->pars->a / arg->pars->h;
    for (int i = arg->rank + 1; i < M - 1; i += arg->n_threads) {
        arg->u[i] = phi1[i];
        arg->u[M + i] = calculate_second_step(phi1[i-1], phi1[i], phi1[i+1], arg->edge->phi2[i]);
    }
    for (int i = arg->rank; i < arg->pars->P; i += arg->n_threads) {
        arg->u[i*M] = arg->edge->psi1[i];
        arg->u[i*M + M - 1] = arg->edge->psi2[i];
    }

    pthread_barrier_wait(&barrier);
    int iteration = 1;
    while (iteration < arg->pars->P - 1) {
        int d = iteration * M;
        double* u = arg->u;
        for (int i = arg->rank + 1; i < M - 1; i += arg->n_threads) {
            u[d + M + i] = calculate_iteration(u[d + i], u[d - M + i], u[d + i - 1], u[d + i + 1]);
        }
        pthread_barrier_wait(&barrier);
        iteration++;
    }

    pthread_exit(0);
}

void solve_barrier(
    Parameters* pars, 
    EdgeConditions* edge, 
    double *result, int n_threads
) {
    pthread_t * ids = malloc(n_threads * sizeof(pthread_t));
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    ThreadArg * args = malloc(n_threads * sizeof(ThreadArg));

    int iteration = 1;
    int status = pthread_barrier_init(&barrier, NULL, n_threads);
    if (status != 0) {
        goto end;   
    }
    for (int i = 0; i < n_threads; i++) {
        args[i].pars = pars;
        args[i].edge = edge;
        args[i].u = result;
        args[i].rank = i;
        args[i].n_threads = n_threads;
    }
    for (int i = 0; i < n_threads; i++) {
        pthread_create(ids + i, &attr, thread_routine, args + i);
    }
    for (int i = 0; i < n_threads; i++) {
        pthread_join(ids[i], NULL);
    }
end:
    free(ids);
    free(args);
}