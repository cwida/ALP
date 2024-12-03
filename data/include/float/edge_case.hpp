#ifndef FLOAT_EDGE_CASE_HPP
#define FLOAT_EDGE_CASE_HPP

#include "column.hpp"

namespace alp_bench {

inline auto get_float_edge_case() {
	static std::array<ALPColumnDescriptor, 1> EDGE_CASE = {{
	    {1, "avx512dq", get_paths().edge_dataset_csv_path + "avx512dq.csv", "", 0, 0, 192, 0, true},

	}};
	return EDGE_CASE;
}

} // namespace alp_bench
#endif
