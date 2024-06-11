add_library(arm64v8_neon_intrinsic_1024_uf1_falp OBJECT
        arm64v8_neon_intrinsic_1024_uf1_falp_src.cpp)
target_compile_definitions(arm64v8_neon_intrinsic_1024_uf1_falp PRIVATE IS_SCALAR)
set(FLAG -O3)
check_cxx_compiler_flag(${FLAG} HAS_FLAG)
if (HAS_FLAG)
else ()
    message(STATUS "The flag ${FLAG} is not supported by the current compiler")
endif ()
target_compile_options(arm64v8_neon_intrinsic_1024_uf1_falp PUBLIC ${FLAG})
cmake_print_properties(TARGETS arm64v8_neon_intrinsic_1024_uf1_falp
        PROPERTIES COMPILE_DEFINITIONS
        PROPERTIES COMPILE_OPTIONS)
LIST(APPEND ALP_GENERATED_OBJECT_FILES
        $<TARGET_OBJECTS:arm64v8_neon_intrinsic_1024_uf1_falp>)
get_target_property(TARGET_NAME arm64v8_neon_intrinsic_1024_uf1_falp NAME)
get_target_property(TARGET_COMPILE_OPTIONS arm64v8_neon_intrinsic_1024_uf1_falp COMPILE_OPTIONS)
#------------------------------------------------------------------------------------------------------
if (BUILD_TESTING)
    add_executable(arm64v8_neon_intrinsic_1024_uf1_falp_test arm64v8_neon_intrinsic_1024_uf1_falp_test.cpp)
    target_link_libraries(arm64v8_neon_intrinsic_1024_uf1_falp_test PRIVATE ALP gtest_main arm64v8_neon_intrinsic_1024_uf1_falp)
    target_include_directories(arm64v8_neon_intrinsic_1024_uf1_falp_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    gtest_discover_tests(arm64v8_neon_intrinsic_1024_uf1_falp_test)
endif ()
#------------------------------------------------------------------------------------------------------
if (BUILD_BENCHMARK)
    configure_file(${CMAKE_SOURCE_DIR}/alp_bench/alp_bench.hpp ${CMAKE_CURRENT_BINARY_DIR}/arm64v8_neon_intrinsic_1024_uf1_falp_bench.hpp)
    add_executable(arm64v8_neon_intrinsic_1024_uf1_falp_bench arm64v8_neon_intrinsic_1024_uf1_falp_bench.cpp)
    target_link_libraries(arm64v8_neon_intrinsic_1024_uf1_falp_bench PRIVATE ALP arm64v8_neon_intrinsic_1024_uf1_falp)
    target_include_directories(arm64v8_neon_intrinsic_1024_uf1_falp_bench PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
    add_alp_benchmark(arm64v8_neon_intrinsic_1024_uf1_falp_bench)
endif ()
