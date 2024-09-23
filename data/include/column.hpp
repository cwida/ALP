#ifndef COLUMN_HPP
#define COLUMN_HPP

#include <array>
#include <cstdint>
#include <string>

namespace alp_bench {
struct Column {
	uint64_t          id;
	std::string       name;
	const std::string csv_file_path;
	const std::string binary_file_path;
	uint8_t           factor {0};
	uint16_t          exponent {0};
	uint16_t          exceptions_count {0};
	uint8_t           bit_width {0};
	bool              suitable_for_cutting {false};
};

struct paths {
	std::string generated_columns_csv_path  = std::string {CMAKE_SOURCE_DIR} + "/data/generated/";
	std::string alp_dataset_csv_path        = std::string {CMAKE_SOURCE_DIR} + "/data/samples/";
	std::string edge_dataset_csv_path       = std::string {CMAKE_SOURCE_DIR} + "/data/edge_case/";
	std::string result_dir_path             = std::string {CMAKE_SOURCE_DIR} + "/publication/";
	std::string evalimplsts_csv_path        = std::string {CMAKE_SOURCE_DIR} + "/data/evalimplsts/";
	std::string alp_dataset_binary_dir_path = std::string {CMAKE_SOURCE_DIR} + "/data/full_data/";

	explicit paths() {
		const auto v = std::getenv("ALP_DATASET_DIR_PATH");
		if (v) { alp_dataset_binary_dir_path = v; }
	}
};

inline paths get_paths() {
	static paths PATHS;
	return PATHS;
}

} // namespace alp_bench

#endif
