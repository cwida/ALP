cmake_minimum_required(VERSION 3.2)

if (UNIX AND NOT APPLE)
    set(LINUX true)
else ()
    set(LINUX)
endif ()

#[[============================================= ========================================================================
FLS: [CHG] ]]
#
# prefer to use clang
# prefer to use c++17
# COMMENTED: -------------------------------------------------------------------------------------------------------
#if (LINUX)
#    set(CMAKE_C_COMPILER gcc)
#    set(CMAKE_CXX_COMPILER g++)
#    ADD_DEFINITIONS(-D_LIBCPP_VERSION=1) # tbb clang compatibility
#else ()
#    set(CMAKE_C_COMPILER gcc)
#    set(CMAKE_CXX_COMPILER g++)
#endif ()

#set(CMAKE_CXX_STANDARD 14)
#[[===================================================================================================================]]

set(CMAKE_CXX_STANDARD_REQUIRED ON)

OPTION(VECTORWISE_BRANCHING "Use branching vectorwise primitives" OFF)
OPTION(AUTOVECTORIZE "Allow the compiler to autovectorize" OFF)
OPTION(DATADIR "Directory containing testdata" "")
OPTION(AVX512EXPERIMENTS OFF)
OPTION(HARDWARE_BENCHMARKS OFF)
OPTION(INTERPRET_SEPARATE OFF)
OPTION(HASH_SIZE_32 OFF)

# Compiler flags for the different targets
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -mtune=native -std=c++14 -fPIC -Wno-error")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -Wno-error -fno-omit-frame-pointer -march=native -fdiagnostics-color ")

if (LINUX)
    #  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -static-libasan")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -pthread")
else ()
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -fsanitize=address")
    # set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ")
endif (LINUX)

string(REPLACE "-O2" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
string(REPLACE "-O2" "" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}  -O3")

string(REPLACE "-O2" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O3")
#set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -fopt-info -fopt-info-missed -O3")
string(REPLACE "-O2" "" CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
set(CMAKE_CFLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -O3")

set(CMAKE_MACOSX_RPATH 1)

IF (VECTORWISE_BRANCHING)
    ADD_DEFINITIONS(-DBRANCHING)
ENDIF (VECTORWISE_BRANCHING)
IF (INTERPRET_SEPARATE)
    ADD_DEFINITIONS(-DINTERPRET_SEPARATE)
ENDIF (INTERPRET_SEPARATE)
IF (HASH_SIZE_32)
    ADD_DEFINITIONS(-DHASH_SIZE=32)
ELSE ()
    ADD_DEFINITIONS(-DHASH_SIZE=64)
ENDIF ()

IF (AUTOVECTORIZE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftree-vectorize")
ELSE (AUTOVECTORIZE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-tree-vectorize")
ENDIF (AUTOVECTORIZE)

## Download and unpack googletest at configure time
#configure_file(3rdparty/GTest.CMakeLists.txt googletest-download/CMakeLists.txt)
#execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
#        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download")
#execute_process(COMMAND "${CMAKE_COMMAND}" --build .
#        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download")
#
## Prevent GoogleTest from overriding our compiler/linker options
## when building with Visual Studio
#set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
#
## Add googletest directly to our build. This adds
## the following targets: gtest, gtest_main, gmock
## and gmock_main
#add_subdirectory("${CMAKE_BINARY_DIR}/googletest-src"
#        "${CMAKE_BINARY_DIR}/googletest-build")
#
## The gtest/gmock targets carry header search path
## dependencies automatically when using CMake 2.8.11 or
## later. Otherwise we have to add them here ourselves.
#if (CMAKE_VERSION VERSION_LESS 2.8.11)
#    include_directories("${gtest_SOURCE_DIR}/include"
#            "${gmock_SOURCE_DIR}/include")
#endif ()
#
## Now simply link your own targets against gtest, gmock,
## etc. as appropriate
#

IF (DATADIR)
    message(STATUS "setting datadir to user defined")
    message(STATUS "${DATADIR}")

    ADD_DEFINITIONS(-DDATADIR="${DATADIR}")
ELSE (DATADIR)
    message(STATUS "setting datadir to proj dir ")
    message(STATUS "${PROJECT_SOURCE_DIR}/data")
    ADD_DEFINITIONS(-DDATADIR="${PROJECT_SOURCE_DIR}/data")
ENDIF (DATADIR)

include(FetchContent)
FetchContent_Declare(tbb
        GIT_REPOSITORY https://github.com/oneapi-src/oneTBB.git
        GIT_TAG master
)
FetchContent_MakeAvailable(tbb)
set(TBB_LIBRARIES tbb)
include_directories(${tbb_SOURCE_DIR}/include)


IF (LINUX)
    add_subdirectory(3rdparty/jevents)
    set(JEVENTSLIB "jevents")
ELSE (LINUX)
    set(JEVENTSLIB "")
ENDIF (LINUX)


FetchContent_Declare(
        roaring
        GIT_REPOSITORY https://github.com/RoaringBitmap/CRoaring.git
        GIT_TAG v0.2.66
        GIT_SHALLOW TRUE)

set(ENABLE_ROARING_TESTS OFF CACHE INTERNAL "")

set(ROARING_BUILD_STATIC ON CACHE INTERNAL "")
FetchContent_MakeAvailable(roaring)

FetchContent_GetProperties(roaring)
SET(CPP_ROARING_HEADERS ${roaring_SOURCE_DIR}/cpp/roaring64map.hh ${roaring_SOURCE_DIR}/cpp/roaring.hh)
file(COPY ${CPP_ROARING_HEADERS} DESTINATION ${roaring_SOURCE_DIR}/include/roaring)

message("=============================================================================================================")
cmake_print_variables(roaring_SOURCE_DIR)
message("=============================================================================================================")

include_directories(roaring)
include_directories(${roaring_SOURCE_DIR}/include)

add_library(common
        src/common/algebra/Operators.cpp
        src/common/algebra/Types.cpp
        src/common/runtime/Database.cpp
        src/common/runtime/MemoryPool.cpp
        src/common/runtime/Types.cpp
        src/common/runtime/String.cpp
        src/common/runtime/Import.cpp
        src/common/runtime/Hashmap.cpp
        src/common/runtime/Concurrency.cpp
        src/common/runtime/Profile.cpp
)


target_include_directories(common PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE src)

target_compile_options(common PUBLIC -mavx512dq)

set(HYPER_TRANSLATORS
        src/hyper/codegen/operators/Map.cpp
        src/hyper/codegen/operators/Print.cpp
        src/hyper/codegen/operators/Scan.cpp
)

add_library(hyper
        src/hyper/codegen/Translator.cpp
        src/hyper/codegen/TranslatorRegistry.cpp
)
target_include_directories(hyper PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE src)
target_link_libraries(hyper common)

file(GLOB PRIMITIVES src/vectorwise/primitives/*.cpp)
add_library(vectorwise
        src/vectorwise/Operations.cpp
        src/vectorwise/Operators.cpp
        ${PRIMITIVES}
        src/vectorwise/QueryBuilder.cpp
)
target_include_directories(vectorwise PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE src)
# Set architecture if AVX512 exerperiments are requested
if (AVX512EXPERIMENTS)
    SET_TARGET_PROPERTIES(vectorwise PROPERTIES
            COMPILE_FLAGS "-march=skylake-avx512"
    )
endif ()
target_link_libraries(vectorwise common)


add_library(benchmark_config
        src/benchmarks/config.cpp)
target_include_directories(benchmark_config PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE src)
target_link_libraries(benchmark_config vectorwise common)

file(GLOB TPCHQUERIES
        src/benchmarks/tpch/queries/q1.cpp
        src/benchmarks/tpch/queries/q3.cpp
        src/benchmarks/tpch/queries/q5.cpp
        src/benchmarks/tpch/queries/q5_no_sel.cpp
        src/benchmarks/tpch/queries/q6.cpp
        src/benchmarks/tpch/queries/q9.cpp
        src/benchmarks/tpch/queries/q18.cpp)

add_library(tpch
        ${TPCHQUERIES}
)
target_include_directories(tpch PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE src)
target_link_libraries(tpch benchmark_config pthread hyper ${TBB_LIBRARIES} vectorwise common)


file(GLOB SBBQUERIES src/benchmarks/sbb/queries/*.cpp)


#[[===================================================================================================================]]

add_subdirectory(btrblocks_copy)
include_directories(btrblocks_copy)
include_directories(btrblocks_copy/btrblocks)

# BtrBlocks ------------------------------------------------------------------------------------------------------------
FetchContent_Declare(
        roaring
        GIT_REPOSITORY https://github.com/RoaringBitmap/CRoaring.git
        GIT_TAG v0.2.66
        GIT_SHALLOW TRUE)

set(ENABLE_ROARING_TESTS OFF CACHE INTERNAL "")

set(ROARING_BUILD_STATIC ON CACHE INTERNAL "")
FetchContent_MakeAvailable(roaring)

FetchContent_GetProperties(roaring)
SET(CPP_ROARING_HEADERS ${roaring_SOURCE_DIR}/cpp/roaring64map.hh ${roaring_SOURCE_DIR}/cpp/roaring.hh)
file(COPY ${CPP_ROARING_HEADERS} DESTINATION ${roaring_SOURCE_DIR}/include/roaring)


# Gflags ---------------------------------------------------------------------------------------------------------------
set(FETCHCONTENT_QUIET OFF)

FetchContent_Declare(gflags
        GIT_REPOSITORY https://github.com/gflags/gflags.git
        GIT_TAG master
)

FetchContent_GetProperties(gflags)
if (NOT gflags_POPULATED)
    FetchContent_Populate(gflags)
    cmake_policy(SET CMP0069 NEW)
    add_subdirectory(${gflags_SOURCE_DIR} ${gflags_BINARY_DIR})
endif ()

# Btrblocks ------------------------------------------------------------------------------------------------------------


#[[=====================================================================================================================
FLS: [CHG] ]]

add_executable(run_all
        ${HYPER_TRANSLATORS}
        src/benchmarks/alp/run_all.cpp
        src/benchmarks/alp/queries/config.cpp
        src/benchmarks/alp/queries/q1.cpp)

target_compile_options(run_all PUBLIC -fno-builtin)

target_include_directories(run_all PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        $<INSTALL_INTERFACE:data/include>
        PRIVATE src)


target_link_libraries(run_all PUBLIC gtest_main gflags btrblocks tpch hyper vectorwise common ${JEVENTSLIB} x86_64_avx512bw_intrinsic_1024_uf1_falp ALP libzstd_shared)
target_compile_options(run_all PUBLIC -mavx512dq)


#[[=====================================================================================================================
FLS: [CHG] ]]

add_executable(run_query
        ${HYPER_TRANSLATORS}
        src/benchmarks/alp/run_query.cpp
        src/benchmarks/alp/queries/config.cpp
        src/benchmarks/alp/queries/q1.cpp)

target_compile_options(run_query PUBLIC -fno-builtin)

target_include_directories(run_query PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        $<INSTALL_INTERFACE:data/include>
        PRIVATE src)


target_link_libraries(run_query PUBLIC gtest_main gflags btrblocks tpch hyper vectorwise common ${JEVENTSLIB} x86_64_avx512bw_intrinsic_1024_uf1_falp ALP libzstd_shared)

target_compile_options(run_query PUBLIC -mavx512dq)