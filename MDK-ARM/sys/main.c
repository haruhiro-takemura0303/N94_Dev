
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

uint8_t data[8];
uint32_t u32Data[2];
uint8_t txData[8] = {0x7A, 0x7A, 0x7D, 0xFD, 0x7F, 0x7F, 0, 0};
uint32_t txCount = 0;
uint32_t rxCount = 0;
int j = 0;

void SendTestMikrobusSPI(void)
{
	/*lpspi_transfer_t transfer = {
		.txData = txData,
		.rxData = data,
		.dataSize = 6,
		.configFlags = kLPSPI_MasterPcsContinuous
	};

	LPSPI_MasterTransferBlocking(SPI, &transfer);*/
	
	SPI->CR = (SPI->CR | (LPSPI_CR_RTF_MASK | LPSPI_CR_RRF_MASK));

	SPI->TCR = (SPI->TCR | (LPSPI_TCR_CONT_MASK | LPSPI_TCR_CONTC_MASK));
	while (SPI->FSR & LPSPI_FSR_TXCOUNT_MASK){
	}
	for (int i = 0; i < 8; i++){
		SPI->TDR = txData[i];
		while ((SPI->FSR & LPSPI_FSR_RXCOUNT_MASK) == 0){

		}
		while (SPI->FSR & LPSPI_FSR_RXCOUNT_MASK){
			if (SPI->SR & LPSPI_SR_RDF_MASK){
				data[i] = SPI->RDR;
			}
		}
	}
	
	SPI->TCR = (SPI->TCR & ~(LPSPI_TCR_CONT_MASK | LPSPI_TCR_CONTC_MASK));
	
}

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
	SendTestMikrobusSPI();
	
	while(1){
	}
}
