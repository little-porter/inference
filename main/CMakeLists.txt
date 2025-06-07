idf_component_register(SRC_DIRS "." "./Src"
                    #    SRCS "main.c" "wifi.c"  
                       INCLUDE_DIRS "." "./Inc")


littlefs_create_partition_image(lfs ./littlefs_image FLASH_IN_PROJECT)

# target_compile_options(${COMPONENT_LIB} PRIVATE -std=gnu++11)
