find_program(CARGO_BIN cargo)

if (NOT DEFINED FASTALP_DIR)
    if (EXISTS "${CMAKE_SOURCE_DIR}/../fastalp/fastalp/Cargo.toml")
        set(FASTALP_DIR "${CMAKE_SOURCE_DIR}/../fastalp/fastalp")
    elseif (EXISTS "${CMAKE_SOURCE_DIR}/../wedb_embed/fastalp/Cargo.toml")
        set(FASTALP_DIR "${CMAKE_SOURCE_DIR}/../wedb_embed/fastalp")
    else ()
        include(FetchContent)
        FetchContent_Declare(
            fastalp_src
            GIT_REPOSITORY https://github.com/webc-site/fastalp.git
            GIT_TAG v0.1.42
            SOURCE_SUBDIR fastalp
        )
        FetchContent_GetProperties(fastalp_src)
        if (NOT fastalp_src_POPULATED)
            FetchContent_Populate(fastalp_src)
            set(FASTALP_DIR "${fastalp_src_SOURCE_DIR}/fastalp")
        endif ()
    endif ()
endif ()

set(FASTALP_TARGET_DIR "${CMAKE_BINARY_DIR}/cargo_target")
set(FASTALP_STATIC_LIB "${FASTALP_TARGET_DIR}/release/libfastalp.a")

add_custom_command(
    OUTPUT ${FASTALP_STATIC_LIB}
    COMMAND ${CMAKE_COMMAND} -E env CARGO_TARGET_DIR=${FASTALP_TARGET_DIR} ${CARGO_BIN} build --release --manifest-path ${FASTALP_DIR}/Cargo.toml --features capi
    WORKING_DIRECTORY ${FASTALP_DIR}
    COMMENT "Compiling fastalp Rust static library with C-API..."
)
add_custom_target(fastalp_lib DEPENDS ${FASTALP_STATIC_LIB})

add_library(benchmarker benchmark.cpp)
add_dependencies(benchmarker fastalp_lib)
target_link_libraries(benchmarker PUBLIC ALP gtest_main ${FASTALP_STATIC_LIB} pthread dl m)
configure_file(${CMAKE_SOURCE_DIR}/benchmarks/fls_bench/fls_bench.hpp ${CMAKE_CURRENT_BINARY_DIR}/bench_alp.hpp)
target_include_directories(benchmarker PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

add_library(ALP::BENCHMARKER ALIAS benchmarker)

if (NOT DEFINED ENV{ALP_DATASET_DIR_PATH})
    message(WARNING "Set ALP_DATASET_DIR_PATH environment variable")
    message(WARNING "Set HURRICANE_ISABEL_DATASET_DIR_PATH" environment variable)
else ()
    add_executable(test_compression_ratio test_compression_ratio.cpp)
    add_dependencies(test_compression_ratio fastalp_lib)
    target_link_libraries(test_compression_ratio PUBLIC ALP::BENCHMARKER ${FASTALP_STATIC_LIB} pthread dl m)
    gtest_discover_tests(test_compression_ratio)
endif ()

add_executable(bench_your_dataset bench_your_dataset.cpp benchmark.cpp)
add_dependencies(bench_your_dataset fastalp_lib)
target_link_libraries(bench_your_dataset PUBLIC ALP::BENCHMARKER ${FASTALP_STATIC_LIB} pthread dl m)
gtest_discover_tests(bench_your_dataset)
