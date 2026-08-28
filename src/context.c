#include "u8g2_idf_adapter.h"

#include "esp_log.h"
#include "esp_check.h"

#include "./constants.h"
#include "./bus/bus.h"

// 初始化上下文
esp_err_t u8g2_idf_adapter_init(
    u8g2_idf_adapter_t *self,
    const u8g2_idf_adapter_bus_type bus_type,
    const u8g2_idf_adapter_config_t *config
) {
    if ((config->tx_buf == nullptr) != (config->tx_buf_size == 0)) {
        ESP_LOGE(TAG, "tx_buf and tx_buf_size must both be set, or both left unset");
        return ESP_ERR_INVALID_ARG;
    }

    self->config = *config;
    self->bus_type = bus_type;
    self->tx_buf_used = 0;
    switch (bus_type) {
        case U8G2_IDF_ADAPTER_SPI:
            spi_device_handle_t spi_handler;
            ESP_RETURN_ON_ERROR(init_spi_device(&spi_handler, config), TAG, "Init SPI Device Failed");
            self->dev_handler.spi = spi_handler;
            break;
        case U8G2_IDF_ADAPTER_I2C:
            i2c_master_dev_handle_t i2c_handle;
            ESP_RETURN_ON_ERROR(init_i2c_device(&i2c_handle, config), TAG, "Init I2C Device Failed");
            self->dev_handler.i2c = i2c_handle;
            break;
    };

    return ESP_OK;
}

esp_err_t u8g2_idf_adapter_init_by_spi_device(
    u8g2_idf_adapter_t *self,
    const u8g2_idf_adapter_config_t *config,
    const spi_device_handle_t device
) {
    if ((config->tx_buf == nullptr) != (config->tx_buf_size == 0)) {
        ESP_LOGE(TAG, "tx_buf and tx_buf_size must both be set, or both left unset");
        return ESP_ERR_INVALID_ARG;
    }
    self->config = *config;
    self->bus_type = U8G2_IDF_ADAPTER_SPI;
    self->tx_buf_used = 0;
    self->dev_handler.spi = device;
    return ESP_OK;
}

esp_err_t u8g2_idf_adapter_init_by_i2c_device(
    u8g2_idf_adapter_t *self,
    const u8g2_idf_adapter_config_t *config,
    const i2c_master_dev_handle_t device
) {
    if ((config->tx_buf == nullptr) != (config->tx_buf_size == 0)) {
        ESP_LOGE(TAG, "tx_buf and tx_buf_size must both be set, or both left unset");
        return ESP_ERR_INVALID_ARG;
    }
    self->config = *config;
    self->bus_type = U8G2_IDF_ADAPTER_I2C;
    self->tx_buf_used = 0;
    self->dev_handler.i2c = device;
    return ESP_OK;
}
