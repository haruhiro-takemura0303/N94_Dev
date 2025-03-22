/**
 * @brief   USB Device Descriptor Set for Dual VCOM Composite Device
 * @author  masa
 * @version 1.00
 */

#include "usbdDualVcomDescriptor.h"
#include <string.h>

#ifndef VSCODE
__ALIGNED(4)
#endif
/*static*/ const uint8_t stDeviceDescriptor[] = {
    USB_DEVICEDESC_SIZE,        /*bLength*/
    DESCTYPE_DEVICE,            /*bDescriptorType*/
    WORDLB(USB_HS_VER),         /*bcdUSB*/
    WORDHB(USB_HS_VER),         
    USB_CLASS_MISC,             /*bDeviceClass*/
    USB_SUBCLASS_MISC_COMMON,   /*bDeviceSubClass*/
    USB_PROTOCOL_MISC_IAD,      /*bDeviceProtocol*/
    USB_HS_EP0_MPS,             /*bMaxPacketSize*/
    WORDLB(USB_ID_VENDOR),      /*idVender*/
    WORDHB(USB_ID_VENDOR),      
    WORDLB(USB_ID_PRODUCT),     /*idProduct*/
    WORDHB(USB_ID_PRODUCT),     
    WORDLB(USB_DEVICE_VER),     /*bcdDevice*/
    WORDHB(USB_DEVICE_VER),     
    STRDESC_IDX_MANUFACTURER,   /*iManufacturer*/
    STRDESC_IDX_PRODUCT,        /*iProduct*/
    STRDESC_IDX_SERIAL,         /*iSerialNumber*/
    USB_NUMOF_CONFIG,           /*bNumConfigurations*/    
};

#ifndef VSCODE
__ALIGNED(4)
#endif
/*static*/ const uint8_t stConfigurationDescriptor[] = {
    /*Configuration Descriptor*/
    USB_CONFIGDESC_SIZE,                /*bLength*/
    DESCTYPE_CONFIG,                    /*bDescriptorType*/
    WORDLB(USB_CONFIGDESC_FULLSIZE),    /*wTotalLength*/
    WORDHB(USB_CONFIGDESC_FULLSIZE),    
    USB_NUMOF_IF,                       /*bNumInterfaces*/
    USB_CONFIG_VALUE,                   /*bConfigurationValue*/
    USB_UNDEF,                          /*iConfiguration*/
    USB_CONFIG_ATTR,                    /*bmAttributes*/
    USB_MAXPOWER,                       /*bMaxPower*/

    /*Virtual COM Port #0*/
    /*Interface Association Descriptor #0*/
    USB_IFASSOCDESC_SIZE,               /*bLength*/
		DESCTYPE_INTERFACEASSOC,            /*bDescriptorType*/
		USB_IAD0_FIRSTIF,                   /*bFirstInterface*/
		USB_IAD_IFCOUNT,                    /*bInterfaceCount*/
		USB_CLASS_COM,                      /*bFunctionClass*/
		USB_SUBCLASS_COM_ACM,               /*bFunctionSubclass*/
		USB_UNDEF,                          /*bFunctionProtocol*/
		USB_UNDEF,                          /*iFunction*/
    
    /*Interface Descriptor #0 : CDC Communication Interface*/
    USB_IFDESC_SIZE,                    /*bLength*/
		DESCTYPE_INTERFACE,                 /*bDescriptorType*/
		USB_VCOM0_IF_NUM,                   /*bInterfaceNumber*/
		USB_UNDEF,                          /*bAlternateSetting*/
		USB_VCOM_NUMOF_EP,                  /*bNumEndpoints*/
		USB_CLASS_COM,                      /*bInterfaceClass*/
		USB_SUBCLASS_COM_ACM,               /*bInterfaceSubclass*/
		USB_PROTOCOL_COM_AT,                /*bInterfaceProtocol*/
		USB_UNDEF,                          /*iInterface*/

    /*Functional Descriptor[0]*/
    /*Header Functional Descriptor*/
    USB_CDC_IF_HDRFUNCDESC_SIZE,        /*bLength*/
    DESCTYPE_CSIF,                      /*bDescriptorType*/
    DESCSUBTYPE_CDC_CSIF_HDR,           /*bDescriptorSubtype*/
    WORDLB(USB_CDC_VER),                /*bcdCDC*/
    WORDHB(USB_CDC_VER),                

    /*Functional Descriptor[1]*/
    /*Call Management Functional Descriptor*/
    USB_CDC_IF_CALLMNGFUNCDESC_SIZE,    /*bLength*/
    DESCTYPE_CSIF,                      /*bDescriptorType*/
    DESCSUBTYPE_CDC_CSIF_CALL,          /*bDescriptorSubtype*/
    USB_UNDEF,                          /*bmCapabilities*/
    USB_CDC0_IF_NUM,                    /*bDataInterface*/

    /*Functional Descriptor[2]*/
    /*ACM Functional Descriptor*/
    USB_CDC_IF_ACMDESC_SIZE,            /*bLength*/
    DESCTYPE_CSIF,                      /*bDescriptorType*/
    DESCSUBTYPE_CDC_CSIF_ACM,           /*bDescriptorSubtype*/
    USB_CDC_ACM_SUPPORT,                /*bmCapabilities*/

    /*Functional Descriptor[3]*/
    /*Union Functional Descriptor*/
    USB_CDC_IF_UNIONDESC_SIZE,          /*bLength*/
    DESCTYPE_CSIF,                      /*bDescriptorType*/
    DESCSUBTYPE_CDC_CSIF_UNION,         /*bDescriptorSubtype*/
    USB_VCOM0_IF_NUM,                   /*bMasterInterface*/
    USB_CDC0_IF_NUM,                    /*bSlaveInterface0*/

    /*Endpoint Descriptor #0 Interrupt IN*/
    USB20_ENDPTDESC_SIZE,               /*bLength*/
		DESCTYPE_ENDPOINT,                  /*bDescriptorType*/
		USB_VCOM0_CMDEP_ADDR,               /*bEndpointAddress*/
		USB_VCOM_CMDEP_ATTR,                /*bmAttributes*/
		WORDLB(USB_VCOM_CMDEP_MPS),         /*wMaxPacketSize*/
    WORDHB(USB_VCOM_CMDEP_MPS),         
		USB_VCOM_CMDEP_INTVAL,              /*bInterval*/

    /*Interface Descriptor #1 : CDC Data Interface*/
    USB_IFDESC_SIZE,                    /*bLength*/
		DESCTYPE_INTERFACE,                 /*bDescriptorType*/
		USB_CDC0_IF_NUM,                    /*bInterfaceNumber*/
		USB_UNDEF,                          /*bAlternateSetting*/
		USB_CDC_NUMOF_EP,                   /*bNumEndpoints*/
		USB_CLASS_CDC,                      /*bInterfaceClass*/
		USB_UNDEF,                          /*bInterfaceSubclass*/
		USB_UNDEF,                          /*bInterfaceProtocol*/
		USB_UNDEF,                          /*iInterface*/

    /*Endpoint Descriptor #1 Bulk OUT*/
    USB20_ENDPTDESC_SIZE,               /*bLength*/
		DESCTYPE_ENDPOINT,                  /*bDescriptorType*/
		USB_CDC0_DATAOUTEP_ADDR,            /*bEndpointAddress*/
		USB_CDC_DATAEP_ATTR,                /*bmAttributes*/
		WORDLB(USB_CDC_DATAEP_MPS),         /*wMaxPacketSize*/
			WORDHB(USB_CDC_DATAEP_MPS),         
		USB_UNDEF,                          /*bInterval*/

    /*Endpoint Descriptor #2 Bulk IN*/
    USB20_ENDPTDESC_SIZE,               /*bLength*/
		DESCTYPE_ENDPOINT,                  /*bDescriptorType*/
		USB_CDC0_DATAINEP_ADDR,             /*bEndpointAddress*/
		USB_CDC_DATAEP_ATTR,                /*bmAttributes*/
		WORDLB(USB_CDC_DATAEP_MPS),         /*wMaxPacketSize*/
    WORDHB(USB_CDC_DATAEP_MPS),         
		USB_UNDEF,                          /*bInterval*/

    /*Virtual COM Port #1*/
    /*Interface Association Descriptor #1*/
    USB_IFASSOCDESC_SIZE,               /*bLength*/
		DESCTYPE_INTERFACEASSOC,            /*bDescriptorType*/
		USB_IAD1_FIRSTIF,                   /*bFirstInterface*/
		USB_IAD_IFCOUNT,                    /*bInterfaceCount*/
		USB_CLASS_COM,                      /*bFunctionClass*/
		USB_SUBCLASS_COM_ACM,               /*bFunctionSubclass*/
		USB_UNDEF,                          /*bFunctionProtocol*/
		USB_UNDEF,                          /*iFunction*/
    
    /*Interface Descriptor #2 : CDC Communication Interface*/
    USB_IFDESC_SIZE,                    /*bLength*/
		DESCTYPE_INTERFACE,                 /*bDescriptorType*/
		USB_VCOM1_IF_NUM,                   /*bInterfaceNumber*/
		USB_UNDEF,                          /*bAlternateSetting*/
		USB_VCOM_NUMOF_EP,                  /*bNumEndpoints*/
		USB_CLASS_COM,                      /*bInterfaceClass*/
		USB_SUBCLASS_COM_ACM,               /*bInterfaceSubclass*/
		USB_PROTOCOL_COM_AT,                /*bInterfaceProtocol*/
		USB_UNDEF,                          /*iInterface*/

    /*Functional Descriptor[0]*/
    /*Header Functional Descriptor*/
    USB_CDC_IF_HDRFUNCDESC_SIZE,        /*bLength*/
    DESCTYPE_CSIF,                      /*bDescriptorType*/
    DESCSUBTYPE_CDC_CSIF_HDR,           /*bDescriptorSubtype*/
    WORDLB(USB_CDC_VER),                /*bcdCDC*/
    WORDHB(USB_CDC_VER),                

    /*Functional Descriptor[1]*/
    /*Call Management Functional Descriptor*/
    USB_CDC_IF_CALLMNGFUNCDESC_SIZE,    /*bLength*/
    DESCTYPE_CSIF,                      /*bDescriptorType*/
    DESCSUBTYPE_CDC_CSIF_CALL,          /*bDescriptorSubtype*/
    USB_UNDEF,                          /*bmCapabilities*/
    USB_CDC1_IF_NUM,                    /*bDataInterface*/

    /*Functional Descriptor[2]*/
    /*ACM Functional Descriptor*/
    USB_CDC_IF_ACMDESC_SIZE,            /*bLength*/
    DESCTYPE_CSIF,                      /*bDescriptorType*/
    DESCSUBTYPE_CDC_CSIF_ACM,           /*bDescriptorSubtype*/
    USB_CDC_ACM_SUPPORT,                /*bmCapabilities*/

    /*Functional Descriptor[3]*/
    /*Union Functional Descriptor*/
    USB_CDC_IF_UNIONDESC_SIZE,          /*bLength*/
    DESCTYPE_CSIF,                      /*bDescriptorType*/
    DESCSUBTYPE_CDC_CSIF_UNION,         /*bDescriptorSubtype*/
    USB_VCOM1_IF_NUM,                   /*bMasterInterface*/
    USB_CDC1_IF_NUM,                    /*bSlaveInterface0*/

    /*Endpoint Descriptor #3 Interrupt IN*/
    USB20_ENDPTDESC_SIZE,               /*bLength*/
		DESCTYPE_ENDPOINT,                  /*bDescriptorType*/
		USB_VCOM1_CMDEP_ADDR,               /*bEndpointAddress*/
		USB_VCOM_CMDEP_ATTR,                /*bmAttributes*/
		WORDLB(USB_VCOM_CMDEP_MPS),         /*wMaxPacketSize*/
    WORDHB(USB_VCOM_CMDEP_MPS),         
		USB_VCOM_CMDEP_INTVAL,              /*bInterval*/

    /*Interface Descriptor #3 : CDC Data Interface*/
    USB_IFDESC_SIZE,                    /*bLength*/
		DESCTYPE_INTERFACE,                 /*bDescriptorType*/
		USB_CDC1_IF_NUM,                    /*bInterfaceNumber*/
		USB_UNDEF,                          /*bAlternateSetting*/
		USB_CDC_NUMOF_EP,                   /*bNumEndpoints*/
		USB_CLASS_CDC,                      /*bInterfaceClass*/
		USB_UNDEF,                          /*bInterfaceSubclass*/
		USB_UNDEF,                          /*bInterfaceProtocol*/
		USB_UNDEF,                          /*iInterface*/

    /*Endpoint Descriptor #4 Bulk OUT*/
    USB20_ENDPTDESC_SIZE,               /*bLength*/
		DESCTYPE_ENDPOINT,                  /*bDescriptorType*/
		USB_CDC1_DATAOUTEP_ADDR,            /*bEndpointAddress*/
		USB_CDC_DATAEP_ATTR,                /*bmAttributes*/
		WORDLB(USB_CDC_DATAEP_MPS),         /*wMaxPacketSize*/
		WORDHB(USB_CDC_DATAEP_MPS),         
		USB_UNDEF,                          /*bInterval*/

    /*Endpoint Descriptor #5 Bulk IN*/
    USB20_ENDPTDESC_SIZE,               /*bLength*/
		DESCTYPE_ENDPOINT,                  /*bDescriptorType*/
		USB_CDC1_DATAINEP_ADDR,             /*bEndpointAddress*/
		USB_CDC_DATAEP_ATTR,                /*bmAttributes*/
		WORDLB(USB_CDC_DATAEP_MPS),         /*wMaxPacketSize*/
    WORDHB(USB_CDC_DATAEP_MPS),         
		USB_UNDEF,                          /*bInterval*/
};

#ifndef VSCODE
__ALIGNED(4)
#endif
/*static*/ uint8_t stStringLangDescriptor[] = {
    USB_LANGDESC_SIZE,                  /*bLength*/
    DESCTYPE_STRING,                    /*bDescriptorType*/
    WORDLB(USB_LANGID_ENG),             /*wLangID[0]*/
    WORDHB(USB_LANGID_ENG),
};

#ifndef VSCODE
__ALIGNED(4)
#endif
/*static*/ uint8_t stStringManufacturerDescriptor[USB_STRING_MAX_SIZE] = {0};

#ifndef VSCODE
__ALIGNED(4)
#endif
/*static*/ uint8_t stStringProductDescriptor[USB_STRING_MAX_SIZE] = {0};

#ifndef VSCODE
__ALIGNED(4)
#endif
/*static*/ uint8_t stStringSerialDescriptor[USB_STRING_MAX_SIZE] = {0};

/*static*/ usbDcd_Descriptor_Info_t stStringArray[4] = {0};

/*static*/ const char stManufacturer[] = "NXP Semiconductor";
/*static*/ const char stProduct[] = "FRDMMCXN947 Experiment";
/*static*/ const char stSerial[] = "21248931";


static int32_t setupStringDesc(uint8_t* desc, char* string)
{
    uint16_t len = strlen(string);
    if ((2 * len + 2) > USB_STRING_MAX_SIZE){
        return -1;
    }
    desc[0] = 2 * len + 2;
    desc[1] = DESCTYPE_STRING;
    for (int i = 0; i < len; i++){
        desc[2 * i + 2] = (uint8_t)string[i];
        desc[2 * i + 3] = 0;
    }
    return 0;
}

