#pragma once
#include "datablock/schemes/CScheme.hpp"
// -------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------
namespace cengine {
namespace db {
namespace v2 {
namespace integer {
// -------------------------------------------------------------------------------------
struct RLEStructure {
   u32 runs_count;
   u32 runs_count_offset;
   u8 values_scheme_code;
   u8 counts_scheme_code;
   u8 data[];
};
// -------------------------------------------------------------------------------------
class RLE : public IntegerScheme {
public:
   virtual double expectedCompressionRatio(SInteger32Stats &stats, u8 allowed_cascading_level) override;
   u32 compress(const INTEGER *src, const BITMAP *nullmap, u8 *dest, SInteger32Stats &stats, u8 allowed_cascading_level) override;
   u32 decompressRuns(INTEGER *values, INTEGER *counts, BitmapWrapper *nullmap, const u8 *src, u32 tuple_count, u32 level);
   void decompress(INTEGER *dest, BitmapWrapper *nullmap, const u8 *src, u32 tuple_count, u32 level) override;
   std::string fullDescription(const u8 *src) override;
   inline virtual IntegerSchemeType schemeType() override
   {
      return staticSchemeType();
   }
   inline static IntegerSchemeType staticSchemeType()
   {
      return IntegerSchemeType::X_RLE;
   }
   INTEGER lookup(u32);
   void scan(Predicate, BITMAP *, const u8 *, u32);
};
// -------------------------------------------------------------------------------------
}
}
}
}
// -------------------------------------------------------------------------------------
