/**
* @brief   MCXN947V USB Host Controller Dummy HID Class Driver 
* @author  masa
* @version 1.00 
*/

#include "hcd_class_dummy_hid.h"

static uint16_t parseInterface(config_rawdesc_t *confRaw, hcd_DeviceInfo_t* device)
{
  uint8_t nextDescType;
  usbDesc_Interface_t* intfPtr;
  uint16_t rdIdx, descInc;
	
	descInc = 0;
	rdIdx = confRaw->readPtr;
  intfPtr = (usbDesc_Interface_t*)&confRaw->rawDesc[rdIdx];
	
	rdIdx += intfPtr->bLength;
  descInc += intfPtr->bLength;
  nextDescType = confRaw->rawDesc[rdIdx + 1];
  while ((nextDescType != DESCTYPE_INTERFACE) && (rdIdx < confRaw->fullLength)){
    
    uint8_t bLen = confRaw->rawDesc[rdIdx];
    rdIdx += bLen;
    descInc += bLen;
    nextDescType = confRaw->rawDesc[rdIdx + 1];
  }
  return descInc;
}

static uint16_t parseIAD(config_rawdesc_t *confRaw, hcd_DeviceInfo_t* device)
{ 
  return sizeof(usbDesc_InterfaceAssoc_t);
}

static void initClass(hcd_DeviceInfo_t* device)
{
  
}

static void terminateClass(hcd_DeviceInfo_t* device)
{
  
}

void HcdDummyHid_InitDriver(void)
{
  hcd_ClassDriver_t drv;
  drv.parseInterface = parseInterface;
  drv.parseIAD = parseIAD;
  drv.initClass = initClass;
  drv.terinateClass = terminateClass;
  
  RegisterClassDriver(&drv, USB_CLASSCODE_HID);  
}