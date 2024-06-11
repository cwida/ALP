#pragma once
#include "datablock/schemes/CScheme.hpp"
// -------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------
namespace cengine {
namespace db {
namespace v1 {
namespace d {
// -------------------------------------------------------------------------------------
class Uncompressed : public DoubleScheme {
public:
   double expectedCompressionRatio(DoubleStats &stats, u8 allowed_cascading_level) override;
   u32 compress(const DOUBLE *src, const BITMAP *nullmap, u8 *dest, DoubleStats &stats, u8 allowed_cascading_level) override;
   void decompress(DOUBLE *dest, BitmapWrapper *bitmap, const u8 *src, u32 tuple_count, u32 level) override;
   inline virtual DoubleSchemeType schemeType()
   {
      return staticSchemeType();
   }
   inline static DoubleSchemeType staticSchemeType()
   {
      return DoubleSchemeType::UNCOMPRESSED;
   }
};
// -------------------------------------------------------------------------------------
}
}
}
}
// -------------------------------------------------------------------------------------
