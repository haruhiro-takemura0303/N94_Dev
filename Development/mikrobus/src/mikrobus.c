/**
* @brief   FRDM-MCXN947 board mikroBUS API
* @author  masa
* @version 1.00
*/

#include "mikrobus.h"

void InitMikroBUS(void)
{
  InitMikroBusPort(DEFUALT_MIKROBUS);
	GPIO1->PSOR = GPIO_PSOR_PTSO3_MASK;
	GPIO1->PCOR = GPIO_PCOR_PTCO3_MASK;
	for (int i = 0; i < 200000; i++){}
  InitMikroBusSPI();
}
