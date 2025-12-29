/**
* @brief   MCXN947V USB Host Controller Simple Asynchronous Transfer Driver
* @author  masa
* @version 1.00 
*/

#include "hcd_async.h"

__ALIGNED(32) ehci_QH_array_t st_QH[HCD_ASYNC_NUM_OF_QH];
__ALIGNED(32) ehci_qTD_t st_qTD[HCD_ASYNC_NUM_OF_QTD];


static hcd_Async_QH_Mgr_t st_QHMgr[HCD_ASYNC_NUM_OF_QH];
static hcd_Async_qTD_Mgr_t st_qTDMgr[HCD_ASYNC_NUM_OF_QTD];

static uint32_t st_TxMap;

static hcd_Async_MsgBox_t st_MsgBox;

static int32_t getNewQH(void)
{
  int32_t ret = -1;
  for (int i = 0; i < HCD_ASYNC_NUM_OF_QH; i++){
    if (st_QHMgr[i].state == HCD_UNUSED){
      st_QHMgr[i].state = HCD_USED;
      ret = i;
      break;
    }
  }
  return ret;
}

static int32_t getNewqTD(uint8_t devAddr, uint8_t epNum)
{
  int32_t ret = -1;
  for (int i = 0; i < HCD_ASYNC_NUM_OF_QTD; i++){
    if (st_qTDMgr[i].state == HCD_UNUSED){
      st_qTDMgr[i].state = HCD_USED;
      st_qTDMgr[i].devAddr = devAddr;
      st_qTDMgr[i].epNum = epNum;
      ret = i;
      break;
    }
  }
  return ret;  
}

static int32_t getMgr(uint8_t devAddr, uint8_t epNum, hcd_Async_QH_Mgr_t** mgr)
{
  int32_t ret = -1;
  for (int i = 0; i < HCD_ASYNC_NUM_OF_QH; i++){
    if (st_QHMgr[i].devAddr == devAddr && st_QHMgr[i].epNum == epNum){
      ret = i;
      *mgr = &st_QHMgr[i];
      break;
    }
  }
  return ret;
}

static int32_t setEp0Transfer(uint8_t devAddr)
{
  usb_SetupPacket_t setup;
  int32_t idx = -1;
  ehci_qTD_t *setupqTD, *dataqTD, *statusqTD;
  hcd_Async_QH_Mgr_t* mgr;
  
  idx = getMgr(devAddr, 0, &mgr);
  if (idx < 0){
    return -1;
  }
  
  setupqTD = &st_qTD[mgr->setupIdx];
  dataqTD = &st_qTD[mgr->dataIdx];
  statusqTD = &st_qTD[mgr->mainTxIdx];
  
  setup.DWORD[0] = mgr->bufPointer[0];
  setup.DWORD[1] = mgr->bufPointer[1];
  if (setup.BIT.bmRequestType.dir){
    setupqTD->DWORD0_NQP = (uint32_t)&dataqTD->DWORD0_NQP;
    setupqTD->DWORD1_ANQP = (uint32_t)&dataqTD->DWORD0_NQP;
    setupqTD->DWORD2_QTO = (EHCI_qTD_QTO_dt_0 | EHCI_qTD_QTO_TBT(8) | EHCI_qTD_QTO_CERR | EHCI_qTD_QTO_PID_SETUP | EHCI_qTD_QTO_Status_Active);
    setupqTD->DWORD3_BP0 = (uint32_t)(mgr->bufPointer);
    setupqTD->DWORD4_BP1 = ((uint32_t)(mgr->bufPointer) & 0xFFFFF000) + 0x1000;
    
    dataqTD->DWORD0_NQP = (uint32_t)&statusqTD->DWORD0_NQP;
    dataqTD->DWORD1_ANQP = (uint32_t)&statusqTD->DWORD1_ANQP;
    dataqTD->DWORD2_QTO = (EHCI_qTD_QTO_dt_1 | EHCI_qTD_QTO_TBT(setup.BIT.wLength) | EHCI_qTD_QTO_CERR | EHCI_qTD_QTO_PID_IN | EHCI_qTD_QTO_Status_Active);
    dataqTD->DWORD3_BP0 = (uint32_t)(&mgr->bufPointer[2]);
    dataqTD->DWORD4_BP1 = ((uint32_t)(&mgr->bufPointer[2]) & 0xFFFFF000) + 0x1000;
    
    statusqTD->DWORD0_NQP = EHCI_qTD_NQP_T;
    statusqTD->DWORD1_ANQP = EHCI_qTD_ANQP_T;
    statusqTD->DWORD2_QTO = (EHCI_qTD_QTO_dt_1 | EHCI_qTD_QTO_CERR | EHCI_qTD_QTO_PID_OUT | EHCI_qTD_QTO_IOC | EHCI_qTD_QTO_Status_Active);
    statusqTD->DWORD3_BP0 = (uint32_t)(&mgr->bufPointer[2]);
  } else if ((setup.BIT.bmRequestType.dir == 0) && setup.BIT.wLength){
    setupqTD->DWORD0_NQP = (uint32_t)&dataqTD->DWORD0_NQP;
    setupqTD->DWORD1_ANQP = (uint32_t)&dataqTD->DWORD0_NQP;
    setupqTD->DWORD2_QTO = (EHCI_qTD_QTO_dt_0 | EHCI_qTD_QTO_TBT(8) | EHCI_qTD_QTO_CERR | EHCI_qTD_QTO_PID_SETUP | EHCI_qTD_QTO_Status_Active);
    setupqTD->DWORD3_BP0 = (uint32_t)(mgr->bufPointer);
    setupqTD->DWORD4_BP1 = ((uint32_t)(mgr->bufPointer) & 0xFFFFF000) + 0x1000;
    
    dataqTD->DWORD0_NQP = (uint32_t)&statusqTD->DWORD0_NQP;
    dataqTD->DWORD1_ANQP = (uint32_t)&statusqTD->DWORD1_ANQP;
    dataqTD->DWORD2_QTO = (EHCI_qTD_QTO_dt_1 | EHCI_qTD_QTO_TBT(setup.BIT.wLength) | EHCI_qTD_QTO_CERR | EHCI_qTD_QTO_PID_OUT | EHCI_qTD_QTO_Status_Active);
    dataqTD->DWORD3_BP0 = (uint32_t)(&mgr->bufPointer[2]);
    dataqTD->DWORD4_BP1 = ((uint32_t)(&mgr->bufPointer[2]) & 0xFFFFF000) + 0x1000;
    
    statusqTD->DWORD0_NQP = EHCI_qTD_NQP_T;
    statusqTD->DWORD1_ANQP = EHCI_qTD_ANQP_T;
    statusqTD->DWORD2_QTO = (EHCI_qTD_QTO_dt_1 | EHCI_qTD_QTO_CERR | EHCI_qTD_QTO_PID_IN | EHCI_qTD_QTO_IOC | EHCI_qTD_QTO_Status_Active);
    statusqTD->DWORD3_BP0 = (uint32_t)(&mgr->bufPointer[2]);
  } else {
    setupqTD->DWORD0_NQP = (uint32_t)&statusqTD->DWORD0_NQP;
    setupqTD->DWORD1_ANQP = (uint32_t)&statusqTD->DWORD0_NQP;
    setupqTD->DWORD2_QTO = (EHCI_qTD_QTO_dt_0 | EHCI_qTD_QTO_TBT(8) | EHCI_qTD_QTO_CERR | EHCI_qTD_QTO_PID_SETUP | EHCI_qTD_QTO_Status_Active);
    setupqTD->DWORD3_BP0 = (uint32_t)(mgr->bufPointer);
    setupqTD->DWORD4_BP1 = ((uint32_t)(mgr->bufPointer) & 0xFFFFF000) + 0x1000;
    
    statusqTD->DWORD0_NQP = EHCI_qTD_NQP_T;
    statusqTD->DWORD1_ANQP = EHCI_qTD_ANQP_T;
    statusqTD->DWORD2_QTO = (EHCI_qTD_QTO_dt_1 | EHCI_qTD_QTO_CERR | EHCI_qTD_QTO_PID_IN | EHCI_qTD_QTO_IOC | EHCI_qTD_QTO_Status_Active);
    statusqTD->DWORD3_BP0 = (uint32_t)(&mgr->bufPointer[2]);    
  }
  return 0;
}

static int32_t startTransfer(uint8_t devAddr, uint8_t epNum, uint16_t txLen)
{
  int32_t idx = -1;
  ehci_QH_t* QH;
  ehci_qTD_t* qTD;
  hcd_Async_QH_Mgr_t* mgr;
  
  idx = getMgr(devAddr, epNum, &mgr);
  if (idx < 0){
    return -1;
  }
  
  QH = &st_QH[idx].QH;
  if (epNum == 0){
    setEp0Transfer(devAddr);
    qTD = &st_qTD[mgr->setupIdx];
  } else {
    qTD = &st_qTD[mgr->mainTxIdx];
    qTD->DWORD0_NQP = EHCI_qTD_NQP_T;
    qTD->DWORD1_ANQP = EHCI_qTD_ANQP_T;
    qTD->DWORD2_QTO &= EHCI_qTD_QTO_dt_1;
    qTD->DWORD2_QTO |= (EHCI_qTD_QTO_TBT(txLen) | EHCI_qTD_QTO_CP_0 | EHCI_qTD_QTO_CERR | EHCI_qTD_QTO_IOC | EHCI_qTD_QTO_Status_Active);
    if (epNum & 0x80){
      qTD->DWORD2_QTO |= EHCI_qTD_QTO_PID_IN;
    } else {
      qTD->DWORD2_QTO |= EHCI_qTD_QTO_PID_OUT;
    }
    qTD->DWORD3_BP0 = (uint32_t)mgr->bufPointer;
    qTD->DWORD4_BP1 = ((uint32_t)mgr->bufPointer & 0xFFFFF000) + 0x1000;
  }
  
  mgr->lastTxSize = txLen;
  st_TxMap |= (1 << idx);
  
  QH->DWORD4_NQLP = (uint32_t)(&qTD->DWORD0_NQP);
  QH->DWORD5_ANQLP = (uint32_t)(&qTD->DWORD0_NQP);
  
  return 0;
}

static int32_t setAddress(uint8_t devAddr)
{
  int32_t idx = -1;
  ehci_QH_t* QH;
  hcd_Async_QH_Mgr_t* mgr;
  
  idx = getMgr(0, 0, &mgr);
  if (idx < 0){
    return -1;
  }
	mgr->devAddr = devAddr;
  QH = &st_QH[idx].QH;
  
  QH->DWORD1_EC0 &= ~EHCI_QH_EC0_DA(0x7F);
  QH->DWORD1_EC0 |= EHCI_QH_EC0_DA(devAddr);
  
  return 0;
}

static int32_t setEp0Mps(uint8_t devAddr, uint16_t mps)
{
  int32_t idx = -1;
  ehci_QH_t* QH;
  hcd_Async_QH_Mgr_t* mgr;
  
  idx = getMgr(devAddr, 0, &mgr);
  if (idx < 0){
    return -1;
  }
  QH = &st_QH[idx].QH;
  
  QH->DWORD1_EC0 &= ~EHCI_QH_EC0_MPL_Msk;
  QH->DWORD1_EC0 |= EHCI_QH_EC0_MPL(mps);
  
  return 0;
  
}

static int32_t enqueueMsg(hcd_Async_Msg_t* msg)
{
  int32_t ret;
  EHCI_DisInt();
  if (((st_MsgBox.enqPtr + 1) % HCD_ASYNC_MSGBOX_SIZE) != st_MsgBox.deqPtr){
    memcpy(&st_MsgBox.msg[st_MsgBox.enqPtr], msg, sizeof(hcd_Async_Msg_t));
    st_MsgBox.enqPtr++;
    if (st_MsgBox.enqPtr == HCD_ASYNC_MSGBOX_SIZE){
      st_MsgBox.enqPtr = 0;
    }
    ret = 0;
  } else {
    ret = -1;
  }
  
  EHCI_EnaInt();
  NVIC_SetPendingIRQ(HcdAsync_IRQn);
  
  return ret;
}

static int32_t dequeueMsg(hcd_Async_Msg_t* msg)
{
  int32_t ret;
  EHCI_DisInt();
  if (st_MsgBox.deqPtr != st_MsgBox.enqPtr){
    memcpy(msg, &st_MsgBox.msg[st_MsgBox.deqPtr], sizeof(hcd_Async_Msg_t));
    st_MsgBox.deqPtr++;
    if (st_MsgBox.deqPtr == HCD_ASYNC_MSGBOX_SIZE){
      st_MsgBox.deqPtr = 0;
    }
    ret = 0;
  } else {
    ret = -1;
  }
  EHCI_EnaInt();
  return ret;
}

static void usbIntProc(void)
{
  uint16_t actTxLen;
  uint32_t txMap_tmp;
  int32_t idx, qidx;
  ehci_qTD_t* txqTD;
  if (st_TxMap == 0){
    return;
  }
  
  txMap_tmp = st_TxMap;
  
  while(txMap_tmp){
    idx = 31 - __CLZ(txMap_tmp);
    if ((*st_QHMgr[idx].iocPointer & EHCI_qTD_QTO_Status_Active) == 0){
      st_TxMap &= ~(1 << idx);
      if (st_QHMgr[idx].epNum != 0){
        qidx = st_QHMgr[idx].mainTxIdx;
      } else {
        qidx = st_QHMgr[idx].dataIdx;
      }
      actTxLen = st_QHMgr[idx].lastTxSize - ((st_qTD[qidx].DWORD2_QTO & EHCI_qTD_QTO_TBT_Msk) >> 16);
      st_QH[idx].completeCallback(st_QHMgr[idx].devAddr, st_QHMgr[idx].epNum, actTxLen);
    }
    txMap_tmp &= ~(1 << idx);
  }
  
}

static void usbIntHandler(void)
{
  hcd_Async_Msg_t msg;
  msg.msgType = ASYNC_USBINT;
  enqueueMsg(&msg);
}


static void asyncMainTask(void)
{
  hcd_Async_Msg_t msg;
  int32_t status;
  NVIC_ClearPendingIRQ(HcdAsync_IRQn);
  for(;;){
    status = dequeueMsg(&msg);
    if (status != 0){
      break;
    }
    switch(msg.msgType){
      case(ASYNC_USBINT):{
        usbIntProc();
        break;
      }
      case(ASYNC_TX_START):{
        startTransfer(msg.devAddr, msg.epNum, msg.txLen);
        break;
      }
      case(ASYNC_SET_ADDRESS):{
        setAddress(msg.devAddr);
        break;
      }
      case(ASYNC_SET_EP0_MPS):{
        setEp0Mps(msg.devAddr, msg.ep0Mps);
        break;
      }
      default:
      break;
    }
  }
}



void InitAsyncSchedule(void)
{
  EHCI_SetCallback(USB_INT_ASYNC, usbIntHandler);
  
  NVIC_SetVector(HcdAsync_IRQn, (uint32_t)asyncMainTask);
  NVIC_SetPriority(HcdAsync_IRQn, 4);
  NVIC_EnableIRQ(HcdAsync_IRQn);
  
  for (int i = 0; i < HCD_ASYNC_NUM_OF_QH; i++){
    
    st_QHMgr[i].state = HCD_UNUSED;
    st_QHMgr[i].devAddr = 0xFF;
    st_QHMgr[i].epNum = 0xFF;
    st_QHMgr[i].mainTxIdx = 0xFF;
    st_QHMgr[i].setupIdx = 0xFF;
    st_QHMgr[i].dataIdx = 0xFF;
    st_QHMgr[i].bufPointer = 0;
    st_QHMgr[i].iocPointer = 0;
    
    st_QH[i].QH.DWORD3_CQLP = EHCI_QH_NQLP_T;
    st_QH[i].QH.DWORD4_NQLP = EHCI_QH_NQLP_T;
    st_QH[i].QH.DWORD5_ANQLP = EHCI_QH_NQLP_T;
    if (i == 0){
      st_QH[i].QH.DWORD1_EC0 = EHCI_QH_EC0_H;
      st_QH[i].QH.DWORD0_QHHLP = (EHCI_QH_QHHLP_QHHLP(((uint32_t)(&st_QH[HCD_ASYNC_NUM_OF_QH - 1].QH.DWORD0_QHHLP))) | EHCI_QH_QHHLP_TYP_QH);
    } else {
      st_QH[i].QH.DWORD0_QHHLP = (EHCI_QH_QHHLP_QHHLP(((uint32_t)(&st_QH[i - 1].QH.DWORD0_QHHLP))) | EHCI_QH_QHHLP_TYP_QH);
    }
  }
  
  
  EHCI->ASYNCLISTADDR = (uint32_t)(&st_QH[0].QH.DWORD0_QHHLP);
  EHCI->USBCMD |= USBHS_USBCMD_ASE_MASK;
}

int32_t OpenAsyncEndpoint(hcd_DeviceInfo_t* device, uint8_t epNum, uint32_t* bufHead, uint16_t mps, void func(uint8_t, uint8_t, uint16_t))
{
  int32_t idx;
  ehci_QH_array_t* QH = 0;
  ehci_qTD_t* qTD = 0;
  hcd_Async_QH_Mgr_t* mgr;
  uint32_t speed;
  
  EHCI_DisInt();
  
  idx = getNewQH();
  if (idx < 0){
    EHCI_EnaInt();
    return -2;
  }
  mgr = &st_QHMgr[idx];
  QH = &st_QH[idx];
  
  mgr->devAddr = device->devAddr;
  mgr->epNum = epNum;
  mgr->bufPointer = bufHead;
  
  idx = getNewqTD(device->devAddr, epNum);
  if (idx < 0){
    EHCI_EnaInt();
    return -2;
  }
  
  mgr->mainTxIdx = idx;
  mgr->iocPointer = &st_qTD[idx].DWORD2_QTO;
  
  QH->QH.DWORD1_EC0 &= EHCI_QH_EC0_H;
  
  if (epNum == 0){
    idx = getNewqTD(device->devAddr, epNum);
    if (idx < 0){
      EHCI_EnaInt();
      return -2;
    }
    mgr->setupIdx = idx;
    
    idx = getNewqTD(device->devAddr, epNum);
    if (idx < 0){
      EHCI_EnaInt();
      return -2;
    }
    mgr->dataIdx = idx;
    
    QH->QH.DWORD1_EC0 |= EHCI_QH_EC0_DTC_qTD;
  } else {
    QH->QH.DWORD1_EC0 |= EHCI_QH_EC0_DTC_QH;
  }
  
  QH->QH.DWORD1_EC0 |= (EHCI_QH_EC0_MPL(mps) | EHCI_QH_EC0_Endpt(epNum) | EHCI_QH_EC0_DA(device->devAddr));
  if (device->speed != DEV_SPEED_UNDEF){
    speed = (uint32_t)(device->speed) - 1;
    QH->QH.DWORD1_EC0 |= (speed << 12);
    if ((device->speed < DEV_SPEED_HIGH) && (epNum == 0)){
      QH->QH.DWORD1_EC0 |= EHCI_QH_EC0_C;
    }
  } else {
    EHCI_EnaInt();
    return -1;
  }
  QH->QH.DWORD2_EC1 |= (EHCI_QH_EC1_Mult_1 | EHCI_QH_EC1_HA(device->hubAddr) | EHCI_QH_EC1_PN(device->hubPort));
  QH->QH.DWORD3_CQLP = EHCI_QH_NQLP_T;
  QH->QH.DWORD4_NQLP = EHCI_QH_NQLP_T;
  QH->QH.DWORD5_ANQLP = EHCI_QH_NQLP_T;
  QH->QH.DWORD6_QTO = 0;
  QH->QH.DWORD7_BP0 = 0;
  QH->QH.DWORD8_BP1 = 0;
  QH->QH.DWORD9_BP2 = 0;
  QH->QH.DWORD10_BP3 = 0;
  QH->QH.DWORD11_BP4 = 0;
  QH->completeCallback = func;
  
  EHCI_EnaInt();
  return 0;
}

int32_t CloseAsyncEndpoint(uint8_t devAddr, uint8_t epNum)
{
  int32_t idx = -1;
  ehci_QH_t* QH;
  ehci_qTD_t* qTD;
  hcd_Async_QH_Mgr_t* mgr;
  
  EHCI_DisInt();
  
  idx = getMgr(devAddr, epNum, &mgr);
  if (idx < 0){
    EHCI_EnaInt();
    return -1;
  }
  
  QH = &st_QH[idx].QH;
  
  QH->DWORD4_NQLP = EHCI_QH_NQLP_T;
  QH->DWORD5_ANQLP = EHCI_QH_NQLP_T;
  
  QH->DWORD1_EC0 = 0;
  QH->DWORD2_EC1 = 0;
  
  
  if (mgr->mainTxIdx != 0xFF){
    st_qTDMgr[mgr->mainTxIdx].state = HCD_UNUSED;
    st_qTDMgr[mgr->mainTxIdx].epNum = 0xFF;
    st_qTDMgr[mgr->mainTxIdx].devAddr = 0xFF;
  }
  
  if (epNum == 0){
    if (mgr->setupIdx != 0xFF){
      st_qTDMgr[mgr->setupIdx].state = HCD_UNUSED;
      st_qTDMgr[mgr->setupIdx].epNum = 0xFF;
      st_qTDMgr[mgr->setupIdx].devAddr = 0xFF;
    }
    if (mgr->dataIdx != 0xFF){
      st_qTDMgr[mgr->dataIdx].state = HCD_UNUSED;
      st_qTDMgr[mgr->dataIdx].epNum = 0xFF;
      st_qTDMgr[mgr->dataIdx].devAddr = 0xFF;
    }    
  }
  
  mgr->state = HCD_UNUSED;
  mgr->devAddr = 0xFF;
  mgr->epNum = 0xFF;
  mgr->mainTxIdx = 0xFF;
  mgr->setupIdx = 0xFF;
  mgr->dataIdx = 0xFF;
  mgr->bufPointer = 0;
  mgr->iocPointer = 0;
  
  EHCI_EnaInt();
  return 0;
}

int32_t HcdAsync_StartTransfer(uint8_t devAddr, uint8_t epNum, uint16_t txLen)
{
  hcd_Async_Msg_t msg;
  msg.msgType = ASYNC_TX_START;
  msg.devAddr = devAddr;
  msg.epNum = epNum;
  msg.txLen = txLen;
  
  return enqueueMsg(&msg);
}

int32_t HcdAsync_SetAddress(uint8_t devAddr)
{
  hcd_Async_Msg_t msg;
  msg.msgType = ASYNC_SET_ADDRESS;
  msg.devAddr = devAddr;
  
  return enqueueMsg(&msg);  
}

int32_t HcdAsync_SetEp0Mps(uint8_t devAddr, uint16_t mps)
{
  hcd_Async_Msg_t msg;
  msg.msgType = ASYNC_SET_EP0_MPS;
  msg.ep0Mps = mps;
  msg.devAddr = devAddr;
  
  return enqueueMsg(&msg);
}

int32_t HcdAsync_GetTransferState(uint8_t devAddr, uint8_t epNum)
{
  int32_t idx = -1;
  ehci_QH_t* QH;
  ehci_qTD_t* qTD;
  hcd_Async_QH_Mgr_t* mgr;
  
  EHCI_DisInt();
  
  idx = getMgr(devAddr, epNum, &mgr);
  if (idx < 0){
    EHCI_EnaInt();
    return -1;
  }
  if (st_TxMap & (1 << idx)){
    EHCI_EnaInt();
    return -2;
  }

  EHCI_EnaInt();
  return 0;
}
