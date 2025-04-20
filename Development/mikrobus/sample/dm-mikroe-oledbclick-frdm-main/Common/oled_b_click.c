/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <oled_b_click.h>

// DISPLAY 96x39
#define DISPLAY_WIDTH 	96
#define DISPLAY_HEIGHT 	39
#define DISPLAY_HEIGHT_PAGE DISPLAY_HEIGHT/8

// Commands --------------------------------
#define OLED_COMMAND								0x00
#define OLED_DATA									0x40
// Fundamental command table -----------
#define OLED_SET_CONTRAST_CONTROL 					0x81 // Data before
#define OLED_SET_ENTTIRE_DISPLAY_RAM 				0xA4
#define OLED_SET_ENTTIRE_DISPLAY_ON 				0xA5
#define OLED_SET_NORMAL_DISPLAY 					0xA6
#define OLED_SET_INVERT_DISPLAY 					0xA7
#define OLED_SET_OFF_DISPLAY 						0xAE
#define OLED_SET_ON_DISPLAY 						0xAF
// -------------------------------------

//Scrolling command table --------------
#define OLED_SCROLL_RIGHT_COMMAND					0x26
#define OLED_SCROLL_LEFT_COMMAND					0x27
#define OLED_SCROLL_SET_VERTICAL_AREA_COMMAND 		0xA3
#define OLED_DESACTIVATE_SCROLL						0x2E
#define OLED_ACTIVATE_SCROLL						0x2F
// -------------------------------------

// Addressing setting command table ----
#define OLED_COLUMN_LOWER_START_ADDRESS_COMMAND		0x00
#define OLED_COLUMN_HIGHER_START_ADDRESS_COMMAND	0x10
#define OLED_SET_MEMORY_ADDRESSING_MODE_COMMAND		0x20
#define OLED_SET_COLUMN_ADDRESS_COMMAND				0x21
#define OLED_SET_PAGE_ADDRESS_COMMAND				0x22
#define OLED_SET_PAGE_START_ADDRESS_COMMAND			0xB0
// -------------------------------------

// Hardware configuration --------------
#define OLED_SET_DISPLAY_START_LINE_COMMAND			0x40
#define OLED_SET_SEGMENT_REMAP_COMMAND				0xA0
#define OLED_SET_MULTIPLEX_RATIO_COMMAND			0xA8
#define OLED_SET_COM_OUTPUT_SCANN_DIRECION_COMMAND	0xC0
#define OLED_SET_DISPLAY_OFFSET_COMMAND				0xD3
#define OLED_SET_COM_PINS_HW_CONFIG_COMMAND			0xDA
// -------------------------------------

// Timing & Driving Scheme Setting -----
#define OLED_SET_DISPLAY_CLOCK_DIV_F_OSC			0xD5
#define OLED_SET_PRE_CHRG_PERIOD					0xD9
#define OLED_SET_VCOM_LEVEL							0xDB
// -------------------------------------
// -----------------------------------------

static oled_i2c_transfer 	g_i2c_transfer_function = 0;
static uint8_t				g_buff	= 0;
static uint8_t				g_buff_img[DISPLAY_WIDTH]	= {0};

// Send options ---------------------------------------------------------------
int32_t oledBclick_send_command(uint8_t command)
{
	int32_t retval = -1;
	if(g_i2c_transfer_function)
	{
		g_buff = command;
		retval = g_i2c_transfer_function(k_oled_i2c_Write, OLEDBCLICK_ADDRESS, OLED_COMMAND, &g_buff, 1);
	}
	return retval;
}
int32_t oledBclick_send_data(uint8_t *buff, uint16_t size)
{
	int32_t retval = -1;
	if(g_i2c_transfer_function)
	{
		retval = g_i2c_transfer_function(k_oled_i2c_Write, OLEDBCLICK_ADDRESS, OLED_DATA, buff, size);
	}
	return retval;
}
// ----------------------------------------------------------------------------
void oledBclick_init(oled_i2c_transfer transfer_func)
{
	g_i2c_transfer_function = transfer_func;
	oledBclick_enable_display(false);

	oledBclick_send_command(0x8D);
	oledBclick_send_command(0x14);

	oledBclick_set_contrast(200);
	oledBclick_enable_inverse(false);
	oledBclick_all_display_on(false);
	oledBclick_enable_display(true);

	oledBclick_set_display_start_line(0);
	oledBclick_set_memory_addressing_mode(k_oled_page_addressing_mode);
	oledBclick_set_page_start_end_address(0, 4);

	oledBclick_clear_screen();
}

void oledBclick_clear_screen()
{
	uint8_t page;
	for(page = 0; page <= DISPLAY_HEIGHT_PAGE; page ++)
	{
		oledBclick_set_column_start_address(0);
		oledBclick_set_page_start_address_for_page_addressing_mode(page);
		oledBclick_send_data(g_buff_img, DISPLAY_WIDTH);
	}
}

void oledBclick_write_image(const uint8_t image[5][96])
{
	uint8_t page;
	for(page = 0; page < 5; page ++)
	{
		oledBclick_set_column_start_address(0);
		oledBclick_set_page_start_address_for_page_addressing_mode(page);
		oledBclick_send_data((uint8_t*)image[page], 96);
	}
}

// Fundamental command table --------------------------------------------------
int32_t oledBclick_enable_display(bool enable)
{
	int32_t retval;
	retval = oledBclick_send_command((enable)? OLED_SET_ON_DISPLAY:OLED_SET_OFF_DISPLAY);
	return retval;
}

int32_t oledBclick_enable_inverse(bool enable)
{
	int32_t retval;
	retval = oledBclick_send_command((enable)? OLED_SET_INVERT_DISPLAY:OLED_SET_NORMAL_DISPLAY);
	return retval;
}


int32_t oledBclick_all_display_on(bool enable)
{
	int32_t retval;
	retval = oledBclick_send_command((enable)? OLED_SET_ENTTIRE_DISPLAY_ON:OLED_SET_ENTTIRE_DISPLAY_RAM);
	return retval;
}

int32_t oledBclick_set_contrast(uint8_t contrast)
{
	int32_t retval;
	retval = oledBclick_send_command(OLED_SET_CONTRAST_CONTROL);
	if(!retval) retval = oledBclick_send_command(contrast);
	return retval;
}
// ----------------------------------------------------------------------------

// Addressing functions -------------------------------------------------------
int32_t oledBclick_set_column_start_address(uint8_t column_start_address)
{
	int32_t retval;
	retval = oledBclick_set_lower_column_start_address(column_start_address & 0x0F);
	if(!retval) retval = oledBclick_set_higher_column_start_address((column_start_address & 0xF0)>>4);
	return retval;
}

int32_t oledBclick_set_lower_column_start_address(uint8_t column_start_address)
{
	int32_t retval;
	column_start_address &= 0x0F;
	column_start_address |= OLED_COLUMN_LOWER_START_ADDRESS_COMMAND;
	retval = oledBclick_send_command(column_start_address);
	return retval;
}

int32_t oledBclick_set_higher_column_start_address(uint8_t column_start_address)
{
	int32_t retval;
	column_start_address &= 0x0F;
	column_start_address |= OLED_COLUMN_HIGHER_START_ADDRESS_COMMAND;
	retval = oledBclick_send_command(column_start_address);
	return retval;
}

int32_t oledBclick_set_memory_addressing_mode(oled_memory_addressing_mode mode)
{
	int32_t retval;
	retval = oledBclick_send_command(OLED_SET_MEMORY_ADDRESSING_MODE_COMMAND);
	if(!retval) retval = oledBclick_send_command(mode);
	return retval;
}

int32_t oledBclick_set_column_start_end_address(uint8_t start, uint8_t end)
{
	int32_t retval;
	retval = oledBclick_send_command(OLED_SET_COLUMN_ADDRESS_COMMAND);
	if(!retval) retval = oledBclick_send_command(start);
	if(!retval) retval = oledBclick_send_command(end);
	return retval;
}

int32_t oledBclick_set_page_start_end_address(uint8_t start, uint8_t end)
{
	int32_t retval;
	retval = oledBclick_send_command(OLED_SET_PAGE_ADDRESS_COMMAND);
	if(!retval) retval = oledBclick_send_command(start);
	if(!retval) retval = oledBclick_send_command(end);
	return retval;
}

int32_t oledBclick_set_page_start_address_for_page_addressing_mode(uint8_t start)
{
	int32_t retval;
	start &= 0x07;
	start |= OLED_SET_PAGE_START_ADDRESS_COMMAND;
	retval = oledBclick_send_command(start);
	return retval;
}
// ----------------------------------------------------------------------------

// Scroll functions -----------------------------------------------------------
int32_t oledBclick_enable_disable_scroll(bool enable)
{
	int32_t retval;
	retval = oledBclick_send_command((enable)? OLED_ACTIVATE_SCROLL:OLED_DESACTIVATE_SCROLL);
	return retval;
}

int32_t oledBclick_set_continuos_horizontal_scroll(oled_scroll_config_t scroll_config)
{
	int32_t retval;
	retval = oledBclick_send_command((scroll_config.scroll_type == k_oled_rigth_scroll)? OLED_SCROLL_RIGHT_COMMAND:OLED_SCROLL_LEFT_COMMAND);
	if(!retval) retval = oledBclick_send_command(0x00);
	if(!retval) retval = oledBclick_send_command(scroll_config.start_page_address);
	if(!retval) retval = oledBclick_send_command(scroll_config.scroll_interval);
	if(!retval) retval = oledBclick_send_command(scroll_config.end_page_address);
	if(!retval) retval = oledBclick_send_command(0x00);
	if(!retval) retval = oledBclick_send_command(0xFF);
	return retval;
}

int32_t oledBclick_set_continuos_horizontal_vertican_scroll(oled_scroll_config_t scroll_config, uint8_t vertical_page_offset)
{
	int32_t retval;
	retval = oledBclick_send_command((scroll_config.scroll_type == k_oled_rigth_scroll)? OLED_SCROLL_RIGHT_COMMAND:OLED_SCROLL_LEFT_COMMAND);
	if(!retval) retval = oledBclick_send_command(0x00);
	if(!retval) retval = oledBclick_send_command(scroll_config.start_page_address);
	if(!retval) retval = oledBclick_send_command(scroll_config.scroll_interval);
	if(!retval) retval = oledBclick_send_command(scroll_config.end_page_address);
	if(!retval) retval = oledBclick_send_command(vertical_page_offset);
	return retval;
}

int32_t oledBclick_set_vertical_scroll_area(uint8_t start_row, uint8_t end_row) // screen rows (sart-end)
{
	int32_t retval;
	retval = oledBclick_send_command(OLED_SCROLL_SET_VERTICAL_AREA_COMMAND);
	if(!retval) retval = oledBclick_send_command(start_row);
	if(!retval) retval = oledBclick_send_command(end_row);
	return retval;
}
// ----------------------------------------------------------------------------

// Hardware config functions --------------------------------------------------
int32_t oledBclick_set_display_start_line(uint8_t start)
{
	int32_t retval;
	start &= 0x3F;
	start |= OLED_SET_DISPLAY_START_LINE_COMMAND;
	retval = oledBclick_send_command(start);
	return retval;
}

int32_t oledBclick_set_segment_remap(oled_segment_remap_t segment_remap)
{
	int32_t retval;
	segment_remap |= OLED_SET_SEGMENT_REMAP_COMMAND;
	retval = oledBclick_send_command(segment_remap);
	return retval;
}

int32_t oledBclick_set_multimplex_ratio(uint8_t mux_ratio)
{
	int32_t retval;
	retval = oledBclick_send_command(OLED_SET_MULTIPLEX_RATIO_COMMAND);
	if(!retval) retval = oledBclick_send_command(mux_ratio);
	return retval;
}

int32_t oledBclick_set_com_output_scan_direction(oled_com_scan_direction_t scan_direction)
{
	int32_t retval;
	scan_direction |= OLED_SET_COM_OUTPUT_SCANN_DIRECION_COMMAND;
	retval = oledBclick_send_command(scan_direction);
	return retval;
}

int32_t oledBclick_set_offset_display(uint8_t offset_display)
{
	int32_t retval;
	retval = oledBclick_send_command(OLED_SET_DISPLAY_OFFSET_COMMAND);
	if(!retval) retval = oledBclick_send_command(offset_display);
	return retval;
}

int32_t oledBclick_set_com_pin_hw_config(oled_com_pin_config_t com_pin_config, oled_com_remap_t com_remap)
{
	int32_t retval;
	retval = oledBclick_send_command(OLED_SET_COM_PINS_HW_CONFIG_COMMAND);
	if(!retval) retval = oledBclick_send_command(com_pin_config|com_remap|0x02);
	return retval;
}
// ----------------------------------------------------------------------------
