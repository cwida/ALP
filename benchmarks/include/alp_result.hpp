/*
-- DATE : 17/04/2024
-- FILE_PATH : benchmarks/bench_full_dataset/result.hpp
-- PROJECT_NAME : ALP
*/

#ifndef BENCHMARKS_RESULT_HPP
#define BENCHMARKS_RESULT_HPP

#include <iomanip>
#include <sstream>
#include <string>
#include <unordered_map>

namespace alp_bench {

struct VectorMetadata {
	uint8_t                                    bit_width {0};
	uint16_t                                   exceptions_count {0};
	uint64_t                                   unq_c {0};
	uint16_t                                   freq {0};
	double                                     size {0};
	uint64_t                                   right_bit_width {0};
	uint64_t                                   left_bit_width {0};
	std::vector<std::pair<uint16_t, uint64_t>> repetition_vec;
};

inline std::string to_str(double val) {
	std::stringstream stream;
	stream << std::fixed << std::setprecision(2) << val;
	std::string str = stream.str();
	return str;
}

inline std::unordered_map<std::string, std::string> results = {
    //
    {"Air-Pressure", "16.43"}, {"Arade/4", "24.94"},     {"Basel-Temp", "30.72"}, {"Basel-Wind", "29.81"},
    {"Bird-Mig", "20.14"},     {"Btc-Price", "26.37"},   {"Blockchain", "36.49"}, {"City-Temp", "10.74"},
    {"CMS/1", "35.65"},        {"CMS/9", "11.67"},       {"CMS/25", "41.11"},     {"Dew-Temp", "13.40"},
    {"Bio-Temp", "10.75"},     {"Food-prices", "23.65"}, {"Gov/10", "30.99"},     {"Gov/26", "0.41"},
    {"Gov/30", "7.48"},        {"Gov/31", "3.05"},       {"Gov/40", "0.83"},      {"Medicare/1", "39.35"},
    {"Medicare/9", "12.26"},   {"PM10-dust", "8.56"},    {"NYC/29", "40.38"},     {"SD-bench", "16.21"},
    {"Stocks-DE", "11.01"},    {"Stocks-UK", "12.59"},   {"Stocks-USA", "7.90"},  {"Wind-dir", "15.89"},
    //
};

} // namespace alp_bench

#endif // BENCHMARKS_RESULT_HPP
