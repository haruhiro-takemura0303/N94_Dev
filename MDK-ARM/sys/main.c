
/**
	*@brief		MCXN947 Development Project Main Program
	*@version	1.00
*/
#include "app.h"
#include "board.h"
#include "clock_config.h"
#include "pin_mux.h"

#include "usbd_dual_vcom.h"

#include "mikrobus.h"

int main (void)
{
	BOARD_InitHardware();
	LED_BLUE_OFF();
	LED_RED_OFF();
	LED_GREEN_OFF();

	/*USB Device*/
	InitDualVcom();
	
	/*mikroBUS*/
	InitMikroBUS();

	while(1){
	}
}
