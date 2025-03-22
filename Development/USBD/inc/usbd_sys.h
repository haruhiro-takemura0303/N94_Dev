/**
 *@brief		NXP MCXN947V USB Device Controller Driver Header
 *@author		masa
 *@version	1.00
*/

#ifndef __USBD_SYS_H__
#define __USBD_SYS_H__

#include "cmsis_armclang.h"
#include "fsl_device_registers.h"
#include "clock_config.h"
#include "fsl_spc.h"

#define UDEV                          USBHS1__USBC

#define USBD_MAX_EP_NUM 				8
#define USBD_MAX_EP_DCI 				32
#define USBD_MAX_dTD_RESOURCE			16
#define USBD_EP0_OUT_DCI                0
#define USBD_EP0_IN_DCI                 1
#define USBD_EP0_OUT    		        0
#define USBD_EP0_IN		                0x80

typedef enum{
    DISCONNECTED = 0,
    ATTACHED,
    DEFAULT,
    ADDRESSED,
    CONFIGURED
} usb_BusState_t;

typedef union{
	struct{
		struct{
			uint8_t attr:5;
			uint8_t type:2;
			uint8_t dir:1;
		} bmRequestType;
		uint8_t bRequest;
		uint16_t wValue;
		uint16_t wIndex;
		uint16_t wLength;
	} BIT;
	uint32_t DWORD[2];
} usb_SetupPacket_t;

enum {
	BMREQ_DIR_OUT = 0,
	BMREQ_DIR_IN
};

enum {
	BMREQ_TYPE_STANDARD = 0,
	BMREQ_TYPE_CLASS,
	BMREQ_TYPE_VENDOR,
};

enum {
	BMREQ_ATTR_DEVICE = 0,
	BMREQ_ATTR_INTERFACE,
	BMREQ_ATTR_ENDPOINT,
	BMREQ_ATTR_OTHER,
	BMREQ_ATTR_VENDOR = 31
};

enum{
	BREQ_GET_STATUS = 0,
	BREQ_CLEAR_FEATURE,
	BREQ_SET_FEATURE = 3,
	BREQ_SET_ADDRESS = 5,
	BREQ_GET_DESCRIPTOR,
	BREQ_SET_DESCRIPTOR,
	BREQ_GET_CONFIGURATION,
	BREQ_SET_CONFIGURATION,
	BREQ_GET_INTERFACE,
	BREQ_SET_INTERFACE,
	BREQ_SYNCH_FRAME,
	BREQ_SET_ENCRYPTION,
	BREQ_GET_ENCRYPTION,
	BREQ_SET_HANDSHAKE,
	BREQ_GET_HANDSHAKE,
	BREQ_SET_CONNECTION,
	BREQ_SET_SECURITY_DATA,
	BREQ_GET_SEQURITY_DATA,
	BREQ_SET_WUSB_DATA,
	BREQ_LOOPBACK_DATA_WRITE,
	BREQ_LOOPBACK_DATA_READ,
	BREQ_SET_INTERFACE_DS,
	BREQ_SET_SEL = 48,
	BREQ_SET_ISOCH_DELAY
};

enum{
	DESCTYPE_DEVICE = 1,
	DESCTYPE_CONFIG = 2,
    DESCTYPE_STRING = 3,
	DESCTYPE_INTERFACE = 4,
	DESCTYPE_ENDPOINT = 5,
	DESCTYPE_INTERFACEASSOC = 11,
    DESCTYPE_CSIF = 0x24,
};

#define DEFAULT_EP		0

typedef struct{
    union{
        struct {
            uint32_t rsvd0:15;
            uint32_t ios:1;
            uint32_t maximumPacketLength:11;
            uint32_t rsvd1:2;
            uint32_t zlt:1;
            uint32_t mult:2;
        }BIT;
        uint32_t DWORD;
    }endpointCapability;
    uint32_t currentdTDPointer;
    uint32_t nextdTDPointer;
    uint32_t overray[6];
    uint32_t reserved;
    uint32_t setupBuffer[2];
    uint32_t pad[4];
}dQH_t;

typedef struct{
    uint32_t nextLinkPointer;
    union{
        struct{
            uint32_t status:8;
            uint32_t rsvd0:2;
            uint32_t multO:2;
            uint32_t rsvd1:3;
            uint32_t ioc:1;
            uint32_t totalBytes:15;
            uint32_t rsvd2:1;
        }BIT;
        uint32_t DWORD;
    }token;
    uint32_t bufferPointer[5];
    uint32_t pad;
}dTD_t;

typedef struct{
	const uint8_t* descriptor;
	uint16_t size;
}usbDcd_Descriptor_Info_t;

typedef struct{
    uint8_t valid;
	uint8_t halt;
	uint16_t lastTxSize;
    void* bufPtr;
    void (*handlerCallback)(uint16_t size);
}usbDcd_Endpoint_Info_t;

typedef struct{
    usb_BusState_t busState;
	usb_SetupPacket_t lastSetup;
    usbDcd_Endpoint_Info_t rxEp[USBD_MAX_EP_NUM];
    usbDcd_Endpoint_Info_t txEp[USBD_MAX_EP_NUM];
	usbDcd_Descriptor_Info_t deviceDesc;
	usbDcd_Descriptor_Info_t configDesc;
	usbDcd_Descriptor_Info_t* strDescArray;
	uint8_t strMaxIndex; 
}usbDcd_Device_info_t;

typedef enum{
	USBD_OK = 0,
	USBD_INVALID_PARAM = -1,
	USBD_DISABLED_EP = -2,
}usbDcd_Status_t;

#define USBD_dTD_Token_Active	0x80
#define USBD_dTD_Token_Mask		0xFF
#define USBD_dQH_dTD_T			0x00000001UL

#define USB_IOC_ENABLE	true
#define	USB_IOC_DISABLE	false

/************** Descriptor Macros **************/
#define WORDLB(x)    (((uint32_t)x) & 0xFF)
#define WORDHB(x)    ((((uint32_t)x) & 0xFF00) >> 8)
#define DWORD0B(x)  (((uint32_t)x) & 0xFF)
#define DWORD1B(x)  ((((uint32_t)x) & 0xFF00) >> 8)
#define DWORD2B(x)  ((((uint32_t)x) & 0xFF0000) >> 16)
#define DWORD3B(x)  ((((uint32_t)x) & 0xFF000000) >> 24)

void Usbd_SysInit(void);
void Usbd_SysStart(void);
void Usbd_SetDescriptor(int descType, const uint8_t* descPtr, uint16_t descSize);
void Usbd_SetStringDescriptor(usbDcd_Descriptor_Info_t* descArray, uint8_t maxIndex);
usbDcd_Status_t Usbd_StartNextTransfer(uint8_t epNum, bool ioc, uint16_t txSize);
void Usbd_SetEpStall(uint8_t epNum);

#endif /*__USBD_SYS_H__*/
