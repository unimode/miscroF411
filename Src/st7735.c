/*
 * ST7735.c
 *
 *  Created on: Aug 13, 2017
 *      Author: paramra
 */
#include "main.h"
#include "stm32f4xx_hal.h"
#include "spi.h"
#include "gpio.h"
#include "font7x15.h"
#include "st7735.h"

extern DMA_HandleTypeDef hdma_spi3_tx;


static void setCS(uint8_t value)
{
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, value);
}

static void setA0(uint8_t value)
{
	HAL_GPIO_WritePin(LCD_A0_GPIO_Port, LCD_A0_Pin, value);
}

static void setRESET(uint8_t value)
{
	HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, value);
}

static void sendCmd(uint8_t data)
{
	uint8_t t = data;
	setA0(0);
	//HAL_SPI_Transmit_DMA(&hspi3, &t, 1);
	HAL_SPI_Transmit(&hspi3, &t, 1, 5000);
	//while(hspi3.Instance->SR  & SPI_SR_BSY);
	while(HAL_SPI_GetState(&hspi3) == HAL_SPI_STATE_BUSY_TX);
}

static void sendData(uint8_t data)
{
	uint8_t t = data;
	setA0(1);
	//HAL_SPI_Transmit_DMA(&hspi3, &t, 1);
	HAL_SPI_Transmit(&hspi3, &t, 1, 5000);
	//while(hspi3.Instance->SR  & SPI_SR_BSY);
	while(HAL_SPI_GetState(&hspi3) == HAL_SPI_STATE_BUSY_TX);
}

void st7735Init(void)
{
	setCS(0);
	HAL_Delay(100);

	// software reset
	sendCmd(0x01);
	HAL_Delay(100);

	// hardware reset
	setRESET(0);
    HAL_Delay(100);
    setRESET(1);
    HAL_Delay(100);

    // wake up
    sendCmd(0x11);
    HAL_Delay(100);

    // color mode 16bit
    sendCmd(0x3A);
    sendData(0x05);

    // direction and color
    sendCmd(0x36);
    sendData(0x14); // RGB
    //sendData(0x1C); // BGR

    sendCmd(0x29); // turn on display

    // setCS(1); ???

}

void st7735SetRect(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
	sendCmd(0x2A);
	sendData(0x00);
	sendData(x1);
	sendData(0x00);
	sendData(x2);

	sendCmd(0x2B);
	sendData(0x00);
	sendData(y1);
	sendData(0x00);
	sendData(y2);
}

void st7735DrawPixel(uint8_t x, uint8_t y, uint16_t color)
{
	uint8_t tmp;

	st7735SetRect(x, y, x, y);
	sendCmd(0x2C);
	tmp = (uint8_t)((color & 0xFF00)>>8);
	sendData(tmp);
	tmp = (uint8_t)(color & 0x00FF);
	sendData(tmp);
}

void st7735FillRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint16_t color)
{
	if(width == 0)
		width = 1;
	if(height == 0)
		height = 1;

	st7735SetRect(x, y, x+width-1, y+height-1);
	sendCmd(0x2C);
	setA0(1);

	HAL_SPI_DeInit(&hspi3);
	hspi3.Init.DataSize = SPI_DATASIZE_16BIT;
	HAL_SPI_Init(&hspi3);

	//while(HAL_SPI_GetState(&hspi3) == HAL_SPI_STATE_RESET);
	HAL_StatusTypeDef result = HAL_SPI_Transmit_DMA(&hspi3, (uint8_t*)(&color), 2*width*height);
	//while(hspi3.Instance->SR  & SPI_SR_BSY);
	while(HAL_SPI_GetState(&hspi3) == HAL_SPI_STATE_BUSY_TX);

	//HAL_Delay(100);

	HAL_SPI_DeInit(&hspi3);
	hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
	HAL_SPI_Init(&hspi3);
}

void st7735FillRect2(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t color)
{
	uint8_t tmp;
	uint8_t x,y;

	st7735SetRect(x1, y1, x2, y2);
	sendCmd(0x2C);
	setA0(1);

	HAL_SPI_DeInit(&hspi3);
	hspi3.Init.DataSize = SPI_DATASIZE_16BIT;
	HAL_SPI_Init(&hspi3);

	//while(HAL_SPI_GetState(&hspi3) == HAL_SPI_STATE_RESET);
	HAL_StatusTypeDef result = HAL_SPI_Transmit_DMA(&hspi3, (uint8_t*)(&color), 2*(x2-x1+1)*(y2-y1+1));
	//while(hspi3.Instance->SR  & SPI_SR_BSY);
	while(HAL_SPI_GetState(&hspi3) == HAL_SPI_STATE_BUSY_TX);


	HAL_SPI_DeInit(&hspi3);
	hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
	HAL_SPI_Init(&hspi3);
}

void st7735DrawRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint16_t color, uint8_t thick)
{
	st7735FillRect(x, y, width, thick, color);
	st7735FillRect(x, y+height-thick, width, thick, color);
	st7735FillRect(x, y, thick, height, color);
	st7735FillRect(x+width-thick, y, thick, height, color);
}

void st7735DrawSymbol(uint8_t x, uint8_t y, uint8_t chr, uint16_t charColor, uint16_t bkgColor)
{
	uint8_t i;
	uint8_t j;

	st7735SetRect(x, y, x+12, y+8);
	sendCmd(0x2C);
	setA0(1);

	HAL_SPI_DeInit(&hspi3);
	hspi3.Init.DataSize = SPI_DATASIZE_16BIT;
	HAL_SPI_Init(&hspi3);

	uint8_t k;
	for (i=0;i<7;i++){
		for (k=2;k>0;k--){
			uint8_t chl=NewBFontLAT[ ( (chr-0x20)*14 + i+ 7*(k-1)) ];
			chl=chl<<2*(k-1);
			uint8_t h;
			if (k==2) h=6; else h=7;
			for (j=0;j<h;j++){
				unsigned int color;
				if(chl & 0x80) color=charColor; else color=bkgColor;
				chl = chl<<1;
				//while(HAL_SPI_GetState(&hspi3) == HAL_SPI_STATE_RESET);
				HAL_SPI_Transmit(&hspi3, &color, 1, 5000);
				//while(hspi3.Instance->SR  & SPI_SR_BSY);
				while(HAL_SPI_GetState(&hspi3) == HAL_SPI_STATE_BUSY_TX);
			}
		}
	}
/*
	for (j=0;j<13;j++) {
		//while(HAL_SPI_GetState(&hspi3) == HAL_SPI_STATE_RESET);
		HAL_SPI_Transmit(&hspi3, &bkgColor, 1, 5000);
		//while(hspi3.Instance->SR  & SPI_SR_BSY);
		while(HAL_SPI_GetState(&hspi3) == HAL_SPI_STATE_BUSY_TX);
	}
*/
	HAL_SPI_Transmit_DMA(&hspi3, (uint8_t*)(&bkgColor), 13);
	while(HAL_SPI_GetState(&hspi3) == HAL_SPI_STATE_BUSY_TX);

	HAL_SPI_DeInit(&hspi3);
	hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
	HAL_SPI_Init(&hspi3);

}

void st7735DrawText(uint8_t x, uint8_t y, const uint8_t str[], uint16_t charColor, uint16_t bkgColor)
{

	while (*str!=0) {
		st7735DrawSymbol(x, y, *str, charColor, bkgColor);
		y=y+8;
		str++;
	}
}

// ---------------- 7-segment display -------------------
//     --A--
//    F     B
//     --G--             byte: A-B-C-D-E-F-G-DP
//    E     C
//     --D--  DP
uint8_t dx = 10; // x --> y
uint8_t dy = 10; // y --> x
uint8_t ds = 2;  // space
uint8_t decode[]={252, 96, 218, 242, 102, 182, 190, 224, 254, 246};


void drawDigit(uint8_t x, uint8_t y, uint16_t color, uint8_t size, uint8_t data)
{
	uint8_t i;
	uint8_t digit = data;
	if(decode[data] & 128)
		st7735FillRect(x+2*dx+2*ds, y+2*ds, ds, dy, color);      // A
	if(decode[data] & 64)
		st7735FillRect(x+1*dx+2*ds, y+1*dx+3*ds, dx, ds, color); // B
	if(decode[data] & 32)
		st7735FillRect(x+0*dx+1*ds, y+1*dx+3*ds, dx, ds, color); // C
	if(decode[data] & 16)
		st7735FillRect(x, y+2*ds, ds, dy, color);                // D
	if(decode[data] & 8)
		st7735FillRect(x+0*dx+1*ds, y, dx, ds, color);           // E
	if(decode[data] & 4)
		st7735FillRect(x+1*dx+2*ds, y, dx, ds, color); 			 // F
	if(decode[data] & 2)
		st7735FillRect(x+1*dx+1*ds, y+2*ds, ds, dy, color);      // G
}

int  disp7Init(Disp7Type *hdisp, uint8_t x, uint8_t y, uint16_t fcolor,
				uint16_t bcolor, uint8_t size, uint16_t data)
{

	return 1;
}

void disp7Update(Disp7Type *hdisp, uint16_t data)
{
	uint8_t i;
	uint16_t a = 0;
	uint16_t div = 1;

	if(hdisp->size == 0){
		dx = 5;
		dy = 5;
		ds = 1;
	}
	else{
		dx = 10;
		dy = 10;
		ds = 2;
	}

	for(i=0; i<hdisp->digits; i++){
		div *= 10;
		a = (data - a) % div;
		drawDigit(hdisp->x, hdisp->y+(dy+6*ds)*(hdisp->digits-i-1), hdisp->bcolor, hdisp->size, 8);
		drawDigit(hdisp->x, hdisp->y+(dy+6*ds)*(hdisp->digits-i-1), hdisp->fcolor, hdisp->size, a*10/div);
	}
}

void st7735DrawImage(uint8_t x, uint8_t y, GIMPImage *image)
{

	//st7735SetRect(x, y, x+image->width-1, y+image->height-1);
	st7735SetRect(x, y, x+image->height-1, y+image->width-1); // width <-> height
	//st7735SetRect(0, 0, 127, 159);

	HAL_SPI_DeInit(&hspi3);
	hspi3.Init.DataSize = SPI_DATASIZE_16BIT;
	HAL_SPI_Init(&hspi3);

	// adjust DMA for image mode
	HAL_DMA_DeInit(&hdma_spi3_tx);
	hdma_spi3_tx.Init.MemInc = DMA_MINC_ENABLE;
	HAL_DMA_Init(&hdma_spi3_tx);


	// send image to display
	sendCmd(0x2C);
	setA0(1);

	HAL_StatusTypeDef result = HAL_SPI_Transmit_DMA(&hspi3, image->pixel_data, image->width*image->height);
	while(HAL_SPI_GetState(&hspi3) == HAL_SPI_STATE_BUSY_TX);

	HAL_SPI_DeInit(&hspi3);
	hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
	HAL_SPI_Init(&hspi3);

	// restore DMA for fill mode

	HAL_DMA_DeInit(&hdma_spi3_tx);
	hdma_spi3_tx.Init.MemInc = DMA_MINC_DISABLE;
	HAL_DMA_Init(&hdma_spi3_tx);

}
