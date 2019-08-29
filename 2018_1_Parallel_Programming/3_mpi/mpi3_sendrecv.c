#include <mpi.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
  int rank, size;
  int tag = 0;
  char message[100];
  MPI_Status status;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  if (rank != 0) {
    snprintf(message, sizeof(message), "Greetings from process %d!", rank);
    int dest = 0;
    MPI_Send(message, strlen(message) + 1, MPI_CHAR, dest, tag, MPI_COMM_WORLD);
  } else {
    for (int src = 1; src < size; ++src) {
      MPI_Recv(message, 100, MPI_CHAR, src, tag, MPI_COMM_WORLD, &status);
      printf("%s\n", message);
    }
    printf("Greetings from process %d!\n", rank);
  }
  MPI_Finalize();
}
