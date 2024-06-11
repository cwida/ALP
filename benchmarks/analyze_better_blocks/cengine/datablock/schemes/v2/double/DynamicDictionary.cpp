#include "Units.hpp"
#include "DynamicDictionary.hpp"
#include "datablock/schemes/v2/templated/DynamicDictionary.hpp"
#include "datablock/schemes/CScheme.hpp"
#include "datablock/schemes/CSchemePicker.hpp"
#include "storage/Chunk.hpp"
// -------------------------------------------------------+------------------------------
#include "gflags/gflags.h"
#include "spdlog/spdlog.h"
// -------------------------------------------------------------------------------------
#include <iomanip>
#include <cmath>
// -------------------------------------------------------------------------------------
namespace cengine {
namespace db {
namespace v2 {
namespace d {
// -------------------------------------------------------------------------------------
using MyDynamicDictionary = TDynamicDictionary<DOUBLE, DoubleScheme, DoubleStats, DoubleSchemeType>;
// -------------------------------------------------------------------------------------
double DynamicDictionary::expectedCompressionRatio(cengine::db::DoubleStats &stats, u8 allowed_cascading_level)
{
   return MyDynamicDictionary::expectedCompressionRatio(stats, allowed_cascading_level);
}
// -------------------------------------------------------------------------------------
u32 DynamicDictionary::compress(const DOUBLE *src, const BITMAP *nullmap, u8 *dest, DoubleStats &stats, u8 allowed_cascading_level)
{
   return MyDynamicDictionary::compressColumn(src, nullmap, dest, stats, allowed_cascading_level);
}
// -------------------------------------------------------------------------------------
void DynamicDictionary::decompress(DOUBLE *dest, BitmapWrapper *nullmap, const u8 *src, u32 tuple_count, u32 level)
{
   return MyDynamicDictionary::decompressColumn(dest, nullmap, src, tuple_count, level);
}

string DynamicDictionary::fullDescription(const u8 *src) {
    return MyDynamicDictionary::fullDescription(src, this->selfDescription());
}
// -------------------------------------------------------------------------------------
}
}
}
}
// -------------------------------------------------------------------------------------
