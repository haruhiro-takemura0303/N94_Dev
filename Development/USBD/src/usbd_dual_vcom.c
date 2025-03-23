/**
 * @brief   MCXN947 USB Device Dual Virtual Communication Device Class Driver
 * @author  masa
 * @version 1.00
 */

#include "usbd_dual_vcom.h"

usbd_DualVcom_Info_t stDualVcom;

#ifndef VSCODE
__STATIC_FORCEINLINE usbd_DualVcom_Info_t* vcom()
#else
usbd_DualVcom_Info_t* vcom()
#endif
{
    return &stDualVcom;
}

static usbDcd_Status_t setupHandler(usb_SetupPacket_t* setup)
{
    usbDcd_Status_t ret = USBD_OK;
    if (setup){
        memcpy(setup, &vcom()->lastSetup, sizeof(usb_SetupPacket_t));
    }
    switch(vcom()->lastSetup.BIT.bRequest){
        case(BREQ_CDC_SET_LINE_CODING):{
            Usbd_StartNextTransfer(USBD_EP0_OUT, USB_IOC_ENABLE, vcom()->lastSetup.BIT.wLength);
            break;
        }
        case(BREQ_CDC_GET_LINE_CODING):{
            if (setup->BIT.wIndex == USB_VCOM0_IF_NUM){
                Usbd_WriteEp0Buffer(&vcom()->lineCoding[0], USBD_CDC_SIZEOF_LINECODING_STRUCT);
            } else if (setup->BIT.wIndex == USB_VCOM1_IF_NUM){
                Usbd_WriteEp0Buffer(&vcom()->lineCoding[1], USBD_CDC_SIZEOF_LINECODING_STRUCT);
            }
            Usbd_StartNextTransfer(USBD_EP0_IN, USB_IOC_ENABLE, vcom()->lastSetup.BIT.wLength);
            break;
        }
        case(BREQ_CDC_SET_CONTROL_LINE_STATE):{
            Usbd_StartNextTransfer(USBD_EP0_OUT, USB_IOC_ENABLE, vcom()->lastSetup.BIT.wLength);
            break;
        }
        default:{
            ret = USBD_UNSUPPORTED_REQ;
            break;
        }
    }
    return ret;
}

static usbDcd_Status_t dataStatHandler(usbDcd_Control_Dir_t dir)
{
    usbDcd_Status_t ret = USBD_OK;
    static usb_CDC_LineCoding_t newLineCoding;
    if (dir == EP0_OUT_TRANSFER){
        switch(vcom()->lastSetup.BIT.bRequest){
            case(BREQ_CDC_SET_LINE_CODING):{
                Usbd_ReadEp0Buffer(&newLineCoding, USBD_CDC_SIZEOF_LINECODING_STRUCT);
                break;
            }
            case(BREQ_CDC_GET_LINE_CODING):{
                break;
            }
            case(BREQ_CDC_SET_CONTROL_LINE_STATE):{
                break;
            }
            default:{
                ret = USBD_UNSUPPORTED_REQ;
                break;
            }
        }
    } else if (dir == EP0_IN_TRANSFER){
        switch(vcom()->lastSetup.BIT.bRequest){
            case(BREQ_CDC_SET_LINE_CODING):{
                if (vcom()->lastSetup.BIT.wIndex == USB_VCOM0_IF_NUM){
                    memcpy(&vcom()->lineCoding[0], &newLineCoding, sizeof(usb_CDC_LineCoding_t));
                }else if (vcom()->lastSetup.BIT.wIndex == USB_VCOM1_IF_NUM){
                    memcpy(&vcom()->lineCoding[1], &newLineCoding, sizeof(usb_CDC_LineCoding_t));
                }
                break;
            }
            case(BREQ_CDC_GET_LINE_CODING):{
                break;
            }
            case(BREQ_CDC_SET_CONTROL_LINE_STATE):{
                break;
            }
            default:{
                ret = USBD_UNSUPPORTED_REQ;
                break;
            }
        }
    }
    return ret;
}

void InitDualVcom(void)
{
    Usbd_SysInit();
    for (int i = 0; i < USBD_DUALVCOM_NUMOF_COM_IF; i++){
        vcom()->lineCoding[i].dwDTERate = 115200;
        vcom()->lineCoding[i].bCharFormat = 0;
        vcom()->lineCoding[i].bParityType = 0;
        vcom()->lineCoding[i].bDataBits = 8;
    }
    UsbdDualVcom_InitDescriptor();
    Usbd_SetClassRequestHandler(setupHandler, dataStatHandler);

    Usbd_SysStart();
}
