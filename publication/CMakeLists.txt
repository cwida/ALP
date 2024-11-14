if (NOT DEFINED ENV{ALP_DATASET_DIR_PATH})
    message(WARNING "You must set ALP_DATASET_DIR_PATH environment variable")
endif ()
add_subdirectory(source_code)

