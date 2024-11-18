#include "zstd.h"
#include "alp.hpp"
#include "data.hpp"
#include "mapper.hpp"
#include "gtest/gtest.h"

class zstd_test : public ::testing::Test {
public:
	void*  enc_dbl_arr;
	void*  dec_dbl_arr;
	size_t zstd_vector_size =
	    alp::config::ROWGROUP_SIZE; // For Zstd we compress rowgroups since it would be unfair to compress small vectors

	void SetUp() override {}

	~zstd_test() override {}

	template <typename T, int N_DATASETS>
	void bench_compression_ratio(const std::array<alp_bench::Column, N_DATASETS>& datasets, const std::string& path) {
		if (const auto v = std::getenv("ALP_DATASET_DIR_PATH"); v != nullptr) {
			alp_bench::get_paths().alp_dataset_binary_dir_path = *v;
		}

		size_t enc_size_upper_bound = zstd_vector_size * sizeof(T);
		size_t input_size           = zstd_vector_size * sizeof(T);
		size_t dec_size             = input_size;
		enc_dbl_arr                 = malloc(input_size);
		dec_dbl_arr                 = malloc(input_size);

		std::ofstream ofile(path, std::ios::out);
		ofile << "dataset,size\n";

		auto* dbl_arr = new T[zstd_vector_size];

		for (auto& dataset : datasets) {
			if (dataset.name.find("bw") != std::string::npos) { continue; }
			std::cout << dataset.name << std::endl;

			size_t      tuples_count;
			const auto* data_column     = mapper::mmap_file<T>(tuples_count, dataset.binary_file_path);
			T           value_to_encode = 0.0;
			size_t      vector_idx {0};
			size_t      processed_tuples     = 0;
			size_t      compressed_data_size = 0;

			if (tuples_count < zstd_vector_size) {
				zstd_vector_size     = tuples_count;
				input_size           = zstd_vector_size * sizeof(T);
				enc_size_upper_bound = zstd_vector_size * sizeof(T);
			}

			/* Encode - Decode - Validate. */
			for (size_t i = 0; i < tuples_count; i++) {
				value_to_encode     = data_column[i];
				dbl_arr[vector_idx] = value_to_encode;
				vector_idx          = vector_idx + 1;

				if (vector_idx != zstd_vector_size) { continue; }

				processed_tuples += zstd_vector_size;

				// Encode
				size_t const ENC_SIZE =
				    ZSTD_compress(enc_dbl_arr, enc_size_upper_bound, dbl_arr, input_size, 3); // Level 3

				// SUM COMPRESSED SIZE
				compressed_data_size += ENC_SIZE * 8;

				// Decode
				ZSTD_decompress(dec_dbl_arr, dec_size, enc_dbl_arr, ENC_SIZE);

				const auto* dec_dbl_arr_tmp = static_cast<T*>(dec_dbl_arr);
				for (size_t j = 0; j < zstd_vector_size; ++j) {
					const auto l = dbl_arr[j];
					if (const auto r = dec_dbl_arr_tmp[j]; l != r) { std::cerr << j << ", " << dataset.name << "\n"; }
					ASSERT_EQ(dbl_arr[j], dec_dbl_arr_tmp[j]);
				}
				vector_idx = 0;
			}

			auto compression_ratio = (double)compressed_data_size / processed_tuples;

			ofile << std::fixed << std::setprecision(2) << dataset.name << "," << compression_ratio << std::endl;
		}

		delete[] dbl_arr;
		free(enc_dbl_arr);
		free(dec_dbl_arr);
	}
};

TEST_F(zstd_test, test_zstd128_on_whole_datasets) {
	auto result_path = alp_bench::get_paths().result_dir_path + "compression_ratio_result/double/zstd.csv";
	bench_compression_ratio<double, 30>(alp_bench::get_alp_dataset(), result_path);
}

TEST_F(zstd_test, test_zstd128_on_float_datasets) {
	auto result_path = alp_bench::get_paths().result_dir_path + "compression_ratio_result/float/zstd.csv";
	bench_compression_ratio<float, 4>(alp_bench::get_sp_datasets(), result_path);
}
