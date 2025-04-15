/**
* @brief   FRDM-MCXN947 board mikroBUS Serial Peripheral Interface Driver
* @author  masa
* @version 1.00
*/

#ifndef __MB_LPSPI_H__
#define __MB_LPSPI_H__

#include "fsl_lpflexcomm.h"
#include "fsl_lpspi.h"

#define SPI LPSPI6

void InitMikroBusSPI(void);

#endif /*__MB_LPSPI_H__*/
