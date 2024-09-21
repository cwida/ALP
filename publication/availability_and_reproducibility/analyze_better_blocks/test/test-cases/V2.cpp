#include "storage/Relation.hpp"
#include "datablock/Datablock.hpp"
#include "TestHelper.hpp"
#include "datablock/schemes/DoubleSchemeType.hpp"
// -------------------------------------------------------------------------------------
#include "gtest/gtest.h"
#include "gflags/gflags.h"
// -------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------
DECLARE_uint32(force_string_scheme);
DECLARE_uint32(force_integer_scheme);
DECLARE_uint32(force_double_scheme);
#include "datablock/schemes/CSchemePool.hpp"
DECLARE_bool(db2);
// -------------------------------------------------------------------------------------
namespace cengine {
using namespace db;
// -------------------------------------------------------------------------------------
TEST(V2, Begin) {
   FLAGS_db2 = true;
   cengine::db::CSchemePool::refresh();
}
// -------------------------------------------------------------------------------------
TEST(V2, StringCompressedDictionary)
{
   FLAGS_force_string_scheme = CB(StringSchemeType::S_DICT);
   Relation relation;
   relation.addColumn(TEST_DATASET("string/COMPRESSED_DICTIONARY.string"));
   cengine::db::Datablock datablockV2(relation);
   TestHelper::CheckRelationCompression(relation, datablockV2, {CB(StringSchemeType::S_DICT)});
}
// -------------------------------------------------------------------------------------
TEST(V2, IntegerRLE)
{
   FLAGS_force_integer_scheme = CB(IntegerSchemeType::X_RLE);
   Relation relation;
   relation.addColumn(TEST_DATASET("integer/RLE.integer"));
   cengine::db::Datablock datablockV2(relation);
   TestHelper::CheckRelationCompression(relation, datablockV2, {CB(IntegerSchemeType::X_RLE)});
}
// -------------------------------------------------------------------------------------
TEST(V2, DoubleRLE)
{
   FLAGS_force_double_scheme = CB(DoubleSchemeType::X_RLE);
   Relation relation;
   relation.addColumn(TEST_DATASET("double/RANDOM.double"));
   cengine::db::Datablock datablockV2(relation);
   TestHelper::CheckRelationCompression(relation, datablockV2, {CB(DoubleSchemeType::X_RLE)});
   FLAGS_force_double_scheme = AUTO_SCHEME;
}
// -------------------------------------------------------------------------------------
TEST(V2, IntegerDyanmicDict)
{
   FLAGS_force_integer_scheme = CB(IntegerSchemeType::X_DICT);
   Relation relation;
   relation.addColumn(TEST_DATASET("integer/DICTIONARY_16.integer"));
   cengine::db::Datablock datablockV2(relation);
   TestHelper::CheckRelationCompression(relation, datablockV2, {CB(IntegerSchemeType::X_DICT)});
   FLAGS_force_integer_scheme = AUTO_SCHEME;
}
// -------------------------------------------------------------------------------------
TEST(V2, DoubleDecimal)
{
   FLAGS_force_double_scheme = CB(DoubleSchemeType::X_DECIMAL);
   Relation relation;
   relation.addColumn(TEST_DATASET("double/DICTIONARY_8.double"));
   cengine::db::Datablock datablockV2(relation);
   TestHelper::CheckRelationCompression(relation, datablockV2, {CB(DoubleSchemeType::X_DECIMAL)});
   FLAGS_force_double_scheme = AUTO_SCHEME;
}
// -------------------------------------------------------------------------------------
TEST(V2, DoubleDyanmicDict)
{
   FLAGS_force_double_scheme = CB(DoubleSchemeType::X_DICT);
   Relation relation;
   relation.addColumn(TEST_DATASET("double/DICTIONARY_8.double"));
   cengine::db::Datablock datablockV2(relation);
   TestHelper::CheckRelationCompression(relation, datablockV2, {CB(DoubleSchemeType::X_DICT)});
   FLAGS_force_double_scheme = AUTO_SCHEME;
}
// -------------------------------------------------------------------------------------
// TEST(V2, IntegerFrequency)
// {
//    FLAGS_force_integer_scheme = CB(IntegerSchemeType::X_FREQUENCY);
//    Relation relation;
//    relation.addColumn(TEST_DATASET("integer/FREQUENCY.integer"));
//    cengine::db::Datablock datablockV2(relation);
//    TestHelper::CheckRelationCompression(relation, datablockV2, {CB(IntegerSchemeType::X_FREQUENCY)});
//    FLAGS_force_integer_scheme = AUTO_SCHEME;
// }
// -------------------------------------------------------------------------------------
// scheme is disabled
TEST(V2, DoubleFrequency)
{
   FLAGS_force_double_scheme = CB(DoubleSchemeType::X_FREQUENCY);
   Relation relation;
   relation.addColumn(TEST_DATASET("double/FREQUENCY.double"));
   cengine::db::Datablock datablockV2(relation);
   TestHelper::CheckRelationCompression(relation, datablockV2, {CB(DoubleSchemeType::X_FREQUENCY)});
   FLAGS_force_double_scheme = AUTO_SCHEME;
}
// -------------------------------------------------------------------------------------
TEST(V2, End)
{
   FLAGS_db2=false;
   cengine::db::CSchemePool::refresh();
}
// -------------------------------------------------------------------------------------
}
// -------------------------------------------------------------------------------------
