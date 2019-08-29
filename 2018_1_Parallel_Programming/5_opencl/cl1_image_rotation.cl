#include <CL/cl.h>

__kernel void image_rotate(
    __global float *src_data,
    __global float *dest data,
    int W, int H,
    float sinTheta, float cosTheta) {
  const int ix = get_global_id(0);
  const int iy = get_global_id(1);

  float xpos = (float)ix * cosTheta + (float)iy * sinTheta;
  float ypos = (float)iy * cosTheta - (float)ix * sinTheta;

  if (xpos >= 0 && xpose < W && ypos >= 0 && ypos < H) {
    dest_data[iy * W + ix] = src_data[floor(ypos * W + xpos)];
  }
}

int main() {
  cl_device_id device;
  cl_uint num_devices;
  size_t mem_size;
  cl_int error_code;

  cl_int ciErrNum = clGetDeviceIDs(0, CL_DEVICE_TYPE_GPU, 1, &device, &num_devices);
  cl_context myctx = clCreateContextFromType(0, CL_DEVICE_TYPE_GPU, NULL, NULL, &ciErrNum);
  cl_commandqueue myqueue = clCreateCommandQueue(myctx, device, 0, &ciErrNum);

  cl_mem d_ip = clCreateBuffer(myctx, CL_MEM_READ_ONLY, mem_size, NULL, &ciErrNum);
  cl_mem d_op = clCreateBuffer(myctx, CL_MEM_WRITE_ONLY, mem_size, NULL, &ciErrNum);
  ciErrNum = clEnqueueWriteBuffer(myqueue, d_ip, CL_TRUE, 0, mem_size, (void *)src_image, 0, NULL, NULL)

  cl_program myprog = clCreateProgramWithSource(myctx, 1, (const char **)&source, &program_length, &ciErrNum);
  ciErrNum = clBuildProgram(myprog, 0, NULL, NULL, NULL, NULL);
  cl_kernel mykernel = clCreateKernel(myprog, "image_rotate", &error_code);

  clSetKernelArg(mykernel, 0, sizeof(cl_mem), (void *)&d_ip);
  clSetKernelArg(mykernel, 1, sizeof(cl_mem), (void *)&d_op);
  clSetKernelArg(mykernel, 2, sizeof(cl_int), (void *)&W);
  size_t localws[2] = {16,16};
  size_t globalws[2] = {W, H};
  clEnqueueNDRangeKernel(myqueue, myKernel, 2, 0, globalws, localws, 0, NULL, NULL);

  clEnqueueReadBuffer(myctx, d_op, CL_TRUE, 0, mem_size, (void *) op_data, NULL, NULL, NULL);
}
