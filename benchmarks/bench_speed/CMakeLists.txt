# Bench ALP --------------------------------------------------------------------------------------------------------
configure_file(${CMAKE_SOURCE_DIR}/benchmarks/fls_bench/fls_bench.hpp ${CMAKE_CURRENT_BINARY_DIR}/bench_alp.hpp)
add_executable(bench_alp_encode bench_alp_encode.cpp)
target_include_directories(bench_alp_encode PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(bench_alp_encode PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(bench_alp_encode PRIVATE ALP)

# Bench ALP CUTTER ENCODE ----------------------------------------------------------------------------------------------
configure_file(${CMAKE_SOURCE_DIR}/benchmarks/fls_bench/fls_bench.hpp ${CMAKE_CURRENT_BINARY_DIR}/bench_alp.hpp)
add_executable(bench_alp_cutter_encode bench_alp_cutter_encode.cpp)
target_include_directories(bench_alp_cutter_encode PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(bench_alp_cutter_encode PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(bench_alp_cutter_encode PRIVATE ALP)

# Bench ALP CUTTER DECODE ----------------------------------------------------------------------------------------------
configure_file(${CMAKE_SOURCE_DIR}/benchmarks/fls_bench/fls_bench.hpp ${CMAKE_CURRENT_BINARY_DIR}/bench_alp.hpp)
add_executable(bench_alp_cutter_decode bench_alp_cutter_decode.cpp)
target_include_directories(bench_alp_cutter_decode PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(bench_alp_cutter_decode PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(bench_alp_cutter_decode PRIVATE ALP)

# Bench ALP without SAMPLING --------------------------------------------------------------------------------------------------------
configure_file(${CMAKE_SOURCE_DIR}/benchmarks/fls_bench/fls_bench.hpp ${CMAKE_CURRENT_BINARY_DIR}/bench_alp_tmp.hpp)
add_executable(bench_alp_without_sampling bench_alp_without_sampling.cpp)
target_include_directories(bench_alp_without_sampling PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(bench_alp_without_sampling PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(bench_alp_without_sampling PRIVATE ALP)