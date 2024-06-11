#ifndef ALP_FLOAT_COLUMNS_HPP
#define ALP_FLOAT_COLUMNS_HPP

#include "column.hpp"

namespace alp_bench {
inline std::array<Column, 4> sp_datasets = {{

    {1, "Dino-Vitb16", "", PATHS.ALP_DATASET_BINARY_DIR_PATH + "sp_dino_vitb16.bin", 0, 0, 0, 0, true},

    {2, "GPT2", "", PATHS.ALP_DATASET_BINARY_DIR_PATH + "sp_gpt2.bin", 0, 0, 0, 0, true},

    {3, "Grammarly-lg", "", PATHS.ALP_DATASET_BINARY_DIR_PATH + "sp_grammarly_coedit_lg.bin", 0, 0, 0, 0, true},

    {4, "WAV2VEC", "", PATHS.ALP_DATASET_BINARY_DIR_PATH + "sp_wav2vec2_base_960h.bin", 0, 0, 0, 0, true},

}};
} // namespace alp_bench
#endif