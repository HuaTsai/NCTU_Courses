#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

int isprime(unsigned long long n) {
  unsigned long long i;
  if (n > 10) {
    for (i = 3; i <= (unsigned long long)sqrt(n); i += 2) {
      if (n % i == 0) {
        return 0;
      }
    }
    return 1;
  } else {
    return 0;
  }
}

int main(int argc, char *argv[]) {
  int pc, rank, size;
  unsigned long long n, limit, result = 11;

  sscanf(argv[1], "%llu", &limit);

  pc = 0;

  MPI_Status status;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (rank == 0) {
    printf("Starting. Numbers to be scanned= %llu\n", limit);
    for (n = 11 + rank * 2; n <= limit; n += 2 * size) {
      if (isprime(n)) {
        ++pc;
        result = n;
      }
    }
    int final_pc = pc + 4;
    unsigned long long final_result = result;
    for (int i = 1; i < size; ++i) {
      MPI_Recv(&pc, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &status);
      MPI_Recv(&result, 1, MPI_LONG_LONG_INT, i, 1, MPI_COMM_WORLD, &status);
      final_pc += pc;
      if (result > final_result) {
        final_result = result;
      }
    }
    printf("Done. Largest prime is %llu Total primes %d\n",
            final_result, final_pc);
  } else {
    for (n = 11 + rank * 2; n <= limit; n += 2 * size) {
      if (isprime(n)) {
        ++pc;
        result = n;
      }
    }
    MPI_Send(&pc, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    MPI_Send(&result, 1, MPI_LONG_LONG_INT, 0, 1, MPI_COMM_WORLD);
  }

  MPI_Finalize();

  return 0;
}
