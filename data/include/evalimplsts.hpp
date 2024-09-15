#ifndef ALP_DOUBLE_EVALIMPLSTS_HPP
#define ALP_DOUBLE_EVALIMPLSTS_HPP

#include "column.hpp"

namespace alp_bench {

inline std::array<Column, 1> evalimplsts = {{
    // prev issue_8
    {0, "active_power", PATHS.EVALIMPLSTS_CSV_PATH + "active_power.csv", "", 0, 0, 0, 0, true},

}};
} // namespace alp_bench
#endif