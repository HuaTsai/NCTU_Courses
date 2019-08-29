# OpenMP

## Table of Content

* [OpenMP](#openmp)
  * [Table of Content](#table-of-content)
  * [Compilier Directives](#compilier-directives)
    * [Creating Threads](#creating-threads)
    * [Synchronization](#synchronization)
    * [Worksharing Constructs](#worksharing-constructs)
    * [More on Synchronization](#more-on-synchronization)
    * [Variable Scopes (shared/private)](#variable-scopes-sharedprivate)
  * [Run-time Libraries and Environment Variables](#run-time-libraries-and-environment-variables)
  * [Task](#task)
    * [Task Constructs](#task-constructs)

## Compilier Directives

Components:

1. Set of compiler directives (80%)
2. Library routines (19%)
3. Environment variables (1%)

### Creating Threads

* [omp_hello.c](omp_hello.c)
* [omp_pi.c](omp_pi.c)
* default: create same number as core
* num_threads(4) = main thread + three new threads
* implementation: thread pool -> less cost of creation and destruction
* SPMD: Single Program Multiple Data
* False sharing: cache lines update issue
  * solution: pad arrays to make variables in different cache lines
  * [omp_pi_pad.c](omp_pi_pad.c)

``` C
void omp_set_num_threads(int num_threads);

int omp_get_num_threads(void);

int omp_get_thread_num(void);

#pragma omp parallel
{
    /* parallel region */
}
```

### Synchronization

* critical
  * larger overhead
  * unnamed crical sections are considered identical
* atomic
  * lower overhead
  * update of only one memory location
  * may not portable
* some caveats
  * shouldn’t mix the different types for a single critical section
  * no guarantee of fairness in mutual exclusion constructs
  * deadlock issue
* [omp_pi_sync.c](omp_pi_sync.c)

``` C
#pragma omp critical(name)

#pragma omp atomic
```

### Worksharing Constructs

* loop construct
  * threads wait until the end of worksharing construct
  * schedule clause
    1. schedule(static[, chuck])
       * deal-out blocks of iterations of size chuck
       * schedule done at comile-time
    2. schedule(dynamic[, chuck])
       * grab size chuck in iteration queue
       * schedule at run-time
    3. schedule(guided[, chuck])
       * grab blocks of iterations, starts from large and shrinks down to size chunk
    4. schedule(runtime)
       * schedule from OMP_SCHEDULE
    5. schedule(auto)
       * left up to the runtime to choose
    * remember to make the loop iterations independent
    * reduction is one of the dependent loop
    * the iterations of the loop should be determined
    * openmp can only handle for loop with canonical form
    * [omp_pi_reduction.c](omp_pi_reduction.c)

``` C
#pragma omp parallel for
for(int i = 0; i < n; ++i) {
}

// nested loops
#pragma omp parallel for collapse(2)
for(int i = 0; i < n; ++i) {
    for (int j = 0; j < m; ++j) {
    }
}

// reduction
#pragma omp parallel for reduction (+:ave)
for (int i = 0; i < n; ++i) {
    ave += A[i];
}
```

* master/single construct
  * master construct: the block is only executed by master thread
  * master construct: the block is only executed by one thread

``` C
#pragma omp master
{
    /* master thread */
}

#pragma omp single
{
    /* any but only one thread */
}
```

* sections/section constructs
  * sections
    * inside parallel region
    * encompasses each section
    * has barrier at the end, use "nowait" to disable it
  * section

``` C
#pragma omp parallel sections
{
#pragma omp section
    double a = alice();
#pragma omp section
    double b = bob();
#pragma omp section
    double c = cy();
}
```

### More on Synchronization

* barrier

``` C
#pragma omp parallel shared(A, B, C) private(id)
{
    id = omp_get_thread_num();
    A[id] = big_calc1(id);
#pragma omp barrier
#pragma omp for
    for (int i = 0; i < n; ++i) {
        C[i] = big_calc3(i,A);
    }
#pragma omp for nowait
    for (int i = 0; i < n; ++i){
        B[i] = big_calc2(C, i);
    }
    A[id] = big_calc4(id);
}
```

* ordered

``` C
#pragma omp parallel private (tmp)
#pragma omp for ordered reduction(+:res)
for (int i = 0; i < n; ++i) {
    tmp = foo(i);
#pragma omp ordered
    res += consum(tmp);
}
```

### Variable Scopes (shared/private)

* shared variables
  * global variable
  * static variable
  * dynamically allocated memory
* storage attributes clauses
  * shared: parallel
  * private: parallel & worksharing
  * firstprivate: parallel & worksharing
  * lastprivate: parallel & worksharing
  * default(private | shared | none)
* private(var)
  * create local copy of var to each threads
* firstprivate(var)
  * same as private(var) but with initialized from shared variable
* lastprivate(var)
  * update shared variable from last iteration
* default(shared)
  * default attribute
* default(private)
  * each variable in construct is made private
  * support only for Fortran not C/C++
* default(none)
  * no default for variables in static extent
* [omp_pi_variable.c](omp_pi_variable.c)

## Run-time Libraries and Environment Variables

* synchronization: lock routines
  * omp_init_lock()
  * omp_set_lock()
  * omp_unset_lock()
  * omp_test_lock()
  * omp_destroy_lock()
* nested locks
  * omp_init_nest_lock()
  * omp_set_nest_lock()
  * omp_unset_nest_lock()
  * omp_test_nest_lock()
  * omp_destroy_nest_lock()

``` C
omp_lock_t lck;
omp_init_lock(&lck);
#pragma omp parallel private(tmp, id)
{
    id = omp_get_thread_num();
    // process
    omp_set_lock(&lck);
    printf("%d %d", id, tmp);
    omp_unset_lock(&lck);
}
omp_destroy_lock(&lck);
```

* runtime library routines
  * modify/check threads
    * omp_set_num_threads()
    * omp_get_num_threads()
    * omp_get_thread_num()
    * omp_get_max_threads()
  * in parallel region?
    * omp_in_parallel()
  * dynamically vary the number of threads from one parallel construct to another
    * omp_set_dynamic()
    * omp_get_dynamic()
  * number of processors
    * omp_num_procs()

``` C
int main() {
    int num_threads;
    omp_set_dynamic(0);
    omp_set_num_threads(omp_get_num_procs());
#pragma omp parallel
    {
        int id = omp_get_thread_num();
#pragma omp single
        num_threads = omp_get_num_threads();
    }
}
```

* environment variables
  * OMP_NUM_THREADS
  * OMP_STACKSIZE
  * OMP_WAIT_POLICY
    * ACTIVE: keep threads alive at barriers/locks
    * PASSIVE: try to release processor at barriers/locks
  * OMP_SCHEDULE
    * control schedule(runtime)

## Task

* data vs task Parallelism
* thread encountering parallel construct -> implicit task
* barrier holds original master thread until all implicit tasks are finished
* OpenMP 3.0
  * unbounded loops
  * recursive algorithms
  * producer/consumer
* tasks are composed of
  * code to execute
  * data environment
  * ICV: internal control variables

### Task Constructs

* clauses
  * shared(list)
  * private(list)
  * firstprivate(list)
  * default(shared | none)
  * if (expression)
  * untied
  * final (expression)
  * mergeable

``` C
// list processing
#pragma omp parallel
{
#pragma omp single
    {
        node *p = head;
        while (0) {
#pragma omp task firstprivate(p)
            process(0);
            p = p->next;
        }
    }
}

//
```
