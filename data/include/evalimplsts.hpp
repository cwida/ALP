#ifndef EVALIMPLSTS_HPP
#define EVALIMPLSTS_HPP

#include "column.hpp"

namespace alp_bench {

inline auto get_evalimplsts() {
	static std::array<ALPColumnDescriptor, 1> EVALIMPLSTS = {{
	    // prev issue_8
	    {0, "active_power", get_paths().evalimplsts_csv_path + "active_power.csv", "", 0, 0, 0, 0, true},

	}};

	return EVALIMPLSTS;
}

} // namespace alp_bench
#endif