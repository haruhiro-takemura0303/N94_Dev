/**
* @brief   FRDM-MCXN947 board mikroBUS API
* @author  masa
* @version 1.00
*/

#include "mikrobus.h"

void InitMikroBUS(void)
{
  InitCapTouch2(DEFAULT_MIKROBUS, eDMA_INST0, eDMA_CH0, eDMA_CH1);
  //InitGyro2(DEFAULT_MIKROBUS, eDMA_INST0, eDMA_CH2, eDMA_CH3);
}
