#include "Units.hpp"
#include "StringSchemeType.hpp"
// -------------------------------------------------------------------------------------
namespace cengine {
namespace db {
// -------------------------------------------------------------------------------------
string ConvertSchemeTypeToString(StringSchemeType type)
{
   switch ( type ) {
      case StringSchemeType::ONE_VALUE:
         return "ONE_VALUE";
      case StringSchemeType::DICTIONARY_8:
         return "DICTIONARY_8";
      case StringSchemeType::DICTIONARY_16:
         return "DICTIONARY_16";
      case StringSchemeType::S_DICT:
         return "S_DICT";
      case StringSchemeType::UNCOMPRESSED:
         return "UNCOMPRESSED";
      case StringSchemeType::FSST:
         return "FSST";
      default:
         throw Generic_Exception("Unknown StringSchemeType");
   }
}
// -------------------------------------------------------------------------------------
}
}
