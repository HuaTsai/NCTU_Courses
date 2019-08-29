#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
long long num_of_tosses;
long long step_count;
long thread_count;

void *func(void *param) {
  int thread = (int)(long)param;
  long long first = (long)param * step_count;
  long long last;
  long long local_num_in_circle = 0;
  if ((long)param == thread_count - 1) {
    last = num_of_tosses;
  } else {
    last = first + step_count;
  }  
  for (long long i = first; i < last; ++i) {
    double x = (double)rand_r(&thread) / RAND_MAX;
    double y = (double)rand_r(&thread) / RAND_MAX;
    if (x * x + y * y <= 1) {
      ++local_num_in_circle;
    }
  }
  return (void *)local_num_in_circle;
}

int main(int argc, char *argv[]) {
  thread_count = atol(argv[1]);
  num_of_tosses = atoll(argv[2]);
  long long num_in_circle = 0;
  long long local_num_in_circle[thread_count];
  pthread_t thread_handles[thread_count];
  step_count = num_of_tosses / thread_count;

  for (long i = 0; i < thread_count; ++i) {
    pthread_create(&thread_handles[i], NULL, func, (void *)i);
  }

  for (long i = 0; i < thread_count; ++i) {
    pthread_join(thread_handles[i], (void **)&local_num_in_circle[i]);
  }

  for (long i = 0; i < thread_count; ++i) {
    num_in_circle += local_num_in_circle[i];
  }

  double pi_estimate = 4.f * num_in_circle / num_of_tosses;
  printf("pi = %f\n", pi_estimate);
}
