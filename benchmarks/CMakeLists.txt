if (NOT DEFINED ENV{ALP_DATASET_DIR_PATH})
    message(WARNING "You must set ALP_DATASET_DIR_PATH environment variable")
else ()
    add_subdirectory(bench_speed)
    add_subdirectory(bench_compression_ratio)
endif ()




