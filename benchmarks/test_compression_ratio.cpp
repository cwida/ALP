#include "benchmark.hpp"

TEST_F(ALPBench, bench_alp_on_alp_dataset) {
	std::string result_path = alp_bench::get_paths().alp_result_dir_path + "compression_ratio/double/alp_dataset.csv";
	typed_bench_dataset<double>(alp_bench::get_alp_dataset(), result_path);
}

TEST_F(ALPBench, bench_alp_on_sp_dataset) {
	std::string result_path = alp_bench::get_paths().alp_result_dir_path + "compression_ratio/float/sp_dataset.csv";
	typed_bench_dataset<float>(alp_bench::get_sp_datasets(), result_path);
}

TEST_F(ALPBench, bench_alp_on_hurricane_isabel) {
	auto result_path =
	    alp_bench::get_paths().alp_result_dir_path + "compression_ratio/float/hurricane_isabel_dataset.csv";
	typed_bench_dataset<float>(alp_bench::get_hurricane_isabel_dataset(), result_path);
}
