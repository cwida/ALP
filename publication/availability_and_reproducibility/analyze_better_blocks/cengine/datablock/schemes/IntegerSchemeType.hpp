#pragma once
namespace cengine {
namespace db {
// -------------------------------------------------------------------------------------
enum class IntegerSchemeType : u8 {
   X_PBP,
   X_PBP_DELTA,
   X_FBP,
   X_RLE,
   X_DICT,
   X_FREQUENCY,
   X_FOR,
   ONE_VALUE,
   UNCOMPRESSED,
   TRUNCATION_8,
   TRUNCATION_16,
   DICTIONARY_8,
   DICTIONARY_16
};
string ConvertSchemeTypeToString(IntegerSchemeType type);
// -------------------------------------------------------------------------------------
}
}
