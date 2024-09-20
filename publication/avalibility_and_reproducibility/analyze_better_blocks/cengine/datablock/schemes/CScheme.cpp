#include "CScheme.hpp"
#include "datablock/cache/ThreadCache.hpp"
// -------------------------------------------------------------------------------------
DEFINE_uint32(sample_size, 64, "");
DEFINE_uint32(sample_count, 10, "");
// -------------------------------------------------------------------------------------
namespace cengine {
namespace db {
// -------------------------------------------------------------------------------------
double DoubleScheme::expectedCompressionRatio(DoubleStats &stats, u8 allowed_cascading_level)
{
   auto dest = makeBytesArray(FLAGS_sample_size * FLAGS_sample_count * sizeof(DOUBLE) * 100);
   u32 total_before = 0;
   u32 total_after = 0;
   if ( ThreadCache::get().estimation_level++ >= 1 ) {
      total_before += stats.total_size;
      total_after += compress(stats.src, stats.bitmap, dest.get(), stats, allowed_cascading_level);
   } else {
      auto sample = stats.samples(FLAGS_sample_count, FLAGS_sample_size);
      DoubleStats c_stats = DoubleStats::generateStats(std::get<0>(sample).data(), std::get<1>(sample).data(), std::get<0>(sample).size());
      total_before += c_stats.total_size;
      total_after += compress(std::get<0>(sample).data(), std::get<1>(sample).data(), dest.get(), c_stats, allowed_cascading_level);
   }
   ThreadCache::get().estimation_level--;
   return CD(total_before) / CD(total_after);
}
// -------------------------------------------------------------------------------------
double IntegerScheme::expectedCompressionRatio(SInteger32Stats &stats, u8 allowed_cascading_level)
{
   auto dest = makeBytesArray(FLAGS_sample_size * FLAGS_sample_count * sizeof(INTEGER) * 100);
   u32 total_before = 0;
   u32 total_after = 0;
   if ( ThreadCache::get().estimation_level++ >= 1 ) {
      total_before += stats.total_size;
      total_after += compress(stats.src, stats.bitmap, dest.get(), stats, allowed_cascading_level);
   } else {
      auto sample = stats.samples(FLAGS_sample_count, FLAGS_sample_size);
      SInteger32Stats c_stats = SInteger32Stats::generateStats(std::get<0>(sample).data(), std::get<1>(sample).data(), std::get<0>(sample).size());
      total_before += c_stats.total_size;
      total_after += compress(std::get<0>(sample).data(), std::get<1>(sample).data(), dest.get(), c_stats, allowed_cascading_level);
   }
   ThreadCache::get().estimation_level--;
   return CD(total_before) / CD(total_after);
}
// -------------------------------------------------------------------------------------
}
}
