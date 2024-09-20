#include "bench_ped.hpp"
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
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

// for some reason, this is only DECLARED in DynamicDictionary but not defined (breaks linking)
// and then DEFINED in every cpp file that uses it
DEFINE_string(fsst_stats, "", "");
DEFINE_string(file_list_file, "pbi-double-columns.txt", "file-list");
DEFINE_int32(cascade_depth, 1, "cascade");

static __attribute__((noinline)) benchmark::BenchmarkReporter::Run
bench_decoding_ped(dataset::Dataset&            dataset,        //
                   cengine::db::v2::d::Decimal& pd,             //
                   cengine::db::DoubleStats&    stats,          //
                   double*                      dst,            //
                   uint8_t*                     compressed_arr, //
                   size_t                       cascade) {

	int benchmark_number = dataset.id;

#ifdef NDEBUG
	uint64_t iterations = 300000;
#else
	uint64_t iterations = 1;
#endif

	std::string benchmark_name = dataset.name + "_decode";

	uint64_t cycles = benchmark::cycleclock::Now();
	for (uint64_t j = 0; j < iterations; ++j) {
		pd.decompress(dst, nullptr, compressed_arr, stats.tuple_count, cascade);
	}

	cycles = benchmark::cycleclock::Now() - cycles;

	return benchmark::BenchmarkReporter::Run(
	    benchmark_number, benchmark_name, iterations, double(cycles) / (double(iterations) * 1024));
}

static __attribute__((noinline)) benchmark::BenchmarkReporter::Run
bench_encoding_ped(dataset::Dataset&            dataset,        //
                   cengine::db::v2::d::Decimal& pd,             //
                   double*                      dbl_arr,        //
                   uint8_t*                     compressed_arr, //
                   cengine::db::DoubleStats&    stats,
                   size_t                       cascade) {

	int benchmark_number = dataset.id;

#ifdef NDEBUG
	uint64_t iterations = 300;
#else
	uint64_t iterations = 1;
#endif

	std::string benchmark_name = dataset.name + "_encode";
	size_t      output_bytes {0};

	uint64_t cycles = benchmark::cycleclock::Now();
	for (uint64_t j = 0; j < iterations; ++j) {
		output_bytes = pd.compress(dbl_arr, nullptr, compressed_arr, stats, cascade);
	}

	cycles = benchmark::cycleclock::Now() - cycles;

	return benchmark::BenchmarkReporter::Run(
	    output_bytes, benchmark_name, iterations, double(cycles) / (double(iterations) * 1024));
}
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
		    && it->first != DoubleSchemeType::ONE_VALUE    //
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

void benchmark_all(benchmark::Benchmark& benchmark) {

	uint8_t* compressed_arr;
	double*  dbl_arr;
	double*  dec_dbl_arr;

	dbl_arr        = new (std::align_val_t {64}) double[1024];
	dec_dbl_arr    = new (std::align_val_t {64}) double[1024 * 100];
	compressed_arr = new (std::align_val_t {64}) uint8_t[1024 * 1000000000];

	for (auto& dataset : dataset::datasets) {
		std::ifstream ifile(dataset.file_path, std::ios::in);

		// check to see that the file was opened correctly:
		if (!ifile.is_open()) {
			std::cerr << "There was a problem opening the input file!\n";
			exit(1); // exit or do additional error checking
		}

		double num = 0.0;
		// keep storing values from the text file so long as data exists:
		size_t c {0};
		while (ifile >> num) {
			dbl_arr[c] = num;
			c += 1;
		}

		/* Init encoding */
		setupSchemePool();
		cengine::db::v2::d::Decimal pd;
		size_t                      cascade = 2;
		size_t                      output_bytes;
		size_t                      size = 1024;
		std::vector<double>         dst(size * 2, 0);
		cengine::db::DoubleStats    stats(dbl_arr, nullptr, size);
		stats = cengine::db::DoubleStats::generateStats(dbl_arr, nullptr, size);

		/* Benchmark Encoding */
		benchmark.Run(bench_encoding_ped(dataset, pd, dbl_arr, compressed_arr, stats, cascade));

		/* Encode */
		output_bytes = pd.compress(dbl_arr, nullptr, compressed_arr, stats, cascade);

		// Init decoding

		// Benchmark decoding
		benchmark.Run(bench_decoding_ped(dataset, pd, stats, dst.data(), compressed_arr, cascade));

		ifile.close();
	}
}

int main() {
	benchmark::Benchmark benchmark =
	    benchmark::create("ped")
	        .save()
	        .at(std::string(SOURCE_DIR) + "/fls_pub/results/" + benchmark::CmakeInfo::getCmakeToolchainFile())
	        .print()
	        .add_extra_info(benchmark::CmakeInfo::getCmakeInfo());
	benchmark_all(benchmark);
}
