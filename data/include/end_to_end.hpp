#ifndef END_TO_END_HPP
#define END_TO_END_HPP

#include "column.hpp"

namespace alp_bench {

inline auto get_alp_end_to_end() {
	static std::array<Column, 5> ALP_END_TO_END_DATASET = {{

	    {0,
	     "Food-prices",
	     get_paths().alp_dataset_csv_path + "food_prices.csv",
	     get_paths().alp_dataset_binary_dir_path + "food_prices.bin",
	     16,
	     12,
	     46,
	     20},

	    {1,
	     "City-Temp",
	     get_paths().alp_dataset_csv_path + "city_temperature_f.csv",
	     get_paths().alp_dataset_binary_dir_path + "city_temperature_f.bin",
	     14,
	     13,
	     0,
	     11},

	    {2,
	     "Btc-Price",
	     get_paths().alp_dataset_csv_path + "bitcoin_f.csv",
	     get_paths().alp_dataset_binary_dir_path + "bitcoin_f.bin",
	     14,
	     10,
	     10,
	     25},

	    {3,
	     "Gov/26",
	     get_paths().alp_dataset_csv_path + "gov26.csv",
	     get_paths().alp_dataset_binary_dir_path + "gov26.bin",
	     18,
	     18,
	     0,
	     0},

	    {4,
	     "NYC/29",
	     get_paths().alp_dataset_csv_path + "nyc29.csv",
	     get_paths().alp_dataset_binary_dir_path + "nyc29.bin",
	     14,
	     1,
	     5,
	     42},

	}};
	return ALP_END_TO_END_DATASET;
};

} // namespace alp_bench
#endif
