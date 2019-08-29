# Pthread

## Table of Content

* [Pthread](#pthread)
  * [Table of Content](#table-of-content)
  * [Memory Layout of C Programs](#memory-layout-of-c-programs)
  * [Processes and Threads](#processes-and-threads)
  * [POSIX functions](#posix-functions)
  * [Hello World](#hello-world)
  * [Matrix-Vector Multiplication](#matrix-vector-multiplication)
  * [Calculating Pi](#calculating-pi)
  * [Producer-Consumer Problem](#producer-consumer-problem)
  * [Barriers](#barriers)

## Memory Layout of C Programs

* stack
  * all variables except global/static variables
* heap
  * malloc
* uninitialized data (bss: block started by symbol)
  * uninitialized global/static variables
* initialized data
  * initialized global/static variables
* text

## Processes and Threads

Comparison table between

1. Processes created with fork
2. Threads of a process
3. Ordinary function calls

## POSIX functions

* pthread_create: create a thread
* pthread_detach: set thread to release resources
* pthread_equal: test two thread IDs for equality
* pthread_exit: exit a thread without exiting process
* pthread_kill: send a signal to a thread
* pthread_join: wait for a thread
* pthread_self: find out own thread ID

``` C
int pthread create(
    pthread_t *thread,
    const pthread_attr_t *attr,
    void *(*start_routine) (void *),
    void *arg);

int pthread_join(
    pthread_t thread,
    void **retval);

void pthread_exit(void *retval);

int pthread_cancel(pthread_t thread);

int pthread_attr_init(pthread_attr_t *attr);

int pthread_attr_destroy(pthread_attr_t *attr);

int pthread_attr_getdetachstate(
    const pthread_attr_t *attr,
    int *detachstate);

int pthread_attr_setdetachstate(
    pthread_attr_t *attr,
    int detachstate); // PTHREAD_CREATE_DETACHED or PTHREAD_CREATE_JOINABLE(default)

int pthread_attr_getstacksize(
    const pthread_attr_t *restrict attr,
    size_t *restrict stacksize);

int pthread_attr_setstacksize(
    pthread_attr_t *attr,
    size_t stacksize);

int pthread_attr_getschedpolicy(
    const pthread_attr_t *restrict attr,
    int *restrict policy);

int pthread_attr_setschedpolicy(
    pthread_attr_t *attr,
    int policy);
```

* pthreat_t objects
  * opaque
  * pthread standard guarantees the object store enough information to uniquely identify
* [pth_create.c](pth_create.c)
* Joinable threads: hold exit status until pthread_join call it
* Detached threads: when terminates, be destroyed and release resources
* [pth_attr.c](pth_attr.c)

## Hello World

* [pth_hello.c](hello.c)

## Matrix-Vector Multiplication

* [pth_matrix.c](pth_matrix.c)

## Calculating Pi

* Busy waiting: not gool
  * in specific order
* Mutexes
  * those who has done its tasks can go to critical section first
  * addition is commutative in this case

``` C
int pthread_mutex_init(
    pthread_mutex_t *mutex,
    const pthread_mutexattr_t *attr);

int pthread_mutex_lock(pthread_mutex_t *mutex);

int pthread_mutex_unlock(pthread_mutex_t *mutex);

int pthread_mutex_destroy(pthread_mutex_t *mutex);
```

* [pth_pi.c](pth_pi.c)

## Producer-Consumer Problem

* Busy wait: not work
* Mutex: not work
* Semaphores
  * counter and a task descriptor queue
  * wait and release(post) (originally called P and V by Dijkstra)
  * can be thought as unsigned int
  * 0: wait, not 0: unlock
* Semaphores vs Mutexes
  * there is no ownership associated with a semaphore (?)
  * semaphore can be initialized with lock/unlock
  * semaphore is not only binary mode

``` C
int sem_init(
    sem_t *sem,
    int pshared,
    unsigned int value);

int sem_destroy(sem_t *sem);

int sem_post(sem_t *sem);

int sem_wait(sem_t *sem);
```

* [pth_semaphore.c](pth_semaphore.c)

## Barriers

* Make threads at the same point in a program
* Busy waiting and Mutex
  * it's hard to reuse counter for second barrier

``` C
int counter = 0;
int thread_count;
pthread_mutex_t barrier_mutex;

void *func(void *param) {
  /* Barrier */
  pthread_mutex_lock(&barrier_mutex);
  ++counter;
  pthread_mutex_unlock(&barrier_mutex);
  while (counter < thread_count) {}
}
```

* Semaphores
  * couunter and count_sem can be reused
  * however barrier_sem can not

``` C
int counter = 0;
sem_t count_sem = 1;
sem_t barrier_sem = 0;

void *func(void *param) {
  /* Barrier */
  sem_wait(&count_sem);
  if (counter == thread_count - 1) {
    counter = 0;
    sem_post(&count_sem);
    for (j = 0; j < thread_count - 1; ++j) {
      sem_post(&barrier_sem);
    }
  } else {
    ++counter;
    sem_post(&count_sem);
    sem_wait(&barrier_sem);
  }
}
```

* Conditional Variables
  * pthread_cond_wait() usually placed in while loop
  * if and only if by calling pthread_cond_signal() or pthread_cond_broadcast()
  * then pthread_cond_wait() will be 0

``` C
int pthread_cond_signal(pthread_cond_t *cond);

int pthread_cond_broadcast(pthread_cond_t *cond);

int pthread_cond_wait(
    pthread_cond_t *restrict cond,
    pthread_mutex_t *restrict mutex);

/* same as
pthread_mutex_unlock(&mutex_p);
wait_on_signal(&cond_var_p);
pthread_mutex_lock(&mutex_p);*/

int pthread_cond_init(
    pthread_cond_t *restrict cond,
    const pthread_condattr_t *restrict attr);

int pthread_cond_destroy(pthread_cond_t *cond);

/* not universally available */
int pthread_barrier_init(
    pthread_barrier_t *restrict barrier,
    const pthread_barrierattr_t *restrict attr,
    unsigned count);

int pthread_barrier_wait(pthread_barrier_t *barrier);

int pthread_barrier_destroy(pthread_barrier_t *barrier);
```

``` C
int counter = 0;
pthread_mutex_t mutex;
pthread_cond_t cond_var;

void ∗func(void *param) {
  /* Barrier */
  pthread_mutex_lock(&mutex);
  ++counter;
  if (counter == thread_count) {
    counter = 0;
    pthread_cond_broadcast(&cond_var);
  } else {
    while (pthread_cond_wait(&cond_var, &mutex) != 0) {}
  }
  pthread_mutex_unlock(&mutex);
}
```
