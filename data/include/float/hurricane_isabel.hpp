#ifndef ALP_HURRICANE_ISABEL_HPP
#define ALP_HURRICANE_ISABEL_HPP

#include "column.hpp"

namespace alp_bench {

constexpr size_t N_HURRICANE_ISABEL_COLUMNS = 20;

inline std::array<ALPColumnDescriptor, N_HURRICANE_ISABEL_COLUMNS> get_hurricane_isabel_dataset() {
	static std::array<ALPColumnDescriptor, N_HURRICANE_ISABEL_COLUMNS> HURRICANE_ISABEL = {{
	    {1, "CLOUDf48", "", get_paths().hs + "CLOUDf48.bin.f32", 0, 0, 0, 0},
	    {2, "CLOUDf48-log10", "", get_paths().hs + "CLOUDf48.log10.bin.f32", 0, 0, 0, 0},
	    {3, "PRECIPf48", "", get_paths().hs + "PRECIPf48.bin.f32", 0, 0, 0, 0},
	    {4, "PRECIPf48-log10", "", get_paths().hs + "PRECIPf48.log10.bin.f32", 0, 0, 0, 0},
	    {5, "Pf48", "", get_paths().hs + "Pf48.bin.f32", 0, 0, 0, 0},
	    {6, "QCLOUDf48", "", get_paths().hs + "QCLOUDf48.bin.f32", 0, 0, 0, 0},
	    {7, "QCLOUDf48-log10", "", get_paths().hs + "QCLOUDf48.log10.bin.f32", 0, 0, 0, 0},
	    {8, "QGRAUPf48", "", get_paths().hs + "QGRAUPf48.bin.f32", 0, 0, 0, 0},
	    {9, "QGRAUPf48-log10", "", get_paths().hs + "QGRAUPf48.log10.bin.f32", 0, 0, 0, 0},
	    {10, "QICEf48", "", get_paths().hs + "QICEf48.bin.f32", 0, 0, 0, 0},
	    {11, "QICEf48-log10", "", get_paths().hs + "QICEf48.log10.bin.f32", 0, 0, 0, 0},
	    {12, "QRAINf48", "", get_paths().hs + "QRAINf48.bin.f32", 0, 0, 0, 0},
	    {13, "QRAINf48-log10", "", get_paths().hs + "QRAINf48.log10.bin.f32", 0, 0, 0, 0},
	    {14, "QSNOWf48", "", get_paths().hs + "QSNOWf48.bin.f32", 0, 0, 0, 0},
	    {15, "QSNOWf48-log10", "", get_paths().hs + "QSNOWf48.log10.bin.f32", 0, 0, 0, 0},
	    {16, "QVAPORf48", "", get_paths().hs + "QVAPORf48.bin.f32", 0, 0, 0, 0},
	    {17, "TCf48", "", get_paths().hs + "TCf48.bin.f32", 0, 0, 0, 0},
	    {18, "Uf48", "", get_paths().hs + "Uf48.bin.f32", 0, 0, 0, 0},
	    {19, "Vf48", "", get_paths().hs + "Vf48.bin.f32", 0, 0, 0, 0},
	    {20, "Wf48", "", get_paths().hs + "Wf48.bin.f32", 0, 0, 0, 0},
	}};
	return HURRICANE_ISABEL;
}

} // namespace alp_bench

#endif // ALP_HURRICANE_ISABEL_HPP
