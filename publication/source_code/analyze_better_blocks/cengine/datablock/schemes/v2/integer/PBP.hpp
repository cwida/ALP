#pragma once
#include "datablock/schemes/CScheme.hpp"
// -------------------------------------------------------------------------------------
namespace cengine {
namespace db {
namespace v2 {
namespace integer {
// -------------------------------------------------------------------------------------
__attribute__((packed)) struct XPBPStructure { //need to be aligned by 4 because of FastPFor encodeArray
   u32 u32_count; // number of 4 bytes written by FastPFor
   u8 padding;
   u8 data[];
};
// -------------------------------------------------------------------------------------
class PBP : public IntegerScheme {
public:
   u32 compress(const INTEGER *src, const BITMAP *nullmap, u8 *dest, SInteger32Stats &stats, u8 allowed_cascading_level) override;
   void decompress(INTEGER *dest, BitmapWrapper *nullmap, const u8 *src, u32 tuple_count, u32 level) override;
   inline virtual IntegerSchemeType schemeType() override
   {
      return staticSchemeType();
   }
   inline static IntegerSchemeType staticSchemeType()
   {
      return IntegerSchemeType::X_PBP;
   }
   INTEGER lookup(u32);
   void scan(Predicate, BITMAP *, const u8 *, u32);
};
// -------------------------------------------------------------------------------------
class PBP_DELTA : public IntegerScheme {
public:
   double expectedCompressionRatio(SInteger32Stats &stats, u8 allowed_cascading_level);
   u32 compress(const INTEGER *src, const BITMAP *nullmap, u8 *dest, SInteger32Stats &stats, u8 allowed_cascading_level) override;
   void decompress(INTEGER *dest, BitmapWrapper *nullmap, const u8 *src, u32 tuple_count, u32 level) override;
   inline virtual IntegerSchemeType schemeType() override
   {
      return staticSchemeType();
   }
   inline static IntegerSchemeType staticSchemeType()
   {
      return IntegerSchemeType::X_PBP_DELTA;
   }
   INTEGER lookup(u32);
   void scan(Predicate, BITMAP *, const u8 *, u32);
};
// -------------------------------------------------------------------------------------
class FBP : public IntegerScheme {
public:
   double expectedCompressionRatio(SInteger32Stats &stats, u8 allowed_cascading_level);
    u32 compress(const INTEGER *src, const BITMAP *nullmap, u8 *dest, SInteger32Stats &stats, u8 allowed_cascading_level) override;
   void decompress(INTEGER *dest, BitmapWrapper *nullmap, const u8 *src, u32 tuple_count, u32 level) override;
   inline virtual IntegerSchemeType schemeType() override
   {
      return staticSchemeType();
   }
   inline static IntegerSchemeType staticSchemeType()
   {
      return IntegerSchemeType::X_FBP;
   }
   INTEGER lookup(u32) override;
   void scan(Predicate, BITMAP *, const u8 *, u32) override;
};
// -------------------------------------------------------------------------------------
class EXP_FBP : public IntegerScheme {
public:
   double expectedCompressionRatio(SInteger32Stats &stats, u8 allowed_cascading_level);
   u32 compress(const INTEGER *src, const BITMAP *nullmap, u8 *dest, SInteger32Stats &stats, u8 allowed_cascading_level) override;
   void decompress(INTEGER *dest, BitmapWrapper *nullmap, const u8 *src, u32 tuple_count, u32 level) override;
   inline virtual IntegerSchemeType schemeType() override
   {
      return staticSchemeType();
   }
   inline static IntegerSchemeType staticSchemeType()
   {
      return IntegerSchemeType::X_FBP;
   }
   INTEGER lookup(u32) override;
   void scan(Predicate, BITMAP *, const u8 *, u32) override;
};
// -------------------------------------------------------------------------------------
  class FBP64 {
  public:
    static u32 compress(u64 *src, u8 *dest, u32 tuple_count);
    static void decompress(u8* dest, const u8 *src, u32 tuple_count, u32 level);
  };
}
}
}
}
// -------------------------------------------------------------------------------------
