/**
* @brief   MCXN947V USB Host Controller Driver
* @author  masa
* @version 1.00 
*/

#ifndef __HCD_H__
#define __HCD_H__

#include "ehci.h"
#include "hcd_async.h"
#include "hcd_periodic.h"

#define HCD_MSGBOX_SIZE 32
#define HCD_IRQn        CAN1_IRQn

typedef struct{
  uint8_t devAddr;
  uint8_t datDir;
  uint16_t datSize;
  usb_SetupPacket_t setup;
  void (*txCompCb)(uint8_t devAddr, uint32_t* bufPtr, uint16_t txLen);
} ctrl_info_t;

typedef struct{
  uint8_t rawDesc[1024];
  uint16_t fullLength;
  uint16_t readPtr;
} config_rawdesc_t;

typedef struct{
  uint16_t langID;
  uint8_t venderStrID;
  uint8_t venderStr[128];
  uint8_t productStrID;
  uint8_t productStr[128];
}string_info_t;

enum{
  HCDMSG_CTRL = 0,
  HCDMSG_PARSE_CONFIG,
  HCDMSG_GPTIMER,
  HCDMSG_GPTIMER_COMPLETE,
  HCDMSG_INIT_DEVICE,
  HCDMSG_INIT_CLASS,
};

#define GPTIMER_COMPLETE  0xFFFFFFFF

typedef struct{
  uint32_t type;
  union{
    struct{
      hcd_DeviceInfo_t* device;
      usb_SetupPacket_t setup;
      uint32_t* sendDataBuf;
      void (*completeCb)(uint16_t transLen, uint8_t devAddr, uint32_t* ep0Buf);
    } ctrl;
    struct{
      hcd_DeviceInfo_t* device;
      uint32_t rsvd[4];
    } parse_config;
    struct{
      uint32_t count_us;
      void (*completeCb)(uint8_t);
      uint8_t miscVal;
      uint8_t rsvd[11];
    } gp_timer;
    struct{
      uint8_t devAddr;
      uint8_t psiv;
      uint8_t hubAddr;
      uint8_t hubPort;
      uint32_t rsvd[4];
    } init_device;
    struct{
      hcd_DeviceInfo_t* device;
      uint32_t rsvd[4];
    } init_class;
    uint32_t DWORD[5];
  } cont;
}hcd_Msg_t;

typedef struct{
  hcd_Msg_t msg[HCD_MSGBOX_SIZE];
  uint8_t enqPtr;
  uint8_t deqPtr;
}hcd_MsgBox_t;

void InitEHCI(void);
void MakeSETUPPacket(uint8_t dir, uint8_t typ, uint8_t attr, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, uint32_t* setup);
int32_t SendMessageToHostControllerDriver(hcd_Msg_t* msg);

#endif /*__HCD_H__*/
