file(GLOB_RECURSE SOURCES src/*.c)
idf_component_register(SRCS ${SOURCES}
        INCLUDE_DIRS "include"
        REQUIRES driver esp_driver_spi esp_driver_i2c esp_driver_gpio freertos u8g2
)