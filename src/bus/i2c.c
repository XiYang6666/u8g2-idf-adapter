#include "u8g2_idf_adapter.h"

#include "esp_check.h"

#include "../constants.h"
#include "./bus.h"

esp_err_t init_i2c_device(
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
        (unsigned long) config->frequency
    );

    return ESP_OK;
}
