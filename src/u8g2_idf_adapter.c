#include "u8g2_idf_adapter.h"

#include "esp_bit_defs.h"
#include "esp_check.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


static const char *TAG = "u8g2_idf_adapter";

// 初始化配置

void u8g2_idf_adapter_config_init_spi(u8g2_idf_adapter_config_t *config) {
    const u8g2_idf_adapter_config_t inited_config = {
        .bus = {.spi = U8G2_IDF_ADAPTER_DEFAULT_SPI_CONFIG,},
        .gpio = U8G2_IDF_ADAPTER_DEFAULT_GPIO_CONFIG,
        .frequency = 10000,
    };
    *config = inited_config;
}

void u8g2_idf_adapter_config_init_i2c(u8g2_idf_adapter_config_t *config) {
    const u8g2_idf_adapter_config_t inited_config = {
        .bus = {.i2c = U8G2_IDF_ADAPTER_DEFAULT_I2C_CONFIG,},
        .gpio = U8G2_IDF_ADAPTER_DEFAULT_GPIO_CONFIG,
        .frequency = 10000,
    };
    *config = inited_config;
}

// 初始化上下文
static esp_err_t init_spi_device(
    spi_device_handle_t *device_handle,
    const u8g2_idf_adapter_config_t *config
) {
    ESP_RETURN_ON_FALSE(
        device_handle != NULL,
        ESP_ERR_INVALID_ARG,
        TAG,
        "device_handle is NULL"
    );

    ESP_RETURN_ON_FALSE(
        config->bus.spi.clk != GPIO_NUM_NC,
        ESP_ERR_INVALID_ARG,
        TAG,
        "SPI CLK is not configured"
    );

    ESP_RETURN_ON_FALSE(
        config->bus.spi.mosi != GPIO_NUM_NC,
        ESP_ERR_INVALID_ARG,
        TAG,
        "SPI MOSI is not configured"
    );

    ESP_RETURN_ON_FALSE(
        config->bus.spi.cs != GPIO_NUM_NC,
        ESP_ERR_INVALID_ARG,
        TAG,
        "SPI CS is not configured"
    );

    const spi_bus_config_t bus_config = {
        .mosi_io_num = config->bus.spi.mosi,
        .miso_io_num = GPIO_NUM_NC,
        .sclk_io_num = config->bus.spi.clk,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz = 4096,
    };

    ESP_RETURN_ON_ERROR(
        spi_bus_initialize(
            config->bus.spi.host,
            &bus_config,
            SPI_DMA_CH_AUTO
        ),
        TAG,
        "Failed to initialize SPI bus"
    );

    const spi_device_interface_config_t dev_config = {
        .spics_io_num = GPIO_NUM_NC,
        .clock_speed_hz = config->frequency,
        .mode = 0,
        .queue_size = 1,
    };

    const esp_err_t err = spi_bus_add_device(
        config->bus.spi.host,
        &dev_config,
        device_handle
    );

    if (err != ESP_OK) {
        spi_bus_free(config->bus.spi.host);
        return err;
    }

    ESP_LOGI(
        TAG,
        "SPI initialized: CLK=%d MOSI=%d CS=%d freq=%lu",
        config->bus.spi.clk,
        config->bus.spi.mosi,
        config->bus.spi.cs,
        (unsigned long)config->frequency
    );

    return ESP_OK;
}

static esp_err_t init_i2c_device(
    i2c_master_dev_handle_t *device_handle,
    const u8g2_idf_adapter_config_t *config
) {
    ESP_RETURN_ON_FALSE(
        device_handle != NULL,
        ESP_ERR_INVALID_ARG,
        TAG,
        "device_handle is NULL"
    );

    ESP_RETURN_ON_FALSE(
        config->bus.i2c.sda != GPIO_NUM_NC,
        ESP_ERR_INVALID_ARG,
        TAG,
        "I2C SDA GPIO is not configured"
    );

    ESP_RETURN_ON_FALSE(
        config->bus.i2c.scl != GPIO_NUM_NC,
        ESP_ERR_INVALID_ARG,
        TAG,
        "I2C SCL GPIO is not configured"
    );


    const i2c_master_bus_config_t bus_config = {
        .i2c_port = config->bus.i2c.port,
        .sda_io_num = config->bus.i2c.sda,
        .scl_io_num = config->bus.i2c.scl,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    i2c_master_bus_handle_t bus_handle = nullptr;

    ESP_RETURN_ON_ERROR(
        i2c_new_master_bus(
            &bus_config,
            &bus_handle
        ),
        TAG,
        "Failed to initialize I2C bus"
    );

    const i2c_device_config_t dev_config = {
        .dev_addr_length = config->bus.i2c.addr_mode,
        .device_address = config->bus.i2c.address,
        .scl_speed_hz = config->frequency,
    };

    const esp_err_t err = i2c_master_bus_add_device(
        bus_handle,
        &dev_config,
        device_handle
    );

    if (err != ESP_OK) {
        i2c_del_master_bus(bus_handle);
        return err;
    }

    ESP_LOGI(
        TAG,
        "I2C initialized: SDA=%d SCL=%d Addr=0x%02X Freq=%lu",
        config->bus.i2c.sda,
        config->bus.i2c.scl,
        config->bus.i2c.address,
        (unsigned long)config->frequency
    );

    return ESP_OK;
}


esp_err_t u8g2_idf_adapter_init(
    u8g2_idf_adapter_t *self,
    const u8g2_idf_adapter_bus_type bus_type,
    const u8g2_idf_adapter_config_t *config
) {
    self->config = *config;
    self->bus_type = bus_type;
    switch (bus_type) {
        case U8G2_IDF_ADAPTER_SPI:
            spi_device_handle_t spi_handler;
            ESP_RETURN_ON_ERROR(init_spi_device(&spi_handler,config), TAG, "Init SPI Device Failed");
            self->dev_handler.spi = spi_handler;
            break;
        case U8G2_IDF_ADAPTER_I2C:
            i2c_master_dev_handle_t i2c_handle;
            ESP_RETURN_ON_ERROR(init_i2c_device(&i2c_handle,config), TAG, "Init I2C Device Failed");
            self->dev_handler.i2c = i2c_handle;
            break;
    };

    return ESP_OK;
}

esp_err_t u8g2_idf_adapter_init_by_spi_device(
    u8g2_idf_adapter_t *self,
    const u8g2_idf_adapter_config_t *config,
    spi_device_handle_t device
) {
    self->config = *config;
    self->bus_type = U8G2_IDF_ADAPTER_SPI;
    self->dev_handler.spi = device;
    return ESP_OK;
}

esp_err_t u8g2_idf_adapter_init_by_i2c_device(
    u8g2_idf_adapter_t *self,
    const u8g2_idf_adapter_config_t *config,
    i2c_master_dev_handle_t device
) {
    self->config = *config;
    self->bus_type = U8G2_IDF_ADAPTER_I2C;
    self->dev_handler.i2c = device;
    return ESP_OK;
}

static u8g2_idf_adapter_t *get_context(u8x8_t *u8x8) {
#ifdef U8G2_IDF_ADAPTER_U8X8_USER_PTR_CONTEXT
    return *context = u8x8.user_ptr;
#else
    // return __containerof(__containerof(u8x8,u8g2_t,u8x8), u8g2_idf_adapter_t, u8g2);
    return (u8g2_idf_adapter_t *) u8x8;
#endif
}

uint8_t u8g2_idf_adapter_byte_cb(u8x8_t *u8x8, const uint8_t msg, const uint8_t arg_int, void *arg_ptr) {
    const u8g2_idf_adapter_t *context = get_context(u8x8);
    switch (msg) {
        case U8X8_MSG_BYTE_INIT:
            u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_disable_level);
            break;
        case U8X8_MSG_BYTE_SET_DC:
            u8x8_gpio_SetDC(u8x8, arg_int);
            break;
        case U8X8_MSG_BYTE_SEND:
            if (context->bus_type == U8G2_IDF_ADAPTER_SPI) {
                spi_transaction_t t = {};
                t.length = arg_int * 8;
                t.tx_buffer = arg_ptr;
                ESP_ERROR_CHECK(spi_device_transmit(context->dev_handler.spi, &t));
            } else if (context->bus_type == U8G2_IDF_ADAPTER_I2C) {
                ESP_ERROR_CHECK(i2c_master_transmit(context->dev_handler.i2c, arg_ptr, arg_int, -1));
            } else {
                ESP_LOGE(TAG, "Unknown Bus Type: %d", context->bus_type);
                abort();
            }
            break;
        case U8X8_MSG_BYTE_START_TRANSFER:
            u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_enable_level);
            break;
        case U8X8_MSG_BYTE_END_TRANSFER:
            u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_disable_level);
            break;
        default: ;
    }
    return 0;
}

uint8_t u8g2_idf_adapter_gpio_and_delay_cb(u8x8_t *u8x8, const uint8_t msg, const uint8_t arg_int, void *arg_ptr) {
    const u8g2_idf_adapter_t *context = get_context(u8x8);
    switch (msg) {
        case U8X8_MSG_GPIO_AND_DELAY_INIT:
            // 初始化 GPIO
            uint64_t bitmask = 0;
            if (context->bus_type == U8G2_IDF_ADAPTER_SPI) {
                if (context->config.bus.spi.cs != GPIO_NUM_NC) bitmask |= BIT(context->config.bus.spi.cs);
            }
            if (context->config.gpio.dc != GPIO_NUM_NC)bitmask |= BIT(context->config.gpio.dc);
            if (context->config.gpio.reset != GPIO_NUM_NC)bitmask |= BIT(context->config.gpio.reset);
            if (bitmask == 0) break;
            gpio_config_t config;
            config.pin_bit_mask = bitmask;
            config.mode = GPIO_MODE_OUTPUT;
            config.pull_up_en = GPIO_PULLUP_DISABLE;
            config.pull_down_en = GPIO_PULLDOWN_ENABLE;
            config.intr_type = GPIO_INTR_DISABLE;
            gpio_config(&config);
            u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_disable_level);
            u8x8_gpio_SetDC(u8x8, 0);
            u8x8_gpio_SetReset(u8x8, 1);
            break;
        case U8X8_MSG_GPIO_RESET:
            if (context->config.gpio.reset != GPIO_NUM_NC) gpio_set_level(context->config.gpio.reset, arg_int);
            break;
        case U8X8_MSG_GPIO_CS:
            if (context->bus_type != U8G2_IDF_ADAPTER_SPI) break;
            if (context->config.bus.spi.cs != GPIO_NUM_NC) gpio_set_level(context->config.bus.spi.cs, arg_int);
            break;
        case U8X8_MSG_GPIO_I2C_CLOCK:
            if (context->config.bus.i2c.scl != GPIO_NUM_NC) gpio_set_level(context->config.bus.i2c.scl, arg_int);
            break;
        case U8X8_MSG_GPIO_I2C_DATA:
            if (context->config.bus.i2c.sda != GPIO_NUM_NC) gpio_set_level(context->config.bus.i2c.sda, arg_int);
            break;
        case U8X8_MSG_DELAY_MILLI:
            vTaskDelay(pdMS_TO_TICKS(arg_int));
            break;
        default:
            break;
    }
    return 0;
}
