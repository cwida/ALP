#include "bench_zstd.hpp"
#include "data.hpp"
#include "mapper.hpp"
#include "zstd.h"

// NOLINTBEGIN

static __attribute__((noinline)) benchmark::BenchmarkReporter::Run
bench_decode_zstd(alp_bench::ALPColumnDescriptor& dataset, void* enc_arr, size_t enc_size, void* dec_arr) {

	int benchmark_number = dataset.id;

#ifdef NDEBUG
	uint64_t iterations = 300000 / 128;
#else
	uint64_t iterations = 1;
#endif

	std::string benchmark_name = dataset.name + "_decode";
	size_t      DECODED_SIZE   = 8 * 131072;

	uint64_t cycles = benchmark::cycleclock::Now();
	for (uint64_t j = 0; j < iterations; ++j) {
		ZSTD_decompress(dec_arr, DECODED_SIZE, enc_arr, enc_size);
	}

	cycles = benchmark::cycleclock::Now() - cycles;

	return benchmark::BenchmarkReporter::Run(
	    benchmark_number, benchmark_name, iterations, double(cycles) / (double(iterations) * 131072));
}

static __attribute__((noinline)) benchmark::BenchmarkReporter::Run
bench_encode_zstd(alp_bench::ALPColumnDescriptor& dataset, double* dbl_arr, void* enc_arr) {

	int benchmark_number = dataset.id;

#ifdef NDEBUG
	uint64_t iterations = 300000 / 128;
#else
	uint64_t iterations = 1;
#endif

	std::string benchmark_name = dataset.name + "_encode";

	size_t ENC_SIZE_UPPER_BOUND = 8 * 131072;
	size_t INPUT_SIZE           = ENC_SIZE_UPPER_BOUND;

	uint64_t cycles = benchmark::cycleclock::Now();
	for (uint64_t j = 0; j < iterations; ++j) {
		ZSTD_compress(enc_arr, ENC_SIZE_UPPER_BOUND, dbl_arr, INPUT_SIZE, 3);
	}

	cycles = benchmark::cycleclock::Now() - cycles;

	return benchmark::BenchmarkReporter::Run(
	    benchmark_number, benchmark_name, iterations, double(cycles) / (double(iterations) * 131072));
}

void benchmark_all(benchmark::Benchmark& benchmark) {
	double* dbl_arr;
	void*   enc_arr;
	void*   dec_arr;
	size_t  enc_size;

	dbl_arr = new (std::align_val_t {64}) double[131072];
	enc_arr = (void*)new (std::align_val_t {64}) double[131072];
	dec_arr = (void*)new (std::align_val_t {64}) double[131072];

	for (auto& dataset : alp_bench::get_alp_dataset()) {

		size_t tup_c;
		const auto* col = mapper::mmap_file<double>(tup_c, dataset.binary_file_path);
		for (size_t i = 0; i < 131072; i++) {
			dbl_arr[i] = col[i];
		}

		// Benchmark encoding
		benchmark.Run(bench_encode_zstd(dataset, dbl_arr, enc_arr));

		// Init Encoding
		size_t ENC_SIZE_UPPER_BOUND = 8 * 131072;
		size_t INPUT_SIZE           = ENC_SIZE_UPPER_BOUND;

		// To store ENC_SIZE
		size_t const ENC_SIZE = ZSTD_compress(enc_arr, ENC_SIZE_UPPER_BOUND, dbl_arr, INPUT_SIZE, 3);
		//		printf("%6u -> %7u\n", (unsigned)INPUT_SIZE, (unsigned)ENC_SIZE);

		// Benchmark decoding
		benchmark.Run(bench_decode_zstd(dataset, enc_arr, ENC_SIZE, dec_arr));
	}
}

int main() {
	benchmark::Benchmark benchmark =
	    benchmark::create("zstd")
	        .save()
	        .at(std::string(SOURCE_DIR) + "/publication/results/" + benchmark::CmakeInfo::getCmakeToolchainFile())
	        .print()
	        .add_extra_info(benchmark::CmakeInfo::getCmakeInfo());
	benchmark_all(benchmark);
}

// NOLINTEND
