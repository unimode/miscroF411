/*
 * panel.c
 *
 *  Created on: Sep 30, 2017
 *      Author: paramra
 */

#include "gui.h"
#include "dma.h"
#include "usart.h"
#include "spi.h"
#include <string.h>





extern int fd; /* Файловый дескриптор для порта */
extern char buf[512];/*размер зависит от размера строки принимаемых данных*/
extern int outa;
extern int iIn;
extern char rxbuf[512];
extern Host2DevCmd	cmd;
//extern InputsData	inputs;

extern const GIMPImage front_panel_img;



// -------------- wrappers for drawing functions-----------------------------------------------------------
void __disp7Update(Disp7Type *hdisp, uint16_t data)
{
#ifdef EMU_DEVICE
	disp7Update(hdisp, data);
#else
	Host2DevCmd cmd;
	cmd.cmd_type		= TYPE_DISP7;
	cmd.disp7.hdisp		= *hdisp;
	sendCmd(&cmd);
#endif
}

void __DrawPixel(uint8_t x, uint8_t y, uint16_t color)
{
#ifdef EMU_DEVICE
	st7735DrawPixel(x, y, color);
#else
	Host2DevCmd cmd;
	cmd.cmd_type		= TYPE_DRAW_PIXEL;
	cmd.drawpix.x		= x;
	cmd.drawpix.y		= y;
	cmd.drawpix.color	= color;
	sendCmd(&cmd);
#endif
}

void __FillRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint16_t color)
{
#ifdef EMU_DEVICE
	st7735FillRect(x, y, width, height, color);
#else
	Host2DevCmd cmd;
	cmd.cmd_type		= TYPE_FILL_RECT;
	cmd.fillrect.x		= x;
	cmd.fillrect.y		= y;
	cmd.fillrect.width	= width;
	cmd.fillrect.height	= height;
	cmd.fillrect.color	= color;
	sendCmd(&cmd);
#endif
}
void __DrawRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint16_t color, uint8_t thick)
{
#ifdef EMU_DEVICE
	st7735DrawRect(x, y, width, height, color, thick);
#else
	Host2DevCmd cmd;
	cmd.cmd_type		= TYPE_DRAW_RECT;
	cmd.drawrect.x		= x;
	cmd.drawrect.y		= y;
	cmd.drawrect.width	= width;
	cmd.drawrect.height	= height;
	cmd.drawrect.color	= color;
	cmd.drawrect.thick	= thick;
	sendCmd(&cmd);
#endif
}

void __DrawText(uint8_t x, uint8_t y, const uint8_t str[], uint16_t charColor, uint16_t bkgColor)
{
#ifdef EMU_DEVICE
	st7735DrawText(x, y, str, charColor, bkgColor);
#else
	Host2DevCmd cmd;
	cmd.cmd_type		= TYPE_DRAW_TEXT;
	cmd.text.x			= x;
	cmd.text.y			= y;
	strcpy(cmd.text.str, str);
	cmd.text.charColor	= charColor;
	cmd.text.bkgColor	= bkgColor;
	sendCmd(&cmd);
#endif
}
//---------------------------------------------------------------------------------------------------------

// ****************************************** USER INTERFACE ********************************************
Host2DevCmd			host2dev_cmd;
volatile uint8_t	h2dev_ready = 1;
volatile InputsData	inputs_data;
uint8_t	 tmp_str[64];

Disp7Type henc ={
  			  .x = 50,
  			  .y = 30,
  			  .fcolor = LCD_RED,
  			  .bcolor = 0x0,
  			  .size = 1,
  			  .digits = 4,
  			  .data = 1234
  	  };

void drawPanel(void)
{
	__FillRect(0, 0, 128, 160, 0);
	__DrawRect(0, 0, 128, 160, LCD_BLUE,1);
	__DrawText(35, 35, "TEST", LCD_GREEN, LCD_RED);
}


void processUART(void)
{
}

// calls in both of host or device side in while(1) loop
// host                - read inputs state, call processGUI
// dev(host-ctrl mode) - just process UART req, processGUI do nithing on dev side
// dev(auto mode)      - call process GUI
void processGUI(void)
{
	static uint8_t firstEntry = 1;

	if(firstEntry){
		firstEntry = 0;
		bzero(&host2dev_cmd, sizeof(host2dev_cmd));
		st7735FillRect(0, 0, 128, 160, 0);
		st7735DrawRect(0, 0, 128, 160, LCD_YELLOW, 1);
		st7735DrawText(57, 15, "WAITING A HOST...", LCD_RED, 0);
		//st7735DrawImage(10, 10, &test_img1);
		st7735DrawImage(0, 0, &front_panel_img);
		//st7735DrawImage(0, 0, &mars_img);
		/*int x,y;
		for(x=0; x<16; x++)
			st7735FillRect(8*x, 0, 1, 160, LCD_YELLOW);
		for(y=0; y<16; y++)
			st7735FillRect(0, 10*y, 128, 1, LCD_YELLOW);
			*/
	}
	//!!! DEMO TEST !!!
	if(inputs_data.enc_update){
		inputs_data.enc_update = 0;
		disp7Update(&henc, inputs_data.enc_value);
		}
	}


void drawInfoLine(void)
{

}

uint8_t callHMenu(void)
{
	return 0;
}



