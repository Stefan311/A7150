#include "esp_ota_ops.h"
#include "wlan.h"
#include "nvs_flash.h"


#define _NVS_SETTING_SSID	"SSID"
#define _NVS_SETTING_PASSWD	"PASSWD"
#define _NVS_SETTING_WLAN_MODE "WLANMODE"

nvs_handle_t sys_nvs_handle;

// NVS Partition initialisieren und Daten vom Flash laden, wenn vorhanden
void setup_flash() 
{
	// Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) 
	{
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );

	err = nvs_open("storage", NVS_READWRITE, &sys_nvs_handle);
    if (err != ESP_OK) 
	{
        printf("Fehler (%s) beim öffnen des NVS handle!\n", esp_err_to_name(err));
    } 
}

// Einstellungen vom Flash laden
void restore_settings() 
{
	if (sys_nvs_handle == 0) return;

	wlan_ssid = heap_caps_malloc(64, MALLOC_CAP_DEFAULT);
	wlan_passwd = heap_caps_malloc(64, MALLOC_CAP_DEFAULT);

	size_t i=64;
	nvs_get_str(sys_nvs_handle, _NVS_SETTING_SSID, wlan_ssid, &i);
	i=64;
	nvs_get_str(sys_nvs_handle, _NVS_SETTING_PASSWD, wlan_passwd, &i);
	nvs_get_u8(sys_nvs_handle, _NVS_SETTING_WLAN_MODE, &wlan_mode);
}

// Einstellungen im Flash sichern
void write_settings() 
{
	if (sys_nvs_handle == 0) return;

	nvs_set_str(sys_nvs_handle, _NVS_SETTING_SSID, wlan_ssid);
	nvs_set_str(sys_nvs_handle, _NVS_SETTING_PASSWD, wlan_passwd);
	nvs_set_u8(sys_nvs_handle, _NVS_SETTING_WLAN_MODE, wlan_mode);
	printf("Einstellungen gespeichert.\n");
}
