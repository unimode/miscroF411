/*
 * panel.c
 *
 *  Created on: Sep 30, 2017
 *      Author: paramra
 */
#include "gui.h"





void getCmd(Host2DevCmd *cmd)
{
#ifdef EMU_HOST
#endif
}

void sendCmd(const Host2DevCmd *cmd)
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
volatile InputsData inputs_data;
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
}



static void updateInputsData(void)
{
	#ifdef EMU_HOST
	// add code for update with UART
	#else
	// do nothing
	#endif
}

// calls in both of host or device side in while(1) loop
// host                - read inputs state, call processGUI
// dev(host-ctrl mode) - just process UART req, processGUI do nithing on dev side
// dev(auto mode)      - call process GUI
void processGUI(void)
{
#ifndef	EMU_DRAW_HOST
	static uint8_t firstEntry = 1;
	if(firstEntry){
		firstEntry = 0;
		drawPanel();
	}
	updateInputsData();

	// check encoder rotation
	if(inputs_data.enc_update){
		inputs_data.enc_update = 0;
		disp7Update(&henc, inputs_data.enc_value);
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

}



