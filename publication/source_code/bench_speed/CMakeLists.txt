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

# Bench PATAS ----------------------------------------------------------------------------------------------------------
configure_file(${CMAKE_SOURCE_DIR}/benchmarks/fls_bench/fls_bench.hpp ${CMAKE_CURRENT_BINARY_DIR}/bench_patas.hpp)
add_executable(bench_patas bench_patas.cpp)
target_include_directories(bench_patas PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

# Bench CHIMP128 -------------------------------------------------------------------------------------------------------
configure_file(${CMAKE_SOURCE_DIR}/benchmarks/fls_bench/fls_bench.hpp ${CMAKE_CURRENT_BINARY_DIR}/bench_chimp128.hpp)
add_executable(bench_chimp128 bench_chimp128.cpp)
target_include_directories(bench_chimp128 PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

# Bench CHIMP ----------------------------------------------------------------------------------------------------------
configure_file(${CMAKE_SOURCE_DIR}/benchmarks/fls_bench/fls_bench.hpp ${CMAKE_CURRENT_BINARY_DIR}/bench_chimp.hpp)
add_executable(bench_chimp bench_chimp.cpp)
target_include_directories(bench_chimp PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

# Bench GORILLAS -------------------------------------------------------------------------------------------------------
configure_file(${CMAKE_SOURCE_DIR}/benchmarks/fls_bench/fls_bench.hpp ${CMAKE_CURRENT_BINARY_DIR}/bench_gorillas.hpp)
add_executable(bench_gorillas bench_gorillas.cpp)
target_include_directories(bench_gorillas PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

# Test ZSTD: ----------------------------------------------------------------------------------------------------------
configure_file(${CMAKE_SOURCE_DIR}/benchmarks/fls_bench/fls_bench.hpp ${CMAKE_CURRENT_BINARY_DIR}/bench_zstd.hpp)
add_executable(bench_zstd bench_zstd.cpp)
target_include_directories(bench_zstd PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(bench_zstd PRIVATE libzstd)
