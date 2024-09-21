# Describe property
define_property(GLOBAL PROPERTY ALP_ALL_BENCHMARK_TARGETS
        BRIEF_DOCS "Global list of elements"
        FULL_DOCS "Global list of elements")
# Initialize property
set_property(GLOBAL PROPERTY ALP_ALL_BENCHMARK_TARGETS "")

# Macro for add values into the list
macro(add_alp_benchmark var)
    set_property(GLOBAL APPEND PROPERTY ALP_ALL_BENCHMARK_TARGETS "${var}")
endmacro(add_alp_benchmark)
#----------------------------------------------------------------------------------------------------------------------- #-----------------------------------------------------------------------------------------------------------------------
# always add fallback implementations. not important for benchmarking
add_compile_definitions(ALP_FALLBACK)
add_subdirectory(fallback)
#-----------------------------------------------------------------------------------------------------------------------
if (${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    add_subdirectory(wasm)
else ()
    if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm64" OR ${CMAKE_SYSTEM_PROCESSOR} MATCHES "aarch64")
        add_subdirectory(arm64v8)
        add_compile_definitions(ALP_ARM64V8)
    elseif (${CMAKE_SYSTEM_PROCESSOR} MATCHES "(x86)|(X86)|(amd64)|(AMD64)")
#        add_subdirectory(x86_64)
        add_compile_definitions(ALP_X86_64)
    else ()
    endif ()
endif ()
#-----------------------------------------------------------------------------------------------------------------------
add_library(alp_generated SHARED
        ${ALP_GENERATED_OBJECT_FILES}
        alp_generated.cpp)
#-----------------------------------------------------------------------------------------------------------------------
cmake_print_properties(
        TARGETS alp_generated
        PROPERTIES COMPILE_DEFINITIONS
        PROPERTIES COMPILE_OPTIONS
        PROPERTIES COMPILE_FEATURES)
#-----------------------------------------------------------------------------------------------------------------------
get_property(BENCHMARK_LIST GLOBAL PROPERTY ALP_ALL_BENCHMARK_TARGETS)
message(STATUS "BENCHMARK LIST: ${BENCHMARK_LIST}")

foreach (i IN LISTS BENCHMARK_LIST)
    list(APPEND add_custom_target_args COMMAND $<TARGET_FILE:${i}>)
endforeach ()

add_custom_target(alp_benchmark_all
        ${add_custom_target_args}
)