#ifndef EXAMPLE_HELPER_HPP
#define EXAMPLE_HELPER_HPP

#include <cstddef>
#include <cstdint>

namespace example {

template <class T>
inline void fill_random_data(T* in, size_t tuples_to_generate, uint8_t precision) {
	std::uniform_real_distribution<T> unif(100, 300);
	std::default_random_engine        re;
	re.seed(42);
	uint8_t doubles_intrinsic_precision = precision;
	const T precision_multiplier        = std::pow(10.0, doubles_intrinsic_precision);
	for (size_t i = 0; i < tuples_to_generate; i++) {
		T random_value                 = unif(re);
		T fixed_precision_random_value = std::round(random_value * precision_multiplier) / precision_multiplier;
		in[i]                          = fixed_precision_random_value;
	}
}

} // namespace example

#endif // EXAMPLE_HELPER_HPP
