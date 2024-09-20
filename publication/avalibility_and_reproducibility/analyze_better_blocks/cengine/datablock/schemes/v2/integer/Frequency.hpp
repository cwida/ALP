#pragma once
#include "datablock/schemes/CScheme.hpp"
// -------------------------------------------------------------------------------------
#include "roaring/roaring.hh"
// -------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------
namespace cengine {
namespace db {
namespace v2 {
namespace integer {
// -------------------------------------------------------------------------------------
struct FrequencyStructure {
   UINTEGER top_value;
   u32 exceptions_offset;
   u8 next_scheme;
   u8 data[];
};
// -------------------------------------------------------------------------------------
class Frequency : public IntegerScheme {
public:
   double expectedCompressionRatio(SInteger32Stats &stats, u8 allowed_cascading_level);
   u32 compress(const INTEGER *src, const BITMAP *nullmap, u8 *dest, SInteger32Stats &stats, u8 allowed_cascading_level) override;
   void decompress(INTEGER *dest, BitmapWrapper *nullmap, const u8 *src, u32 tuple_count, u32 level) override;
   std::string fullDescription(const u8 *src) override;
   inline virtual IntegerSchemeType schemeType()
   {
      return staticSchemeType();
   }
   inline static IntegerSchemeType staticSchemeType()
   {
      return IntegerSchemeType::X_FREQUENCY;
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
