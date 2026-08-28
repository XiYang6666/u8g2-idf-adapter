#include "u8g2_idf_adapter.h"

#include "esp_check.h"

#include "../constants.h"
#include "./bus.h"

esp_err_t init_spi_device(
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
        .mode = config->bus.spi.mode,
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
        (unsigned long) config->frequency
    );

    return ESP_OK;
}
