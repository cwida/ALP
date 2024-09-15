#include "alp.hpp"
#include "data.hpp"
#include "gtest/gtest.h"
#include <fstream>

class arm64v8_neon_intrinsic_1024_uf1_falp: public ::testing::Test
{
	public:
	   double * dbl_arr;
	   double * exc_arr;
	   uint16_t * pos_arr;
	   uint16_t * exc_c_arr;
	   int64_t * ffor_arr;
	   int64_t * unffor_arr;
	   int64_t * base_arr;
	   int64_t * dig_arr;
	   double * dec_dbl_arr;
	   uint8_t bw;
	   uint8_t factor;
	   uint8_t exponent;
	   double * smp_arr;
	void SetUp() override
	{
		dbl_arr = new double[1024];
		exc_arr = new double[1024];
		pos_arr = new uint16_t[1024];
		dig_arr = new int64_t[1024];
		dec_dbl_arr = new double[1024];
		exc_c_arr = new uint16_t[1024];
		ffor_arr = new int64_t[1024];
		unffor_arr = new int64_t[1024];
		base_arr = new int64_t[1024];
		smp_arr = new double[1024];
	}
	~arm64v8_neon_intrinsic_1024_uf1_falp () override
	{
		delete[] dbl_arr;
		delete[] exc_arr;
		delete[] pos_arr;
		delete[] dig_arr;
		delete[] dec_dbl_arr;
		delete[] exc_c_arr;
		delete[] ffor_arr;
		delete[] unffor_arr;
		delete[] base_arr;
		delete[] smp_arr;
	}
}
;
TEST_F(arm64v8_neon_intrinsic_1024_uf1_falp, fused)
{
	for (auto & dataset : alp_bench::alp_dataset)
	{
		std:: ifstream         ifile(dataset.csv_file_path, std::ios:: in );
		ASSERT_EQ(ifile.fail(), false);
		   alp::state<double> stt;
		if (dataset.suitable_for_cutting) { continue; }
		if (dataset.name.find("bw") != std::string::npos) { continue; }
		double num = 0.0;
		size_t c {0};
		while (ifile >> num)
		{
			dbl_arr[c] = num;
			c          = c + 1;
		}
		// Init
		alp::encoder<double>::init(dbl_arr, 0, 1024, smp_arr, stt);
		// Encode
		alp::encoder<double>::encode(dbl_arr, exc_arr, pos_arr, exc_c_arr, dig_arr, stt);
		alp::encoder<double>::analyze_ffor(dig_arr, bw, base_arr);
		fastlanes::generated::ffor::fallback::scalar::ffor(dig_arr, ffor_arr, bw, base_arr);
		// Decode
		generated::falp::arm64v8::neon::falp(reinterpret_cast < uint64_t * > (ffor_arr), dec_dbl_arr, bw, reinterpret_cast < uint64_t * > (base_arr),stt.fac, stt.exp);
		alp::AlpDecode<double>::patch_exceptions(dec_dbl_arr, exc_arr, pos_arr, exc_c_arr);
		for (size_t i = 0; i < 1024; ++i)
		{
			ASSERT_EQ(dbl_arr[i], dec_dbl_arr[i]);
		}
		ASSERT_EQ(dataset.exceptions_count, exc_c_arr[0]);
		ASSERT_EQ(dataset.bit_width, bw);
		ifile.close();
	}
}

TEST_F(arm64v8_neon_intrinsic_1024_uf1_falp, unfused)
{
	for (auto & dataset : alp_bench::alp_dataset)
	{
		std:: ifstream         ifile(dataset.csv_file_path, std::ios:: in );
		ASSERT_EQ(ifile.fail(), false);
		   alp::state<double> stt;
		if (dataset.suitable_for_cutting) { continue; }
		if (dataset.name.find("bw") != std::string::npos) { continue; }
		double num = 0.0;
		size_t c {0};
		while (ifile >> num)
		{
			dbl_arr[c] = num;
			c          = c + 1;
		}
		// Init
		alp::encoder<double>::init(dbl_arr, 0, 1024, smp_arr, stt);
		// Encode
		alp::encoder<double>::encode(dbl_arr, exc_arr, pos_arr, exc_c_arr, dig_arr, stt);
		alp::encoder<double>::analyze_ffor(dig_arr, bw, base_arr);
		fastlanes::generated::ffor::fallback::scalar::ffor(dig_arr, ffor_arr, bw, base_arr);
		// Decode
		fastlanes::generated::unffor::fallback::scalar::unffor(ffor_arr, unffor_arr, bw, base_arr);
		alp::AlpDecode<double>::decode(unffor_arr, stt.fac, stt.exp, dec_dbl_arr);
		alp::AlpDecode<double>::patch_exceptions(dec_dbl_arr, exc_arr, pos_arr, exc_c_arr);
		for (size_t i = 0; i < 1024; ++i)
		{
			ASSERT_EQ(dbl_arr[i], dec_dbl_arr[i]);
		}
		ASSERT_EQ(dataset.exceptions_count, exc_c_arr[0]);
		ASSERT_EQ(dataset.bit_width, bw);
		ifile.close();
	}
}
