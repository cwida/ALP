#include "arm64v8_sve_intrinsic_1024_uf1_falp_bench.hpp"
#include "alp/alp.hpp"
#include "datasets.hpp"
#include "alp/ffor.hpp"
#include "alp/unffor.hpp"
static __attribute__((noinline)) benchmark::BenchmarkReporter::Run bench_alp_fused_decode(alp_bench::Dataset& dataset, int64_t* ffor_arr, uint8_t bw, int64_t*base_arr,uint8_t factor,uint8_t exponent,double* dec_dbl_arr,double* exc_arr,uint16_t* pos_arr,uint16_t* exc_c_arr)
{
	int benchmark_number = dataset.id;

            #ifdef NDEBUG
            uint64_t iterations = 3000000;
            #else
            uint64_t iterations = 1;
            #endif

            std::string benchmark_name = dataset.name + "_fused";

            uint64_t cycles = benchmark::cycleclock::Now();
            for (uint64_t i = 0; i < iterations; ++i) {
                generated::falp::arm64v8::sve::falp(reinterpret_cast<uint64_t*>(ffor_arr),
                                                        dec_dbl_arr,
                                                        bw,
                                                        reinterpret_cast<uint64_t*>(base_arr),
                                                        factor,
                                                        exponent);
            alp::AlpDecode<double>::patch_exceptions(dec_dbl_arr, exc_arr, pos_arr, exc_c_arr);
            }

            cycles = benchmark::cycleclock::Now() - cycles;

        return benchmark::BenchmarkReporter::Run(
            benchmark_number, benchmark_name, iterations, double(cycles) / (double(iterations) * 1024));
}
static __attribute__((noinline)) benchmark::BenchmarkReporter::Run bench_alp_decode(alp_bench::Dataset& dataset, int64_t* ffor_arr, int64_t* unffor_arr, uint8_t bw, int64_t* base_arr, uint8_t factor, uint8_t exponent, double* dec_dbl_arr, double* exc_arr, uint16_t* pos_arr, uint16_t* exc_c_arr)
{
	
            	int benchmark_number = dataset.id;

                #ifdef NDEBUG
                    uint64_t iterations = 3000000;
                #else
                    uint64_t iterations = 1;
                #endif
                
                    std::string benchmark_name = dataset.name + "";
                
                    uint64_t cycles = benchmark::cycleclock::Now();
                    for (uint64_t i = 0; i < iterations; ++i) {
                        alp::generated::unffor::fallback::scalar::unffor(ffor_arr, unffor_arr, bw, base_arr);
                        alp::AlpDecode<double>(reinterpret_cast<const uint64_t*>(unffor_arr), factor, exponent, dec_dbl_arr);
                        alp::AlpDecode<double>::patch_exceptions(dec_dbl_arr, exc_arr, pos_arr, exc_c_arr);
                    }
                
                    cycles = benchmark::cycleclock::Now() - cycles;
                
                    return benchmark::BenchmarkReporter::Run(
                        benchmark_number, benchmark_name, iterations, double(cycles) / (double(iterations) * 1024));
                
}
void benchmark_all(benchmark::Benchmark& benchmark)
{
	
            double*   dbl_arr;
            double*   exc_arr;
            uint16_t* pos_arr;
            uint16_t* exc_c_arr;
            int64_t*  ffor_arr;
            int64_t*  unffor_arr;
        
            int64_t* base_arr;
            int64_t* dig_arr;
            double*  dec_dbl_arr;
        
            uint8_t bw;
            uint8_t factor;
            uint8_t exponent;
        
            dbl_arr     = new (std::align_val_t {64}) double[1024];
            exc_arr     = new (std::align_val_t {64}) double[1024];
            pos_arr     = new (std::align_val_t {64}) uint16_t[1024];
            dig_arr     = new (std::align_val_t {64}) int64_t[1024];
            dec_dbl_arr = new (std::align_val_t {64}) double[1024];
            exc_c_arr   = new (std::align_val_t {64}) uint16_t[1024];
            ffor_arr    = new (std::align_val_t {64}) int64_t[1024];
            unffor_arr  = new (std::align_val_t {64}) int64_t[1024];
            base_arr    = new (std::align_val_t {64}) int64_t[1024];
        
            for (auto& dataset : alp_bench::datasets) {
                std::ifstream ifile(dataset.sample_csv_file_path, std::ios::in);
        
                // check to see that the file was opened correctly:
                if (!ifile.is_open()) {
                    exit(1); // exit or do additional error checking
                }
        
                double num = 0.0;
                // keep storing values from the text file so long as data exists:
                size_t c {0};
                while (ifile >> num) {
                    dbl_arr[c] = num;
                    c += 1;
                }
        
                factor   = dataset.factor;
                exponent = dataset.exponent;
        
                alp::AlpEncode<double>::encode(dbl_arr, exc_arr, pos_arr, exc_c_arr, dig_arr, stt);
                alp::AlpEncode<double>::analyze_ffor(dig_arr, bw, base_arr);
                alp::generated::ffor::fallback::scalar::ffor(dig_arr, ffor_arr, bw, base_arr);
        
                benchmark.Run(bench_alp_fused_decode(
                    dataset, unffor_arr, bw, base_arr, factor, exponent, dec_dbl_arr, exc_arr, pos_arr, exc_c_arr));
        
                benchmark.Run(bench_alp_decode(
                    dataset, ffor_arr, unffor_arr, bw, base_arr, factor, exponent, dec_dbl_arr, exc_arr, pos_arr, exc_c_arr));
        
                ifile.close();}
}
int main()
{
		benchmark::Benchmark benchmark =
                         benchmark::create("arm64v8_sve_intrinsic_1024_uf1_falp")
                        .save()
                        .at(std::string(SOURCE_DIR) + "/alp_pub/results/" + benchmark::CmakeInfo::getCmakeToolchainFile())
                        .print()
                        .add_extra_info(benchmark::CmakeInfo::getCmakeInfo());
                    benchmark_all(benchmark);
}
