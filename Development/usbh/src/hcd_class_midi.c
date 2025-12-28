/**
* @brief   MCXN947V USB Host Controller MIDI Class Driver
* @author  masa
* @version 1.00 
*/

#include "hcd_class_midi.h"

uint8_t st_NrMidiDevice;
hcd_MIDI_Info_t st_MidiInfo[NUM_OF_MAX_MIDI_DEVICE];
hcd_MIDI_MsgBox_t st_MidiMsgBox;
hcd_MIDI_RingBuf_t st_OutRingBuf[NUM_OF_MAX_MIDI_DEVICE];
usb_MidiPacket_t st_BulkOutBuf[NUM_OF_MAX_MIDI_DEVICE][64];
usb_MidiPacket_t st_BulkInBuf[NUM_OF_MAX_MIDI_DEVICE][64];

void midiIn(usb_MidiPacket_t* buf, uint16_t nrMidiPkt);

static hcd_Status_t enqueueMsg(hcd_MIDI_Msg_t* msg)
{
  hcd_Status_t ret;
  EHCI_DisInt();
  if (((st_MidiMsgBox.enqPtr + 1) % HCD_MIDI_MSGBOX_SIZE) != st_MidiMsgBox.deqPtr){
    memcpy(&st_MidiMsgBox.msg[st_MidiMsgBox.enqPtr], msg, sizeof(hcd_MIDI_Msg_t));
    st_MidiMsgBox.enqPtr++;
    if (st_MidiMsgBox.enqPtr == HCD_MIDI_MSGBOX_SIZE){
      st_MidiMsgBox.enqPtr = 0;
    }
    ret = HCD_OK;
  } else {
    ret = HCD_FULL;
  }
  
  EHCI_EnaInt();
  NVIC_SetPendingIRQ(HcdMIDI_IRQn);
  
  return ret;
}

static hcd_Status_t dequeueMsg(hcd_MIDI_Msg_t* msg)
{
  hcd_Status_t ret;
  EHCI_DisInt();
  if (st_MidiMsgBox.deqPtr != st_MidiMsgBox.enqPtr){
    memcpy(msg, &st_MidiMsgBox.msg[st_MidiMsgBox.deqPtr], sizeof(hcd_MIDI_Msg_t));
    st_MidiMsgBox.deqPtr++;
    if (st_MidiMsgBox.deqPtr == HCD_MIDI_MSGBOX_SIZE){
      st_MidiMsgBox.deqPtr = 0;
    }
    ret = HCD_OK;
  } else {
    ret = HCD_FULL;
  }
  EHCI_EnaInt();
  return ret;
}

static hcd_MIDI_Info_t* getInfo(uint8_t devAddr)
{
  hcd_MIDI_Info_t* ret = NULL;
  for (int i = 0; i < st_NrMidiDevice; i++){
    if (st_MidiInfo[i].device && (st_MidiInfo[i].device->devAddr == devAddr)){
      ret = &st_MidiInfo[i];
      break;
    }
  }
  return ret;    
}

static hcd_MIDI_Info_t* getInfoFromIndex(uint8_t index)
{
  hcd_MIDI_Info_t* ret = NULL;
  for (int i = 0; i < st_NrMidiDevice; i++){
    if (st_MidiInfo[i].index  == index){
      ret = &st_MidiInfo[i];
      break;
    }
  }
  return ret;    
}

static hcd_MIDI_RingBuf_t* getRingBuf(uint8_t devAddr)
{
  hcd_MIDI_RingBuf_t* ret = NULL;
  for (int i = 0; i < st_NrMidiDevice; i++){
    if (st_OutRingBuf[i].devAddr == devAddr){
      ret = &st_OutRingBuf[i];
      break;
    }
  }
  return ret;    
}

static uint16_t parseInterface(config_rawdesc_t* confRaw, hcd_DeviceInfo_t* device)
{
  hcd_MIDI_Info_t* info;
  uint16_t descInc = 0;
  volatile uint16_t rdIdx = confRaw->readPtr;
  usbDesc_Interface_t* intfPtr = (usbDesc_Interface_t*)&confRaw->rawDesc[rdIdx];
  uint8_t nextDescType, nextDescSubType, uacSubType, currentEp;
  
  info = getInfo(device->devAddr);
  if (info == NULL){
    if (st_NrMidiDevice < NUM_OF_MAX_MIDI_DEVICE){
      for (int i = 0; i < NUM_OF_MAX_MIDI_DEVICE; i++){
        if (!st_MidiInfo[i].device){
          info = &st_MidiInfo[i];
          st_MidiInfo[i].device = device;
          st_NrMidiDevice++;
          break;
        }
      }
    } else {
      return 0;
    }
  }
  
  uacSubType = intfPtr->bInterfaceSubclass;
  if (uacSubType == UAC_MIDI){
    info->intf = intfPtr;
  }
  rdIdx += intfPtr->bLength;
  descInc += intfPtr->bLength;
  nextDescType = confRaw->rawDesc[rdIdx + 1];
  currentEp = 0xFF;
  while ((nextDescType != DESCTYPE_INTERFACE) && (rdIdx < confRaw->fullLength)){
    if(uacSubType == UAC_MIDI){
      switch(nextDescType){
        case(DESCTYPE_CSIF):{
					nextDescSubType = confRaw->rawDesc[rdIdx + 2];
          switch(nextDescSubType){
            case(MS_HEADER):{
              csUsbDesc_MidiStrmIfHdr_t* msHdr = (csUsbDesc_MidiStrmIfHdr_t*)(&confRaw->rawDesc[rdIdx]);
              if (U16FromU8x2(msHdr->bcdMSC_msB, msHdr->bcdMSC_lsB) != 0x0100){
                return 0;
              }
              break;
            }
            case(MIDI_IN_JACK):{
              for (int i = 0; i < HCD_MIDI_MAX_JACK_NUM; i++){
                if (info->inJackTbl[i] == NULL){
                  info->inJackTbl[i] = (csUsbDesc_MidiInJk_t*)(&confRaw->rawDesc[rdIdx]);
                  break;
                }
              }
              break;
            }
            case(MIDI_OUT_JACK):{
              for (int i = 0; i < HCD_MIDI_MAX_JACK_NUM; i++){
                if (info->outJackTbl[i] == NULL){
                  info->outJackTbl[i] = (csUsbDesc_MidiOutJk_t*)(&confRaw->rawDesc[rdIdx]);
                  break;
                }
              }
              break;
            }
            default:
            break;
          }
          break;
        }
        case(DESCTYPE_ENDPOINT):{
          usbDesc_Endpoint_t* epPtr = (usbDesc_Endpoint_t*)(&confRaw->rawDesc[rdIdx]);
          if (epPtr->bEndpointAddress & 0x80){
            info->bulkIn.epDesc = epPtr;
          } else {
            info->bulkOut.epDesc = epPtr;
          }
          currentEp = epPtr->bEndpointAddress;
          break;
        }
        case(DESCTYPE_CSEP):{
					nextDescSubType = confRaw->rawDesc[rdIdx + 2];
          switch(nextDescSubType){
            case(MS_GENERAL):{
              csUsbDesc_MidiStrmBulkEndpt_t* csEp = (csUsbDesc_MidiStrmBulkEndpt_t*)(&confRaw->rawDesc[rdIdx]);
              if ((currentEp != 0xFF) && (currentEp & 0x80)){
                info->bulkIn.csEpDesc = csEp;
              } else if (!(currentEp & 0x80)){
                info->bulkOut.csEpDesc = csEp;
              } else if ((currentEp == 0xFF))
              {
                return 0;
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

static void bulkOutComplete(uint8_t devAddr, uint8_t epNum, uint16_t transLen)
{
  hcd_MIDI_Msg_t msg;
  msg.msgType = HCD_MIDI_BULK_OUT_COMP;
  msg.devAddr = devAddr;
  msg.transLen = transLen;
  enqueueMsg(&msg);
}

static void bulkInComplete(uint8_t devAddr, uint8_t epNum, uint16_t transLen)
{
  hcd_MIDI_Msg_t msg;
  msg.msgType = HCD_MIDI_BULK_IN;
  msg.devAddr = devAddr;
  msg.transLen = transLen;
  enqueueMsg(&msg);  
}

static void initClass(hcd_DeviceInfo_t* device)
{
  hcd_MIDI_Info_t* info = getInfo(device->devAddr);
  uint8_t inJkID, curNrCables;
  hcd_Status_t stat;
  uint16_t mps;
  
  for (int i = 0; i < HCD_MIDI_MAX_JACK_NUM; i++){
    if (info->inJackTbl[i] == NULL){
      break;
    }
    inJkID = info->inJackTbl[i]->bJackID;
    for (int j = 0; j < HCD_MIDI_MAX_JACK_NUM; j++){
      if (info->outJackTbl[j] == NULL){
        break;
      }
      if (info->outJackTbl[j]->baSourceID == inJkID){
        if (info->inJackTbl[i]->bJackType == JACK_EMBEDDED){
          curNrCables = info->bulkOut.nrCables;
          info->bulkOut.cables[curNrCables].inJack = info->inJackTbl[i];
          info->bulkOut.cables[curNrCables].outJack = info->outJackTbl[j];
          info->bulkOut.nrCables++;
        } else {
          curNrCables = info->bulkIn.nrCables;
          info->bulkIn.cables[curNrCables].inJack = info->inJackTbl[i];
          info->bulkIn.cables[curNrCables].outJack = info->outJackTbl[j];
          info->bulkIn.nrCables++;
        }
      }
    }
  }
  
  if (info->bulkOut.nrCables && info->bulkOut.epDesc){
    stat = OpenAsyncEndpoint(device, info->bulkOut.epDesc->bEndpointAddress, (uint32_t*)info->bulkOut.dataBuf, U16FromU8x2(info->bulkOut.epDesc->wMaxPacketSize_msB, info->bulkOut.epDesc->wMaxPacketSize_lsB), bulkOutComplete);
    if (stat){
      CloseAsyncEndpoint(device->devAddr, info->bulkOut.epDesc->bEndpointAddress);
    } else if (info->midiDeviceReady){
      info->midiDeviceReady(info->index, MIDI_DIR_OUT);
    }
  }
  if (info->bulkIn.nrCables && info->bulkIn.epDesc){
    mps = U16FromU8x2(info->bulkIn.epDesc->wMaxPacketSize_msB, info->bulkIn.epDesc->wMaxPacketSize_lsB);
    stat = OpenAsyncEndpoint(device, info->bulkIn.epDesc->bEndpointAddress, (uint32_t*)info->bulkIn.dataBuf, mps, bulkInComplete);
    if (stat){
      CloseAsyncEndpoint(device->devAddr, info->bulkIn.epDesc->bEndpointAddress);
    } else {
      if (info->midiDeviceReady){
        info->midiDeviceReady(info->index, MIDI_DIR_IN);
      }
      HcdAsync_StartTransfer(device->devAddr, info->bulkIn.epDesc->bEndpointAddress, mps);
    }
  }
}

static void terminateClass(hcd_DeviceInfo_t* device)
{
  
}

static inline uint32_t getCurrentMidiSize(uint8_t devAddr)
{ 
  hcd_MIDI_RingBuf_t* buf = getRingBuf(devAddr);
  uint32_t enq = buf->enqPtr;
  uint32_t deq = buf->deqPtr;
  return (enq >= deq) ? (enq - deq) : (HCD_MIDI_OUT_RINGBUF_SIZE - (deq - enq));
}

static inline uint32_t getCurrentFreeSize(uint8_t devAddr)
{
  return (HCD_MIDI_OUT_RINGBUF_SIZE - 1u) - getCurrentMidiSize(devAddr);
}

static void bufCopy(uint8_t devAddr)
{
  uint32_t deq, first, size;
  hcd_MIDI_RingBuf_t* buf = getRingBuf(devAddr);
  hcd_MIDI_Info_t* info = getInfo(devAddr);
  
  size = getCurrentMidiSize(devAddr);
  deq = buf->deqPtr;
  first = HCD_MIDI_OUT_RINGBUF_SIZE - (deq % HCD_MIDI_OUT_RINGBUF_SIZE);
  if (first > size){
    first = size;
  }
  for (int i = 0; i < first; i++){
    info->bulkOut.dataBuf[i].DWORD = buf->umidi[(deq + i) % HCD_MIDI_OUT_RINGBUF_SIZE].DWORD;
  }
  for (int i = first; i < size; i++){
    info->bulkOut.dataBuf[i].DWORD = buf->umidi[(deq + i) % HCD_MIDI_OUT_RINGBUF_SIZE].DWORD;
  }
  buf->deqPtr = buf->enqPtr;
}

static hcd_Status_t enqueueBuf(uint8_t devAddr, usb_MidiPacket_t umidi)
{
  hcd_MIDI_RingBuf_t* buf = getRingBuf(devAddr);
  if (!getCurrentFreeSize(devAddr)){
    return HCD_FULL;
  }
  buf->umidi[buf->enqPtr % HCD_MIDI_OUT_RINGBUF_SIZE].DWORD = umidi.DWORD;
  buf->enqPtr++;

  return HCD_OK;
}


static void midiClassTask(void)
{
  int ret;
  hcd_MIDI_Msg_t msg;
  hcd_Status_t status;
  hcd_MIDI_Info_t* info;
  uint16_t mps, nrMidiPkts;
  
  NVIC_ClearPendingIRQ(HcdMIDI_IRQn);
  
  for(;;){
    status = dequeueMsg(&msg);
    if (status != HCD_OK){
      break;
    }
    info = getInfo(msg.devAddr);
    if (info == NULL){
      continue;
    }
    switch(msg.msgType){
      case(HCD_MIDI_BULK_IN):{
        mps = U16FromU8x2(info->bulkIn.epDesc->wMaxPacketSize_msB, info->bulkIn.epDesc->wMaxPacketSize_lsB);
        nrMidiPkts = msg.transLen >> 2;
        if (info->bulkIn.completeCallback){
          info->bulkIn.completeCallback(&info->bulkIn.dataBuf[0], nrMidiPkts);
        }
        HcdAsync_StartTransfer(msg.devAddr, info->bulkIn.epDesc->bEndpointAddress, mps);
        break;
      }
      case(HCD_MIDI_BULK_OUT_COMP):{
        nrMidiPkts = (uint16_t)getCurrentMidiSize(msg.devAddr);
        if (nrMidiPkts){
          bufCopy(msg.devAddr);
          HcdAsync_StartTransfer(msg.devAddr, info->bulkOut.epDesc->bEndpointAddress, nrMidiPkts);
        }
        break;
      }
      case(HCD_MIDI_BULK_OUT_SEND):{
        ret = HcdAsync_GetTransferState(msg.devAddr, info->bulkOut.epDesc->bEndpointAddress);
        if (ret == -2){
          enqueueBuf(msg.devAddr, msg.umidiPkt);
        }
        break;
      }
      default:
      break;
    }
  }  
}

void UsbhMIDI_SetReadyNotify(void func(uint8_t, uint8_t))
{
  for (int i = 0; i < NUM_OF_MAX_MIDI_DEVICE; i++){
    st_MidiInfo[i].midiDeviceReady = func;
  }
}

void UsbhMIDI_SetInCallback(uint8_t index, void func(usb_MidiPacket_t* buf, uint16_t nrMidiPkt))
{
  hcd_MIDI_Info_t* info = getInfoFromIndex(index);
  if (info){
    info->bulkIn.completeCallback = func;
  }
}

void HcdMIDI_InitMidiClass(void)
{
  hcd_ClassDriver_t comDriver;
  for (int i = 0; i < NUM_OF_MAX_MIDI_DEVICE; i++){
    st_MidiInfo[i].index = i + 1;
    st_MidiInfo[i].bulkIn.dataBuf = &st_BulkInBuf[i][0];
    st_MidiInfo[i].bulkOut.dataBuf = &st_BulkOutBuf[i][0];
  }
  comDriver.parseInterface = parseInterface;
  comDriver.parseIAD = parseIAD;
  comDriver.initClass = initClass;
  comDriver.terinateClass = terminateClass;
  
  NVIC_SetPriority(HcdMIDI_IRQn, 4);
  NVIC_SetVector(HcdMIDI_IRQn, (uint32_t)midiClassTask);
  NVIC_EnableIRQ(HcdMIDI_IRQn);
  
  HcdAudioMgr_RegisterMidiDriver(&comDriver);
  
}
