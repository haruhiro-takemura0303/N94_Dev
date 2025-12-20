/**
* @brief   MCXN947V USB Host Controller Dummy HID Class Driver 
* @author  masa
* @version 1.00 
*/

#ifndef __HCD_CLASS_DUMMY_HID_H__
#define __HCD_CLASS_DUMMY_HID_H__

#include "hcd_class.h"

#define USB_CLASSCODE_HID 0x03

void HcdDummyHid_InitDriver(void);

#endif /*__HCD_CLASS_DUMMY_HID_H__*/
