#include "patas/patas.hpp"
#include "alp.hpp"
#include "data.hpp"
#include "mapper.hpp"
#include "gtest/gtest.h"

// NOLINTBEGIN

class patas_test : public ::testing::Test {
public:
	uint8_t*  data_arr;
	uint16_t* packed_data_arr;

	// Encode
	uint16_t*                                  packed_metadata;
	alp_bench::patas::PatasUnpackedValueStats* unpacked_data;

	// Decode
	alp_bench::ByteReader byte_reader;

	void SetUp() override {
		data_arr        = new uint8_t[8192 + 2048]; // We leave some overhead room in case of negative compression
		packed_data_arr = new uint16_t[1024];
		packed_metadata = new uint16_t[1024];
		unpacked_data   = new alp_bench::patas::PatasUnpackedValueStats[1024];
	}

	~patas_test() override {
		delete[] data_arr;
		delete[] packed_data_arr;
		delete[] packed_metadata;
		delete[] unpacked_data;
	}

	/*
	 * Patas overhead per vector in a hypothetic file format = next_block_offset;
	 * Next block offset is needed to be able to skip blocks of data
	 */
	double patas_overhead_per_vector {static_cast<double>(16)};

	template <typename T, int N_DATASETS>
	void bench_compression_ratio(const std::array<alp_bench::ALPColumnDescriptor, N_DATASETS>& datasets, const std::string& path) {
		using INNERTYPE =
		    typename std::conditional_t<std::is_same_v<T, double>,
		                                uint64_t,
		                                typename std::conditional_t<std::is_same_v<T, float>, uint32_t, void>>;

		alp_bench::patas::PatasCompressionState<INNERTYPE, false> patas_state;
		INNERTYPE*                                                uint64_p;
		auto*                                                     dbl_arr = new T[1024];
		T*                                                        dec_dbl_p;
		auto*                                                     dec_arr = new INNERTYPE[1024];

		std::ofstream ofile(path, std::ios::out);
		ofile << "dataset,size,vectors_count\n";

		for (auto& dataset : datasets) {
			std::cout << dataset.name << std::endl;

			size_t compressed_data_size = 0;

			size_t tuples_count;
			auto*  data_column = mapper::mmap_file<T>(tuples_count, dataset.binary_file_path);
			T value_to_encode {0.0};
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
				patas_state.SetOutputBuffer(data_arr);
				patas_state.packed_data_buffer.SetBuffer(packed_metadata);
				patas_state.Reset();

				/*
				 *
				 * Encode
				 *
				 */
				uint64_p = reinterpret_cast<INNERTYPE*>(dbl_arr);
				for (size_t i {0}; i < alp::config::VECTOR_SIZE; ++i) {
					alp_bench::patas::PatasCompression<INNERTYPE, false>::Store(uint64_p[i], patas_state);
				}

				// SUM COMPRESSION SIZE
				size_t bytes_used_by_data = patas_state.byte_writer.BytesWritten();
				size_t packed_data_size   = patas_state.packed_data_buffer.index * sizeof(uint16_t);
				compressed_data_size += (alp_bench::AlignValue(bytes_used_by_data) + packed_data_size) * 8;
				compressed_data_size += patas_overhead_per_vector;

				// Init decoding
				byte_reader.SetStream(data_arr);

				/*
				 *
				 * DECODE
				 *
				 */
				// UNPACKING METADATA (16 bits)
				for (size_t i = 0; i < alp::config::VECTOR_SIZE; i++) {
					alp_bench::PackedDataUtils<uint64_t>::Unpack(packed_metadata[i],
					                                             (alp_bench::UnpackedData&)unpacked_data[i]);
				}

				// USING UNPACKED METADATA AND DATA BUFFER WE LOAD THE DOUBLE VALUES
				dec_arr[0] = (uint64_t)0;
				for (size_t i = 0; i < alp::config::VECTOR_SIZE; i++) {
					dec_arr[i] = alp_bench::patas::PatasDecompression<uint64_t>::DecompressValue(
					    byte_reader,
					    unpacked_data[i].significant_bytes,
					    unpacked_data[i].trailing_zeros,
					    dec_arr[i - unpacked_data[i].index_diff]);
				}

				for (size_t j = 0; j < alp::config::VECTOR_SIZE; j++) {
					if (uint64_p[j] != dec_arr[j]) {
						std::cout << j << ", " << rowgroup_offset << ", " << dataset.name << std::endl;
					}
					ASSERT_EQ(uint64_p[j], dec_arr[j]);
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

TEST_F(patas_test, test_patas128_on_whole_datasets) {
	auto result_path = alp_bench::get_paths().result_dir_path + "compression_ratio_result/double/patas.csv";
	bench_compression_ratio<double, 30>(alp_bench::get_alp_dataset(), result_path);
}

TEST_F(patas_test, test_patas128_on_float_datasets) {
	auto result_path = alp_bench::get_paths().result_dir_path + "compression_ratio_result/float/patas.csv";
	bench_compression_ratio<float, 4>(alp_bench::get_sp_datasets(), result_path);
}

// NOLINTEND
