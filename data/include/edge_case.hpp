#ifndef EDGE_CASE_HPP
#define EDGE_CASE_HPP

// NOLINTBEGIN

#include "column.hpp"

namespace alp_bench {
inline std::array<Column, 1> edge_case = {{

    {1, "edge_case", PATHS.EDGE_DATASET_CSV_PATH + "edge_case.csv", "", 0, 0, 12, 0, true},

}};
} // namespace alp_bench
#endif

// NOLINTEND
