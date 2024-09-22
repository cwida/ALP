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
struct DynamicDictionaryStructure {
   u8 codes_scheme_code;
   u32 codes_offset;
   u8 data[];
};
// -------------------------------------------------------------------------------------
class DynamicDictionary : public IntegerScheme {
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
      return IntegerSchemeType::X_DICT;
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
