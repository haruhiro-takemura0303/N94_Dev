/**
 * @brief   MCXN947V USB Host Controller Driver
 * @author  masa
 * @version 1.00 
 */

#ifndef __HCD_H__
#define __HCD_H__

#include "ehci.h"
#include "hcd_async.h"

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
	HCD_CTRL = 0,
  HCD_PARSE_CONFIG,
  HCD_GPTIMER,
  HCD_INIT_DEVICE,
};

typedef union{
  struct{
    hcd_DeviceInfo_t* device;
    usb_SetupPacket_t setup;
    uint32_t* sendDataBuf;
    void (*completeCb)(uint8_t devAddr, uint32_t* ep0Buf);
  } ctrl;
  struct{
    hcd_DeviceInfo_t* device;
    uint32_t rsvd[4];
  } parse_config;
  struct{
    uint32_t count_us;
    uint
  }

}hcd_Msg_t;

#endif /*__HCD_H__*/
