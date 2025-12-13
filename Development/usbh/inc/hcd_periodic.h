/**
* @brief   MCXN947V USB Host Controller Simple Periodic Transfer Driver
* @author  masa
* @version 1.00 
*/

#ifndef __HCD_PERIODIC_H__
#define __HCD_PERIODIC_H__

#include "ehci.h"


#define HCD_PERIODIC_MAX_NUM_OF_INTR_EP  2
#define HCD_PERIODIC_MAX_NUM_OF_ISOCHOUT_EP  1
#define HCD_PERIODIC_MAX_NUM_OF_ISOCHIN_EP  1
#define HCD_PERIODIC_iTD_SINGLE_BUF 4
#define HCD_PERIODIC_PFL_SIZE 8

#define HCD_PERIODIC_MSGBOX_SIZE 32

#define HcdPeriodic_IRQn EWM0_IRQn

typedef struct{
	struct{
		ehci_iTD_t iTD[HCD_PERIODIC_iTD_SINGLE_BUF];
	} doubleBuf[2];
} hcd_Periodic_iTD_buf_t;

typedef struct{
  struct{
    uint8_t mFrame[8];
  }frame[4];
}hcd_Periodic_IsochIn_ActTxInfo_t;

typedef union{
  struct{
    uint8_t isochIn[2];
    uint8_t isochOut[2];
  }ep;
  uint32_t whole;
}hcd_Periodic_Isoch_Map_t;

typedef struct{
  uint8_t state;
  uint8_t devAddr;
  uint8_t epNum;
  uint8_t bytePerSample;
  struct{
    uint8_t esitCountMAX;
    uint8_t esitCount;
    uint8_t bytePerESIT_Even;
    uint8_t bytePerESIT_Odd;
    uint8_t sampleRemainder;
  } outParam;
  struct{
    uint8_t bytePerESIT;
    hcd_Periodic_IsochIn_ActTxInfo_t dataBuf[2];
  }inParam;
  uint32_t *buf[2];
  void (*completeCallback)(uint8_t devAddr, uint8_t epNum, uint16_t nextTxSize, uint32_t* bufPtr, hcd_Periodic_IsochIn_ActTxInfo_t* inTxMap);
} hcd_Periodic_Isoch_Mgr_t;

typedef struct{
  uint8_t state;
  uint8_t devAddr;
  uint8_t epNum;
  uint16_t lastTxSize;
  uint32_t* bufPointer;
}hcd_Periodic_QH_Mgr_t;

typedef struct {
  uint8_t msgType;
  uint8_t devAddr;
  uint8_t epNum;
  uint16_t txLen;
}hcd_Periodic_Msg_t;

typedef struct{
  hcd_Periodic_Msg_t msg[HCD_PERIODIC_MSGBOX_SIZE];
  uint8_t enqPtr;
  uint8_t deqPtr;
}hcd_Periodic_MsgBox_t;

enum{
  PERIODIC_USBINT = 1,
  PERIODIC_INTR_TX_START = 2,
  PERIODIC_ISOCH_TX_START = 3,
};

void InitPeriodicSchedule(void);
hcd_Status_t HcdPeriodic_StartIsochronousTransfer(uint8_t devAddr, uint8_t epNum);
hcd_Status_t HcdPeriodic_StartInterruptTransfer(uint8_t devAddr, uint8_t epNum, uint16_t txLen);
hcd_Status_t OpenIsochronousEndpoint(uint8_t devAddr, uint8_t epNum, uint16_t mps, usb_psiv_t speed, uint32_t samFreq, uint8_t bytePerSample, uint32_t* buf0, uint32_t* buf1, void func(uint8_t, uint8_t, uint16_t, uint32_t*, hcd_Periodic_IsochIn_ActTxInfo_t*));
hcd_Status_t OpenInterruptEndpoint(hcd_DeviceInfo_t* device, uint8_t epNum, uint32_t* bufHead, uint16_t mps, void func(uint8_t, uint8_t, uint16_t));
hcd_Status_t CloseIsochronousEndpoint(uint8_t devAddr, uint8_t epNum);
hcd_Status_t CloseInterruptEndpoint(uint8_t devAddr, uint8_t epNum);

#endif
