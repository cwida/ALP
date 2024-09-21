#include "LZ4.hpp"
// -------------------------------------------------------------------------------------
#include "lz4.h"
// -------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------
u32 LZ4::getCompressedSize(u8 *src, SIZE size)
{
   const SIZE LZ4_buffer_size = LZ4_compressBound(size);
   vector<char> dest_buffer(LZ4_buffer_size);
   auto after_size = LZ4_compress_default(const_cast<const char *>(reinterpret_cast<char *>(src)), dest_buffer.data(), size, LZ4_buffer_size);
   if ( after_size == 0 ) {
      throw Generic_Exception("LZ4 compression failed");
   }
   return after_size;
}
// -------------------------------------------------------------------------------------