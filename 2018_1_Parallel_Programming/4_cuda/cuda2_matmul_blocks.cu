#include <cuda_runtime.h>
#include <iostream>

#define WIDTH 15
#define TILE_WIDTH 5

void MatrixMulOnDevice(float *M, float *N, float *P, int Width);
__global__ void MatrixMulKernel(float *Md, float *Nd, float *Pd, int Width);
void PrintMatrix(float *X, int Width, char ch);

int main() {
  float A[WIDTH * WIDTH];
  float B[WIDTH * WIDTH];
  float C[WIDTH * WIDTH];
  for (int i = 0; i < WIDTH; ++i) {
    for (int j = 0; j < WIDTH; ++j) {
      A[i * WIDTH + j] = i * WIDTH + j;
      B[i * WIDTH + j] = i * WIDTH + j;
    }
  }
  MatrixMulOnDevice(A, B, C, WIDTH);
  PrintMatrix(A, WIDTH, 'A');
  PrintMatrix(B, WIDTH, 'B');
  PrintMatrix(C, WIDTH, 'C');
}

void MatrixMulOnDevice(float *M, float *N, float *P, int Width) {
  int size = Width * Width * sizeof(float);
  float *Md, *Nd, *Pd;
  cudaMalloc(&Md, size);
  cudaMalloc(&Nd, size);
  cudaMalloc(&Pd, size);

  cudaMemcpy(Md, M, size, cudaMemcpyHostToDevice);
  cudaMemcpy(Nd, N, size, cudaMemcpyHostToDevice);

  dim3 dimGrid(Width / TILE_WIDTH, Width / TILE_WIDTH);
  dim3 dimBlock(TILE_WIDTH, TILE_WIDTH);
  MatrixMulKernel<<<dimGrid, dimBlock>>>(Md, Nd, Pd, Width);

  cudaMemcpy(P, Pd, size, cudaMemcpyDeviceToHost);
  cudaFree(Md);
  cudaFree(Nd);
  cudaFree(Pd);
}

__global__ void MatrixMulKernel(float *Md, float *Nd, float *Pd, int Width) {
  int row = blockIdx.y * blockDim.y + threadIdx.y;
  int col = blockIdx.x * blockDim.x + threadIdx.x;
  float Pvalue = 0;
  for (int k = 0; k < Width; ++k) {
    Pvalue += Md[row * Width + k] * Nd[k * Width + col];
  }
  Pd[row * Width + col] = Pvalue;
}

void PrintMatrix(float *X, int Width, char ch) {
  std::cout << ch << ":" << std::endl;
  for (int i = 0; i < Width; ++i) {
    for (int j = 0; j < Width; ++j) {
      std::cout << X[i * Width + j] << " ";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}
