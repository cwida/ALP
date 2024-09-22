#pragma once
#include "Units.hpp"
#include "Column.hpp"
#include "Chunk.hpp"
// -------------------------------------------------------------------------------------
#include "yaml-cpp/yaml.h"
// -------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------
namespace cengine {
// -------------------------------------------------------------------------------------
enum class SplitStrategy : u8 {
   SEQUENTIAL,
   RANDOM
};
// -------------------------------------------------------------------------------------
class Chunk;
class InputChunk;
// -------------------------------------------------------------------------------------
using Range = tuple<u64, u64>;
// -------------------------------------------------------------------------------------
class Relation {
public:
   string name;
   u64 tuple_count;
   vector<Column> columns;
   void collectStatistics(); //TODO
   Relation(const YAML::Node &schema, const string &columns_dir);
   Relation();
   vector<Range> getRanges(cengine::SplitStrategy strategy, u32 max_chunk_count) const;
   Chunk getChunk(vector<Range> &ranges, SIZE chunk_i) const;
   InputChunk getInputChunk(Range &range, SIZE chunk_i, u32 column) const;
   void addColumn(const string column_file_path);
private:
   void fixTupleCount();
};
// -------------------------------------------------------------------------------------
}
// -------------------------------------------------------------------------------------
