# Step 1: Check compiler support for AVX-512DQ
check_cxx_compiler_flag("-mavx512dq" COMPILER_SUPPORTS_AVX512DQ)

if (COMPILER_SUPPORTS_AVX512DQ)
    message(STATUS "Compiler supports AVX-512DQ.")

    # Write the test program to check for AVX-512DQ support
    file(WRITE "${CMAKE_BINARY_DIR}/check_avx512dq.cpp"
            "#include <immintrin.h>
        int main() {
            __m512d x = _mm512_set1_pd(1.0); // AVX-512DQ specific intrinsic for double-precision
            return 0;
        }"
            )

    try_compile(HAS_AVX512DQ_SUPPORT
            "${CMAKE_BINARY_DIR}"
            "${CMAKE_BINARY_DIR}/check_avx512dq.cpp"
            COMPILE_DEFINITIONS -mavx512dq
            )

    if (HAS_AVX512DQ_SUPPORT)
        message(STATUS "Hardware supports AVX-512DQ. Adding AVX-512DQ flags.")
        set(AVX512DQ_FLAG "-mavx512dq")
    else ()
        message(STATUS "Hardware does not support AVX-512DQ.")
    endif ()
else ()
    message(STATUS "Compiler does not support AVX-512DQ.")
endif ()

# Define the library
add_library(ALP
        SHARED
        falp.cpp
        fastlanes_generated_unffor.cpp
        fastlanes_generated_ffor.cpp
        fastlanes_ffor.cpp
        fastlanes_unffor.cpp
        )

# Add AVX-512DQ flag if supported
if (AVX512DQ_FLAG)
    target_compile_options(ALP PUBLIC ${AVX512DQ_FLAG})
endif ()
