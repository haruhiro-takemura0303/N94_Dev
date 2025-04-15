/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "KeyClick4x4.h"

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void KeyClick_Task(void* arg);
/*******************************************************************************
 * Variables
 ******************************************************************************/


/* If timerHandler == NULL, no period task is made. */
int32_t KeyClick4x4_Init(
		keyClickHandler_t* pHandler,
		timer_handle_t timerHandler,
		hal_gpio_pin_config_t* cs_hal_gpio_pin_config,
		trasferFunction_t recvFun
		)
{
	uint8_t 	status = KEY_CLICK_ERROR_NONE;
	uint16_t 	buttonsValue = 0;
	for(;;)
	{

		if( !(pHandler->status & k_keyClickStarus_Init) )
		{

			if( (cs_hal_gpio_pin_config == NULL) || (recvFun == NULL) || (pHandler == NULL))
			{
				status |= KEY_CLICK_ERROR_INIVALID_PARAM;
				break;
			}

			status |= HAL_GpioInit(pHandler->chipSelectHandler, cs_hal_gpio_pin_config);

			pHandler->transFun = recvFun;
			pHandler->timerHandler = timerHandler;

			KeyClick4x4_GetInputs(pHandler);
			KeyClick4x4_GetInputs(pHandler);

			if(timerHandler != NULL)
			{
				status |= TM_Open(pHandler->timerHandler);
				status |= TM_InstallCallback(pHandler->timerHandler, KeyClick_Task, (void*)pHandler);
				status |= TM_Start(pHandler->timerHandler, (uint8_t)kTimerModeIntervalTimer, REFRESH_PERIOD);
			}

			pHandler->status |= k_keyClickStarus_Init;

			break;
		}
	}

	return status;

}

int32_t KeyClick4x4_Deinit(keyClickHandler_t* pHandler,
		trasferFunction_t recvFun)
{
	uint8_t status = KEY_CLICK_ERROR_NONE;
	if(pHandler->status & k_keyClickStarus_Init)
	{
		status |= HAL_GpioDeinit(pHandler->chipSelectHandler);
		status |= TM_Close(pHandler->timerHandler);
		pHandler->status &= ~k_keyClickStarus_Init;
	}
	return status;
}

int32_t KeyClick4x4_InstallCallback(keyClickHandler_t* pHandler,
		keyClickCallback_t callback)
{
	uint8_t status = KEY_CLICK_ERROR_NONE;
	if(callback != NULL)
	{
		pHandler->cb = callback;
	}
	else
	{
		status = KEY_CLICK_ERROR_INIVALID_PARAM;
	}

	return status;
}

keyClikSw_t KeyClick4x4_GetInputs(keyClickHandler_t* pHandler)
{
	/* Toggle CS to latch registers. */
	HAL_GpioSetOutput(pHandler->chipSelectHandler, 0U);
	HAL_GpioSetOutput(pHandler->chipSelectHandler, 1U);

	/* Read shift registers. */
	pHandler->transFun((void*)&pHandler->buttonsValue, TRANSFER_SIZE);

	/* Buttons have pull-up configuration. */
	pHandler->buttonsValue = ~pHandler->buttonsValue;
	return (keyClikSw_t)(pHandler->buttonsValue);

}

static void KeyClick_Task(void*arg)
{

	keyClickHandler_t * pHandler = (keyClickHandler_t*)arg;

	KeyClick4x4_GetInputs(pHandler);

	if( (pHandler->buttonsValue) && (pHandler->cb != NULL) )
	{
		pHandler->cb((keyClikSw_t)pHandler->buttonsValue);
	}
}

