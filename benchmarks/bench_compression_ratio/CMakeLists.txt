if (NOT DEFINED ENV{ALP_DATASET_DIR_PATH})
    message(WARNING "You must set ALP_DATASET_DIR_PATH environment variable")
endif ()

add_executable(bench_alp_compression_ratio bench_alp_compression_ratio.cpp)
target_link_libraries(bench_alp_compression_ratio PUBLIC ALP gtest_main)
gtest_discover_tests(bench_alp_compression_ratio)

# Test ALP32 on ML Data: -----------------------------------------------------------------------------------------------
add_executable(bench_alp32_compression_ratio alp32.cpp)
target_link_libraries(bench_alp32_compression_ratio PUBLIC ALP gtest_main)
gtest_discover_tests(bench_alp32_compression_ratio)

