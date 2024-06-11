/*
-- DATE : 18/04/2024
-- FILE_PATH : include/alp/falp.hpp
-- PROJECT_NAME : ALP
*/

#ifndef ALP_FALP_HPP
#define ALP_FALP_HPP

#include <cstdint>

namespace generated { namespace falp {
namespace fallback {
namespace scalar {
void falp(const uint64_t* __restrict in,
          double* __restrict out,
          uint8_t bw,
          const uint64_t* __restrict a_base_p,
          uint8_t factor,
          uint8_t exponent);

} // namespace scalar
namespace unit64 {
void falp(const uint64_t* __restrict in,
          double* __restrict out,
          uint8_t bw,
          const uint64_t* __restrict a_base_p,
          uint8_t factor,
          uint8_t exponent);

} // namespace unit64
} // namespace fallback

namespace helper { namespace scalar {
void falp(const uint64_t* __restrict in,
          double* __restrict out,
          uint8_t bw,
          const uint64_t* __restrict a_base_p,
          uint8_t factor,
          uint8_t exponent);
}} // namespace helper::scalar

namespace x86_64 {
namespace sse {
void falp(const uint64_t* __restrict in,
          double* __restrict out,
          uint8_t bw,
          const uint64_t* __restrict a_base_p,
          uint8_t factor,
          uint8_t exponent);
} // namespace sse

namespace avx2 {
void falp(const uint64_t* __restrict in,
          double* __restrict out,
          uint8_t bw,
          const uint64_t* __restrict a_base_p,
          uint8_t factor,
          uint8_t exponent);
} // namespace avx2

namespace avx512f {
void falp(const uint64_t* __restrict in,
          double* __restrict out,
          uint8_t bw,
          const uint64_t* __restrict a_base_p,
          uint8_t factor,
          uint8_t exponent);
}

namespace avx512bw {
void falp(const uint64_t* __restrict in,
          double* __restrict out,
          uint8_t bw,
          const uint64_t* __restrict a_base_p,
          uint8_t factor,
          uint8_t exponent);
} // namespace avx512bw

} // namespace x86_64
namespace wasm { namespace simd128 {
void falp(const uint64_t* __restrict in,
          double* __restrict out,
          uint8_t bw,
          const uint64_t* __restrict a_base_p,
          uint8_t factor,
          uint8_t exponent);
}} // namespace wasm::simd128

namespace arm64v8 {
namespace neon {
void falp(const uint64_t* __restrict in,
          double* __restrict out,
          uint8_t bw,
          const uint64_t* __restrict a_base_p,
          uint8_t factor,
          uint8_t exponent);
} // namespace neon

namespace sve {
void falp(const uint64_t* __restrict in,
          double* __restrict out,
          uint8_t bw,
          const uint64_t* __restrict a_base_p,
          uint8_t factor,
          uint8_t exponent);
} // namespace sve
} // namespace arm64v8
}} // namespace generated::falp

#endif // FALP_HPP
