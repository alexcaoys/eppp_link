/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <string.h>
#include <stdint.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_check.h"
#include "esp_event.h"
#include "eppp_link.h"
#include "eppp_transport.h"
#include "driver/usb_serial_jtag.h"

#define TAG "eppp_jtag"

struct eppp_jtag {
    struct eppp_handle parent;
};

/* Maximum size of a packet sent over JTAG, including header and payload */
#define JTAG_BUF_SIZE   (1500)

static esp_err_t transmit(void *h, void *buffer, size_t len)
{
    ESP_LOG_BUFFER_HEXDUMP("ppp_jtag_send", buffer, len, ESP_LOG_DEBUG);
    usb_serial_jtag_write_bytes(buffer, len, portMAX_DELAY);

    return ESP_OK;
}

static esp_err_t init_jtag(struct eppp_jtag *h, struct eppp_config_jtag_s *config)
{
    usb_serial_jtag_driver_config_t usb_serial_jtag_config = {};
    usb_serial_jtag_config.tx_buffer_size = config->tx_buffer_size;
    usb_serial_jtag_config.rx_buffer_size = config->rx_buffer_size;

    ESP_RETURN_ON_ERROR(usb_serial_jtag_driver_install(&usb_serial_jtag_config), TAG, "Failed to install JTAG");
    return ESP_OK;
}

static void deinit_jtag()
{
    usb_serial_jtag_driver_uninstall();
}

esp_err_t eppp_perform(esp_netif_t *netif)
{
    struct eppp_handle *handle = esp_netif_get_io_driver(netif);
    struct eppp_jtag *h = __containerof(handle, struct eppp_jtag, parent);
    if (h->parent.stop) {
        return ESP_ERR_TIMEOUT;
    }

    static uint8_t buffer[JTAG_BUF_SIZE] = {};
    size_t len = usb_serial_jtag_read_bytes(buffer, JTAG_BUF_SIZE, 0);

    if (len > 0) {
        ESP_LOG_BUFFER_HEXDUMP("ppp_jtag_recv", buffer, len, ESP_LOG_DEBUG);
        esp_netif_receive(netif, buffer, len, NULL);
    }

    return ESP_OK;
}


static esp_err_t post_attach(esp_netif_t *esp_netif, void *args)
{
    eppp_transport_handle_t h = (eppp_transport_handle_t)args;
    ESP_RETURN_ON_FALSE(h, ESP_ERR_INVALID_ARG, TAG, "Transport handle cannot be null");
    h->base.netif = esp_netif;

    esp_netif_driver_ifconfig_t driver_ifconfig = {
        .handle =  h,
        .transmit = transmit,
    };

    ESP_RETURN_ON_ERROR(esp_netif_set_driver_config(esp_netif, &driver_ifconfig), TAG, "Failed to set driver config");
    ESP_LOGI(TAG, "EPPP JTAG transport attached to EPPP netif %s", esp_netif_get_desc(esp_netif));
    return ESP_OK;
}

eppp_transport_handle_t eppp_jtag_init(struct eppp_config_jtag_s *config)
{
    __attribute__((unused)) esp_err_t ret = ESP_OK;
    ESP_RETURN_ON_FALSE(config, NULL, TAG, "Config cannot be null");
    struct eppp_jtag *h = calloc(1, sizeof(struct eppp_jtag));
    ESP_RETURN_ON_FALSE(h, NULL, TAG, "Failed to allocate eppp_handle");

    h->parent.base.post_attach = post_attach;
    ESP_GOTO_ON_ERROR(init_jtag(h, config), err, TAG, "Failed to init JTAG");
    return &h->parent;
err:
    free(h);
    return NULL;
}

void eppp_jtag_deinit(eppp_transport_handle_t handle)
{
    struct eppp_jtag *h = __containerof(handle, struct eppp_jtag, parent);
    deinit_jtag();
    free(h);
}
