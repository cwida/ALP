//#include "benchmarks/alp/queries.hpp"
//#include "common/runtime/Import.hpp"
//#include <cerrno>
//#include <cstdlib>
//#include <iostream>
//#include <iterator>
//#include <unordered_set>
//
//using namespace runtime;
//
//static void escape(void* p) { asm volatile("" : : "g"(p) : "memory"); }
//
//namespace benchmark::cycleclock {
//inline int64_t Now() {
//	// #if defined(BENCHMARK_OS_MACOSX)
//	//   // this goes at the top because we need ALL Macs, regardless of
//	//   // architecture, to return the number of "mach time units" that
//	//   // have passed since startup.  See sysinfo.cc where
//	//   // InitializeSystemInfo() sets the supposed cpu clock frequency of
//	//   // macs to the number of mach time units per second, not actual
//	//   // CPU clock frequency (which can change in the face of CPU
//	//   // frequency scaling).  Also note that when the Mac sleeps, this
//	//   // counter pauses; it does not continue counting, nor does it
//	//   // reset to zero.
//	//   return mach_absolute_time();
//	// #el
//#if defined(BENCHMARK_OS_EMSCRIPTEN)
//	// this goes above x86-specific code because old versions of Emscripten
//	// define __x86_64__, although they have nothing to do with it.
//	//        return static_cast<int64_t>(emscripten_get_now() * 1e+6);
//
//	return std::chrono::high_resolution_clock::now().time_since_epoch().count();
//#elif defined(__i386__)
//	int64_t ret;
//	__asm__ volatile("rdtsc" : "=A"(ret));
//	return ret;
//#elif defined(__x86_64__) || defined(__amd64__)
//	uint64_t low, high;
//	__asm__ volatile("rdtsc" : "=a"(low), "=d"(high));
//	return (high << 32) | low;
//#elif defined(__powerpc__) || defined(__ppc__)
//	// This returns a time-base, which is not always precisely a cycle-count.
//#if defined(__powerpc64__) || defined(__ppc64__)
//	int64_t tb;
//	asm volatile("mfspr %0, 268" : "=r"(tb));
//	return tb;
//#else
//	uint32_t tbl, tbu0, tbu1;
//	asm volatile("mftbu %0\n"
//	             "mftb %1\n"
//	             "mftbu %2"
//	             : "=r"(tbu0), "=r"(tbl), "=r"(tbu1));
//	tbl &= -static_cast<int32_t>(tbu0 == tbu1);
//	// high 32 bits in tbu1; low 32 bits in tbl  (tbu0 is no longer needed)
//	return (static_cast<uint64_t>(tbu1) << 32) | tbl;
//#endif
//#elif defined(__sparc__)
//	int64_t tick;
//	asm(".byte 0x83, 0x41, 0x00, 0x00");
//	asm("mov   %%g1, %0" : "=r"(tick));
//	return tick;
//#elif defined(__ia64__)
//	int64_t itc;
//	asm("mov %0 = ar.itc" : "=r"(itc));
//	return itc;
//#elif defined(COMPILER_MSVC) && defined(_M_IX86)
//	// Older MSVC compilers (like 7.x) don't seem to support the
//	// __rdtsc intrinsic properly, so I prefer to use _asm instead
//	// when I know it will work.  Otherwise, I'll use __rdtsc and hope
//	// the code is being compiled with a non-ancient compiler.
//	_asm rdtsc
//#elif defined(COMPILER_MSVC) && defined(_M_ARM64)
//	// See
//	// https://docs.microsoft.com/en-us/cpp/intrinsics/arm64-intrinsics?view=vs-2019
//	// and https://reviews.llvm.org/D53115
//	int64_t virtual_timer_value;
//	virtual_timer_value = _ReadStatusReg(ARM64_CNTVCT);
//	return virtual_timer_value;
//#elif defined(COMPILER_MSVC)
//	return __rdtsc();
//#elif defined(BENCHMARK_OS_NACL)
//	// Native Client validator on x86/x86-64 allows RDTSC instructions,
//	// and this case is handled above. Native Client validator on ARM
//	// rejects MRC instructions (used in the ARM-specific sequence below),
//	// so we handle it here. Portable Native Client compiles to
//	// architecture-agnostic bytecode, which doesn't provide any
//	// cycle counter access mnemonics.
//
//	// Native Client does not provide any API to access cycle counter.
//	// Use clock_gettime(CLOCK_MONOTONIC, ...) instead of gettimeofday
//	// because is provides nanosecond resolution (which is noticable at
//	// least for PNaCl modules running on x86 Mac & Linux).
//	// Initialize to always return 0 if clock_gettime fails.
//	struct timespec ts = {0, 0};
//	clock_gettime(CLOCK_MONOTONIC, &ts);
//	return static_cast<int64_t>(ts.tv_sec) * 1000000000 + ts.tv_nsec;
//#elif defined(__aarch64__)
//	// System timer of ARMv8 runs at a different frequency than the CPU's.
//	// The frequency is fixed, typically in the range 1-50MHz.  It can be
//	// read at CNTFRQ special register.  We assume the OS has set up
//	// the virtual timer properly.
//	//  int64_t virtual_timer_value;
//	//  asm volatile("mrs %0, cntvct_el0" : "=r"(virtual_timer_value));
//	//  return virtual_timer_value;
//#if defined(__APPLE__)
//	return get_counters();
//#else
//	return cycles.now();
//#endif
//
//#elif defined(__ARM_ARCH)
//	// V6 is the earliest arch that has a standard cyclecount
//	// Native Client validator doesn't allow MRC instructions.
//#if (__ARM_ARCH >= 6)
//	uint32_t pmccntr;
//	uint32_t pmuseren;
//	uint32_t pmcntenset;
//	// Read the user mode perf monitor counter access permissions.
//	asm volatile("mrc p15, 0, %0, c9, c14, 0" : "=r"(pmuseren));
//	if (pmuseren & 1) // Allows reading perfmon counters for user mode code.
//	{
//		asm volatile("mrc p15, 0, %0, c9, c12, 1" : "=r"(pmcntenset));
//		if (pmcntenset & 0x80000000ul) // Is it counting?
//		{
//			asm volatile("mrc p15, 0, %0, c9, c13, 0" : "=r"(pmccntr));
//			// The counter is set up to count every 64th cycle
//			return static_cast<int64_t>(pmccntr) * 64; // Should optimize to << 6
//		}
//	}
//#endif
//	struct timeval tv;
//	gettimeofday(&tv, nullptr);
//	return static_cast<int64_t>(tv.tv_sec) * 1000000 + tv.tv_usec;
//#elif defined(__mips__) || defined(__m68k__)
//	// mips apparently only allows rdtsc for superusers, so we fall
//	// back to gettimeofday.  It's possible clock_gettime would be better.
//	struct timeval tv;
//	gettimeofday(&tv, nullptr);
//	return static_cast<int64_t>(tv.tv_sec) * 1000000 + tv.tv_usec;
//#elif defined(__s390__) // Covers both s390 and s390x.
//	// Return the CPU clock.
//	uint64_t tsc;
//#if defined(BENCHMARK_OS_ZOS) && defined(COMPILER_IBMXL)
//	// z/OS XL compiler HLASM syntax.
//	asm(" stck %0" : "=m"(tsc) : : "cc");
//#else
//	asm("stck %0" : "=Q"(tsc) : : "cc");
//#endif
//	return tsc;
//#elif defined(__riscv) // RISC-V
//	// Use RDCYCLE (and RDCYCLEH on riscv32)
//#if __riscv_xlen == 32
//	uint32_t cycles_lo, cycles_hi0, cycles_hi1;
//	// This asm also includes the PowerPC overflow handling strategy, as above.
//	// Implemented in assembly because Clang insisted on branching.
//	asm volatile("rdcycleh %0\n"
//	             "rdcycle %1\n"
//	             "rdcycleh %2\n"
//	             "sub %0, %0, %2\n"
//	             "seqz %0, %0\n"
//	             "sub %0, zero, %0\n"
//	             "and %1, %1, %0\n"
//	             : "=r"(cycles_hi0), "=r"(cycles_lo), "=r"(cycles_hi1));
//	return (static_cast<uint64_t>(cycles_hi1) << 32) | cycles_lo;
//#else
//	uint64_t cycles;
//	asm volatile("rdcycle %0" : "=r"(cycles));
//	return cycles;
//#endif
//#elif defined(__e2k__) || defined(__elbrus__)
//	struct timeval tv;
//	gettimeofday(&tv, nullptr);
//	return static_cast<int64_t>(tv.tv_sec) * 1000000 + tv.tv_usec;
//#else
//	// The soft failover to a generic implementation is automatic only for ARM.
//	// For other platforms the developer is expected to make an attempt to create
//	// a fast implementation and use generic version if nothing better is available.
//#error You need to define CycleTimer for your OS and CPU
//	//    return
//	// std::chrono::high_resolution_clock::now().time_since_epoch().count();
//
//#endif
//}
//
//} // namespace benchmark::cycleclock
//
//size_t nrTuples(Database& db, std::vector<std::string> tables) {
//	size_t sum = 0;
//	for (auto& table : tables)
//		sum += db[table].tup_c;
//	return sum;
//}
//
///// Clears Linux page cache.
///// This function only works on Linux.
//void clearOsCaches() {
//	if (system("sync; echo 3 > /proc/sys/vm/drop_caches")) {
//		throw std::runtime_error("Could not flush system caches: " + std::string(std::strerror(errno)));
//	}
//}
//
//double alp_profile(std::function<void()> fn, uint64_t rep_c, uint64_t warmup_rep_c) {
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
//}
//
//int main() {
//	runtime::cur_q_mtd.repetition         = cfg::rep_c;
//	runtime::cur_q_mtd.warm_up_repetition = cfg::warmup_rep_c;
//	std::cout << "dataset,repetition,warmup_repetition,scheme,thread_n,query,time(s),"
//	             "result(tpc),corrected_result(tpc)"
//	             ",validity,compression_cycles,cycles"
//	          << std::endl;
//	// ALL COLUMNS
//	for (auto& col : alp_bench::get_alp_end_to_end()) { // TODO
//		runtime::cur_q_mtd.col_name = col.name;
//
//		/* Extension. */
//		if (!experiment::is_expanded(col)) { experiment::expand_binary_x_times(col, cfg::ext_c); }
//
//		// ALL ENCODINGS
//		for (auto& scheme : encoding::scheme_vec) {
//			//			if (scheme.enc_t != encoding::scheme_t::CHIMP128) { continue; }
//
//			runtime::cur_q_mtd.scheme = scheme;
//
//			// ALL THREADS
//			for (auto j : cfg::threads_pool) {
//				//				if (j != 32) { continue; }
//				runtime::cur_q_mtd.thr_c = j;
//
//				// ALL Queries
//				for (auto& query : cfg::query_vec) {
//					//					if (query.q_t != cfg::query_t::SCAN) { continue; }
//
//					runtime::cur_q_mtd.query                    = query;
//					size_t                          thread_c    = j;
//					size_t                          vector_c    = cfg::vec_tup_c;
//					bool                            clear_cache = false;
//					std::unordered_set<std::string> q           = {};
//					std::string                     name;
//
//					/* Init execution. */
//					Database ALP_DB;
//					runtime::cur_q_mtd.compression_cycles = import_alp(col, ALP_DB, scheme);
//					//					if (tmp_time != 0) { runtime::cur_q_mtd.compression_cycles = import_alp(col,
//					// ALP_DB, scheme); }
//
//					//					 Verify
//					//					if (clear_cache) { clearOsCaches(); }
//					//					auto res_rel = alp_q(ALP_DB, thread_c, vector_c);
//					//					escape(&res_rel);
//					//					auto result                 =
//					//*reinterpret_cast<double*>(res_rel.attributes.at("aggr").data());
//					//runtime::cur_q_mtd.validity = result;
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
//					std::cout << runtime::cur_q_mtd.get_csv_row() << std::endl;
//				}
//			}
//
//			/* removal. */
//			//			experiment::clean_compressed_data(col, scheme);
//		}
//		//		experiment::remove_binary_file(col);
//	}
//	return 0;
//}
