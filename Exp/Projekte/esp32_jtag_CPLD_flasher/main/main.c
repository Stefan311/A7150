#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"

#include "wlan.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "xapp058/micro.h"

#include <driver/gpio.h>
#include "soc/gpio_reg.h"
#include "soc/soc.h"
#include "esp_timer.h"

#define PIN_TDI    41
#define PIN_TDO    13
#define PIN_TCK    45
#define PIN_TMS    39

void app_main(void)
{
    gpio_config_t pincfg =
    {
        .pin_bit_mask = 1ULL<<PIN_TDO,
        .mode = GPIO_MODE_INPUT,
	      .pull_down_en = false,
    };
    ESP_ERROR_CHECK(gpio_config(&pincfg));
    pincfg.pin_bit_mask = 1ULL<<PIN_TDI | 1ULL<<PIN_TCK | 1ULL<<PIN_TMS;
    pincfg.mode = GPIO_MODE_OUTPUT;
    pincfg.pull_down_en = false;
    ESP_ERROR_CHECK(gpio_config(&pincfg));

    nvs_flash_init();
    setup_wlan(2);
}
