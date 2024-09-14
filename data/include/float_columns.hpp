#ifndef FLOAT_COLUMNS_HPP
#define FLOAT_COLUMNS_HPP

// NOLINTBEGIN
#include "column.hpp"

namespace alp_bench {
inline std::array<Column, 4> sp_datasets = {{
    {1, "Dino-Vitb16", "", PATHS.ALP_DATASET_BINARY_DIR_PATH + "sp_dino_vitb16.bin", 0, 0, 0, 0, true},
    {2, "GPT2", "", PATHS.ALP_DATASET_BINARY_DIR_PATH + "sp_gpt2.bin", 0, 0, 0, 0, true},
    {3, "Grammarly-lg", "", PATHS.ALP_DATASET_BINARY_DIR_PATH + "sp_grammarly_coedit_lg.bin", 0, 0, 0, 0, true},
    {4, "WAV2VEC", "", PATHS.ALP_DATASET_BINARY_DIR_PATH + "sp_wav2vec2_base_960h.bin", 0, 0, 0, 0, true},

}};

inline std::array<Column, 1> float_test_dataset = {{
    {2,
     "Arade/4",
     PATHS.ALP_DATASET_SAMPLE_CSV_PATH + "arade4.csv",
     PATHS.ALP_DATASET_BINARY_DIR_PATH + "arade4.bin",
     14,
     10,
     8,
     24},
}};

} // namespace alp_bench
#endif

// NOLINTEND