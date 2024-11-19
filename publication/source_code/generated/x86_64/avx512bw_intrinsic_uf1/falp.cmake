add_library(x86_64_avx512bw_intrinsic_1024_uf1_falp OBJECT
            x86_64_avx512bw_intrinsic_1024_uf1_falp_src.cpp)
target_compile_definitions(x86_64_avx512bw_intrinsic_1024_uf1_falp PRIVATE IS_SCALAR)
set(FLAG -mavx512dq)
check_cxx_compiler_flag(${FLAG} HAS_FLAG)
if(HAS_FLAG)
else()
 message(STATUS "The flag ${FLAG} is not supported by the current compiler")
endif()
target_compile_options(x86_64_avx512bw_intrinsic_1024_uf1_falp PUBLIC ${FLAG})
cmake_print_properties(TARGETS x86_64_avx512bw_intrinsic_1024_uf1_falp
                       PROPERTIES COMPILE_DEFINITIONS
                       PROPERTIES COMPILE_OPTIONS)
LIST (APPEND ALP_GENERATED_OBJECT_FILES
      $<TARGET_OBJECTS:x86_64_avx512bw_intrinsic_1024_uf1_falp>)
get_target_property(TARGET_NAME x86_64_avx512bw_intrinsic_1024_uf1_falp NAME)
get_target_property(TARGET_COMPILE_OPTIONS x86_64_avx512bw_intrinsic_1024_uf1_falp COMPILE_OPTIONS)
#------------------------------------------------------------------------------------------------------
add_executable(x86_64_avx512bw_intrinsic_1024_uf1_falp_test x86_64_avx512bw_intrinsic_1024_uf1_falp_test.cpp)
target_link_libraries(x86_64_avx512bw_intrinsic_1024_uf1_falp_test PRIVATE x86_64_avx512bw_intrinsic_1024_uf1_falp)
target_link_libraries(x86_64_avx512bw_intrinsic_1024_uf1_falp_test PRIVATE ALP)
target_link_libraries(x86_64_avx512bw_intrinsic_1024_uf1_falp_test PRIVATE gtest_main)
target_include_directories(x86_64_avx512bw_intrinsic_1024_uf1_falp_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
gtest_discover_tests(x86_64_avx512bw_intrinsic_1024_uf1_falp_test)
#------------------------------------------------------------------------------------------------------
configure_file(${CMAKE_SOURCE_DIR}/benchmarks/fls_bench/fls_bench.hpp ${CMAKE_CURRENT_BINARY_DIR}/x86_64_avx512bw_intrinsic_1024_uf1_falp_bench.hpp)
add_executable(x86_64_avx512bw_intrinsic_1024_uf1_falp_bench x86_64_avx512bw_intrinsic_1024_uf1_falp_bench.cpp)
target_link_libraries(x86_64_avx512bw_intrinsic_1024_uf1_falp_bench PRIVATE x86_64_avx512bw_intrinsic_1024_uf1_falp)
target_link_libraries(x86_64_avx512bw_intrinsic_1024_uf1_falp_bench PRIVATE ALP)
target_include_directories(x86_64_avx512bw_intrinsic_1024_uf1_falp_bench PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
add_alp_benchmark(x86_64_avx512bw_intrinsic_1024_uf1_falp_bench)
#------------------------------------------------------------------------------------------------------
configure_file(${CMAKE_SOURCE_DIR}/benchmarks/fls_bench/fls_bench.hpp ${CMAKE_CURRENT_BINARY_DIR}/x86_64_avx512bw_intrinsic_1024_uf1_falp_bench_bw.hpp)
add_executable(x86_64_avx512bw_intrinsic_1024_uf1_falp_bench_bw x86_64_avx512bw_intrinsic_1024_uf1_falp_bench_bw.cpp)
target_link_libraries(x86_64_avx512bw_intrinsic_1024_uf1_falp_bench_bw PRIVATE x86_64_avx512bw_intrinsic_1024_uf1_falp)
target_link_libraries(x86_64_avx512bw_intrinsic_1024_uf1_falp_bench_bw PRIVATE ALP)
target_include_directories(x86_64_avx512bw_intrinsic_1024_uf1_falp_bench_bw PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
add_alp_benchmark(x86_64_avx512bw_intrinsic_1024_uf1_falp_bench_bw)
