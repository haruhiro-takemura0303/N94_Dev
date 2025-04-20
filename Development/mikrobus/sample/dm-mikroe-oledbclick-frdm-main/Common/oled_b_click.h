/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef OLED_B_CLICK_H_
#define OLED_B_CLICK_H_

#include <stdint.h>
#include <stdbool.h>

#define OLEDBCLICK_ADDRESS 0x3C

typedef enum _oled_i2c_option_t
{
	k_oled_i2c_Read,
	k_oled_i2c_Write
}oled_i2c_option_t;

// Scroll types -------------------------------
typedef enum _oled_scroll_type_t
{
	k_oled_rigth_scroll,
	k_oled_left_scroll
} oled_scroll_type_t;

typedef enum _oled_scroll_interval_t // In terms of frame frequency
{
	k_oled_2_frames		= 0x07,
	k_oled_3_frames		= 0x04,
	k_oled_4_frames		= 0x05,
	k_oled_5_frames		= 0x00,
	k_oled_25_frames	= 0x06,
	k_oled_64_frames	= 0x01,
	k_oled_128_frames	= 0x02,
	k_oled_256_frames	= 0x03
} oled_scroll_interval_t;

typedef struct _oled_scroll_config_t
{
	oled_scroll_type_t 		scroll_type;
	uint8_t					start_page_address;
	oled_scroll_interval_t 	scroll_interval;
	uint8_t					end_page_address;
} oled_scroll_config_t;
// --------------------------------------------

// Addressing types ---------------------------
typedef enum _oled_memory_addressing_mode_t
{
	k_oled_horizontal_addressing_mode,
	k_oled_vertical_addressing_mode,
	k_oled_page_addressing_mode
} oled_memory_addressing_mode;
// --------------------------------------------

// Hardware configuration type ----------------
typedef enum _oled_segment_remap_t
{
	k_oled_column_addr0_map_to_seg0,
	k_oled_column_addr127_map_to_seg0
} oled_segment_remap_t;

typedef enum _oled_com_scan_direction_t
{
	k_oled_com_normal_mode,
	k_oled_com_remaped_mode = 0x08
} oled_com_scan_direction_t;

typedef enum _oled_com_pin_config_t
{
	k_oled_sequential_com_pin_config,
	k_oled_alternative_com_pin_config = 0x10
} oled_com_pin_config_t;

typedef enum _oled_com_remap_t
{
	k_disable_com_remap,
	k_enable_com_remap = 0x20
} oled_com_remap_t;
// --------------------------------------------

typedef int32_t(*oled_i2c_transfer)(oled_i2c_option_t option, uint8_t address, uint8_t subaddress,uint8_t* buff, uint32_t len);

void oledBclick_init(oled_i2c_transfer transfer_func);

void oledBclick_clear_screen();

void oledBclick_write_image(const uint8_t image[5][96]);

// Fundamental command table --------------------------------------------------
int32_t oledBclick_enable_display(bool enable);

int32_t oledBclick_enable_inverse(bool enable);

int32_t oledBclick_all_display_on(bool enable);

int32_t oledBclick_set_contrast(uint8_t contrast);
// ----------------------------------------------------------------------------

// Addressing functions -------------------------------------------------------
int32_t oledBclick_set_column_start_address(uint8_t column_start_address);

int32_t oledBclick_set_lower_column_start_address(uint8_t column_start_address);

int32_t oledBclick_set_higher_column_start_address(uint8_t column_start_address);

int32_t oledBclick_set_memory_addressing_mode(oled_memory_addressing_mode mode);

int32_t oledBclick_set_column_start_end_address(uint8_t start, uint8_t end);

int32_t oledBclick_set_page_start_end_address(uint8_t start, uint8_t end);

int32_t oledBclick_set_page_start_address_for_page_addressing_mode(uint8_t start);
// ----------------------------------------------------------------------------

// Scroll functions -----------------------------------------------------------
int32_t oledBclick_enable_disable_scroll(bool enable);

int32_t oledBclick_set_continuos_horizontal_scroll(oled_scroll_config_t scroll_config);

int32_t oledBclick_set_continuos_horizontal_vertican_scroll(oled_scroll_config_t scroll_config, uint8_t vertical_page_offset);

int32_t oledBclick_set_vertical_scroll_area(uint8_t start_row, uint8_t end_row); // screen rows (sart-end)
// ----------------------------------------------------------------------------

// Hardware config functions --------------------------------------------------
int32_t oledBclick_set_display_start_line(uint8_t start);

int32_t oledBclick_set_segment_remap(oled_segment_remap_t segment_remap);

int32_t oledBclick_set_multimplex_ratio(uint8_t mux_ratio);

int32_t oledBclick_set_com_output_scan_direction(oled_com_scan_direction_t scan_direction);

int32_t oledBclick_set_offset_display(uint8_t offset_display);

int32_t oledBclick_set_com_pin_hw_config(oled_com_pin_config_t com_pin_config, oled_com_remap_t com_remap);
// ----------------------------------------------------------------------------

#endif /* OLED_B_CLICK_H_ */
