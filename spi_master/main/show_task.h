/*
 * show_task.h
 *
 *  Created on: 2018Äê3ÔÂ6ÈÕ
 *      Author: MO
 */
#ifndef SHOW_TASK_H
#define SHOW_TASK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
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
#include "weather_icon.h"

#define PI 3.141593

#define COLORED      1
#define UNCOLORED    0
void display_init(void);

#endif



