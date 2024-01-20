#include "header.h"

Parameters pars;
double c2;


int min(int a, int b) { return a < b ? a : b; }

int main(int argc, char **argv)
{
  int rank, number_of_processes;
  MPI_Status status;
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &number_of_processes);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  
  EdgeConditions edge;
  if (rank == 0)
  {
    // read parameters
    int err = read_from_stdin(&edge, &pars);
    if (err) {
      report_read_error(err);
      MPI_Abort(MPI_COMM_WORLD, err);
    }
  }

  MPI_Bcast(&pars, sizeof(pars), MPI_CHAR, 0, MPI_COMM_WORLD);
  // c squared = (tau * a / h)^2
  c2 = calculate_c2(&pars);

  if (rank != 0)
  {
    LOG("%d: received pars. M = %d, P = %d, a = %lf, tau = %lf, h = %lf\n",
        rank, pars.M, pars.P, pars.a, pars.tau, pars.h);
  }

  // every process work with at least two values
  if (number_of_processes >= pars.M - 2)
  {
    if (rank >= (pars.M - 2) / 2) 
    {
      MPI_Finalize();
      return 0;
    }
    number_of_processes = (pars.M - 2) / 2;
  }

  // fan out
  int *to_fan_out = malloc(number_of_processes * sizeof(int));
  to_fan_out[0] = number_of_processes;
  double *phi1_buffer = NULL, *phi2_buffer = NULL;
  int buffer_size = 0;
  int local_result_size = 0;
  if (rank == 0) 
  {
    // sending without edges
    buffer_size = pars.M - 2;
    phi1_buffer = edge.phi1 + 1;
    phi2_buffer = edge.phi2 + 1;
  }

  int doubles_in_part = (pars.M - 2) / number_of_processes;
  int start_k = 1;
  while((start_k << 1) < number_of_processes)
    start_k <<= 1;
  for (int k = start_k; k > 0; k >>= 1)
  {
    for (int i = 0; i + k < number_of_processes; i += 2 * k)
    {
      int to_send = min(number_of_processes, i + 2 * k) - (i + k);
      int doubles_to_send = doubles_in_part * to_send;
      to_fan_out[i + k] = to_send;
      to_fan_out[i] -= to_send;
      if (rank == i) 
      {
        LOG("%d: about to send %d to %d, remains: %d\n", i, to_send, i + k,
            to_fan_out[i]);
        // send last `doubles_to_send` values
        MPI_Send(
            phi1_buffer + buffer_size - doubles_to_send,
            doubles_to_send, MPI_DOUBLE, i + k, TAG, MPI_COMM_WORLD);
        MPI_Send(
            phi2_buffer + buffer_size - doubles_to_send,
            doubles_to_send, MPI_DOUBLE, i + k, TAG, MPI_COMM_WORLD);
        // "free" last `doubles_to_send` values
        buffer_size -= doubles_to_send;
      }
      if (rank == i + k) 
      {
        LOG("%d: about to receive %d from %d\n", i + k, to_send, i);
        phi1_buffer = malloc(doubles_to_send * sizeof(double));
        phi2_buffer = malloc(doubles_to_send * sizeof(double));
        MPI_Recv(phi1_buffer, doubles_to_send, MPI_DOUBLE, i,
                 TAG, MPI_COMM_WORLD, &status);
        MPI_Recv(phi2_buffer, doubles_to_send, MPI_DOUBLE, i,
                 TAG, MPI_COMM_WORLD, &status);
        local_result_size = buffer_size = doubles_to_send;
      }
    }
  }
  free(to_fan_out);

  LOG("%d: my first is %lf, last is %lf\n", rank, phi1_buffer[0], phi1_buffer[buffer_size - 1]);


  // rank 0 processes x in range from 2 to its `buffer_size`, but 
  // allocates memory for global result
  int x_dim = rank == 0 ? pars.M : local_result_size;
  LOG("%d: my x_dim = %d, buffer_size = %d\n", rank, x_dim, buffer_size);
  double * local_result = malloc(x_dim * pars.P * sizeof(double));

  // for rank 0 to gather all the result
  double * global_result = 0;
  if (rank == 0) {
    global_result = local_result;
    // rank 0 writes in its buffer from 1 because it bypasses psi1
    local_result += pars.P;
  }

  // calculate u[1]
  for (int x = 0; x < buffer_size; x++)
    set_to_local_result(local_result, x_dim, x, 0, phi1_buffer[x]);
  // calculate u[2]
  // need to get phi1 values from neighbors
  // rank 0 exchanges with last rank, but it sends tight boundary
  double exchange_left = (rank == 0) ? edge.psi2[0] : phi1_buffer[0],
         exchange_right = phi1_buffer[buffer_size - 1];
  LOG("\t%d: about to exchange %lf and %lf\n", rank, exchange_left, exchange_right);
  exchange(&exchange_left, &exchange_right, rank, number_of_processes);
  LOG("\t\t%d: after exchange %lf and %lf\n", rank, exchange_left, exchange_right);
  // rank 0 does not actually need to exchange, 
  // because its left value is left boundary
  if (rank == 0)
    exchange_left = edge.psi1[0];
  // calculate left edge
  set_to_local_result(local_result, x_dim, 0, 1, calculate_second_step(
      exchange_left, phi1_buffer[0], phi1_buffer[1], phi2_buffer[0]
  ));
  // calculate right edge
  set_to_local_result(
      local_result, x_dim, buffer_size - 1, 1, calculate_second_step(
        phi1_buffer[buffer_size - 2], phi1_buffer[buffer_size - 1], 
        exchange_right, phi2_buffer[buffer_size - 1]));
  // calculate inner points
  for (int x = 1; x < buffer_size - 1; x++)
    set_to_local_result(local_result, x_dim, x, 1, calculate_second_step(
      phi1_buffer[x - 1], phi1_buffer[x], phi1_buffer[x + 1], phi2_buffer[x])
    );

  // we do not need these buffers anymore
  if (rank == 0)
  {
    free(edge.phi1);
    free(edge.phi2);
    edge.phi1 = edge.phi2 = 0;
  }
  else
  {
    free(phi1_buffer);
    free(phi2_buffer);
  }
  // MPI_Finalize();
  // return 0;

  // for 3..P iterate
  for (int p = 2; p < pars.P; p++)
  {
    double * row = local_result + x_dim * p;
    // exchange with neighbors
    // get leftmost value
    exchange_left = rank == 0 ? edge.psi2[p - 1]
                              : get_from_local_result(local_result, x_dim, 0, p - 1);
    // get rightmost value
    exchange_right = get_from_local_result(local_result, x_dim, buffer_size - 1, p - 1);
    exchange(&exchange_left, &exchange_right, rank, number_of_processes);
    if (rank == 0)
    {
      exchange_left = edge.psi1[p - 1];
    }
    // calculate edges
    set_to_local_result(local_result, x_dim, 0, p, 
      calculate_iteration(
        get_from_local_result(local_result, x_dim, 0, p - 1),
        get_from_local_result(local_result, x_dim, 0, p - 2),
        exchange_left,
        get_from_local_result(local_result, x_dim, 1, p - 1))
      );
    set_to_local_result(local_result, x_dim, buffer_size - 1, p, 
      calculate_iteration(
        get_from_local_result(local_result, x_dim, buffer_size - 1, p - 1),
        get_from_local_result(local_result, x_dim, buffer_size - 1, p - 2),
        get_from_local_result(local_result, x_dim, buffer_size - 2, p - 1),
        exchange_right)
    );
    // calculate inner parts
    for (int x = 1; x < buffer_size - 1; x++)
    {
      set_to_local_result(local_result, x_dim, x, p, 
        calculate_iteration(
          get_from_local_result(local_result, x_dim, x, p - 1),
          get_from_local_result(local_result, x_dim, x, p - 2),
          get_from_local_result(local_result, x_dim, x - 1, p - 1),
          get_from_local_result(local_result, x_dim, x + 1, p - 1))
      );
    }
  }
  // fan in
  
  for (int k = 1; k < number_of_processes; k <<= 1)
  {
    for (int i = 0; i + k < number_of_processes; i += 2*k)
    {
      int to_receive = min(number_of_processes, i + 2 * k) - (i + k);
      int doubles_to_receive = to_receive * pars.P * doubles_in_part;
      if (rank == i) 
      {
        //recv
        LOG("%d: about to receive %d doubles from %d\n",
            rank, doubles_to_receive, i + k);
        MPI_Recv(
          local_result + buffer_size * pars.P, 
          doubles_to_receive, 
          MPI_DOUBLE,
          i + k,
          TAG,
          MPI_COMM_WORLD,
          &status);
        buffer_size += doubles_to_receive / pars.P;
      }
      if (rank == i + k)
      {
        // send
        LOG("%d: about to send %d doubles to %d\n",
            rank, doubles_to_receive, i);
        MPI_Send(
          local_result,
          doubles_to_receive,
          MPI_DOUBLE,
          i,
          TAG,
          MPI_COMM_WORLD
        );
      }
    }
  }

  // rank=0 write to stdin
  
  if (rank == 0)
  {
    // complete global result with edges
    for (int i = 0; i < pars.P; i++) 
    {
      set_to_local_result(global_result, x_dim, 0, i, edge.psi1[i]);
      set_to_local_result(global_result, x_dim, pars.M - 1, i, edge.psi2[i]);
    }
    // write to stdout
    write_to_std(global_result, &pars, 1);
    free(global_result);
    free_edge(&edge);
  }
  else 
  {
    free(local_result);
  }

  MPI_Finalize();
  return 0;
}

// Process `r` exchanges values with its left and right neighbor
void exchange(double* left, double* right, int rank, int np) 
{
  double leftBuff, rightBuff;
  // if process is single, it exchanges with itself
  if (np == 1)
  {
    leftBuff = *right;
    *right = *left;
    *left = leftBuff;
    return;
  }
  int leftNeighbor = rank - 1 >= 0 ? rank - 1 : np - 1;
  int rightNeighbor  = rank + 1 < np ? rank + 1 : 0;
  MPI_Status status;
  MPI_Sendrecv(
    right, 1, MPI_DOUBLE, rightNeighbor, TAG, 
    &leftBuff, 1, MPI_DOUBLE, leftNeighbor, TAG,
    MPI_COMM_WORLD, &status
  );
  MPI_Sendrecv(
    left, 1, MPI_DOUBLE, leftNeighbor, TAG, 
    &rightBuff, 1, MPI_DOUBLE, rightNeighbor, TAG,
    MPI_COMM_WORLD, &status
  );
  *right = rightBuff;
  *left = leftBuff;
}

void set_to_local_result(double* arr, int x_dim, int m, int p, double value) 
{
  arr[m * pars.P + p] = value; 
}

double get_from_local_result(double* arr, int x_dim, int m, int p)
{
  return arr[m * pars.P + p];
} 