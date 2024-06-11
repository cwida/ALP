#include "bench_patas.hpp"
#include "data.hpp"
#include "patas/patas.hpp"

static __attribute__((noinline)) benchmark::BenchmarkReporter::Run
bench_decoding_patas(alp_bench::Column&                         dataset,
                     uint16_t*                                  packed_metadata,
                     uint8_t*                                   data_arr,
                     uint64_t*                                  dec_arr,
                     alp_bench::ByteReader                      byte_reader,
                     alp_bench::patas::PatasUnpackedValueStats* unpacked_data) {

	int benchmark_number = dataset.id;

#ifdef NDEBUG
	uint64_t iterations = 300000;
#else
	uint64_t iterations = 1;
#endif

	std::string benchmark_name = dataset.name + "_decode";

	uint64_t cycles = benchmark::cycleclock::Now();
	for (uint64_t j = 0; j < iterations; ++j) {
		byte_reader.SetStream(data_arr);

		// UNPACKING METADATA (16 bits - 3 bytes)
		for (idx_t i = 0; i < 1024; i++) {
			alp_bench::PackedDataUtils<uint64_t>::Unpack(((uint16_t*)packed_metadata)[i],
			                                             (alp_bench::UnpackedData&)unpacked_data[i]);
		}
		dec_arr[0] = (uint64_t)0; // Not sure why without this, it does not work on the > 2nd iteration...
		for (idx_t i = 0; i < 1024; i++) {
			dec_arr[i] = alp_bench::patas::PatasDecompression<uint64_t>::DecompressValue(
			    byte_reader,
			    unpacked_data[i].significant_bytes,
			    unpacked_data[i].trailing_zeros,
			    dec_arr[i - unpacked_data[i].index_diff]);
		}
	}

	cycles = benchmark::cycleclock::Now() - cycles;

	return benchmark::BenchmarkReporter::Run(
	    benchmark_number, benchmark_name, iterations, double(cycles) / (double(iterations) * 1024));
}

static __attribute__((noinline)) benchmark::BenchmarkReporter::Run
bench_encoding_patas(alp_bench::Column&                                       dataset,
                     alp_bench::patas::PatasCompressionState<uint64_t, false> patas_state,
                     uint8_t*                                                 data_arr,
                     uint16_t*                                                packed_metadata,
                     uint64_t*                                                uint64_p,
                     double*                                                  dbl_arr) {

	int benchmark_number = dataset.id;

#ifdef NDEBUG
	uint64_t iterations = 300000;
#else
	uint64_t iterations = 1;
#endif

	std::string benchmark_name = dataset.name + "_encode";

	uint64_t cycles = benchmark::cycleclock::Now();
	for (uint64_t j = 0; j < iterations; ++j) {
		patas_state.Reset();
		patas_state.SetOutputBuffer(data_arr);
		patas_state.packed_data_buffer.SetBuffer(packed_metadata);

		/*
		 * Encode
		 */
		uint64_p = reinterpret_cast<uint64_t*>(dbl_arr);
		for (size_t i {0}; i < 1024; ++i) {
			alp_bench::patas::PatasCompression<uint64_t, false>::Store(uint64_p[i], patas_state);
		}
	}

	cycles = benchmark::cycleclock::Now() - cycles;

	return benchmark::BenchmarkReporter::Run(
	    benchmark_number, benchmark_name, iterations, double(cycles) / (double(iterations) * 1024));
}

void benchmark_all(benchmark::Benchmark& benchmark) {
	double*                                                  dbl_arr;
	uint8_t*                                                 data_arr;
	uint64_t*                                                dec_arr;
	uint64_t*                                                uint64_p;
	uint16_t*                                                packed_metadata;
	alp_bench::patas::PatasCompressionState<uint64_t, false> patas_state;
	alp_bench::patas::PatasUnpackedValueStats*               unpacked_data;
	alp_bench::ByteReader                                    byte_reader;

	data_arr        = new (std::align_val_t {64}) uint8_t[8096];
	dec_arr         = new (std::align_val_t {64}) uint64_t[1024];
	packed_metadata = new (std::align_val_t {64}) uint16_t[1024];
	unpacked_data   = new (std::align_val_t {64}) alp_bench::patas::PatasUnpackedValueStats[1024];
	dbl_arr         = new (std::align_val_t {64}) double[1024];

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

		// Init Encoding
		patas_state.Reset();
		patas_state.SetOutputBuffer(data_arr);
		patas_state.packed_data_buffer.SetBuffer(packed_metadata);

		/*
		 * Encode
		 */
		uint64_p = reinterpret_cast<uint64_t*>(dbl_arr);
		for (size_t i {0}; i < 1024; ++i) {
			alp_bench::patas::PatasCompression<uint64_t, false>::Store(uint64_p[i], patas_state);
		}

		// Benchmark Encoding
		benchmark.Run(bench_encoding_patas(dataset, patas_state, data_arr, packed_metadata, uint64_p, dbl_arr));

		// Init decoding
		byte_reader.SetStream(data_arr);

		// Benchmark decoding
		benchmark.Run(bench_decoding_patas(dataset, packed_metadata, data_arr, dec_arr, byte_reader, unpacked_data));

		ifile.close();
	}
}

int main() {
	benchmark::Benchmark benchmark =
	    benchmark::create("patas")
	        .save()
	        .at(std::string(SOURCE_DIR) + "/alp_pub/results/" + benchmark::CmakeInfo::getCmakeToolchainFile())
	        .print()
	        .add_extra_info(benchmark::CmakeInfo::getCmakeInfo());
	benchmark_all(benchmark);
}
