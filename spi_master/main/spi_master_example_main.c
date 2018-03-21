#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "https_task.h"
#include "epd4in2.h"
#include "epdif.h"
#include "epdpaint.h"
#include "imagedata.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "soc/gpio_struct.h"
#include "driver/gpio.h"
#include "show_task.h"
#include "ble_task.h"
#include "https_request.h"
#include "esp_spi_flash.h"


void app_main(){
	ble_init();
	display_init();
	//https_request();
}
