#------------------------------------------------------------------------------------------------------
add_executable(fallback_scalar_aav_1024_uf1_falp_test fallback_scalar_aav_1024_uf1_falp_test.cpp)
target_link_libraries(fallback_scalar_aav_1024_uf1_falp_test PRIVATE ALP)
target_link_libraries(fallback_scalar_aav_1024_uf1_falp_test PRIVATE gtest_main)
target_include_directories(fallback_scalar_aav_1024_uf1_falp_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
gtest_discover_tests(fallback_scalar_aav_1024_uf1_falp_test)
#------------------------------------------------------------------------------------------------------
configure_file(${CMAKE_SOURCE_DIR}/benchmarks/fls_bench/fls_bench.hpp ${CMAKE_CURRENT_BINARY_DIR}/fallback_scalar_aav_1024_uf1_falp_bench.hpp)
add_executable(fallback_scalar_aav_1024_uf1_falp_bench fallback_scalar_aav_1024_uf1_falp_bench.cpp)
target_link_libraries(fallback_scalar_aav_1024_uf1_falp_bench PRIVATE ALP)
target_include_directories(fallback_scalar_aav_1024_uf1_falp_bench PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
add_alp_benchmark(fallback_scalar_aav_1024_uf1_falp_bench)
