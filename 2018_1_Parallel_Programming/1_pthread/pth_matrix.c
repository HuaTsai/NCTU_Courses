#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

int thread_count;
int m, n;
double *A;
double *x;
double *y;

void *Pth_mat_vect(void *rank) {
  long my_rank = (long)rank;
  int i, j;
  int local_m = m / thread_count;
  int my_first_row = my_rank * local_m;
  int my_last_row = (my_rank + 1) * local_m - 1;
  for (i = my_first_row; i <= my_last_row; i++) {
    y[i] = 0.;
    for (j = 0; j < n; j++) {
      y[i] += A[i * n + j] * x[j];
    }
  }
}

void Read_matrix(char *prompt, double A[], unsigned m, unsigned n) {
  unsigned i, j;
  printf("%s\n", prompt);
  for (i = 0; i < m; i++) {
    for (j = 0; j < n; j++) {
      scanf("%lf", &A[i * n + j]);
    }
  }
}

void Read_vector(char *prompt, double x[], unsigned n) {
  unsigned i;
  printf("%s\n", prompt);
  for (i = 0; i < n; i++) {
    scanf("%lf", &x[i]);
  }
}

void Print_matrix(char *title, double A[], int m, int n) {
  int i, j;
  printf("%s\n", title);
  for (i = 0; i < m; i++) {
    for (j = 0; j < n; j++) {
      printf("%6.3f ", A[i * n + j]);
    }
    printf("\n");
  }
}

void Print_vector(char *title, double y[], unsigned m) {
  unsigned i;
  printf("%s\n", title);
  for (i = 0; i < m; i++) {
    printf("%4.1f ", y[i]);
  }
  printf("\n");
}

int main(int argc, char *argv[]) {
  long thread;
  pthread_t *thread_handles;
  thread_count = atoi(argv[1]);
  m = strtol(argv[2], NULL, 10);
  n = strtol(argv[3], NULL, 10);
  thread_handles = malloc(thread_count * sizeof(pthread_t));
  A = malloc(m * n * sizeof(double));
  x = malloc(n * sizeof(double));
  y = malloc(m * sizeof(double));
  Read_matrix("Enter the matrix", A, m, n);
  Read_vector("Enter the vector", x, n);
  for (thread = 0; thread < thread_count; thread++) {
    pthread_create(&thread_handles[thread], NULL, Pth_mat_vect, (void *)thread);
  }
  for (thread = 0; thread < thread_count; thread++) {
    pthread_join(thread_handles[thread], NULL);
  }
  Print_vector("result", y, m);
}
