#add_subdirectory(scalar_av_uf1)
add_subdirectory(scalar_aav_uf1)
#add_subdirectory(scalar_nav_uf1)

add_library(generated_fallback
        OBJECT
        fallback.cpp)

set(ALP_GENERATED_OBJECT_FILES
        ${ALP_GENERATED_OBJECT_FILES} $<TARGET_OBJECTS:generated_fallback>
        PARENT_SCOPE)
