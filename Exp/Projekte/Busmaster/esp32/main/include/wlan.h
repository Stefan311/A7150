#pragma once
#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"

void wifi_scan(uint16_t* ap_count, wifi_ap_record_t* ap_list);
void setup_wlan(uint8_t new_mode);

extern char* wlan_ssid;
extern char* wlan_passwd;
extern uint8_t wlan_mode;

