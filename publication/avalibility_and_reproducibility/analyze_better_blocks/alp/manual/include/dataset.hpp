#ifndef DATASET_HPP
#define DATASET_HPP

#include "string"
#include <cstdint>
#include <vector>

namespace dataset {
struct Dataset {
	uint64_t    id;
	std::string name;
	std::string file_path;
	std::string digits_file_path;
	std::string exceptions_file_path;
	int         exponent;
	int64_t     factor_idx;
	uint16_t    exc_c;
	double      coverage;
	double      bits_per_value;
	int64_t     max_digit;
	int64_t     min_digit;
	uint8_t     bw;
};

struct paths {
	std::string DATASETS_1024_SAMPLES_PATH    = "../data/1024_data_samples/";
	std::string DATASETS_1024_DIGITS_PATH     = "../data/1024_data_digits/";
	std::string DATASETS_1024_EXCEPTIONS_PATH = "../data/1024_data_exceptions/";
	std::string DATASETS_COMPLETE_PATH        = "../data/data_for_c/";
	std::string ALP_DATA_DIR_PATH             = "../data";
	explicit paths() {
		if (auto v = std::getenv("DATASETS_1024_SAMPLES_PATH")) { DATASETS_1024_SAMPLES_PATH = v; }
		if (auto v = std::getenv("DATASETS_1024_DIGITS_PATH")) { DATASETS_1024_DIGITS_PATH = v; }
		if (auto v = std::getenv("DATASETS_1024_EXCEPTIONS_PATH")) { DATASETS_1024_EXCEPTIONS_PATH = v; }
		if (auto v = std::getenv("DATASETS_COMPLETE_PATH")) { DATASETS_COMPLETE_PATH = v; }
		if (auto v = std::getenv("ALP_DATA_DIR_PATH")) { ALP_DATA_DIR_PATH = v; }
	}
};

paths PATHS;

} // namespace dataset

#endif