add_library(benchmarker benchmark.cpp)
target_link_libraries(benchmarker PUBLIC ALP gtest_main)
configure_file(${CMAKE_SOURCE_DIR}/benchmarks/fls_bench/fls_bench.hpp ${CMAKE_CURRENT_BINARY_DIR}/bench_alp.hpp)
target_include_directories(benchmarker PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

add_library(ALP::BENCHMARKER ALIAS benchmarker)

if (NOT DEFINED ENV{ALP_DATASET_DIR_PATH})
    message(WARNING "Set ALP_DATASET_DIR_PATH environment variable")
    message(WARNING "Set HURRICANE_ISABEL_DATASET_DIR_PATH" environment variable)
else ()
    add_executable(test_compression_ratio test_compression_ratio.cpp)
    target_link_libraries(test_compression_ratio PUBLIC ALP::BENCHMARKER)
    gtest_discover_tests(test_compression_ratio)
endif ()

add_executable(bench_your_dataset bench_your_dataset.cpp benchmark.cpp)
target_link_libraries(bench_your_dataset PUBLIC ALP::BENCHMARKER)
gtest_discover_tests(bench_your_dataset)


