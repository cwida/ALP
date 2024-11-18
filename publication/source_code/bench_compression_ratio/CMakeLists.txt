if (NOT DEFINED ENV{ALP_DATASET_DIR_PATH})
    message(WARNING "You must set ALP_DATASET_DIR_PATH environment variable")
endif ()

add_executable(publication_bench_alp_compression_ratio alp.cpp)
target_link_libraries(publication_bench_alp_compression_ratio PUBLIC ALP gtest_main)
gtest_discover_tests(publication_bench_alp_compression_ratio)

# Test ALP32 on ML Data: ----------------------------------------------------------------------------------------------------------
add_executable(publication_bench_alp32_compression_ratio alp32.cpp)
target_link_libraries(publication_bench_alp32_compression_ratio PUBLIC ALP gtest_main)
gtest_discover_tests(publication_bench_alp32_compression_ratio)

# Test ZSTD: ----------------------------------------------------------------------------------------------------------
add_executable(publication_bench_zstd_compression_ratio zstd.cpp)
target_link_libraries(publication_bench_zstd_compression_ratio PRIVATE gtest_main libzstd_shared)
gtest_discover_tests(publication_bench_zstd_compression_ratio)

# Test Chimp: ----------------------------------------------------------------------------------------------------------
add_executable(publication_bench_chimp_compression_ratio chimp.cpp)
target_link_libraries(publication_bench_chimp_compression_ratio PRIVATE gtest_main)
gtest_discover_tests(publication_bench_chimp_compression_ratio)

# Test Chimp128: ----------------------------------------------------------------------------------------------------------
add_executable(publication_bench_chimp128_compression_ratio chimp128.cpp)
target_link_libraries(publication_bench_chimp128_compression_ratio PRIVATE gtest_main)
gtest_discover_tests(publication_bench_chimp128_compression_ratio)

# Test Gorillas: ----------------------------------------------------------------------------------------------------------
add_executable(publication_bench_gorillas_compression_ratio gorillas.cpp)
target_link_libraries(publication_bench_gorillas_compression_ratio PRIVATE gtest_main)
gtest_discover_tests(publication_bench_gorillas_compression_ratio)

# Test Patas: ----------------------------------------------------------------------------------------------------------
add_executable(publication_bench_patas_compression_ratio patas.cpp)
target_link_libraries(publication_bench_patas_compression_ratio PRIVATE gtest_main)
gtest_discover_tests(publication_bench_patas_compression_ratio)