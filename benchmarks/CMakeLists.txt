if (NOT DEFINED ENV{ALP_DATASET_DIR_PATH})
    message(WARNING "Set ALP_DATASET_DIR_PATH environment variable")
else ()

endif ()

add_subdirectory(bench_speed)


add_executable(bench_alp_compression_ratio bench_compression_ratio.cpp)
target_link_libraries(bench_alp_compression_ratio PUBLIC ALP gtest_main)
gtest_discover_tests(bench_alp_compression_ratio)





