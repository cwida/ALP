#include "chimp/chimp128.hpp"
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
double chimp128_overhead_per_vector {static_cast<double>(8 + 16 + 16)};

class chimp128_test : public ::testing::Test {
public:
	uint8_t*                            data_arr;
	uint8_t*                            flags_arr;
	uint8_t*                            leading_zero_arr;
	uint16_t*                           packed_data_arr;
	alp_bench::ChimpConstants::Flags*   flags;
	uint8_t*                            leading_zero_unpacked;
	alp_bench::UnpackedData*            unpacked_data_arr;
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
		packed_data_arr       = new uint16_t[1024];
		unpacked_data_arr     = new alp_bench::UnpackedData[1024];
	}

	~chimp128_test() override {
		delete[] data_arr;
		delete[] flags_arr;
		delete[] leading_zero_arr;
		delete[] packed_data_arr;
		delete[] unpacked_data_arr;
	}

	template <typename T, int N_DATASETS>
	void bench_compression_ratio(const std::array<alp_bench::Column, N_DATASETS>& datasets, const std::string& path) {
		using INNERTYPE =
		    typename std::conditional_t<std::is_same_v<T, double>,
		                                uint64_t,
		                                typename std::conditional_t<std::is_same_v<T, float>, uint32_t, void>>;

		alp_bench::Chimp128CompressionState<INNERTYPE, false> com_stt;
		alp_bench::Chimp128DecompressionState<INNERTYPE>      chimp_de_state;
		T*                                                    dec_dbl_p;
		auto*                                                 dbl_arr = new T[1024];
		auto*                                                 dec_arr = new INNERTYPE[1024];
		INNERTYPE*                                            uint64_p;

		if (const auto v = std::getenv("ALP_DATASET_DIR_PATH"); v == nullptr) {
			throw std::runtime_error("Environment variable ALP_DATASET_DIR_PATH is not set!");
		}

		std::ofstream ofile(path, std::ios::out);
		ofile << "dataset,size,vectors_count\n";

		for (auto& dataset : datasets) {

			std::cout << dataset.name << std::endl;

			size_t compressed_data_size = 0;

			size_t tuples_count;
			auto*  data_column = mapper::mmap_file<T>(tuples_count, dataset.binary_file_path);
			T      value_to_encode {0.0};
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
				com_stt.Reset();
				com_stt.output.SetStream(data_arr);
				com_stt.leading_zero_buffer.SetBuffer(leading_zero_arr);
				com_stt.flag_buffer.SetBuffer(flags_arr);
				com_stt.packed_data_buffer.SetBuffer(packed_data_arr);

				/*
				 *
				 * Encode
				 *
				 */
				uint64_p = reinterpret_cast<INNERTYPE*>(dbl_arr);
				for (size_t value_idx {0}; value_idx < alp::config::VECTOR_SIZE; ++value_idx) {
					alp_bench::Chimp128Compression<INNERTYPE, false>::Store(uint64_p[value_idx], com_stt);
				}

				com_stt.Flush();
				com_stt.output.Flush();

				// SUM COMPRESSION SIZE
				size_t   bytes_used_by_data                     = com_stt.output.BytesWritten();
				size_t   flag_bytes                             = com_stt.flag_buffer.BytesUsed();
				size_t   a_leading_zero_block_count             = com_stt.leading_zero_buffer.BlockCount();
				size_t   bytes_used_by_leading_zero_blocks      = 3 * a_leading_zero_block_count;
				uint16_t packed_data_blocks_count               = com_stt.packed_data_buffer.index;
				uint64_t bytes_used_by_packed_data_blocks_count = packed_data_blocks_count * sizeof(uint16_t);
				compressed_data_size += (alp_bench::AlignValue(bytes_used_by_data) + flag_bytes +
				                         bytes_used_by_leading_zero_blocks + bytes_used_by_packed_data_blocks_count) *
				                        8;
				compressed_data_size += chimp128_overhead_per_vector;

				// Init decoding
				a_leading_zero_block_count = com_stt.leading_zero_buffer.BlockCount();
				leading_zero_block_size    = static_cast<size_t>(a_leading_zero_block_count) * 8;
				uint32_t unpacked_index    = 0;
				leading_zero_index         = 0;
				chimp_de_state.input.SetStream(data_arr);
				flag_buffer.SetBuffer(flags_arr);
				leading_zero_buffer.SetBuffer(leading_zero_arr);

				// Decode flags
				flags[0] = alp_bench::ChimpConstants::Flags::VALUE_IDENTICAL; // First value doesn't require a flag
				for (size_t val_idx = 0; val_idx < alp::config::VECTOR_SIZE - 1; val_idx++) {
					flags[1 + val_idx] = (alp_bench::ChimpConstants::Flags)flag_buffer.Extract();
				}

				// Decode leading zero
				for (size_t val_idx = 0; val_idx < leading_zero_block_size; val_idx++) {
					leading_zero_unpacked[val_idx] =
					    alp_bench::ChimpConstants::Decompression::LEADING_REPRESENTATION[leading_zero_buffer.Extract()];
				}

				/*
				 * count how many cases of 'TRAILING_EXCEEDS_THRESHOLD' are based on the flags
				 * that is the exact number of packed data blocks
				 * that is the case in which in Chimp128 they save data in a block of 16 bits
				 */
				size_t packed_data_block_count = 0;
				for (size_t val_idx = 0; val_idx < alp::config::VECTOR_SIZE; val_idx++) {
					packed_data_block_count +=
					    flags[1 + val_idx] == alp_bench::ChimpConstants::Flags::TRAILING_EXCEEDS_THRESHOLD;
				}

				for (size_t val_idx = 0; val_idx < packed_data_block_count; val_idx++) {
					alp_bench::PackedDataUtils<INNERTYPE>::Unpack(((uint16_t*)packed_data_arr)[val_idx],
					                                              unpacked_data_arr[val_idx]);
					if (unpacked_data_arr[val_idx].significant_bits == 0) {
						unpacked_data_arr[val_idx].significant_bits = 64;
					}
					unpacked_data_arr[val_idx].leading_zero =
					    alp_bench::ChimpConstants::Decompression::LEADING_REPRESENTATION[unpacked_data_arr[val_idx]
					                                                                         .leading_zero];
				}

				chimp_de_state.Reset();

				for (size_t val_idx = 0; val_idx < alp::config::VECTOR_SIZE; val_idx++) {
					dec_arr[val_idx] = alp_bench::Chimp128Decompression<INNERTYPE>::Load(flags[val_idx],
					                                                                     leading_zero_unpacked,
					                                                                     leading_zero_index,
					                                                                     unpacked_data_arr,
					                                                                     unpacked_index,
					                                                                     chimp_de_state);
				}

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

TEST_F(chimp128_test, test_chimp128_on_whole_datasets) {
	auto result_path = alp_bench::get_paths().result_dir_path + "compression_ratio_result/double/chimp128.csv";
	bench_compression_ratio<double, 30>(alp_bench::get_alp_dataset(), result_path);
}

TEST_F(chimp128_test, test_chimp128_on_float_datasets) {
	auto result_path = alp_bench::get_paths().result_dir_path + "compression_ratio_result/float/chimp128.csv";
	bench_compression_ratio<float, 4>(alp_bench::get_sp_datasets(), result_path);
}

// NOLINTEND
