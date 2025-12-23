
/**
*@brief		MCXN947 Development Project Main Program
*@version	1.00
*/

#include "app.h"
#include "board.h"
#include "clock_config.h"
#include "pin_mux.h"

#include "usbd_dual_vcom.h"
#include "vcom_writer.h"

#include "hcd.h"

#include "mikrobus.h"

#include "flexspi_w25q64.h"

#define NUMOF_SYS_VECT	16
#define NUMOF_EXT_VECT	156
#define NUMOF_VECT			(NUMOF_SYS_VECT + NUMOF_EXT_VECT)

uint8_t send[256];
uint8_t rec[256];

uint32_t dword;
uint16_t word;
uint8_t byte;

uint8_t sr1;
uint8_t sr2;

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
	InitVcomWriter();
	InitDualVcom();
	
	/*USB Host(Enhanced Host Controller Interface)*/
	//InitEHCI();
	
	/*mikroBUS*/
	InitMikroBUS();
	
	/*FlexSPI(Quad) Onboard W25Q64*/
	InitQSPI_FlexSpi0();
	//W25Q64_Read(0, rec, 256);
	FLEXSPI_Read(FLEXSPI_AHB_BASE, rec, 256);
	dword = *(uint32_t*)(FLEXSPI_AHB_BASE);
	word = *(uint16_t*)(FLEXSPI_AHB_BASE);
	byte = *(uint8_t*)(FLEXSPI_AHB_BASE);
	while(1){
	}
}

void HardFault_Handler(void)
{
	__BKPT(0);
	while(1){
	}
}
