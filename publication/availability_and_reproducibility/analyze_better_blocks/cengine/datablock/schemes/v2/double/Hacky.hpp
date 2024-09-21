#pragma once
#include "datablock/schemes/CScheme.hpp"
// -------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------
namespace cengine {
namespace db {
namespace v2 {
namespace d {
// -------------------------------------------------------------------------------------
struct HackyStructure {
   bool three_way_split;
   // -------------------------------------------------------------------------------------
   s16 common_exponent;
   // -------------------------------------------------------------------------------------
   u8 sign_exponent_code;
   u32 sign_exponent_offset;
   // -------------------------------------------------------------------------------------
   u32 mantissa_top_offset;
   u8 mantissa_top_code;
   // -------------------------------------------------------------------------------------
   u32 mantissa_bottom_offset;
   u8 mantissa_bottom_code;
   // -------------------------------------------------------------------------------------
   u8 data[];
};
// -------------------------------------------------------------------------------------
class Hacky : public DoubleScheme {
public:
   double expectedCompressionRatio(DoubleStats &stats, u8 allowed_cascading_level) override;
   u32 compress(const DOUBLE *src, const BITMAP *nullmap, u8 *dest, DoubleStats &stats, u8 allowed_cascading_level) override;
   void decompress(DOUBLE *dest, BitmapWrapper *bitmap, const u8 *src, u32 tuple_count, u32 level) override;
   std::string fullDescription(const u8 *src) override;
   inline virtual DoubleSchemeType schemeType()
   {
      return staticSchemeType();
   }
   inline static DoubleSchemeType staticSchemeType()
   {
      return DoubleSchemeType::X_HACKY;
   }
};
// -------------------------------------------------------------------------------------
}
}
}
}
// -------------------------------------------------------------------------------------
