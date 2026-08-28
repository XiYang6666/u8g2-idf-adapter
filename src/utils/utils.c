#include "u8g2_idf_adapter.h"

#include "esp_log.h"

#include "../constants.h"
#include "./utils.h"

u8g2_idf_adapter_t *get_context(u8x8_t *u8x8) {
#ifdef U8G2_IDF_ADAPTER_U8X8_USER_PTR_CONTEXT
    return *context = u8x8.user_ptr;
#else
    // return __containerof(__containerof(u8x8,u8g2_t,u8x8), u8g2_idf_adapter_t, u8g2);
    return (u8g2_idf_adapter_t *) u8x8;
#endif
}

esp_err_t send_raw(const u8g2_idf_adapter_t *context, const void *data, const size_t len) {
    if (context->bus_type == U8G2_IDF_ADAPTER_SPI) {
        spi_transaction_t t = {};
        t.length = len * 8;
        t.tx_buffer = data;
        return spi_device_transmit(context->dev_handler.spi, &t);
    }
    if (context->bus_type == U8G2_IDF_ADAPTER_I2C) {
        return i2c_master_transmit(context->dev_handler.i2c, data, len, -1);
    }
    ESP_LOGE(TAG, "Unknown Bus Type: %d", context->bus_type);
    abort();
}

esp_err_t flush_tx_buffer(u8g2_idf_adapter_t *context) {
    if (context->config.tx_buf == nullptr || context->tx_buf_used == 0)return ESP_OK;
    const esp_err_t err = send_raw(context, context->config.tx_buf, context->tx_buf_used);
    context->tx_buf_used = 0;
    return err;
}
