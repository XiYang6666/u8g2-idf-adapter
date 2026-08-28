#include "u8g2_idf_adapter.h"

#include "esp_bit_defs.h"
#include "esp_check.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "./utils/utils.h"

// u8g2 回调
uint8_t u8g2_idf_adapter_byte_cb(u8x8_t *u8x8, const uint8_t msg, const uint8_t arg_int, void *arg_ptr) {
    u8g2_idf_adapter_t *context = get_context(u8x8);
    switch (msg) {
        case U8X8_MSG_BYTE_INIT:
            u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_disable_level);
            break;
        case U8X8_MSG_BYTE_SET_DC:
            u8x8_gpio_SetDC(u8x8, arg_int);
            break;
        case U8X8_MSG_BYTE_START_TRANSFER:
            u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_enable_level);
            context->tx_buf_used = 0;
            break;
        case U8X8_MSG_BYTE_SEND:
            if (context->config.tx_buf == nullptr) {
                ESP_ERROR_CHECK(send_raw(context, arg_ptr, arg_int));
                break;
            }
            if (arg_int > context->config.tx_buf_size) {
                // Chunk bigger than the whole buffer: flush what's pending
                // (to preserve ordering) then send this chunk directly.
                ESP_ERROR_CHECK(flush_tx_buffer(context));
                ESP_ERROR_CHECK(send_raw(context, arg_ptr, arg_int));
            } else {
                if (context->tx_buf_used + arg_int > context->config.tx_buf_size) {
                    // Not enough room left: flush to make space.
                    ESP_ERROR_CHECK(flush_tx_buffer(context));
                }
                memcpy(context->config.tx_buf + context->tx_buf_used, arg_ptr, arg_int);
                context->tx_buf_used += arg_int;
            }
            break;
        case U8X8_MSG_BYTE_END_TRANSFER:
            ESP_ERROR_CHECK(flush_tx_buffer(context));
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
                if (context->config.bus.spi.cs != GPIO_NUM_NC) bitmask |= BIT64(context->config.bus.spi.cs);
            }
            if (context->config.gpio.dc != GPIO_NUM_NC)bitmask |= BIT64(context->config.gpio.dc);
            if (context->config.gpio.reset != GPIO_NUM_NC)bitmask |= BIT64(context->config.gpio.reset);
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
