#pragma once

esp_err_t init_spi_device(
    spi_device_handle_t *device_handle,
    const u8g2_idf_adapter_config_t *config
);

esp_err_t init_i2c_device(
    i2c_master_dev_handle_t *device_handle,
    const u8g2_idf_adapter_config_t *config
);
