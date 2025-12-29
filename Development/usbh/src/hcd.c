/**
* @brief   MCXN947V USB Host Controller Driver
* @author  masa
* @version 1.00 
*/

#include "hcd.h"
#include "hcd_class.h"

hcd_DeviceInfo_t st_DeviceInfo[MAX_DEVICE_NUM];

static usbDesc_Device_t st_DeviceDescriptorContainer[MAX_DEVICE_NUM];
static usbDesc_Config_t st_ConfigDescriptorContainer[MAX_DEVICE_NUM];
config_rawdesc_t st_ConfigRawDesc[MAX_DEVICE_NUM];
static string_info_t st_StringInfo[MAX_DEVICE_NUM];
usb_EnumState_t st_EnumState[MAX_DEVICE_NUM];

static hcd_MsgBox_t st_HcdMsgBox;
static hcd_MsgBox_t st_CtrlPendBox;
static hcd_MsgBox_t st_GpTimerPendBox;

struct{
  uint32_t buf[256];
}st_Ep0DatBuf[MAX_DEVICE_NUM];

static struct{
  uint32_t count_us;
  void (*completeCb)(uint8_t miscVal);
  uint8_t miscVal;
}st_GpTimerTable;

static struct{
  uint8_t busyFlg;
  void (*completeCb)(uint16_t transLen, uint8_t devAddr, uint32_t* ep0Buf);
}st_CsControlTable[MAX_DEVICE_NUM];

static void (*st_HubPendStart)(uint8_t hubAddr);

static void cscCb_StableConnectionDetect(void);
static void pedCb_StartEnum(void);

static int32_t enqueueMsg(hcd_MsgBox_t* box, hcd_Msg_t* msg)
{
  int32_t ret;
  EHCI_DisInt();
  if (((box->enqPtr + 1) % HCD_MSGBOX_SIZE) != box->deqPtr){
    memcpy(&box->msg[box->enqPtr], msg, sizeof(hcd_Msg_t));
    box->enqPtr++;
    if (box->enqPtr == HCD_MSGBOX_SIZE){
      box->enqPtr = 0;
    }
    ret = 0;
  } else {
    ret = -1;
  }
  
  EHCI_EnaInt();
  NVIC_SetPendingIRQ(HCD_IRQn);
  
  return ret;
}

static int32_t dequeueMsg(hcd_MsgBox_t* box, hcd_Msg_t* msg)
{
  int32_t ret;
  EHCI_DisInt();
  if (box->deqPtr != box->enqPtr){
    memcpy(msg, &box->msg[box->deqPtr], sizeof(hcd_Msg_t));
    box->deqPtr++;
    if (box->deqPtr == HCD_MSGBOX_SIZE){
      box->deqPtr = 0;
    }
    ret = 0;
  } else {
    ret = -1;
  }
  EHCI_EnaInt();
  return ret;
}

static void initGpTimer(void)
{
  EHCI->GPTIMER0CTRL = USBHS_GPTIMER0CTL_MODE(0);
  EHCI->USBINTR |= USBHS_USBINTR_TIE0_MASK;
}

static void gpTimerIntCb_gpTimerComplete(void)
{
  hcd_Msg_t msg;
	
	EHCI->GPTIMER0CTRL &= ~USBHS_GPTIMER0CTL_RUN_MASK;
	msg.type = HCDMSG_GPTIMER;
  msg.cont.gp_timer.count_us = GPTIMER_COMPLETE;
	msg.cont.gp_timer.completeCb = NULL;
  enqueueMsg(&st_HcdMsgBox, &msg);
}

static void gpTimerCb_NegatePortReset(uint8_t miscVal)
{
  EHCI->PORTSC1 &= ~USBHS_PORTSC1_PR_MASK;
  while (EHCI->PORTSC1 & USBHS_PORTSC1_PR_MASK);
	if (EHCI->PORTSC1 & USBHS_PORTSC1_PE_MASK){
		pedCb_StartEnum();
	}
}

static void ehciResetSequence(void)
{
  hcd_Msg_t msg;
  if ((EHCI->PORTSC1 & USBHS_PORTSC1_LS_MASK) == (USBHS_PORTSC1_LS(0b10))) {
    EHCI->PORTSC1 |= USBHS_PORTSC1_PR_MASK;
    msg.type = HCDMSG_GPTIMER;
    msg.cont.gp_timer.completeCb = gpTimerCb_NegatePortReset;
    msg.cont.gp_timer.miscVal = 0;
    msg.cont.gp_timer.count_us = 50*1000; //50ms
    enqueueMsg(&st_HcdMsgBox, &msg);
  }
}

static void gpTimerCb_IsConnectStable(uint8_t miscVal)
{
  if (EHCI->PORTSC1 & USBHS_PORTSC1_CCS_MASK){
    ehciResetSequence();
  } else {
    /*Detach Msg*/
  }
  EHCI_SetCallback(PORT_CSC, cscCb_StableConnectionDetect);
}

static void cscCb_StableConnectionDetect(void)
{
  hcd_Msg_t msg;
  EHCI_SetCallback(PORT_CSC, NULL);
  msg.type = HCDMSG_GPTIMER;
  msg.cont.gp_timer.completeCb = gpTimerCb_IsConnectStable;
  msg.cont.gp_timer.miscVal = 0;
  msg.cont.gp_timer.count_us = 30*1000; //De-bounce Time:30ms
  enqueueMsg(&st_HcdMsgBox, &msg);
}

static void gpTimerCb_InitRhDevice(uint8_t miscVal)
{
  hcd_Msg_t msg;
  msg.type = HCDMSG_INIT_DEVICE;
  msg.cont.init_device.devAddr = 0;
  msg.cont.init_device.hubAddr = 0;
  msg.cont.init_device.hubPort = 0;
  if ((EHCI->PORTSC1 & USBHS_PORTSC1_PSPD_MASK) == USBHS_PORTSC1_PSPD(0b10)){
    msg.cont.init_device.psiv = DEV_SPEED_HIGH;
    enqueueMsg(&st_HcdMsgBox, &msg);
  } else if ((EHCI->PORTSC1 & USBHS_PORTSC1_PSPD_MASK) == USBHS_PORTSC1_PSPD(0b00)){
    msg.cont.init_device.psiv = DEV_SPEED_FULL;
    enqueueMsg(&st_HcdMsgBox, &msg);
  } else if ((EHCI->PORTSC1 & USBHS_PORTSC1_PSPD_MASK) == USBHS_PORTSC1_PSPD(0b01)){
    msg.cont.init_device.psiv = DEV_SPEED_LOW;
    enqueueMsg(&st_HcdMsgBox, &msg);
  } else {
    /*Detach Sequence*/
  }
}

static void pedCb_StartEnum(void)
{
  hcd_Msg_t msg;
  if (EHCI->PORTSC1 & USBHS_PORTSC1_PE_MASK){
    msg.type = HCDMSG_GPTIMER;
    msg.cont.gp_timer.completeCb = gpTimerCb_InitRhDevice;
    msg.cont.gp_timer.miscVal = 0;
    msg.cont.gp_timer.count_us = 30*1000;
    enqueueMsg(&st_HcdMsgBox, &msg);
  }
}

static uint8_t getDeviceIndex(uint8_t devAddr)
{
  uint8_t devIdx = 0xFF;
  if (devAddr == 0){
    for (int i = 0; i < MAX_DEVICE_NUM; i++){
      if ((st_DeviceInfo[i].devAddr == devAddr) && (st_DeviceInfo[i].state == HCD_USED)){
        devIdx = i;
        break;
      }
    }
  } else {
    devIdx = devAddr - 1;
  }
  
  return devIdx;
}

static uint32_t gpTimerBusy(void)
{
  return (EHCI->GPTIMER0CTRL & USBHS_GPTIMER0CTL_RUN_MASK);
}

static void setGpTimer(uint32_t count_us)
{
  EHCI->GPTIMER0LD = count_us;
  EHCI->GPTIMER0CTRL |= USBHS_GPTIMER0CTL_RST_MASK;
  __DSB();
  EHCI->GPTIMER0CTRL |= USBHS_GPTIMER0CTL_RUN_MASK;
}

static void enumerationHandler(uint8_t devAddr, uint8_t epNum, uint16_t txLen)
{
  hcd_Msg_t msg, parseMsg, clsMsg, pendMsg;
  int32_t stat;
  uint16_t ep0Mps;
  uint8_t devIdx;
  hcd_DeviceInfo_t* device;
  
  NVIC_ClearPendingIRQ(HCD_IRQn);
  
  devIdx = getDeviceIndex(devAddr);
  device = &st_DeviceInfo[devIdx];
  if (devIdx == 0xFF){
    return;
  }
  msg.type = HCDMSG_CTRL;
  msg.cont.ctrl.completeCb = NULL;
  msg.cont.ctrl.device = device;
  msg.cont.ctrl.sendDataBuf = NULL;
  switch (st_EnumState[devIdx]){
    case (ATTACHED):{
      /*Initial GET_DESCRIPTOR Device Done*/
      st_EnumState[devIdx] = GOT_DESCRIPTOR_DEV;
      uint8_t bMaxPacketSize0 = ((st_Ep0DatBuf[devIdx].buf[3] & 0xFF000000) >> 24);
      ep0Mps = bMaxPacketSize0;
      HcdAsync_SetEp0Mps(devAddr, ep0Mps);
      MakeSETUPPacket(BMREQ_DIR_OUT, BMREQ_TYPE_STANDARD, BMREQ_ATTR_DEVICE, BREQ_SET_ADDRESS, devIdx + 1, 0, 0, &msg.cont.ctrl.setup);
      enqueueMsg(&st_HcdMsgBox, &msg);
      break;
    }
    case (GOT_DESCRIPTOR_DEV):{
      /*SET_ADDRESS completed*/
			st_EnumState[devIdx] = ADDRESSED;
      device->devAddr = devIdx + 1;
      HcdAsync_SetAddress(devIdx + 1);
      if (st_HubPendStart){
        st_HubPendStart(device->hubAddr);
      }
      MakeSETUPPacket(BMREQ_DIR_IN, BMREQ_TYPE_STANDARD, BMREQ_ATTR_DEVICE, BREQ_GET_DESCRIPTOR, (DESCTYPE_DEVICE << 8 | 0), 0, 0x12, &msg.cont.ctrl.setup);
      enqueueMsg(&st_HcdMsgBox, &msg);
      break;
    }
    case (ADDRESSED):{
      /*Full GET_DESCRIPTOR Device Done*/
      st_EnumState[devIdx] = GOT_DESCRIPTOR_DEV_RE;
      //StartResetPendedPort();
      memcpy(&st_DeviceDescriptorContainer[devIdx], &st_Ep0DatBuf[devIdx].buf[2], 0x12);
      st_StringInfo[devIdx].venderStrID = st_DeviceDescriptorContainer[devIdx].desc.iManufacturer;
      st_StringInfo[devIdx].productStrID = st_DeviceDescriptorContainer[devIdx].desc.iProduct;
      MakeSETUPPacket(BMREQ_DIR_IN, BMREQ_TYPE_STANDARD, BMREQ_ATTR_DEVICE, BREQ_GET_DESCRIPTOR, (DESCTYPE_CONFIG << 8 | 0), 0, 4, &msg.cont.ctrl.setup);
      enqueueMsg(&st_HcdMsgBox, &msg);
      break;
    }
    case (GOT_DESCRIPTOR_DEV_RE):{
      /*GET_DESCRIPTOR Config for wTotalLength done*/
      st_EnumState[devIdx] = GOT_DESCRIPTOR_CFG_INI;
      uint16_t cfgLen = (st_Ep0DatBuf[devIdx].buf[2] >> 16);
      st_ConfigRawDesc[devIdx].fullLength = cfgLen;
      MakeSETUPPacket(BMREQ_DIR_IN, BMREQ_TYPE_STANDARD, BMREQ_ATTR_DEVICE, BREQ_GET_DESCRIPTOR, (DESCTYPE_CONFIG << 8 | 0), 0, cfgLen, &msg.cont.ctrl.setup);
      enqueueMsg(&st_HcdMsgBox, &msg);
      break;
    }
    case (GOT_DESCRIPTOR_CFG_INI):{
      /*GET_DESCRIPTOR Config Full done*/
      parseMsg.type = HCDMSG_PARSE_CONFIG;
      st_EnumState[devIdx] = GOT_DESCRIPTOR_CFG;
      memcpy(&st_ConfigRawDesc[devIdx].rawDesc[0], &st_Ep0DatBuf[devIdx].buf[2], st_ConfigRawDesc[devIdx].fullLength);
      parseMsg.cont.parse_config.device = device;
      enqueueMsg(&st_HcdMsgBox, &parseMsg);
      break;
    }
    case (GOT_DESCRIPTOR_CFG):{
      /*GET_DESCRIPTOR String Lang done*/
      st_StringInfo[devIdx].langID = (uint16_t)(st_Ep0DatBuf[devIdx].buf[2] >> 16);
      if (st_StringInfo[devIdx].venderStrID){
        st_EnumState[devIdx] = GOT_DESCRIPTOR_LANG;
        MakeSETUPPacket(BMREQ_DIR_IN, BMREQ_TYPE_STANDARD, BMREQ_ATTR_DEVICE, BREQ_GET_DESCRIPTOR, (DESCTYPE_STRING << 8 | st_StringInfo[devIdx].venderStrID), st_StringInfo[devIdx].langID, 0xFF, &msg.cont.ctrl.setup);
      } else{
        st_EnumState[devIdx] = GOT_DESCRIPTOR_LANG_STR_VENDOR_SKIP;
        MakeSETUPPacket(BMREQ_DIR_IN, BMREQ_TYPE_STANDARD, BMREQ_ATTR_DEVICE, BREQ_GET_DESCRIPTOR, (DESCTYPE_STRING << 8 | st_StringInfo[devIdx].productStrID), st_StringInfo[devIdx].langID, 0xFF, &msg.cont.ctrl.setup);
      }
      enqueueMsg(&st_HcdMsgBox, &msg);
      break;
    }
    case (GOT_DESCRIPTOR_LANG):{
      /*GET_DESCRIPTOR String Vendor done*/
      st_EnumState[devIdx] = GOT_DESCRIPTOR_STR_VENDOR;
      uint16_t stringLen = ((uint8_t)(st_Ep0DatBuf[devIdx].buf[2] & 0x000000FF) - 2) >> 1;
      uint8_t* rdPtr = (uint8_t*)((uint32_t)(&st_Ep0DatBuf[devIdx].buf[2]) + 2);
      for (int i = 0; i < stringLen; i++){
        st_StringInfo[devIdx].venderStr[i] = rdPtr[2 * i];
      }
      if (st_StringInfo[devIdx].productStrID){
        MakeSETUPPacket(BMREQ_DIR_IN, BMREQ_TYPE_STANDARD, BMREQ_ATTR_DEVICE, BREQ_GET_DESCRIPTOR, (DESCTYPE_STRING << 8 | st_StringInfo[devIdx].productStrID), st_StringInfo[devIdx].langID, 0xFF, &msg.cont.ctrl.setup);
      }else{
        st_EnumState[devIdx] = GET_DESCRIPTOR_STR_SKIPPED;
        uint8_t configNum = st_ConfigDescriptorContainer[devIdx].desc.bConfigurationValue;
        MakeSETUPPacket(BMREQ_DIR_OUT, BMREQ_TYPE_STANDARD, BMREQ_ATTR_DEVICE, BREQ_SET_CONFIGURATION, configNum, 0, 0, &msg.cont.ctrl.setup);
      }
      enqueueMsg(&st_HcdMsgBox, &msg);
      break;
    }
    case (GOT_DESCRIPTOR_LANG_STR_VENDOR_SKIP):
    case (GOT_DESCRIPTOR_STR_VENDOR):{
      /*GET_DESCRIPTOR String Product done*/
      st_EnumState[devIdx] = GOT_DESCRIPTOR_STR_PROD;
      uint8_t stringLen = ((uint8_t)(st_Ep0DatBuf[devIdx].buf[2] & 0x000000FF) - 2) >> 1;
      uint8_t* rdPtr = (uint8_t*)((uint32_t)(&st_Ep0DatBuf[devIdx].buf[2]) + 2);
      for (int i = 0; i < stringLen; i++){
        st_StringInfo[devIdx].productStr[i] = rdPtr[2 * i];
      }
      uint8_t configNum = st_ConfigDescriptorContainer[devIdx].desc.bConfigurationValue;
      MakeSETUPPacket(BMREQ_DIR_OUT, BMREQ_TYPE_STANDARD, BMREQ_ATTR_DEVICE, BREQ_SET_CONFIGURATION, configNum, 0, 0, &msg.cont.ctrl.setup);
      enqueueMsg(&st_HcdMsgBox, &msg);
      break;
    }
    case (GOT_DESCRIPTOR_STR_PROD):
    case (GET_DESCRIPTOR_STR_SKIPPED):{
      /*SET_CONFIGURATION done*/
      st_EnumState[devIdx] = CONFIGURED;
      clsMsg.type = HCDMSG_INIT_CLASS;
      clsMsg.cont.init_class.device = &st_DeviceInfo[devIdx];
      enqueueMsg(&st_HcdMsgBox, &clsMsg);
      break;
    }
    case (CONFIGURED):{
      if (st_CsControlTable[devIdx].busyFlg){
        st_CsControlTable[devIdx].busyFlg = 0;
        if (st_CsControlTable[devIdx].completeCb){
          st_CsControlTable[devIdx].completeCb(txLen, devAddr, &st_Ep0DatBuf[devIdx].buf[0]);
        }
        stat = dequeueMsg(&st_CtrlPendBox, &pendMsg);
        if (stat == 0){
          enqueueMsg(&st_HcdMsgBox, &pendMsg);
        }
      }
      break;
    }
    default:
    break;
  }
}

static void hcdMainTask(void)
{
  hcd_Msg_t msg, repMsg, tmrPendMsg;
  int32_t status, parseStat, pendStat, devStat;
  uint8_t devIdx;
  uint32_t u32Len;
  NVIC_ClearPendingIRQ(HCD_IRQn);
  for(;;){
    status = dequeueMsg(&st_HcdMsgBox, &msg);
    if (status != 0){
      break;
    }
    switch(msg.type){
      case(HCDMSG_CTRL):{
        devIdx = getDeviceIndex(msg.cont.ctrl.device->devAddr);
        if ((st_CsControlTable[devIdx].busyFlg == 0) || (st_EnumState[devIdx] < CONFIGURED)){
          st_Ep0DatBuf[devIdx].buf[0] = msg.cont.ctrl.setup.DWORD[0];
          st_Ep0DatBuf[devIdx].buf[1] = msg.cont.ctrl.setup.DWORD[1];
          if ((msg.cont.ctrl.setup.BIT.bmRequestType.dir == BMREQ_DIR_OUT) && (msg.cont.ctrl.sendDataBuf) && (msg.cont.ctrl.setup.BIT.wLength)){
            u32Len = (msg.cont.ctrl.setup.BIT.wLength + 3) >> 2;
            for (int i = 0; i < u32Len; i++){
              st_Ep0DatBuf[devIdx].buf[2 + i] = msg.cont.ctrl.sendDataBuf[i];
            }
          }
          if ((st_EnumState[devIdx] == CONFIGURED) && (msg.cont.ctrl.completeCb)){
            st_CsControlTable[devIdx].completeCb = msg.cont.ctrl.completeCb;
						st_CsControlTable[devIdx].busyFlg = 1;
          }
          HcdAsync_StartTransfer(msg.cont.ctrl.device->devAddr, 0, 0);
        } else {
          enqueueMsg(&st_CtrlPendBox, &msg);
        }
        break;
      }
      case(HCDMSG_PARSE_CONFIG):{
        devIdx = getDeviceIndex(msg.cont.parse_config.device->devAddr);
        parseStat = ParseConfigurationDescriptor(msg.cont.parse_config.device, &st_ConfigRawDesc[devIdx], &st_ConfigDescriptorContainer[devIdx]);
        if (parseStat == 0){
          repMsg.type = HCDMSG_CTRL;
          repMsg.cont.ctrl.completeCb = NULL;
          repMsg.cont.ctrl.sendDataBuf = NULL;
          repMsg.cont.ctrl.device = msg.cont.parse_config.device;
          if ((msg.cont.ctrl.device->deviceDesc->desc.iManufacturer != 0) || (msg.cont.ctrl.device->deviceDesc->desc.iProduct != 0)){
            MakeSETUPPacket(BMREQ_DIR_IN, BMREQ_TYPE_STANDARD, BMREQ_ATTR_DEVICE, BREQ_GET_DESCRIPTOR, (DESCTYPE_STRING << 8 | 0), 0, 4, &repMsg.cont.ctrl.setup);
          } else {
            uint8_t configNum = st_ConfigDescriptorContainer[devIdx].desc.bConfigurationValue;
            MakeSETUPPacket(BMREQ_DIR_OUT, BMREQ_TYPE_STANDARD, BMREQ_ATTR_DEVICE, BREQ_SET_CONFIGURATION, configNum, 0, 0, &repMsg.cont.ctrl.setup);
          }
          enqueueMsg(&st_HcdMsgBox, &repMsg);
        }
        break;
      }
      case(HCDMSG_GPTIMER):{
        if (msg.cont.gp_timer.count_us == GPTIMER_COMPLETE){
          if (st_GpTimerTable.completeCb){
            st_GpTimerTable.completeCb(st_GpTimerTable.miscVal);
          }
          pendStat = dequeueMsg(&st_GpTimerPendBox, &tmrPendMsg);
          if (pendStat == 0){
            st_GpTimerTable.completeCb = tmrPendMsg.cont.gp_timer.completeCb;
            st_GpTimerTable.count_us = tmrPendMsg.cont.gp_timer.count_us;
            st_GpTimerTable.miscVal = tmrPendMsg.cont.gp_timer.miscVal;
            setGpTimer(tmrPendMsg.cont.gp_timer.count_us);
          }
        } else if (gpTimerBusy() == 0){
          st_GpTimerTable.completeCb = msg.cont.gp_timer.completeCb;
          st_GpTimerTable.count_us = msg.cont.gp_timer.count_us;
          st_GpTimerTable.miscVal = msg.cont.gp_timer.miscVal;
          setGpTimer(msg.cont.gp_timer.count_us);          
        } else {
          enqueueMsg(&st_GpTimerPendBox, &msg);
        }
        break;
      }
      case(HCDMSG_INIT_DEVICE):{
        devIdx = 0xFF;
        for (int i = 0; i < MAX_DEVICE_NUM; i++){
          if (st_DeviceInfo[i].state == HCD_UNUSED){
            st_DeviceInfo[i].state = HCD_USED;
            st_DeviceInfo[i].devAddr = 0;
            st_DeviceInfo[i].hubAddr = msg.cont.init_device.hubAddr;
            st_DeviceInfo[i].hubPort = msg.cont.init_device.hubPort;
            st_DeviceInfo[i].speed = (usb_psiv_t)msg.cont.init_device.psiv;
            st_DeviceInfo[i].deviceDesc = &st_DeviceDescriptorContainer[i];
            devIdx = i;
            st_EnumState[i] = ATTACHED;
						break;
          }
        }
        if (devIdx != 0xFF){
          devStat = OpenAsyncEndpoint(&st_DeviceInfo[devIdx], 0, &st_Ep0DatBuf[devIdx].buf[0], 8, enumerationHandler);
          if (devStat != 0){
            CloseAsyncEndpoint(0, 0);
          } else {
            repMsg.type = HCDMSG_CTRL;
            repMsg.cont.ctrl.completeCb = NULL;
            repMsg.cont.ctrl.device = &st_DeviceInfo[devIdx];
            repMsg.cont.ctrl.sendDataBuf = NULL;
            MakeSETUPPacket(BMREQ_DIR_IN, BMREQ_TYPE_STANDARD, BMREQ_ATTR_DEVICE, BREQ_GET_DESCRIPTOR, (DESCTYPE_DEVICE << 8 | 0), 0, 8, &repMsg.cont.ctrl.setup);
            enqueueMsg(&st_HcdMsgBox, &repMsg);
          }
        }
        break;
      }
      case(HCDMSG_INIT_CLASS):{
        StartClassDriver(msg.cont.init_class.device);
        break;
      }
      default:
      break;
    }
  }  
}



void InitEHCI(void)
{
  HcdClass_InitClassDrivers();

  EHCI_SetCallback(PORT_CSC, cscCb_StableConnectionDetect);
  //EHCI_SetCallback(PORT_PED, pedCb_StartEnum);
	EHCI_SetCallback(GPTIMER0, gpTimerIntCb_gpTimerComplete);
	
	UsbPhy_HighSpeedInit();
	
  initGpTimer();
  //NVIC_SetPriority(EHCI_Detach_VDIn, 2);
  //NVIC_EnableIRQ(EHCI_Detach_VDIn);
  NVIC_SetVector(HCD_IRQn, (uint32_t)hcdMainTask);
  NVIC_SetPriority(HCD_IRQn, 4);
  NVIC_EnableIRQ(HCD_IRQn);
	EHCI_SysInit();
  InitAsyncSchedule();
  InitPeriodicSchedule();
	  /*Port Start*/
  EHCI->PORTSC1 |= USBHS_PORTSC1_PP_MASK;

}

void MakeSETUPPacket(uint8_t dir, uint8_t typ, uint8_t attr, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, usb_SetupPacket_t* setup)
{
  uint8_t bmRequestType = (dir << 7 | typ << 5 | attr);
  setup->DWORD[0] = ((uint32_t)wValue << 16 | (uint32_t)bRequest << 8 | bmRequestType);
  setup->DWORD[1] = ((uint32_t)wLength << 16 | (uint32_t)wIndex);
}

int32_t SendMessageToHostControllerDriver(hcd_Msg_t* msg)
{
  return enqueueMsg(&st_HcdMsgBox, msg);
}

void Hcd_SetHubPendStartFunc(void func(uint8_t))
{
  st_HubPendStart = func;
}
