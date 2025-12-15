/**
* @brief   MCXN947V USB Host Controller Audio Class 2.0 Driver
* @author  masa
* @version 1.00 
*/

#include "hcd_class_audio20.h"
#include "hcd_class_mgr_audio.h"

static uint8_t st_NumOfDevice;
static hcd_UAC20_Info_t st_Info[NUM_OF_MAX_AUDIO_DEVICE];

static hcd_UAC20_Info_t* getInfo(hcd_DeviceInfo_t* device)
{
  hcd_UAC20_Info_t* ret = NULL;
  for (int i = 0; i < st_NumOfDevice; i++){
    if (st_Info[i].device == device){
      ret = &st_Info[i];
      break;
    }
  }
  return ret;  
}

static uint16_t parseControlInterface(config_rawdesc_t *confRaw, hcd_Audio_Endpoint_Info_t* intf, hcd_DeviceInfo_t* device)
{
  hcd_UAC20_Info_t* info;
  uint8_t nextDescType, nextDescSubType, entityID;
  UAC_SubClass_t ifSubClass;
  usbDesc_Interface_t* descPtr;
  uint16_t rdIdx, descInc;
  
  info = getInfo(device);
  if (!info && st_NumOfDevice < NUM_OF_MAX_AUDIO_DEVICE){
    info = &st_Info[st_NumOfDevice];
    info->device = device;
    st_NumOfDevice++;
  }
  descInc = 0;
  rdIdx = confRaw->readPtr;
  descPtr = (usbDesc_Interface_t*)&confRaw->rawDesc[rdIdx];
  ifSubClass = (UAC_SubClass_t)(descPtr->bInterfaceSubclass);
  if (ifSubClass != UAC_CONTROL){
    return 0;
  }
  info->control.intfPtr = descPtr;
  rdIdx += descPtr->bLength;
  descInc += descPtr->bLength;
  nextDescType = confRaw->rawDesc[rdIdx + 1];
  nextDescSubType = confRaw->rawDesc[rdIdx + 2];
  while ((nextDescType != DESCTYPE_INTERFACE) && (rdIdx < confRaw->fullLength)){
    switch(nextDescType){
      case(CS_INTERFACE):{
        switch(nextDescSubType){
          case(HEADER):{
            csUsbDesc_AudioCtrlIfHdr2_t* acifhdr = (csUsbDesc_AudioCtrlIfHdr2_t*)(&confRaw->rawDesc[rdIdx]);
            if (acifhdr->bCategory > CATEGORY_CONTROL_PANEL){
              return 0;
            }
            info->category = acifhdr->bCategory;
            break;
          }
          case(INPUT_TERMINAL):{
            csUsbDesc_AudioCtrlInputTerm2_t* acifIpt = (csUsbDesc_AudioCtrlInputTerm2_t*)(&confRaw->rawDesc[rdIdx]);
            if (acifIpt->bTerminalID >= HCD_UAC20_MAX_ENTITY_ID){
              return 0;
            }
            entityID = acifIpt->bTerminalID;
            info->control.entity[entityID].descPtr = acifIpt;
            info->control.entity[entityID].type = INPUT_TERMINAL;
            info->control.entity[entityID].cSourceID = acifIpt->bCSourceID;
            info->control.entity[entityID].sourceID = HCD_UAC20_SOURCE_ROOT;
            if (U16FromU8x2(acifIpt->wTerminalType_msB, acifIpt->wTerminalType_lsB) == TERMINAL_TYPE_USB){
              info->control.entity[entityID].usbTerm = 1;
            }
            break;
          }
          case(OUTPUT_TERMINAL):{
            csUsbDesc_AudioCtrlOutputTerm2_t* acifOpt = (csUsbDesc_AudioCtrlOutputTerm2_t*)(&confRaw->rawDesc[rdIdx]);
            if (acifOpt->bTerminalID >= HCD_UAC20_MAX_ENTITY_ID){
              return 0;
            }
            entityID = acifOpt->bTerminalID;
            info->control.entity[entityID].descPtr = acifOpt;
            info->control.entity[entityID].type = OUTPUT_TERMINAL;
            info->control.entity[entityID].cSourceID = acifOpt->bCSourceID;
            info->control.entity[entityID].sourceID = acifOpt->bSourceID;
            if (U16FromU8x2(acifOpt->wTerminalType_msB, acifOpt->wTerminalType_lsB) == TERMINAL_TYPE_USB){
              info->control.entity[entityID].usbTerm = 1;
            }
            break;
          }
          case(MIXER_UNIT):
          case(SELECTOR_UNIT):{
            entityID = confRaw->rawDesc[rdIdx + 3];
            if (entityID >= HCD_UAC20_MAX_ENTITY_ID){
              return 0;
            }
            info->control.entity[entityID].descPtr = (void*)(&confRaw->rawDesc[rdIdx]);
            info->control.entity[entityID].type = confRaw->rawDesc[rdIdx + 2];
            info->control.entity[entityID].sourceID = HCD_UAC20_SOURCE_MULTI;
            info->control.entity[entityID].cSourceID = HCD_UAC20_CLOCK_NOT_CONNECTED;
            break;
          }
          case (CLOCK_SOURCE): {

            break;
          }
          case (CLOCK_SELECTOR): {

            break;
          }
          case (CLOCK_MULTIPLIER): {

            break;
          }     
          default:{
            break;
          }
        }
        break;
      }
      case(ENDPOINT):{
        
        break;
      }
      default:
      break;
    }
    uint8_t bLen = confRaw->rawDesc[rdIdx];
    rdIdx += bLen;
    descInc += bLen;
    nextDescType = confRaw->rawDesc[rdIdx + 1];
    nextDescSubType = confRaw->rawDesc[rdIdx + 2];
  }
  return descInc;
}

