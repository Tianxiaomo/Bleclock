/*
 * https_task.c
 *
 *  Created on: 2018Äê3ÔÂ13ÈÕ
 *      Author: MO
 */

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event_loop.h"
#include "esp_pm.h"
#include "nvs_flash.h"
#include "ssl_task.h"

/*set the ssid and password via "make menuconfig"*/
#define DEFAULT_SSID "12"
#define DEFAULT_PWD "123456789"

#if CONFIG_POWER_SAVE_MODEM
#define DEFAULT_PS_MODE WIFI_PS_MODEM
#elif CONFIG_POWER_SAVE_NONE
#define DEFAULT_PS_MODE WIFI_PS_NONE
#else
#define DEFAULT_PS_MODE WIFI_PS_NONE
#endif /*CONFIG_POWER_SAVE_MODEM*/


static const char *TAG = "power_save";


static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
		case SYSTEM_EVENT_STA_START:
			ESP_LOGI(TAG, "SYSTEM_EVENT_STA_START\n");
			ESP_ERROR_CHECK(esp_wifi_connect());
			break;
		case SYSTEM_EVENT_STA_GOT_IP:
			ESP_LOGI(TAG, "SYSTEM_EVENT_STA_GOT_IP\n");
			ESP_LOGI(TAG, "got ip:%s\n",
			ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
			break;
		case SYSTEM_EVENT_STA_DISCONNECTED:
			ESP_LOGI(TAG, "SYSTEM_EVENT_STA_DISCONNECTED\n");
			ESP_ERROR_CHECK(esp_wifi_connect());
			break;
				/**< ESP32 finish scanning AP */
		case SYSTEM_EVENT_SCAN_DONE:
			ESP_LOGI(TAG, "WiFi started\n");
			break;
		case SYSTEM_EVENT_STA_STOP:
			ESP_LOGI(TAG,"ESP32 station stop\n");
			break;
		case SYSTEM_EVENT_STA_CONNECTED:
			ESP_LOGI(TAG,"ESP32 station connected to AP\n");
			user_conn_init();
			break;
		case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
			ESP_LOGI(TAG,"the auth mode of AP connected by ESP32 station changed\n");
			break;
		case SYSTEM_EVENT_STA_LOST_IP:
			ESP_LOGI(TAG,"ESP32 station lost IP and the IP is reset to 0\n");
			break;
		case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
			ESP_LOGI(TAG,"ESP32 station wps succeeds in enrollee mode\n");
			break;
		case SYSTEM_EVENT_STA_WPS_ER_FAILED:
			ESP_LOGI(TAG,"ESP32 station wps fails in enrollee mode\n");
			break;
		case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
			ESP_LOGI(TAG,"ESP32 station wps timeout in enrollee mode\n");
			break;
		case SYSTEM_EVENT_STA_WPS_ER_PIN:
			ESP_LOGI(TAG,"ESP32 station wps pin code in enrollee mode\n");
			break;
		case SYSTEM_EVENT_AP_START:
			ESP_LOGI(TAG,"ESP32 soft-AP start\n");
			break;
		case SYSTEM_EVENT_AP_STOP:
			ESP_LOGI(TAG,"ESP32 soft-AP stop\n");
			break;
		case SYSTEM_EVENT_AP_STACONNECTED:
			ESP_LOGI(TAG,"a station connected to ESP32 soft-AP\n");
			break;
		case SYSTEM_EVENT_AP_STADISCONNECTED:
			ESP_LOGI(TAG,"a station disconnected from ESP32 soft-AP\n");
			break;
		case SYSTEM_EVENT_AP_PROBEREQRECVED:
			ESP_LOGI(TAG,"Receive probe request packet in soft-AP interface\n");
			break;
		case SYSTEM_EVENT_GOT_IP6:
			ESP_LOGI(TAG,"ESP32 station or ap or ethernet interface v6IP addr is preferred\n");
			break;
		case SYSTEM_EVENT_ETH_START:
			ESP_LOGI(TAG,"ESP32 ethernet start\n");
			break;
		case SYSTEM_EVENT_ETH_STOP:
			ESP_LOGI(TAG,"ESP32 ethernet stop\n");
			break;
		case SYSTEM_EVENT_ETH_CONNECTED:
			ESP_LOGI(TAG,"ESP32 ethernet phy link up\n");
			break;
		case SYSTEM_EVENT_ETH_DISCONNECTED:
			ESP_LOGI(TAG,"ESP32 ethernet phy link down \n");
			break;
		case SYSTEM_EVENT_ETH_GOT_IP:
			ESP_LOGI(TAG,"ESP32 ethernet got IP from connected AP\n");
			break;
		default:
			break;
	}
    return ESP_OK;
}

/*init wifi as sta and set power save mode*/
static void wifi_power_save(void)
{
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_config_t wifi_config = {
	.sta = {
	    .ssid = DEFAULT_SSID,
	    .password = DEFAULT_PWD,
	},
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "esp_wifi_set_ps().");
    esp_wifi_set_ps(DEFAULT_PS_MODE);
}

void https_task()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

#if CONFIG_PM_ENABLE
    // Configure dynamic frequency scaling: maximum frequency is set in sdkconfig,
    // minimum frequency is XTAL.
    rtc_cpu_freq_t max_freq;
    rtc_clk_cpu_freq_from_mhz(CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ, &max_freq);
    esp_pm_config_esp32_t pm_config = {
            .max_cpu_freq = max_freq,
            .min_cpu_freq = RTC_CPU_FREQ_XTAL
    };
    ESP_ERROR_CHECK( esp_pm_configure(&pm_config) );
#endif // CONFIG_PM_ENABLE

    wifi_power_save();
}
