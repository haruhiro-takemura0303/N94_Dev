/**
* @brief   MCXN947V USB Host Controller Hub Class Driver 
* @author  masa
* @version 1.00 
*/

#include "hcd_class_hub.h"

hcd_Hub_Info_t st_HubInfo[NUM_OF_MAX_HUB_DEVICE];
uint8_t st_NrHub = 0;
hcd_Hub_Request_Buf_t st_HubReqBuf[HCD_HUB_REQ_BOX_SIZE];
hcd_Hub_MsgBox_t st_HubMsgBox;

static hcd_Status_t createRequest(usb_SetupPacket_t* setup, hcd_DeviceInfo_t* device);

static hcd_Status_t enqueueMsg(hcd_Hub_Msg_t* msg)
{
  hcd_Status_t ret;
  EHCI_DisInt();
  if (((st_HubMsgBox.enqPtr + 1) % HCD_HUB_MSGBOX_SIZE) != st_HubMsgBox.deqPtr){
    memcpy(&st_HubMsgBox.msg[st_HubMsgBox.enqPtr], msg, sizeof(hcd_Hub_Msg_t));
    st_HubMsgBox.enqPtr++;
    if (st_HubMsgBox.enqPtr == HCD_HUB_MSGBOX_SIZE){
      st_HubMsgBox.enqPtr = 0;
    }
    ret = HCD_OK;
  } else {
    ret = HCD_FULL;
  }
  
  EHCI_EnaInt();
  NVIC_SetPendingIRQ(HcdHub_IRQn);
  
  return ret;
}

static hcd_Status_t dequeueMsg(hcd_Hub_Msg_t* msg)
{
  hcd_Status_t ret;
  EHCI_DisInt();
  if (st_HubMsgBox.deqPtr != st_HubMsgBox.enqPtr){
    memcpy(msg, &st_HubMsgBox.msg[st_HubMsgBox.deqPtr], sizeof(hcd_Hub_Msg_t));
    st_HubMsgBox.deqPtr++;
    if (st_HubMsgBox.deqPtr == HCD_HUB_MSGBOX_SIZE){
      st_HubMsgBox.deqPtr = 0;
    }
    ret = HCD_OK;
  } else {
    ret = HCD_FULL;
  }
  EHCI_EnaInt();
  return ret;
}

static hcd_Hub_Info_t* getInfo(uint8_t devAddr)
{
  hcd_Hub_Info_t* ret = NULL;
  for (int i = 0; i < st_NrHub; i++){
    if (st_HubInfo[i].device && (st_HubInfo[i].device->devAddr == devAddr)){
      ret = &st_HubInfo[i];
      break;
    }
  }
  return ret;    
}

static uint16_t parseInterface(config_rawdesc_t* confRaw, hcd_DeviceInfo_t* device)
{
  hcd_Hub_Info_t* info;
  uint16_t descInc = 0;
  volatile uint16_t rdIdx = confRaw->readPtr;
  usbDesc_Interface_t* intfPtr = (usbDesc_Interface_t*)&confRaw->rawDesc[rdIdx];
  hcd_Hub_AltSet_t* curAlt;
  uint8_t nextDescType = intfPtr->bInterfaceClass;
  
  info = getInfo(device->devAddr);
  if (info == NULL){
    if (st_NrHub < NUM_OF_MAX_HUB_DEVICE){
      for (int i = 0; i < NUM_OF_MAX_HUB_DEVICE; i++){
        if (!st_HubInfo[i].device){
          info = &st_HubInfo[i];
          st_HubInfo[i].device = device;
          st_NrHub++;
          break;
        }
      }
    } else {
      return 0;
    }
  }
  if (intfPtr->bAlternateSetting >= HCD_HUB_MAX_NUM_OF_ALTSET){
    /*AltSet No.x(x >= HCD_HUB_MAX_NUM_OF_ALTSET) is ignored, but not error, go to next interface*/
    rdIdx += intfPtr->bLength;
    descInc += intfPtr->bLength;
    while ((confRaw->rawDesc[rdIdx + 1] != DESCTYPE_INTERFACE) && (rdIdx < confRaw->fullLength)){
      uint8_t bLen = confRaw->rawDesc[rdIdx];
      rdIdx += bLen;
      descInc += bLen;
    }
    return descInc;    
  }
  curAlt = &info->altSet[intfPtr->bAlternateSetting];
  curAlt->intfDesc = intfPtr;
  rdIdx += intfPtr->bLength;
  descInc += intfPtr->bLength;
  nextDescType = confRaw->rawDesc[rdIdx + 1];	
  while ((nextDescType != DESCTYPE_INTERFACE) && (rdIdx < confRaw->fullLength)){
    switch(nextDescType){
      case(DESCTYPE_ENDPOINT):{
        curAlt->epDesc = (usbDesc_Endpoint2_t*)&confRaw->rawDesc[rdIdx];
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

static void initClass(hcd_DeviceInfo_t* device)
{
  hcd_Hub_Info_t* info = getInfo(device->devAddr);
  uint8_t altSetNum = 0;
  usb_SetupPacket_t setup;
	
    info->curAltSet = &info->altSet[0];
    MakeSETUPPacket(BMREQ_DIR_IN, BMREQ_TYPE_CLASS, BMREQ_ATTR_DEVICE, BREQ_GET_DESCRIPTOR, (DESCTYPE_HUB << 8 | 0), 0, sizeof(usbDesc_Hub_t), &setup);
    createRequest(&setup, device);
  
  /*for (int i = 1; i < HCD_HUB_MAX_NUM_OF_ALTSET; i++){
    if (info->altSet[i].intfDesc && info->altSet[i].intfDesc->bInterfaceProtocol == 0x02){
      altSetNum = i;
      break;
    }
  }
  if (altSetNum != 0){
    MakeSETUPPacket(BMREQ_DIR_OUT, BMREQ_TYPE_STANDARD, BMREQ_ATTR_INTERFACE, BREQ_SET_INTERFACE, altSetNum, info->altSet[0].intfDesc->bInterfaceNumber, 0, &setup);
    createRequest(&setup, device);
  } else {
    
  }*/
}

void terminateClass(hcd_DeviceInfo_t* device)
{
  
}

static void requestComplete(uint16_t transLen, uint8_t devAddr, uint32_t* ep0Buf)
{
  usb_SetupPacket_t setup;
  hcd_Hub_Msg_t msg;
  hcd_Hub_Info_t* info;
  uint8_t bLength;
  
  msg.msgType = HCD_HUB_CTRL_REQ_DONE;
  info = getInfo(devAddr);
  
  setup.DWORD[0] = ep0Buf[0];
  setup.DWORD[1] = ep0Buf[1];
  if (setup.BIT.bRequest == BREQ_GET_STATUS){
    for (int i = 0; i < HCD_HUB_REQ_BOX_SIZE; i++){
      if ((st_HubReqBuf[i].setup.DWORD[0] == setup.DWORD[0]) && (st_HubReqBuf[i].setup.DWORD[1] == setup.DWORD[1])){
        st_HubReqBuf[i].data.DWORD = ep0Buf[2];
        break;
      }
    }
  } else if (setup.BIT.bRequest == BREQ_GET_DESCRIPTOR){
    bLength = (ep0Buf[2] & 0x000000FF);
    memcpy(&info->hubDescBuf, &ep0Buf[2], bLength);
  }
  
  msg.devAddr = devAddr;
  msg.setup.DWORD[0] = setup.DWORD[0];
  msg.setup.DWORD[1] = setup.DWORD[1];
  enqueueMsg(&msg);
}

static void timerDelayComplete(uint8_t miscVal)
{
  hcd_Hub_TimerCompVal_t hubCompVal;
  hcd_Hub_Msg_t msg;
  uint8_t port, devAddr;
  
  hubCompVal.BYTE = miscVal;
  port = (uint8_t)hubCompVal.BIT.hubPort;
  devAddr = (uint8_t)hubCompVal.BIT.hubAddr;
  
  msg.msgType = HCD_HUB_TIMER_REQ_DONE;
  msg.devAddr = devAddr;
  msg.portNum = port;
  enqueueMsg(&msg);
}

static void interruptComplete(uint8_t devAddr, uint8_t epNum, uint16_t transLen)
{
  hcd_Hub_Msg_t msg;
  msg.msgType = HCD_HUB_INTR_RECEIVED;
  msg.devAddr = devAddr;
  enqueueMsg(&msg);
}

static hcd_Status_t createRequest(usb_SetupPacket_t* setup, hcd_DeviceInfo_t* device)
{
  hcd_Hub_Msg_t msg;
  int i;
  msg.devAddr = device->devAddr;
  msg.msgType = HCD_HUB_CTRL_REQ;
  msg.setup.DWORD[0] = setup->DWORD[0];
  msg.setup.DWORD[1] = setup->DWORD[1];
  
  if (setup->BIT.bmRequestType.dir == BMREQ_DIR_IN){
    for (i = 0; i < HCD_HUB_REQ_BOX_SIZE; i++){
      if ((st_HubReqBuf[i].setup.DWORD[0] == 0) && (st_HubReqBuf[i].setup.DWORD[1] == 0)){
        st_HubReqBuf[i].setup.DWORD[0] = setup->DWORD[0];
        st_HubReqBuf[i].setup.DWORD[1] = setup->DWORD[1];
        break;
      }
    }
    if (i == HCD_HUB_REQ_BOX_SIZE){
      return HCD_FULL;
    }
  }
  return enqueueMsg(&msg);
}

static void portStatManagement(uint8_t portNum, usb_Hub_PortStatus_t newStat, hcd_DeviceInfo_t* device)
{
  hcd_Hub_Msg_t msg;
  usb_SetupPacket_t setup;
  hcd_Hub_Info_t* info = getInfo(device->devAddr);
  hcd_Hub_Port_Info_t* portInfo = &info->portInfo[portNum - 1];
  uint8_t needToRestartIntr = 0;
  
  portInfo->portStatus.DWORD = newStat.DWORD;
  
  if (newStat.BIT.portStatus.portPower && !portInfo->portMgr.pwrConfirmed){
    portInfo->portMgr.pwrConfirmed = 1;
  }
  
  if (portInfo->portMgr.connConfirming || portInfo->portMgr.disconnConfirming){
    if (portInfo->portMgr.connConfirming && newStat.BIT.portStatus.curConnStat){
      info->nrPendedPorts++;
      portInfo->pendPrio = info->nrPendedPorts;
      portInfo->portMgr.connConfirming = 0;
      portInfo->portMgr.connConfirmed = 1;
      if (portInfo->pendPrio == 1){
        MakeSETUPPacket(BMREQ_DIR_OUT, BMREQ_TYPE_CLASS, BMREQ_ATTR_OTHER, BREQ_SET_FEATURE, PORT_RESET, portInfo->portNum, 0, &setup);
        createRequest(&setup, device);
      }
    } else if (portInfo->portMgr.disconnConfirming && !newStat.BIT.portStatus.curConnStat){
      portInfo->portMgr.disconnConfirming = 0;
      /*disconnection*/
    } else { //Connect status changed while confimation delay
      if (newStat.BIT.portStatus.curConnStat){
        portInfo->portMgr.disconnConfirming = 0;
        portInfo->portMgr.connConfirming = 1;
      } else {
        portInfo->portMgr.disconnConfirming = 1;
        portInfo->portMgr.connConfirming = 0;        
      }
      msg.msgType = HCD_HUB_TIMER_REQ;
      msg.devAddr = device->devAddr;
      msg.portNum = portNum;
      msg.timerCb = timerDelayComplete;
      msg.count_ms = 10;
      enqueueMsg(&msg);
    }
  }
  
  if (newStat.BIT.portChange.connStatChg){
    MakeSETUPPacket(BMREQ_DIR_OUT, BMREQ_TYPE_CLASS, BMREQ_ATTR_OTHER, BREQ_CLEAR_FEATURE, C_PORT_CONNECTION, portInfo->portNum, 0, &setup);
    createRequest(&setup, device);
    if (!portInfo->portMgr.connConfirming && !portInfo->portMgr.disconnConfirming){
      if (newStat.BIT.portStatus.curConnStat){
        portInfo->portMgr.connConfirming = 1;
      } else {
        portInfo->portMgr.disconnConfirming = 1;
      }
      msg.msgType = HCD_HUB_TIMER_REQ;
      msg.devAddr = device->devAddr;
      msg.portNum = portNum;
      msg.timerCb = timerDelayComplete;
      msg.count_ms = 10;
      enqueueMsg(&msg);
    }
    needToRestartIntr = 1;
  }
  if (newStat.BIT.portChange.rstChg){
    MakeSETUPPacket(BMREQ_DIR_OUT, BMREQ_TYPE_CLASS, BMREQ_ATTR_OTHER, BREQ_CLEAR_FEATURE, C_PORT_RESET, portInfo->portNum, 0, &setup);
    createRequest(&setup, device);
    if (newStat.BIT.portStatus.portEnDis){
      msg.msgType = HCD_HUB_INIT_DEVICE;
      msg.devAddr = device->devAddr;
      msg.portNum = portNum;
      if (newStat.BIT.portStatus.lsAttached){
        msg.psiv = DEV_SPEED_LOW;
      } else if (newStat.BIT.portStatus.hsAttached){
        msg.psiv = DEV_SPEED_HIGH;
      } else {
        msg.psiv = DEV_SPEED_FULL;
      }
      enqueueMsg(&msg);
    }
    needToRestartIntr = 1;
  }
  if (newStat.BIT.portChange.ovCurrChg){
    MakeSETUPPacket(BMREQ_DIR_OUT, BMREQ_TYPE_CLASS, BMREQ_ATTR_OTHER, BREQ_CLEAR_FEATURE, C_PORT_OVER_CURRENT, portInfo->portNum, 0, &setup);
    createRequest(&setup, device);
    needToRestartIntr = 1;
  }
  if (newStat.BIT.portChange.portEnDisChg){
    MakeSETUPPacket(BMREQ_DIR_OUT, BMREQ_TYPE_CLASS, BMREQ_ATTR_OTHER, BREQ_CLEAR_FEATURE, C_PORT_ENABLE, portInfo->portNum, 0, &setup);
    createRequest(&setup, device);
    needToRestartIntr = 1;    
  }
  if (newStat.BIT.portChange.suspChg){
    MakeSETUPPacket(BMREQ_DIR_OUT, BMREQ_TYPE_CLASS, BMREQ_ATTR_OTHER, BREQ_CLEAR_FEATURE, C_PORT_SUSPEND, portInfo->portNum, 0, &setup);
    createRequest(&setup, device);
    needToRestartIntr = 1;    
  }
  
  if (needToRestartIntr){
    msg.msgType = HCD_HUB_INTR_RESTART;
    msg.devAddr = device->devAddr;
    enqueueMsg(&msg);
  }
}

static void requestDone(hcd_DeviceInfo_t* device, uint32_t setup0, uint32_t setup1)
{
  hcd_Hub_Info_t* info;
  usb_SetupPacket_t setup, nextSetup;
  uint8_t portNum, featSel, nrPorts;
  hcd_Hub_Port_Info_t* portInfo;
  usb_Hub_PortStatus_t newStatus;
  hcd_Hub_Msg_t msg;
  
  info = getInfo(device->devAddr);
  if (!info){
    return;
  }
  
  setup.DWORD[0] = setup0;
  setup.DWORD[1] = setup1;
  
  if (setup.BIT.bmRequestType.dir == BMREQ_DIR_IN){
    for (int i = 0; i < HCD_HUB_REQ_BOX_SIZE; i++){
      if ((setup0 == st_HubReqBuf[i].setup.DWORD[0]) && (setup1 == st_HubReqBuf[i].setup.DWORD[1])){
        st_HubReqBuf[i].setup.DWORD[0] = 0;
        st_HubReqBuf[i].setup.DWORD[1] = 0;
				newStatus.DWORD = st_HubReqBuf[i].data.DWORD;
        break;
      }
    }
  }
  switch(setup.BIT.bRequest){
    case(BREQ_GET_STATUS):{
      if (setup.BIT.bmRequestType.attr ==  BMREQ_ATTR_OTHER){
        portNum = (uint8_t)(setup.BIT.wIndex & 0x00FF);
        portStatManagement(portNum, newStatus, info->device);
      }
      break;
    }
    case(BREQ_CLEAR_FEATURE):{
      if (setup.BIT.bmRequestType.attr ==  BMREQ_ATTR_OTHER){
        portNum = (uint8_t)(setup.BIT.wIndex & 0x00FF);
        featSel = (uint8_t)(setup.BIT.wValue);
        info->portInfo[portNum - 1].portStatus.DWORD &= ~(1 << featSel);
      }
      break;
    }
    case(BREQ_SET_FEATURE):{
      if (setup.BIT.bmRequestType.attr ==  BMREQ_ATTR_OTHER){
        portNum = (uint8_t)(setup.BIT.wIndex & 0x00FF);
        featSel = (uint8_t)(setup.BIT.wValue);
        info->portInfo[portNum - 1].portStatus.DWORD |= (1 << featSel);
        if (featSel == PORT_POWER){
          msg.msgType = HCD_HUB_TIMER_REQ;
          msg.count_ms = 2 * info->hubDescBuf.bPwrOn2PwrGood;
          msg.timerCb = timerDelayComplete;
          msg.devAddr = device->devAddr;
          msg.portNum = portNum;
          enqueueMsg(&msg);
        }
      }
      break;      
    }
    case(BREQ_GET_DESCRIPTOR):{
      nrPorts = info->hubDescBuf.bNbrPorts;
      if (nrPorts > HCD_HUB_MAX_NUM_OF_PORT){
        nrPorts = HCD_HUB_MAX_NUM_OF_PORT;
      }
      for (int i = 0; i < nrPorts; i++){
        info->portInfo[i].portNum = i + 1;
        MakeSETUPPacket(BMREQ_DIR_OUT, BMREQ_TYPE_CLASS, BMREQ_ATTR_OTHER, BREQ_SET_FEATURE, PORT_POWER, i + 1, 0, &nextSetup);
        createRequest(&nextSetup, info->device);
      }
      msg.msgType = HCD_HUB_INTR_START;
      msg.devAddr = device->devAddr;
      enqueueMsg(&msg);
      break;
    }
    case(BREQ_SET_INTERFACE):{
      info->curAltSet = &info->altSet[setup.BIT.wValue];
      MakeSETUPPacket(BMREQ_DIR_IN, BMREQ_TYPE_CLASS, BMREQ_ATTR_DEVICE, BREQ_GET_DESCRIPTOR, (DESCTYPE_HUB << 8 | 0), 0, sizeof(usbDesc_Hub_t), &nextSetup);
      createRequest(&nextSetup, info->device);
      break;
    }
    default:{
      break;
    }
  }
}

static void pendedPortRestart(hcd_DeviceInfo_t* device)
{
  uint8_t portNum = 0;
  hcd_Hub_Info_t* info = getInfo(device->devAddr);
  usb_SetupPacket_t setup;
  
  if (info->nrPendedPorts > 0){
    info->nrPendedPorts--;
  }
  for (int i = 0; i < HCD_HUB_MAX_NUM_OF_PORT; i++){
    if (info->portInfo[i].pendPrio > 0){
      info->portInfo[i].pendPrio--;
    }
    if (info->portInfo[i].pendPrio == 1){
      portNum = info->portInfo[i].portNum;
    }
  }
  if (portNum > 0){
    MakeSETUPPacket(BMREQ_DIR_OUT, BMREQ_TYPE_CLASS, BMREQ_ATTR_OTHER, BREQ_SET_FEATURE, PORT_RESET, portNum, 0, &setup);
    createRequest(&setup, device);    
  }
}


static void hubClassTask(void)
{
  int i;
  uint8_t portNum;
  uint16_t mps;
  uint32_t intrData;
  hcd_Hub_Msg_t msg;
  hcd_Hub_Info_t* info;
  hcd_Msg_t hcdMsg;
  hcd_Status_t status, epStat;
  usbDesc_Endpoint2_t* ep;
  usb_SetupPacket_t setup;
  hcd_Hub_TimerCompVal_t compVal;
  
  static uint32_t ignoredMsg = 0;
  
  NVIC_ClearPendingIRQ(HcdHub_IRQn);
  
  for(;;){
    status = dequeueMsg(&msg);
    if (status != HCD_OK){
      break;
    }
    
    info = getInfo(msg.devAddr);
    if (!info){
      ignoredMsg++;
      return;
    }
    
    switch(msg.msgType){
      case(HCD_HUB_CTRL_REQ):{
        if (msg.setup.BIT.bmRequestType.dir == BMREQ_DIR_IN){
          for (i = 0; i < HCD_HUB_REQ_BOX_SIZE; i++){
            if ((st_HubReqBuf[i].setup.DWORD[0] == 0) && (st_HubReqBuf[i].setup.DWORD[1] == 0)){
              st_HubReqBuf[i].setup.DWORD[0] = msg.setup.DWORD[0];
              st_HubReqBuf[i].setup.DWORD[1] = msg.setup.DWORD[1];
              break;
            }
          }
          if (i == HCD_HUB_REQ_BOX_SIZE){
            ignoredMsg++;
            break;
          }
        }
        hcdMsg.type = HCDMSG_CTRL;
        hcdMsg.cont.ctrl.completeCb = requestComplete;
        hcdMsg.cont.ctrl.device = info->device;
        hcdMsg.cont.ctrl.setup.DWORD[0] = msg.setup.DWORD[0];
        hcdMsg.cont.ctrl.setup.DWORD[1] = msg.setup.DWORD[1];
        hcdMsg.cont.ctrl.sendDataBuf = NULL;
        SendMessageToHostControllerDriver(&hcdMsg);
        break;
      }
      case(HCD_HUB_CTRL_REQ_DONE):{
        requestDone(info->device, msg.setup.DWORD[0], msg.setup.DWORD[1]);
        break;
      }
      case(HCD_HUB_INTR_START):{
        if (info->curAltSet){
          ep = info->curAltSet->epDesc;
          mps = U16FromU8x2(ep->wMaxPacketSize_msB, ep->wMaxPacketSize_lsB);
          epStat = OpenInterruptEndpoint(info->device, ep->bEndpointAddress, &info->intrBuf, mps, interruptComplete);
          if (epStat == HCD_OK){
            HcdPeriodic_StartInterruptTransfer(info->device->devAddr, ep->bEndpointAddress, mps);
          } else {
            CloseInterruptEndpoint(info->device->devAddr, ep->bEndpointAddress);
          }
        }
        break;
      }
      case(HCD_HUB_INTR_RESTART):{
        ep = info->curAltSet->epDesc;
        mps = U16FromU8x2(ep->wMaxPacketSize_msB, ep->wMaxPacketSize_lsB);
        HcdPeriodic_StartInterruptTransfer(info->device->devAddr, ep->bEndpointAddress, mps);
        break;
      }
      case(HCD_HUB_INTR_RECEIVED):{
        intrData = (info->intrBuf & 0x000000FE);
        while (intrData){
          portNum = 31 - __CLZ(intrData);
          if (portNum <= HCD_HUB_MAX_NUM_OF_PORT){
            MakeSETUPPacket(BMREQ_DIR_IN, BMREQ_TYPE_CLASS, BMREQ_ATTR_OTHER, BREQ_GET_STATUS, 0, portNum, 4, &setup);
            createRequest(&setup, info->device);
          }
          intrData &= ~(1 << portNum);
        }
        break;
      }
      case(HCD_HUB_PORT_PEND_RELEASE):{
        pendedPortRestart(info->device);
        break;
      }
      case(HCD_HUB_TIMER_REQ):{
        compVal.BIT.hubAddr = (msg.devAddr & 0x0F);
        compVal.BIT.hubPort = (msg.portNum & 0x0F);
        hcdMsg.type = HCDMSG_GPTIMER;
        hcdMsg.cont.gp_timer.completeCb = msg.timerCb;
        hcdMsg.cont.gp_timer.count_us = msg.count_ms * 1000;
        hcdMsg.cont.gp_timer.miscVal = compVal.BYTE;
        SendMessageToHostControllerDriver(&hcdMsg);
        break;
      }
      case(HCD_HUB_TIMER_REQ_DONE):{
        MakeSETUPPacket(BMREQ_DIR_IN, BMREQ_TYPE_CLASS, BMREQ_ATTR_OTHER, BREQ_GET_STATUS, 0, msg.portNum, 4, &setup);
        createRequest(&setup, info->device);        
        break;
      }
      case(HCD_HUB_INIT_DEVICE):{
        hcdMsg.type = HCDMSG_INIT_DEVICE;
        hcdMsg.cont.init_device.devAddr = 0;
        hcdMsg.cont.init_device.hubAddr = msg.devAddr;
        hcdMsg.cont.init_device.hubPort = msg.portNum;
        hcdMsg.cont.init_device.psiv = msg.psiv;
        SendMessageToHostControllerDriver(&hcdMsg);
        break;
      }
      default:
      break;
    }
  }    
}

static void pendedPortReleaseNotify(uint8_t hubAddr)
{
  hcd_Hub_Msg_t msg;
  msg.msgType = HCD_HUB_PORT_PEND_RELEASE;
  msg.devAddr = hubAddr;
  enqueueMsg(&msg);
}

void HcdHub_InitDriver(void)
{
  hcd_ClassDriver_t drv;
  drv.parseInterface = parseInterface;
  drv.parseIAD = parseIAD;
  drv.initClass = initClass;
  drv.terinateClass = terminateClass;
  
  Hcd_SetHubPendStartFunc(pendedPortReleaseNotify);
  
  NVIC_SetPriority(HcdHub_IRQn, 4);
  NVIC_SetVector(HcdHub_IRQn, (uint32_t)hubClassTask);
  NVIC_EnableIRQ(HcdHub_IRQn);
  
  RegisterClassDriver(&drv, USB_CLASSCODE_HUB);  
}
