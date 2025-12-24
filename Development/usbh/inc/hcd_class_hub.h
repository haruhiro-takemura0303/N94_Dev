/**
* @brief   MCXN947V USB Host Controller Hub Class Driver 
* @author  masa
* @version 1.00 
*/

#ifndef __HCD_CLASS_HUB_H__
#define __HCD_CLASS_HUB_H__

#include "hcd_class.h"

#define USB_CLASSCODE_HUB         0x09
#define NUM_OF_MAX_HUB_DEVICE     1
#define HCD_HUB_MSGBOX_SIZE 32
#define HCD_HUB_MAX_NUM_OF_ALTSET 2
#define HCD_HUB_MAX_NUM_OF_PORT   4
#define HCD_HUB_REQ_BOX_SIZE      16

#define HcdHub_IRQn               HSCMP1_IRQn

enum {
  HCD_HUB_PORT_CONNECTION       = 0,
  HCD_HUB_PORT_ENABLE           = 1,
  HCD_HUB_PORT_SUSPEND          = 2,
  HCD_HUB_PORT_OVER_CURRENT     = 3,
  HCD_HUB_PORT_RESET            = 4,
  
  HCD_HUB_PORT_POWER            = 8,
  HCD_HUB_PORT_LOW_SPEED        = 9,
  
  HCD_HUB_C_PORT_CONNECTION     = 16,
  HCD_HUB_C_PORT_ENABLE         = 17,
  HCD_HUB_C_PORT_SUSPEND        = 18,
  HCD_HUB_C_PORT_OVER_CURRENT   = 19,
  HCD_HUB_C_PORT_RESET          = 20,
  
  HCD_HUB_PORT_TEST             = 21,
  HCD_HUB_PORT_INDICATOR        = 22
};

typedef struct{
  usbDesc_Interface_t* intfDesc;
  usbDesc_Endpoint2_t* epDesc;
} hcd_Hub_AltSet_t;

typedef union{
  struct{
    struct{
      uint16_t curConnStat:1;
      uint16_t portEnDis:1;
      uint16_t suspend:1;
      uint16_t ovCurr:1;
      uint16_t portReset:1;
      uint16_t RESERVED0:3;
      uint16_t portPower:1;
      uint16_t lsAttached:1;
      uint16_t hsAttached:1;
      uint16_t testMode:1;
      uint16_t indicatorCtrl:1;
      uint16_t RESERVED:3;
    }portStatus;
    struct{
      uint16_t connStatChg:1;
      uint16_t portEnDisChg:1;
      uint16_t suspChg:1;
      uint16_t ovCurrChg:1;
      uint16_t rstChg:1;
      uint16_t RESERVED:11;
    }portChange;
  }BIT;
  uint32_t DWORD;
}usb_Hub_PortStatus_t;

typedef struct{
  uint8_t pwrConfirmed:1;
  uint8_t connConfirmed:1;
  uint8_t RESERVED:6;
}hcd_Hub_PortManager_t;

typedef struct{
  struct{
    uint8_t hubAddr:4;
    uint8_t hubPort:4;
  }BIT;
  uint8_t BYTE;
}hcd_Hub_TimerCompVal_t;

typedef struct{
  uint8_t portNum;
  uint8_t pendPrio;
  hcd_Hub_PortManager_t portMgr;
  usb_Hub_PortStatus_t portStatus;
}hcd_Hub_Port_Info_t;

typedef struct{
  hcd_DeviceInfo_t* device;
  hcd_Hub_AltSet_t altSet[HCD_HUB_MAX_NUM_OF_ALTSET];
  hcd_Hub_AltSet_t* curAltSet;
  usbDesc_Hub_t hubDescBuf;
  hcd_Hub_Port_Info_t portInfo[HCD_HUB_MAX_NUM_OF_PORT];
} hcd_Hub_Info_t;

typedef struct{
  usb_SetupPacket_t setup;
  usb_Hub_PortStatus_t data;
} hcd_Hub_Request_Buf_t;

typedef struct{
  uint8_t msgType;
  uint8_t devAddr;
  uint8_t portNum;
  uint32_t count_ms;
  void (*timerCb)(uint8_t miscVal);
  usb_SetupPacket_t setup;
} hcd_Hub_Msg_t;

typedef struct{
  hcd_Hub_Msg_t msg[HCD_HUB_MSGBOX_SIZE];
  uint8_t enqPtr;
  uint8_t deqPtr;
} hcd_Hub_MsgBox_t;

enum{
  HCD_HUB_CTRL_REQ = 1,
  HCD_HUB_CTRL_REQ_DONE,
  HCD_HUB_INTR_RECEIVED,
  HCD_HUB_PORT_PEND_RELEASE,
  HCD_HUB_TIMER_REQ,
};

#endif /*__HCD_CLASS_HUB_H__*/
