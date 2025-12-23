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

#include "usb20.h"
#include "usbphy.h"

#define UDEV                          USBHS1__USBC

#define USBD_MAX_EP_NUM 				8
#define USBD_MAX_EP_DCI 				32
#define USBD_MAX_dTD_RESOURCE			16
#define USBD_EP0_OUT_DCI                0
#define USBD_EP0_IN_DCI                 1
#define USBD_EP0_OUT    		        0
#define USBD_EP0_IN		                0x80

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

typedef enum{
	USBD_OK = 0,
	USBD_INVALID_PARAM = -1,
	USBD_DISABLED_EP = -2,
	USBD_UNSUPPORTED_REQ = -3,
	USBD_BUFFER_OVER = -4,
	USBD_USED = -5,
    USBD_BUSY = -6,
}usbDcd_Status_t;

typedef struct{
	const uint8_t* descriptor;
	uint16_t size;
}usbDcd_Descriptor_Info_t;

typedef struct{
    uint8_t doesExist;
	uint8_t halt;
	uint16_t lastTxSize;
	uint32_t epCtrl_RegVal;
    void* bufPtr;
    void (*handlerCallback)(uint16_t size);
}usbDcd_Endpoint_Info_t;

typedef enum{
	EP0_OUT_TRANSFER = 0,
	EP0_IN_TRANSFER = 0x80
} usbDcd_Control_Dir_t;

typedef struct{
	usbDcd_Status_t (*setupHandler)(usb_SetupPacket_t* setup);
	usbDcd_Status_t (*dataStatHandler)(usbDcd_Control_Dir_t dir);
} usbDcd_Request_Set_t;

typedef union{
	struct{
		uint16_t selfPower:1;
		uint16_t remoteWakeUp:1;
		uint16_t rsvd:14;
	}BIT;
	uint16_t WORD;
} usbDcd_DeviceStatus_t;

typedef struct{
    usb_BusState_t busState;
	usb_SetupPacket_t lastSetup;
	uint8_t strMaxIndex;
	uint8_t curConfigVal;
	usbDcd_DeviceStatus_t status;
    usbDcd_Endpoint_Info_t rxEp[USBD_MAX_EP_NUM];
    usbDcd_Endpoint_Info_t txEp[USBD_MAX_EP_NUM];
	usbDcd_Descriptor_Info_t deviceDesc;
	usbDcd_Descriptor_Info_t configDesc;
	usbDcd_Descriptor_Info_t* strDescArray;
	usbDcd_Descriptor_Info_t deviceQualiferDesc;
	usbDcd_Descriptor_Info_t otherSpdConfigDesc;
	usbDcd_Request_Set_t classSpec;
	usbDcd_Request_Set_t vendorSpec;
	void (*notifyConfigured)(void);
}usbDcd_Device_info_t;

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
usbDcd_Status_t Usbd_OpenEndpoint(uint8_t epNum, int txType, uint16_t mps, uint8_t mult, void* bufPtr, void func(uint16_t));
usbDcd_Status_t Usbd_StartNextTransfer(uint8_t epNum, bool ioc, uint16_t txSize);
void Usbd_SetEpStall(uint8_t epNum);
usbDcd_Status_t Usbd_ReadEp0Buffer(void* buf, uint16_t size);
usbDcd_Status_t Usbd_WriteEp0Buffer(void* buf, uint16_t size);
void Usbd_SetClassRequestHandler (usbDcd_Status_t setupfunc(usb_SetupPacket_t*), usbDcd_Status_t dataFunc(usbDcd_Control_Dir_t));
void Usbd_SetConfiguredFunc(void func(void));
usbDcd_Status_t Usbd_Idle(uint8_t epNum);

#endif /*__USBD_SYS_H__*/
