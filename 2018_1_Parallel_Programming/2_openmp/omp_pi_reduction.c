#include <omp.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#define NUM_THREADS 2

int main() {
  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);

  long num_steps = 100000000;
  double step, pi = 0.f;

  step = 1.f / (double)num_steps;
  omp_set_num_threads(NUM_THREADS);
#pragma omp parallel for reduction(+ : pi)
  for (int i = 0; i < num_steps; ++i) {
    double x = (i + 0.5) * step;
    pi += 4.f / (1.f + x * x) * step;
  }
  printf("pi = %.10f\n", pi);

  clock_gettime(CLOCK_MONOTONIC, &end);
  double diff =
      (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.f;
  printf("%f s\n", diff);
}