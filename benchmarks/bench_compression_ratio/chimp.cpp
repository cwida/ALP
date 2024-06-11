#include "chimp/chimp.hpp"
#include "alp.hpp"
#include "data.hpp"
#include "test/mapper.hpp"
#include "gtest/gtest.h"

class chimp_test : public ::testing::Test {
public:
	uint8_t*                                          data_arr;
	uint8_t*                                          flags_arr;
	uint8_t*                                          leading_zero_arr;
	double*                                           dbl_arr;
	double*                                           dec_dbl_p;
	uint64_t*                                         dec_arr;
	uint64_t*                                         uint64_p;
	alp_bench::ChimpCompressionState<uint64_t, false> state;
	alp_bench::ChimpConstants::Flags*                 flags;
	uint8_t*                                          leading_zero_unpacked;
	alp_bench::FlagBuffer<false>                      flag_buffer;
	alp_bench::LeadingZeroBuffer<false>               leading_zero_buffer;
	alp_bench::ChimpDecompressionState<uint64_t>      chimp_de_state;
	uint32_t                                          leading_zero_index;
	uint8_t                                           leading_zero_block_count;
	size_t                                            leading_zero_block_size;

	void SetUp() override {
		dbl_arr               = new double[1024];
		data_arr              = new uint8_t[8096];
		flags_arr             = new uint8_t[1025];
		leading_zero_arr      = new uint8_t[1024];
		dec_arr               = new uint64_t[1024];
		leading_zero_unpacked = new uint8_t[1024];
		flags                 = new alp_bench::ChimpConstants::Flags[1024];
	}

	~chimp_test() override {
		delete[] dbl_arr;
		delete[] data_arr;
		delete[] flags_arr;
		delete[] leading_zero_arr;
		delete[] dec_arr;
	}
};

/*
 * Chimp overhead per vector in a hypothetic file format = leading_zero_block_count + size_of_data_block +
 * start_of_data; Start of Data is needed if Data Blocks and Metadata are stored separately (like in DuckDB to optimize
 * decoding speed)
 */
double chimp_overhead_per_vector {static_cast<double>(8 + 16 + 16)};

TEST_F(chimp_test, test_chimp_on_whole_datasets) {

	if (const auto v = std::getenv("ALP_DATASET_DIR_PATH"); v == nullptr) {
		throw std::runtime_error("Environment variable ALP_DATASET_DIR_PATH is not set!");
	}

	std::ofstream ofile(alp_bench::PATHS.RESULT_DIR_PATH + "chimp_compression_ratio.csv", std::ios::out);
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
			state.leading_zero_buffer.SetBuffer(leading_zero_arr);
			state.flag_buffer.SetBuffer(flags_arr);

			/*
			 *
			 * Encode
			 *
			 */
			uint64_p = reinterpret_cast<uint64_t*>(dbl_arr);
			for (size_t i {0}; i < alp::config::VECTOR_SIZE; ++i) {
				alp_bench::ChimpCompression<uint64_t, false>::Store(uint64_p[i], state);
			}

			state.Flush();
			state.output.Flush();

			// SUM COMPRESSION SIZE
			compressed_data_size += 16;
			size_t bytes_used_by_data                = state.output.BytesWritten();
			size_t flag_bytes                        = state.flag_buffer.BytesUsed();
			size_t leading_zero_block_count          = state.leading_zero_buffer.BlockCount();
			size_t bytes_used_by_leading_zero_blocks = 3 * leading_zero_block_count;
			compressed_data_size +=
			    (alp_bench::AlignValue(bytes_used_by_data) + flag_bytes + bytes_used_by_leading_zero_blocks) * 8;
			compressed_data_size += chimp_overhead_per_vector;

			// Init decoding
			leading_zero_block_count = state.leading_zero_buffer.BlockCount();
			leading_zero_block_size  = static_cast<int64_t>(leading_zero_block_count) * 8;
			leading_zero_index       = 0;
			chimp_de_state.input.SetStream(data_arr);
			flag_buffer.SetBuffer(flags_arr);
			leading_zero_buffer.SetBuffer(leading_zero_arr);

			flags[0] = alp_bench::ChimpConstants::Flags::VALUE_IDENTICAL; // First value doesn't require a flag
			for (size_t i = 0; i < alp::config::VECTOR_SIZE - 1; i++) {
				flags[1 + i] = (alp_bench::ChimpConstants::Flags)flag_buffer.Extract();
			}

			for (size_t i = 0; i < leading_zero_block_size; i++) {
				leading_zero_unpacked[i] =
				    alp_bench::ChimpConstants::Decompression::LEADING_REPRESENTATION[leading_zero_buffer.Extract()];
			}

			for (size_t i = 0; i < alp::config::VECTOR_SIZE; i++) {
				dec_arr[i] = alp_bench::ChimpDecompression<uint64_t>::Load(
				    flags[i], leading_zero_unpacked, leading_zero_index, chimp_de_state);
			}

			chimp_de_state.Reset();

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
