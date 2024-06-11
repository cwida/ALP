#include "CScheme.hpp"
#include "CSchemePool.hpp"
#include "utils/Utils.hpp"
// -------------------------------------------------------------------------------------
namespace cengine {
namespace db {
bool shouldUseFOR(str min)
{
   return false;
}
bool shouldUseFOR(DOUBLE min)
{
   return false;
   return (Utils::getBitsNeeded(static_cast<u32>(min)) >= 8);
}
bool shouldUseFOR(INTEGER min)
{
   return false;
   return (Utils::getBitsNeeded(min) >= 8);
}
}
}
