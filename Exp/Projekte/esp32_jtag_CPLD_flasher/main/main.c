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
#include "xapp058/micro.h"

#include <driver/gpio.h>
#include "soc/gpio_reg.h"
#include "soc/soc.h"
#include "esp_timer.h"
#include "cmdline.h"
#include "pthread.h"

#define PIN_TDI    41
#define PIN_TDO    13
#define PIN_TCK    45
#define PIN_TMS    39

void cmdline_task(void*)
{
    while (1)
    {
        run_cmdline();
        usleep(10000);
    }
}

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

    setup_flash();
    restore_settings();
    setup_wlan(wlan_mode);
    xTaskCreatePinnedToCore(cmdline_task,"cmdline_task",5000,NULL,0,NULL,1);
}
