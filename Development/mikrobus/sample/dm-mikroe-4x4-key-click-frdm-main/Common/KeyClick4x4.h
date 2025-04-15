/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef KEYCLICK4X4_H_
#define KEYCLICK4X4_H_

#include "fsl_adapter_gpio.h"
#include "fsl_component_timer_manager.h"

#define TRANSFER_SIZE     		2U     /*! Transfer dataSize */


#define REFRESH_PERIOD			166U /* Timer timeout in milliseconds */

#define KEY_CLICK_ERROR_NONE			0U
#define KEY_CLICK_ERROR_INIVALID_PARAM	1U

typedef enum
{
	k_keyClikSw_9       = 0x0001,
	k_keyClikSw_A       = 0x0002,
	k_keyClikSw_B       = 0x0004,
	k_keyClikSw_C       = 0x0008,
	k_keyClikSw_E       = 0x0010,
	k_keyClikSw_F       = 0x0020,
	k_keyClikSw_10      = 0x0040,
	k_keyClikSw_D       = 0x0080,
	k_keyClikSw_1       = 0x0100,
	k_keyClikSw_2       = 0x0200,
	k_keyClikSw_3       = 0x0400,
	k_keyClikSw_4       = 0x0800,
	k_keyClikSw_5   	= 0x1000,
	k_keyClikSw_6       = 0x2000,
	k_keyClikSw_7 		= 0x4000,
	k_keyClikSw_8       = 0x8000,
}keyClikSw_t;


typedef int32_t (*trasferFunction_t)(void *data, uint32_t num);
typedef int32_t (*keyClickCallback_t)(keyClikSw_t keyMask);

typedef enum{
	k_keyClickStarus_Init 	= 0x1,
	k_keyClickStarus_First 	= 0x2,
}keyClickStatus_t;

typedef struct{
	timer_handle_t timerHandler;
	GPIO_HANDLE_DEFINE(chipSelectHandler);
	trasferFunction_t transFun;
	keyClickCallback_t cb;
	keyClikSw_t buttonsValue;
	keyClickStatus_t status;
}keyClickHandler_t;


int32_t KeyClick4x4_Init(keyClickHandler_t* pHandler,
		timer_handle_t timerHandler,
		hal_gpio_pin_config_t* cs_hal_gpio_pin_config,
		trasferFunction_t recvFun);

int32_t KeyClick4x4_Deinit(keyClickHandler_t* pHandler,
		trasferFunction_t recvFun);

int32_t KeyClick4x4_InstallCallback(keyClickHandler_t* pHandler,
		keyClickCallback_t callback);

keyClikSw_t KeyClick4x4_GetInputs(keyClickHandler_t* pHandler);

#endif /* KEYCLICK4X4_H_ */
