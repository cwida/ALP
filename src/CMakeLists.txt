# Detect AVX512 support
include(CheckCXXCompilerFlag)

check_cxx_compiler_flag("-mavx512dq" COMPILER_SUPPORTS_AVX512)

if(COMPILER_SUPPORTS_AVX512)
    message(STATUS "AVX512 support detected. Adding AVX512 flag for ALP library.")
    set(AVX512_FLAG "-mavx512dq")
else()
    message(STATUS "AVX512 support not detected.")
endif()

# Define the library
add_library(ALP SHARED
        falp.cpp
        fastlanes_generated_unffor.cpp
        fastlanes_generated_ffor.cpp
        fastlanes_ffor.cpp
        fastlanes_unffor.cpp
        )

# Add AVX512 flag if supported
if(AVX512_FLAG)
    target_compile_options(ALP PUBLIC ${AVX512_FLAG})
endif()