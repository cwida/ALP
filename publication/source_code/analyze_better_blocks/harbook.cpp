// // #include "Units.hpp"
#include "MMapvector.hpp"
#include "Exceptions.hpp"
#include "parser/Parser.hpp"
#include "storage/Relation.hpp"
#include "datablock/Datablock.hpp"
#include "datablock/CMachine.hpp"
#include "datablock/schemes/CSchemePool.hpp"
#include "analysis/Analysis.hpp"
#include "extern/BZIP2.hpp"
#include "extern/LZ4.hpp"
// -------------------------------------------------------------------------------------
#include "gflags/gflags.h"
#include "yaml-cpp/yaml.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "tbb/parallel_for.h"
#include "tbb/task_scheduler_init.h"
// -------------------------------------------------------------------------------------
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <regex>
#include <iomanip>
#include <mutex>
#include <csignal>
#include <datablock/cache/ThreadCache.hpp>
// -------------------------------------------------------------------------------------
using namespace std;
// -------------------------------------------------------------------------------------
DEFINE_string(out, "", "Output directory for parsed columns (binary format)");
DEFINE_string(yaml, "", "Relation's descriptor file in YAML");
// -------------------------------------------------------------------------------------
DEFINE_bool(print_csv, false, "");
DEFINE_bool(print_header, false, "");
// -------------------------------------------------------------------------------------
DEFINE_bool(analyze, false, "");
DEFINE_bool(verify, false, "");
DEFINE_bool(include_null_bitmap, false, "");
DEFINE_bool(parse, false, "Parse the data before processing");
DEFINE_bool(only_parse, false, "Stop after parsing");
DEFINE_bool(print_chunk_sample, false, "");
DEFINE_string(single_in, "", "Prase single column file only");
DEFINE_uint32(chunks, 9999, "Limit the maximum number of processed relation chunks");
DEFINE_uint32(split_strategy, 1, "");
DEFINE_uint32(threads, 20, "");
DEFINE_uint32(schemes, 12, "");
// -------------------------------------------------------------------------------------
DEFINE_bool(log_stdout, false, "");
DEFINE_string(decision_tree, "", "");
DEFINE_string(estimation_deviation, "", "");
DEFINE_string(fsst_stats, "", "");
// -------------------------------------------------------------------------------------
DECLARE_bool(db1);
DECLARE_bool(db2);
DECLARE_uint32(sample_size);
DECLARE_uint32(sample_count);
DECLARE_uint64(block_size);
DEFINE_bool(bzip2, false, "");
DEFINE_bool(lz4, false, "");
// -------------------------------------------------------------------------------------
DEFINE_string(variant, "latest", "");
// -------------------------------------------------------------------------------------
// Constants
const string KEY_RELATION_NAME = "r_name";
const string KEY_COLUMN_NAME = "c_name";
// -------------------------------------------------------------------------------------
void signalHandler(int signum)
{
   cerr << "Interrupt signal (" << signum << ") received.\n";
   cerr << FLAGS_yaml << endl;
   cerr << FLAGS_single_in << endl;
   exit(signum);
}
int main(int argc, char **argv)
{
   // -------------------------------------------------------------------------------------
   signal(SIGINT, signalHandler);
   signal(SIGTERM, signalHandler);
   signal(SIGSEGV, signalHandler);
   signal(SIGFPE, signalHandler);
   signal(SIGABRT, signalHandler);
   // -------------------------------------------------------------------------------------
   gflags::ParseCommandLineFlags(&argc, &argv, true);
   // -------------------------------------------------------------------------------------
   cengine::db::CSchemePool::refresh();
   // -------------------------------------------------------------------------------------
   // Set the default logger to file logger
   if ( !FLAGS_log_stdout ) {
      auto file_logger = spdlog::rotating_logger_mt("main_logger", "log.txt", 1024 * 1024 * 10, 3);
      spdlog::set_default_logger(file_logger);// change log pattern
   }
   spdlog::info("Started harbook with single_in = {}", FLAGS_single_in);
   // -------------------------------------------------------------------------------------
   tbb::task_scheduler_init init(FLAGS_threads);
   //tbb::task_scheduler_init init(tbb::task_scheduler_init::default_num_threads());  // Explicit number of threads
   // -------------------------------------------------------------------------------------
   string schema_path = FLAGS_yaml;
   string relation_name;

   cengine::Relation relation;
   if ( FLAGS_single_in == "" ) {
      const string out_dir = FLAGS_yaml.substr(0, FLAGS_yaml.length() - 5) + "/";
      string csv_path = FLAGS_yaml.replace(FLAGS_yaml.length() - 5, 5, ".csv");
      {
         std::regex re("\\/([^\\/]+).(yaml)");
         std::smatch match;
         if ( std::regex_search(schema_path, match, re) && match.size() > 1 ) {
            relation_name = match.str(1);
         }
      }
      const auto schema = YAML::LoadFile(schema_path);
      if ( FLAGS_parse ) {
         ifstream csv(csv_path);
         if ( !csv.good()) {
            throw Generic_Exception("Can not open csv file.");
         }
         cengine::Parser::parse(csv_path, schema, out_dir);
         cout << "Done parsing." << endl;
      }
      if ( FLAGS_only_parse ) {
         return 0;
      }
      // -------------------------------------------------------------------------------------
      // Create relation out of yaml schema
      relation = cengine::Relation(schema, out_dir);
   } else {
      {
         std::regex re("\\/([^\\/]+)(\\/[^\\/]+).(integer|double|string)");
         std::smatch match;
         if ( std::regex_search(FLAGS_single_in, match, re) && match.size() > 1 ) {
            relation_name = match.str(1);
         }
      }
      relation.addColumn(FLAGS_single_in);
   }
   // -------------------------------------------------------------------------------------
   if ( relation.columns.size() == 0 ) {
      return 0;
   }
   relation.name = relation_name;
   // -------------------------------------------------------------------------------------
   auto ranges = relation.getRanges(static_cast<cengine::SplitStrategy>(FLAGS_split_strategy), FLAGS_chunks);
   vector<std::unique_ptr<u8[]>> output_blocks;
   cengine::db::Datablock datablockV2(relation);
   // -------------------------------------------------------------------------------------
   // Poor man stats
   srand(time(NULL));
   vector<map<string, string>> results(relation.columns.size());
   // -------------------------------------------------------------------------------------
   if ( FLAGS_analyze ) {
      results = cengine::analyzeRelation(relation);
   }
   // -------------------------------------------------------------------------------------
   if ((FLAGS_db1 & FLAGS_db2) || (!FLAGS_db1 && !FLAGS_db2)) {
      throw Generic_Exception("You have to choose between db1 and db2");
   }
   // -------------------------------------------------------------------------------------
   std::mutex compression_summary_mutex;
   vector<std::map<u8, u32>> schemes_frequency(relation.columns.size());
   vector<cengine::OutputBlockStats> db_metas;
   vector<SIZE> col_tuplet_counts(relation.columns.size(), 0);
   vector<SIZE> before_col_sizes(relation.columns.size(), 0);
   vector<SIZE> after_col_sizes(relation.columns.size(), 0);
   vector<vector<string>> chunk_sample(ranges.size(), {relation.columns.size(), ""});
   vector<BytesArray> compressed_chunks;
   compressed_chunks.resize(ranges.size());
   // -------------------------------------------------------------------------------------
   if ( ranges.size() == 0 ) {
      cerr << "Warning: col_id = " << FLAGS_single_in << " is empty !" << endl;
   }
   // -------------------------------------------------------------------------------------
   // External compression
   // BZIP2
   vector<SIZE> after_col_bzip2_sizes(relation.columns.size(), 0);
   // LZ4
   vector<SIZE> after_col_lz4_sizes(relation.columns.size(), 0);
   vector<SIZE> after_db_lz4_sizes(relation.columns.size(), 0);
   // -------------------------------------------------------------------------------------
   tbb::parallel_for(SIZE(0), ranges.size(), [&](SIZE chunk_i) {
      cengine::db::ThreadCache::get().dump_meta.chunk_i = chunk_i;
      // -------------------------------------------------------------------------------------
      auto chunk = relation.getChunk(ranges, chunk_i);
      auto db_meta = datablockV2.compress(chunk, compressed_chunks[chunk_i]);
      // -------------------------------------------------------------------------------------
      // External compression tools
      {
         vector<map<string, u32>> external_compression_results(relation.columns.size());
         for ( u32 col_i = 0; col_i < relation.columns.size(); col_i++ ) {
            if ( FLAGS_bzip2 ) {
               after_col_bzip2_sizes[col_i] += BZIP2::getCompressedSize(chunk.columns[col_i].get(), chunk.size(col_i));
            }
            if ( FLAGS_lz4 ) {
               after_col_lz4_sizes[col_i] += LZ4::getCompressedSize(chunk.columns[col_i].get(), chunk.size(col_i));
               u8 *compressed_column_ptr;
               u32 compressed_column_size;
               datablockV2.getCompressedColumn(compressed_chunks[chunk_i], col_i, compressed_column_ptr, compressed_column_size);
               after_db_lz4_sizes[col_i] += LZ4::getCompressedSize(compressed_column_ptr, compressed_column_size);
            }
         }
      }
      {
         lock_guard lock(compression_summary_mutex);
         for ( u32 col_i = 0; col_i < relation.columns.size(); col_i++ ) {
            col_tuplet_counts[col_i] += chunk.tuple_count;
            // -------------------------------------------------------------------------------------
            auto used_scheme = db_meta.used_compression_schemes[col_i];

            if ( schemes_frequency[col_i].find(used_scheme) == schemes_frequency[col_i].end()) {
               schemes_frequency[col_i].insert({used_scheme, 1});
            } else {
               schemes_frequency[col_i][used_scheme]++;
            }
            if(FLAGS_include_null_bitmap) {
              after_col_sizes[col_i] += db_meta.nullmap_sizes[col_i];
            }
            after_col_sizes[col_i] += db_meta.data_sizes[col_i];
            before_col_sizes[col_i] += chunk.size(col_i);
            db_metas.push_back(db_meta);
         }
      }
      // -------------------------------------------------------------------------------------
      if ( FLAGS_verify ) {
         spdlog::info("Verifying chunk_i = {}", chunk_i);
         vector<cengine::Chunk> decompressed_chunks;
         if ( !(datablockV2.decompress(compressed_chunks[chunk_i]) == chunk)) {
            cerr << "Compressed != Decompressed; used scheme = " << CI(db_metas[chunk_i].used_compression_schemes[0]) << " for chunk_i  = " << chunk_i << endl;
            cerr << schema_path << endl;
            cerr << FLAGS_single_in << endl;
         }
      }
      spdlog::info("Release chunk_i = {}", chunk_i);
      chunk.reset();
      compressed_chunks[chunk_i].reset();
   });
   // -------------------------------------------------------------------------------------
   // Print decision tree
   if ( FLAGS_decision_tree != "" ) {
      ofstream decision_tree_file;
      decision_tree_file.open(FLAGS_decision_tree);
      for ( auto i = cengine::db::ThreadCache::data.begin();
            i != cengine::db::ThreadCache::data.end(); ++i ) {
         decision_tree_file << i->log.str();
      }
      decision_tree_file.close();
   }
   // -------------------------------------------------------------------------------------
   if ( FLAGS_estimation_deviation != "" ) {
      ofstream estimation_deviation_file;
      estimation_deviation_file.open(FLAGS_estimation_deviation, std::ofstream::out | std::ofstream::app);
      if ( estimation_deviation_file.tellp() == 0 ) {
         estimation_deviation_file << "rel_name\tcol_name\tcol_type\tblock_i\tlevel\tscheme_name\testimated_cf\tbefore_size\tafter_size\tactual_cf\tcomment\tunique_count\n";
      }
      for ( auto i = cengine::db::ThreadCache::data.begin();
            i != cengine::db::ThreadCache::data.end(); ++i ) {
         estimation_deviation_file << i->estimation_deviation_csv.str();
      }
      estimation_deviation_file.close();
   }
   // -------------------------------------------------------------------------------------
   if ( FLAGS_fsst_stats != "" ) {
      ofstream fsst_stats_file;
      fsst_stats_file.open(FLAGS_fsst_stats, std::ofstream::out | std::ofstream::app);
      if ( fsst_stats_file.tellp() == 0 ) {
         fsst_stats_file << "rel_name\tcol_name\tblock_i\tbefore_col\tbefore_pool\tafter_pool\tafter_col\n";
      }
      for ( auto i = cengine::db::ThreadCache::data.begin();
            i != cengine::db::ThreadCache::data.end(); ++i ) {
         fsst_stats_file << i->fsst_csv.str();
      }
      fsst_stats_file.close();
   }
   // -------------------------------------------------------------------------------------
   cout << setprecision(4) << fixed;
   // -------------------------------------------------------------------------------------
   string db_version_name = string("db_db") + (FLAGS_db1 ? "1" : "2");
   for ( u32 col_i = 0; col_i < relation.columns.size(); col_i++ ) {
      auto &column_result = results[col_i];
      auto &column = relation.columns[col_i];
      column_result[KEY_COLUMN_NAME] = column.name;
      column_result[KEY_RELATION_NAME] = relation.name;
      column_result["c_type"] = ConvertTypeToString(relation.columns[col_i].type);
      column_result["db_before_size"] = to_string(before_col_sizes[col_i]);
      column_result["db_tuple_count"] = to_string(col_tuplet_counts[col_i]);
      // -------------------------------------------------------------------------------------
      column_result[db_version_name + "_size"] = to_string(after_col_sizes[col_i]);
      column_result[db_version_name + "_bits_pe"] = to_string(CD(after_col_sizes[col_i]) * 8.0 / CD(col_tuplet_counts[col_i]));
      // -------------------------------------------------------------------------------------
      // External compression : data aggregation
      if ( FLAGS_bzip2 ) {
         column_result["db_bzip2_size"] = to_string(after_col_bzip2_sizes[col_i]);
         column_result["db_bzip2_bits_pe"] = to_string(CD(after_col_bzip2_sizes[col_i]) * 8.0 / CD(col_tuplet_counts[col_i]));
      }
      if ( FLAGS_lz4 ) {
         column_result["db_lz4_size"] = to_string(after_col_lz4_sizes[col_i]);
         column_result["db_lz4_bits_pe"] = to_string(CD(after_col_lz4_sizes[col_i]) * 8.0 / CD(col_tuplet_counts[col_i]));
         column_result[db_version_name + "_lz4_size"] = to_string(after_db_lz4_sizes[col_i]);
         column_result[db_version_name + "_lz4_bits_pe"] = to_string(CD(after_db_lz4_sizes[col_i]) * 8.0 / CD(col_tuplet_counts[col_i]));
      }
      // -------------------------------------------------------------------------------------
      if ( FLAGS_db2 ) {
         for ( u8 scheme_i = 0; scheme_i < FLAGS_schemes; scheme_i++ ) {
            auto it = schemes_frequency[col_i].find(scheme_i);
            if ( it == schemes_frequency[col_i].end()) {
               column_result["s_" + to_string(scheme_i)] = "0";
            } else {
               column_result["s_" + to_string(scheme_i)] = to_string(it->second);
            }
         }
      }
   }
   // -------------------------------------------------------------------------------------
   // Print header
   if ( FLAGS_print_csv ) {
      if ( FLAGS_print_header ) {
         for ( auto &map_tuple: results[0] ) {
            cout << map_tuple.first << '\t';
         }
         cout << endl;
      }
      for ( auto &column_result: results ) {
         for ( auto &map_tuple : column_result ) {
            cout << map_tuple.second << '\t';
         }
         cout << endl;
      }
   }
   return 0;
}
