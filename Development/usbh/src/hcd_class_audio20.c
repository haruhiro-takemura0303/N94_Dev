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

static uint16_t parseControlInterface(config_rawdesc_t *confRaw, hcd_Audio_Endpoint_Info_t* intr, hcd_DeviceInfo_t* device)
{
  hcd_UAC20_Info_t* info;
  uint8_t nextDescType, nextDescSubType, entityID, curClkNum;
  UAC_SubClass_t ifSubClass;
  usbDesc_Interface_t* intfPtr;
  uint16_t rdIdx, descInc;
  
  info = getInfo(device);
  if (!info && st_NumOfDevice < NUM_OF_MAX_AUDIO_DEVICE){
    info = &st_Info[st_NumOfDevice];
    info->device = device;
    st_NumOfDevice++;
  }
  descInc = 0;
  rdIdx = confRaw->readPtr;
  intfPtr = (usbDesc_Interface_t*)&confRaw->rawDesc[rdIdx];
  ifSubClass = (UAC_SubClass_t)(intfPtr->bInterfaceSubclass);
  if (ifSubClass != UAC_CONTROL){
    return 0;
  }
  info->control.intfPtr = intfPtr;
  rdIdx += intfPtr->bLength;
  descInc += intfPtr->bLength;
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
          case(SELECTOR_UNIT):
          case(EXTENSION_UNIT):{
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
            csUsbDesc_AudioCtrlIfClkSrc_t* acifCs = (csUsbDesc_AudioCtrlIfClkSrc_t*)(&confRaw->rawDesc[rdIdx]);
            entityID = acifCs->bClockID;
            if (entityID >= HCD_UAC20_MAX_ENTITY_ID){
              return 0;
            }
            if (info->control.clock.numOfClockSrc >= HCD_UAC20_MAX_CLK_SRC){
              return 0;
            }
            curClkNum = info->control.clock.numOfClockSrc;
            info->control.entity[entityID].descPtr = acifCs;
            info->control.entity[entityID].sourceID = HCD_UAC20_CLOCK_ENTITY;
            info->control.entity[entityID].cSourceID = HCD_UAC20_SOURCE_ROOT;
            info->control.entity[entityID].type = CLOCK_SOURCE;
            info->control.clock.clockSrc[curClkNum].clockID = entityID;
            info->control.clock.numOfClockSrc++;
            break;
          }
          case (CLOCK_SELECTOR): {
            csUsbDesc_AudioCtrlIfClkSel_t* acifCsel = (csUsbDesc_AudioCtrlIfClkSel_t*)(&confRaw->rawDesc[rdIdx]);
            entityID = acifCsel->bClockID;
            if (entityID >= HCD_UAC20_MAX_ENTITY_ID){
              return 0;
            }
            info->control.entity[entityID].descPtr = acifCsel;
            info->control.entity[entityID].sourceID = HCD_UAC20_CLOCK_ENTITY;
            info->control.entity[entityID].cSourceID = HCD_UAC20_SOURCE_MULTI;
            info->control.entity[entityID].type = CLOCK_SELECTOR;            
            break;
          }
          case (CLOCK_MULTIPLIER): {
            csUsbDesc_AudioCtrlIfClkMult_t* acifCmul = (csUsbDesc_AudioCtrlIfClkMult_t*)(&confRaw->rawDesc[rdIdx]);
            entityID = acifCmul->bClockID;
            if (entityID >= HCD_UAC20_MAX_ENTITY_ID){
              return 0;
            }
            info->control.entity[entityID].descPtr = acifCmul;
            info->control.entity[entityID].sourceID = HCD_UAC20_CLOCK_ENTITY;
            info->control.entity[entityID].cSourceID = acifCmul->bCSourceID;
            info->control.entity[entityID].type = CLOCK_MULTIPLIER;                
            break;
          }     
          default:{
            entityID = confRaw->rawDesc[rdIdx + 3];
            if (entityID >= HCD_UAC20_MAX_ENTITY_ID){
              return 0;
            }
            info->control.entity[entityID].descPtr = (void*)(&confRaw->rawDesc[rdIdx]);
            info->control.entity[entityID].type = confRaw->rawDesc[rdIdx + 2];
            info->control.entity[entityID].sourceID = confRaw->rawDesc[rdIdx + 4];
            info->control.entity[entityID].cSourceID = HCD_UAC20_CLOCK_NOT_CONNECTED;
            break;
          }
        }
        break;
      }
      case(ENDPOINT):{
        usbDesc_Endpoint2_t* epPtr = (usbDesc_Endpoint2_t*)(&confRaw->rawDesc[rdIdx]);
        if ((epPtr->bmAttributes & 0x03) != TYPE_INTERRUPT){
          return 0;
        }
        info->control.epPtr = epPtr;
        intr->interval = epPtr->bInterval;
        intr->mps = U16FromU8x2(epPtr->wMaxPacketSize_msB, epPtr->wMaxPacketSize_lsB);
        intr->intfNum = info->control.intfPtr->bInterfaceNumber;
        intr->num = epPtr->bEndpointAddress;
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

uint16_t parseStreamingInterface(config_rawdesc_t *confRaw, hcd_Audio_Endpoint_Info_t* isochOutEp, hcd_Audio_Endpoint_Info_t* isochInEp, hcd_DeviceInfo_t* device)
{
  hcd_UAC20_Info_t* info;
  uint8_t nextDescType, nextDescSubType, curAltNum;
  UAC_SubClass_t ifSubClass;
  usbDesc_Interface_t* intfPtr;
  uint16_t rdIdx, descInc, instIdx;
  hcd_UAC20_AltSet_t* curAlt;
  
  info = getInfo(device);
  if (!info && st_NumOfDevice < NUM_OF_MAX_AUDIO_DEVICE){
    info = &st_Info[st_NumOfDevice];
    info->device = device;
    st_NumOfDevice++;
  }
  descInc = 0;
  rdIdx = confRaw->readPtr;
  intfPtr = (usbDesc_Interface_t*)&confRaw->rawDesc[rdIdx];
  ifSubClass = (UAC_SubClass_t)(intfPtr->bInterfaceSubclass);
  if (ifSubClass != UAC_STREAMING){
    return 0;
  }
  
  if (intfPtr->bAlternateSetting >= HCD_UAC20_MAX_ALTSET){
    /*AltSet No.x(x >= HCD_UAC20_MAX_ALTSET) is ignored, but not error, go to next interface*/
    rdIdx += intfPtr->bLength;
    descInc += intfPtr->bLength;
    while ((confRaw->rawDesc[rdIdx + 1] != DESCTYPE_INTERFACE) && (rdIdx < confRaw->fullLength)){
      uint8_t bLen = confRaw->rawDesc[rdIdx];
      rdIdx += bLen;
      descInc += bLen;
    }
    return descInc;
  }
  
  curAltNum = intfPtr->bAlternateSetting;
  
  if (info->streamIn.altSet[0].intfPtr->bInterfaceNumber == intfPtr->bInterfaceNumber){
    curAlt = &info->streamIn.altSet[curAltNum];
  } else if (info->streamOut.altSet[0].intfPtr->bInterfaceNumber == intfPtr->bInterfaceNumber){
    curAlt = &info->streamOut.altSet[curAltNum];
  } else {
    instIdx = rdIdx;
    while (confRaw->rawDesc[instIdx + 1] != DESCTYPE_ENDPOINT){
      instIdx += confRaw->rawDesc[instIdx];
    }
    usbDesc_Endpoint2_t* epPtr = (usbDesc_Endpoint2_t*)(&confRaw->rawDesc[instIdx]);
    if (epPtr->bEndpointAddress & 0x80){
      curAlt = &info->streamIn.altSet[curAltNum];
    } else {
      curAlt = &info->streamOut.altSet[curAltNum];
    }
  }

  curAlt->intfPtr = intfPtr;
  
  rdIdx += intfPtr->bLength;
  descInc += intfPtr->bLength;
  nextDescType = confRaw->rawDesc[rdIdx + 1];
  nextDescSubType = confRaw->rawDesc[rdIdx + 2];
  while ((nextDescType != DESCTYPE_INTERFACE) && (rdIdx < confRaw->fullLength)){
    switch(nextDescType){
      case(CS_INTERFACE):{
        switch(nextDescSubType){
          case(IF_GENERAL): curAlt->strmIfPtr = (csUsbDesc_AudioStrmIf2_t*)(&confRaw->rawDesc[rdIdx]); break;
          case(FORMAT_TYPE): curAlt->fmtPtr = (csUsbDesc_AudioStrmFmtTypI2_t*)(&confRaw->rawDesc[rdIdx]); break;
          default: break;
        }
        break;
      }
      case(ENDPOINT):{
        usbDesc_Endpoint2_t* epPtr = (usbDesc_Endpoint2_t*)(&confRaw->rawDesc[rdIdx]);
        curAlt->epPtr = epPtr;
        if (epPtr->bEndpointAddress & 0x80 && (isochInEp->num == 0)){
          isochInEp->intfNum = intfPtr->bInterfaceNumber;
          isochInEp->num = epPtr->bEndpointAddress;
        } else if (!(epPtr->bEndpointAddress & 0x80) && (isochOutEp->num == 0)){
          isochOutEp->intfNum = intfPtr->bInterfaceNumber;
          isochOutEp->num = epPtr->bEndpointAddress;
        }
        break;
      }
      case(CS_ENDPOINT):{
        switch(nextDescSubType){
          case(EP_GENERAL):curAlt->csEpPtr = (csUsbDesc_AudioStrmDataEndpt2_t*)(&confRaw->rawDesc[rdIdx]); break;
          default: break;
        }
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

hcd_Status_t sendInitialRequest(hcd_DeviceInfo_t* device)
{
  hcd_UAC20_Info_t* info;
  
  info = getInfo(device);
  if (!info){
    return HCD_NULL;
  }



}
