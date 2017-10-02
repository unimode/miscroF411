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


void sendCmd(Host2DevCmd *cmd)
{
#ifdef EMU_HOST
#endif
}

// -------------- wrappers for drawing functions-----------------------------------------------------------
void wrap_disp7Update(Disp7Type *hdisp, uint16_t data)
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

void wrap_st7735DrawPixel(uint8_t x, uint8_t y, uint16_t color)
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

void wrap_st7735FillRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint16_t color)
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
void wrap_st7735DrawRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint16_t color, uint8_t thick)
{
#ifdef EMU_DEVICE
	st7735DrawRect(x, y, width, height, color, thick);
	Host2DevCmd cmd;
	cmd.cmd_type		= TYPE_DRAW_RECT;
	cmd.drawrect.x		= x;
	cmd.drawrect.y		= y;
	cmd.drawrect.width	= width;
	cmd.drawrect.height	= height;
	cmd.drawrect.color	= color;
	cmd.drawrect.thick	= thick;
	sendCmd(&cmd);
#else


#endif
}

void wrap_st7735DrawText(uint8_t x, uint8_t y, const uint8_t str[], uint16_t charColor, uint16_t bkgColor)
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
	wrap_st7735FillRect(0, 0, 128, 160, 0);
	wrap_st7735DrawRect(0, 0, 128, 160, LCD_BLUE, 1);
}


void processUART(void)
{
#ifdef	EMU_DRAW_HOST
	if(h2dev_ready){
		h2dev_ready = 0;
		// translate command from host
		HAL_UART_Receive_DMA(&huart2, (uint8_t *)&host2dev_cmd, sizeof(Host2DevCmd));
		switch(host2dev_cmd.cmd_type){
			case TYPE_NONE:
			break;

			case TYPE_DRAW_PIXEL:
				st7735DrawPixel(host2dev_cmd.drawpix.x, host2dev_cmd.drawpix.y,
						host2dev_cmd.drawpix.color);
			break;

			case TYPE_DRAW_RECT:
				st7735DrawRect(host2dev_cmd.drawrect.x, host2dev_cmd.drawrect.y,
						host2dev_cmd.drawrect.width, host2dev_cmd.drawrect.height,
						host2dev_cmd.drawrect.color, host2dev_cmd.drawrect.thick);
			break;

			case TYPE_FILL_RECT:
				st7735FillRect(host2dev_cmd.fillrect.x, host2dev_cmd.fillrect.y,
						host2dev_cmd.fillrect.width, host2dev_cmd.fillrect.height,
						host2dev_cmd.fillrect.color);
			break;

			case TYPE_DRAW_TEXT:
				st7735DrawText(host2dev_cmd.text.x, host2dev_cmd.text.y,
						host2dev_cmd.text.str, host2dev_cmd.text.charColor,
						host2dev_cmd.text.bkgColor);
			break;

			case TYPE_DISP7:
				disp7Update(&host2dev_cmd.disp7.hdisp, host2dev_cmd.disp7.data);
			break;

			case TYPE_INPUTS:
			break;
		}

		// reply inputs state
		if(host2dev_cmd.cmd_type != TYPE_NONE){ //in first call don't send reply to host
			HAL_UART_Transmit_DMA(&huart2, (uint8_t*)&inputs_data, sizeof(inputs_data));
		}
	}
#endif
}

// calls in both of host or device side in while(1) loop
// host                - read inputs state, call processGUI
// dev(host-ctrl mode) - just process UART req, processGUI do nithing on dev side
// dev(auto mode)      - call process GUI
void processGUI(void)
{
	static uint8_t firstEntry = 1;
#ifdef	EMU_DRAW_HOST // device translate commands from host
	if(firstEntry){
		firstEntry = 0;
		bzero(&host2dev_cmd, sizeof(host2dev_cmd));
		st7735FillRect(0, 0, 128, 160, 0);
		st7735DrawRect(0, 0, 128, 160, LCD_YELLOW, 1);
		st7735DrawText(57, 15, "WAITING A HOST...", LCD_RED, 0);
		/*int x,y;
		for(x=0; x<16; x++)
			st7735FillRect(8*x, 0, 1, 160, LCD_YELLOW);
		for(y=0; y<16; y++)
			st7735FillRect(0, 10*y, 128, 1, LCD_YELLOW);
			*/
	}
	processUART();
#else // host, device drawing
	if(firstEntry){
		firstEntry = 0;
		drawPanel();
	}

	// check encoder rotation
	if(inputs_data.enc_update){
		inputs_data.enc_update = 0;
		wrap_disp7Update(&henc, inputs_data.enc_value);
	}

	// check encoder switch
	if(inputs_data.enc_sw){
		inputs_data.enc_sw = 0;
		wrap_st7735DrawText(20, 110, tmp_str, 0x0000, 0);
		sprintf(tmp_str, "%d", inputs_data.enc_enter_value);
		wrap_st7735DrawText(20, 110, tmp_str, LCD_BLUE, 0);
	}
#endif
}



void drawInfoLine(void)
{

}

uint8_t callHMenu(void)
{
	return 0;
}



