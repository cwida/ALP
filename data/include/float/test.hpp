#ifndef ALP_TEST_HPP
#define ALP_TEST_HPP

#include "column.hpp"

namespace alp_bench {
inline auto get_float_test_dataset() {
	static std::array<ALPColumnDescriptor, 5> FLOAT_TEST_DATASET = {{
	    {0, "Arade/4", get_paths().alp_dataset_csv_path + "arade4.csv", "", 0, 0, 0, 0},
	    {1, "test_0", ALP_CMAKE_SOURCE_DIR "/data/float/test_0.csv", "", 0, 0, 0, 4},
	    {2, "test_1", ALP_CMAKE_SOURCE_DIR "/data/float/test_1.csv", "", 0, 0, 0, 10},
	    {3, "test_2", ALP_CMAKE_SOURCE_DIR "/data/float/test_2.csv", "", 0, 0, 0, 17},
	    {4, "test_3", ALP_CMAKE_SOURCE_DIR "/data/float/test_3.csv", "", 0, 0, 0, 0},

	}};
	return FLOAT_TEST_DATASET;
}

} // namespace alp_bench
#endif // ALP_TEST_HPP
