#include "gorillas/gorillas.hpp"
#include "alp.hpp"
#include "data.hpp"
#include "test/mapper.hpp"
#include "gtest/gtest.h"

class gorillas_test : public ::testing::Test {
public:
	double*                                              dbl_arr;
	double*                                              dec_dbl_p;
	uint64_t*                                            dec_arr;
	uint64_t*                                            uint64_p;
	alp_bench::GorillasCompressionState<uint64_t, false> state;
	alp_bench::GorillasConstants::Flags*                 flags;
	alp_bench::GorillasDecompressionState<uint64_t>      gorillas_de_state;
	uint8_t*                                             data_arr;
	uint8_t*                                             flags_arr;

	void SetUp() override {
		dbl_arr   = new double[1024];
		data_arr  = new uint8_t[8192 + 1024]; // We leave some headroom in case of negative compression
		dec_arr   = new uint64_t[1024];
		flags     = new alp_bench::GorillasConstants::Flags[1024];
		flags_arr = new uint8_t[1024];
	}

	~gorillas_test() override {
		delete[] dbl_arr;
		delete[] data_arr;
		delete[] flags_arr;
		delete[] dec_arr;
	}
};

/*
 * Gorillas overhead per vector in a hypothetic file format = size_of_data_block + start_of_data + size_of_data;
 * Start of Data is needed if Data Blocks and Metadata are stored separately (like in DuckDB to optimize decoding speed)
 */
double gorillas_overhead_per_vector {static_cast<double>(16 + 16 + 16)};

TEST_F(gorillas_test, test_gorillas_on_whole_datasets) {

	if (const auto v = std::getenv("ALP_DATASET_DIR_PATH"); v == nullptr) {
		throw std::runtime_error("Environment variable ALP_DATASET_DIR_PATH is not set!");
	}

	std::ofstream ofile(alp_bench::PATHS.RESULT_DIR_PATH + "gorillas_compression_ratio.csv", std::ios::out);
	ofile << "dataset,size,vectors_count\n";

	for (auto& dataset : alp_bench::alp_dataset) {

		std::cout << dataset.name << std::endl;

		size_t compressed_data_size = 0;

		size_t tuples_count;
		auto*  data_column = mapper::mmap_file<double>(tuples_count, dataset.binary_file_path);
		double value_to_encode {0.0};
		size_t vector_idx {0};
		size_t rowgroup_offset {0};
		size_t vectors_count = {0};
		/* Encode - Decode - Validate. */
		for (size_t i = 0; i < tuples_count; i++) {
			value_to_encode     = data_column[i];
			dbl_arr[vector_idx] = value_to_encode;
			vector_idx          = vector_idx + 1;
			rowgroup_offset     = rowgroup_offset + 1;

			if (vector_idx != alp::config::VECTOR_SIZE) { continue; }

			// Init Encoding
			state.Reset();
			state.output.SetStream(data_arr);
			state.flag_buffer.SetBuffer(flags_arr);

			/*
			 *
			 * Encode
			 *
			 */
			uint64_p = reinterpret_cast<uint64_t*>(dbl_arr);
			for (size_t i {0}; i < alp::config::VECTOR_SIZE; i++) {
				alp_bench::GorillasCompression<uint64_t, false>::Store(uint64_p[i], state);
			}

			state.Flush();
			state.output.Flush();

			// SUM COMPRESSION SIZE
			size_t bytes_used_by_data = state.output.BytesWritten();
			size_t flag_bytes         = state.flag_buffer.BytesUsed();
			compressed_data_size += (alp_bench::AlignValue(bytes_used_by_data) + flag_bytes) * 8;
			compressed_data_size += gorillas_overhead_per_vector;

			// Init decoding
			gorillas_de_state.input.SetStream(data_arr);
			alp_bench::FlagBuffer<false> flag_buffer;
			flag_buffer.SetBuffer(flags_arr);

			/*
			 *
			 * DECODE
			 *
			 */
			flags[0] = alp_bench::GorillasConstants::Flags::VALUE_IDENTICAL; // First value doesn't require a flag
			for (size_t i = 0; i < alp::config::VECTOR_SIZE - 1; i++) {
				flags[1 + i] = (alp_bench::GorillasConstants::Flags)flag_buffer.Extract();
			}

			gorillas_de_state.Reset();

			for (size_t i = 0; i < alp::config::VECTOR_SIZE; i++) {
				dec_arr[i] = alp_bench::GorillasDecompression<uint64_t>::Load(flags[i], gorillas_de_state);
			}

			dec_dbl_p = reinterpret_cast<double*>(dec_arr);

			for (size_t j = 0; j < alp::config::VECTOR_SIZE; j++) {
				auto l = dbl_arr[j];
				auto r = dec_dbl_p[j];
				if (l != r) { std::cerr << j << ", " << rowgroup_offset << ", " << dataset.name << "\n"; }
				ASSERT_EQ(dbl_arr[j], dec_dbl_p[j]);
			}
			vector_idx = 0;
			vectors_count += 1;
		}
		auto processed_tuples  = vectors_count * alp::config::VECTOR_SIZE;
		auto compression_ratio = (double)compressed_data_size / processed_tuples;

		ofile << std::fixed << std::setprecision(2) << dataset.name << "," << compression_ratio << "," << vectors_count
		      << std::endl;
	}
}
