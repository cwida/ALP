#include "Units.hpp"
#include "MMapvector.hpp"
#include "storage/Relation.hpp"
// -------------------------------------------------------------------------------------
#include "gflags/gflags.h"
#include "tbb/parallel_for.h"
#include "tbb/task_scheduler_init.h"
// -------------------------------------------------------------------------------------
#include <set>
#include <map>
#include <functional>
#include <mutex>
// -------------------------------------------------------------------------------------
using namespace std;

// -------------------------------------------------------------------------------------
DECLARE_uint32(block_print_length);
DECLARE_uint32(block_count);
DECLARE_uint32(block_length);
// -------------------------------------------------------------------------------------
namespace cengine {
// -------------------------------------------------------------------------------------
template<typename T>
map<string, string> analyzeBlock(const Vector<T> &column, Vector<BITMAP> &bitmap, u32 start_index, u32 tuple_count, bool print_block = false)
{
   map<string, string> stats;
   stats["random_element"] = "-";
   T min, max;
   bool is_starting_values_initialized = false;
   u32 null_count = 0;
   u32 zero_count = 0;
   std::unordered_map<T, u32> frequency;

   for ( u32 tuple_i = start_index; tuple_i < start_index + tuple_count; tuple_i++ ) {
      BITMAP is_set = bitmap.data[tuple_i];

      if ( !is_set ) {
         null_count++;
         continue;
      }

      auto current_value = column[tuple_i];
      if ( current_value == 0 ) {
         zero_count++;
      }

      if ( frequency.find(current_value) == frequency.end()) {
         frequency.insert({current_value, 1});
      } else {
         frequency[current_value] = frequency[current_value] + 1;
      }

      if ( is_starting_values_initialized ) {
         if ( current_value > max )
            max = current_value;
         if ( current_value < min )
            min = current_value;
      } else {
         is_starting_values_initialized = true;
         min = max = current_value;
         stats["random_element"] = to_string(column[start_index]);
      }

   }
   const u32 set_count = tuple_count - null_count;
   const u32 unique_count = frequency.size();
   {
      using Comparator = std::function<bool(pair<T, u32>, pair<T, u32>)>;
      // Defining a lambda function to compare two pairs. It will compare two pairs using second field
      Comparator compFunctor =
        [](pair<T, u32> elem1, pair<T, u32> elem2) {
          return elem1.second > elem2.second;
        };
      // Declaring a set that will store the pairs using above comparision logic
      set<pair<T, u32>, Comparator> frequency_set(frequency.begin(), frequency.end(), compFunctor);
      u32 top_i = 1;
      for ( const auto &element: frequency_set ) {
         T value = element.first;
         double frequency = static_cast<double>(element.second) * 100.0 / static_cast<double>(set_count);
         string key_prefix = "top_" + to_string(top_i);
         string value_key = key_prefix + "_value";
         string percent_key = key_prefix + "_percent";

         stats[value_key] = to_string(value);
         stats[percent_key] = to_string(frequency);
         if ( top_i++ == ((unique_count >= 3) ? 3 : unique_count)) {
            break;
         }
      }
      for ( ; top_i <= 3; top_i++ ) {
         string key_prefix = "top_" + to_string(top_i);
         string value_key = key_prefix + "_value";
         string percent_key = key_prefix + "_percent";

         stats[value_key] = "";
         stats[percent_key] = "0";
      }
   }

   if ( is_starting_values_initialized ) {
      stats["min"] = to_string(min);
      stats["max"] = to_string(max);
   } else {
      stats["min"] = "-";
      stats["max"] = "-";
   }
   stats["null_count"] = to_string(null_count);
   stats["zero_count"] = to_string(zero_count);
   stats["unique_count"] = to_string(unique_count);
   stats["average_length"] = "-";
   // -------------------------------------------------------------------------------------
   if ( print_block ) {
      string block_rep = "";
      for ( u32 tuple_i = start_index + 1; tuple_i < start_index + FLAGS_block_print_length; tuple_i++ ) {
         BITMAP is_set = bitmap.data[tuple_i];
         if ( !is_set ) {
            block_rep += "N";
         } else if ( column[tuple_i] == column[tuple_i - 1] ) {
            block_rep += ".";
         } else {
            block_rep += "x";
         }
      }
      stats["block"] = block_rep;
   }
   // -------------------------------------------------------------------------------------
   return stats;
}
// -------------------------------------------------------------------------------------
map<string, string> analyzeStrBlock(const Vector<str> &column, Vector<BITMAP> &bitmap, u32 start_index, u32 tuple_count, bool print_block = false);
// -------------------------------------------------------------------------------------
vector<map<string, string>> analyzeRelation(Relation &relation);
}