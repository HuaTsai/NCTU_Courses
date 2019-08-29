#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

long thread_count;
long long n;
double sum;
/* busy waiting */
// int flag;
/* mutex */
pthread_mutex_t mutex;

void *Thread_sum(void *rank) {
  long my_rank = (long)rank;
  double factor;
  long long i;
  long long my_n = n / thread_count;
  long long my_first_i = my_n * my_rank;
  long long my_last_i = my_first_i + my_n;
  double my_sum = 0.;

  if (my_first_i % 2 == 0)
    factor = 1.;
  else
    factor = -1.;

  for (i = my_first_i; i < my_last_i; ++i, factor = -factor) {
    my_sum += factor / (2 * i + 1);
  }
  /* busy waiting */
  /* while (flag != my_rank);
  sum += my_sum;
  flag = (flag + 1) % thread_count;*/
  pthread_mutex_lock(&mutex);
  sum += my_sum;
  pthread_mutex_unlock(&mutex);

  return NULL;
}

int main(int argc, char* argv[]) {
  long thread;
  pthread_t* thread_handles;

  thread_count = strtol(argv[1], NULL, 10);
  n = strtoll(argv[2], NULL, 10);
  thread_handles = (pthread_t*)malloc(thread_count * sizeof(pthread_t));

  sum = 0.;
  /* busy waiting */
  // flag = 0;
  pthread_mutex_init(&mutex, NULL);
  for (thread = 0; thread < thread_count; thread++)
    pthread_create(&thread_handles[thread], NULL, Thread_sum, (void*)thread);
  
  for (thread = 0; thread < thread_count; thread++)
    pthread_join(thread_handles[thread], NULL);
  sum = 4. * sum;
  pthread_mutex_destroy(&mutex);
  free(thread_handles);
  printf("pi = %.20f\n", sum);
}