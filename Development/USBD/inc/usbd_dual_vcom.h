/**
* @brief   MCXN947 USB Device Dual Virtual Communication Device Class Driver Header
* @author  masa
* @version 1.00
*/

#ifndef __USBD_DUAL_VCOM_H__
#define __USBD_DUAL_VCOM_H__

#include "usbd_sys.h"
#include "usbdDualVcomDescriptor.h"

#define USBD_DUALVCOM_NUMOF_COM_IF 2
#define USBD_CDC_SIZEOF_LINECODING_STRUCT 7

typedef struct{
  uint32_t dwDTERate;
  uint8_t bCharFormat;
  uint8_t bParityType;
  uint8_t bDataBits;
}usb_CDC_LineCoding_t;

enum{
  BREQ_CDC_SET_LINE_CODING = 0x20,
  BREQ_CDC_GET_LINE_CODING,
  BREQ_CDC_SET_CONTROL_LINE_STATE,
};

typedef struct{
  usb_SetupPacket_t lastSetup;
  usb_CDC_LineCoding_t lineCoding[USBD_DUALVCOM_NUMOF_COM_IF];
}usbd_DualVcom_Info_t;

void InitDualVcom(void);

#endif /*__USBD_DUAL_VCOM_H__*/
