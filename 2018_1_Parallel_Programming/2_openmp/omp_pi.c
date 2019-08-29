#include <omp.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#define NUM_THREADS 2

int main() {
  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);

  int nthreads;
  long num_steps = 100000000;
  double sum[NUM_THREADS] = {0.f};
  double step, pi = 0.f;

  step = 1.f / (double)num_steps;
  omp_set_num_threads(NUM_THREADS);
#pragma omp parallel
  {
    int id, nthrds;
    double x;
    id = omp_get_thread_num();
    nthrds = omp_get_num_threads();
    if (id == 0) {
      nthreads = nthrds;
    }
    for (int i = 0; i < num_steps; i += nthrds) {
      x = (i + 0.5) * step;
      sum[id] += 4.f / (1.f + x * x);
    }
  }
  for (int i = 0; i < nthreads; ++i) {
    pi += sum[i] * step;
  }
  printf("pi = %.10f\n", pi);

  clock_gettime(CLOCK_MONOTONIC, &end);
  double diff =
      (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.f;
  printf("%f s\n", diff);
}