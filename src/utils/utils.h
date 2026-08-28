#pragma once

u8g2_idf_adapter_t *get_context(u8x8_t *u8x8);

esp_err_t send_raw(const u8g2_idf_adapter_t *context, const void *data, const size_t len);

esp_err_t flush_tx_buffer(u8g2_idf_adapter_t *context);
