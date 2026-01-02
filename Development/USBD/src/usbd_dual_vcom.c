/**
* @brief   MCXN947 USB Device Dual Virtual Communication Device Class Driver
* @author  masa
* @version 1.00
*/

#include "usbd_dual_vcom.h"

usbd_DualVcom_Info_t stDualVcom;
/*static*/ uint8_t stCmdBuf[2][32];
/*static*/ uint8_t stCdcTxBuf[2][1024];
/*static*/ uint8_t stCdcRxBuf[2][1024];
/*static*/ struct{
  uint8_t idx;
  uint8_t lineBuf[256];
} stCommandLine[2];

#ifndef VSCODE
__STATIC_FORCEINLINE usbd_DualVcom_Info_t* vcom()
#else
usbd_DualVcom_Info_t* vcom()
#endif
{
  return &stDualVcom;
}

static usbDcd_Status_t setupHandler(usb_SetupPacket_t* setup)
{
  usbDcd_Status_t ret = USBD_OK;
	uint16_t actTxLen;
  if (setup){
    memcpy(&vcom()->lastSetup, setup, sizeof(usb_SetupPacket_t));
  }
  switch(vcom()->lastSetup.BIT.bRequest){
    case(BREQ_CDC_SET_LINE_CODING):{
      Usbd_StartNextTransfer(USBD_EP0_OUT, USB_IOC_ENABLE, vcom()->lastSetup.BIT.wLength);
      break;
    }
    case(BREQ_CDC_GET_LINE_CODING):{
      if (setup->BIT.wIndex == USB_VCOM0_IF_NUM){
        Usbd_WriteEp0Buffer(&vcom()->lineCoding[0], USBD_CDC_SIZEOF_LINECODING_STRUCT);
      } else if (setup->BIT.wIndex == USB_VCOM1_IF_NUM){
        Usbd_WriteEp0Buffer(&vcom()->lineCoding[1], USBD_CDC_SIZEOF_LINECODING_STRUCT);
      }
			actTxLen = vcom()->lastSetup.BIT.wLength;
			if (actTxLen > USBD_CDC_SIZEOF_LINECODING_STRUCT){
				actTxLen = USBD_CDC_SIZEOF_LINECODING_STRUCT;
			}
      Usbd_StartNextTransfer(USBD_EP0_IN, USB_IOC_ENABLE, actTxLen);
      break;
    }
    case(BREQ_CDC_SET_CONTROL_LINE_STATE):{
      Usbd_StartNextTransfer(USBD_EP0_IN, USB_IOC_ENABLE, 0);
      break;
    }
    default:{
      ret = USBD_UNSUPPORTED_REQ;
      break;
    }
  }
  return ret;
}

static usbDcd_Status_t dataStatHandler(usbDcd_Control_Dir_t dir)
{
  usbDcd_Status_t ret = USBD_OK;
  static usb_CDC_LineCoding_t newLineCoding;
  if (dir == EP0_OUT_TRANSFER){
    switch(vcom()->lastSetup.BIT.bRequest){
      case(BREQ_CDC_SET_LINE_CODING):{
        Usbd_ReadEp0Buffer(&newLineCoding, USBD_CDC_SIZEOF_LINECODING_STRUCT);
        break;
      }
      case(BREQ_CDC_GET_LINE_CODING):{
        break;
      }
      case(BREQ_CDC_SET_CONTROL_LINE_STATE):{
        break;
      }
      default:{
        ret = USBD_UNSUPPORTED_REQ;
        break;
      }
    }
  } else if (dir == EP0_IN_TRANSFER){
    switch(vcom()->lastSetup.BIT.bRequest){
      case(BREQ_CDC_SET_LINE_CODING):{
        if (vcom()->lastSetup.BIT.wIndex == USB_VCOM0_IF_NUM){
          memcpy(&vcom()->lineCoding[0], &newLineCoding, sizeof(usb_CDC_LineCoding_t));
        }else if (vcom()->lastSetup.BIT.wIndex == USB_VCOM1_IF_NUM){
          memcpy(&vcom()->lineCoding[1], &newLineCoding, sizeof(usb_CDC_LineCoding_t));
        }
        break;
      }
      case(BREQ_CDC_GET_LINE_CODING):{

        break;
      }
      case(BREQ_CDC_SET_CONTROL_LINE_STATE):{
        break;
      }
      default:{
        ret = USBD_UNSUPPORTED_REQ;
        break;
      }
    }
  }
  return ret;
}

static void configured()
{
  Usbd_StartNextTransfer(USB_CDC0_DATAOUTEP_ADDR, USB_IOC_ENABLE, USB_CDC_DATAEP_MPS);
  Usbd_StartNextTransfer(USB_CDC1_DATAOUTEP_ADDR, USB_IOC_ENABLE, USB_CDC_DATAEP_MPS);
}

static void cmdHandler(uint8_t comIdx, uint16_t size)
{
}

static void dataOutHandler(uint8_t comIdx, uint16_t size)
{
	uint8_t snap[512];
	memcpy(snap, &stCdcRxBuf[comIdx][0], size);
  if (vcom()->outCallback[comIdx]){
    vcom()->outCallback[comIdx](comIdx, snap, size);
  }
  Usbd_StartNextTransfer(USB_CDC0_DATAOUTEP_ADDR + (comIdx * 2), USB_IOC_ENABLE, USB_CDC_DATAEP_MPS);
}

static void dataInHandler(uint8_t comIdx, uint16_t size)
{
  if (vcom()->inCallback[comIdx]){
    vcom()->inCallback[comIdx](comIdx, &stCdcRxBuf[comIdx][0], size);
  }  
}

static void vcom0CmdHandler(uint16_t size)
{
  cmdHandler(0, size);
}

static void vcom1CmdHandler(uint16_t size)
{
  cmdHandler(1, size);
}

static void cdc0DataOutHandler(uint16_t size)
{
  dataOutHandler(0, size);
}

static void cdc1DataOutHandler(uint16_t size)
{
  dataOutHandler(1, size);
}

static void cdc0DataInHandler(uint16_t size)
{
  dataInHandler(0, size);
}

static void cdc1DataInHandler(uint16_t size)
{
  dataInHandler(1, size);
}

void DualVcom_SetOutCallBack(uint8_t idx, cdcCallback_t func)
{
  if (idx >= USBD_DUALVCOM_NUMOF_COM_IF){
    return;
  }
  vcom()->outCallback[idx] = func;
}

void DualVcom_SetInCallBack(uint8_t idx, cdcCallback_t func)
{
  if (idx >= USBD_DUALVCOM_NUMOF_COM_IF){
    return;
  }
  vcom()->inCallback[idx] = func;
}

void DualVcom_SetPortCallBack(portEnabledCallback_t func)
{
  vcom()->portCallback = func;
}

void InitDualVcom(void)
{
  Usbd_SysInit();
  for (int i = 0; i < USBD_DUALVCOM_NUMOF_COM_IF; i++){
    vcom()->lineCoding[i].dwDTERate = 115200;
    vcom()->lineCoding[i].bCharFormat = 0;
    vcom()->lineCoding[i].bParityType = 0;
    vcom()->lineCoding[i].bDataBits = 8;
  }
  UsbdDualVcom_InitDescriptor();
  Usbd_SetClassRequestHandler(setupHandler, dataStatHandler);
  Usbd_SetConfiguredFunc(configured);
  
  Usbd_OpenEndpoint(USB_VCOM0_CMDEP_ADDR, USB_VCOM_CMDEP_ATTR, USB_VCOM_CMDEP_MPS, 0, &stCmdBuf[0][0], vcom0CmdHandler);
  Usbd_OpenEndpoint(USB_VCOM1_CMDEP_ADDR, USB_VCOM_CMDEP_ATTR, USB_VCOM_CMDEP_MPS, 0, &stCmdBuf[1][0], vcom1CmdHandler);
  
  Usbd_OpenEndpoint(USB_CDC0_DATAOUTEP_ADDR, USB_CDC_DATAEP_ATTR, USB_CDC_DATAEP_MPS, 0, &stCdcRxBuf[0][0], cdc0DataOutHandler);
  Usbd_OpenEndpoint(USB_CDC1_DATAOUTEP_ADDR, USB_CDC_DATAEP_ATTR, USB_CDC_DATAEP_MPS, 0, &stCdcRxBuf[1][0], cdc1DataOutHandler);
  
  Usbd_OpenEndpoint(USB_CDC0_DATAINEP_ADDR, USB_CDC_DATAEP_ATTR, USB_CDC_DATAEP_MPS, 0, &stCdcTxBuf[0][0], cdc0DataInHandler);
  Usbd_OpenEndpoint(USB_CDC1_DATAINEP_ADDR, USB_CDC_DATAEP_ATTR, USB_CDC_DATAEP_MPS, 0, &stCdcTxBuf[1][0], cdc1DataInHandler);
  
  Usbd_SysStart();
}

usbDcd_Status_t DualVcom_StartInTransfer(uint8_t comIdx, const void* buf, uint16_t len)
{
  usbDcd_Status_t ret;

  ret = Usbd_Idle(USB_CDC0_DATAINEP_ADDR + (comIdx * 2));
  if (ret){
    return ret;
  }
  memcpy(&stCdcTxBuf[comIdx][0], buf, len);
  return Usbd_StartNextTransfer(USB_CDC0_DATAINEP_ADDR + (comIdx * 2), USB_IOC_ENABLE, len);
}

