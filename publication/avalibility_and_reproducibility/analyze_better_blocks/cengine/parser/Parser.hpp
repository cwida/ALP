#pragma once
#include "Units.hpp"
#include "MMapvector.hpp"
#include "Exceptions.hpp"
// -------------------------------------------------------------------------------------
#include "yaml-cpp/yaml.h"
// -------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------
namespace cengine {
// -------------------------------------------------------------------------------------
class Parser {
public:
   struct ColumnDescriptor {
      string name;
      ColumnType column_type;
      u32 vector_offset;
      vector<BITMAP> set_bitmap;
      u32 null_count = 0; // when 'null' comes in the input
      u32 empty_count = 0; // 0 by double and integers, '' by strings
   };
   static void parse(const string csv_path, const YAML::Node &, const string &);
};
// -------------------------------------------------------------------------------------
}

