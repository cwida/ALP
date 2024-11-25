#ifndef EDGE_CASE_HPP
#define EDGE_CASE_HPP

#include "column.hpp"

namespace alp_bench {

inline auto get_edge_case() {
	static std::array<ALPColumnDescriptor, 1> EDGE_CASE = {{
	    {1, "edge_case", get_paths().edge_dataset_csv_path + "edge_case.csv", "", 0, 0, 12, 0, true},

	}};
	return EDGE_CASE;
}

} // namespace alp_bench
#endif
