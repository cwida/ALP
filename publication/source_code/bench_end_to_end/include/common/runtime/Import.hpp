#pragma once
#include "Database.hpp"
#include "dataset/column.hpp"
#include "encoding/scheme_pool.hpp"
#include <string>

namespace runtime {
/// imports tpch relations from CSVs in dir into db
void importTPCH(std::string dir, Database& db);

/// imports star schema benchmark from CSVs in dir into db
void importSSB(std::string dir, Database& db);

/// imports alp benchmark from CSVs in dir into db
double import_alp(dataset::Column& col, Database& db, encoding::scheme& encoding);
} // namespace runtime

namespace experiment {
void   remove_binary_file(dataset::Column& col);
void   expand_binary_x_times(dataset::Column& col, size_t x);
bool   is_expanded(dataset::Column& col);
void   clean_compressed_data(dataset::Column& col, encoding::scheme& scheme);
double sum(double* in, size_t c);

} // namespace experiment