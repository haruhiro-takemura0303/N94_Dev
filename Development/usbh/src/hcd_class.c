/**
* @brief   MCXN947V USB Host Controller Driver Class Manager
* @author  masa
* @version 1.00 
*/

#include "hcd_class.h"

static hcd_ClassMgr_t st_ClassMgr[MAX_DEVICE_NUM];
static hcd_ClassDriver_t st_ClassDriver[MAX_DEFINED_CLASS_CODE + 1];

static int32_t clsNew(uint8_t clsIdx, uint8_t clsCode)
{
  int32_t ret = 0;
  for (int i = 0; i < 3; i++){
    if (st_ClassMgr[clsIdx].clsCode[i] == clsCode){
      ret = -1;
      break;
    }
  }
  return ret;
}

hcd_Status_t ParseConfigurationDescriptor(hcd_DeviceInfo_t* device, config_rawdesc_t* confRaw, usbDesc_Config_t* configDesc)
{
  uint8_t bLength;
  uint16_t instRdPtr = 0;
  uint16_t retReadPtr;
  uint8_t numIfs, curCls, clsCount;
  int clsIdx = -1;
  hcd_Status_t ret;
  usbDesc_Interface_t* intfDesc;
  usbDesc_InterfaceAssoc_t* iad;
  
  /*Class Manager Allocation*/
  for (int i = 0; i < MAX_DEVICE_NUM; i++){
    if (st_ClassMgr[i].state == HCD_UNUSED){
      clsIdx = i;
      st_ClassMgr[i].device = device;
      break;
    }
  }
  if (clsIdx < 0){
    return HCD_FULL;
  }

  /*The Head of Config Raw Desc should be a Configuration Descriptor*/
	if (confRaw->rawDesc[confRaw->readPtr] != 0x09 || confRaw->rawDesc[confRaw->readPtr + 1] != DESCTYPE_CONFIG){
		return HCD_INVALID_DESC;
	}

	/*Confoguration Descriptor Copy*/
	bLength = confRaw->rawDesc[confRaw->readPtr];
	memcpy(configDesc, &confRaw->rawDesc[confRaw->readPtr], bLength);
	confRaw->readPtr += bLength;
	instRdPtr += bLength;

  /*Parse Interface Descriptor/Interface Assoc Descriptor*/
  numIfs = configDesc->desc.bNumInterfaces;
  clsCount = 0;
  for (int j = 0; j < numIfs;){
    switch(confRaw->rawDesc[confRaw->readPtr + 1]){
      case(DESCTYPE_INTERFACE):{
        intfDesc = (usbDesc_Interface_t*)(&confRaw->rawDesc[confRaw->readPtr]);
        curCls = intfDesc->bInterfaceClass;
        if (clsNew(clsIdx, curCls) == 0){
          st_ClassMgr[clsIdx].clsCode[clsCount] = curCls;
          clsCount++;
        }
        if (st_ClassDriver[curCls].parseInterface){
          retReadPtr = st_ClassDriver[curCls].parseInterface(confRaw, device);
          if (retReadPtr){
            confRaw->readPtr += retReadPtr;
          } else {
            return HCD_UNSUPPORTED_CLASS;
          }
        } else {
          return HCD_UNSUPPORTED_CLASS;
        }
        j++;
        break;
      }
      case(DESCTYPE_INTERFACEASSOC):{
        iad = (usbDesc_InterfaceAssoc_t*)(&confRaw->rawDesc[confRaw->readPtr]);
        curCls = iad->bFunctionClass;
        if (clsNew(clsIdx, curCls) == 0){
          st_ClassMgr[clsIdx].clsCode[clsCount] = curCls;
          clsCount++;
        }
        if (st_ClassDriver[curCls].parseIAD){
          retReadPtr = st_ClassDriver[curCls].parseIAD(confRaw, device);
          if (retReadPtr){
            confRaw->readPtr += retReadPtr;
          } else {
            return HCD_UNSUPPORTED_CLASS;
          }
        } else {
          return HCD_UNSUPPORTED_CLASS;
        }
        break;
      }
      default:
        return HCD_INVALID_DESC;
    }
  }
  if (confRaw->fullLength != confRaw->readPtr){
    return HCD_INVALID_DESC;
  }
  return HCD_OK;
}

hcd_Status_t InitClassDriver(hcd_DeviceInfo_t* device)
{
  hcd_ClassMgr_t* mgr = NULL;
  for (int i = 0; i < MAX_DEVICE_NUM; i++){
    if (st_ClassMgr[i].device == device){
      mgr = &st_ClassMgr[i];
      break;
    }
  }
  if (mgr == NULL){
    return HCD_INVALID_PARAM;
  }
  for (int i = 0; i < 3; i++){
    if (st_ClassDriver[mgr->clsCode[i]].initClass){
      st_ClassDriver[i].initClass(device);
    }
  }

  return HCD_OK;
}

hcd_Status_t RegisterClassDriver(hcd_ClassDriver_t* map, uint8_t clsCode)
{
  if (clsCode > MAX_DEFINED_CLASS_CODE){
    return HCD_UNSUPPORTED_CLASS;
  }
  st_ClassDriver[clsCode].parseInterface = map->parseInterface;
  st_ClassDriver[clsCode].parseIAD = map->parseIAD;
  st_ClassDriver[clsCode].initClass =  map->initClass;
  st_ClassDriver[clsCode].terinateClass = map->terinateClass;

  return HCD_OK;
}

