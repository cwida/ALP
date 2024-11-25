#ifndef ALP_SP_HPP
#define ALP_SP_HPP

#include "column.hpp"

namespace alp_bench {
inline auto get_sp_datasets() {
	static std::array<ALPColumnDescriptor, 4> SP_DATASETS = {{
	    {1, "Dino-Vitb16", "", get_paths().alp_dataset_binary_dir_path + "sp_dino_vitb16.bin", 0, 0, 0, 0, true},
	    {2, "GPT2", "", get_paths().alp_dataset_binary_dir_path + "sp_gpt2.bin", 0, 0, 0, 0, true},
	    {3,
	     "Grammarly-lg",
	     "",
	     get_paths().alp_dataset_binary_dir_path + "sp_grammarly_coedit_lg.bin",
	     0,
	     0,
	     0,
	     0,
	     true},
	    {4, "W2V Tweets", "", get_paths().alp_dataset_binary_dir_path + "sp_w2v.bin", 0, 0, 0, 0, true},

	}};

	return SP_DATASETS;
}


} // namespace alp_bench
#endif // ALP_SP_HPP
