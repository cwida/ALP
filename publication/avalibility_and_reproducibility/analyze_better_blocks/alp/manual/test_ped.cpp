#include "PerfEvent.hpp"
#include "Units.hpp"
#include "datablock/schemes/CScheme.hpp"
#include "datablock/schemes/CSchemePicker.hpp"
#include "datablock/schemes/CSchemePool.hpp"
#include "datablock/schemes/v2/double/Decimal.hpp"
#include "datablock/schemes/v2/double/DoubleBP.hpp"
#include "datablock/schemes/v2/double/DynamicDictionary.hpp"
#include "datablock/schemes/v2/double/Frequency.hpp"
#include "datablock/schemes/v2/double/RLE.hpp"
#include "datablock/schemes/v2/integer/PBP.hpp"
#include "gflags/gflags.h"
#include "include/datasets.hpp"
#include "include/datasets_complete.hpp"
#include "spdlog/fmt/bundled/ranges.h"
#include "spdlog/spdlog.h"
#include "gtest/gtest.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

// for some reason, this is only DECLARED in DynamicDictionary but not defined (breaks linking)
// and then DEFINED in every cpp file that uses it
DEFINE_string(fsst_stats, "", "");
DEFINE_string(file_list_file, "pbi-double-columns.txt", "file-list");
DEFINE_int32(cascade_depth, 1, "cascade");

class ped_test : public ::testing::Test {
public:
	uint8_t* compressed_arr;
	double*  dbl_arr;
	double*  dec_dbl_arr;

	void SetUp() override {
		dbl_arr        = new double[1024];
		dec_dbl_arr    = new double[1024 * 100];
		compressed_arr = new uint8_t[1024 * 1000000000];
	}
	~ped_test() override {
		delete[] dbl_arr;
		delete[] compressed_arr;
		delete[] dec_dbl_arr;
	}
};
void setupSchemePool() {
	using namespace cengine::db;
	cengine::db::CSchemePool::refresh();
	auto& schemes = *cengine::db::CSchemePool::available_schemes;

	//	for (auto& scheme : schemes.double_schemes) {
	//		std::cout << ConvertSchemeTypeToString(scheme.first) << std::endl;
	//	}
	//	for (auto& scheme : schemes.integer_schemes) {
	//		std::cout << ConvertSchemeTypeToString(scheme.first) << std::endl;
	//	}

	//	 double: DOUBLE_BP, UNCOMPRESSED,
	for (auto it = schemes.double_schemes.begin(); it != schemes.double_schemes.end();) {
		if (it->first != DoubleSchemeType::DOUBLE_BP       //
		    && it->first != DoubleSchemeType::UNCOMPRESSED //
		) {
			it = schemes.double_schemes.erase(it);
		} else {
			++it;
		}
	}

	//	 int: X_FBP, UNCOMPRESSED,
	for (auto it = schemes.integer_schemes.begin(); it != schemes.integer_schemes.end();) {
		if (it->first != IntegerSchemeType::X_PBP           //
		    && it->first != IntegerSchemeType::UNCOMPRESSED //
		                                                    //		    && it->first != IntegerSchemeType::ONE_VALUE //
		) {
			it = schemes.integer_schemes.erase(it);
		} else {
			++it;
		}
	}
}

TEST_F(ped_test, test_one_vector) {
	setupSchemePool();
	cengine::db::v2::d::Decimal pd;
	std::cout << pd.selfDescription() << std::endl;
	for (auto& dataset : dataset::datasets) {
		std::ifstream ifile(dataset.file_path, std::ios::in);

		if (ifile.fail()) {
			std::cout << "ifile fails.";
			std::exit(-1);
		}

		// Read Data
		double num = 0.0;
		// keep storing values from the text file so long as data exists:
		size_t c {0};
		while (ifile >> num) {
			dbl_arr[c] = num;
			c          = c + 1;
		}

		/* Init Encoding */
		size_t              cascade = 2;
		size_t              output_bytes;
		size_t              size = 1024;
		std::vector<double> dst(size * 2, 0);

		cengine::db::DoubleStats stats(dbl_arr, nullptr, size);
		stats = cengine::db::DoubleStats::generateStats(dbl_arr, nullptr, size);

		/* Encode */
		output_bytes = pd.compress(dbl_arr, nullptr, compressed_arr, stats, cascade);

//		std::cout << pd.fullDescription(compressed_arr) << std::endl;

		/* Init decoding. */

		/* DECODE */
		pd.decompress(dst.data(), nullptr, compressed_arr, stats.tuple_count, cascade);

		/* Validate. */
		for (auto i = 0ul; i != size; ++i) {
			ASSERT_EQ(dbl_arr[i], dst[i]);
		}
		std::cout << dataset.name << " : " << output_bytes / (1.0 * size * sizeof(double)) * 64 << std::endl;
	}
}

TEST_F(ped_test, test_all_dataset) {
	setupSchemePool();
	cengine::db::v2::d::Decimal pd;
	//	std::cout << pd.selfDescription() << std::endl;
	for (auto& dataset : dataset::datasets_complete) {
		Vector<double> doubles(dataset.file_path.c_str());

		/* Init Encoding */
		size_t cascade = 2;
		size_t output_bytes;
		size_t size = doubles.size();
		std::cout << "size is : " << size << std::endl;
		std::vector<double> dst(size * 2, 0);

		cengine::db::DoubleStats stats(doubles.data, nullptr, size);
		stats = cengine::db::DoubleStats::generateStats(doubles.data, nullptr, size);

		/* Encode */
		output_bytes = pd.compress(doubles.data, nullptr, compressed_arr, stats, cascade);

		std::cout << pd.fullDescription(compressed_arr) << std::endl;

		/* Init decoding. */

		/* DECODE */
		pd.decompress(dst.data(), nullptr, compressed_arr, stats.tuple_count, cascade);

		/* Validate. */
		for (auto i = 0ul; i != size; ++i) {
			ASSERT_EQ(doubles.data[i], dst[i]);
		}
		std::cout << dataset.name << " : " << output_bytes / (1.0 * size * sizeof(double)) * 64 << std::endl;
	}
}