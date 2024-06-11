add_library(fallback_scalar_nav_1024_uf1_falp OBJECT
            fallback_scalar_nav_1024_uf1_falp_src.cpp)
target_compile_definitions(fallback_scalar_nav_1024_uf1_falp PRIVATE IS_SCALAR)
set(FLAG -O3)
check_cxx_compiler_flag(${FLAG} HAS_FLAG)
if(HAS_FLAG)
else()
 message(STATUS "The flag ${FLAG} is not supported by the current compiler")
endif()
target_compile_options(fallback_scalar_nav_1024_uf1_falp PUBLIC ${FLAG})
cmake_print_properties(TARGETS fallback_scalar_nav_1024_uf1_falp
                       PROPERTIES COMPILE_DEFINITIONS
                       PROPERTIES COMPILE_OPTIONS)
LIST (APPEND ALP_GENERATED_OBJECT_FILES
      $<TARGET_OBJECTS:fallback_scalar_nav_1024_uf1_falp>)
get_target_property(TARGET_NAME fallback_scalar_nav_1024_uf1_falp NAME)
get_target_property(TARGET_COMPILE_OPTIONS fallback_scalar_nav_1024_uf1_falp COMPILE_OPTIONS)
#------------------------------------------------------------------------------------------------------
add_executable(fallback_scalar_nav_1024_uf1_falp_test fallback_scalar_nav_1024_uf1_falp_test.cpp)
target_link_libraries(fallback_scalar_nav_1024_uf1_falp_test PRIVATE fallback_scalar_nav_1024_uf1_falp)
target_link_libraries(fallback_scalar_nav_1024_uf1_falp_test PRIVATE alp_ffor)
target_link_libraries(fallback_scalar_nav_1024_uf1_falp_test PRIVATE gtest_main)
target_include_directories(fallback_scalar_nav_1024_uf1_falp_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
gtest_discover_tests(fallback_scalar_nav_1024_uf1_falp_test)
#------------------------------------------------------------------------------------------------------
configure_file(${CMAKE_SOURCE_DIR}/alp_bench/alp_bench.hpp ${CMAKE_CURRENT_BINARY_DIR}/fallback_scalar_nav_1024_uf1_falp_bench.hpp)
add_executable(fallback_scalar_nav_1024_uf1_falp_bench fallback_scalar_nav_1024_uf1_falp_bench.cpp)
target_link_libraries(fallback_scalar_nav_1024_uf1_falp_bench PRIVATE fallback_scalar_nav_1024_uf1_falp)
target_link_libraries(fallback_scalar_nav_1024_uf1_falp_bench PRIVATE alp_ffor)
target_include_directories(fallback_scalar_nav_1024_uf1_falp_bench PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
add_alp_benchmark(fallback_scalar_nav_1024_uf1_falp_bench)
