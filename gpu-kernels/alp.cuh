#include <cstdint>
#include <cstdio>
#include <type_traits>

#include "../alp/constants.hpp"
#include "../common/utils.hpp"
#include "../gpu-fls/fls.cuh"
#include "src/alp/config.hpp"

#ifndef ALP_CUH
#define ALP_CUH

template <typename T> struct AlpColumn {
  using UINT_T = typename utils::same_width_uint<T>::type;

  UINT_T *ffor_array;
  UINT_T *ffor_bases;
  uint8_t *bit_widths;
  uint8_t *exponents;
  uint8_t *factors;

  T *exceptions;
  uint16_t *positions;
  uint16_t *counts;
};

template <typename T> struct AlpRdColumn {
  using UINT_T = typename utils::same_width_uint<T>::type;

  uint16_t *left_ffor_array;
  uint16_t *left_ffor_bases;
  uint8_t *left_bit_widths;

  UINT_T *right_ffor_array;
  UINT_T *right_ffor_bases;
  uint8_t *right_bit_widths;

  uint16_t *left_parts_dicts;

  uint16_t *exceptions;
  uint16_t *positions;
  uint16_t *counts;
};

namespace constant_memory {
constexpr int32_t F_FACT_ARR_COUNT = 10;
constexpr int32_t F_FRAC_ARR_COUNT = 11;
__constant__ int32_t F_FACT_ARRAY[F_FACT_ARR_COUNT];
__constant__ float F_FRAC_ARRAY[F_FRAC_ARR_COUNT];

constexpr int32_t D_FACT_ARR_COUNT = 19;
constexpr int32_t D_FRAC_ARR_COUNT = 21;
__constant__ int64_t D_FACT_ARRAY[D_FACT_ARR_COUNT];
__constant__ double D_FRAC_ARRAY[D_FRAC_ARR_COUNT];

template <typename T> __host__ void load_alp_constants() {
  cudaMemcpyToSymbol(F_FACT_ARRAY, alp::Constants<float>::FACT_ARR,
                     F_FACT_ARR_COUNT * sizeof(int32_t));
  cudaMemcpyToSymbol(F_FRAC_ARRAY, alp::Constants<float>::FRAC_ARR,
                     F_FRAC_ARR_COUNT * sizeof(float));

  cudaMemcpyToSymbol(D_FACT_ARRAY, alp::Constants<double>::FACT_ARR,
                     D_FACT_ARR_COUNT * sizeof(int64_t));
  cudaMemcpyToSymbol(D_FRAC_ARRAY, alp::Constants<double>::FRAC_ARR,
                     D_FRAC_ARR_COUNT * sizeof(double));
}

template <typename T> __device__ __forceinline__ T *get_frac_arr();
template <> __device__ __forceinline__ float *get_frac_arr() {
  return F_FRAC_ARRAY;
}
template <> __device__ __forceinline__ double *get_frac_arr() {
  return D_FRAC_ARRAY;
}

template <typename T> __device__ __forceinline__ T *get_fact_arr();
template <> __device__ __forceinline__ int32_t *get_fact_arr() {
  return F_FACT_ARRAY;
}
template <> __device__ __forceinline__ int64_t *get_fact_arr() {
  return D_FACT_ARRAY;
}
} // namespace constant_memory

// WARNING
// WARNING
// TODO WARNING IS IT NOT FASTER TO PASS THESE ARGUMENTS IN FULL WIDTH?

// SO uint8_T -> uint32_t (if it gets multiplied with 32) This saves a cast
// in each kernel, and we do not care how big parameters are, as they are
// passed via const
// INFO Hypothesis: not stalling on arithmetic, so it does not matter in
// execution time. Check # executed instructions tho.
// WARNING
// WARNING
template <typename T_in, typename T_out, UnpackingType unpacking_type,
          unsigned UNPACK_N_VECTORS, unsigned UNPACK_N_VALUES>
__device__ void unalp(T_out *__restrict out, const AlpColumn<T_out> column,
                      const uint16_t vector_index, const uint16_t lane,
                      const uint16_t start_index) {
  static_assert((std::is_same<T_in, uint32_t>::value &&
                 std::is_same<T_out, float>::value) ||
                    (std::is_same<T_in, uint64_t>::value &&
                     std::is_same<T_out, double>::value),
                "Wrong type arguments");
  using INT_T = typename utils::same_width_int<T_out>::type;
  using UINT_T = typename utils::same_width_int<T_out>::type;

  T_in *in = column.ffor_array + consts::VALUES_PER_VECTOR * vector_index;
  uint16_t value_bit_width = column.bit_widths[vector_index];
  UINT_T base = column.ffor_bases[vector_index];
  INT_T factor =
      constant_memory::get_fact_arr<INT_T>()[column.factors[vector_index]];
  T_out frac10 = constant_memory::get_frac_arr<
      T_out>()[column.exponents[vector_index]]; // WARNING TODO implement a
                                                // compile time switch to grab
                                                // float array
  auto lambda = [base, factor, frac10](const T_in value) -> T_out {
    return static_cast<T_out>(static_cast<INT_T>((value + base) *
                                                 static_cast<UINT_T>(factor))) *
           frac10;
  };

  unpack_vector<T_in, T_out, unpacking_type, UNPACK_N_VECTORS, UNPACK_N_VALUES>(
      in, out, lane, value_bit_width, start_index, lambda);

  // Patch exceptions
  constexpr auto N_LANES = utils::get_n_lanes<INT_T>();
  auto exceptions_count = column.counts[vector_index];

  auto vec_exceptions =
      column.exceptions + consts::VALUES_PER_VECTOR * vector_index;
  auto vec_exceptions_positions =
      column.positions + consts::VALUES_PER_VECTOR * vector_index;

  const int first_pos = start_index * N_LANES + lane;
  const int last_pos = first_pos + N_LANES * (UNPACK_N_VALUES - 1);
  if (unpacking_type == UnpackingType::VectorArray) {
    for (int i{lane}; i < exceptions_count; i += N_LANES) {
      // WARNING Currently assumes that you are decoding an entire vector
      // TODO Implement an if (position > startindex && position < (start_index
      // + UNPACK_N_VALUES * n_lanes) {...}
      auto position = vec_exceptions_positions[i];
      out[position] = vec_exceptions[i];
    }
  } else if (unpacking_type == UnpackingType::LaneArray) {
    for (int i{0}; i < exceptions_count; i++) {
      auto position = vec_exceptions_positions[i];
      auto exception = vec_exceptions[i];
      if (position >= first_pos) {
        if (position <= last_pos && position % N_LANES == lane) {
          out[(position - first_pos) / N_LANES] = exception;
        }
        if (position + 1 > last_pos) {
          return;
        }
      }
    }
  }
}

template <typename T_in, typename T_out, UnpackingType unpacking_type,
          unsigned UNPACK_N_VECTORS, unsigned UNPACK_N_VALUES>
__device__ void
unalp_with_scanner(T_out *__restrict out, const AlpColumn<T_out> column,
                   const uint16_t vector_index, const uint16_t lane,
                   const uint16_t start_index) {
  static_assert((std::is_same<T_in, uint32_t>::value &&
                 std::is_same<T_out, float>::value) ||
                    (std::is_same<T_in, uint64_t>::value &&
                     std::is_same<T_out, double>::value),
                "Wrong type arguments");
  using INT_T = typename utils::same_width_int<T_out>::type;
  using UINT_T = typename utils::same_width_int<T_out>::type;

  T_in *in = column.ffor_array + consts::VALUES_PER_VECTOR * vector_index;
  uint16_t value_bit_width = column.bit_widths[vector_index];
  UINT_T base = column.ffor_bases[vector_index];
  INT_T factor =
      constant_memory::get_fact_arr<INT_T>()[column.factors[vector_index]];
  T_out frac10 = constant_memory::get_frac_arr<
      T_out>()[column.exponents[vector_index]]; // WARNING TODO implement a
                                                // compile time switch to grab
                                                // float array
  auto lambda = [base, factor, frac10](const T_in value) -> T_out {
    return static_cast<T_out>(static_cast<INT_T>((value + base) *
                                                 static_cast<UINT_T>(factor))) *
           frac10;
  };

  unpack_vector<T_in, T_out, unpacking_type, UNPACK_N_VECTORS, UNPACK_N_VALUES>(
      in, out, lane, value_bit_width, start_index, lambda);

  // Patch exceptions
  constexpr auto N_LANES = utils::get_n_lanes<INT_T>();
  auto exceptions_count = column.counts[vector_index];

  auto vec_exceptions =
      column.exceptions + consts::VALUES_PER_VECTOR * vector_index;
  auto vec_exceptions_positions =
      column.positions + consts::VALUES_PER_VECTOR * vector_index;

  const int first_pos = start_index * N_LANES + lane;
  const int last_pos = first_pos + N_LANES * (UNPACK_N_VALUES - 1);
  if (unpacking_type == UnpackingType::VectorArray) {
    for (int i{lane}; i < exceptions_count; i += N_LANES) {
      // WARNING Currently assumes that you are decoding an entire vector
      // TODO Implement an if (position > startindex && position < (start_index
      // + UNPACK_N_VALUES * n_lanes) {...}
      auto position = vec_exceptions_positions[i];
      out[position] = vec_exceptions[i];
    }
  } else if (unpacking_type == UnpackingType::LaneArray) {
    constexpr int32_t SCANNER_SIZE = 1;
    uint16_t scanner[SCANNER_SIZE];

    for (int i{0}; i < exceptions_count; i += SCANNER_SIZE) {

      for (int j{0}; j < SCANNER_SIZE && j + i < exceptions_count; ++j) {
        scanner[j] = vec_exceptions_positions[j + i];
      }

      for (int j{0}; j < SCANNER_SIZE && j + i < exceptions_count; ++j) {
        auto position = scanner[j];
        if (position >= first_pos) {
          if (position <= last_pos && position % N_LANES == lane) {
            out[(position - first_pos) / N_LANES] = vec_exceptions[j + i];
          }
          if (position + 1 > last_pos) {
            return;
          }
        }
      }
    }
  }
}

template <typename T_in, typename T_out, UnpackingType unpacking_type,
          unsigned UNPACK_N_VECTORS, unsigned UNPACK_N_VALUES>

struct Unpacker {
  const int16_t vector_index;
  const uint16_t lane;
  const AlpColumn<T_out> column;
  int32_t start_index = 0;
  int32_t exception_index = 0;

  __device__ Unpacker(const uint16_t vector_index, const uint16_t lane,
           const AlpColumn<T_out> column)
      : vector_index(vector_index), lane(lane), column(column) {}

  __device__ void unpack_next_into(T_out *__restrict out) {
    static_assert((std::is_same<T_in, uint32_t>::value &&
                   std::is_same<T_out, float>::value) ||
                      (std::is_same<T_in, uint64_t>::value &&
                       std::is_same<T_out, double>::value),
                  "Wrong type arguments");
    using INT_T = typename utils::same_width_int<T_out>::type;
    using UINT_T = typename utils::same_width_int<T_out>::type;

    T_in *in = column.ffor_array + consts::VALUES_PER_VECTOR * vector_index;
    uint16_t value_bit_width = column.bit_widths[vector_index];
    UINT_T base = column.ffor_bases[vector_index];
    INT_T factor =
        constant_memory::get_fact_arr<INT_T>()[column.factors[vector_index]];
    T_out frac10 = constant_memory::get_frac_arr<
        T_out>()[column.exponents[vector_index]]; 
    auto lambda = [base, factor, frac10](const T_in value) -> T_out {
      return static_cast<T_out>(static_cast<INT_T>(
                 (value + base) * static_cast<UINT_T>(factor))) *
             frac10;
    };

    unpack_vector<T_in, T_out, unpacking_type, UNPACK_N_VECTORS,
                  UNPACK_N_VALUES>(in, out, lane, value_bit_width, start_index,
                                   lambda);

    // Patch exceptions
    constexpr auto N_LANES = utils::get_n_lanes<INT_T>();
    auto exceptions_count = column.counts[vector_index];

    auto vec_exceptions =
        column.exceptions + consts::VALUES_PER_VECTOR * vector_index;
    auto vec_exceptions_positions =
        column.positions + consts::VALUES_PER_VECTOR * vector_index;

    const int first_pos = start_index * N_LANES + lane;
    const int last_pos = first_pos + N_LANES * (UNPACK_N_VALUES - 1);
		start_index += UNPACK_N_VALUES;
    if (unpacking_type == UnpackingType::VectorArray) {
      for (int i{lane}; i < exceptions_count; i += N_LANES) {
        auto position = vec_exceptions_positions[i];
        out[position] = vec_exceptions[i];
      }
    } else if (unpacking_type == UnpackingType::LaneArray) {
      for (; exception_index < exceptions_count; exception_index++) {
        auto position = vec_exceptions_positions[exception_index];
        auto exception = vec_exceptions[exception_index];
        if (position >= first_pos) {
          if (position <= last_pos && position % N_LANES == lane) {
            out[(position - first_pos) / N_LANES] = exception;
          }
          if (position + 1 > last_pos) {
            return;
          }
        }
      }
    }
  }
};

template <typename T_in, typename T_out, UnpackingType unpacking_type,
          unsigned UNPACK_N_VECTORS, unsigned UNPACK_N_VALUES>
__device__ void unalprd(T_out *__restrict out, const AlpRdColumn<T_out> column,
                        const uint16_t vector_index, const uint16_t lane,
                        const uint16_t start_index) {
  static_assert((std::is_same<T_in, uint32_t>::value &&
                 std::is_same<T_out, float>::value) ||
                    (std::is_same<T_in, uint64_t>::value &&
                     std::is_same<T_out, double>::value),
                "Wrong type arguments");
  using INT_T = typename utils::same_width_int<T_out>::type;
  using UINT_T = typename utils::same_width_uint<T_out>::type;

  constexpr int32_t N_LANES = utils::get_n_lanes<UINT_T>();
  constexpr int32_t VALUES_PER_LANE = utils::get_values_per_lane<UINT_T>();

  UINT_T *out_ut = reinterpret_cast<UINT_T *>(out);

  // Loading left parts dict
  // TODO Let the threads collaborate to load this data
  const uint16_t *left_parts_dicts_p =
      column.left_parts_dicts +
      vector_index * alp::config::MAX_RD_DICTIONARY_SIZE;
  uint16_t left_parts_dict[alp::config::MAX_RD_DICTIONARY_SIZE];
  for (size_t j{0}; j < alp::config::MAX_RD_DICTIONARY_SIZE; j++) {
    left_parts_dict[j] = left_parts_dicts_p[j];
  }

  // Unfforring Arrays
  // INFO The paper says something about fusing, consider using a custom lambda
  // TODO Do thread local array instead of shared
  __shared__ uint16_t left_array[consts::VALUES_PER_VECTOR];
  const uint16_t *left_ffor_array =
      column.left_ffor_array + vector_index * consts::VALUES_PER_VECTOR;
  const uint16_t left_bitwidth = column.left_bit_widths[vector_index];
  const uint16_t left_base = column.left_ffor_bases[vector_index];

  // WARNING This mapping from alprd lane to uint16_t lane should be independent
  // of block sizes
  for (int i{lane}; i < utils::get_n_lanes<uint16_t>(); i += N_LANES) {
    undict_vector<uint16_t, uint16_t, UnpackingType::VectorArray, 1,
                  utils::get_values_per_lane<uint16_t>()>(
        left_ffor_array, left_array, i, left_bitwidth, 0, &left_base,
        left_parts_dict);
  }

  // TODO Do thread local array instead of shared
  __shared__ UINT_T right_array[consts::VALUES_PER_VECTOR];
  const UINT_T *right_ffor_array =
      column.right_ffor_array + vector_index * consts::VALUES_PER_VECTOR;
  const uint16_t right_bitwidth = column.right_bit_widths[vector_index];
  const UINT_T right_base = column.right_ffor_bases[vector_index];
  unffor_vector<UINT_T, UnpackingType::VectorArray, 1, VALUES_PER_LANE>(
      right_ffor_array, right_array, lane, right_bitwidth, 0, &right_base);

  // Decoding
#pragma unroll
  for (int i{lane}; i < consts::VALUES_PER_VECTOR; i += N_LANES) {
    // WARNING THIS SHOULD NOT WRITE TO GLOBAL
    // TODO write to thread local, and then patch all thread local values
    // INFO THIS IS ALSO an issue in the normal ALP kernel
    out_ut[i] =
        (static_cast<UINT_T>(left_array[i]) << right_bitwidth) | right_array[i];
  }

  // Patching exceptions
  const uint16_t exceptions_count = column.counts[vector_index];
  const uint16_t *vec_exceptions =
      column.exceptions + consts::VALUES_PER_VECTOR * vector_index;
  const uint16_t *vec_exceptions_positions =
      column.positions + consts::VALUES_PER_VECTOR * vector_index;

  if (unpacking_type == UnpackingType::VectorArray) {
    for (int i{lane}; i < exceptions_count; i += N_LANES) {
      const auto position = vec_exceptions_positions[i];
      const UINT_T right = right_array[position];
      const uint16_t left = vec_exceptions[i];
      out_ut[position] = (static_cast<UINT_T>(left) << right_bitwidth) | right;
    }
  } else if (unpacking_type == UnpackingType::LaneArray) {
    for (int i{0}; i < exceptions_count; ++i) {
      const auto position = vec_exceptions_positions[i];

      if (position % N_LANES == lane) {
        const UINT_T right = right_array[position];
        const uint16_t left = vec_exceptions[i];
        out_ut[position / N_LANES] =
            (static_cast<UINT_T>(left) << right_bitwidth) | right;
      }
    }
  }
}

#endif // ALP_CUH
