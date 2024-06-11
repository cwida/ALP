#include "bench_chimp.hpp"
#include "chimp/chimp.hpp"
#include "data.hpp"

static __attribute__((noinline)) benchmark::BenchmarkReporter::Run
bench_decode_chimp(alp_bench::Column&                                dataset,
                   idx_t                                             leading_zero_block_size,
                   uint32_t                                          leading_zero_index,
                   alp_bench::ChimpDecompressionState<uint64_t>      chimp_de_state,
                   alp_bench::FlagBuffer<false>                      flag_buffer,
                   alp_bench::LeadingZeroBuffer<false>               leading_zero_buffer,
                   alp_bench::ChimpConstants::Flags*                 flags,
                   uint8_t*                                          leading_zero_unpacked,
                   uint64_t*                                         dec_arr,
                   uint8_t*                                          flags_arr,
                   uint8_t*                                          data_arr,
                   uint8_t*                                          leading_zero_arr,
                   uint8_t                                           leading_zero_block_count,
                   alp_bench::ChimpCompressionState<uint64_t, false> state) {

	int benchmark_number = dataset.id;

#ifdef NDEBUG
	uint64_t iterations = 300000;
#else
	uint64_t iterations = 1;
#endif

	std::string benchmark_name = dataset.name + "_decode";

	uint64_t cycles = benchmark::cycleclock::Now();
	for (uint64_t j = 0; j < iterations; ++j) {
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
		chimp_de_state.input.SetStream(data_arr);
		flag_buffer.SetBuffer(flags_arr);
		leading_zero_buffer.SetBuffer(leading_zero_arr);
		leading_zero_block_count = state.leading_zero_buffer.BlockCount();
		leading_zero_block_size  = static_cast<int64_t>(leading_zero_block_count) * 8;
		leading_zero_index       = 0;
	}

	cycles = benchmark::cycleclock::Now() - cycles;

	return benchmark::BenchmarkReporter::Run(
	    benchmark_number, benchmark_name, iterations, double(cycles) / (double(iterations) * 1024));
}

static __attribute__((noinline)) benchmark::BenchmarkReporter::Run
bench_encode_chimp(alp_bench::Column&                                dataset,
                   alp_bench::ChimpCompressionState<uint64_t, false> state,
                   uint8_t*                                          data_arr,
                   uint8_t*                                          flags_arr,
                   uint8_t*                                          leading_zero_arr,
                   uint64_t*                                         uint64_p,
                   double*                                           dbl_arr) {

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
	}

	cycles = benchmark::cycleclock::Now() - cycles;

	return benchmark::BenchmarkReporter::Run(
	    benchmark_number, benchmark_name, iterations, double(cycles) / (double(iterations) * 1024));
}

void benchmark_all(benchmark::Benchmark& benchmark) {
	uint8_t*                                          data_arr;
	uint8_t*                                          flags_arr;
	uint8_t*                                          leading_zero_arr;
	double*                                           dbl_arr;
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

	dbl_arr               = new (std::align_val_t {64}) double[1024];
	data_arr              = new (std::align_val_t {64}) uint8_t[8096];
	flags_arr             = new (std::align_val_t {64}) uint8_t[1025];
	leading_zero_arr      = new (std::align_val_t {64}) uint8_t[1024];
	dec_arr               = new (std::align_val_t {64}) uint64_t[1024];
	flags                 = new (std::align_val_t {64}) alp_bench::ChimpConstants::Flags[1024];
	leading_zero_unpacked = new (std::align_val_t {64}) uint8_t[1024];

	for (auto& dataset : alp_bench::alp_dataset) {
		std::ifstream ifile(dataset.sample_csv_file_path, std::ios::in);

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
		benchmark.Run(bench_encode_chimp(dataset, state, data_arr, flags_arr, leading_zero_arr, uint64_p, dbl_arr));

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

		// Benchmark decoding
		benchmark.Run(bench_decode_chimp(dataset,
		                                 leading_zero_block_size,
		                                 leading_zero_index,
		                                 chimp_de_state,
		                                 flag_buffer,
		                                 leading_zero_buffer,
		                                 flags,
		                                 leading_zero_unpacked,
		                                 dec_arr,
		                                 flags_arr,
		                                 data_arr,
		                                 leading_zero_arr,
		                                 leading_zero_block_count,
		                                 state));

		ifile.close();
	}
}

int main() {
	benchmark::Benchmark benchmark =
	    benchmark::create("chimp")
	        .save()
	        .at(std::string(SOURCE_DIR) + "/alp_pub/results/" + benchmark::CmakeInfo::getCmakeToolchainFile())
	        .print()
	        .add_extra_info(benchmark::CmakeInfo::getCmakeInfo());
	benchmark_all(benchmark);
}
