#include "benchmarks/alp/queries.hpp"
#include "common/runtime/Import.hpp"
#include "data.hpp"
#include "profile.hpp"
#include "tbb/tbb.h"
#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <unordered_set>

using namespace runtime;

static void escape(void* p) { asm volatile("" : : "g"(p) : "memory"); }

size_t nrTuples(Database& db, std::vector<std::string> tables) {
	size_t sum = 0;
	for (auto& table : tables)
		sum += db[table].tup_c;
	return sum;
}

/// Clears Linux page cache.
/// This function only works on Linux.
void clearOsCaches() {
	if (system("sync; echo 3 > /proc/sys/vm/drop_caches")) {
		throw std::runtime_error("Could not flush system caches: " + std::string(std::strerror(errno)));
	}
}

// double alp_profile(std::function<void()> fn, uint64_t rep_c, uint64_t warmup_rep_c) {
//	using namespace std;
//	// warmup round
//	for (size_t i {0}; i < warmup_rep_c; ++i) {
//		fn();
//	}
//
//	auto start = benchmark::cycleclock::Now();
//	for (size_t i {0}; i < rep_c; ++i) {
//		fn();
//	}
//	auto end = benchmark::cycleclock::Now();
//
//	return end - start;
// }

int main() {
	runtime::cur_q_mtd.repetition         = cfg::rep_c;
	runtime::cur_q_mtd.warm_up_repetition = cfg::warmup_rep_c;
	std::cout << "dataset,repetition,warmup_repetition,scheme,thread_n,query,time(s),"
	             "result(tpc),corrected_result(tpc)"
	             ",validity,compression_cycles,cycles"
	          << std::endl;
	// ALL COLUMNS
	for (auto& col : alp_bench::get_alp_end_to_end()) { // TODO
		runtime::cur_q_mtd.col_name = col.name;

		/* Extension. */
		if (!experiment::is_expanded(col)) { experiment::expand_binary_x_times(col, cfg::ext_c); }

		// ALL ENCODINGS
		for (auto& scheme : encoding::scheme_vec) {
			//			if (scheme.enc_t != encoding::scheme_t::CHIMP128) { continue; }

			runtime::cur_q_mtd.scheme = scheme;

			// ALL THREADS
			for (auto j : cfg::threads_pool) {
				if (j != 32) { continue; }
				runtime::cur_q_mtd.thr_c = j;

				// ALL Queries
				for (auto& query : cfg::query_vec) {
					if (query.q_t != cfg::query_t::COMPRESSION) { continue; }

					runtime::cur_q_mtd.query                    = query;
					size_t                          thread_c    = j;
					size_t                          vector_c    = cfg::vec_tup_c;
					bool                            clear_cache = false;
					std::unordered_set<std::string> q           = {};
					std::string                     name;

					/* Init execution. */
					Database ALP_DB;
					double   tmp_time = import_alp(col, ALP_DB, scheme);
					if (tmp_time != 0) { runtime::cur_q_mtd.compression_cycles = import_alp(col, ALP_DB, scheme); }

					//					// Verify
					//					if (clear_cache) { clearOsCaches(); }
					//					auto res_rel = alp_q(ALP_DB, thread_c, vector_c);
					//					escape(&res_rel);
					//
					//					auto result                 =
					//*reinterpret_cast<double*>(res_rel.attributes.at("aggr").data());
					// runtime::cur_q_mtd.validity = result;
					//
					//					// Benchmark
					//					runtime::cur_q_mtd.cycles = alp_profile(
					//					    [&]() {
					//						    if (clear_cache) { clearOsCaches(); }
					//						    auto result = alp_q(ALP_DB, thread_c, vector_c);
					//						    escape(&result);
					//					    },
					//					    cfg::rep_c,
					//					    cfg::warmup_rep_c);
					//
					std::cout << runtime::cur_q_mtd.get_csv_row();
				}
			}

			/* removal. */
			//			experiment::clean_compressed_data(col, scheme);
		}
		//		experiment::remove_binary_file(col);
	}
	return 0;
}
