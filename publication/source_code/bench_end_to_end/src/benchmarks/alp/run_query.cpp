#include "benchmarks/alp/queries.hpp"
#include "column.hpp"
#include "common/runtime/Import.hpp"
#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <unordered_set>

using namespace runtime;

static void escape(void* p) { asm volatile("" : : "g"(p) : "memory"); }

namespace benchmark::cycleclock {
inline int64_t Now() {
	// #if defined(BENCHMARK_OS_MACOSX)
	//   // this goes at the top because we need ALL Macs, regardless of
	//   // architecture, to return the number of "mach time units" that
	//   // have passed since startup.  See sysinfo.cc where
	//   // InitializeSystemInfo() sets the supposed cpu clock frequency of
	//   // macs to the number of mach time units per second, not actual
	//   // CPU clock frequency (which can change in the face of CPU
	//   // frequency scaling).  Also note that when the Mac sleeps, this
	//   // counter pauses; it does not continue counting, nor does it
	//   // reset to zero.
	//   return mach_absolute_time();
	// #el
#if defined(BENCHMARK_OS_EMSCRIPTEN)
	// this goes above x86-specific code because old versions of Emscripten
	// define __x86_64__, although they have nothing to do with it.
	//        return static_cast<int64_t>(emscripten_get_now() * 1e+6);

	return std::chrono::high_resolution_clock::now().time_since_epoch().count();
#elif defined(__i386__)
	int64_t ret;
	__asm__ volatile("rdtsc" : "=A"(ret));
	return ret;
#elif defined(__x86_64__) || defined(__amd64__)
	uint64_t low, high;
	__asm__ volatile("rdtsc" : "=a"(low), "=d"(high));
	return (high << 32) | low;
#elif defined(__powerpc__) || defined(__ppc__)
	// This returns a time-base, which is not always precisely a cycle-count.
#if defined(__powerpc64__) || defined(__ppc64__)
	int64_t tb;
	asm volatile("mfspr %0, 268" : "=r"(tb));
	return tb;
#else
	uint32_t tbl, tbu0, tbu1;
	asm volatile("mftbu %0\n"
	             "mftb %1\n"
	             "mftbu %2"
	             : "=r"(tbu0), "=r"(tbl), "=r"(tbu1));
	tbl &= -static_cast<int32_t>(tbu0 == tbu1);
	// high 32 bits in tbu1; low 32 bits in tbl  (tbu0 is no longer needed)
	return (static_cast<uint64_t>(tbu1) << 32) | tbl;
#endif
#elif defined(__sparc__)
	int64_t tick;
	asm(".byte 0x83, 0x41, 0x00, 0x00");
	asm("mov   %%g1, %0" : "=r"(tick));
	return tick;
#elif defined(__ia64__)
	int64_t itc;
	asm("mov %0 = ar.itc" : "=r"(itc));
	return itc;
#elif defined(COMPILER_MSVC) && defined(_M_IX86)
	// Older MSVC compilers (like 7.x) don't seem to support the
	// __rdtsc intrinsic properly, so I prefer to use _asm instead
	// when I know it will work.  Otherwise, I'll use __rdtsc and hope
	// the code is being compiled with a non-ancient compiler.
	_asm rdtsc
#elif defined(COMPILER_MSVC) && defined(_M_ARM64)
	// See
	// https://docs.microsoft.com/en-us/cpp/intrinsics/arm64-intrinsics?view=vs-2019
	// and https://reviews.llvm.org/D53115
	int64_t virtual_timer_value;
	virtual_timer_value = _ReadStatusReg(ARM64_CNTVCT);
	return virtual_timer_value;
#elif defined(COMPILER_MSVC)
	return __rdtsc();
#elif defined(BENCHMARK_OS_NACL)
	// Native Client validator on x86/x86-64 allows RDTSC instructions,
	// and this case is handled above. Native Client validator on ARM
	// rejects MRC instructions (used in the ARM-specific sequence below),
	// so we handle it here. Portable Native Client compiles to
	// architecture-agnostic bytecode, which doesn't provide any
	// cycle counter access mnemonics.

	// Native Client does not provide any API to access cycle counter.
	// Use clock_gettime(CLOCK_MONOTONIC, ...) instead of gettimeofday
	// because is provides nanosecond resolution (which is noticable at
	// least for PNaCl modules running on x86 Mac & Linux).
	// Initialize to always return 0 if clock_gettime fails.
	struct timespec ts = {0, 0};
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return static_cast<int64_t>(ts.tv_sec) * 1000000000 + ts.tv_nsec;
#elif defined(__aarch64__)
	// System timer of ARMv8 runs at a different frequency than the CPU's.
	// The frequency is fixed, typically in the range 1-50MHz.  It can be
	// read at CNTFRQ special register.  We assume the OS has set up
	// the virtual timer properly.
	//  int64_t virtual_timer_value;
	//  asm volatile("mrs %0, cntvct_el0" : "=r"(virtual_timer_value));
	//  return virtual_timer_value;
#if defined(__APPLE__)
	return get_counters();
#else
	return cycles.now();
#endif

#elif defined(__ARM_ARCH)
	// V6 is the earliest arch that has a standard cyclecount
	// Native Client validator doesn't allow MRC instructions.
#if (__ARM_ARCH >= 6)
	uint32_t pmccntr;
	uint32_t pmuseren;
	uint32_t pmcntenset;
	// Read the user mode perf monitor counter access permissions.
	asm volatile("mrc p15, 0, %0, c9, c14, 0" : "=r"(pmuseren));
	if (pmuseren & 1) // Allows reading perfmon counters for user mode code.
	{
		asm volatile("mrc p15, 0, %0, c9, c12, 1" : "=r"(pmcntenset));
		if (pmcntenset & 0x80000000ul) // Is it counting?
		{
			asm volatile("mrc p15, 0, %0, c9, c13, 0" : "=r"(pmccntr));
			// The counter is set up to count every 64th cycle
			return static_cast<int64_t>(pmccntr) * 64; // Should optimize to << 6
		}
	}
#endif
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	return static_cast<int64_t>(tv.tv_sec) * 1000000 + tv.tv_usec;
#elif defined(__mips__) || defined(__m68k__)
	// mips apparently only allows rdtsc for superusers, so we fall
	// back to gettimeofday.  It's possible clock_gettime would be better.
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	return static_cast<int64_t>(tv.tv_sec) * 1000000 + tv.tv_usec;
#elif defined(__s390__) // Covers both s390 and s390x.
	// Return the CPU clock.
	uint64_t tsc;
#if defined(BENCHMARK_OS_ZOS) && defined(COMPILER_IBMXL)
	// z/OS XL compiler HLASM syntax.
	asm(" stck %0" : "=m"(tsc) : : "cc");
#else
	asm("stck %0" : "=Q"(tsc) : : "cc");
#endif
	return tsc;
#elif defined(__riscv) // RISC-V
	// Use RDCYCLE (and RDCYCLEH on riscv32)
#if __riscv_xlen == 32
	uint32_t cycles_lo, cycles_hi0, cycles_hi1;
	// This asm also includes the PowerPC overflow handling strategy, as above.
	// Implemented in assembly because Clang insisted on branching.
	asm volatile("rdcycleh %0\n"
	             "rdcycle %1\n"
	             "rdcycleh %2\n"
	             "sub %0, %0, %2\n"
	             "seqz %0, %0\n"
	             "sub %0, zero, %0\n"
	             "and %1, %1, %0\n"
	             : "=r"(cycles_hi0), "=r"(cycles_lo), "=r"(cycles_hi1));
	return (static_cast<uint64_t>(cycles_hi1) << 32) | cycles_lo;
#else
	uint64_t cycles;
	asm volatile("rdcycle %0" : "=r"(cycles));
	return cycles;
#endif
#elif defined(__e2k__) || defined(__elbrus__)
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	return static_cast<int64_t>(tv.tv_sec) * 1000000 + tv.tv_usec;
#else
	// The soft failover to a generic implementation is automatic only for ARM.
	// For other platforms the developer is expected to make an attempt to create
	// a fast implementation and use generic version if nothing better is available.
#error You need to define CycleTimer for your OS and CPU
	//    return
	// std::chrono::high_resolution_clock::now().time_since_epoch().count();

#endif
}

} // namespace benchmark::cycleclock

/// Clears Linux page cache.
/// This function only works on Linux.
void clearOsCaches() {
	if (system("sync; echo 3 > /proc/sys/vm/drop_caches")) {
		throw std::runtime_error("Could not flush system caches: " + std::string(std::strerror(errno)));
	}
}

double alp_profile(std::function<void()> fn, uint64_t rep_c, uint64_t warmup_rep_c) {
	using namespace std;
	// warmup round
	for (size_t i {0}; i < warmup_rep_c; ++i) {
		fn();
	}

	auto start = benchmark::cycleclock::Now();
	for (size_t i {0}; i < rep_c; ++i) {
		fn();
	}
	auto end = benchmark::cycleclock::Now();

	return end - start;
}

alp_bench::ALPColumnDescriptor& get_dataset(const std::string& col_name) {
	static std::unordered_map<std::string, alp_bench::ALPColumnDescriptor> ALP_END_TO_END_DATASET = {
	    {"food_prices", {0, "food_prices_tw", "", "", 16, 12, 46, 20}},
	    {"city_temperature_f", {1, "city_temperature_f_tw", "", "", 14, 13, 0, 11}},
	    {"bitcoin_transactions_f", {2, "bitcoin_transactions_f_tw", "", "", 14, 10, 10, 25}},
	    {"gov26", {3, "gov26_tw", "", "", 18, 18, 0, 0}},
	    {"nyc29", {4, "nyc29_tw", "", "", 14, 1, 5, 42}}};

	auto it = ALP_END_TO_END_DATASET.find(col_name);
	if (it != ALP_END_TO_END_DATASET.end()) { return it->second; }

	throw std::runtime_error("Invalid column name: " + col_name);
}

int main(int argc, char* argv[]) {
	if (argc < 5) {
		std::cerr << "Usage: " << argv[0] << " <THREAD> <QUERY> <SCHEME> <dataset>" << std::endl;
		return 1;
	}

	// Parse command-line arguments
	int              thread         = std::stoi(argv[1]);
	std::string      query_string   = argv[2];
	std::string      scheme_string  = argv[3];
	std::string      dataset_string = argv[4];
	cfg::alp_query   query {cfg::QueryT::INVALID};
	encoding::scheme encoding_scheme;

	// parse query string
	if (query_string == "scan") {
		query = {cfg::QueryT::SCAN};
	} else if (query_string == "sum") {
		query = {cfg::QueryT::SUM};
	} else {
		throw std::runtime_error("Wrong Query Type.");
	}

	// parse encoding scheme string
	if (scheme_string == "alp") {
		encoding_scheme = {encoding::alp_scheme};
	} else if (scheme_string == "pde") {
		encoding_scheme = {encoding::pde_scheme};
	} else if (scheme_string == "patas") {
		encoding_scheme = {encoding::patas_scheme};
	} else if (scheme_string == "gorilla") {
		encoding_scheme = {encoding::gorilla_scheme};
	} else if (scheme_string == "alp_rd") {
		encoding_scheme = {encoding::alp_rd_scheme};
	} else if (scheme_string == "ztsd") {
		encoding_scheme = {encoding::zstd_scheme};
	} else if (scheme_string == "bitpacked") {
		encoding_scheme = {encoding::bitpacked_scheme};
	} else if (scheme_string == "cst_bp") {
		encoding_scheme = {encoding::cst_bp_scheme};
	} else if (scheme_string == "chimp") {
		encoding_scheme = {encoding::chimp_scheme};
	} else if (scheme_string == "chimp128") {
		encoding_scheme = {encoding::chimp128_scheme};
	} else if (scheme_string == "uncompressed") {
		encoding_scheme = {encoding::uncompressed_scheme};
	} else {
		throw std::runtime_error("Wrong Scheme Type.");
	}

	alp_bench::ALPColumnDescriptor& col = get_dataset(dataset_string);

	runtime::cur_q_mtd.repetition         = cfg::rep_c;
	runtime::cur_q_mtd.warm_up_repetition = cfg::warmup_rep_c;

	// ALL COLUMNS
	runtime::cur_q_mtd.col_name = col.name;

	/* Extension. */
	if (!experiment::is_expanded(col)) { experiment::expand_binary_x_times(col, cfg::ext_c); }

	runtime::cur_q_mtd.scheme = encoding_scheme;
	runtime::cur_q_mtd.thr_c  = thread;
	runtime::cur_q_mtd.query  = query;

	size_t                          thread_c    = thread;
	size_t                          vector_c    = cfg::vec_tup_c;
	bool                            clear_cache = false;
	std::unordered_set<std::string> q           = {};
	std::string                     name;

	/* Init execution. */
	Database alp_db;
	runtime::cur_q_mtd.compression_cycles = import_alp(col, alp_db, encoding_scheme);

	// Benchmark
	runtime::cur_q_mtd.cycles = alp_profile(
	    [&]() {
		    if (clear_cache) { clearOsCaches(); }
		    auto result = alp_q(alp_db, thread_c, vector_c);
		    escape(&result);
	    },
	    cfg::rep_c,
	    cfg::warmup_rep_c);

	std::cout << runtime::cur_q_mtd.get_csv_row() << std::endl;
	/* removal. */
	experiment::clean_compressed_data(col, encoding_scheme);
	experiment::remove_binary_file(col);
}
