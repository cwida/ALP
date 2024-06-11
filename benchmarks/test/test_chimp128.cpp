#include "chimp/chimp128.hpp"
#include "data.hpp"
#include "gtest/gtest.h"
#include <fstream>

class chimp128_test : public ::testing::Test {
public:
	uint8_t*  data_arr;
	uint8_t*  flags_arr;
	uint8_t*  leading_zero_arr;
	uint16_t* packed_data_arr;
	double*   dbl_arr;
	double*   dec_dbl_p;
	uint64_t* dec_arr;
	uint64_t* uint64_p;

	// Encode
	alp_bench::Chimp128CompressionState<uint64_t, false> com_stt;
	uint8_t                                              leading_zero_block_count;

	// Decode
	idx_t                                           leading_zero_block_size;
	uint32_t                                        unpacked_index;
	uint32_t                                        leading_zero_index;
	alp_bench::FlagBuffer<false>                    flag_buffer;
	alp_bench::LeadingZeroBuffer<false>             leading_zero_buffer;
	alp_bench::Chimp128DecompressionState<uint64_t> chimp_de_state;

	alp_bench::ChimpConstants::Flags* flags;
	uint8_t*                          leading_zero_unpacked;
	alp_bench::UnpackedData*          unpacked_data_arr;

	void SetUp() override {
		dbl_arr               = new double[1024];
		data_arr              = new uint8_t[8096];
		flags_arr             = new uint8_t[1025];
		leading_zero_arr      = new uint8_t[1024];
		dec_arr               = new uint64_t[1024];
		packed_data_arr       = new uint16_t[1024];
		flags                 = new alp_bench::ChimpConstants::Flags[1025];
		leading_zero_unpacked = new uint8_t[1024];
		unpacked_data_arr     = new alp_bench::UnpackedData[1024];
	}

	~chimp128_test() override {
		delete[] dbl_arr;
		delete[] data_arr;
		delete[] flags_arr;
		delete[] leading_zero_arr;
		delete[] dec_arr;
		delete[] packed_data_arr;
		delete[] flags;
		delete[] leading_zero_unpacked;
		delete[] unpacked_data_arr;
	}
};

TEST_F(chimp128_test, test_chimp) {
	for (auto& dataset : alp_bench::alp_dataset) {
		std::ifstream ifile(dataset.sample_csv_file_path, std::ios::in);
		ASSERT_EQ(ifile.fail(), false);

		// Read Data
		double num = 0.0;
		// keep storing values from the text file so long as data exists:
		size_t c {0};
		while (ifile >> num) {
			dbl_arr[c] = num;
			c          = c + 1;
		}

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
		uint64_p = reinterpret_cast<uint64_t*>(dbl_arr);
		for (size_t i {0}; i < 1024; ++i) {
			alp_bench::Chimp128Compression<uint64_t, false>::Store(uint64_p[i], com_stt);
		}
		com_stt.Flush();
		com_stt.output.Flush();

		// Init decoding
		leading_zero_block_count = com_stt.leading_zero_buffer.BlockCount();
		leading_zero_block_size  = static_cast<idx_t>(leading_zero_block_count) * 8;
		unpacked_index           = 0;
		leading_zero_index       = 0;
		chimp_de_state.input.SetStream(data_arr);
		flag_buffer.SetBuffer(flags_arr);
		leading_zero_buffer.SetBuffer(leading_zero_arr);

		/*
		 *
		 * DECODE
		 *
		 */

		// Decode flags
		flags[0] = alp_bench::ChimpConstants::Flags::VALUE_IDENTICAL; // First value doesn't require a flag
		for (idx_t i = 0; i < 1023; i++) {
			flags[1 + i] = (alp_bench::ChimpConstants::Flags)flag_buffer.Extract();
		}

		// Decode leading zero
		for (idx_t i = 0; i < leading_zero_block_size; i++) {
			leading_zero_unpacked[i] =
			    alp_bench::ChimpConstants::Decompression::LEADING_REPRESENTATION[leading_zero_buffer.Extract()];
		}

		/*
		 * count how many cases of 'TRAILING_EXCEEDS_THRESHOLD' are based on the flags
		 * that is the exact number of packed data blocks
		 * that is the case in which in Chimp128 they save data in a block of 16 bits
		 */
		idx_t packed_data_block_count = 0;
		for (idx_t i = 0; i < 1024; i++) {
			packed_data_block_count += flags[1 + i] == alp_bench::ChimpConstants::Flags::TRAILING_EXCEEDS_THRESHOLD;
		}

		for (idx_t i = 0; i < packed_data_block_count; i++) {
			alp_bench::PackedDataUtils<uint64_t>::Unpack(((uint16_t*)packed_data_arr)[i], unpacked_data_arr[i]);
			if (unpacked_data_arr[i].significant_bits == 0) { unpacked_data_arr[i].significant_bits = 64; }
			unpacked_data_arr[i].leading_zero =
			    alp_bench::ChimpConstants::Decompression::LEADING_REPRESENTATION[unpacked_data_arr[i].leading_zero];
		}

		chimp_de_state.Reset();

		for (idx_t i = 0; i < 1024; i++) {
			dec_arr[i] = alp_bench::Chimp128Decompression<uint64_t>::Load(
			    flags[i], leading_zero_unpacked, leading_zero_index, unpacked_data_arr, unpacked_index, chimp_de_state);
		}

		dec_dbl_p = reinterpret_cast<double*>(dec_arr);

		for (size_t i = 0; i < 1024; ++i) {
			ASSERT_EQ(dbl_arr[i], dec_dbl_p[i]);
		}

		ifile.close();
	}
}