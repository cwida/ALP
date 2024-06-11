#include "zstd.h"
#include "alp.hpp"
#include "data.hpp"
#include "test/mapper.hpp"
#include "gtest/gtest.h"

class zstd_test : public ::testing::Test {
public:
	double* dbl_arr;
	void*   enc_dbl_arr;
	void*   dec_dbl_arr;
	size_t  ZSTD_VECTOR_SIZE =
	    alp::config::ROWGROUP_SIZE; // For Zstd we compress rowgroups since it would be unfair to compress small vectors
	size_t ENC_SIZE_UPPER_BOUND = ZSTD_VECTOR_SIZE * 8;
	size_t INPUT_SIZE           = ZSTD_VECTOR_SIZE * 8;
	size_t DEC_SIZE             = INPUT_SIZE;

	void SetUp() override {
		dbl_arr     = new double[ZSTD_VECTOR_SIZE];
		enc_dbl_arr = malloc(INPUT_SIZE);
		dec_dbl_arr = malloc(INPUT_SIZE);

		const auto v = std::getenv("ALP_DATASET_DIR_PATH");
		if (v == nullptr) { throw std::runtime_error("Environment variable ALP_DATASET_DIR_PATH is not set!"); }
		alp_bench::PATHS.ALP_DATASET_BINARY_DIR_PATH = v;
	}

	~zstd_test() override {
		delete[] dbl_arr;
		free(enc_dbl_arr);
		free(dec_dbl_arr);
	}
};

TEST_F(zstd_test, test_zstd_on_whole_datasets) {
	std::ofstream ofile(alp_bench::PATHS.RESULT_DIR_PATH + "zstd_compression_ratio.csv", std::ios::out);
	ofile << "dataset,size\n";

	for (auto& dataset : alp_bench::alp_dataset) {
		if (dataset.name.find("bw") != std::string::npos) { continue; }

		size_t      tuples_count;
		const auto* data_column     = mapper::mmap_file<double>(tuples_count, dataset.binary_file_path);
		double      value_to_encode = 0.0;
		size_t      vector_idx {0};
		size_t      processed_tuples = 0;

		size_t compressed_data_size = 0;

		std::cout << dataset.name << "\n";

		if (tuples_count < ZSTD_VECTOR_SIZE) {
			ZSTD_VECTOR_SIZE     = tuples_count;
			INPUT_SIZE           = ZSTD_VECTOR_SIZE * 8;
			ENC_SIZE_UPPER_BOUND = ZSTD_VECTOR_SIZE * 8;
		}

		/* Encode - Decode - Validate. */
		for (size_t i = 0; i < tuples_count; i++) {
			value_to_encode     = data_column[i];
			dbl_arr[vector_idx] = value_to_encode;
			vector_idx          = vector_idx + 1;

			if (vector_idx != ZSTD_VECTOR_SIZE) { continue; }

			processed_tuples += ZSTD_VECTOR_SIZE;

			// Encode
			size_t const ENC_SIZE = ZSTD_compress(enc_dbl_arr, ENC_SIZE_UPPER_BOUND, dbl_arr, INPUT_SIZE, 3); // Level 3

			// SUM COMPRESSED SIZE
			compressed_data_size += ENC_SIZE * 8;

			// Decode
			ZSTD_decompress(dec_dbl_arr, DEC_SIZE, enc_dbl_arr, ENC_SIZE);

			const auto* dec_dbl_arr_tmp = static_cast<double*>(dec_dbl_arr);
			for (size_t j = 0; j < ZSTD_VECTOR_SIZE; ++j) {
				const auto l = dbl_arr[j];
				if (const auto r = dec_dbl_arr_tmp[j]; l != r) { std::cerr << j << ", " << dataset.name << "\n"; }
				ASSERT_EQ(dbl_arr[j], dec_dbl_arr_tmp[j]);
			}
			vector_idx = 0;
		}

		auto compression_ratio = (double)compressed_data_size / processed_tuples;

		ofile << std::fixed << std::setprecision(2) << dataset.name << "," << compression_ratio << std::endl;
	}
}
