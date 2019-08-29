/**********************************************************************
 * DESCRIPTION:
 *   Serial Concurrent Wave Equation - C Version
 *   This program implements the concurrent wave equation
 *********************************************************************/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cuda_runtime.h>

#define MAXPOINTS 1000000
#define MAXSTEPS 1000000
#define MINPOINTS 20
#define PI 3.14159265

void check_param(void);
void init_line(void);
void update(void);
void printfinal(void);

int nsteps,                  /* number of time steps */
    tpoints,                 /* total points along string */
    rcode;                   /* generic return code */
float values[MAXPOINTS + 1], /* values at time t */
    oldval[MAXPOINTS + 1];   /* values at time (t-dt) */

/**********************************************************************
 *	Checks input values from parameters
 *********************************************************************/
void check_param(void) {
  char tchar[20];

  /* check number of points, number of iterations */
  while ((tpoints < MINPOINTS) || (tpoints > MAXPOINTS)) {
    printf("Enter number of points along vibrating string [%d-%d]: ", MINPOINTS,
           MAXPOINTS);
    scanf("%s", tchar);
    tpoints = atoi(tchar);
    if ((tpoints < MINPOINTS) || (tpoints > MAXPOINTS))
      printf("Invalid. Please enter value between %d and %d\n", MINPOINTS,
             MAXPOINTS);
  }
  while ((nsteps < 1) || (nsteps > MAXSTEPS)) {
    printf("Enter number of time steps [1-%d]: ", MAXSTEPS);
    scanf("%s", tchar);
    nsteps = atoi(tchar);
    if ((nsteps < 1) || (nsteps > MAXSTEPS))
      printf("Invalid. Please enter value between 1 and %d\n", MAXSTEPS);
  }

  printf("Using points = %d, steps = %d\n", tpoints, nsteps);
}

/**********************************************************************
 *     Initialize points on line
 *********************************************************************/
void init_line(void) {
  int i, j;
  float x, fac, k, tmp;

  /* Calculate initial values based on sine curve */
  fac = 2.0 * PI;
  k = 0.0;
  tmp = tpoints - 1;
  for (j = 1; j <= tpoints; j++) {
    x = k / tmp;
    values[j] = sin(fac * x);
    k = k + 1.0;
  }

  /* Initialize old values array */
  for (i = 1; i <= tpoints; i++) oldval[i] = values[i];
}

/**********************************************************************
 *     Update all values along line a specified number of times
 *********************************************************************/
__global__ void updateKernel(float *oldvald, float *valuesd, int nsteps, int tpoints) {
  int idx = blockIdx.x * blockDim.x + threadIdx.x;
  float newval;
  for (int i = 1; i <= nsteps; ++i) {
    if (idx == 1 || idx == tpoints)
      newval = 0.0;
    else
      newval = 1.82 * valuesd[idx] - oldvald[idx];
    oldvald[idx] = valuesd[idx];
    valuesd[idx] = newval;
  }
}

/**********************************************************************
 *     Print final results
 *********************************************************************/
void printfinal() {
  int i;

  for (i = 1; i <= tpoints; i++) {
    printf("%6.4f ", values[i]);
    if (i % 10 == 0) printf("\n");
  }
}

/**********************************************************************
 *	Main program
 *********************************************************************/
int main(int argc, char *argv[]) {
  float *oldvald, *valuesd;
  sscanf(argv[1], "%d", &tpoints);
  sscanf(argv[2], "%d", &nsteps);
  check_param();
  printf("Initializing points on the line...\n");
  init_line();
  printf("Updating all points for all time steps...\n");

  int blocksize = 1024;
  int gridsize = (tpoints + blocksize) / blocksize;
  int size = blocksize * gridsize * sizeof(float);
  cudaMalloc(&oldvald, size);
  cudaMalloc(&valuesd, size);
  cudaMemcpy(oldvald, oldval, size, cudaMemcpyHostToDevice);
  cudaMemcpy(valuesd, values, size, cudaMemcpyHostToDevice);

  updateKernel<<<gridsize, blocksize>>>(oldvald, valuesd, nsteps, tpoints);
  cudaMemcpy(values, valuesd, size, cudaMemcpyDeviceToHost);

  cudaFree(oldvald);
  cudaFree(valuesd);

  printf("Printing final results...\n");
  printfinal();
  printf("\nDone.\n\n");

  return 0;
}
