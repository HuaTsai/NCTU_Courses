__kernel void histogram(
    __global unsigned int *image_input,
    __global unsigned int *histogram_output) {
  const int idx = get_global_id(0);
  int idx_rgb = image_input[idx] + (idx % 3) * 256;
  atomic_add(&histogram_output[idx_rgb], 1);
}
