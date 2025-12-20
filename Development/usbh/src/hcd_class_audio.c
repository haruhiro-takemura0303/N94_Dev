/**
* @brief   MCXN947V USB Host Controller Audio Class Driver
* @author  masa
* @version 1.00 
*/

#include "hcd_class_mgr_audio.h"
#include "hcd_class_audio.h"

static uint8_t st_NumOfAudioDevice;
static hcd_Audio_Protocol_Driver_t st_UAC10;
static hcd_Audio_Protocol_Driver_t st_UAC20;
static hcd_Audio_Protocol_Driver_t *st_CurrentUAC[NUM_OF_MAX_AUDIO_DEVICE] = {NULL};
static hcd_Audio_Transfer_Driver_t st_Driver[NUM_OF_MAX_AUDIO_DEVICE];
__ALIGNED(4096) static uint32_t st_IsochOutBuf0[NUM_OF_MAX_AUDIO_DEVICE][256];
__ALIGNED(4096) static uint32_t st_IsochOutBuf1[NUM_OF_MAX_AUDIO_DEVICE][256];
__ALIGNED(4096) static hcd_Audio_IsochIn_Raw_Buf_t st_IsochInRawBuf0[NUM_OF_MAX_AUDIO_DEVICE];
__ALIGNED(4096) static hcd_Audio_IsochIn_Raw_Buf_t st_IsochInRawBuf1[NUM_OF_MAX_AUDIO_DEVICE];
static uint32_t st_IsochInContBuf0[NUM_OF_MAX_AUDIO_DEVICE][256];
static uint32_t st_IsochInContBuf1[NUM_OF_MAX_AUDIO_DEVICE][256];
static uint32_t st_InterruptBuf[NUM_OF_MAX_AUDIO_DEVICE][2];
static hcd_Audio_MsgBox_t st_MsgBox;

static void interruptComplete(uint8_t devAddr, uint8_t epNum, uint16_t trnsLen);

static hcd_Audio_Transfer_Driver_t* getDriver(uint8_t devAddr)
{
  hcd_Audio_Transfer_Driver_t* ret = NULL;
  for (int i = 0; i < st_NumOfAudioDevice; i++){
    if (st_Driver[i].device->devAddr == devAddr){
      ret = &st_Driver[i];
      break;
    }
  }
  return ret;
}

static hcd_Audio_Protocol_Driver_t* getProtocol(hcd_DeviceInfo_t* device)
{
  hcd_Audio_Protocol_Driver_t* ret = NULL;
  for (int i = 0; i < st_NumOfAudioDevice; i++){
    if (st_Driver[i].device == device){
      ret = st_CurrentUAC[i];
      break;
    }
  }
  return ret;
}

static hcd_Status_t enqueueMsg(hcd_Audio_Msg_t* msg)
{
  hcd_Status_t ret;
  EHCI_DisInt();
  if (st_MsgBox.deqPtr - st_MsgBox.enqPtr != 1){
    memcpy(&st_MsgBox.msg[st_MsgBox.enqPtr], msg, sizeof(hcd_Audio_Msg_t));
    st_MsgBox.enqPtr++;
    if (st_MsgBox.enqPtr == HCD_PERIODIC_MSGBOX_SIZE){
      st_MsgBox.enqPtr = 0;
    }
    ret = HCD_OK;
  } else {
    ret = HCD_FULL;
  }
  
  EHCI_EnaInt();
  NVIC_SetPendingIRQ(HcdAudio_IRQn);
  
  return ret;
}

static hcd_Status_t dequeueMsg(hcd_Audio_Msg_t* msg)
{
  hcd_Status_t ret;
  EHCI_DisInt();
  if (st_MsgBox.deqPtr != st_MsgBox.enqPtr){
    memcpy(msg, &st_MsgBox.msg[st_MsgBox.deqPtr], sizeof(hcd_Audio_Msg_t));
    st_MsgBox.deqPtr++;
    if (st_MsgBox.deqPtr == HCD_PERIODIC_MSGBOX_SIZE){
      st_MsgBox.deqPtr = 0;
    }
    ret = HCD_OK;
  } else {
    ret = HCD_FULL;
  }
  EHCI_EnaInt();
  return ret;
}

static uint16_t parseInterface(config_rawdesc_t *confRaw, hcd_DeviceInfo_t* device)
{
  uint16_t rtnReadBytes = 0;
  UAC_SubClass_t bIntfSub;
  usbDesc_Interface_t* descPtr;
  hcd_Audio_Protocol_Driver_t* protocol;
  hcd_Audio_Transfer_Driver_t* driver;
  
  descPtr = (usbDesc_Interface_t*)(&confRaw->rawDesc[confRaw->readPtr]);
  if (descPtr->bInterfaceClass != USB_CLASSCODE_AUDIO){
    return 0;
  }
  
  bIntfSub = (UAC_SubClass_t)descPtr->bInterfaceSubclass;
  protocol = getProtocol(device);
  if ((protocol == NULL) && (st_NumOfAudioDevice < NUM_OF_MAX_AUDIO_DEVICE)){
    st_CurrentUAC[st_NumOfAudioDevice] = &st_UAC10;
    st_Driver[st_NumOfAudioDevice].device = device;
    st_NumOfAudioDevice++;
    protocol = &st_UAC10;
  } else if (st_NumOfAudioDevice >= NUM_OF_MAX_AUDIO_DEVICE){
    return 0;
  }
  driver = getDriver(device->devAddr);
  
  switch(bIntfSub){
    case(UAC_CONTROL): rtnReadBytes = protocol->parseControlInterface(confRaw, &driver->ep.interrupt, device); break;
    case(UAC_STREAMING): rtnReadBytes = protocol->parseStreamingInterface(confRaw, &driver->ep.isochOut, &driver->ep.isochIn, device); break;
		default: break;
  }

  return rtnReadBytes;
}

static uint16_t parseIAD(config_rawdesc_t *confRaw, hcd_DeviceInfo_t* device)
{
  uint16_t rtnReadBytes = 0;
  usbDesc_InterfaceAssoc_t* iadPtr = (usbDesc_InterfaceAssoc_t*)(&confRaw->rawDesc[confRaw->readPtr]);
  if (iadPtr->bFunctionClass == USB_CLASSCODE_AUDIO){
    if (iadPtr->bFunctionProtocol == 0x00){
      st_CurrentUAC[st_NumOfAudioDevice] = &st_UAC10;
    } else if (iadPtr->bFunctionProtocol == 0x20){
      st_CurrentUAC[st_NumOfAudioDevice] = &st_UAC20;
    }
  }
  if (st_CurrentUAC[st_NumOfAudioDevice]){
    st_Driver[st_NumOfAudioDevice].device = device;
    st_NumOfAudioDevice++;
    rtnReadBytes = sizeof(usbDesc_InterfaceAssoc_t);
  }

  return rtnReadBytes;
}

static void initClass(hcd_DeviceInfo_t* device)
{
  hcd_Status_t status;
  hcd_Audio_Msg_t msg;
  hcd_Audio_Transfer_Driver_t* driver = getDriver(device->devAddr);
  if (driver->ep.interrupt.num){
    status = OpenInterruptEndpoint(device, driver->ep.interrupt.num, driver->interruptBuf, driver->ep.isochIn.mps, interruptComplete);
    if (status != HCD_OK){
      CloseInterruptEndpoint(device->devAddr, driver->ep.interrupt.num);
    } else {
      HcdPeriodic_StartInterruptTransfer(device->devAddr, driver->ep.interrupt.num, driver->ep.interrupt.mps);
    }
  }
  msg.msgType = HCD_AUDIO_INITIAL_REQ;
  msg.devAddr = device->devAddr;
  enqueueMsg(&msg);

}

static void terminateClass(hcd_DeviceInfo_t* device)
{

}

static void requestComplete(uint16_t transLen, uint8_t devAddr, uint32_t* ep0Buf)
{
  hcd_Audio_Msg_t msg;
  hcd_Audio_Transfer_Driver_t* driver = getDriver(devAddr);
  hcd_Audio_Protocol_Driver_t* protocol = getProtocol(driver->device);
  msg.msgType = HCD_AUDIO_CTRL_REQ_DONE;
  msg.devAddr = devAddr;
  msg.other.setup.DWORD[0] = ep0Buf[0];
  msg.other.setup.DWORD[1] = ep0Buf[1];
  protocol->requestDoneFromISR(driver->device, ep0Buf);
  enqueueMsg(&msg);
}

static void isochronousOutComplete(uint8_t devAddr, uint8_t epNum, uint16_t nextTxSize, uint32_t* bufPtr, hcd_Periodic_IsochIn_ActTxInfo_t* inTxMap)
{
  getDriver(devAddr)->isochOutCallback(bufPtr, nextTxSize);
}

static void isochronousInComplete(uint8_t devAddr, uint8_t epNum, uint16_t nextTxSize, uint32_t* bufPtr, hcd_Periodic_IsochIn_ActTxInfo_t* inTxMap)
{
  uint16_t currentTxSize = 0;
  hcd_Audio_Transfer_Driver_t* driver = getDriver(devAddr);
  uint32_t* contBuf;
  hcd_Audio_IsochIn_Raw_Buf_t* rawBuf;
  if (bufPtr == (uint32_t*)driver->isochInRaw[0]){
    contBuf = driver->isochInContinuousBuf[0];
    rawBuf = driver->isochInRaw[0];
  } else {
    contBuf = driver->isochInContinuousBuf[1];
    rawBuf = driver->isochInRaw[1];
  }
  for (int i = 0; i < HCD_PERIODIC_iTD_SINGLE_BUF; i++){
    for (int j = 0; j < MAX_iTD_TSC; j++){
      memcpy(&contBuf[currentTxSize], &rawBuf->frame[i].mFrame[j].buf[0], inTxMap->frame[i].mFrame[j]);
      currentTxSize += inTxMap->frame[i].mFrame[j];
    }
  }
  driver->isochInCallback(contBuf, currentTxSize);
}

static void interruptComplete(uint8_t devAddr, uint8_t epNum, uint16_t trnsLen)
{

}

static void audioClassTask(void)
{
  hcd_Audio_Msg_t msg, repMsg;
  hcd_Msg_t hcdMsg;
  hcd_Status_t status;
  hcd_Audio_Transfer_Driver_t* driver;
  hcd_Audio_Protocol_Driver_t* protocol;
  uint8_t bps;

  NVIC_ClearPendingIRQ(HcdAudio_IRQn);

  for(;;){
    status = dequeueMsg(&msg);
    if (status != HCD_OK){
      break;
    }
    
    driver = getDriver(msg.devAddr);
    protocol = getProtocol(driver->device);

    switch(msg.msgType){
      case(HCD_AUDIO_CTRL_REQ):{
        hcdMsg.type = HCDMSG_CTRL;
        hcdMsg.cont.ctrl.setup.DWORD[0] = msg.other.setup.DWORD[0];
        hcdMsg.cont.ctrl.setup.DWORD[1] = msg.other.setup.DWORD[1];
        if ((hcdMsg.cont.ctrl.setup.BIT.bmRequestType.dir == BMREQ_DIR_OUT) && hcdMsg.cont.ctrl.setup.BIT.wLength){
          hcdMsg.cont.ctrl.sendDataBuf = msg.bufPtr;
        }
        hcdMsg.cont.ctrl.device = driver->device;
        hcdMsg.cont.ctrl.completeCb = requestComplete;
        SendMessageToHostControllerDriver(&hcdMsg);
        break;
      }
      case(HCD_AUDIO_CTRL_REQ_DONE):{
        protocol->requestDone(driver->device, msg.other.setup.DWORD[0], msg.other.setup.DWORD[1]);
        break;
      }
      case(HCD_AUDIO_INITIAL_REQ):{
        status = protocol->sendInitialRequest(driver->device);
        if (status != HCD_OK){
          /*error*/
        }
        break;
      }
      case(HCD_AUDIO_INITIAL_REQ_DONE):{
        if (driver->ep.isochIn.num){
          repMsg.msgType = HCD_AUDIO_SET_SAMPLING_RATE;
          repMsg.devAddr = driver->device->devAddr;
          repMsg.epNum = driver->ep.isochIn.num;
          repMsg.intfNum = driver->ep.isochIn.intfNum;
          repMsg.other.audio.bitReso = DEFAULT_BIT_RESO_DIV8 * 8;
          repMsg.other.audio.fs = DEFAULT_SAMPLING_RATE;
          repMsg.other.audio.numOfChannels = DEFAULT_NUM_OF_CHANNEL;
          enqueueMsg(&repMsg);
        }
        if (driver->ep.isochOut.num){
          repMsg.msgType = HCD_AUDIO_SET_SAMPLING_RATE;
          repMsg.devAddr = driver->device->devAddr;
          repMsg.epNum = driver->ep.isochOut.num;
          repMsg.intfNum = driver->ep.isochOut.intfNum;
          repMsg.other.audio.bitReso = DEFAULT_BIT_RESO_DIV8 * 8;
          repMsg.other.audio.fs = DEFAULT_SAMPLING_RATE;
          repMsg.other.audio.numOfChannels = DEFAULT_NUM_OF_CHANNEL;
          enqueueMsg(&repMsg);
        }
        break;
      }
      case(HCD_AUDIO_SET_SAMPLING_RATE):{
        protocol->setSamplingRate(msg.other.audio.fs, msg.other.audio.bitReso, msg.intfNum, driver->device);
        break;
      }
      case(HCD_AUDIO_SAMPLING_RATE_UPDATED):{
        if (msg.epNum & 0x80){
          driver->ep.isochIn.bitReso = msg.other.audio.bitReso;
          driver->ep.isochIn.fs = msg.other.audio.fs;
          driver->ep.isochIn.mps = msg.other.audio.mps;
          driver->ep.isochIn.numOfChannels = msg.other.audio.numOfChannels;
          bps = (msg.other.audio.bitReso >> 3) * msg.other.audio.numOfChannels;
          if (driver->ep.isochIn.init == 0){
            status = OpenIsochronousEndpoint(msg.devAddr, msg.epNum, msg.other.audio.mps, driver->device->speed, 
                                            msg.other.audio.fs, bps, (uint32_t*)driver->isochInRaw[0], (uint32_t*)driver->isochInRaw[1], 
                                            isochronousInComplete);
            if (status != HCD_OK){
              CloseIsochronousEndpoint(msg.devAddr, msg.epNum);
            } else {
              driver->ep.isochIn.init = 1;
              repMsg.msgType = HCD_AUDIO_STREAMING_START;
              repMsg.devAddr = msg.devAddr;
              repMsg.epNum = msg.epNum;
              enqueueMsg(&repMsg);
            }
          }
        } else {
          driver->ep.isochOut.bitReso = msg.other.audio.bitReso;
          driver->ep.isochOut.fs = msg.other.audio.fs;
          driver->ep.isochOut.mps = msg.other.audio.mps;
          driver->ep.isochOut.numOfChannels = msg.other.audio.numOfChannels;
          bps = (msg.other.audio.bitReso >> 3) * msg.other.audio.numOfChannels;
          if (driver->ep.isochOut.init == 0){
            status = OpenIsochronousEndpoint(msg.devAddr, msg.epNum, msg.other.audio.mps, driver->device->speed, 
                                            msg.other.audio.fs, bps, driver->isochOutBuf[0], driver->isochOutBuf[1], 
                                            isochronousOutComplete);
            if (status != HCD_OK){
              CloseIsochronousEndpoint(msg.devAddr, msg.epNum);
            } else {
              driver->ep.isochOut.init = 1;
              repMsg.msgType = HCD_AUDIO_STREAMING_START;
              repMsg.devAddr = msg.devAddr;
              repMsg.epNum = msg.epNum;
              enqueueMsg(&repMsg);
            }
          }     
        }
        break;
      }
      case(HCD_AUDIO_STREAMING_START):{
        HcdPeriodic_StartIsochronousTransfer(msg.devAddr, msg.epNum);
        break;
      }
      default:
        break;
    }
  }  
}

hcd_Status_t HcdAudio_SendMsg(hcd_Audio_Msg_t* msg)
{
  return enqueueMsg(msg);
}

void HcdAudio_InitUACProtocol(uint8_t revision, hcd_Audio_Protocol_Driver_t* protocol)
{
  if (revision == 1){
    st_UAC10.parseControlInterface = protocol->parseControlInterface;
    st_UAC10.parseStreamingInterface = protocol->parseStreamingInterface;
    st_UAC10.requestDoneFromISR = protocol->requestDoneFromISR;
    st_UAC10.requestDone = protocol->requestDone;
    st_UAC10.sendInitialRequest = protocol->sendInitialRequest;
    st_UAC10.setSamplingRate = protocol->setSamplingRate;
  } else if (revision == 2){
    st_UAC20.parseControlInterface = protocol->parseControlInterface;
    st_UAC20.parseStreamingInterface = protocol->parseStreamingInterface;
    st_UAC20.requestDoneFromISR = protocol->requestDoneFromISR;
    st_UAC20.requestDone = protocol->requestDone;
    st_UAC20.sendInitialRequest = protocol->sendInitialRequest;
    st_UAC20.setSamplingRate = protocol->setSamplingRate;
  }
}

void HcdAudio_InitAudioClass(void)
{
  hcd_ClassDriver_t comDriver;
  for (int i = 0; i < NUM_OF_MAX_AUDIO_DEVICE; i++){
    st_Driver[i].interruptBuf = &st_InterruptBuf[i][0];
    st_Driver[i].isochInContinuousBuf[0] = &st_IsochInContBuf0[i][0];
    st_Driver[i].isochInContinuousBuf[1] = &st_IsochInContBuf1[i][0];
    st_Driver[i].isochInRaw[0] = &st_IsochInRawBuf0[i];
    st_Driver[i].isochInRaw[1] = &st_IsochInRawBuf1[i];
    st_Driver[i].isochOutBuf[0] = &st_IsochOutBuf0[i][0];
    st_Driver[i].isochOutBuf[1] = &st_IsochOutBuf0[i][1];
  }
  comDriver.parseInterface = parseInterface;
  comDriver.parseIAD = parseIAD;
  comDriver.initClass = initClass;
  comDriver.terinateClass = terminateClass;

  NVIC_SetPriority(HcdAudio_IRQn, 4);
  NVIC_SetVector(HcdAudio_IRQn, (uint32_t)audioClassTask);
  NVIC_EnableIRQ(HcdAudio_IRQn);

  HcdAudioMgr_RegisterAudioDriver(&comDriver);

}
