#ifndef COLUMN_HPP
#define COLUMN_HPP

#include <array>
#include <cstdint>
#include <string>

namespace alp_bench {

enum class DataType : uint8_t {
	INVALID = 0,
	DOUBLE  = 1,
	FLOAT   = 2,
};

enum class FileType : uint8_t {
	INVALID = 0,
	BINARY  = 1,
	CSV     = 2,
};

struct ColumnDescriptor {
	int         id;
	DataType    data_type;
	std::string name;
	std::string path;
	FileType    file_type;
};

struct ALPColumnDescriptor {
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
	std::string generated_columns_csv_path  = std::string {ALP_CMAKE_SOURCE_DIR} + "/data/generated/";
	std::string alp_dataset_csv_path        = std::string {ALP_CMAKE_SOURCE_DIR} + "/data/samples/";
	std::string edge_dataset_csv_path       = std::string {ALP_CMAKE_SOURCE_DIR} + "/data/edge_case/";
	std::string result_dir_path             = std::string {ALP_CMAKE_SOURCE_DIR} + "/publication/";
	std::string evalimplsts_csv_path        = std::string {ALP_CMAKE_SOURCE_DIR} + "/data/evalimplsts/";
	std::string alp_dataset_binary_dir_path = std::string {ALP_CMAKE_SOURCE_DIR} + "/data/full_data/";
	std::string alp_result_dir_path         = std::string {ALP_CMAKE_SOURCE_DIR} + "/benchmarks/result/";
	std::string hs                          = std::string {ALP_CMAKE_SOURCE_DIR} + "/data/full_data/";
	std::string issue_24_dataset_dir_path   = std::string {ALP_CMAKE_SOURCE_DIR} + "/data/issue/";

	explicit paths() {
		const auto alp_dataset_dir_path_env_variable = std::getenv("ALP_DATASET_DIR_PATH");
		if (alp_dataset_dir_path_env_variable) { alp_dataset_binary_dir_path = alp_dataset_dir_path_env_variable; }

		const auto hs_env_variable = std::getenv("HURRICANE_ISABEL_DATASET_DIR_PATH");
		if (hs_env_variable) { hs = hs_env_variable; }
	}
};

inline paths get_paths() {
	static paths PATHS;
	return PATHS;
}

} // namespace alp_bench

#endif
