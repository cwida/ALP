cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 17)
project(ALP)

add_compile_options(-fPIC)

# Options : ------------------------------------------------------------------------------------------------------------
option(ALP_BUILD_EXAMPLE "Build Example" OFF)
option(ALP_BUILD_TESTING "Build Test" OFF)
option(ALP_BUILD_BENCHMARKING "Build Benchmark" OFF)
option(ALP_BUILD_BENCHMARKING_COMPRESSION_RATIO "Build Benchmarking compression ratio" OFF)
option(ALP_BUILD_GENERATED "Build generated ALP" OFF)

#-----------------------------------------------------------------------------------------------------------------------
include(FetchContent)
include(CheckCXXCompilerFlag)
include(CMakePrintHelpers)
include(CTest)

# CMAKE_SOURCE_DIR: ----------------------------------------------------------------------------------------------------
add_compile_definitions(CMAKE_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

# Include --------------------------------------------------------------------------------------------------------------
include_directories(include)

# Src: -------------------------------------------------------------------------------------------------------
add_subdirectory(src)

# Example : ------------------------------------------------------------------------------------------------------------
if (ALP_BUILD_EXAMPLE)
    message("---------------------------------------------------------------------------------------------------------")
    message("- Example:")
    include_directories(${CMAKE_SOURCE_DIR}/data/include)
    include_directories(${CMAKE_SOURCE_DIR}/example/include)

    add_subdirectory(${CMAKE_SOURCE_DIR}/example)
endif ()

# Test : ---------------------------------------------------------------------------------------------------------------
if (ALP_BUILD_TESTING)
    message("---------------------------------------------------------------------------------------------------------")
    message("- Testing:")

    # Gtest: -----------------------------------------------------------------------------------------------------------
    include(GoogleTest)
    FetchContent_Declare(googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG e2239ee6043f73722e7aa812a459f54a28552929 # release-1.11.0
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    include_directories(${CMAKE_SOURCE_DIR}/test/include)
    include_directories(${CMAKE_SOURCE_DIR}/data/include)
    add_subdirectory(${CMAKE_SOURCE_DIR}/test)
endif ()

# Generated : ----------------------------------------------------------------------------------------------------------
if (ALP_BUILD_GENERATED)
    message("---------------------------------------------------------------------------------------------------------")
    message("- Generated:")

    add_subdirectory(generated)
endif ()

# Benchmark : ----------------------------------------------------------------------------------------------------------
if (ALP_BUILD_BENCHMARKING)
    message("---------------------------------------------------------------------------------------------------------")
    message("- Benchmarking:")

    include_directories(${CMAKE_SOURCE_DIR}/benchmarks/include)
    include_directories(${CMAKE_SOURCE_DIR}/data/include)
    add_subdirectory(benchmarks)
endif ()