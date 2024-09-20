message(STATUS "Fetching ZSTD ${ZSTD_REPOSITORY}")
if (${CMAKE_VERSION} VERSION_LESS "3.11")
    message(FATAL_ERROR "CMake 3.11 required to fetch zlib")
endif ()

FetchContent_Declare(
        zstd
        GIT_REPOSITORY https://github.com/facebook/zstd
        GIT_TAG 794ea1b0afca0f020f4e57b6732332231fb23c70)

FetchContent_MakeAvailable(zstd)

include_directories(${zstd_SOURCE_DIR}/lib)
option(ZSTD_BUILD_TESTS OFF)

add_subdirectory(${zstd_SOURCE_DIR}/build/cmake ${zstd_BINARY_DIR})

add_subdirectory(test)
add_subdirectory(bench_speed)


# Benchmark Full Dataset : ---------------------------------------------------------------------------------------------
if (ALP_BUILD_BENCHMARKING_COMPRESSION_RATIO)
    message("---------------------------------------------------------------------------------------------------------")
    message("- Benchmark compression ratio:")
    add_subdirectory(bench_compression_ratio)
endif ()



