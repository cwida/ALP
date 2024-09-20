// ---------------------------------------------------------------------------
// cengine
// ---------------------------------------------------------------------------
#include <iostream>
#include "gtest/gtest.h"
#include "gflags/gflags.h"
#include "datablock/schemes/CSchemePool.hpp"
#include "SIMD.hpp"
// ---------------------------------------------------------------------------
DEFINE_string(fsst_stats, "", "");
// ---------------------------------------------------------------------------
int main(int argc, char *argv[])
{
   #ifdef BTR_USE_SIMD
   std::cout << "\033[0;35mSIMD ENABLED\033[0m" << std::endl;
   #else
   std::cout << "\033[0;31mSIMD DISABLED\033[0m" << std::endl;
   #endif
   testing::InitGoogleTest(&argc, argv);
   // -------------------------------------------------------------------------------------
   gflags::ParseCommandLineFlags(&argc, &argv, true);
   cengine::db::CSchemePool::available_schemes = make_unique<cengine::db::SchemesCollection>();
   return RUN_ALL_TESTS();
}
// ---------------------------------------------------------------------------
