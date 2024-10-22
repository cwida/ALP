#include "alp.cuh"

#include "../common/utils.hpp"

#ifndef ALP_TEST_KERNELS_GLOBAL_CUH
#define ALP_TEST_KERNELS_GLOBAL_CUH

namespace alp {
namespace kernels {
namespace global {
namespace test {

template <typename T, typename UINT_T, int UNPACK_N_VECTORS,
          int UNPACK_N_VALUES>
__global__ void decode_complete_alp_vector(T *out, AlpColumn<T> data) {
  constexpr uint8_t LANE_BIT_WIDTH = utils::sizeof_in_bits<T>();
  constexpr uint32_t N_LANES = utils::get_n_lanes<T>();
  constexpr uint32_t N_VALUES_IN_LANE = utils::get_values_per_lane<T>();

  const int16_t lane = threadIdx.x % N_LANES;
  const int16_t vector_index = threadIdx.x / N_LANES;
  const int32_t block_index = blockIdx.x;

  constexpr int32_t n_vectors_per_block = UNPACK_N_VECTORS;

  out += (block_index * n_vectors_per_block + vector_index) *
         consts::VALUES_PER_VECTOR;

  for (int i = 0; i < N_VALUES_IN_LANE; i += UNPACK_N_VALUES) {
    unalp<UINT_T, T, UnpackingType::VectorArray, UNPACK_N_VECTORS,
          UNPACK_N_VALUES>(out, data, block_index, lane, i);
    out += UNPACK_N_VALUES * N_LANES;
  }
}

template <typename T, typename UINT_T, int UNPACK_N_VECTORS,
          int UNPACK_N_VALUES>
__global__ void decode_alp_vector_into_lane(T *out, AlpColumn<T> data) {
  constexpr uint8_t LANE_BIT_WIDTH = utils::sizeof_in_bits<T>();
  constexpr uint32_t N_LANES = utils::get_n_lanes<T>();
  constexpr uint32_t N_VALUES = UNPACK_N_VALUES * UNPACK_N_VECTORS;
  constexpr uint32_t N_VALUES_IN_LANE = utils::get_values_per_lane<T>();

  const int16_t lane = threadIdx.x % N_LANES;
  const int32_t block_index = blockIdx.x;
  constexpr int32_t n_vectors_per_block = UNPACK_N_VECTORS;
  const int16_t vector_index =
      block_index * n_vectors_per_block + (threadIdx.x / N_LANES);

  T registers[N_VALUES];
  out += vector_index * consts::VALUES_PER_VECTOR;

  for (int i = 0; i < N_VALUES_IN_LANE; i += UNPACK_N_VALUES) {
    unalp<UINT_T, T, UnpackingType::LaneArray, UNPACK_N_VECTORS,
          UNPACK_N_VALUES>(registers, data, block_index, lane, i);

    for (int i = 0; i < UNPACK_N_VALUES; i++) {
      out[lane + i * N_LANES] = registers[i];
    }

    out += UNPACK_N_VALUES * N_LANES;
  }
}

template <typename T, typename UINT_T, int UNPACK_N_VECTORS,
          int UNPACK_N_VALUES>
__global__ void decode_alp_vector_with_state(T *out, AlpColumn<T> data) {
  constexpr uint8_t LANE_BIT_WIDTH = utils::sizeof_in_bits<T>();
  constexpr uint32_t N_LANES = utils::get_n_lanes<T>();
  constexpr uint32_t N_VALUES = UNPACK_N_VALUES * UNPACK_N_VECTORS;
  constexpr uint32_t N_VALUES_IN_LANE = utils::get_values_per_lane<T>();

  const int16_t lane = threadIdx.x % N_LANES;
  const int32_t block_index = blockIdx.x;
  constexpr int32_t n_vectors_per_block = UNPACK_N_VECTORS;
  const int16_t vector_index =
      block_index * n_vectors_per_block + (threadIdx.x / N_LANES);

  T registers[N_VALUES];
  out += vector_index * consts::VALUES_PER_VECTOR;

  auto iterator =
      Unpacker<UINT_T, T, UnpackingType::LaneArray, UNPACK_N_VECTORS,
               UNPACK_N_VALUES>(vector_index, lane, data);

  for (int i = 0; i < N_VALUES_IN_LANE; i += UNPACK_N_VALUES) {
    iterator.unpack_next_into(registers);

    for (int i = 0; i < UNPACK_N_VALUES; i++) {
      out[lane + i * N_LANES] = registers[i];
    }

    out += UNPACK_N_VALUES * N_LANES;
  }
}

template <typename T, typename UINT_T, int UNPACK_N_VECTORS,
          int UNPACK_N_VALUES>
__global__ void decode_complete_alprd_vector(T *out, AlpRdColumn<T> data) {
  constexpr uint8_t LANE_BIT_WIDTH = utils::sizeof_in_bits<T>();
  constexpr uint32_t N_LANES = utils::get_n_lanes<T>();
  constexpr uint32_t N_VALUES_IN_LANE = utils::get_values_per_lane<T>();

  const int16_t lane = threadIdx.x % N_LANES;
  const int16_t vector_index = threadIdx.x / N_LANES;
  const int32_t block_index = blockIdx.x;

  constexpr int32_t n_vectors_per_block = UNPACK_N_VECTORS;

  out += (block_index * n_vectors_per_block + vector_index) *
         consts::VALUES_PER_VECTOR;

  for (int i = 0; i < N_VALUES_IN_LANE; i += UNPACK_N_VALUES) {
    unalprd<UINT_T, T, UnpackingType::VectorArray, UNPACK_N_VECTORS,
            UNPACK_N_VALUES>(out, data, block_index, lane, i);
    out += UNPACK_N_VALUES * N_LANES;
  }
}

} // namespace test
} // namespace global
} // namespace kernels
} // namespace alp

#endif // ALP_TEST_KERNELS_GLOBAL_CUH
