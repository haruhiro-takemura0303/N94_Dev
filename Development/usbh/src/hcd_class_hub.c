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

static hcd_Status_t enqueueMsg(hcd_Hub_Msg_t* msg)
{
  hcd_Status_t ret;
  EHCI_DisInt();
  if (st_HubMsgBox.deqPtr - st_HubMsgBox.enqPtr != 1){
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

static void requestComplete(uint16_t transLen, uint8_t devAddr, uint32_t* ep0Buf)
{

}


static void hcdHubTask(void)
{
  hcd_Hub_Msg_t msg, repMsg;
  static uint32_t ignoredMsg = 0;
  hcd_Hub_Info_t* info;
  int i;
  hcd_Msg_t hcdMsg;
  hcd_Status_t status;
  uint8_t bps;
  
  NVIC_ClearPendingIRQ(HcdHub_IRQn);

  info = getInfo(msg.devAddr);
  if (!info){
    ignoredMsg++;
    return;
  }
  
  for(;;){
    status = dequeueMsg(&msg);
    if (status != HCD_OK){
      break;
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
      default:
      break;
    }
  }    
}

