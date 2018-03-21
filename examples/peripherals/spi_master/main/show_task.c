/*
 * show_task.c
 *
 *  Created on: 2018Äê3ÔÂ6ÈÕ
 *      Author: MO
 */

#include "show_task.h"

EPD epd;
unsigned char* frame_buffer;
Paint paint;

void redraw(void * arg)
{
	int x = 200;
    int y = 150;
    int minute_radius = 100;
    int hour_radius = 60;
    int hour = 0;
    int minute=11;
    int temp_x;
    int temp_y;

    printf("Redraw start!\n");
    vTaskDelay(6000 / portTICK_PERIOD_MS);
    for(;;){
    	if(minute%12 == 0){
			temp_x = (int)(sin((hour-1)*6*PI/180) * hour_radius);
			temp_y = (int)(cos((hour-1)*6*PI/180) * hour_radius);
			temp_x = x + temp_x;
			temp_y = y - temp_y;
			Paint_DrawLine(&paint, x, y, temp_x , temp_y, UNCOLORED);

			temp_x = (int)(sin(hour*6*PI/180) * hour_radius);
			temp_y = (int)(cos(hour*6*PI/180) * hour_radius);
			temp_x = x + temp_x;
			temp_y = y - temp_y;
			Paint_DrawLine(&paint, x, y, temp_x , temp_y, COLORED);
			hour++;
			if(hour == 60)hour=0;
    	}
        temp_x = (int)(sin(minute*6*PI/180) * minute_radius);
        temp_y = (int)(cos(minute*6*PI/180) * minute_radius);
        printf("111111111111temp_x = %d , temp_y = %d , minute = %d,hour = %d\n",temp_x,temp_y,minute,hour);
        temp_x = x + temp_x;
        temp_y = y - temp_y;
        printf("222222222222temp_x = %d , temp_y = %d , minute = %d\n",temp_x,temp_y,minute);
        Paint_DrawLine(&paint, x, y, temp_x , temp_y, COLORED);
        EPD_DisplayFrame(&epd, frame_buffer);
        minute ++;
        if(minute == 60){
        	minute = 0;
        }
        printf("temp_x = %d , temp_y = %d , minute = %d\n",temp_x,temp_y,minute);
        vTaskDelay(60000 / portTICK_PERIOD_MS);
        Paint_DrawLine(&paint, x, y, temp_x , temp_y, UNCOLORED);

    }
    vTaskDelete(NULL);
}


void display_init(){

	  if (EPD_Init(&epd) != 0) {
	    printf("e-Paper init failed\n");
	    return ;
	  }
	  printf("e-Paper init seccuss\n");
	  frame_buffer = (unsigned char*)malloc(EPD_WIDTH * EPD_HEIGHT / 8);
	  Paint_Init(&paint, frame_buffer, epd.width, epd.height);
	  Paint_Clear(&paint, UNCOLORED);
	  Paint_DrawCircle(&paint, 200, 150, 120, COLORED);
	  Paint_DrawCircle(&paint, 200, 150, 121, COLORED);
	  Paint_DrawCircle(&paint, 200, 150, 122, COLORED);
	  EPD_DisplayFrame(&epd, frame_buffer);
	  xTaskCreate(redraw, "spp_cmd_task", 2048, NULL, 10, NULL);
}




