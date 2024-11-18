add_compile_options(-Wno-error)

message(STATUS "Fetching ZSTD ${ZSTD_REPOSITORY}")
if (${CMAKE_VERSION} VERSION_LESS "3.11")
    message(FATAL_ERROR "CMake 3.11 required to fetch zlib")
endif ()

FetchContent_Declare(
        zstd
        GIT_REPOSITORY https://github.com/facebook/zstd.git
        GIT_TAG v1.5.5
)

FetchContent_MakeAvailable(zstd)

include_directories(${zstd_SOURCE_DIR}/lib)
option(ZSTD_BUILD_TESTS OFF)

add_subdirectory(${zstd_SOURCE_DIR}/build/cmake ${zstd_BINARY_DIR})

add_subdirectory(test)
add_subdirectory(bench_speed)
add_subdirectory(bench_compression_ratio)
add_subdirectory(generated)

if (${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
else ()
    if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm64" OR ${CMAKE_SYSTEM_PROCESSOR} MATCHES "aarch64")
    elseif (${CMAKE_SYSTEM_PROCESSOR} MATCHES "(x86)|(X86)|(amd64)|(AMD64)|(x86_64)")
        add_subdirectory(bench_end_to_end)
    else ()
    endif ()
endif ()



