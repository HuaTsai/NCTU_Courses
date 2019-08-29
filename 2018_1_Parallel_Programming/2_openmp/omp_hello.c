#include <omp.h>
#include <stdio.h>

int thread_count = 4;

int main() {
  // omp_set_num_threads(4);
#pragma omp parallel num_threads(thread_count)
  {
    int ID = omp_get_thread_num();
    printf("Hello from thread %d of %d\n", ID, thread_count);
  }
}