#pragma once
namespace cengine {
namespace db {
// -------------------------------------------------------------------------------------
enum class DoubleSchemeType : u8 {
   X_DECIMAL,
   X_RLE,
   X_DICT,
   X_FREQUENCY,
   X_HACKY,
   ONE_VALUE,
   DICTIONARY_8,
   DICTIONARY_16,
   UNCOMPRESSED,
   X_FOR, // TODO: dirty hack
   DOUBLE_BP
};
string ConvertSchemeTypeToString(DoubleSchemeType type);
// -------------------------------------------------------------------------------------
}
}
