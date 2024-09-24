#pragma once

#include "encoding/encoding.hpp"
#include "string"
#include <stdint.h>
#include <vector>

namespace dataset {
struct paths {
	static std::string get_cache_dir_path() { return CMAKE_SOURCE_DIR "/data/cached/"; }
	static std::string get_1_rg_dir_path() { return CMAKE_SOURCE_DIR "/data/1_rg_data_sample/"; }
	static std::string get_128_1024_rg_dir_path() { return CMAKE_SOURCE_DIR "/data/128_1024_rg_data_sample/"; }
};
} // namespace dataset

namespace cfg {
inline std::string options[] = {"0"};

inline int32_t answers[] = {1 * 256 * 1024 * 1024 * 2, //
                            5 * 1024 * 2,              //
                            5 * 1024 * 4,              //
                            1 * 256 * 1024 * 1024 * 4,
                            3 * 256 * 1024 * 1024 * 2,
                            -1609039872};

inline size_t threads_pool[] = {32, 16, 8, 1};

//
enum query_t : uint8_t {
	DEBUG       = 0, // ONLY FOR DEBUG
	SCAN        = 1,
	SUM         = 2,
	SUM_SIMD    = 3,
	COMPRESSION = 4,
};

inline std::string to_string(query_t q_t) {
	switch (q_t) {
	case DEBUG:
		return "DEBUG";
	case SCAN:
		return "SCAN";
	case SUM:
		return "SUM";
	case SUM_SIMD:
		return "SUM_SIMD";
	case COMPRESSION:
		return "COMPRESSION";
	}
}
struct alp_query {
	query_t q_t;
};

inline alp_query scan        = {SCAN};
inline alp_query sum         = {SUM};
inline alp_query sum_simd    = {SUM_SIMD};
inline alp_query compression = {COMPRESSION};

inline std::vector<alp_query> query_vec = {scan, sum, compression};

enum input_t : uint8_t {
	BINARY = 0,
	CSV    = 1,
};

inline const std::string schema       = "0";
inline const size_t      tab_n        = 0;
inline const int32_t     answer       = answers[tab_n];
inline const std::string name         = options[tab_n];
inline uint8_t           bw           = 2;
inline const size_t      rep_c        = 4;
inline const size_t      warmup_rep_c = 0;
inline const size_t      vec_tup_c    = 1024;
inline const size_t      morsel_c     = 1024 * 128;
inline const bool        is_bitpacked = false;
inline const std::string tbl_name     = "ALP";
inline const std::string cached_dir   = dataset::paths::get_cache_dir_path();
inline const std::string data_dir     = dataset::paths::get_1_rg_dir_path();
inline const std::string data_ext_dir = dataset::paths::get_128_1024_rg_dir_path();
inline const size_t      m_sz         = 48;
inline const input_t     input_t      = BINARY;
inline const size_t      vec_c        = 1024 * 1024;
inline const size_t      t_c          = vec_c * 1024;
inline const size_t      ext_c        = t_c / morsel_c;

} // namespace cfg

namespace runtime {

struct query_mtd {
	std::string      col_name;
	size_t           repetition;
	size_t           warm_up_repetition;
	encoding::scheme scheme;
	cfg::alp_query   query;
	std::size_t      thr_c;
	double           time {0.0};
	double           validity {-1};
	int64_t          compression_cycles {0};
	int64_t          cycles {0};

	//	"dataset,repetition,warmup_repetition,scheme,thread_n,query,time(s),"
	//	"result(tpc),corrected_result(tpc)"
	//	",validity,compression_cycles,cycles"
	std::string get_csv_row() {
		std::string result_str;

		result_str += col_name + ",";
		result_str += std::to_string(repetition) + ",";
		result_str += std::to_string(warm_up_repetition) + ",";
		result_str += scheme.name + ",";
		result_str += std::to_string(thr_c) + ",";
		result_str += cfg::to_string(query.q_t) + ",";
		result_str += std::to_string(time / repetition) + ",";
		result_str += std::to_string(get_result_tpc()) + ",";
		result_str += std::to_string(get_corrected_result_tpc()) + ",";
		result_str += std::to_string(validity) + ",";
		result_str += std::to_string(compression_cycles) + ",";
		result_str += std::to_string(cycles) + ",";

		result_str += "\n";
		return result_str;
	}

	double get_result_tpc() { return 0; }
	double get_corrected_result_tpc() { return 0; }
};

inline query_mtd cur_q_mtd;

} // namespace runtime