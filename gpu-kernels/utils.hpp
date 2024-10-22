#include <cstdint>
#include <cstdint>
#include <type_traits>

#include "consts.hpp"

#ifndef FASTLANES_UTILS_H
#define FASTLANES_UTILS_H

namespace utils { // internal functions

template <typename T> constexpr int32_t sizeof_in_bits() {
  return sizeof(T) * 8;
}

template <typename T> constexpr T set_first_n_bits(const int32_t count) {
  return (count < sizeof_in_bits<T>() ? static_cast<T>((T{1} << int32_t{count}) - T{1})
                                      : static_cast<T>(~T{0}));
}

template <typename T> constexpr int32_t get_lane_bitwidth() {
  return sizeof_in_bits<T>();
}

template <typename T> constexpr int32_t get_n_lanes() {
  return consts::REGISTER_WIDTH / get_lane_bitwidth<T>();
}

template <typename T> constexpr int32_t get_values_per_lane() {
  return consts::VALUES_PER_VECTOR / get_n_lanes<T>();
}

template <typename T>
constexpr int32_t get_compressed_vector_size(int32_t value_bit_width) {
  return (consts::VALUES_PER_VECTOR * value_bit_width) / sizeof_in_bits<T>();
}

constexpr size_t get_n_vecs_from_size(const size_t size) {
	return (size + consts::VALUES_PER_VECTOR - 1) / consts::VALUES_PER_VECTOR;
}

template<typename T>
struct same_width_int {
  using type = 
		typename std::conditional<sizeof(T) == 8, int64_t, 
		typename std::conditional<sizeof(T) == 4, int32_t, 
		typename std::conditional<sizeof(T) == 2, int16_t, 
		int8_t>::type>::type>::type;
};

template<typename T>
struct same_width_uint {
  using type = 
		typename std::conditional<sizeof(T) == 8, uint64_t, 
		typename std::conditional<sizeof(T) == 4, uint32_t, 
		typename std::conditional<sizeof(T) == 2, uint16_t, 
		uint8_t>::type>::type>::type;
};

} // namespace utils

#endif // FASTLANES_UTILS_H
