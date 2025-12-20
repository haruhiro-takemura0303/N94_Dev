/**
* @brief   MCXN947V USB Host Controller Driver Class Manager
* @author  masa
* @version 1.00 
*/

#ifndef __HCD_CLASS_H__
#define __HCD_CLASS_H__

#include "hcd.h"

#define MAX_DEFINED_CLASS_CODE  0xA

typedef struct{
  uint16_t (*parseInterface)(config_rawdesc_t *confRaw, hcd_DeviceInfo_t* device);
  uint16_t (*parseIAD)(config_rawdesc_t *confRaw, hcd_DeviceInfo_t* device);
  void (*initClass)(hcd_DeviceInfo_t* device);
  void (*terinateClass)(hcd_DeviceInfo_t* device);
}hcd_ClassDriver_t;

typedef struct{
  hcd_DeviceInfo_t* device;
  uint8_t state;
  uint8_t clsCode[3];
}hcd_ClassMgr_t;

hcd_Status_t ParseConfigurationDescriptor(hcd_DeviceInfo_t* device, config_rawdesc_t* confRaw, usbDesc_Config_t* configDesc);
hcd_Status_t StartClassDriver(hcd_DeviceInfo_t* device);
hcd_Status_t RegisterClassDriver(hcd_ClassDriver_t* map, uint8_t clsCode);
void HcdClass_InitClassDrivers(void);

#endif /*__HCD_CLASS_H__*/
