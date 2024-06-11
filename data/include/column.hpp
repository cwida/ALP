#ifndef COLUMN_HPP
#define COLUMN_HPP

#include <array>
#include <cstdint>
#include <string>

namespace alp_bench {
struct Column {
	uint64_t          id;
	std::string       name;
	const std::string sample_csv_file_path;
	const std::string binary_file_path;
	uint8_t           factor {0};
	uint16_t          exponent {0};
	uint16_t          exceptions_count {0};
	uint8_t           bit_width {0};
	bool              suitable_for_cutting {false};
};

struct paths {
	std::string GENERATED_COLUMNS_CSV_PATH  = std::string {CMAKE_SOURCE_DIR} + "/data/generated/";
	std::string ALP_DATASET_SAMPLE_CSV_PATH = std::string {CMAKE_SOURCE_DIR} + "/data/samples/";
	std::string EDGE_DATASET_CSV_PATH       = std::string {CMAKE_SOURCE_DIR} + "/data/edge_case/";
	std::string RESULT_DIR_PATH             = std::string {CMAKE_SOURCE_DIR} + "/publication/";
	std::string ALP_DATASET_BINARY_DIR_PATH = " ";

	explicit paths() {
		auto v = std::getenv("ALP_DATASET_DIR_PATH");
		if (v) { ALP_DATASET_BINARY_DIR_PATH = v; }
	}
};

inline paths PATHS;

} // namespace alp_bench

#endif