if (NOT DEFINED ENV{ALP_DATASET_DIR_PATH})
    message(WARNING "Set ALP_DATASET_DIR_PATH environment variable")
    message(WARNING "Set HURRICANE_ISABEL_DATASET_DIR_PATH" environment variable)
else ()
    add_executable(test_compression_ratio test_compression_ratio.cpp)
    target_link_libraries(test_compression_ratio PUBLIC ALP gtest_main)
    gtest_discover_tests(test_compression_ratio)
endif ()

add_subdirectory(bench_speed)

add_executable(bench_your_dataset bench_your_dataset.cpp)
target_link_libraries(bench_your_dataset PUBLIC ALP gtest_main)
gtest_discover_tests(bench_your_dataset)

