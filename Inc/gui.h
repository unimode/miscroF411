/*
 * panel.h
 *
 *  Created on: Sep 30, 2017
 *      Author: paramra
 */

#ifndef __PANEL_H__
#define __PANEL_H__

#include "main.h"
#include "config.h"
#include "st7735.h"
#include <stdint.h>

typedef enum{
	TYPE_DRAW_PIXEL,
	TYPE_FILL_RECT,
	TYPE_DISP7,
	TYPE_DRAW_TEXT,
	TYPE_INPUTS
} CmdType;

typedef struct{
	uint8_t x;
	uint8_t y;
	uint16_t color;
} CmdDrawPixel;

typedef struct{
	uint8_t x;
	uint8_t y;
	uint8_t width;
	uint8_t height;
	uint16_t color;
} CmdFillRect;

typedef struct{
	Disp7Type	hdisp;
	uint16_t	data;
} CmdDisp7;

#define CMD_TEXT_MAX_DATA_SIZE	32
typedef struct{
	uint8_t x;
	uint8_t y;
	uint8_t str[CMD_TEXT_MAX_DATA_SIZE];
	uint16_t charColor;
	uint16_t bkgColor;
}CmdText;

typedef struct{
	uint8_t		enc_value;
	uint8_t		enc_dir;
	uint8_t		enc_sw; // 1 - if clicked
	uint16_t	enc_enter_value;
	uint8_t		enc_update;
} InputsData;

typedef struct{
	CmdType		cmd_type;
	uint8_t		flags;
	union{
		CmdDrawPixel	drawpix;
		CmdFillRect		fillrect;
		CmdDisp7		disp7;
		CmdText			text;
		InputsData		iputs;
	};
} Host2DevCmd;

extern volatile InputsData inputs_data;
void processGUI(void);
void drawPanel(void);
void drawInfoLine(void);
uint8_t callHMenu(void);


#endif /* PANEL_H_ */
