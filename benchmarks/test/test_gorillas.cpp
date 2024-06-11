#include "data.hpp"
#include "gorillas/gorillas.hpp"
#include "gtest/gtest.h"
#include <fstream>

class gorillas_test : public ::testing::Test {
public:
	uint8_t*                                             data_arr;
	uint8_t*                                             flags_arr;
	double*                                              dbl_arr;
	double*                                              dec_dbl_p;
	uint64_t*                                            dec_arr;
	uint64_t*                                            uint64_p;
	alp_bench::GorillasCompressionState<uint64_t, false> state;
	alp_bench::GorillasConstants::Flags*                 flags;
	alp_bench::FlagBuffer<false>                         flag_buffer;
	alp_bench::GorillasDecompressionState<uint64_t>      gorillas_de_state;

	void SetUp() override {
		dbl_arr   = new double[1024];
		data_arr  = new uint8_t[8192 + 1024];
		flags_arr = new uint8_t[1025];
		dec_arr   = new uint64_t[1024];
		flags     = new alp_bench::GorillasConstants::Flags[1024];
	}

	~gorillas_test() override {
		delete[] dbl_arr;
		delete[] data_arr;
		delete[] flags_arr;
		delete[] dec_arr;
	}
};

TEST_F(gorillas_test, test_gorillas) {
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
		state.flag_buffer.SetBuffer(flags_arr);

		/*
		 *
		 * Encode
		 *
		 */
		uint64_p = reinterpret_cast<uint64_t*>(dbl_arr);
		for (size_t i {0}; i < 1024; ++i) {
			alp_bench::GorillasCompression<uint64_t, false>::Store(uint64_p[i], state);
		}

		state.Flush();
		state.output.Flush();

		// Init decoding
		gorillas_de_state.input.SetStream(data_arr);
		flag_buffer.SetBuffer(flags_arr);

		/*
		 *
		 * DECODE
		 *
		 */
		flags[0] = alp_bench::GorillasConstants::Flags::VALUE_IDENTICAL; // First value doesn't require a flag
		for (idx_t i = 0; i < 1023; i++) {
			flags[1 + i] = (alp_bench::GorillasConstants::Flags)flag_buffer.Extract();
		}

		for (idx_t i = 0; i < 1024; i++) {
			dec_arr[i] = alp_bench::GorillasDecompression<uint64_t>::Load(flags[i], gorillas_de_state);
		}
		gorillas_de_state.Reset();

		dec_dbl_p = reinterpret_cast<double*>(dec_arr);

		for (size_t i = 0; i < 1024; ++i) {
			auto l = dbl_arr[i];
			auto r = dec_dbl_p[i];
			if (l != r) { std::cerr << l << ", " << r << dataset.name << "\n"; }

			ASSERT_EQ(dbl_arr[i], dec_dbl_p[i]);
		}

		ifile.close();
	}
}