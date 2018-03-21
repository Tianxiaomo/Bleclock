/*
 * show_task.c
 *
 *  Created on: 2018年3月6日
 *      Author: MO
 */
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_sleep.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "apps/sntp/sntp.h"

#include "show_task.h"

EPD epd;
unsigned char* frame_buffer;
RTC_DATA_ATTR static Paint paint;
static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;
static const char *TAG = "SHOW";
RTC_DATA_ATTR static int boot_count = 0;
RTC_DATA_ATTR static char first = 1;

#define EXAMPLE_WIFI_SSID  "LS-PC"
#define EXAMPLE_WIFI_PASS  "092413131"

typedef struct coordinate {
    int x;
    int y;
} Coordinate;

//static void Draw_Time(Coordinate timeinfo);
static void show_init(void);
static void obtain_time(void);
static void initialize_sntp(void);
static void initialise_wifi(void);
static esp_err_t event_handler(void *ctx, system_event_t *event);


void Draw_Min(Paint* paint, int x0, int y0, int x1, int y1, int color){
	int dx,dy;
	if(x1>x0){dx = x1-x0;}
	else{dx = x0 -x1;}

	if(y1>y0){dy = y1 - y0;}
	else{dy = y0 -y1;}

	if(dx > dy){
		Paint_DrawLine(paint,x0,y0-1,x1,y1-1,color);
		Paint_DrawLine(paint,x0,y0,x1,y1,color);
		Paint_DrawLine(paint,x0,y0+1,x1,y1+1,color);
	}else{
		Paint_DrawLine(paint,x0-1,y0,x1-1,y1,color);
		Paint_DrawLine(paint,x0,y0,x1,y1,color);
		Paint_DrawLine(paint,x0+1,y0,x1+1,y1,color);
	}
}

void Draw_Hour(Paint* paint, int x0, int y0, int x1, int y1, int color){
	int dx,dy;
	if(x1>x0){dx = x1-x0;}
	else{dx = x0 -x1;}

	if(y1>y0){dy = y1 - y0;}
	else{dy = y0 -y1;}

	if(dx > dy){
		Paint_DrawLine(paint,x0,y0-2,x1,y1-2,color);
		Paint_DrawLine(paint,x0,y0-1,x1,y1-1,color);
		Paint_DrawLine(paint,x0,y0,x1,y1,color);
		Paint_DrawLine(paint,x0,y0+1,x1,y1+1,color);
		Paint_DrawLine(paint,x0,y0+2,x1,y1+2,color);
	}else{
		Paint_DrawLine(paint,x0-2,y0,x1-2,y1,color);
		Paint_DrawLine(paint,x0-1,y0,x1-1,y1,color);
		Paint_DrawLine(paint,x0,y0,x1,y1,color);
		Paint_DrawLine(paint,x0+1,y0,x1+1,y1,color);
		Paint_DrawLine(paint,x0+2,y0,x1+2,y1,color);
	}
}

static void Draw_Time(struct tm* timeinfo)
{
	int x = 200;
    int y = 150;
    int minute_radius = 100;
    int hour_radius = 60;
    int hour = timeinfo->tm_hour;
    int min= timeinfo->tm_min;
    static Coordinate hour_now,hour_old,min_now,min_old;

    printf("Redraw start!\n");

    if(first){
    	Draw_Hour(&paint, x, y, hour_old.x , hour_old.y, UNCOLORED);
		hour_now.x = (int)(sin( (hour*30 + (min/12)*6) *PI/180) * hour_radius);
		hour_now.y = (int)(cos( (hour*30 + (min/12)*6) *PI/180) * hour_radius);
		printf("hour_now=%d:%d\n",hour_now.x,hour_now.y);
		hour_now.x = x + hour_now.x;
		hour_now.y = y - hour_now.y;
		hour_old = hour_now;
		Draw_Hour(&paint, x, y, hour_now.x , hour_now.y, COLORED);
    	first = 0;
    }

	if(min%12 == 0){
		Draw_Hour(&paint, x, y, hour_old.x , hour_old.y, UNCOLORED);
		hour_now.x = (int)(sin( (hour*30 + (min/12)*6) *PI/180) * hour_radius);
		hour_now.y = (int)(cos( (hour*30 + (min/12)*6) *PI/180) * hour_radius);
		printf("hour_now=%d:%d\n",hour_now.x,hour_now.y);
		hour_now.x = x + hour_now.x;
		hour_now.y = y - hour_now.y;
		hour_old = hour_now;
		Draw_Hour(&paint, x, y, hour_now.x , hour_now.y, COLORED);
	}
	Draw_Min(&paint, x, y, min_old.x , min_old.y, UNCOLORED);
	min_now.x = (int)(sin(min*6*PI/180) * minute_radius);
	min_now.y = (int)(cos(min*6*PI/180) * minute_radius);
	min_now.x = x + min_now.x;
	min_now.y = y - min_now.y;
	min_old = min_now;
	Draw_Min(&paint, x, y, min_now.x , min_now.y, COLORED);

	printf("hour=%d,min=%d,hour_now=%d:%d,min_now=%d:%d\n",hour,min,hour_now.x,hour_now.y,min_now.x,min_now.y);

	EPD_DisplayFrame(&epd, frame_buffer);

//    vTaskDelete(NULL);
}


static void show_init(void){
	  if (EPD_Init(&epd) != 0) {
	    printf("e-Paper init failed\n");
	    return ;
	  }
	  printf("e-Paper init seccuss\n");
	  frame_buffer = (unsigned char*)malloc(EPD_WIDTH * EPD_HEIGHT / 8);
	  Paint_Init(&paint, frame_buffer, epd.width, epd.height);
	  Paint_Clear(&paint, UNCOLORED);
	  Paint_DrawFilledCircle(&paint, 200, 150, 123, COLORED);
	  Paint_DrawFilledCircle(&paint, 200, 150, 120, UNCOLORED);
//	  Draw_Min(&paint,100,100, 250, 250,COLORED);
	  Paint_DrawString(&paint, 10, 10,20,"今天夜间不太热也不太冷，风力不大，相信您在这样的天气条件下，应会感到比较清爽和舒适。", COLORED);
	  Paint_DrawString(&paint, 10, 100,20,"abcdefjhijklmnopqrstovwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()_+-=[]{}\\|,.<>/?`~", COLORED);
	  EPD_DisplayFrame(&epd, frame_buffer);
//	  xTaskCreate(redraw, "spp_cmd_task", 2048, NULL, 10, NULL);
}


void display_init()
{
    ++boot_count;
    show_init();
    ESP_LOGI(TAG, "Boot count: %d", boot_count);

    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    // Is time set? If not, tm_year will be (1970 - 1900).
    if (timeinfo.tm_year < (2016 - 1900)) {
        ESP_LOGI(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
        obtain_time();
        // update 'now' variable with current time
        time(&now);
    }
    char strftime_buf[64];

    setenv("TZ", "CST-8", 1);
    tzset();
    localtime_r(&now, &timeinfo);

    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "The current date/time in Shanghai is: %s", strftime_buf);
    Draw_Time(&timeinfo);

    const int deep_sleep_sec = 10;
    ESP_LOGI(TAG, "Entering deep sleep for %d seconds", deep_sleep_sec);
    esp_deep_sleep(1000000LL * deep_sleep_sec);
}

static void obtain_time(void)
{
    ESP_ERROR_CHECK( nvs_flash_init() );
    initialise_wifi();
    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,
                        false, true, portMAX_DELAY);
    initialize_sntp();

    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 10;
    while(timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
    }
    ESP_ERROR_CHECK( esp_wifi_stop());
}

static void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();
}

static void initialise_wifi(void)
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_WIFI_SSID,
            .password = EXAMPLE_WIFI_PASS,
        },
    };
    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        ESP_LOGI(TAG,"wifi connect\n");
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG,"sta got ip\n");
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        /* This is a workaround as ESP32 WiFi libs don't currently
           auto-reassociate. */
        ESP_LOGI(TAG,"wifi disconnected\n");
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        break;
    default:
        break;
    }
    return ESP_OK;
}



