/**
 * @brief   MCXN947 USB Device Dual Virtual Communication Device Class Driver
 * @author  masa
 * @version 1.00
 */

#include "usbd_dual_vcom.h"

void InitDualVcom(void)
{
    Usbd_SysInit();
    UsbdDualVcom_InitDescriptor();

    Usbd_SysStart();
}
