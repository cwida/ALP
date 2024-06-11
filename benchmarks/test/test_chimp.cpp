#include "chimp/chimp.hpp"
#include "data.hpp"
#include "gtest/gtest.h"
#include <fstream>

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
	idx_t                                             leading_zero_block_size;

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

TEST_F(chimp_test, test_chimp) {
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
		for (size_t i {0}; i < 1024; ++i) {
			alp_bench::ChimpCompression<uint64_t, false>::Store(uint64_p[i], state);
		}

		state.Flush();
		state.output.Flush();

		// Init decoding
		leading_zero_block_count = state.leading_zero_buffer.BlockCount();
		leading_zero_block_size  = static_cast<int64_t>(leading_zero_block_count) * 8;
		leading_zero_index       = 0;
		chimp_de_state.input.SetStream(data_arr);
		flag_buffer.SetBuffer(flags_arr);
		leading_zero_buffer.SetBuffer(leading_zero_arr);

		/*
		 *
		 * DECODE
		 *
		 */
		flags[0] = alp_bench::ChimpConstants::Flags::VALUE_IDENTICAL; // First value doesn't require a flag
		for (idx_t i = 0; i < 1023; i++) {
			flags[1 + i] = (alp_bench::ChimpConstants::Flags)flag_buffer.Extract();
		}

		for (idx_t i = 0; i < leading_zero_block_size; i++) {
			leading_zero_unpacked[i] =
			    alp_bench::ChimpConstants::Decompression::LEADING_REPRESENTATION[leading_zero_buffer.Extract()];
		}

		for (idx_t i = 0; i < 1024; i++) {
			dec_arr[i] = alp_bench::ChimpDecompression<uint64_t>::Load(
			    flags[i], leading_zero_unpacked, leading_zero_index, chimp_de_state);
		}
		chimp_de_state.Reset();

		dec_dbl_p = reinterpret_cast<double*>(dec_arr);

		for (size_t i = 0; i < 1024; ++i) {
			ASSERT_EQ(dbl_arr[i], dec_dbl_p[i]);
		}

		ifile.close();
	}
}