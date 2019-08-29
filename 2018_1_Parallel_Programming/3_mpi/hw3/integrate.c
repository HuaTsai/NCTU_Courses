#include <math.h>
#include <mpi.h>
#include <stdio.h>

#define PI 3.1415926535

int main(int argc, char **argv) {
  unsigned long long i, num_intervals;
  double rect_width, area, sum, x_middle;
  int rank, size;

  sscanf(argv[1], "%llu", &num_intervals);
  rect_width = PI / num_intervals;

  sum = 0;

  MPI_Status status;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  for (i = 1 + rank; i < num_intervals + 1; i += size) {
    x_middle = (i - 0.5) * rect_width;
    area = sin(x_middle) * rect_width;
    sum = sum + area;
  }

  if (rank == 0) {
    double final_sum = sum;
    for (int i = 1; i < size; ++i) {
      MPI_Recv(&sum, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &status);
      final_sum += sum;
    }
    printf("The total area is: %f\n", (float)final_sum);
  } else {
    MPI_Send(&sum, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
  }

  MPI_Finalize();

  return 0;
}
