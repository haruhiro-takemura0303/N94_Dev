/**
 * @brief   USB Device Descriptor Set for Dual VCOM Composite Device
 * @author  masa
 * @version 1.00
 */

#ifndef __USBDDUALVCOMDESCRIPTOR_H__
#define __USBDDUALVCOMDESCRIPTOR_H__

#include "usbd_sys.h"

/************** Descriptor Macros **************/

enum{
    STRDESC_IDX_MANUFACTURER = 1,
    STRDESC_IDX_PRODUCT,
    STRDESC_IDX_SERIAL,
};
#define USB_UNDEF                   0

/* Descriptor Sizes*/
#define USB_DEVICEDESC_SIZE         18
#define USB_CONFIGDESC_SIZE         9
#define USB_LANGDESC_SIZE           4
#define USB_IFDESC_SIZE             9
#define USB20_ENDPTDESC_SIZE        7
#define USB_IFASSOCDESC_SIZE        8

/* Communication Device Class(Abstruct Communication Model) */
#define USB_CLASS_COM               0x02
#define USB_CLASS_CDC               0x0A
#define USB_SUBCLASS_COM_ACM        0x02
#define USB_PROTOCOL_COM_AT         0x01
#define USB_CDC_VER                 0x0120
enum{
    DESCSUBTYPE_CDC_CSIF_HDR = 0,
    DESCSUBTYPE_CDC_CSIF_CALL,
    DESCSUBTYPE_CDC_CSIF_ACM,
    DESCSUBTYPE_CDC_CSIF_UNION = 6,
};
#define USB_CDC_IF_HDRFUNCDESC_SIZE     5
#define USB_CDC_IF_CALLMNGFUNCDESC_SIZE 5
#define USB_CDC_IF_ACMDESC_SIZE         4
#define USB_CDC_IF_UNIONDESC_SIZE       5
#define USB_CDC_CSIF_SIZE               (USB_CDC_IF_HDRFUNCDESC_SIZE + USB_CDC_IF_CALLMNGFUNCDESC_SIZE + \
                                        USB_CDC_IF_ACMDESC_SIZE + USB_CDC_IF_UNIONDESC_SIZE)
#define USB_VCOM_NUMOF_IF                 1
#define USB_CDC_NUMOF_IF                1
#define USB_CDC_ACM_SUPPORT             0x02             

/* Device */
#define USB_HS_VER                  0x0200
#define USB_CLASS_MISC              0xEF
#define USB_SUBCLASS_MISC_COMMON    0x02
#define USB_PROTOCOL_MISC_IAD       0x01
#define USB_HS_EP0_MPS              0x40
#define USB_ID_VENDOR               0x1FC9
#define USB_ID_PRODUCT              0xFF3F
#define USB_DEVICE_VER              0x0100
#define USB_NUMOF_CONFIG            1

/* Configuration */
#define USB_CONFIGDESC_FULLSIZE     (USB_CONFIGDESC_SIZE + \
                                    2 * (USB_IFASSOCDESC_SIZE + USB_IFDESC_SIZE + USB_CDC_CSIF_SIZE + USB20_ENDPTDESC_SIZE + \
                                        USB_IFDESC_SIZE + 2 * USB20_ENDPTDESC_SIZE))
#define USB_NUMOF_IF                (2 * (USB_VCOM_NUMOF_IF + USB_CDC_NUMOF_IF))
#define USB_CONFIG_VALUE            1
#define USB_CONFIG_ATTR             0xC0
#define USB_MAXPOWER                0x32

/* Interface */
#define USB_VCOM0_IF_NUM            0
#define USB_CDC0_IF_NUM             1
#define USB_VCOM1_IF_NUM            2
#define USB_CDC1_IF_NUM             3
#define USB_VCOM_NUMOF_EP           1
#define USB_CDC_NUMOF_EP            2

/* Interface Association */
#define USB_IAD0_FIRSTIF             USB_VCOM0_IF_NUM
#define USB_IAD1_FIRSTIF             USB_VCOM1_IF_NUM
#define USB_IAD_IFCOUNT             (USB_VCOM_NUMOF_IF + USB_CDC_NUMOF_IF)

/* Endpoint */
#define USB_IN_EP                   0x80
#define USB_OUT_EP                  0x00
#define USB_VCOM0_CMDEP_ADDR        (USB_IN_EP | 0x01)
#define USB_VCOM1_CMDEP_ADDR        (USB_IN_EP | 0x03)
#define USB_VCOM_CMDEP_ATTR         0x03    // Interrupt
#define USB_VCOM_CMDEP_INTVAL       0x04
#define USB_VCOM_CMDEP_MPS          0x08
#define USB_CDC0_DATAOUTEP_ADDR     (USB_OUT_EP | 0x02)
#define USB_CDC0_DATAINEP_ADDR      (USB_IN_EP | 0x02)
#define USB_CDC1_DATAOUTEP_ADDR     (USB_OUT_EP | 0x04)
#define USB_CDC1_DATAINEP_ADDR      (USB_IN_EP | 0x04)
#define USB_CDC_DATAEP_ATTR         0x02    // Bulk
#define USB_CDC_DATAEP_MPS          0x200

/* String */
#define USB_LANGID_ENG              0x0409
#define USB_STRING_MAX_SIZE         (128 + 2)


int32_t UsbdDualVcom_InitDescriptor(void);

#endif /*__USBDDUALVCOMDESCRIPTOR_H__*/
