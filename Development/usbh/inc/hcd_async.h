/**
 * @brief   MCXN947V USB Host Controller Asynchronous Transfer Driver
 * @author  masa
 * @version 1.00 
 */

#ifndef __HCD_ASYNC_H__
#define __HCD_ASYNC_H__

#include "ehci.h"

#define HCD_ASYNC_NUM_OF_QH   8
#define HCD_ASYNC_NUM_OF_QTD  16
#define HCD_ASYNC_MSGBOX_SIZE 16

#define HcdAsync_IRQn   CAN0_IRQn

enum{
  ASYNC_USBINT = 1,
  ASYNC_TX_START = 2,
  ASYNC_SET_ADDRESS = 3,
};

enum{
  HCD_UNUSED,
  HCD_USED
};

typedef struct{
  uint8_t state;
  uint8_t devAddr;
  uint8_t epNum;
  uint8_t mainTxIdx;
  uint8_t setupIdx;
  uint8_t dataIdx;
  uint16_t lastTxSize;
  uint32_t* bufPointer;
  uint32_t* iocPointer;
}hcd_Async_QH_Mgr_t;

typedef struct{
  uint8_t state;
  uint8_t devAddr;
  uint8_t epNum;
}hcd_Async_qTD_Mgr_t;

typedef struct {
  uint8_t msgType;
  uint8_t devAddr;
  uint8_t epNum;
  uint16_t txLen;
}hcd_Async_Msg_t;

typedef struct{
  hcd_Async_Msg_t msg[HCD_ASYNC_MSGBOX_SIZE];
  uint8_t enqPtr;
  uint8_t deqPtr;
}hcd_Async_MsgBox_t;

void InitAsyncSchedule(void);
int32_t OpenAsyncEndpoint(uint8_t devAddr, uint8_t epNum, uint32_t* bufHead, usb_psiv_t psiv, uint16_t mps, uint8_t hubAddr, uint8_t hubPort, void func(uint8_t, uint8_t, uint16_t));
int32_t CloseAsyncEndpoint(uint8_t devAddr, uint8_t epNum);
int32_t HcdAsync_StartTransfer(uint8_t devAddr, uint8_t epNum, uint16_t txLen);
int32_t HcdAsync_SetAddress(uint8_t devAddr);



#endif /*__HCD_ASYNC_H__*/
