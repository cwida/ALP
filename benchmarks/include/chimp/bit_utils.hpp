//===----------------------------------------------------------------------===//
//                         DuckDB
//
// lwcbench/compression/chimp/algorithm/bit_utils.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

namespace alp_bench {

template <class R>
struct BitUtils {
	static constexpr R Mask(unsigned int const bits) {
		return (((uint64_t)(bits < (sizeof(R) * 8))) << (bits & ((sizeof(R) * 8) - 1))) - 1U;
	}
};

} // namespace alp_bench
