#include "alp.hpp"
#include "alp_result.hpp"
#include "data.hpp"
#include "test/mapper.hpp"
#include "gtest/gtest.h"
#include <unordered_map>

using namespace alp::config;
/*
 * ALP overhead per vector in a hypothetic file format = bit_width + factor-idx + exponent-idx + ffor base;
 */
double overhead_per_vector {static_cast<double>(8 + 8 + 8 + 64) / VECTOR_SIZE};

double calculate_alp_compression_size(std::vector<alp_bench::VectorMetadata>& vector_metadata) {
	double avg_bits_per_value {0};
	for (auto& metadata : vector_metadata) {
		avg_bits_per_value = avg_bits_per_value + metadata.bit_width;
		avg_bits_per_value = avg_bits_per_value +
		                     (static_cast<double>(metadata.exceptions_count) *
		                      (alp::Constants<double>::EXCEPTION_SIZE + alp::EXCEPTION_POSITION_SIZE) / VECTOR_SIZE);
	}

	avg_bits_per_value = avg_bits_per_value / vector_metadata.size();
	avg_bits_per_value = avg_bits_per_value + overhead_per_vector;
	return avg_bits_per_value;
}

/*
 * ALPRD Overhead per vector in a hypothetic file format in which the left parts dictionary is at the start of a
 * rowgroup
 */
double alprd_overhead_per_vector {static_cast<double>(MAX_RD_DICTIONARY_SIZE * 16) / ROWGROUP_SIZE};

double calculate_alprd_compression_size(std::vector<alp_bench::VectorMetadata>& vector_metadata) {
	double avg_bits_per_value {0};
	for (auto& metadata : vector_metadata) {
		avg_bits_per_value = avg_bits_per_value + metadata.right_bit_width + metadata.left_bit_width +
		                     static_cast<double>(metadata.exceptions_count *
		                                         (alp::RD_EXCEPTION_SIZE + alp::RD_EXCEPTION_POSITION_SIZE)) /
		                         VECTOR_SIZE;
	}

	avg_bits_per_value = avg_bits_per_value / vector_metadata.size();
	avg_bits_per_value = avg_bits_per_value + alprd_overhead_per_vector;

	return avg_bits_per_value;
}

double get_average_exception_count(std::vector<alp_bench::VectorMetadata>& vector_metadata) {
	double avg_exceptions_count {0};
	for (auto& metadata : vector_metadata) {
		avg_exceptions_count = avg_exceptions_count + metadata.exceptions_count;
	}

	avg_exceptions_count = avg_exceptions_count / vector_metadata.size();
	return avg_exceptions_count;
}

class alp_test : public ::testing::Test {
public:
	double*   dbl_arr {};
	double*   exc_arr {};
	uint16_t* rd_exc_arr {};
	uint16_t* pos_arr {};
	uint16_t* exc_c_arr {};
	int64_t*  ffor_arr {};
	int64_t*  unffor_arr {};
	int64_t*  base_arr {};
	int64_t*  encoded_arr {};
	double*   dec_dbl_arr {};
	double*   smp_arr {};
	uint64_t* ffor_right_arr {};
	uint16_t* ffor_left_arr {};
	uint64_t* right_arr {};
	uint16_t* left_arr {};
	uint64_t* unffor_right_arr {};
	uint16_t* unffor_left_arr {};
	double*   glue_arr {};

	alp::state state;

	uint8_t bit_width {};

	void SetUp() override {
		dbl_arr          = new double[VECTOR_SIZE];
		exc_arr          = new double[VECTOR_SIZE];
		rd_exc_arr       = new uint16_t[VECTOR_SIZE];
		pos_arr          = new uint16_t[VECTOR_SIZE];
		encoded_arr      = new int64_t[VECTOR_SIZE];
		dec_dbl_arr      = new double[VECTOR_SIZE];
		exc_c_arr        = new uint16_t[VECTOR_SIZE];
		ffor_arr         = new int64_t[VECTOR_SIZE];
		unffor_arr       = new int64_t[VECTOR_SIZE];
		base_arr         = new int64_t[VECTOR_SIZE];
		smp_arr          = new double[VECTOR_SIZE];
		right_arr        = new uint64_t[VECTOR_SIZE];
		left_arr         = new uint16_t[VECTOR_SIZE];
		ffor_right_arr   = new uint64_t[VECTOR_SIZE];
		ffor_left_arr    = new uint16_t[VECTOR_SIZE];
		unffor_right_arr = new uint64_t[VECTOR_SIZE];
		unffor_left_arr  = new uint16_t[VECTOR_SIZE];
		glue_arr         = new double[VECTOR_SIZE];
	}

	~alp_test() override {
		delete[] dbl_arr;
		delete[] exc_arr;
		delete[] rd_exc_arr;
		delete[] pos_arr;
		delete[] encoded_arr;
		delete[] dec_dbl_arr;
		delete[] exc_c_arr;
		delete[] ffor_arr;
		delete[] unffor_arr;
		delete[] base_arr;
		delete[] smp_arr;
		delete[] right_arr;
		delete[] left_arr;
		delete[] unffor_right_arr;
		delete[] unffor_left_arr;
	}
};

/*
 * Test to encode and decode whole datasets using ALP
 * This test will output and write a file with the estimated bits/value after compression with alp
 */

TEST_F(alp_test, test_alp_on_whole_datasets) {

	if (const auto v = std::getenv("ALP_DATASET_DIR_PATH"); v == nullptr) {
		throw std::runtime_error("Environment variable ALP_DATASET_DIR_PATH is not set!");
	}

	std::ofstream ofile(alp_bench::PATHS.RESULT_DIR_PATH + "alp_compression_ratio.csv", std::ios::out);
	ofile << "dataset,size,rowgroups_count,vectors_count\n";

	for (auto& dataset : alp_bench::alp_dataset) {
		if (dataset.suitable_for_cutting) { continue; }

		std::cout << dataset.name << std::endl;

		std::vector<alp_bench::VectorMetadata> compression_metadata;
		size_t                                 tuples_count;
		auto*      data_column = mapper::mmap_file<double>(tuples_count, dataset.binary_file_path);
		double     value_to_encode {0.0};
		size_t     vector_idx {0};
		size_t     rowgroup_counter {0};
		size_t     rowgroup_offset {0};
		alp::state stt;
		size_t     rowgroups_count = std::ceil(static_cast<double>(tuples_count) / ROWGROUP_SIZE);
		size_t     vectors_count   = tuples_count / VECTOR_SIZE;
		/* Init */
		alp::AlpEncode<double>::init(data_column, rowgroup_offset, tuples_count, smp_arr, stt);
		/* Encode - Decode - Validate. */
		for (size_t i = 0; i < tuples_count; i++) {
			value_to_encode     = data_column[i];
			dbl_arr[vector_idx] = value_to_encode;
			vector_idx          = vector_idx + 1;
			rowgroup_offset     = rowgroup_offset + 1;
			rowgroup_counter    = rowgroup_counter + 1;

			if (vector_idx != VECTOR_SIZE) { continue; }
			if (rowgroup_counter == ROWGROUP_SIZE) {
				rowgroup_counter = 0;
				alp::AlpEncode<double>::init(data_column, rowgroup_offset, tuples_count, smp_arr, stt);
			}
			alp::AlpEncode<double>::encode(dbl_arr, exc_arr, pos_arr, exc_c_arr, encoded_arr, stt);
			alp::AlpEncode<double>::analyze_ffor(encoded_arr, bit_width, base_arr);
			ffor::ffor(encoded_arr, ffor_arr, bit_width, base_arr);

			unffor::unffor(ffor_arr, unffor_arr, bit_width, base_arr);
			alp::AlpDecode<double>::decode(unffor_arr, stt.fac, stt.exp, dec_dbl_arr);
			alp::AlpDecode<double>::patch_exceptions(dec_dbl_arr, exc_arr, pos_arr, exc_c_arr);

			for (size_t j = 0; j < VECTOR_SIZE; j++) {
				auto l = dbl_arr[j];
				auto r = dec_dbl_arr[j];
				if (l != r) { std::cerr << j << ", " << rowgroup_offset << ", " << dataset.name << "\n"; }
				ASSERT_EQ(dbl_arr[j], dec_dbl_arr[j]);
			}
			compression_metadata.push_back({bit_width, exc_c_arr[0]});
			vector_idx = 0;
			bit_width  = 0;
		}
		auto compression_ratio = calculate_alp_compression_size(compression_metadata);

		ofile << std::fixed << std::setprecision(2) << dataset.name << "," << compression_ratio << ","
		      << rowgroups_count << "," << vectors_count << std::endl;

		if (alp_bench::results.find(dataset.name) !=
		    alp_bench::results.end()) { // To avoid error when tested dataset is not found on results
			ASSERT_EQ(alp_bench::to_str(compression_ratio), alp_bench::results.find(dataset.name)->second);
		}
	}
}

/*
 * Test to encode and decode whole datasets using ALP RD (aka ALP Cutter)
 * This test will output and write a file with the estimated bits/value after compression with alp
 */
TEST_F(alp_test, test_alprd_on_whole_datasets) {
	std::ofstream ofile(alp_bench::PATHS.RESULT_DIR_PATH + "alp_rd_compression_ratio.csv", std::ios::out);
	ofile << "dataset,size,rowgroups_count,vectors_count\n";

	for (auto& dataset : alp_bench::alp_dataset) {
		if (!dataset.suitable_for_cutting) { continue; }

		std::vector<alp_bench::VectorMetadata> compression_metadata;
		size_t                                 tuples_count;
		auto*      data_column     = mapper::mmap_file<double>(tuples_count, dataset.binary_file_path);
		double     value_to_encode = 0.0;
		size_t     vector_idx {0};
		size_t     rowgroup_counter {0};
		size_t     rowgroup_offset {0};
		alp::state stt;
		size_t     rowgroups_count {1};
		size_t     vectors_count {1};

		/* Init */
		alp::AlpEncode<double>::init(data_column, rowgroup_offset, tuples_count, smp_arr, stt);

		ASSERT_EQ(stt.scheme, alp::SCHEME::ALP_RD);

		alp::AlpRD<double>::init(data_column, rowgroup_offset, tuples_count, smp_arr, stt);

		/* Encode - Decode - Validate. */
		for (size_t i = 0; i < tuples_count; i++) {
			value_to_encode     = data_column[i];
			dbl_arr[vector_idx] = value_to_encode;
			vector_idx          = vector_idx + 1;
			rowgroup_offset     = rowgroup_offset + 1;
			rowgroup_counter    = rowgroup_counter + 1;

			if (vector_idx != VECTOR_SIZE) { continue; }

			if (rowgroup_counter == ROWGROUP_SIZE) {
				rowgroup_counter = 0;
				rowgroups_count  = rowgroups_count + 1;
			}

			// Encode
			alp::AlpRD<double>::encode(dbl_arr, rd_exc_arr, pos_arr, exc_c_arr, right_arr, left_arr, stt);
			ffor::ffor(right_arr, ffor_right_arr, stt.right_bit_width, &stt.right_for_base);
			ffor::ffor(left_arr, ffor_left_arr, stt.left_bit_width, &stt.left_for_base);

			// Decode
			unffor::unffor(ffor_right_arr, unffor_right_arr, stt.right_bit_width, &stt.right_for_base);
			unffor::unffor(ffor_left_arr, unffor_left_arr, stt.left_bit_width, &stt.left_for_base);
			alp::AlpRD<double>::decode(
			    glue_arr, unffor_right_arr, unffor_left_arr, rd_exc_arr, pos_arr, exc_c_arr, stt);

			auto* dbl_glue_arr = reinterpret_cast<double*>(glue_arr);
			for (size_t j = 0; j < VECTOR_SIZE; ++j) {
				auto l = dbl_arr[j];
				auto r = dbl_glue_arr[j];
				if (l != r) { std::cerr << j << ", " << dataset.name << "\n"; }

				ASSERT_EQ(dbl_arr[j], dbl_glue_arr[j]);
			}

			alp_bench::VectorMetadata vector_metadata;
			vector_metadata.right_bit_width  = stt.right_bit_width;
			vector_metadata.left_bit_width   = stt.left_bit_width;
			vector_metadata.exceptions_count = stt.exceptions_count;

			compression_metadata.push_back(vector_metadata);
			vector_idx = 0;
			bit_width  = 0;

			vectors_count = vectors_count + 1;
		}

		auto compression_ratio = calculate_alprd_compression_size(compression_metadata);

		ofile << std::fixed << std::setprecision(2) << dataset.name << "," << compression_ratio << ","
		      << rowgroups_count << "," << vectors_count << std::endl;

		if (alp_bench::results.find(dataset.name) !=
		    alp_bench::results.end()) { // To avoid error when tested dataset is not found on results
			ASSERT_EQ(alp_bench::to_str(compression_ratio), alp_bench::results.find(dataset.name)->second);
		}
	}
}
