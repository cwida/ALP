#include "bench_chimp128.hpp"
#include "chimp/chimp128.hpp"
#include "data.hpp"

static __attribute__((noinline)) benchmark::BenchmarkReporter::Run
bench_decode_chimp128(const double*                                        dbl_arr,
                      alp_bench::Column&                                   dataset,
                      uint8_t                                              leading_zero_block_count,
                      alp_bench::Chimp128CompressionState<uint64_t, false> com_stt,
                      idx_t                                                leading_zero_block_size,
                      uint32_t                                             unpacked_index,
                      uint32_t                                             leading_zero_index,
                      alp_bench::Chimp128DecompressionState<uint64_t>      chimp_de_state,
                      uint8_t*                                             data_arr,
                      alp_bench::FlagBuffer<false>                         flag_buffer,
                      uint8_t*                                             flags_arr,
                      alp_bench::LeadingZeroBuffer<false>                  leading_zero_buffer,
                      uint8_t*                                             leading_zero_arr,
                      alp_bench::ChimpConstants::Flags*                    flags,
                      uint16_t*                                            packed_data_arr,
                      uint8_t*                                             leading_zero_unpacked,
                      alp_bench::UnpackedData*                             unpacked_data_arr,
                      uint64_t*                                            dec_arr) {

	int benchmark_number = dataset.id;

#ifdef NDEBUG
	uint64_t iterations = 300000;
#else
	uint64_t iterations = 1;
#endif

	std::string benchmark_name = dataset.name + "_decode";

	uint64_t cycles = benchmark::cycleclock::Now();
	for (uint64_t j = 0; j < iterations; ++j) {
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
	}

	cycles = benchmark::cycleclock::Now() - cycles;

	return benchmark::BenchmarkReporter::Run(
	    benchmark_number, benchmark_name, iterations, double(cycles) / (double(iterations) * 1024));
}

static __attribute__((noinline)) benchmark::BenchmarkReporter::Run
bench_encode_chimp128(alp_bench::Column&                                   dataset,
                      alp_bench::Chimp128CompressionState<uint64_t, false> com_stt,
                      uint8_t                                              leading_zero_block_count,
                      idx_t                                                leading_zero_block_size,
                      uint32_t                                             unpacked_index,
                      uint32_t                                             leading_zero_index,
                      alp_bench::Chimp128DecompressionState<uint64_t>      chimp_de_state,
                      uint8_t*                                             data_arr,
                      alp_bench::FlagBuffer<false>                         flag_buffer,
                      uint8_t*                                             flags_arr,
                      alp_bench::LeadingZeroBuffer<false>                  leading_zero_buffer,
                      uint8_t*                                             leading_zero_arr,
                      alp_bench::ChimpConstants::Flags*                    flags,
                      uint16_t*                                            packed_data_arr,
                      uint8_t*                                             leading_zero_unpacked,
                      alp_bench::UnpackedData*                             unpacked_data_arr,
                      uint64_t*                                            dec_arr) {

	int benchmark_number = dataset.id;

#ifdef NDEBUG
	uint64_t iterations = 300000;
#else
	uint64_t iterations = 1;
#endif

	std::string benchmark_name = dataset.name + "_encode";

	uint64_t cycles = benchmark::cycleclock::Now();
	for (uint64_t j = 0; j < iterations; ++j) {
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
	}

	cycles = benchmark::cycleclock::Now() - cycles;

	return benchmark::BenchmarkReporter::Run(
	    benchmark_number, benchmark_name, iterations, double(cycles) / (double(iterations) * 1024));
}

void benchmark_all(benchmark::Benchmark& benchmark) {
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
	alp_bench::ChimpConstants::Flags*               flags;
	uint8_t*                                        leading_zero_unpacked;
	alp_bench::UnpackedData*                        unpacked_data_arr;

	dbl_arr               = new (std::align_val_t {64}) double[1024];
	data_arr              = new (std::align_val_t {64}) uint8_t[8096];
	flags_arr             = new (std::align_val_t {64}) uint8_t[1025];
	leading_zero_arr      = new (std::align_val_t {64}) uint8_t[1024];
	dec_arr               = new (std::align_val_t {64}) uint64_t[1024];
	packed_data_arr       = new (std::align_val_t {64}) uint16_t[1024];
	flags                 = new (std::align_val_t {64}) alp_bench::ChimpConstants::Flags[1025];
	leading_zero_unpacked = new (std::align_val_t {64}) uint8_t[1024];
	unpacked_data_arr     = new (std::align_val_t {64}) alp_bench::UnpackedData[1024];

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

		// Benchmark encoding
		benchmark.Run(bench_encode_chimp128(dataset,
		                                    com_stt,
		                                    leading_zero_block_count,
		                                    leading_zero_block_size,
		                                    unpacked_index,
		                                    leading_zero_index,
		                                    chimp_de_state,
		                                    data_arr,
		                                    flag_buffer,
		                                    flags_arr,
		                                    leading_zero_buffer,
		                                    leading_zero_arr,
		                                    flags,
		                                    packed_data_arr,
		                                    leading_zero_unpacked,
		                                    unpacked_data_arr,
		                                    dec_arr));

		// Benchmark decoding
		benchmark.Run(bench_decode_chimp128(dbl_arr,
		                                    dataset,
		                                    leading_zero_block_count,
		                                    com_stt,
		                                    leading_zero_block_size,
		                                    unpacked_index,
		                                    leading_zero_index,
		                                    chimp_de_state,
		                                    data_arr,
		                                    flag_buffer,
		                                    flags_arr,
		                                    leading_zero_buffer,
		                                    leading_zero_arr,
		                                    flags,
		                                    packed_data_arr,
		                                    leading_zero_unpacked,
		                                    unpacked_data_arr,
		                                    dec_arr));

		ifile.close();
	}
}

int main() {
	benchmark::Benchmark benchmark =
	    benchmark::create("chimp128")
	        .save()
	        .at(std::string(SOURCE_DIR) + "/alp_pub/results/" + benchmark::CmakeInfo::getCmakeToolchainFile())
	        .print()
	        .add_extra_info(benchmark::CmakeInfo::getCmakeInfo());
	benchmark_all(benchmark);
}
