#pragma once

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "driver/i2c_master.h"

#include "esp_err.h"

#include "u8g2.h"

#define U8G2_IDF_ADAPTER_DEFAULT_GPIO_CONFIG                  \
    {                                                         \
        .reset = GPIO_NUM_NC,                                 \
        .dc = GPIO_NUM_NC,                                    \
    }

#define U8G2_IDF_ADAPTER_DEFAULT_SPI_CONFIG                   \
    {                                                         \
        .host = SPI2_HOST,                                    \
        .mode = 0,                                            \
        .clk = GPIO_NUM_NC,                                   \
        .mosi = GPIO_NUM_NC,                                  \
        .cs = GPIO_NUM_NC,                                    \
    }

#define U8G2_IDF_ADAPTER_DEFAULT_I2C_CONFIG                   \
    {                                                         \
        .port = I2C_NUM_0,                                    \
        .addr_mode = U8G2_IDF_ADAPTER_I2C_ADDR_BIT_LEN_7,     \
        .sda = GPIO_NUM_NC,                                   \
        .scl = GPIO_NUM_NC,                                   \
        .address = 0,                                         \
    }

#define U8G2_IDF_ADAPTER_CONFIG_DEFAULT_SPI                   \
    {                                                         \
        .bus = {.spi = U8G2_IDF_ADAPTER_DEFAULT_SPI_CONFIG,}, \
        .gpio = U8G2_IDF_ADAPTER_DEFAULT_GPIO_CONFIG,         \
        .frequency = 10000,                                   \
        .tx_buf = nullptr,                                    \
        .tx_buf_size = 0,                                     \
    }

#define U8G2_IDF_ADAPTER_CONFIG_DEFAULT_I2C                   \
    {                                                         \
        .bus = {.i2c = U8G2_IDF_ADAPTER_DEFAULT_I2C_CONFIG,}, \
        .gpio = U8G2_IDF_ADAPTER_DEFAULT_GPIO_CONFIG,         \
        .frequency = 10000,                                   \
        .tx_buf = nullptr,                                    \
        .tx_buf_size = 0,                                     \
    }


#ifdef __cplusplus
extern "C" {


#endif


typedef enum {
    U8G2_IDF_ADAPTER_SPI,
    U8G2_IDF_ADAPTER_I2C,
} u8g2_idf_adapter_bus_type;

typedef enum {
    U8G2_IDF_ADAPTER_I2C_ADDR_BIT_LEN_7,
    U8G2_IDF_ADAPTER_I2C_ADDR_BIT_LEN_10,
} u8g2_idf_adapter_i2c_addr_mode;

typedef struct {
    // bus
    union {
        struct {
            spi_host_device_t host;
            int8_t mode;
            gpio_num_t clk;
            gpio_num_t mosi;
            gpio_num_t cs;
        } spi;

        struct {
            i2c_port_t port;
            u8g2_idf_adapter_i2c_addr_mode addr_mode;
            gpio_num_t sda;
            gpio_num_t scl;
            uint16_t address;
        } i2c;
    } bus;

    struct {
        gpio_num_t reset;
        gpio_num_t dc;
    } gpio;

    int frequency;

    uint8_t *tx_buf;
    size_t tx_buf_size;
}
u8g2_idf_adapter_config_t;


// 上下文

typedef struct {
    u8g2_t u8g2;
    u8g2_idf_adapter_config_t config;
    u8g2_idf_adapter_bus_type bus_type;
    size_t tx_buf_used;

    union {
        spi_device_handle_t spi;
        i2c_master_dev_handle_t i2c;
    } dev_handler;
} u8g2_idf_adapter_t;


// 初始化上下文

esp_err_t u8g2_idf_adapter_init(
    u8g2_idf_adapter_t *self,
    u8g2_idf_adapter_bus_type bus_type,
    const u8g2_idf_adapter_config_t *config
);

esp_err_t u8g2_idf_adapter_init_by_spi_device(
    u8g2_idf_adapter_t *self,
    const u8g2_idf_adapter_config_t *config,
    spi_device_handle_t device
);

esp_err_t u8g2_idf_adapter_init_by_i2c_device(
    u8g2_idf_adapter_t *self,
    const u8g2_idf_adapter_config_t *config,
    i2c_master_dev_handle_t device
);

// 销毁上下文

// u8g2 回调

uint8_t u8g2_idf_adapter_byte_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);

uint8_t u8g2_idf_adapter_gpio_and_delay_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);

#ifdef __cplusplus
}
#endif
