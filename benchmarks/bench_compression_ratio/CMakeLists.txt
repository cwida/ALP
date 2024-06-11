if (NOT DEFINED ENV{ALP_DATASET_DIR_PATH})
    message(FATAL_ERROR "You must set ALP_DATASET_DIR_PATH environment variable")
endif ()

add_executable(bench_alp_compression_ratio alp.cpp)
target_link_libraries(bench_alp_compression_ratio PUBLIC ALP gtest_main)
gtest_discover_tests(bench_alp_compression_ratio)

# Test ALP32 on ML Data: ----------------------------------------------------------------------------------------------------------
add_executable(bench_alp32_compression_ratio alp32.cpp)
target_link_libraries(bench_alp32_compression_ratio PUBLIC ALP gtest_main)
gtest_discover_tests(bench_alp32_compression_ratio)

# Test ZSTD: ----------------------------------------------------------------------------------------------------------
add_executable(bench_zstd_compression_ratio zstd.cpp)
target_link_libraries(bench_zstd_compression_ratio PRIVATE gtest_main libzstd)
gtest_discover_tests(bench_zstd_compression_ratio)

# Test Chimp: ----------------------------------------------------------------------------------------------------------
add_executable(bench_chimp_compression_ratio chimp.cpp)
target_link_libraries(bench_chimp_compression_ratio PRIVATE gtest_main)
gtest_discover_tests(bench_chimp_compression_ratio)

# Test Chimp128: ----------------------------------------------------------------------------------------------------------
add_executable(bench_chimp128_compression_ratio chimp128.cpp)
target_link_libraries(bench_chimp128_compression_ratio PRIVATE gtest_main)
gtest_discover_tests(bench_chimp128_compression_ratio)

# Test Gorillas: ----------------------------------------------------------------------------------------------------------
add_executable(bench_gorillas_compression_ratio gorillas.cpp)
target_link_libraries(bench_gorillas_compression_ratio PRIVATE gtest_main)
gtest_discover_tests(bench_gorillas_compression_ratio)

# Test Patas: ----------------------------------------------------------------------------------------------------------
add_executable(bench_patas_compression_ratio patas.cpp)
target_link_libraries(bench_patas_compression_ratio PRIVATE gtest_main)
gtest_discover_tests(bench_patas_compression_ratio)