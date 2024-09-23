if (NOT DEFINED ENV{ALP_DATASET_DIR_PATH})
    message(FATAL_ERROR "You must set ALP_DATASET_DIR_PATH environment variable")
else ()
    add_subdirectory(source_code)
endif ()

