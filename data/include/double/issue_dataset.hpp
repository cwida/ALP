#ifndef ISSUE_DATASET_HPP
#define ISSUE_DATASET_HPP

#include "column.hpp"

namespace alp_bench {

inline auto get_issue_dataset() {
	static std::array<ALPColumnDescriptor, 2> ISSUE_DATASET = {{
	    // prev issue_8
	    {0,
	     "issue_24_replicated_data",
	     get_paths().issue_24_dataset_dir_path + "issue_24_102400_values.csv",
	     "",
	     0,
	     0,
	     0,
	     0,
	     false},
	    {1,
	     "issue_24_actual_data",
	     get_paths().issue_24_dataset_dir_path + "ShapesALL_TEST.csv",
	     "",
	     0,
	     0,
	     0,
	     0,
	     false},

	}};

	return ISSUE_DATASET;
}

} // namespace alp_bench
#endif