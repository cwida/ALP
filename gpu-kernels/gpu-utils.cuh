#include <__clang_cuda_runtime_wrapper.h>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cuda.h>
#include <cuda_runtime.h>
#include <iostream>
#include <stdio.h>

#ifndef GPU_UTILS_H
#define GPU_UTILS_H

#define CUDA_SAFE_CALL(call)                                                   \
  do {                                                                         \
    cudaError_t err = call;                                                    \
    if (cudaSuccess != err) {                                                  \
      fprintf(stderr, "Cuda error in file '%s' in line %i : %s.", __FILE__,    \
              __LINE__, cudaGetErrorString(err));                              \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
  } while (0)

template <typename T> void free_device_pointer(T *&device_ptr) {
  if (device_ptr != nullptr) {
    CUDA_SAFE_CALL(cudaFree(device_ptr));
  }
  device_ptr = nullptr;
}

template <typename T> class GPUArray {
private:
  size_t count;
  T *device_ptr = nullptr;

  void allocate() {
    CUDA_SAFE_CALL(
        cudaMalloc(reinterpret_cast<void **>(&device_ptr), count * sizeof(T)));
  }

public:
  GPUArray(const size_t a_count) {
    count = a_count;
    allocate();
  }

  GPUArray(const size_t a_count, const T *host_p) {
    count = a_count;
    allocate();
    CUDA_SAFE_CALL(cudaMemcpy(device_ptr, host_p, count * sizeof(T),
                              cudaMemcpyHostToDevice));
  }

  // Copy constructor
  GPUArray(const GPUArray &) = delete;
  // Assignment operator deleted
  GPUArray &operator=(const GPUArray &) = delete;

  // Move constructor
  GPUArray(GPUArray &&other) noexcept : device_ptr(other.device_ptr) {
    other.device_ptr = nullptr;
  }

  // Assignment operator
  GPUArray &operator=(GPUArray &&other) noexcept {
    if (this != &other) {
      free_device_pointer(device_ptr);
      device_ptr = other.device_ptr;
      other.device_ptr = nullptr;
    }
    return *this;
  }

  ~GPUArray() {
    free_device_pointer(device_ptr);
  }

  void copy_to_host(T *host_p) {
    CUDA_SAFE_CALL(cudaMemcpy(host_p, device_ptr, count * sizeof(T),
                              cudaMemcpyDeviceToHost));
  }

  T *get() { return device_ptr; }

  T *release() {
    auto temp = device_ptr;
    device_ptr = nullptr;
    return temp;
  }
};

#endif // GPU_UTILS_H
