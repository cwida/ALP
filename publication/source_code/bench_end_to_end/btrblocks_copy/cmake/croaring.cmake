# ---------------------------------------------------------------------------
# btrblocks
# ---------------------------------------------------------------------------


include(ExternalProject)
find_package(Git REQUIRED)

# Get croaring
ExternalProject_Add(
        croaring_src
        PREFIX "vendor/croaring"
        GIT_REPOSITORY "https://github.com/RoaringBitmap/CRoaring.git"
        GIT_TAG b88b002407b42fafaea23ea5009a54a24d1c1ed4
        TIMEOUT 10
        CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/vendor/croaring
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
        -DCMAKE_CXX_STANDARD=17
        UPDATE_COMMAND ""
        INSTALL_COMMAND ""
)

# Prepare croaring
ExternalProject_Get_Property(croaring_src install_dir)
set(CROARING_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/btrblocks/include)
set(CROARING_LIBRARY_PATH ${CMAKE_SOURCE_DIR}/btrblocks/lib/libroaring.so)
file(MAKE_DIRECTORY ${CROARING_INCLUDE_DIR})
add_library(croaring SHARED IMPORTED)
set_property(TARGET croaring PROPERTY IMPORTED_LOCATION ${CROARING_LIBRARY_PATH})
set_property(TARGET croaring APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${CROARING_INCLUDE_DIR})

# Dependencies
add_dependencies(croaring croaring_src)

message("================================================ HERE ================================================")
cmake_print_variables(install_dir)
message("================================================ HERE ================================================")


