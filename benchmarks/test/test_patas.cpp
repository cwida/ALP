#include "data.hpp"
#include "patas/patas.hpp"
#include "gtest/gtest.h"
#include <fstream>

class patas_test : public ::testing::Test {
public:
	uint8_t*  data_arr;
	uint16_t* packed_data_arr;
	double*   dbl_arr;
	double*   dec_dbl_p;
	uint64_t* dec_arr;
	uint64_t* uint64_p;

	// Encode
	uint16_t*                                                packed_metadata;
	alp_bench::patas::PatasCompressionState<uint64_t, false> patas_state;

	// Decode
	alp_bench::patas::PatasUnpackedValueStats* unpacked_data;
	alp_bench::ByteReader                      byte_reader;

	void SetUp() override {
		dbl_arr         = new double[1024];
		data_arr        = new uint8_t[8096];
		dec_arr         = new uint64_t[1024];
		packed_data_arr = new uint16_t[1024];
		packed_metadata = new uint16_t[1024];
		unpacked_data   = new alp_bench::patas::PatasUnpackedValueStats[1024];
	}

	~patas_test() override {
		delete[] dbl_arr;
		delete[] data_arr;
		delete[] dec_arr;
		delete[] packed_data_arr;
		delete[] packed_metadata;
		delete[] unpacked_data;
	}
};

TEST_F(patas_test, one_vec) {
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
		patas_state.Reset();
		patas_state.SetOutputBuffer(data_arr);
		patas_state.packed_data_buffer.SetBuffer(packed_metadata);

		/*
		 *
		 * Encode
		 *
		 */
		uint64_p = reinterpret_cast<uint64_t*>(dbl_arr);
		for (size_t i {0}; i < 1024; ++i) {
			alp_bench::patas::PatasCompression<uint64_t, false>::Store(uint64_p[i], patas_state);
		}

		// Init decoding
		byte_reader.SetStream(data_arr);

		/*
		 *
		 * DECODE
		 *
		 */

		// UNPACKING METADATA (16 bits - 3 bytes)
		for (idx_t i = 0; i < 1024; i++) {
			alp_bench::PackedDataUtils<uint64_t>::Unpack(((uint16_t*)packed_metadata)[i],
			                                             (alp_bench::UnpackedData&)unpacked_data[i]);
		}

		// USING UNPACKED METADATA AND DATA BUFFER WE LOAD THE DOUBLE VALUES
		dec_arr[0] = (uint64_t)0; // Not sure why without this, it does not work on the > 2nd iteration...
		for (idx_t i = 0; i < 1024; i++) {
			dec_arr[i] = alp_bench::patas::PatasDecompression<uint64_t>::DecompressValue(
			    byte_reader,
			    unpacked_data[i].significant_bytes,
			    unpacked_data[i].trailing_zeros,
			    dec_arr[i - unpacked_data[i].index_diff]);
		}

		dec_dbl_p = reinterpret_cast<double*>(dec_arr);

		std::cout << dataset.name << std::endl;
		for (size_t i = 0; i < 1024; ++i) {
			ASSERT_EQ(dbl_arr[i], dec_dbl_p[i]);
		}

		ifile.close();
	}
}