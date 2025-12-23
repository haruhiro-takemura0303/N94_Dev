
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
	for (int i = 0; i < 256; i++){
		send[i] = i;
	}
	//W25Q64_Erase4K(0);
	//W25Q64_ProgramPage(0x100, send, 256);
	flexspi_transfer_t flashXfer;
    flashXfer.deviceAddress = 0;
    flashXfer.port          = kFLEXSPI_PortA1;
    flashXfer.cmdType       = kFLEXSPI_Read;
    flashXfer.SeqNumber     = 1;
    flashXfer.seqIndex      = LUT_READ_QIO;
    flashXfer.data          = (uint32_t *)rec;
    flashXfer.dataSize      = 256;
	FLEXSPI_TransferBlocking(FLEXSPI0, &flashXfer);
	//W25Q64_Read(0, rec, 256);
	//W25Q64_ReadSR1(&sr1);
	//W25Q64_ReadSR2(&sr2);
	
	
	while(1){
	}
}

void HardFault_Handler(void)
{
	__BKPT(0);
	while(1){
	}
}
