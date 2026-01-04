/**
* @brief   MCXN947 General-Purpose DWT Performance Counter 
* @author  masa
* @version 1.00
*/

#ifndef __DWT_COUNTER_H__
#define __DWT_COUNTER_H__

#include "cmsis_armclang.h"
#include "fsl_device_registers.h"

#define DWT_COUNTER_MAX_LAP 24

typedef struct{
  uint32_t lap[DWT_COUNTER_MAX_LAP];
  uint32_t total;
}dwt_Counter_t;

typedef struct{
  uint32_t startCount;
  uint8_t curLap;
}dwt_Counter_Internal_t;

void DwtCounter_Init(void);
void DwtCounter_Hook(void);
void DwtCounter_SetLimit(uint32_t limit);
void DwtCounter_Lap(void);
void DwtCounter_End(void);

#endif /*__DWT_COUNTER_H__*/
