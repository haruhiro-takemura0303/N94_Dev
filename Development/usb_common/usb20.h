/**
 *@brief		USB 2.0 Common Definitions
 *@author		masa
 *@version	1.00
*/


#ifndef __USB20_H__
#define __USB20_H__

#include <stdint.h>

typedef enum{
  DEV_SPEED_UNDEF = 0,
  DEV_SPEED_FULL,
  DEV_SPEED_LOW,
  DEV_SPEED_HIGH
} usb_psiv_t;

typedef enum{
    DEV_DISCONNECTED = 0,
    DEV_ATTACHED,
    DEV_DEFAULT,
    DEV_ADDRESSED,
    DEV_CONFIGURED
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
	FEATURE_ENDPOINT_HALT = 0,
	FEATURE_DEV_REMOTE_WUP,
	FEATURE_TEST_MODE
};

enum{
	DESCTYPE_DEVICE = 1,
	DESCTYPE_CONFIG = 2,
	DESCTYPE_STRING = 3,
	DESCTYPE_INTERFACE = 4,
	DESCTYPE_ENDPOINT = 5,
	DESCTYPE_DEVICE_QUALIFIER = 6,
	DESCTYPE_OTHERSPD_CONFIG = 7,
	DESCTYPE_INTERFACEASSOC = 11,
  DESCTYPE_CSIF = 0x24,
	DESCTYPE_CSEP = 0x25,
};

enum{
	TYPE_CONTROL = 0,
	TYPE_ISOCHRONOUS,
	TYPE_BULK,
	TYPE_INTERRUPT
};

typedef enum {
	DETACHED = 0,
	ATTACHED,
	GOT_DESCRIPTOR_DEV,
	ADDRESSED,
	GOT_DESCRIPTOR_DEV_RE,
	GOT_DESCRIPTOR_CFG_INI,
	GOT_DESCRIPTOR_CFG,
	GOT_DESCRIPTOR_LANG,
	GOT_DESCRIPTOR_LANG_STR_VENDOR_SKIP,
	GOT_DESCRIPTOR_STR_VENDOR,
	GOT_DESCRIPTOR_STR_PROD,
	GET_DESCRIPTOR_STR_SKIPPED,
	CONFIGURED,
	NUM_OF_ENUM_STATE
}usb_EnumState_t;

typedef union{
	struct{
		uint8_t bLength;
		uint8_t bDescriptorType;
		uint16_t bcdUSB;
		uint8_t bDeviceClass;
		uint8_t bDeviceSubClass;
		uint8_t bDeviceProtocol;
		uint8_t bMaxPacketSize;
		uint16_t idVender;
		uint16_t idProduct;
		uint16_t bcdDevice;
		uint8_t iManufacturer;
		uint8_t iProduct;
		uint8_t iSerialNumber;
		uint8_t bNumConfigurations;
	}desc;
	uint8_t data[18];
} usbDesc_Device_t;

typedef union{
	struct {
		uint8_t bLength;
		uint8_t bDescriptorType;
		uint16_t wTotalLength;
		uint8_t bNumInterfaces;
		uint8_t bConfigurationValue;
		uint8_t iConfiguration;
		uint8_t bmAttributes;
		uint8_t MaxPower;
	}desc;
	uint8_t data[9];
} usbDesc_Config_t;

typedef struct{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bInterfaceNumber;
	uint8_t bAlternateSetting;
	uint8_t bNumEndpoints;
	uint8_t bInterfaceClass;
	uint8_t bInterfaceSubclass;
	uint8_t bInterfaceProtocol;
	uint8_t iInterface;
} usbDesc_Interface_t;

typedef struct{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bEndpointAddress;
	uint8_t bmAttributes;
	uint8_t wMaxPacketSize_lsB;
	uint8_t wMaxPacketSize_msB;
	uint8_t bInterval;
} usbDesc_Endpoint2_t;

typedef struct{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bEndpointAddress;
	uint8_t bmAttributes;
	uint8_t wMaxPacketSize_lsB;
	uint8_t wMaxPacketSize_msB;
	uint8_t wInterval_lsB;
	uint8_t wInterval_msB;
	uint8_t bSyncAddress;
} usbDesc_Endpoint_t;

typedef struct{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bFirstInterface;
	uint8_t bInterfaceCount;
	uint8_t bFunctionClass;
	uint8_t bFunctionSubclass;
	uint8_t bFunctionProtocol;
	uint8_t iFunction;
}usbDesc_InterfaceAssoc_t;

typedef struct{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bNbrPorts;
	uint8_t wHubCharacteristics_lsB;
	uint8_t wHubCharacteristics_msB;
	uint8_t bPwrOn2PwrGood;
	uint8_t bHubContrCurrent;
	uint8_t DeviceRemovable[32];
	uint8_t PortPwrCtrlMask[32];
} usbDesc_Hub_t;

#define DEFAULT_EP		0

#endif /*__USB20_H__*/
