
#ifndef __MB_GYRO2_H__
#define __MB_GYRO2_H__

#include "fsl_lpflexcomm.h"
#include "fsl_lpi2c.h"
#include "fsl_edma.h"
#include "mb_pin.h"

enum{
  kI2C_TransmitData                         = (0b000 << 8),
  kI2C_ReceiveData                          = (0b001 << 8),
  kI2C_GenStop                              = (0b010 << 8),
  kI2C_ReceiveDataDiscard                   = (0b011 << 8),
  kI2C_GenStartAndSendAddress               = (0b100 << 8),
  kI2C_GenStartAndSendAddressExpectNack     = (0b101 << 8),
  kI2C_GenStartAndSendAddressHS             = (0b110 << 8),
  kI2C_GenStartAndSendAddressExpectNackHS   = (0b111 << 8),
};

void InitGyro2(mikrobus_hdr_t hdr, uint8_t instNum, uint8_t txCh, uint8_t rxCh);

#endif /*__MB_GYRO2_H__*/
