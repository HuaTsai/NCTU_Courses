#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <CL/cl.h>
#include <string.h>
#include <fstream>
#include <iostream>

char *ConvertCLFileToString(const char *filename) {
  size_t size;
  char *str;
  std::fstream cl_file(filename, std::ios_base::in);
  if (cl_file.is_open()) {
    cl_file.seekg(0, std::fstream::end);
    size = (size_t)cl_file.tellg();
    cl_file.seekg(0, std::fstream::beg);
    str = new char[size + 1];
    if (!str) {
      cl_file.close();
      return NULL;
    }
    cl_file.read(str, size);
    cl_file.close();
    str[size] = '\0';
    return str;
  }
}

int main() {
  unsigned int *histogram = new unsigned int[256 * 3];
  unsigned int i = 0, tmp;
  size_t buf_size;
  size_t input_size;
  size_t output_size = 256 * 3 * sizeof(unsigned int);
  std::fstream inFile("input", std::ios_base::in);
  std::ofstream outFile("0750727.out", std::ios_base::out);

  inFile >> buf_size;
  unsigned int *image = new unsigned int[buf_size];
  input_size = buf_size * sizeof(unsigned int);
  while (inFile >> tmp) {
    image[i++] = tmp;
  }

  cl_platform_id platform_id;
  cl_device_id device;
  clGetPlatformIDs(1, &platform_id, NULL);
  clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
  cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, NULL);
  cl_command_queue queue = clCreateCommandQueue(context, device, 0, NULL);

  cl_mem image_input =
      clCreateBuffer(context, CL_MEM_READ_ONLY, input_size, NULL, NULL);
  cl_mem histogram_output =
      clCreateBuffer(context, CL_MEM_WRITE_ONLY, output_size, NULL, NULL);
  clEnqueueWriteBuffer(queue, image_input, CL_TRUE, 0, input_size,
                       (void *)image, 0, NULL, NULL);

  const char *source = ConvertCLFileToString("histogram.cl");
  size_t source_size = strlen(source);
  cl_program program =
      clCreateProgramWithSource(context, 1, &source, &source_size, NULL);
  clBuildProgram(program, 1, &device, NULL, NULL, NULL);
  cl_kernel kernel = clCreateKernel(program, "histogram", NULL);

  clSetKernelArg(kernel, 0, sizeof(image_input), (void *)&image_input);
  clSetKernelArg(kernel, 1, sizeof(histogram_output), (void *)&histogram_output);
  clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &buf_size, NULL, 0, NULL, NULL);

  clEnqueueReadBuffer(queue, histogram_output, CL_TRUE, 0, output_size,
                      (void *)histogram, 0, NULL, NULL);

  for (unsigned int i = 0; i < 256 * 3; ++i) {
    if (i % 256 == 0 && i != 0) outFile << std::endl;
    outFile << histogram[i] << ' ';
  }
  inFile.close();
  outFile.close();
}
