/**
* @brief   MCXN947V USB Host Controller Audio Class 2.0 Driver
* @author  masa
* @version 1.00 
*/

#include "hcd_class_audio20.h"
#include "hcd_class_mgr_audio.h"

static uint8_t st_NumOfDevice;
static hcd_UAC20_Info_t st_Info[NUM_OF_MAX_AUDIO_DEVICE];

static hcd_UAC20_RequestOut_Buf_t st_ReqOutBuf[HCD_UAC20_MAX_OUT_REQUEST_NUM];
static hcd_UAC20_RequestIn_Buf_t st_ReqInBuf[HCD_UAC20_MAX_IN_REQUEST_NUM];
static hcd_UAC20_PendRequest_Box_t st_PendBox;

static hcd_Status_t enqueue(usb_SetupPacket_t* setup)
{
  hcd_Status_t ret;
  if (st_PendBox.deqPtr - st_PendBox.enqPtr != 1){
    memcpy(&st_PendBox.pendedSetup[st_PendBox.enqPtr], setup, sizeof(usb_SetupPacket_t));
    st_PendBox.enqPtr++;
    if (st_PendBox.enqPtr == HCD_PERIODIC_MSGBOX_SIZE){
      st_PendBox.enqPtr = 0;
    }
    ret = HCD_OK;
  } else {
    ret = HCD_FULL;
  }
  return ret;
}

static hcd_Status_t dequeue(usb_SetupPacket_t* setup)
{
  hcd_Status_t ret;
  if (st_PendBox.deqPtr != st_PendBox.enqPtr){
    memcpy(setup, &st_PendBox.pendedSetup[st_PendBox.deqPtr], sizeof(usb_SetupPacket_t));
    st_PendBox.deqPtr++;
    if (st_PendBox.deqPtr == HCD_PERIODIC_MSGBOX_SIZE){
      st_PendBox.deqPtr = 0;
    }
    ret = HCD_OK;
  } else {
    ret = HCD_FULL;
  }
  return ret;
}

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

hcd_Status_t createRequest(usb_SetupPacket_t* setup, uint32_t data0, uint32_t data1)
{
  int i = 0;
  hcd_Audio_Msg_t msg;
  if (setup->BIT.bmRequestType.dir == BMREQ_DIR_IN){
    for (i = 0; i < HCD_UAC20_MAX_IN_REQUEST_NUM; i++){
      if ((st_ReqInBuf[i].setup.DWORD[0] == 0) && (st_ReqInBuf[i].setup.DWORD[1] == 0)){
        st_ReqInBuf[i].setup.DWORD[0] = setup->DWORD[0];
        st_ReqInBuf[i].setup.DWORD[1] = setup->DWORD[1];
        msg.other.setup.DWORD[0] = setup->DWORD[0];
        msg.other.setup.DWORD[1] = setup->DWORD[1];
        HcdAudio_SendMsg(&msg);
        break;
      }
    }
    if (i == HCD_UAC20_MAX_IN_REQUEST_NUM){
      return enqueue(setup);
    }
  } else {
    for (i = 0; i < HCD_UAC20_MAX_OUT_REQUEST_NUM; i++){
      if ((st_ReqOutBuf[i].setup.DWORD[0] == 0) && (st_ReqOutBuf[i].setup.DWORD[1] == 0)){
        st_ReqOutBuf[i].setup.DWORD[0] = setup->DWORD[0];
        st_ReqOutBuf[i].setup.DWORD[1] = setup->DWORD[1];
        msg.other.setup.DWORD[0] = setup->DWORD[0];
        msg.other.setup.DWORD[1] = setup->DWORD[1];
        if (setup->BIT.wLength > 0){
          st_ReqOutBuf[i].dataBuf[0] = data0;
          st_ReqOutBuf[i].dataBuf[1] = data1;
          msg.bufPtr = &st_ReqOutBuf[i].dataBuf[0];
        }
        HcdAudio_SendMsg(&msg);
        break;
      }
    }
    if (i == HCD_UAC20_MAX_OUT_REQUEST_NUM){
      return HCD_FULL;
    }        
  }
  return HCD_OK;
}

hcd_Status_t sendInitialRequest(hcd_DeviceInfo_t* device)
{
  hcd_UAC20_Info_t* info;
  usb_SetupPacket_t setup;
  uint8_t clockID, intfNum;
  hcd_Status_t ret;
  
  info = getInfo(device);
  if (!info){
    return HCD_NULL;
  }
  
  intfNum = info->control.intfPtr->bInterfaceNumber;
  
  for (int i = 0; i < info->control.clock.numOfClockSrc; i++){
    clockID = info->control.clock.clockSrc[i].clockID;
    MakeSETUPPacket(BMREQ_DIR_IN, BMREQ_TYPE_CLASS, BMREQ_ATTR_INTERFACE, BREQ_RANGE, UAC_WVALUE_CONTROL_SEL(CS_SAMFREQ_CONTROL), UAC_WINDEX_ENTITY(clockID) | intfNum, HCD_UAC20_IN_REQUEST_DATA_SIZE, &setup);
    ret = createRequest(&setup, 0, 0);
    if (ret){
      break;
    }
  }
  
  return ret;
}

void requestDone(hcd_DeviceInfo_t* device, uint32_t setup0, uint32_t setup1)
{
  hcd_UAC20_Info_t* info;
  usb_SetupPacket_t setup;
  uint8_t entityID, entityType, ctrlSel, intfNum;
  uint32_t* dataBuf;
  
  info = getInfo(device);
  if (!info){
    return;
  }
  
  setup.DWORD[0] = setup0;
  setup.DWORD[1] = setup1;
  
  if (setup.BIT.bmRequestType.dir == BMREQ_DIR_IN){
    for (int i = 0; i < HCD_UAC20_MAX_IN_REQUEST_NUM; i++){
      if ((setup0 == st_ReqInBuf[i].setup.DWORD[0]) && (setup1 == st_ReqInBuf[i].setup.DWORD[1])){
        st_ReqInBuf[i].setup.DWORD[0] = 0;
        st_ReqInBuf[i].setup.DWORD[1] = 0;
        dataBuf = &st_ReqInBuf[i].dataBuf[0];
        break;
      }
    }
  } else {
    for (int i = 0; i < HCD_UAC20_MAX_OUT_REQUEST_NUM; i++){
      if ((setup0 == st_ReqOutBuf[i].setup.DWORD[0]) && (setup1 == st_ReqOutBuf[i].setup.DWORD[1])){
        st_ReqOutBuf[i].setup.DWORD[0] = 0;
        st_ReqOutBuf[i].setup.DWORD[1] = 0;
        dataBuf = &st_ReqOutBuf[i].dataBuf[0];
        break;
      }
    }    
  }
  
  switch(setup.BIT.bmRequestType.type){
    case(BMREQ_TYPE_STANDARD):{
      break;
    }
    case(BMREQ_TYPE_CLASS):{

      switch(setup.BIT.bmRequestType.attr){
        case(BMREQ_ATTR_INTERFACE):{
          intfNum = setup.BIT.wIndex & 0xFF;
          if (intfNum == info->control.intfPtr->bInterfaceNumber){
            /*Audio Control Request*/
            entityID = setup.BIT.wIndex >> 8;
            entityType = info->control.entity[entityID].type;

            switch(entityType){
              case(CLOCK_SOURCE):{
                ctrlSel = setup.BIT.wValue >> 8;
                
                switch(ctrlSel){
                  case(CS_SAMFREQ_CONTROL):{

                    switch(setup.BIT.bRequest){
                      case(BREQ_CUR):{
                        break;
                      }
                      case(BREQ_RANGE):{
                        hcd_UAC20_ClockSrcInfo_t* clk;
                        uint16_t numSubRange;
                        uint16_t* buf_u16;
                        if (setup.BIT.bmRequestType.dir == BMREQ_DIR_IN){
                          numSubRange = dataBuf[0] & 0xFFFF;
                          for (int j = 0; j < info->control.clock.numOfClockSrc; j++){
                            if (entityID == info->control.clock.clockSrc[j].clockID){
                              clk = &info->control.clock.clockSrc[j];
                              break;
                            }
                          }

                          if (numSubRange > HCD_UAC20_MAX_CLK_SRC_SUBRANGE){
                            numSubRange = HCD_UAC20_MAX_CLK_SRC_SUBRANGE;
                          }
                          clk->numOfSubrange = numSubRange;
                          buf_u16 = (uint16_t*)dataBuf;
                          for (int j = 0; j < numSubRange; j++){
                            clk->subRange[j].dMin = U32FromU16x2(buf_u16[6*j + 2], buf_u16[6*j + 1]);
                            clk->subRange[j].dMax = U32FromU16x2(buf_u16[6*j + 4], buf_u16[6*j + 3]);
                            clk->subRange[j].dRes = U32FromU16x2(buf_u16[6*j + 6], buf_u16[6*j + 5]);
                          }
                        }
                        break;
                      }
                      default:
                        break;
                    }
                    break;
                  }
                  case(CS_CLOCK_VALID_CONTROL):{
                    break;
                  }
                  default:
                  break;
                }
                break;
              }
              case(CLOCK_SELECTOR):{
                break;
              }
              default:
              break;
            }
          } else if ((intfNum == info->streamIn.altSet[0].intfPtr->bInterfaceNumber) || (intfNum == info->streamOut.altSet[0].intfPtr->bInterfaceNumber)){
            /*Audio Streaming Request*/
          }
          break;
        }
        default:
        break;
      }
      break;
    }
    default:
    break;
  }
}
