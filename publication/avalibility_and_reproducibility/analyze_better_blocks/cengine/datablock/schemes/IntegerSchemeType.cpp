#include "Units.hpp"
#include "IntegerSchemeType.hpp"
// -------------------------------------------------------------------------------------
namespace cengine {
namespace db {
// -------------------------------------------------------------------------------------
string ConvertSchemeTypeToString(IntegerSchemeType type)
{
   switch ( type ) {
      case IntegerSchemeType::X_PBP:
         return "X_PBP";
      case IntegerSchemeType::X_PBP_DELTA:
         return "X_PBP_DETA";
      case IntegerSchemeType::X_FBP:
         return "X_FBP";
      case IntegerSchemeType::X_RLE:
         return "X_RLE";
      case IntegerSchemeType::X_DICT:
         return "X_DICT";
      case IntegerSchemeType::X_FREQUENCY:
         return "X_FREQUENCY";
      case IntegerSchemeType::ONE_VALUE:
         return "ONE_VALUE";
      case IntegerSchemeType::DICTIONARY_8:
         return "DICTIONARY_8";
      case IntegerSchemeType::DICTIONARY_16:
         return "DICTIONARY_16";
      case IntegerSchemeType::TRUNCATION_8:
         return "TRUNCATION_8";
      case IntegerSchemeType::TRUNCATION_16:
         return "TRUNCATION_16";
      case IntegerSchemeType::UNCOMPRESSED:
         return "UNCOMPRESSED";
      case IntegerSchemeType::X_FOR:
         return "X_FOR";
      default:
         throw Generic_Exception("Unknown IntegerSchemeType");
   }
}
// -------------------------------------------------------------------------------------
}
}
