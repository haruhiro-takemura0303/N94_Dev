
/**
	*@brief		MCXN947 Development Project Main Program
	*@version	1.00
*/

#include "app.h"
#include "board.h"
#include "clock_config.h"
#include "pin_mux.h"

#include "usbd_dual_vcom.h"
#include "hcd.h"

#include "mikrobus.h"

#define NUMOF_SYS_VECT	16
#define NUMOF_EXT_VECT	156
#define NUMOF_VECT			(NUMOF_SYS_VECT + NUMOF_EXT_VECT)

__attribute__((section(".ramx"))) __ALIGNED(128) uint32_t vectorOnRam[NUMOF_VECT];

static void sysInit(void)
{
	uint32_t* curVect = (uint32_t*)(SCB->VTOR);
	for (int i = 0; i < NUMOF_VECT; i++){
		vectorOnRam[i] = curVect[i];
	}
	SCB->VTOR = (uint32_t)(&vectorOnRam[0]);
  __DSB();
}

int main (void)
{
	sysInit();
	
	BOARD_InitHardware();
	LED_BLUE_OFF();
	LED_RED_OFF();
	LED_GREEN_OFF();

	/*USB Device*/
	InitDualVcom();
	
	/*USB Host(Enhanced Host Controller Interface)*/
	InitEHCI();
	
	/*mikroBUS*/
	InitMikroBUS();

	while(1){
	}
}
