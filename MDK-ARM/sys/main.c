
/**
	*@brief		MCXN947 Development Project Main Program
	*@version	1.00
*/
#include "app.h"
#include "board.h"
#include "clock_config.h"
#include "pin_mux.h"

#include "usbd_dual_vcom.h"

int main (void)
{
	BOARD_InitHardware();

	InitDualVcom();
	
	while(1){
	}
}
