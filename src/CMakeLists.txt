# Detect hardware AVX-512 support
include(CheckCXXCompilerFlag)

# Step 1: Check compiler support for AVX-512
check_cxx_compiler_flag("-mavx512dq" COMPILER_SUPPORTS_AVX512)

if (COMPILER_SUPPORTS_AVX512)
    message(STATUS "Compiler supports AVX512.")

    # Step 2: Create a test program for AVX-512
    file(WRITE "${CMAKE_BINARY_DIR}/check_avx512.cpp"
            "#include <immintrin.h>
        int main() {
            __m512i x = _mm512_set1_epi32(1); // AVX-512 instruction
            return _mm512_reduce_add_epi32(x);
        }")

    # Step 3: Try to compile and run the program
    try_compile(HAS_AVX512_SUPPORT "${CMAKE_BINARY_DIR}" "${CMAKE_BINARY_DIR}/check_avx512.cpp"
            CMAKE_FLAGS "-DCMAKE_CXX_FLAGS=-mavx512dq")

    if (HAS_AVX512_SUPPORT)
        message(STATUS "Hardware supports AVX-512. Adding AVX-512 flags.")
        set(AVX512_FLAG "-mavx512dq")
    else ()
        message(STATUS "Hardware does not support AVX-512.")
    endif ()
else ()
    message(STATUS "Compiler does not support AVX-512.")
endif ()

# Define the library
add_library(ALP SHARED
        falp.cpp
        fastlanes_generated_unffor.cpp
        fastlanes_generated_ffor.cpp
        fastlanes_ffor.cpp
        fastlanes_unffor.cpp
        )

# Add AVX-512 flag if supported
if (AVX512_FLAG)
    target_compile_options(ALP PUBLIC ${AVX512_FLAG})
endif ()
