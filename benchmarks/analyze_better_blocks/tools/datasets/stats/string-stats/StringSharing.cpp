#include "Units.hpp"
#include "MMapvector.hpp"
#include "Exceptions.hpp"
#include "parser/Parser.hpp"
#include "storage/Relation.hpp"
#include "datablock/Datablock.hpp"
#include "datablock/CMachine.hpp"
#include "datablock/schemes/CSchemePool.hpp"
// -------------------------------------------------------------------------------------
#include "gflags/gflags.h"
#include "yaml-cpp/yaml.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "tbb/parallel_for.h"
#include "tbb/task_scheduler_init.h"
#include "roaring/roaring.hh"
// -------------------------------------------------------------------------------------
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <regex>
#include <iomanip>
#include <mutex>
// -------------------------------------------------------------------------------------
using namespace std;
// -------------------------------------------------------------------------------------
DEFINE_string(in, "", "CSV input file path you want to parse without csv extension");
DEFINE_string(out, "", "Output directory for parsed columns (binary format)");
DEFINE_bool(verify, false, "");
DEFINE_bool(parse, false, "Parse the data before processing");
DEFINE_bool(only_parse, false, "Stop after parsing");
DEFINE_bool(print_chunk_sample, false, "");
DEFINE_string(single_in, "", "Prase single column file only");
DEFINE_uint32(chunks, 1, "Limit the maximum number of processed relation chunks");
DEFINE_uint32(split_strategy, 1, "");
DEFINE_uint32(threads, 20, "");
DEFINE_uint32(schemes, 15, "");
// -------------------------------------------------------------------------------------
DEFINE_string(yaml, "", "");
// -------------------------------------------------------------------------------------
int main(int argc, char **argv)
{
    // Relation::split was replace by Relation::getRanges in 010ace27, but this code was not adjusted
    // Commenting out for now, as I have no idea what it is good for atm.

    /*
   gflags::ParseCommandLineFlags(&argc, &argv, true);
   cengine::db::CSchemePool::refresh();
   // -------------------------------------------------------------------------------------
   tbb::task_scheduler_init init(FLAGS_threads);
   //tbb::task_scheduler_init init(tbb::task_scheduler_init::default_num_threads());  // Explicit number of threads
   // -------------------------------------------------------------------------------------
   const string schema_path = FLAGS_yaml;
   const string out_dir = schema_path.substr(0, schema_path.size() - str(".yaml").size()) + "/";
   // -------------------------------------------------------------------------------------
   cengine::Relation relation;
   const auto schema = YAML::LoadFile(schema_path);
   // Create relation out of yaml schema
   relation = cengine::Relation(schema, out_dir);
   auto chunks = relation.split(static_cast<cengine::SplitStrategy>(FLAGS_split_strategy), FLAGS_chunks);
   // -------------------------------------------------------------------------------------
   auto string_columns = vector<u32>();
   for ( u32 col_i = 0; col_i < relation.columns.size(); col_i++ ) {
      if ( relation.columns[col_i].type == ColumnType::STRING ) {
         string_columns.push_back(col_i);
      }
   }
   // -------------------------------------------------------------------------------------
   u32 before_size = 0;
   u32 after_size = 0;
   for ( const auto &chunk: chunks ) {
      std::map<str, Roaring> all_strings;
      for ( auto col_i : string_columns ) {
         for ( u32 row_i = 0; row_i < chunk.tuple_count; row_i++ ) {
            auto current_str = chunk(col_i, row_i);
            auto it = all_strings.find(current_str);
            if ( it == all_strings.end()) {
               all_strings.insert({current_str, {}});
            }
            all_strings[current_str].add(col_i);
         }
      }
      for ( const auto &str: all_strings ) {
         auto &r = str.second;
         before_size += str.first.length() * r.cardinality();
         after_size += str.first.length() * 1;
      }
   }
   cout << before_size << '\t' << after_size << '\t' << CD(before_size) / CD(after_size) << endl;
     */
   return 0;
}