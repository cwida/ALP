#include "Units.hpp"
#include "DoubleSchemeType.hpp"
// -------------------------------------------------------------------------------------
namespace cengine {
namespace db {
// -------------------------------------------------------------------------------------
string ConvertSchemeTypeToString(DoubleSchemeType type)
{
   switch ( type ) {
      case DoubleSchemeType::X_DECIMAL:
         return "X_DECIMAL";
      case DoubleSchemeType::X_RLE:
         return "X_RLE";
      case DoubleSchemeType::X_DICT:
         return "X_DICT";
      case DoubleSchemeType::X_FREQUENCY:
         return "X_FREQUENCY";
      case DoubleSchemeType::X_HACKY:
         return "X_HACKY";
      case DoubleSchemeType::ONE_VALUE:
         return "ONE_VALUE";
      case DoubleSchemeType::DICTIONARY_8:
         return "DICTIONARY_8";
      case DoubleSchemeType::DICTIONARY_16:
         return "DICTIONARY_16";
      case DoubleSchemeType::UNCOMPRESSED:
         return "UNCOMPRESSED";
      case DoubleSchemeType::X_FOR:
         return "X_FOR";
      default:
         throw Generic_Exception("Unknown IntegerSchemeType");
   }
}
// -------------------------------------------------------------------------------------
}
}
