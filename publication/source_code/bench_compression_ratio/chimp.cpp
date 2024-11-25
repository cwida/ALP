#include "chimp/chimp.hpp"
#include "alp.hpp"
#include "data.hpp"
#include "mapper.hpp"
#include "gtest/gtest.h"

// NOLINTBEGIN

/*
 * Chimp overhead per vector in a hypothetic file format = leading_zero_block_count + size_of_data_block +
 * start_of_data; Start of Data is needed if Data Blocks and Metadata are stored separately (like in DuckDB to optimize
 * decoding speed)
 */
double chimp_overhead_per_vector {static_cast<double>(8 + 16 + 16)};

class chimp_test : public ::testing::Test {
public:
	uint8_t*                            data_arr;
	uint8_t*                            flags_arr;
	uint8_t*                            leading_zero_arr;
	alp_bench::ChimpConstants::Flags*   flags;
	uint8_t*                            leading_zero_unpacked;
	alp_bench::FlagBuffer<false>        flag_buffer;
	alp_bench::LeadingZeroBuffer<false> leading_zero_buffer;
	uint32_t                            leading_zero_index;
	uint8_t                             leading_zero_block_count;
	size_t                              leading_zero_block_size;

	void SetUp() override {
		data_arr              = new uint8_t[8096];
		flags_arr             = new uint8_t[1025];
		leading_zero_arr      = new uint8_t[1024];
		leading_zero_unpacked = new uint8_t[1024];
		flags                 = new alp_bench::ChimpConstants::Flags[1024];
	}

	~chimp_test() override {
		delete[] data_arr;
		delete[] flags_arr;
		delete[] leading_zero_arr;
	}

	template <typename T, int N_DATASETS>
	void bench_compression_ratio(const std::array<alp_bench::ALPColumnDescriptor, N_DATASETS>& datasets, const std::string& path) {
		if (const auto v = std::getenv("ALP_DATASET_DIR_PATH"); v != nullptr) {
			alp_bench::get_paths().alp_dataset_binary_dir_path = *v;
		}

		using INNERTYPE =
		    typename std::conditional_t<std::is_same_v<T, double>,
		                                uint64_t,
		                                typename std::conditional_t<std::is_same_v<T, float>, uint32_t, void>>;

		alp_bench::ChimpDecompressionState<INNERTYPE>      chimp_de_state;
		alp_bench::ChimpCompressionState<INNERTYPE, false> state;
		INNERTYPE*                                         uint64_p;
		auto*                                              dbl_arr = new T[1024];
		T*                                                 dec_dbl_p;
		auto*                                              dec_arr = new INNERTYPE[1024];

		std::ofstream ofile(path, std::ios::out);
		ofile << "dataset,size,vectors_count\n";

		for (auto& dataset : datasets) {
			std::cout << dataset.name << std::endl;

			size_t compressed_data_size = 0;

			size_t tuples_count;
			auto*  data_column = mapper::mmap_file<T>(tuples_count, dataset.binary_file_path);
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
				uint64_p = reinterpret_cast<INNERTYPE*>(dbl_arr);
				for (size_t val_idx {0}; val_idx < alp::config::VECTOR_SIZE; ++val_idx) {
					alp_bench::ChimpCompression<INNERTYPE, false>::Store(uint64_p[val_idx], state);
				}

				state.Flush();
				state.output.Flush();

				// SUM COMPRESSION SIZE
				compressed_data_size += 16;
				size_t bytes_used_by_data                = state.output.BytesWritten();
				size_t flag_bytes                        = state.flag_buffer.BytesUsed();
				size_t a_leading_zero_block_count        = state.leading_zero_buffer.BlockCount();
				size_t bytes_used_by_leading_zero_blocks = 3 * a_leading_zero_block_count;
				compressed_data_size +=
				    (alp_bench::AlignValue(bytes_used_by_data) + flag_bytes + bytes_used_by_leading_zero_blocks) * 8;
				compressed_data_size += chimp_overhead_per_vector;

				// Init decoding
				a_leading_zero_block_count = state.leading_zero_buffer.BlockCount();
				leading_zero_block_size    = static_cast<int64_t>(a_leading_zero_block_count) * 8;
				leading_zero_index         = 0;
				chimp_de_state.input.SetStream(data_arr);
				flag_buffer.SetBuffer(flags_arr);
				leading_zero_buffer.SetBuffer(leading_zero_arr);

				flags[0] = alp_bench::ChimpConstants::Flags::VALUE_IDENTICAL; // First value doesn't require a flag
				for (size_t val_idx = 0; val_idx < alp::config::VECTOR_SIZE - 1; val_idx++) {
					flags[1 + val_idx] = (alp_bench::ChimpConstants::Flags)flag_buffer.Extract();
				}

				for (size_t val_idx = 0; val_idx < leading_zero_block_size; val_idx++) {
					leading_zero_unpacked[val_idx] =
					    alp_bench::ChimpConstants::Decompression::LEADING_REPRESENTATION[leading_zero_buffer.Extract()];
				}

				for (size_t val_idx = 0; val_idx < alp::config::VECTOR_SIZE; val_idx++) {
					dec_arr[val_idx] = alp_bench::ChimpDecompression<INNERTYPE>::Load(
					    flags[val_idx], leading_zero_unpacked, leading_zero_index, chimp_de_state);
				}

				chimp_de_state.Reset();

				dec_dbl_p = reinterpret_cast<T*>(dec_arr);

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

			ofile << std::fixed << std::setprecision(2) << dataset.name << "," << compression_ratio << ","
			      << vectors_count << std::endl;
		}

		delete[] dbl_arr;
		delete[] dec_arr;
	}
};

TEST_F(chimp_test, test_chimp128_on_whole_datasets) {
	auto result_path = alp_bench::get_paths().result_dir_path + "compression_ratio_result/double/chimp.csv";
	bench_compression_ratio<double, 30>(alp_bench::get_alp_dataset(), result_path);
}

TEST_F(chimp_test, test_chimp128_on_float_datasets) {
	auto result_path = alp_bench::get_paths().result_dir_path + "compression_ratio_result/float/chimp.csv";
	bench_compression_ratio<float, 4>(alp_bench::get_sp_datasets(), result_path);
}

// NOLINTEND