idf_component_register(SRC_DIRS "." "./Src" "./models"
                    #    SRCS "main.c" "wifi.c"  
                       INCLUDE_DIRS "." "./Inc" "./models")


# littlefs_create_partition_image(lfs ./littlefs_image FLASH_IN_PROJECT)

# target_compile_options(${COMPONENT_LIB} PRIVATE -std=gnu++11)
