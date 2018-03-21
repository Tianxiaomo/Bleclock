/**
 *  @filename   :   epdif.c
 *  @brief      :   Implements EPD interface functions
 *                  Users have to implement all the functions in epdif.cpp
 *  @author     :   Yehui from Waveshare
 *
 *  Copyright (C) Waveshare     July 7 2017
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documnetation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to  whom the Software is
 * furished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "epdif.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "soc/gpio_struct.h"
#include "driver/gpio.h"
//#include "main.h"

//extern spi_device_handle_t hspi1;

static spi_device_handle_t spi;

//EPD_Pin epd_cs_pin = {
//  SPI_CS_GPIO_Port,
//  SPI_CS_Pin,
//};
//
//EPD_Pin epd_rst_pin = {
//  RST_GPIO_Port,
//  RST_Pin,
//};
//
//EPD_Pin epd_dc_pin = {
//  DC_GPIO_Port,
//  DC_Pin,
//};
//
//EPD_Pin epd_busy_pin = {
//  BUSY_GPIO_Port,
//  BUSY_Pin,
//};
#define PIN_NUM_MISO 25
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK  19
#define PIN_NUM_CS   22

#define PIN_NUM_DC   21
#define PIN_NUM_RST  18
#define PIN_NUM_BUSY 5
int pins[4];

void EpdDigitalWriteCallback(int pin_num, int value) {
	  gpio_set_level(pin_num,value);

//	if (value == HIGH) {
//    HAL_GPIO_WritePin((GPIO_TypeDef*)pins[pin_num].port, pins[pin_num].pin, GPIO_PIN_SET);
//  } else {
//    HAL_GPIO_WritePin((GPIO_TypeDef*)pins[pin_num].port, pins[pin_num].pin, GPIO_PIN_RESET);
//  }
}

int EpdDigitalReadCallback(int pin_num) {
	return gpio_get_level(pin_num);
	//  if (HAL_GPIO_ReadPin(pins[pin_num].port, pins[pin_num].pin) == GPIO_PIN_SET) {
//  if(gpio_get_level(pin_num)){
//    return HIGH;
//  } else {
//    return LOW;
//  }
}

void EpdDelayMsCallback(unsigned int delaytime) {
//  HAL_Delay(delaytime);
	vTaskDelay(delaytime / portTICK_RATE_MS);
}

void EpdSpiTransferCallback(unsigned char data) {
//  HAL_GPIO_WritePin((GPIO_TypeDef*)pins[CS_PIN].port, pins[CS_PIN].pin, GPIO_PIN_RESET);
//  HAL_SPI_Transmit(&hspi1, &data, 1, 1000);
//  HAL_GPIO_WritePin((GPIO_TypeDef*)pins[CS_PIN].port, pins[CS_PIN].pin, GPIO_PIN_SET);
}

void EpdSpiTransferCallback_cd(unsigned char c_d,unsigned char command) {
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                     //Command is 8 bits
    t.tx_buffer=&command;               //The data is the cmd itself
    if(c_d){
    	t.user=(void*)1;                //D/C needs to be set to 0
    }else{
    	t.user=(void*)0;                //D/C needs to be set to 0
    }

    ret=spi_device_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}


int EpdInitCallback(void) {
//  pins[CS_PIN] = epd_cs_pin;
//  pins[RST_PIN] = epd_rst_pin;
//  pins[DC_PIN] = epd_dc_pin;
//  pins[BUSY_PIN] = epd_busy_pin;
  pins[CS_PIN]  = PIN_NUM_CS;
  pins[RST_PIN] = PIN_NUM_RST;
  pins[DC_PIN]  = PIN_NUM_DC;
  pins[BUSY_PIN] = PIN_NUM_BUSY;
  return 0;
}

//this function set command/data, one IO
void spi_cd_transfer_callback(spi_transaction_t *t)
{
    int dc=(int)t->user;
    gpio_set_level(PIN_NUM_DC, dc);
}
//set spi miso mosi clk cs,three IO
void Epaper_init(void){
	esp_err_t ret;
//	spi_device_handle_t spi;
	spi_bus_config_t buscfg={
		.miso_io_num=PIN_NUM_MISO,
		.mosi_io_num=PIN_NUM_MOSI,
		.sclk_io_num=PIN_NUM_CLK,
		.quadwp_io_num=-1,
		.quadhd_io_num=-1
	};
	spi_device_interface_config_t devcfg={
		.clock_speed_hz=10000000,               //Clock out at 1 MHz
		.mode=0,                                //SPI mode 0
		.spics_io_num=PIN_NUM_CS,               //CS pin
		.queue_size=7,                          //We want to be able to queue 7 transactions at a time
		.pre_cb=spi_cd_transfer_callback,  		//Specify pre-transfer callback to handle D/C line
	};
	//Initialize the SPI bus
	ret=spi_bus_initialize(HSPI_HOST, &buscfg, 1);
	assert(ret==ESP_OK);
	//Attach the LCD to the SPI bus
	ret=spi_bus_add_device(HSPI_HOST, &devcfg, &spi);
	assert(ret==ESP_OK);
    //Initialize non-SPI GPIOs
    gpio_set_direction(PIN_NUM_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_RST, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_BUSY, GPIO_MODE_INPUT);
    //Reset the display
    gpio_set_level(PIN_NUM_RST, 0);
    vTaskDelay(100 / portTICK_RATE_MS);
    gpio_set_level(PIN_NUM_RST, 1);
    vTaskDelay(100 / portTICK_RATE_MS);
}



