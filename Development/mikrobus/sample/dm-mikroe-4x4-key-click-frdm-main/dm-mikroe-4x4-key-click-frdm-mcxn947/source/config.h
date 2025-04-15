/*
 * Copyright 2024 NXP
 *  
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#ifndef CONFIG_H_
#define CONFIG_H_

#include "fsl_clock.h"

#define CORE_CLOCK_HZ					CLOCK_GetFreq(kCLOCK_CoreSysClk)


#define Driver_I2C 						Driver_I2C2
#define I2C_MASTER_CLOCK_FREQUENCY 		CLOCK_GetLPFlexCommClkFreq(2u)

#define I2C_GetFreq						LPI2C2_GetFreq

								/* Attach peripheral clock */
#define I2C_CLOCK_ENABLE		CLOCK_SetClkDiv(kCLOCK_DivFlexcom2Clk, 1u);	\
								CLOCK_AttachClk(kFRO12M_to_FLEXCOMM2);

#define DRIVER_SPI       		Driver_SPI6

#define SPI_GetFreq				LPSPI6_GetFreq

#define SPI_CLOCK_FREQ			CLOCK_GetLPFlexCommClkFreq(6u)
								/* Attach peripheral clock */
#define SPI_CLOCK_ENABLE		CLOCK_SetClkDiv(kCLOCK_DivFlexcom6Clk, 1u); \
								CLOCK_AttachClk(kFRO12M_to_FLEXCOMM6);

#define TRANSFER_BAUDRATE 		1000000U /*! Transfer baudrate - 1M */

#define TIMER_INSTANCE          0

#define TIMER_SOURCE_CLOCK 		(CLOCK_GetCTimerClkFreq(0U))

#define TMR_CLOCK_ENABLE		CLOCK_SetClkDiv(kCLOCK_DivCtimer0Clk, 1u);\
								CLOCK_AttachClk(kFRO_HF_to_CTIMER0);

#define KEYCLIK_CS_GPIO_LEVEL	1
#define KEYCLIK_CS_GPIO_PORT	3U
#define KEYCLIK_CS_GPIO_PIN		23U

#endif /* CONFIG_H_ */
