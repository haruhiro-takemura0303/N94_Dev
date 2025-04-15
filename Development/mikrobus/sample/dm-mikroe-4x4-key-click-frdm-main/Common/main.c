/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"

#include "fsl_clock.h"
#if(defined(MCXC041_SERIES) | defined(MCXC242_SERIES) | defined(MCXC444_SERIES) | defined(RW612_SERIES))
#include "fsl_spi_cmsis.h"
#else
#include "fsl_lpspi_cmsis.h"
#endif
#include <stdbool.h>

#include "KeyClick4x4.h"

#include "config.h"

#if(defined(RW612_SERIES))
#include "clock_config.h"
#endif
/*******************************************************************************
 * Definitions
 ******************************************************************************/


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
int32_t KeyClickCallback(keyClikSw_t keyMask);
int32_t recvFun(void *data, uint32_t num);

/*******************************************************************************
 * Variables
 ******************************************************************************/
const uint8_t sw_character[] =
{
		'7','8','9','C',
		'0','#','D','*',
		'1','2','3','A',
		'4','5','6','B'
};

static keyClickHandler_t keyHandler;
static keyClickHandler_t* keyHandlerPtr = &keyHandler;

static TIMER_MANAGER_HANDLE_DEFINE(s_timerHandler);

const hal_gpio_pin_config_t keyClick4x4Cs_pinConfig =
{
		kHAL_GpioDirectionOut,
		KEYCLIK_CS_GPIO_LEVEL,
		KEYCLIK_CS_GPIO_PORT,
		KEYCLIK_CS_GPIO_PIN
};
#if(defined(RW612_SERIES))
static const IRQn_Type s_ctimerIRQ[] = CTIMER_IRQS;
volatile bool isTransferCompleted = false;
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/
uint32_t SPI_GetFreq(void)
{
    return SPI_CLOCK_FREQ;
}


void LPSPI_MasterSignalEvent_t(uint32_t event)
{
	(void)event;

#if(defined(RW612_SERIES))
	isTransferCompleted = true;
#endif

}
/*!
 * @brief Main function
 */
int main(void)
{
    char ch;
    timer_config_t timerConfig;

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    PRINTF("hello world.\r\n");

    SPI_CLOCK_ENABLE;

    TMR_CLOCK_ENABLE;

    timerConfig.instance 	= TIMER_INSTANCE;
    timerConfig.srcClock_Hz = TIMER_SOURCE_CLOCK;

    TM_Init(&timerConfig);

    /*LPSPI master init*/
    DRIVER_SPI.Initialize(LPSPI_MasterSignalEvent_t);
    DRIVER_SPI.PowerControl(ARM_POWER_FULL);
    DRIVER_SPI.Control(ARM_SPI_MODE_MASTER, TRANSFER_BAUDRATE);

#if(defined(RW612_SERIES))
    /*
     * Since CTIMER by default has more priority than FLEXCOMM
     * the priority is update in order to receive interrupts.
     */
    IRQ_SetPriority(s_ctimerIRQ[TIMER_INSTANCE], 1U);
#endif

    KeyClick4x4_Init(
    		keyHandlerPtr,
			s_timerHandler,
			(hal_gpio_pin_config_t*)&keyClick4x4Cs_pinConfig,
    		recvFun);
    KeyClick4x4_InstallCallback(
    		keyHandlerPtr,
    		KeyClickCallback);

    while (1)
    {
        ch = GETCHAR();
        PUTCHAR(ch);
    }
}

int32_t recvFun(void *data, uint32_t num)
{

	uint32_t status =  DRIVER_SPI.Receive(data, num);
#if(defined(RW612_SERIES))
	while (!isTransferCompleted)
	{
	}
	isTransferCompleted = false;
#endif
	return status;
}

int32_t KeyClickCallback(keyClikSw_t keyMask)
{
	uint8_t i = 0;

	for(i = 0; i < 16; i++)
	{
		if( (keyMask >> i) & 0x1)
		{
			PRINTF("SW Press: %c\n\r", sw_character[i]);
		}
	}
}
