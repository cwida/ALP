#ifndef END_TO_END_HPP
#define END_TO_END_HPP

#include "column.hpp"

namespace alp_bench {

inline auto get_alp_end_to_end() {
	static std::array<Column, 5> ALP_END_TO_END_DATASET = {{
	    {0, "food_prices_tw", "", "", 16, 12, 46, 20},
	    {1, "city_temperature_f", "", "", 14, 13, 0, 11},
	    {2, "bitcoin_transactions_f_tw", "", "", 14, 10, 10, 25},
	    {3, "gov26_tw", "", "", 18, 18, 0, 0},
	    {4, "nyc29_tw", "", "", 14, 1, 5, 42},
	}};

	return ALP_END_TO_END_DATASET;
};

} // namespace alp_bench
#endif
