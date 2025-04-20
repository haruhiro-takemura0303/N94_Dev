/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <stdbool.h>
#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_clock.h"
#include "fsl_debug_console.h"
#include "fsl_adapter_gpio.h"

// CMSIS driver ------------------
#ifdef MCXC041
#include "fsl_i2c.h"
#include "fsl_i2c_cmsis.h"

#else
#ifdef MCXC242
#include "fsl_i2c.h"
#include "fsl_i2c_cmsis.h"

#else
#ifdef MCXC444
#include "fsl_i2c.h"
#include "fsl_i2c_cmsis.h"

#else
#ifdef MCXA153
#include "fsl_lpi2c.h"
#include "fsl_lpi2c_cmsis.h"

#else
#ifdef MCXA156
#include "fsl_lpi2c.h"
#include "fsl_lpi2c_cmsis.h"

#else
#ifdef MCXN236
#include "fsl_lpi2c.h"
#include "fsl_lpi2c_cmsis.h"

#else
#ifdef MCXN947
#include "fsl_lpi2c.h"
#include "fsl_lpi2c_cmsis.h"

#else
#ifdef MCXW71
#include "fsl_lpi2c.h"
#include "fsl_lpi2c_cmsis.h"

#else
#ifdef RW612
#include "fsl_i2c.h"
#include "fsl_i2c_cmsis.h"

#else
#error "NOT BOARD DEFINED IN PREPROCESSOR"

#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
// -------------------------------

// OLED driver -------------------
#include "oled_b_click.h"
// -------------------------------

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifdef MCXC041
#define CMSIS_I2C_MASTER 			Driver_I2C0
#define I2C_CLOCK_FREQUENCY 		CLOCK_GetFreq(kCLOCK_CoreSysClk)
#define MAIN_CLOCK_FREQUENCY		CLOCK_GetFreq(kCLOCK_CoreSysClk)/2

#else
#ifdef MCXC242
#define CMSIS_I2C_MASTER 			Driver_I2C0
#define I2C_CLOCK_FREQUENCY 		CLOCK_GetFreq(I2C0_CLK_SRC);
#define MAIN_CLOCK_FREQUENCY		CLOCK_GetFreq(kCLOCK_CoreSysClk)

#else
#ifdef MCXC444
#define CMSIS_I2C_MASTER 			Driver_I2C1
#define I2C_CLOCK_FREQUENCY 		CLOCK_GetFreq(kCLOCK_CoreSysClk)
#define MAIN_CLOCK_FREQUENCY		CLOCK_GetFreq(kCLOCK_CoreSysClk)

#else
#ifdef MCXA153
#define CMSIS_I2C_MASTER 			Driver_I2C0
#define I2C_CLOCK_FREQUENCY 		CLOCK_GetLpi2cClkFreq()
#define MAIN_CLOCK_FREQUENCY		CLOCK_GetFreq(kCLOCK_MainClk)

#else
#ifdef MCXA156
#define CMSIS_I2C_MASTER 			Driver_I2C3
#define I2C_CLOCK_FREQUENCY 		CLOCK_GetLpi2cClkFreq(3)
#define MAIN_CLOCK_FREQUENCY		CLOCK_GetFreq(kCLOCK_MainClk)

#else
#ifdef MCXN236
#define CMSIS_I2C_MASTER 			Driver_I2C2
#define I2C_CLOCK_FREQUENCY 		CLOCK_GetLPFlexCommClkFreq(2u)
#define MAIN_CLOCK_FREQUENCY		CLOCK_GetCoreSysClkFreq()

#else
#ifdef MCXN947
#define CMSIS_I2C_MASTER 			Driver_I2C3
#define I2C_CLOCK_FREQUENCY 		CLOCK_GetLPFlexCommClkFreq(3u)
#define MAIN_CLOCK_FREQUENCY		CLOCK_GetMainClkFreq()

#else
#ifdef MCXW71
#define CMSIS_I2C_MASTER 			Driver_I2C1
#define I2C_CLOCK_FREQUENCY 		CLOCK_GetIpFreq(kCLOCK_Lpi2c1)
#define MAIN_CLOCK_FREQUENCY		CLOCK_GetCoreSysClkFreq()
#else
#define CMSIS_I2C_MASTER 			Driver_I2C2
#define I2C_CLOCK_FREQUENCY 		CLOCK_GetFlexCommClkFreq(2U);
#define MAIN_CLOCK_FREQUENCY		CLOCK_GetCoreSysClkFreq()

#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif

#define ICON_PERIOD_US				700000
#define SEND_I2C_BUFF_SIZE			1000
/*******************************************************************************
 * Variables
 ******************************************************************************/
extern const uint8_t icon_5g[8][96];
extern const uint8_t icon_hmi[8][96];
extern const uint8_t icon_mcu[8][96];
extern const uint8_t icon_wireless[8][96];

volatile bool g_MasterCompletionFlag = false;
/*******************************************************************************
 * Code
 ******************************************************************************/
#ifdef MCXC041
uint32_t I2C0_GetFreq(void)
{
    return I2C_CLOCK_FREQUENCY;
}

#else
#ifdef MCXC242
uint32_t I2C0_GetFreq(void)
{
    return I2C_CLOCK_FREQUENCY;
}

#else
#ifdef MCXC444
uint32_t I2C1_GetFreq(void)
{
    return I2C_CLOCK_FREQUENCY;
}

#else
#ifdef MCXA153
uint32_t LPI2C0_GetFreq(void)
{
    return I2C_CLOCK_FREQUENCY;
}

#else
#ifdef MCXA156
uint32_t LPI2C3_GetFreq(void)
{
    return I2C_CLOCK_FREQUENCY;
}

#else
#ifdef MCXN236
uint32_t LPI2C2_GetFreq(void)
{
    return I2C_CLOCK_FREQUENCY;
}

#else
#ifdef MCXN947
uint32_t LPI2C3_GetFreq(void)
{
    return I2C_CLOCK_FREQUENCY;
}

#else
#ifdef MCXW71
uint32_t LPI2C1_GetFreq(void)
{
    return I2C_CLOCK_FREQUENCY;
}
#else
uint32_t I2C2_GetFreq(void)
{
    return CLOCK_GetFlexCommClkFreq(2U);
}

#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif

static void i2c_master_callback(uint32_t event)
{
    switch (event)
    {
        /* The master has sent a stop transition on the bus */
        case ARM_I2C_EVENT_TRANSFER_DONE:
            g_MasterCompletionFlag = true;
            break;

        /* master arbitration lost */
        case ARM_I2C_EVENT_ARBITRATION_LOST:
            g_MasterCompletionFlag = true;
            break;

        default:
            break;
    }
}

// I2C transmit function ---------------------------------------------------------------
void vector_copy(uint8_t *src, uint8_t *dest, uint32_t len)
{
	uint32_t index;
	for(index = 0; index < len; index ++) dest[index] = src[index];
}
int32_t oled_i2c_transfer_function(oled_i2c_option_t option, uint8_t address, uint8_t subaddress, uint8_t* buff, uint32_t len)
{
	static uint8_t s_buff_size[SEND_I2C_BUFF_SIZE];
	int32_t retval;

	if(option == k_oled_i2c_Read)
	{
		retval = CMSIS_I2C_MASTER.MasterTransmit(address, &subaddress, 1, true);
		while(!g_MasterCompletionFlag);
		g_MasterCompletionFlag = false;
		retval = CMSIS_I2C_MASTER.MasterReceive(address, buff, len, false);
	}
	else
	{
		s_buff_size[0] = subaddress;
		vector_copy(buff, s_buff_size+1, len);
		retval = CMSIS_I2C_MASTER.MasterTransmit(address, s_buff_size, len+1, false);
	}

	while(!g_MasterCompletionFlag);
	g_MasterCompletionFlag = false;
	return retval;
}
// -------------------------------------------------------------------------------------

void GPIO_initialize(void)
{
	#ifndef MCXW71
	GPIO_HANDLE_DEFINE(rst_gpio_handle);
	hal_gpio_pin_config_t 	rst_gpio_config;
	#endif
	GPIO_HANDLE_DEFINE(dc_gpio_handle);
	hal_gpio_pin_config_t 	dc_gpio_config;

	#ifdef MCXC041
    CLOCK_EnableClock(kCLOCK_PortB);
    // OLED Address pin ----------------------------------------
    dc_gpio_config.direction	= kHAL_GpioDirectionOut;
    dc_gpio_config.level		= 0;
    dc_gpio_config.port			= 1;
    dc_gpio_config.pin			= 13;
    HAL_GpioInit(dc_gpio_handle, &dc_gpio_config);
    // ---------------------------------------------------------
    // Reset ---------------------------------------------------
    rst_gpio_config.direction	= kHAL_GpioDirectionOut;
    rst_gpio_config.level		= 0;
    rst_gpio_config.port		= 1;
    rst_gpio_config.pin			= 10;
    HAL_GpioInit(rst_gpio_handle, &rst_gpio_config);
    SDK_DelayAtLeastUs(500000, MAIN_CLOCK_FREQUENCY);
    HAL_GpioSetOutput(rst_gpio_handle, 1);
    // ---------------------------------------------------------

	#else
	#ifdef MCXC242
    CLOCK_EnableClock(kCLOCK_PortE);
    // OLED Address pin ----------------------------------------
    dc_gpio_config.direction	= kHAL_GpioDirectionOut;
    dc_gpio_config.level		= 0;
    dc_gpio_config.port			= 4;
    dc_gpio_config.pin			= 25;
    HAL_GpioInit(dc_gpio_handle, &dc_gpio_config);
    // ---------------------------------------------------------
    // Reset ---------------------------------------------------
    rst_gpio_config.direction	= kHAL_GpioDirectionOut;
    rst_gpio_config.level		= 0;
    rst_gpio_config.port		= 4;
    rst_gpio_config.pin			= 30;
    HAL_GpioInit(rst_gpio_handle, &rst_gpio_config);
    SDK_DelayAtLeastUs(500000, MAIN_CLOCK_FREQUENCY);
    HAL_GpioSetOutput(rst_gpio_handle, 1);
    // ---------------------------------------------------------

	#else
	#ifdef MCXC444
    CLOCK_EnableClock(kCLOCK_PortE);
    // OLED Address pin ----------------------------------------
    dc_gpio_config.direction	= kHAL_GpioDirectionOut;
    dc_gpio_config.level		= 0;
    dc_gpio_config.port			= 0;
    dc_gpio_config.pin			= 12;
    HAL_GpioInit(dc_gpio_handle, &dc_gpio_config);
    // ---------------------------------------------------------
    // Reset ---------------------------------------------------
    rst_gpio_config.direction	= kHAL_GpioDirectionOut;
    rst_gpio_config.level		= 0;
    rst_gpio_config.port		= 4;
    rst_gpio_config.pin			= 30;
    HAL_GpioInit(rst_gpio_handle, &rst_gpio_config);
    SDK_DelayAtLeastUs(500000, MAIN_CLOCK_FREQUENCY);
    HAL_GpioSetOutput(rst_gpio_handle, 1);
    // ---------------------------------------------------------

	#else
	#ifdef MCXA153
    RESET_PeripheralReset(kGPIO3_RST_SHIFT_RSTn);
    CLOCK_EnableClock(kCLOCK_GateGPIO3);
    // OLED Address pin ----------------------------------------
    dc_gpio_config.direction	= kHAL_GpioDirectionOut;
    dc_gpio_config.level		= 0;
    dc_gpio_config.port			= 3;
    dc_gpio_config.pin			= 12;
    HAL_GpioInit(dc_gpio_handle, &dc_gpio_config);
    // ---------------------------------------------------------
    // Reset ---------------------------------------------------
    rst_gpio_config.direction	= kHAL_GpioDirectionOut;
    rst_gpio_config.level		= 0;
    rst_gpio_config.port		= 3;
    rst_gpio_config.pin			= 1;
    HAL_GpioInit(rst_gpio_handle, &rst_gpio_config);
    SDK_DelayAtLeastUs(500000, MAIN_CLOCK_FREQUENCY);
    HAL_GpioSetOutput(rst_gpio_handle, 1);
    // ---------------------------------------------------------

	#else
	#ifdef MCXA156
    RESET_ReleasePeripheralReset(kGPIO3_RST_SHIFT_RSTn);
    CLOCK_EnableClock(kCLOCK_GateGPIO3);
    // OLED Address pin ----------------------------------------
    dc_gpio_config.direction	= kHAL_GpioDirectionOut;
    dc_gpio_config.level		= 0;
    dc_gpio_config.port			= 3;
    dc_gpio_config.pin			= 18;
    HAL_GpioInit(dc_gpio_handle, &dc_gpio_config);
    // ---------------------------------------------------------
    // Reset ---------------------------------------------------
    rst_gpio_config.direction	= kHAL_GpioDirectionOut;
    rst_gpio_config.level		= 0;
    rst_gpio_config.port		= 3;
    rst_gpio_config.pin			= 29;
    HAL_GpioInit(rst_gpio_handle, &rst_gpio_config);
    SDK_DelayAtLeastUs(500000, MAIN_CLOCK_FREQUENCY);
    HAL_GpioSetOutput(rst_gpio_handle, 1);
    // ---------------------------------------------------------

	#else
	#ifdef MCXN236
    CLOCK_EnableClock(kCLOCK_Gpio4);
    // OLED Address pin ----------------------------------------
    dc_gpio_config.direction	= kHAL_GpioDirectionOut;
    dc_gpio_config.level		= 0;
    dc_gpio_config.port			= 4;
    dc_gpio_config.pin			= 18;
    HAL_GpioInit(dc_gpio_handle, &dc_gpio_config);
    // ---------------------------------------------------------
    // Reset ---------------------------------------------------
    rst_gpio_config.direction	= kHAL_GpioDirectionOut;
    rst_gpio_config.level		= 0;
    rst_gpio_config.port		= 5;
    rst_gpio_config.pin			= 2;
    HAL_GpioInit(rst_gpio_handle, &rst_gpio_config);
    SDK_DelayAtLeastUs(500000, MAIN_CLOCK_FREQUENCY);
    HAL_GpioSetOutput(rst_gpio_handle, 1);
    // ---------------------------------------------------------
	#else

	#ifdef MCXN947
    CLOCK_EnableClock(kCLOCK_Gpio1);
    CLOCK_EnableClock(kCLOCK_Gpio3);
    // OLED Address pin ----------------------------------------
    dc_gpio_config.direction	= kHAL_GpioDirectionOut;
    dc_gpio_config.level		= 0;
    dc_gpio_config.port			= 3;
    dc_gpio_config.pin			= 19;
    HAL_GpioInit(dc_gpio_handle, &dc_gpio_config);
    // ---------------------------------------------------------
    // Reset ---------------------------------------------------
    rst_gpio_config.direction	= kHAL_GpioDirectionOut;
    rst_gpio_config.level		= 0;
    rst_gpio_config.port		= 1;
    rst_gpio_config.pin			= 3;
    HAL_GpioInit(rst_gpio_handle, &rst_gpio_config);
    SDK_DelayAtLeastUs(500000, MAIN_CLOCK_FREQUENCY);
    HAL_GpioSetOutput(rst_gpio_handle, 1);
    // ---------------------------------------------------------

	#else
	#if MCXW71
    CLOCK_EnableClock(kCLOCK_GpioC);
    // OLED Address pin ----------------------------------------
    dc_gpio_config.direction	= kHAL_GpioDirectionOut;
    dc_gpio_config.level		= 0;
    dc_gpio_config.port			= 2;
    dc_gpio_config.pin			= 4;
    HAL_GpioInit(dc_gpio_handle, &dc_gpio_config);
    SDK_DelayAtLeastUs(500000, MAIN_CLOCK_FREQUENCY);
    // ---------------------------------------------------------
	#else
    // OLED Address pin ----------------------------------------
    dc_gpio_config.direction	= kHAL_GpioDirectionOut;
    dc_gpio_config.level		= 0;
    dc_gpio_config.port			= 0;
    dc_gpio_config.pin			= 1;
    HAL_GpioInit(dc_gpio_handle, &dc_gpio_config);
    // ---------------------------------------------------------
    // Reset ---------------------------------------------------
    rst_gpio_config.direction	= kHAL_GpioDirectionOut;
    rst_gpio_config.level		= 0;
    rst_gpio_config.port		= 0;
    rst_gpio_config.pin			= 19;
    HAL_GpioInit(rst_gpio_handle, &rst_gpio_config);
    SDK_DelayAtLeastUs(500000, MAIN_CLOCK_FREQUENCY);
    HAL_GpioSetOutput(rst_gpio_handle, 1);
    // ---------------------------------------------------------

	#endif
	#endif
	#endif
	#endif
	#endif
	#endif
	#endif
	#endif
}

void I2C_clock(void)
{
	#ifdef MCXA153
	// Reset peripheral
	RESET_PeripheralReset(kLPI2C0_RST_SHIFT_RSTn);
	// Attach I2C clock
	CLOCK_SetClockDiv(kCLOCK_DivLPI2C0, 1u);
	CLOCK_AttachClk(kFRO_HF_DIV_to_LPI2C0);

	#else
	#ifdef MCXA156
	CLOCK_SetupFRO12MClocking();
	CLOCK_SetClockDiv(kCLOCK_DivLPI2C3, 1u);
	CLOCK_AttachClk(kFRO_HF_DIV_to_LPI2C3);

	#else
	#ifdef MCXN236
	CLOCK_SetClkDiv(kCLOCK_DivFlexcom2Clk, 1u);
	CLOCK_AttachClk(kFRO12M_to_FLEXCOMM2);

	#else
	#ifdef MCXN947
	CLOCK_SetClkDiv(kCLOCK_DivFlexcom3Clk, 1u);
	CLOCK_AttachClk(kFRO12M_to_FLEXCOMM3);

	#else
	#ifdef MCXW71
	CLOCK_SetIpSrc(kCLOCK_Lpi2c1, kCLOCK_IpSrcFro192M);
	CLOCK_SetIpSrcDiv(kCLOCK_Lpi2c1, kSCG_SysClkDivBy1);
	#else
	#ifdef RW612
	CLOCK_AttachClk(kSFRO_to_FLEXCOMM2);
	#endif
	#endif
	#endif
	#endif
	#endif
	#endif
}

int main(void)
{
	uint8_t repeat;

	I2C_clock();

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    // Initialize GPIO component ------------------------------------------------
    GPIO_initialize();
    // --------------------------------------------------------------------------

    // I2C CMSIS Driver ---------------------------------------------------------
    // Initialize the LPI2C master peripheral
    CMSIS_I2C_MASTER.Initialize(i2c_master_callback);
    CMSIS_I2C_MASTER.PowerControl(ARM_POWER_FULL);
    // Set I2C speed configuration as 1MHz
    CMSIS_I2C_MASTER.Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_FAST_PLUS);
    // --------------------------------------------------------------------------

    // OLED B click ---------------------------------------------------------
    // --------------------------------------------------------------------------
    oledBclick_init(oled_i2c_transfer_function);
    PRINTF("#######################################################\r\n");
    PRINTF("################## OLED B DEMO started ################\r\n");
    PRINTF("#######################################################\r\n");

    while(1) {
    	oledBclick_write_image(icon_5g);
    	SDK_DelayAtLeastUs(ICON_PERIOD_US, MAIN_CLOCK_FREQUENCY);
    	oledBclick_write_image(icon_hmi);
    	SDK_DelayAtLeastUs(ICON_PERIOD_US, MAIN_CLOCK_FREQUENCY);
    	oledBclick_write_image(icon_mcu);
    	SDK_DelayAtLeastUs(ICON_PERIOD_US, MAIN_CLOCK_FREQUENCY);
    	oledBclick_write_image(icon_wireless);
    	SDK_DelayAtLeastUs(ICON_PERIOD_US, MAIN_CLOCK_FREQUENCY);
    	for(repeat = 0; repeat < 10; repeat ++)
    	{
        	SDK_DelayAtLeastUs(ICON_PERIOD_US/3, MAIN_CLOCK_FREQUENCY);
        	oledBclick_enable_inverse(((repeat%2)==0));
    	}
    	oledBclick_enable_inverse(false);
    }
    return 0 ;
}
