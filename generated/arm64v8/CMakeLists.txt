add_subdirectory(neon_intrinsic_uf1)

add_library(generated_arm64v8
        OBJECT
        arm64v8.cpp)

SET(ALP_GENERATED_OBJECT_FILES
        ${ALP_GENERATED_OBJECT_FILES} $<TARGET_OBJECTS:generated_arm64v8> PARENT_SCOPE)