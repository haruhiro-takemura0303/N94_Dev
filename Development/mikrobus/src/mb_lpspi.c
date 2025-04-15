/**
* @brief   FRDM-MCXN947 board mikroBUS Serial Peripheral Interface Driver
* @author  masa
* @version 1.00
*/

#include "mb_lpspi.h"

void InitMikroBusSPI(void)
{
  CLOCK_SetClkDiv(kCLOCK_DivFlexcom6Clk, 1u);
  CLOCK_AttachClk(kFRO12M_to_FLEXCOMM6);

  /*FLEXCOMM Enable*/
  CLOCK_EnableClock(kCLOCK_LPFlexComm6);

  /*FLEXCOMM SPI Enable*/
  LP_FLEXCOMM6->PSELID |= LP_FLEXCOMM_PSELID_PERSEL(0b010);
  LP_FLEXCOMM6->PSELID |= LP_FLEXCOMM_PSELID_LOCK_MASK;

  /*LPSPI Clock Enable*/
  CLOCK_EnableClock(kCLOCK_LPSpi6);

  lpspi_master_config_t _masterConfig = {
    .baudRate = 3000000,
    .bitsPerFrame = 8,
    .cpol = kLPSPI_ClockPolarityActiveLow,
    .cpha = kLPSPI_ClockPhaseSecondEdge,
    .direction = kLPSPI_MsbFirst,
    .pcsToSckDelayInNanoSec = 15000,
    .lastSckToPcsDelayInNanoSec = 15000,
    .betweenTransferDelayInNanoSec = 200,
    .whichPcs = kLPSPI_Pcs0,
    .pcsActiveHighOrLow = kLPSPI_PcsActiveLow,
    .pinCfg = kLPSPI_SdiInSdoOut,
    .pcsFunc = kLPSPI_PcsAsCs,
    .dataOutConfig = kLpspiDataOutRetained,
    .enableInputDelay = false
  };
  
  LPSPI_MasterInit(SPI, &_masterConfig, 12000000);

}
