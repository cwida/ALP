#pragma once
namespace cengine {
namespace db {
// -------------------------------------------------------------------------------------
enum class StringSchemeType : u8 {
   ONE_VALUE,
   DICTIONARY_8,
   DICTIONARY_16,
   S_DICT,
   UNCOMPRESSED,
   FSST
};
string ConvertSchemeTypeToString(StringSchemeType type);
// -------------------------------------------------------------------------------------
}
}