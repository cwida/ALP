#include "bench_gorillas.hpp"
#include "data.hpp"
#include "gorillas/gorillas.hpp"

static __attribute__((noinline)) benchmark::BenchmarkReporter::Run
bench_decode_gorillas(alp_bench::Column&                                   dataset,
                      alp_bench::GorillasDecompressionState<uint64_t>      gorillas_de_state,
                      alp_bench::FlagBuffer<false>                         flag_buffer,
                      alp_bench::GorillasConstants::Flags*                 flags,
                      uint64_t*                                            dec_arr,
                      uint8_t*                                             flags_arr,
                      uint8_t*                                             data_arr,
                      alp_bench::GorillasCompressionState<uint64_t, false> state) {

	int benchmark_number = dataset.id;

#ifdef NDEBUG
	uint64_t iterations = 300000;
#else
	uint64_t iterations = 1;
#endif

	std::string benchmark_name = dataset.name + "_decode";

	uint64_t cycles = benchmark::cycleclock::Now();
	for (uint64_t j = 0; j < iterations; ++j) {
		flags[0] = alp_bench::GorillasConstants::Flags::VALUE_IDENTICAL; // First value doesn't require a flag
		for (idx_t i = 0; i < 1023; i++) {
			flags[1 + i] = (alp_bench::GorillasConstants::Flags)flag_buffer.Extract();
		}

		for (idx_t i = 0; i < 1024; i++) {
			dec_arr[i] = alp_bench::GorillasDecompression<uint64_t>::Load(flags[i], gorillas_de_state);
		}
		gorillas_de_state.Reset();
		gorillas_de_state.input.SetStream(data_arr);
		flag_buffer.SetBuffer(flags_arr);
	}

	cycles = benchmark::cycleclock::Now() - cycles;

	return benchmark::BenchmarkReporter::Run(
	    benchmark_number, benchmark_name, iterations, double(cycles) / (double(iterations) * 1024));
}

static __attribute__((noinline)) benchmark::BenchmarkReporter::Run
bench_encode_gorillas(alp_bench::Column&                                   dataset,
                      alp_bench::GorillasCompressionState<uint64_t, false> state,
                      uint8_t*                                             data_arr,
                      uint8_t*                                             flags_arr,
                      uint64_t*                                            uint64_p,
                      double*                                              dbl_arr) {

	int benchmark_number = dataset.id;

#ifdef NDEBUG
	uint64_t iterations = 300000;
#else
	uint64_t iterations = 1;
#endif

	std::string benchmark_name = dataset.name + "_encode";

	uint64_t cycles = benchmark::cycleclock::Now();
	for (uint64_t j = 0; j < iterations; ++j) {
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
	}

	cycles = benchmark::cycleclock::Now() - cycles;

	return benchmark::BenchmarkReporter::Run(
	    benchmark_number, benchmark_name, iterations, double(cycles) / (double(iterations) * 1024));
}

void benchmark_all(benchmark::Benchmark& benchmark) {
	uint8_t*                                             data_arr;
	uint8_t*                                             flags_arr;
	double*                                              dbl_arr;
	uint64_t*                                            dec_arr;
	uint64_t*                                            uint64_p;
	alp_bench::GorillasCompressionState<uint64_t, false> state;
	alp_bench::GorillasConstants::Flags*                 flags;
	alp_bench::FlagBuffer<false>                         flag_buffer;
	alp_bench::GorillasDecompressionState<uint64_t>      gorillas_de_state;

	dbl_arr   = new (std::align_val_t {64}) double[1024];
	data_arr  = new (std::align_val_t {64}) uint8_t[8192 + 1024];
	flags_arr = new (std::align_val_t {64}) uint8_t[1025];
	dec_arr   = new (std::align_val_t {64}) uint64_t[1024];
	flags     = new (std::align_val_t {64}) alp_bench::GorillasConstants::Flags[1024];

	for (auto& dataset : alp_bench::alp_dataset) {
		std::ifstream ifile(dataset.sample_csv_file_path, std::ios::in);
		if (dataset.name.find("bw") != std::string::npos) { continue; }

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

		// Benchmark decoding
		benchmark.Run(bench_encode_gorillas(dataset, state, data_arr, flags_arr, uint64_p, dbl_arr));

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

		// Benchmark decoding
		benchmark.Run(
		    bench_decode_gorillas(dataset, gorillas_de_state, flag_buffer, flags, dec_arr, flags_arr, data_arr, state));

		ifile.close();
	}
}

int main() {
	benchmark::Benchmark benchmark =
	    benchmark::create("gorillas")
	        .save()
	        .at(std::string(SOURCE_DIR) + "/alp_pub/results/" + benchmark::CmakeInfo::getCmakeToolchainFile())
	        .print()
	        .add_extra_info(benchmark::CmakeInfo::getCmakeInfo());
	benchmark_all(benchmark);
}
