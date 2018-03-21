/**
 *  @filename   :   epdpaint.c
 *  @brief      :   Paint tools
 *  @author     :   Yehui from Waveshare
 *  
 *  Copyright (C) Waveshare     July 28 2017
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

#include "epdpaint.h"
#include <stdio.h>
#include "esp_spi_flash.h"


void Paint_Init(Paint* paint, unsigned char* image, int width, int height) {
    paint->rotate = ROTATE_0;
    paint->image = image;
    /* 1 byte = 8 pixels, so the width should be the multiple of 8 */
    paint->width = width % 8 ? width + 8 - (width % 8) : width;
    paint->height = height;
}

/**
 *  @brief: clear the image
 */
void Paint_Clear(Paint* paint, int colored) {
    for (int x = 0; x < paint->width; x++) {
        for (int y = 0; y < paint->height; y++) {
            Paint_DrawAbsolutePixel(paint, x, y, colored);
        }
    }
}

/**
 *  @brief: this draws a pixel by absolute coordinates.
 *          this function won't be affected by the rotate parameter.
 */
void Paint_DrawAbsolutePixel(Paint* paint, int x, int y, int colored) {
    if (x < 0 || x >= paint->width || y < 0 || y >= paint->height) {
        return;
    }
    if (IF_INVERT_COLOR) {
        if (colored) {
            paint->image[(x + y * paint->width) / 8] |= 0x80 >> (x % 8);
        } else {
            paint->image[(x + y * paint->width) / 8] &= ~(0x80 >> (x % 8));
        }
    } else {
        if (colored) {
            paint->image[(x + y * paint->width) / 8] &= ~(0x80 >> (x % 8));
        } else {
            paint->image[(x + y * paint->width) / 8] |= 0x80 >> (x % 8);
        }
    }
}

/**
 *  @brief: Getters and Setters
 */
unsigned char* Paint_GetImage(Paint* paint) {
    return paint->image;
}

int Paint_GetWidth(Paint* paint) {
    return paint->width;
}

void Paint_SetWidth(Paint* paint, int width) {
    paint->width = width % 8 ? width + 8 - (width % 8) : width;
}

int Paint_GetHeight(Paint* paint) {
    return paint->height;
}

void Paint_SetHeight(Paint* paint, int height) {
    paint->height = height;
}

int Paint_GetRotate(Paint* paint) {
    return paint->rotate;
}

void Paint_SetRotate(Paint* paint, int rotate){
    paint->rotate = rotate;
}

/**
 *  @brief: this draws a pixel by the coordinates
 */
void Paint_DrawPixel(Paint* paint, int x, int y, int colored) {
    int point_temp;
    if (paint->rotate == ROTATE_0) {
        if(x < 0 || x >= paint->width || y < 0 || y >= paint->height) {
            return;
        }
        Paint_DrawAbsolutePixel(paint, x, y, colored);
    } else if (paint->rotate == ROTATE_90) {
        if(x < 0 || x >= paint->height || y < 0 || y >= paint->width) {
          return;
        }
        point_temp = x;
        x = paint->width - y;
        y = point_temp;
        Paint_DrawAbsolutePixel(paint, x, y, colored);
    } else if (paint->rotate == ROTATE_180) {
        if(x < 0 || x >= paint->width || y < 0 || y >= paint->height) {
          return;
        }
        x = paint->width - x;
        y = paint->height - y;
        Paint_DrawAbsolutePixel(paint, x, y, colored);
    } else if (paint->rotate == ROTATE_270) {
        if(x < 0 || x >= paint->height || y < 0 || y >= paint->width) {
          return;
        }
        point_temp = x;
        x = y;
        y = paint->height - point_temp;
        Paint_DrawAbsolutePixel(paint, x, y, colored);
    }
}

/**
 *  @brief: this draws a charactor on the frame buffer but not refresh
 */
void Paint_DrawCharAt(Paint* paint, int x, int y, char ascii_char, sFONT* font, int colored) {
    int i, j;
    unsigned int char_offset = (ascii_char - ' ') * font->Height * (font->Width / 8 + (font->Width % 8 ? 1 : 0));
    const unsigned char* ptr = &font->table[char_offset];

    for (j = 0; j < font->Height; j++) {
        for (i = 0; i < font->Width; i++) {
            if (*ptr & (0x80 >> (i % 8))) {
                Paint_DrawPixel(paint, x + i, y + j, colored);
            }
            if (i % 8 == 7) {
                ptr++;
            }
        }
        if (font->Width % 8 != 0) {
            ptr++;
        }
    }
}

/**
*  @brief: this displays a string on the frame buffer but not refresh
*/
void Paint_DrawStringAt(Paint* paint, int x, int y, const char* text, sFONT* font, int colored) {
    const char* p_text = text;
    unsigned int counter = 0;
    int refcolumn = x;
    
    /* Send the string character by character on EPD */
    while (*p_text != 0) {
        /* Display one character on EPD */
        Paint_DrawCharAt(paint, refcolumn, y, *p_text, font, colored);
        /* Decrement the column position by 16 */
        refcolumn += font->Width;
        /* Point on the next character */
        p_text++;
        counter++;
    }
}

/**
*  @brief: this draws a line on the frame buffer
*/
void Paint_DrawLine(Paint* paint, int x0, int y0, int x1, int y1, int colored) {
    /* Bresenham algorithm */
    int dx = x1 - x0 >= 0 ? x1 - x0 : x0 - x1;
    int sx = x0 < x1 ? 1 : -1;
    int dy = y1 - y0 <= 0 ? y1 - y0 : y0 - y1;
    int sy = y0 < y1 ? 1 : -1;
    int  err= dx + dy;

    if(dy == 0 && sx == 1){
    	Paint_DrawHorizontalLine(paint,x0,y0,dx,colored);
    }else if(dy == 0 && sx == -1){
    	Paint_DrawHorizontalLine(paint,x1,y1,dx,colored);
    }else if(dx == 0 && sy == 1){
    	Paint_DrawVerticalLine(paint,x0,y0,dy,colored);
    }else if(dx == 0 && sy == -1){
    	Paint_DrawVerticalLine(paint,x1,y1,dy,colored);
    }else{
		while((x0 != x1) && (y0 != y1)) {
			Paint_DrawPixel(paint, x0, y0 , colored);
			if (2 * err >= dy) {
				err += dy;
				x0 += sx;
			}
			if (2 * err <= dx) {
				err += dx;
				y0 += sy;
			}
		}
    }
}

/**
*  @brief: this draws a horizontal line on the frame buffer
*/
void Paint_DrawHorizontalLine(Paint* paint, int x, int y, int line_width, int colored) {
    int i;
    for (i = x; i < x + line_width; i++) {
        Paint_DrawPixel(paint, i, y, colored);
    }
}

/**
*  @brief: this draws a vertical line on the frame buffer
*/
void Paint_DrawVerticalLine(Paint* paint, int x, int y, int line_height, int colored) {
    int i;
    for (i = y; i < y + line_height; i++) {
        Paint_DrawPixel(paint, x, i, colored);
    }
}

/**
*  @brief: this draws a rectangle
*/
void Paint_DrawRectangle(Paint* paint, int x0, int y0, int x1, int y1, int colored) {
    int min_x, min_y, max_x, max_y;
    min_x = x1 > x0 ? x0 : x1;
    max_x = x1 > x0 ? x1 : x0;
    min_y = y1 > y0 ? y0 : y1;
    max_y = y1 > y0 ? y1 : y0;
    
    Paint_DrawHorizontalLine(paint, min_x, min_y, max_x - min_x + 1, colored);
    Paint_DrawHorizontalLine(paint, min_x, max_y, max_x - min_x + 1, colored);
    Paint_DrawVerticalLine(paint, min_x, min_y, max_y - min_y + 1, colored);
    Paint_DrawVerticalLine(paint, max_x, min_y, max_y - min_y + 1, colored);
}

/**
*  @brief: this draws a filled rectangle
*/
void Paint_DrawFilledRectangle(Paint* paint, int x0, int y0, int x1, int y1, int colored) {
    int min_x, min_y, max_x, max_y;
    int i;
    min_x = x1 > x0 ? x0 : x1;
    max_x = x1 > x0 ? x1 : x0;
    min_y = y1 > y0 ? y0 : y1;
    max_y = y1 > y0 ? y1 : y0;
    
    for (i = min_x; i <= max_x; i++) {
      Paint_DrawVerticalLine(paint, i, min_y, max_y - min_y + 1, colored);
    }
}

/**
*  @brief: this draws a circle
*/
void Paint_DrawCircle(Paint* paint, int x, int y, int radius, int colored) {
    /* Bresenham algorithm */
    int x_pos = -radius;
    int y_pos = 0;
    int err = 2 - 2 * radius;
    int e2;

    do {
        Paint_DrawPixel(paint, x - x_pos, y + y_pos, colored);
        Paint_DrawPixel(paint, x + x_pos, y + y_pos, colored);
        Paint_DrawPixel(paint, x + x_pos, y - y_pos, colored);
        Paint_DrawPixel(paint, x - x_pos, y - y_pos, colored);
        e2 = err;
        if (e2 <= y_pos) {
            err += ++y_pos * 2 + 1;
            if(-x_pos == y_pos && e2 <= x_pos) {
              e2 = 0;
            }
        }
        if (e2 > x_pos) {
            err += ++x_pos * 2 + 1;
        }
    } while (x_pos <= 0);
}

/**
*  @brief: this draws a filled circle
*/
void Paint_DrawFilledCircle(Paint* paint, int x, int y, int radius, int colored) {
    /* Bresenham algorithm */
    int x_pos = -radius;
    int y_pos = 0;
    int err = 2 - 2 * radius;
    int e2;

    do {
        Paint_DrawPixel(paint, x - x_pos, y + y_pos, colored);
        Paint_DrawPixel(paint, x + x_pos, y + y_pos, colored);
        Paint_DrawPixel(paint, x + x_pos, y - y_pos, colored);
        Paint_DrawPixel(paint, x - x_pos, y - y_pos, colored);
        Paint_DrawHorizontalLine(paint, x + x_pos, y + y_pos, 2 * (-x_pos) + 1, colored);
        Paint_DrawHorizontalLine(paint, x + x_pos, y - y_pos, 2 * (-x_pos) + 1, colored);
        e2 = err;
        if (e2 <= y_pos) {
            err += ++y_pos * 2 + 1;
            if(-x_pos == y_pos && e2 <= x_pos) {
                e2 = 0;
            }
        }
        if(e2 > x_pos) {
            err += ++x_pos * 2 + 1;
        }
    } while(x_pos <= 0);
}
/**
 *  @brief: this draws a char
 */
void Paint_DrawChar(Paint* paint, int x, int y,int size,const char* ch, int colored)
{
	int x0,y0;
	uint dat;
    for(y0=0;y0<size;y0++){                              //0x10,0x02 0x1000 | 0x02 = 0x1002
		dat=(*ch<<16) | (*(ch+1)<<8) | *(ch+2);    //汉字宽度为16，由两个字节组成一个16位数据
		ch+=3;                            //每次取两个字节
		for(x0=0;x0<size;x0++){             //换列
			//画点
			if(dat&0x800000)                  //从最高开描点，只须判断一位
			{
				Paint_DrawPixel(paint, x + x0, y + y0, colored);
				printf("#");
			}
			else
			{
				printf(".");
			}
			dat<<=1;                      //将次高位，提升为最高位
		}
		printf("\n");
	}
}

void Paint_DrawString(Paint* paint, int x, int y,int size,const char* chs, int colored)
{
	uint zk_read_addr = 0x0;
	char read_buffer[READ_NUM];
	while(*chs !='\0')
	{
		if(chs[0]<0x80){
			Paint_DrawCharAt(paint, x, y,chs[0], &Font20,colored);
			if(x>385){
				x=0;
				y+=size;
			}else{
				x+=(size/2);
			}
			chs ++;
		}else{
			zk_read_addr = 0x0;
			if(chs[1]<0x7f)		zk_read_addr += ((chs[0]-0x81)*190+ chs[1]-0x40) *READ_NUM;//注意!
			else 	zk_read_addr += ((chs[0]-0x81)*190+ chs[1]-0x41) *READ_NUM;

			printf("zk_read_addr = %x\n",zk_read_addr);
			spi_flash_read(zk_read_addr + FONT_FLASH_START,read_buffer,READ_NUM);
			if(x>381){
				x=0;
				y+=size;
			}else{
				x+=size;
			}
			Paint_DrawChar(paint,x,y,size,read_buffer,colored);
			chs+=2;
		}
	}
}

void Paint_DrawString(Paint* paint, int x, int y,int h,int w,const char* chs, int colored)
{
	int h_y,w_x,p;
	for(w_x = 0;w_x < w;w_x+=8){
		for(p= 0;p<8;p++){
			if(chs[wx] | 1<<p){
				Paint_DrawPixel(paint, x + w_x + p, y + h_y, colored);
			}
		}
		if(chs[w_x/8])
	}
	Paint_DrawPixel(paint, x - x_pos, y + y_pos, colored);
}

/* END OF FILE */























