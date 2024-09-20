#include "BZIP2.hpp"
// -------------------------------------------------------------------------------------
#include "bzlib.h"
// -------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------
u32 BZIP2::getCompressedSize(u8 *src, SIZE size)
{
   const SIZE bzip2_buffer_size = size * 2;
   vector<char> bzip_buffer(bzip2_buffer_size);
   unsigned int bzip_dest_len = bzip2_buffer_size;
   auto ret_code = BZ2_bzBuffToBuffCompress(bzip_buffer.data(), &bzip_dest_len, reinterpret_cast<char *>(src), size, 9, 0, 30);
   if ( ret_code != BZ_OK ) {
      throw Generic_Exception("BZIP2 compression failed , error code = " + std::to_string(ret_code));
   }
   die_if(bzip_dest_len != 0);
   return bzip_dest_len;
}
// -------------------------------------------------------------------------------------