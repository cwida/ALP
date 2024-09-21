#include "Units.hpp"
#include "MMapvector.hpp"
#include "Exceptions.hpp"
#include "parser/Parser.hpp"
#include "datablock/Datablock.hpp"
#include "datablock/CMachine.hpp"
#include "datablock/schemes/CSchemePool.hpp"
#include "analysis/Analysis.hpp"
// -------------------------------------------------------------------------------------
#include "gflags/gflags.h"
#include "yaml-cpp/yaml.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "tbb/parallel_for.h"
#include "tbb/task_scheduler_init.h"
// -------------------------------------------------------------------------------------
#include <iostream>
#include <mutex>
// -------------------------------------------------------------------------------------
DEFINE_uint32(block_print_length, 20, ".");
DEFINE_uint32(block_count, 3, ".");
DEFINE_uint32(block_length, 65000, ".");
// -------------------------------------------------------------------------------------
namespace cengine {
vector<map<string, string>> analyzeRelation(Relation &relation)
{
   std::mutex sync_mutex;
   std::vector<map<string, string>> relation_analysis(relation.columns.size());
   tbb::parallel_for(SIZE(0), relation.columns.size(), [&](SIZE col_i) {
     auto &column = relation.columns[col_i];
     auto tuple_count = relation.tuple_count;
     auto &bitmap = column.bitmap;
     map<string, string> column_analysis;
     column_analysis["c_tuple_count"] = to_string(relation.tuple_count);
     column_analysis["c_size"] = to_string(column.sizeInBytes());
     // -------------------------------------------------------------------------------------
     map<string, string> all_tuples;
     switch ( column.type ) {
        case ColumnType::INTEGER : {
           all_tuples = analyzeBlock(column.integers(), bitmap, 0, tuple_count, true);
           break;
        }
        case ColumnType::DOUBLE: {
           all_tuples = analyzeBlock(column.doubles(), bitmap, 0, tuple_count, true);
           break;
        }
        case ColumnType::STRING: {
           all_tuples = analyzeStrBlock(column.strings(), bitmap, 0, tuple_count, true);
           break;
        }
        default : {
           UNREACHABLE();
        }
     }
     for ( const auto &element: all_tuples ) {
        column_analysis["c_" + element.first] = element.second;
     }
     // -------------------------------------------------------------------------------------
     for ( u32 block_i = 1; block_i <= FLAGS_block_count; block_i++ ) {
        u32 start_index = rand() % tuple_count;
        u32 block_length = std::min(FLAGS_block_length, static_cast<u32>(tuple_count - start_index));
        map<string, string> block_tuples;
        switch ( column.type ) {
           case ColumnType::INTEGER : {
              block_tuples = analyzeBlock(column.integers(), bitmap, start_index, block_length, true);
              break;
           }
           case ColumnType::DOUBLE: {
              block_tuples = analyzeBlock(column.doubles(), bitmap, start_index, block_length, true);
              break;
           }
           case ColumnType::STRING: {
              block_tuples = analyzeStrBlock(column.strings(), bitmap, start_index, block_length, true);
              break;
           }
           default : {
              UNREACHABLE();
           }
        }
        for ( const auto &element: block_tuples ) {
           column_analysis["b_" + to_string(block_i) + "_" + element.first] = element.second;
        }
     }
     // -------------------------------------------------------------------------------------
     {
        std::lock_guard lock(sync_mutex);
        relation_analysis[col_i] = column_analysis;
     }
   });
   return relation_analysis;
}
// -------------------------------------------------------------------------------------
map<string, string> analyzeStrBlock(const Vector<str> &column, Vector<BITMAP> &bitmap, u32 start_index, u32 tuple_count, bool print_block)
{
   // CAN NOT simply substr strings, because we have multibyte chars (:@)
   map<string, string> stats;
   stats["random_element"] = "-";
   bool is_starting_values_initialized = false;
   u32 min_length = column[start_index].length(), max_length = column[start_index].length();
   u32 null_count = 0;
   u32 zero_count = 0;
   std::unordered_map<str, u32> frequency;
   u64 sum_length = 0;

   for ( u32 tuple_i = start_index; tuple_i < start_index + tuple_count; tuple_i++ ) {
      BITMAP is_set = bitmap.data[tuple_i];

      if ( !is_set ) {
         null_count++;
         continue;
      }

      auto current_value = column[tuple_i];
      if ( current_value.size() == 0 ) {
         zero_count++;
      }

      if ( frequency.find(current_value) == frequency.end()) {
         frequency.insert({current_value, 1});
      } else {
         frequency[current_value] = frequency[current_value] + 1;
      }

      if ( is_starting_values_initialized ) {
         if ( current_value.length() > max_length )
            max_length = current_value.length();
         if ( current_value.length() < min_length )
            min_length = current_value.length();
      } else {
         is_starting_values_initialized = true;
         min_length = max_length = current_value.length();
         stats["random_element"] = current_value;
      }

      sum_length += current_value.length();
   }
   const u32 set_count = tuple_count - null_count;
   const u32 unique_count = frequency.size();
   {
      using Comparator = function<bool(pair<str, u32>, pair<str, u32>)>;
      // Defining a lambda function to compare two pairs. It will compare two pairs using second field
      Comparator compFunctor =
        [](pair<str, u32> elem1, pair<str, u32> elem2) {
          return elem1.second > elem2.second;
        };
      // Declaring a set that will store the pairs using above comparision logic
      set<pair<str, u32>, Comparator> frequency_set(frequency.begin(), frequency.end(), compFunctor);
      u32 top_i = 1;
      for ( const auto &element: frequency_set ) {
         str value = element.first;
         double frequency = static_cast<double>(element.second) * 100.0 / static_cast<double>(set_count);
         string key_prefix = "top_" + to_string(top_i);
         string value_key = key_prefix + "_value";
         string percent_key = key_prefix + "_percent";

         stats[value_key] = value;
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
         stats[percent_key] = "";
      }
   }
   float average_length = sum_length / tuple_count;

   if ( is_starting_values_initialized ) {
      stats["min"] = to_string(min_length);
      stats["max"] = to_string(max_length);
   } else {
      stats["min"] = "-";
      stats["max"] = "-";
   }
   stats["null_count"] = to_string(null_count);
   stats["zero_count"] = to_string(zero_count);
   stats["unique_count"] = to_string(unique_count);
   stats["average_length"] = to_string(average_length);
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
}