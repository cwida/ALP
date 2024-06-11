#include "alp.hpp"
#include "bench_alp.hpp"
#include "data.hpp"

/* Bench ALP encode. */
static __attribute__((noinline)) benchmark::BenchmarkReporter::Run b_a_e(const double*      dbl_arr,
                                                                         double*            exc_arr,
                                                                         uint16_t*          pos_arr,
                                                                         uint16_t*          exc_c_arr,
                                                                         int64_t*           encoded_arr,
                                                                         uint8_t&           bw,
                                                                         int64_t*           ffor_arr,
                                                                         int64_t*           base_arr,
                                                                         alp::state&        stt,
                                                                         alp_bench::Column& dataset) {

	int benchmark_number = dataset.id;

#ifdef NDEBUG
	uint64_t iterations = 300000;
#else
	uint64_t iterations = 1;
#endif

	std::string benchmark_name = dataset.name + "_encode";

	uint64_t cycles = benchmark::cycleclock::Now();
	for (uint64_t i = 0; i < iterations; ++i) {
		alp::AlpEncode<double>::encode(dbl_arr, exc_arr, pos_arr, exc_c_arr, encoded_arr, stt);
		alp::AlpEncode<double>::analyze_ffor(encoded_arr, bw, base_arr);
		ffor::ffor(encoded_arr, ffor_arr, bw, base_arr);
	}

	cycles = benchmark::cycleclock::Now() - cycles;

	return benchmark::BenchmarkReporter::Run(
	    benchmark_number, benchmark_name, iterations, double(cycles) / (double(iterations) * 1024));
}

void benchmark_all(benchmark::Benchmark& benchmark) {
	double*   dbl_arr;
	double*   exc_arr;
	uint16_t* pos_arr;
	uint16_t* exc_c_arr;
	int64_t*  ffor_arr;
	int64_t*  base_arr;
	int64_t*  encoded_arr;
	double*   dec_dbl_arr;
	double*   rg_smp_arr;

	uint8_t bw;

	dbl_arr     = new (std::align_val_t {64}) double[1024];
	exc_arr     = new (std::align_val_t {64}) double[1024];
	pos_arr     = new (std::align_val_t {64}) uint16_t[1024];
	encoded_arr = new (std::align_val_t {64}) int64_t[1024];
	dec_dbl_arr = new (std::align_val_t {64}) double[1024];
	exc_c_arr   = new (std::align_val_t {64}) uint16_t[1024];
	ffor_arr    = new (std::align_val_t {64}) int64_t[1024];
	base_arr    = new (std::align_val_t {64}) int64_t[1024];
	rg_smp_arr  = new (std::align_val_t {64}) double[1024];

	for (auto& dataset : alp_bench::alp_dataset) {
		std::ifstream ifile(dataset.sample_csv_file_path, std::ios::in);
		if (dataset.suitable_for_cutting) { continue; }
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

		size_t n_values = 1024;

		size_t     global_c {0};
		alp::state stt;
		alp::AlpEncode<double>::init(dbl_arr, global_c, n_values, rg_smp_arr, stt); // 32 runs of 1 value

		// benchmark alp encode
		benchmark.Run(b_a_e(dbl_arr, exc_arr, pos_arr, exc_c_arr, encoded_arr, bw, ffor_arr, base_arr, stt, dataset));

		// decode
		generated::falp::fallback::scalar::falp(reinterpret_cast<uint64_t*>(ffor_arr),
		                                        dec_dbl_arr,
		                                        bw,
		                                        reinterpret_cast<uint64_t*>(base_arr),
		                                        stt.fac,
		                                        stt.exp);

		alp::AlpDecode<double>::patch_exceptions(dec_dbl_arr, exc_arr, pos_arr, exc_c_arr);

		// Validate
		for (size_t j = 0; j < alp::config::VECTOR_SIZE; ++j) {
			auto l = dbl_arr[j];
			auto r = dec_dbl_arr[j];
			if (l != r) {
				std::cerr << j << ", " << global_c << ", " << dataset.name << "\n";
				std::exit(-1);
			}
		}

		auto exceptions_count = exc_c_arr[0];
		if (dataset.exceptions_count != exceptions_count) {
			std::cout << dataset.name << " with exceptions_count : " << dataset.exceptions_count << " should be "
			          << exceptions_count << "\n";
		}

		if (dataset.bit_width != bw) {
			std::cout << dataset.name << " with bw " << static_cast<uint64_t>(dataset.bit_width) << " should be "
			          << static_cast<uint64_t>(bw) << "\n";
		}
	}
}

int main() {
	benchmark::Benchmark benchmark =
	    benchmark::create("alp_encode_pde")
	        .save()
	        .at(std::string(SOURCE_DIR) + "/alp_pub/results/" + benchmark::CmakeInfo::getCmakeToolchainFile())
	        .print()
	        .add_extra_info(benchmark::CmakeInfo::getCmakeInfo());
	benchmark_all(benchmark);
}
