#include "Units.hpp"
#include "Reinterpret.hpp"
#include "Hacky.hpp"
#include "datablock/schemes/CScheme.hpp"
#include "datablock/schemes/CSchemePicker.hpp"
#include "datablock/schemes/v2/integer/PBP.hpp"
#include "storage/Chunk.hpp"
// -------------------------------------------------------------------------------------
#include "gflags/gflags.h"
#include "spdlog/spdlog.h"
#include "double-conversion/fast-dtoa.h"
#include "roaring/roaring.hh"
// -------------------------------------------------------------------------------------
#include <iomanip>
#include <cmath>
#include <unordered_map>
#include <bitset>
// -------------------------------------------------------------------------------------
DEFINE_uint32(hacky, 120, "");
DEFINE_uint32(hacky_min_exponent, 10, "");
DEFINE_uint32(hacky_min_occurrence_count, 1000, "");
// -------------------------------------------------------------------------------------
namespace cengine {
namespace db {
namespace v2 {
namespace d {
// -------------------------------------------------------------------------------------
double Hacky::expectedCompressionRatio(DoubleStats &stats, u8 allowed_cascading_level)
{
   if ( stats.tuple_count < 65000 || stats.unique_count * 3 >= stats.total_size )
      return 0;
   return DoubleScheme::expectedCompressionRatio(stats, allowed_cascading_level);
}
// -------------------------------------------------------------------------------------
u32 Hacky::compress(const DOUBLE *src, const BITMAP *nullmap, u8 *dest, DoubleStats &stats, u8 allowed_cascading_level)
{
   // -------------------------------------------------------------------------------------
   auto &col_struct = *reinterpret_cast<HackyStructure *>(dest);
   auto write_ptr = col_struct.data;
   // -------------------------------------------------------------------------------------
   vector<u64> mantissa_v;
   vector<u32> exponent_v;
   vector<INTEGER> sign_exponent_v;
   // -------------------------------------------------------------------------------------
   for ( u32 row_i = 0; row_i < stats.tuple_count; row_i++ ) {
      u64 sign_exponent = RU64(src[row_i]) & 0xFFF0000000000000;
      sign_exponent = sign_exponent >> 52;
      sign_exponent_v.push_back(static_cast<INTEGER>(sign_exponent));
      // -------------------------------------------------------------------------------------
      exponent_v.push_back(sign_exponent & 0x7FFF);
      // -------------------------------------------------------------------------------------
      u64 mantissa = RU64(src[row_i]) & 0x000FFFFFFFFFFFFF;
      mantissa_v.push_back(mantissa);
   }
   // -------------------------------------------------------------------------------------
   die_if(mantissa_v.size() == sign_exponent_v.size());
   // -------------------------------------------------------------------------------------
   {
      col_struct.sign_exponent_offset = write_ptr - col_struct.data;
      u32 used_space;
      IntegerSchemePicker::compress(sign_exponent_v.data(), nullmap, write_ptr, sign_exponent_v.size(), allowed_cascading_level - 1, used_space, col_struct.sign_exponent_code, AUTO_SCHEME, "sign_exponents");
      write_ptr += used_space;
   }
   // -------------------------------------------------------------------------------------
   NumberStats<u32> exponent_stats = NumberStats<u32>::generateStats(exponent_v.data(), nullmap, exponent_v.size());
   NumberStats<INTEGER> sign_exponent_stats = NumberStats<INTEGER>::generateStats(sign_exponent_v.data(), nullmap, sign_exponent_v.size());
//   NumberStats<u64> mantissa_stats = NumberStats<u64>::generateStats(mantissa_v.data(), nullptr, mantissa_v.size());
   // -------------------------------------------------------------------------------------
   col_struct.common_exponent = exponent_stats.distinct_values.begin()->first;
   u32 occurence_count = exponent_stats.distinct_values.begin()->second;
   for ( const auto &t : exponent_stats.distinct_values ) {
      if ( t.second > occurence_count ) {
         occurence_count = t.second;
         col_struct.common_exponent = t.first;
      }
   }
   col_struct.common_exponent -= 1023;
   // -------------------------------------------------------------------------------------
   if ( stats.min >= 0 && col_struct.common_exponent > 0 ) {
      col_struct.three_way_split = true;
      col_struct.common_exponent = std::min(col_struct.common_exponent, s16(31));
      // -------------------------------------------------------------------------------------
      vector<INTEGER> opt_mantissa_top_v;
      vector<u64> opt_mantissa_bot_v;
      // -------------------------------------------------------------------------------------
      for ( u32 row_i = 0; row_i < stats.tuple_count; row_i++ ) {
         u64 opt_mantissa = RU64(src[row_i]) & 0x000FFFFFFFFFFFFF;
         // -------------------------------------------------------------------------------------
         u64 opt_mantissa_top = opt_mantissa >> (52 - col_struct.common_exponent);
         opt_mantissa_top_v.push_back(static_cast<INTEGER>(opt_mantissa_top)); // top 23 bits
         // -------------------------------------------------------------------------------------
         u64 opt_mantissa_bot = opt_mantissa << (12 + col_struct.common_exponent);
         opt_mantissa_bot = opt_mantissa_bot >> (12 + col_struct.common_exponent);
         opt_mantissa_bot_v.push_back(opt_mantissa_bot);
      }
      NumberStats<INTEGER> opt_mantissa_top_stats = NumberStats<INTEGER>::generateStats(opt_mantissa_top_v.data(), nullmap, opt_mantissa_top_v.size());
      NumberStats<u64> opt_mantissa_bot_stats = NumberStats<u64>::generateStats(opt_mantissa_bot_v.data(), nullmap, opt_mantissa_bot_v.size());
      // -------------------------------------------------------------------------------------
      {
         col_struct.mantissa_top_offset = write_ptr - col_struct.data;
         u32 used_space;
         IntegerSchemePicker::compress(opt_mantissa_top_v.data(), nullmap, write_ptr, opt_mantissa_top_v.size(), allowed_cascading_level - 1, used_space, col_struct.mantissa_top_code, AUTO_SCHEME, "mantissa_top");
         write_ptr += used_space;
      }
      // -------------------------------------------------------------------------------------
      {
         col_struct.mantissa_bottom_offset = write_ptr - col_struct.data;
         u32 used_space = integer::FBP64::compress(opt_mantissa_bot_v.data(), write_ptr, opt_mantissa_bot_v.size());
         write_ptr += used_space;
      }
      // -------------------------------------------------------------------------------------
      return write_ptr - dest;
   } else {
      col_struct.three_way_split = false;
      col_struct.mantissa_top_offset = write_ptr - col_struct.data;
      // -------------------------------------------------------------------------------------
      u32 used_space = integer::FBP64::compress(mantissa_v.data(), write_ptr, mantissa_v.size());
      write_ptr += used_space;
      return write_ptr - dest;
   }
}
// -------------------------------------------------------------------------------------
    void Hacky::decompress(DOUBLE *dest, BitmapWrapper *, const u8 *src, u32 tuple_count, u32 level)
{
   // -------------------------------------------------------------------------------------
   auto &col_struct = *reinterpret_cast<const HackyStructure *>(src);
   thread_local std::vector<std::vector<INTEGER>> sign_exponent_v;
   auto sign_exponent = get_level_data(sign_exponent_v, tuple_count + SIMD_EXTRA_ELEMENTS(INTEGER), level);
   {
      IntegerScheme &scheme = IntegerSchemePicker::MyTypeWrapper::getScheme(col_struct.sign_exponent_code);
       scheme.decompress(sign_exponent, nullptr, col_struct.data + col_struct.sign_exponent_offset,
                         tuple_count,
                         level+1);
   }
   auto write_ptr = reinterpret_cast<u64 *>(dest);
   if ( col_struct.three_way_split ) {
      // -------------------------------------------------------------------------------------
      thread_local std::vector<std::vector<INTEGER>> mantissa_top_v;
      auto mantissa_top = get_level_data(mantissa_top_v, tuple_count + SIMD_EXTRA_ELEMENTS(INTEGER), level);
      {
         IntegerScheme &scheme = IntegerSchemePicker::MyTypeWrapper::getScheme(col_struct.mantissa_top_code);
          scheme.decompress(mantissa_top, nullptr, col_struct.data + col_struct.mantissa_top_offset,
                            tuple_count, level+1);
      }
      // -------------------------------------------------------------------------------------
      thread_local std::vector<std::vector<u64>> mantissa_bot_v;
      auto mantissa_bot = get_level_data(mantissa_bot_v, tuple_count * 2, level);
      {
        integer::FBP64::decompress(reinterpret_cast<u8*>(mantissa_bot), col_struct.data + col_struct.mantissa_bottom_offset, tuple_count, level+1);
      }
      // -------------------------------------------------------------------------------------
      for ( u32 row_i = 0; row_i < tuple_count; row_i++ ) {
         write_ptr[row_i] = sign_exponent[row_i];
         write_ptr[row_i] = write_ptr[row_i] << 52;
         // -------------------------------------------------------------------------------------
         u64 tmp = mantissa_top[row_i];
         tmp = tmp << (52 - col_struct.common_exponent);
         write_ptr[row_i] = write_ptr[row_i] | tmp;
         // -------------------------------------------------------------------------------------
//         write_ptr[row_i] = write_ptr[row_i] >> (52 - col_struct.common_exponent);
//         write_ptr[row_i] = write_ptr[row_i] << (52 - col_struct.common_exponent);
         write_ptr[row_i] = write_ptr[row_i] | mantissa_bot[row_i];
      }
   } else {
      thread_local std::vector<std::vector<u64>> mantissa_v;
      auto mantissa = get_level_data(mantissa_v, tuple_count + 100, level);
      // -------------------------------------------------------------------------------------
      integer::FBP64::decompress(reinterpret_cast<u8*>(mantissa), col_struct.data + col_struct.mantissa_top_offset, tuple_count, level+1);
      for ( u32 row_i = 0; row_i < tuple_count; row_i++ ) {
         write_ptr[row_i] = sign_exponent[row_i];
         write_ptr[row_i] = write_ptr[row_i] << 52;
         // -------------------------------------------------------------------------------------
         write_ptr[row_i] = write_ptr[row_i] | mantissa[row_i];
      }
   }

}

string Hacky::fullDescription(const u8 *src) {
    auto &col_struct = *reinterpret_cast<const HackyStructure *>(src);
    string result = this->selfDescription();
    {
        IntegerScheme &scheme = IntegerSchemePicker::MyTypeWrapper::getScheme(col_struct.sign_exponent_code);
        result += "\n\t-> ([int] sign exponent) " + scheme.fullDescription(col_struct.data + col_struct.sign_exponent_offset);
    }

    if ( col_struct.three_way_split ) {
        {
            IntegerScheme &scheme = IntegerSchemePicker::MyTypeWrapper::getScheme(col_struct.mantissa_top_code);
            result += "\n\t-> ([int] mantissa top) " + scheme.fullDescription(col_struct.data + col_struct.mantissa_top_offset);
        }
        result += "\n\t-> ([int] mantissa bottom) FBP64";
    } else {
        result += "\n\t-> ([int] mantissa) FBP64";
    }

    return result;
}
// -------------------------------------------------------------------------------------
}
}
}
}
// -------------------------------------------------------------------------------------
