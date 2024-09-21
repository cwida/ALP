#include "Units.hpp"
#include "Uncompressed.hpp"
#include "datablock/schemes/CScheme.hpp"
#include "storage/Chunk.hpp"
// -------------------------------------------------------------------------------------
#include "gflags/gflags.h"
// -------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------
namespace cengine {
namespace db {
namespace v1 {
namespace d {
// -------------------------------------------------------------------------------------
double Uncompressed::expectedCompressionRatio(DoubleStats &stats, u8 allowed_cascading_level)
{
   return 1.0;
}
// -------------------------------------------------------------------------------------
u32 Uncompressed::compress(const DOUBLE *src, const BITMAP *nullmap, u8 *dest, DoubleStats &stats, u8 allowed_cascading_level)
{
   std::memcpy(dest, src, stats.total_size);
   return stats.total_size;
}
// -------------------------------------------------------------------------------------
void Uncompressed::decompress(DOUBLE *dest, BitmapWrapper *, const u8 *src, u32 tuple_count, u32 level)
{
   std::memcpy(dest, src, tuple_count * sizeof(DOUBLE));
}
// -------------------------------------------------------------------------------------
}
}
}
}
// -------------------------------------------------------------------------------------
