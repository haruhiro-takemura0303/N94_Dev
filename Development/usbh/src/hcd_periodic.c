/**
* @brief   MCXN947V USB Host Controller Simple Periodic Transfer Driver
* @author  masa
* @version 1.00 
*/

#include "hcd_periodic.h"

__ALIGNED(4096) static uint32_t st_PeriodicFrameList[HCD_PERIODIC_PFL_SIZE];

__ALIGNED(32) static hcd_Periodic_iTD_buf_t st_Out_iTD[HCD_PERIODIC_MAX_NUM_OF_ISOCHOUT_EP];
__ALIGNED(32) static hcd_Periodic_iTD_buf_t st_In_iTD[HCD_PERIODIC_MAX_NUM_OF_ISOCHIN_EP];
__ALIGNED(32) static ehci_QH_array_t st_Intr_QH[HCD_PERIODIC_MAX_NUM_OF_INTR_EP];
__ALIGNED(32) static ehci_qTD_t st_Intr_qTD[HCD_PERIODIC_MAX_NUM_OF_INTR_EP];

static hcd_Periodic_Isoch_Mgr_t st_IsochOutMgr[HCD_PERIODIC_MAX_NUM_OF_ISOCHOUT_EP];
static hcd_Periodic_Isoch_Mgr_t st_IsochInMgr[HCD_PERIODIC_MAX_NUM_OF_ISOCHIN_EP];
static hcd_Periodic_QH_Mgr_t st_IntrMgr[HCD_PERIODIC_MAX_NUM_OF_INTR_EP];

static uint32_t st_IntrTxMap;
static hcd_Periodic_Isoch_Map_t st_IsochMap;
static uint8_t st_IsochInInitialMap;
static uint8_t st_IsochOutInitialMap;

static hcd_Periodic_MsgBox_t st_MsgBox;


static int getNewIsoch(uint8_t devAddr, uint8_t epNum, hcd_Periodic_Isoch_Mgr_t** mgr)
{
  int ret = -1;
  uint8_t maxSize;
  hcd_Periodic_Isoch_Mgr_t* mgrArray;
  if (epNum & 0x80){
    mgrArray = st_IsochInMgr;
    maxSize = HCD_PERIODIC_MAX_NUM_OF_ISOCHIN_EP;
  } else {
    mgrArray = st_IsochOutMgr;
    maxSize = HCD_PERIODIC_MAX_NUM_OF_ISOCHOUT_EP;
  }
  for (int i = 0; i < maxSize; i++){
    if (mgrArray[i].state == HCD_UNUSED){
      mgrArray[i].state = HCD_USED;
      mgrArray[i].devAddr = devAddr;
      mgrArray[i].epNum = epNum;
      *mgr = &mgrArray[i];
      ret = i;
      break;
    }
  }
  return ret;
}

static int getIsochMgr(uint8_t devAddr, uint8_t epNum, hcd_Periodic_Isoch_Mgr_t** mgr)
{
  int ret = -1;
  uint8_t maxSize;
  hcd_Periodic_Isoch_Mgr_t* mgrArray;
  if (epNum & 0x80){
    mgrArray = st_IsochInMgr;
    maxSize = HCD_PERIODIC_MAX_NUM_OF_ISOCHIN_EP;
  } else {
    mgrArray = st_IsochOutMgr;
    maxSize = HCD_PERIODIC_MAX_NUM_OF_ISOCHOUT_EP;
  }
  for (int i = 0; i < maxSize; i++){
    if ((mgrArray[i].state == HCD_USED) && (mgrArray[i].devAddr == devAddr) && (mgrArray[i].epNum == epNum)){
      *mgr = &mgrArray[i];
      ret = i;
      break;
    }
  }
  return ret;
}

static int32_t getNewQH(void)
{
  int32_t ret = -1;
  for (int i = 0; i < HCD_PERIODIC_MAX_NUM_OF_INTR_EP; i++){
    if (st_IntrMgr[i].state == HCD_UNUSED){
      st_IntrMgr[i].state = HCD_USED;
      ret = i;
      break;
    }
  }
  return ret;
}

static int32_t getMgr(uint8_t devAddr, uint8_t epNum, hcd_Periodic_QH_Mgr_t** mgr)
{
  int32_t ret = -1;
  for (int i = 0; i < HCD_PERIODIC_MAX_NUM_OF_INTR_EP; i++){
    if (st_IntrMgr[i].devAddr == devAddr && st_IntrMgr[i].epNum == epNum){
      ret = i;
      *mgr = &st_IntrMgr[i];
      break;
    }
  }
  return ret;
}

static hcd_Status_t enqueueMsg(hcd_Periodic_Msg_t* msg)
{
  hcd_Status_t ret;
  EHCI_DisInt();
  if (st_MsgBox.deqPtr - st_MsgBox.enqPtr != 1){
    memcpy(&st_MsgBox.msg[st_MsgBox.enqPtr], msg, sizeof(hcd_Periodic_Msg_t));
    st_MsgBox.enqPtr++;
    if (st_MsgBox.enqPtr == HCD_PERIODIC_MSGBOX_SIZE){
      st_MsgBox.enqPtr = 0;
    }
    ret = HCD_OK;
  } else {
    ret = HCD_FULL;
  }
  
  EHCI_EnaInt();
  NVIC_SetPendingIRQ(HcdPeriodic_IRQn);
  
  return ret;
}

static hcd_Status_t dequeueMsg(hcd_Periodic_Msg_t* msg)
{
  hcd_Status_t ret;
  EHCI_DisInt();
  if (st_MsgBox.deqPtr != st_MsgBox.enqPtr){
    memcpy(msg, &st_MsgBox.msg[st_MsgBox.deqPtr], sizeof(hcd_Periodic_Msg_t));
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


hcd_Status_t OpenIsochronousEndpoint(uint8_t devAddr, uint8_t epNum, uint16_t mps, usb_psiv_t speed, uint32_t samFreq, uint8_t bytePerSample, uint32_t* buf0, uint32_t* buf1, void func(uint8_t, uint8_t, uint16_t, uint32_t*, hcd_Periodic_IsochIn_ActTxInfo_t*))
{
  uint16_t bytePerESIT_lo, bytePerESIT_hi, fs_10ms, fs_1ms, fs_250us;
  uint8_t rem;
  hcd_Periodic_Isoch_Mgr_t* mgr;
  hcd_Periodic_iTD_buf_t* iTDs;
  int idx;
  
  EHCI_DisInt();
  
  if (((uint32_t)buf0 & 0xFFF) || ((uint32_t)buf1 & 0xFFF)){
    EHCI_EnaInt();
    return HCD_INVALID_PARAM;
  }
  
  idx = getNewIsoch(devAddr, epNum, &mgr);
  if (idx < 0){
    EHCI_EnaInt();
    return HCD_FULL;
  }
  
  /*Sampling Frequency Evaluation*/
  if (samFreq % 100){
    EHCI_EnaInt();
    return HCD_UNSUPPORTED_SAMFREQ;
  }
  
  fs_10ms = samFreq / 100;
  rem = fs_10ms % 10;
  fs_1ms = (fs_10ms - rem) / 10;
  
  if (fs_1ms % 4){
    EHCI_EnaInt();
    return HCD_UNSUPPORTED_SAMFREQ;
  }
  
  fs_250us = fs_1ms >> 2;
  
  mgr->bytePerSample = bytePerSample;
  mgr->buf[0] = buf0;
  mgr->buf[1] = buf1;
  mgr->completeCallback = func;
  
  if (speed == DEV_SPEED_HIGH){
    bytePerESIT_hi = ((fs_250us + 1) >> 1) * bytePerSample;
    bytePerESIT_lo = (fs_250us >> 1)  * bytePerSample;
    
    if (epNum & 0x80){
      iTDs = &st_In_iTD[idx];
      mgr->inParam.bytePerESIT = bytePerESIT_hi;
      for (int i = 0; i < 2; i++){
        for (int j = 0; j < HCD_PERIODIC_iTD_SINGLE_BUF; j++){
          iTDs->doubleBuf[i].iTD[j].DWORD9_BP0 = EHCI_iTD_BPx_BP(mgr->buf[i]) | EHCI_iTD_BP0_EndPt(epNum) | EHCI_iTD_BP0_DA(devAddr);
          iTDs->doubleBuf[i].iTD[j].DWORD10_BP1 = EHCI_iTD_BP1_D_IN | EHCI_iTD_BP1_MPS(mps);
          iTDs->doubleBuf[i].iTD[j].DWORD11_BP2 = EHCI_iTD_BP2_Mult_1;
          iTDs->doubleBuf[i].iTD[j].DWORD12_BP3 = 0;
          iTDs->doubleBuf[i].iTD[j].DWORD13_BP4 = 0;
          iTDs->doubleBuf[i].iTD[j].DWORD14_BP5 = 0;
          iTDs->doubleBuf[i].iTD[j].DWORD15_BP6 = 0;
        }
      }
    } else {
      iTDs = &st_Out_iTD[idx];
      mgr->outParam.sampleRemainder = rem;
      mgr->outParam.esitCountMAX = 80;
      mgr->outParam.esitCount = 80;
      mgr->outParam.bytePerESIT_Even = bytePerESIT_hi;
      mgr->outParam.bytePerESIT_Odd = bytePerESIT_lo;
      for (int i = 0; i < 2; i++){
        for (int j = 0; j < HCD_PERIODIC_iTD_SINGLE_BUF; j++){
          iTDs->doubleBuf[i].iTD[j].DWORD9_BP0 = EHCI_iTD_BPx_BP(mgr->buf[i]) | EHCI_iTD_BP0_EndPt(epNum) | EHCI_iTD_BP0_DA(devAddr);
          iTDs->doubleBuf[i].iTD[j].DWORD10_BP1 = EHCI_iTD_BP1_D_OUT | EHCI_iTD_BP1_MPS(mps);
          iTDs->doubleBuf[i].iTD[j].DWORD11_BP2 = EHCI_iTD_BP2_Mult_1;
          iTDs->doubleBuf[i].iTD[j].DWORD12_BP3 = 0;
          iTDs->doubleBuf[i].iTD[j].DWORD13_BP4 = 0;
          iTDs->doubleBuf[i].iTD[j].DWORD14_BP5 = 0;
          iTDs->doubleBuf[i].iTD[j].DWORD15_BP6 = 0;
        }
      }
    }
  } else {
    bytePerESIT_hi = fs_1ms * bytePerSample;
    bytePerESIT_lo = fs_1ms * bytePerSample;
  }
  
  EHCI_EnaInt();
  return HCD_OK;
}

hcd_Status_t OpenInterruptEndpoint(hcd_DeviceInfo_t* device, uint8_t epNum, uint32_t* bufHead, uint16_t mps, void func(uint8_t, uint8_t, uint16_t))
{
  int32_t idx;
  ehci_QH_array_t* QH = 0;
  ehci_qTD_t* qTD = 0;
  hcd_Periodic_QH_Mgr_t* mgr;
  uint32_t speed;
  
  EHCI_DisInt();
  
  idx = getNewQH();
  if (idx < 0){
    EHCI_EnaInt();
    return HCD_FULL;
  }
  mgr = &st_IntrMgr[idx];
  QH = &st_Intr_QH[idx];
  qTD = &st_Intr_qTD[idx];
  
  mgr->devAddr = device->devAddr;
  mgr->epNum = epNum;
  mgr->bufPointer = bufHead;
  
  
  QH->QH.DWORD1_EC0 &= EHCI_QH_EC0_H;
  QH->QH.DWORD1_EC0 |= EHCI_QH_EC0_DTC_QH;
  
  QH->QH.DWORD1_EC0 |= (EHCI_QH_EC0_MPL(mps) | EHCI_QH_EC0_Endpt(epNum) | EHCI_QH_EC0_DA(device->devAddr));
  if (device->speed != DEV_SPEED_UNDEF){
    speed = (uint32_t)(device->speed) - 1;
    QH->QH.DWORD1_EC0 |= (speed << 12);
  } else {
    EHCI_EnaInt();
    return HCD_INVALID_PARAM;
  }
  QH->QH.DWORD2_EC1 |= (EHCI_QH_EC1_Mult_1 | EHCI_QH_EC1_HA(device->hubAddr) | EHCI_QH_EC1_PN(device->hubPort) | EHCI_QH_EC1_ISM(1));
  if ((device->speed == DEV_SPEED_FULL) || (device->speed == DEV_SPEED_LOW)){
    QH->QH.DWORD2_EC1 |= EHCI_QH_EC1_SCM(0xFF);
  }
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
  return HCD_OK;
}

hcd_Status_t CloseInterruptEndpoint(uint8_t devAddr, uint8_t epNum)
{
  int32_t idx = -1;
  ehci_QH_t* QH;
  ehci_qTD_t* qTD;
  hcd_Periodic_QH_Mgr_t* mgr;
  
  EHCI_DisInt();
  
  idx = getMgr(devAddr, epNum, &mgr);
  if (idx < 0){
    EHCI_EnaInt();
    return HCD_NULL;
  }
  
  QH = &st_Intr_QH[idx].QH;
  
  QH->DWORD4_NQLP = EHCI_QH_NQLP_T;
  QH->DWORD5_ANQLP = EHCI_QH_NQLP_T;
  
  QH->DWORD1_EC0 = 0;
  QH->DWORD2_EC1 = 0;
  
  mgr->state = HCD_UNUSED;
  mgr->devAddr = 0xFF;
  mgr->epNum = 0xFF;
  mgr->bufPointer = 0;
  
  EHCI_EnaInt();
  return HCD_OK;
}

hcd_Status_t CloseIsochronousEndpoint(uint8_t devAddr, uint8_t epNum)
{
  int idx;
  hcd_Periodic_Isoch_Mgr_t* mgr;
  hcd_Periodic_iTD_buf_t* iTDs;
  
  EHCI_DisInt();
  
  idx = getIsochMgr(devAddr, epNum, &mgr);
  if (idx < 0){
    EHCI_EnaInt();
    return HCD_NULL;
  }
  
  if (epNum & 0x80){
    iTDs = &st_In_iTD[idx];
  } else {
    iTDs = &st_Out_iTD[idx];
  }
  for (int i = 0; i < 2; i++){
    for (int j = 0; j < HCD_PERIODIC_iTD_SINGLE_BUF; j++){
      for (int k = 0; k < MAX_iTD_TSC; k++){
        iTDs->doubleBuf[i].iTD[j].TSCx[k] = 0;
      }
      iTDs->doubleBuf[i].iTD[j].DWORD9_BP0 = 0;
      iTDs->doubleBuf[i].iTD[j].DWORD10_BP1 = 0;
      iTDs->doubleBuf[i].iTD[j].DWORD11_BP2 = 0;
    }
  }
  return HCD_OK;
}

static uint16_t refillIsochronousTD(uint8_t devAddr, uint8_t epNum, uint8_t bufIdx)
{
  int idx;
  uint16_t samCount = 0;
  hcd_Periodic_Isoch_Mgr_t* mgr;
  
  idx = getIsochMgr(devAddr, epNum, &mgr);
  if (idx < 0){
    return HCD_FULL;
  }
  
  if (epNum & 0x80){
    for(int i = 0; i < HCD_PERIODIC_iTD_SINGLE_BUF; i++){
      for (int j = 0; j < MAX_iTD_TSC; j++){
        st_In_iTD[idx].doubleBuf[bufIdx].iTD[i].TSCx[j] = EHCI_iTD_TSCx_Status_Active | EHCI_iTD_TSCx_TL(mgr->inParam.bytePerESIT) | EHCI_iTD_TSCx_PG_0 | EHCI_iTD_TSCx_OFFSET(samCount);
        samCount += mgr->inParam.bytePerESIT;
      }
    }
    st_In_iTD[idx].doubleBuf[bufIdx].iTD[HCD_PERIODIC_iTD_SINGLE_BUF - 1].TSCx[MAX_iTD_TSC - 1] |= EHCI_iTD_TSCx_IOC;
  } else {
    for(int i = 0; i < HCD_PERIODIC_iTD_SINGLE_BUF; i++){
      for (int j = 0; j < MAX_iTD_TSC; j++){
        if ((j & 0b1) && (mgr->outParam.esitCount > 2 * mgr->outParam.sampleRemainder - 1)){
          st_Out_iTD[idx].doubleBuf[bufIdx].iTD[i].TSCx[j] = EHCI_iTD_TSCx_Status_Active | EHCI_iTD_TSCx_TL(mgr->outParam.bytePerESIT_Odd) | EHCI_iTD_TSCx_PG_0 | EHCI_iTD_TSCx_OFFSET(samCount);
          samCount += mgr->outParam.bytePerESIT_Odd;
        } else {
          st_Out_iTD[idx].doubleBuf[bufIdx].iTD[i].TSCx[j] = EHCI_iTD_TSCx_Status_Active | EHCI_iTD_TSCx_TL(mgr->outParam.bytePerESIT_Even) | EHCI_iTD_TSCx_PG_0 | EHCI_iTD_TSCx_OFFSET(samCount);
          samCount += mgr->outParam.bytePerESIT_Even;
        }
        mgr->outParam.esitCount--;
        if (mgr->outParam.esitCount == 0){
          mgr->outParam.esitCount = mgr->outParam.esitCountMAX;
        }
      }
    }
    st_Out_iTD[idx].doubleBuf[bufIdx].iTD[HCD_PERIODIC_iTD_SINGLE_BUF - 1].TSCx[MAX_iTD_TSC - 1] |= EHCI_iTD_TSCx_IOC;
  }
  return samCount;
}

static hcd_Status_t startIsochronous(uint8_t devAddr, uint8_t epNum)
{
  EHCI_DisInt();
  int32_t idx = -1;
  hcd_Periodic_Isoch_Mgr_t* mgr;
  
  idx = getIsochMgr(devAddr, epNum, &mgr);
  if (idx < 0){
    return -1;
  }
  if (epNum & 0x80){
    st_IsochInInitialMap |= (1 << idx);
  } else {
    st_IsochOutInitialMap |= (1 << idx);
  }
  refillIsochronousTD(devAddr, epNum, 0);
  refillIsochronousTD(devAddr, epNum, 1);
  
  EHCI_EnaInt();
  
  return HCD_OK;
}

static hcd_Status_t startInterrupt(uint8_t devAddr, uint8_t epNum, uint16_t txLen)
{
  int32_t idx = -1;
  ehci_QH_t* QH;
  ehci_qTD_t* qTD;
  hcd_Periodic_QH_Mgr_t* mgr;
  
  idx = getMgr(devAddr, epNum, &mgr);
  if (idx < 0){
    return HCD_NULL;
  }
  if (st_IntrTxMap & (1 << idx)){
    return HCD_FULL;
  }
  
  QH = &st_Intr_QH[idx].QH;
  qTD = &st_Intr_qTD[idx];
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
  
  mgr->lastTxSize = txLen;
  st_IntrTxMap |= (1 << idx);
  
  QH->DWORD4_NQLP = (uint32_t)(&qTD->DWORD0_NQP);
  QH->DWORD5_ANQLP = (uint32_t)(&qTD->DWORD0_NQP);
  
  return HCD_OK;
}

static void usbIntProc_Interrupt(void)
{
  uint16_t actTxLen;
  uint32_t txMap_tmp;
  int32_t idx;
  ehci_qTD_t* txqTD;
  if (st_IntrTxMap == 0){
    return;
  }
  
  txMap_tmp = st_IntrTxMap;
  
  while(txMap_tmp){
    idx = 31 - __CLZ(txMap_tmp);
    if ((st_Intr_qTD[idx].DWORD2_QTO & EHCI_qTD_QTO_Status_Active) == 0){
      actTxLen = st_IntrMgr[idx].lastTxSize - ((st_Intr_qTD[idx].DWORD2_QTO & EHCI_qTD_QTO_TBT_Msk) >> 16);
      st_Intr_QH[idx].completeCallback(st_IntrMgr[idx].devAddr, st_IntrMgr[idx].epNum, actTxLen);
    }
    txMap_tmp &= ~(1 << idx);
  }
}

static void usbIntIsochronous(void)
{
  hcd_Periodic_Isoch_Map_t curMap, nextMap;
  hcd_Periodic_Isoch_Mgr_t* mgr;
  uint16_t nextLen;
  uint8_t idx;
  uint32_t extendedMap;
  
  nextMap.whole = 0;
  
  if (st_IsochInInitialMap){
    extendedMap = st_IsochInInitialMap;
    while(extendedMap){
      idx = 31 - __CLZ((uint32_t)(extendedMap));
      if (!(st_In_iTD[idx].doubleBuf[0].iTD[HCD_PERIODIC_iTD_SINGLE_BUF - 1].TSCx[MAX_iTD_TSC - 1] & EHCI_iTD_TSCx_Status_Active)){
        mgr = &st_IsochInMgr[idx];
        for (int i = 0; i < HCD_PERIODIC_iTD_SINGLE_BUF; i++){
          for (int j = 0; j < MAX_iTD_TSC; j++){
            mgr->inParam.dataBuf[0].frame[i].mFrame[j] = mgr->inParam.bytePerESIT - iTD_TRANSFER_LENGTH_from_TSCx(st_In_iTD[idx].doubleBuf[0].iTD[i].TSCx[j]);
          }
        }
        refillIsochronousTD(mgr->devAddr, mgr->epNum, 0);
        mgr->completeCallback(mgr->devAddr, mgr->epNum, 0, mgr->buf[0], &mgr->inParam.dataBuf[0]);
        nextMap.ep.isochIn[1] |= (1 << idx);
        st_IsochInInitialMap &= ~(1 << idx);
      } else if (!(st_In_iTD[idx].doubleBuf[1].iTD[HCD_PERIODIC_iTD_SINGLE_BUF - 1].TSCx[MAX_iTD_TSC - 1] & EHCI_iTD_TSCx_Status_Active)){
        mgr = &st_IsochInMgr[idx];
        for (int i = 0; i < HCD_PERIODIC_iTD_SINGLE_BUF; i++){
          for (int j = 0; j < MAX_iTD_TSC; j++){
            mgr->inParam.dataBuf[1].frame[i].mFrame[j] = mgr->inParam.bytePerESIT - iTD_TRANSFER_LENGTH_from_TSCx(st_In_iTD[idx].doubleBuf[1].iTD[i].TSCx[j]);
          }
        }
        refillIsochronousTD(mgr->devAddr, mgr->epNum, 1);
        mgr->completeCallback(mgr->devAddr, mgr->epNum, 0, mgr->buf[1], &mgr->inParam.dataBuf[1]);
        nextMap.ep.isochIn[0] |= (1 << idx);
        st_IsochInInitialMap &= ~(1 << idx);
      }
      extendedMap &= ~(1 << idx);
    }
  }
  
  if (st_IsochOutInitialMap){
    extendedMap = st_IsochOutInitialMap;
    while(extendedMap){
      idx = 31 - __CLZ((uint32_t)(extendedMap));
      if (!(st_Out_iTD[idx].doubleBuf[0].iTD[HCD_PERIODIC_iTD_SINGLE_BUF - 1].TSCx[MAX_iTD_TSC - 1] & EHCI_iTD_TSCx_Status_Active)){
        mgr = &st_IsochOutMgr[idx];
        nextLen = refillIsochronousTD(mgr->devAddr, mgr->epNum, 0);
        mgr->completeCallback(mgr->devAddr, mgr->epNum, nextLen, mgr->buf[0], NULL);
        nextMap.ep.isochOut[1] |= (1 << idx);
        st_IsochOutInitialMap &= ~(1 << idx);
      } else if (!(st_Out_iTD[idx].doubleBuf[1].iTD[HCD_PERIODIC_iTD_SINGLE_BUF - 1].TSCx[MAX_iTD_TSC - 1] & EHCI_iTD_TSCx_Status_Active)){
        mgr = &st_IsochOutMgr[idx];
        nextLen = refillIsochronousTD(mgr->devAddr, mgr->epNum, 1);
        mgr->completeCallback(mgr->devAddr, mgr->epNum, nextLen, mgr->buf[1], NULL);
        nextMap.ep.isochOut[0] |= (1 << idx);
        st_IsochOutInitialMap &= ~(1 << idx);
      }
      extendedMap &= ~(1 << idx);
    }
  }
  
  if (st_IsochMap.whole){
    curMap.whole = st_IsochMap.whole;
    if (curMap.ep.isochIn[0]){
      extendedMap = curMap.ep.isochIn[0];
      while(extendedMap){
        idx = 31 - __CLZ((uint32_t)(extendedMap));
        if (!(st_In_iTD[idx].doubleBuf[0].iTD[HCD_PERIODIC_iTD_SINGLE_BUF - 1].TSCx[MAX_iTD_TSC - 1] & EHCI_iTD_TSCx_Status_Active)){
          mgr = &st_IsochInMgr[idx];
          for (int i = 0; i < HCD_PERIODIC_iTD_SINGLE_BUF; i++){
            for (int j = 0; j < MAX_iTD_TSC; j++){
              mgr->inParam.dataBuf[0].frame[i].mFrame[j] = mgr->inParam.bytePerESIT - iTD_TRANSFER_LENGTH_from_TSCx(st_In_iTD[idx].doubleBuf[0].iTD[i].TSCx[j]);
            }
          }
          refillIsochronousTD(mgr->devAddr, mgr->epNum, 0);
          mgr->completeCallback(mgr->devAddr, mgr->epNum, 0, mgr->buf[0], &mgr->inParam.dataBuf[0]);
          nextMap.ep.isochIn[1] |= (1 << idx);
        } else {
          nextMap.ep.isochIn[0] |= (1 << idx);
        }
        extendedMap &= ~(1 << idx);
      }
    }
    if (curMap.ep.isochIn[1]){
      extendedMap = curMap.ep.isochIn[1];
      while(extendedMap){
        idx = 31 - __CLZ((uint32_t)(extendedMap));
        if (!(st_In_iTD[idx].doubleBuf[1].iTD[HCD_PERIODIC_iTD_SINGLE_BUF - 1].TSCx[MAX_iTD_TSC - 1] & EHCI_iTD_TSCx_Status_Active)){
          mgr = &st_IsochInMgr[idx];
          for (int i = 0; i < HCD_PERIODIC_iTD_SINGLE_BUF; i++){
            for (int j = 0; j < MAX_iTD_TSC; j++){
              mgr->inParam.dataBuf[1].frame[i].mFrame[j] = mgr->inParam.bytePerESIT - iTD_TRANSFER_LENGTH_from_TSCx(st_In_iTD[idx].doubleBuf[1].iTD[i].TSCx[j]);
            }
          }
          refillIsochronousTD(mgr->devAddr, mgr->epNum, 1);
          mgr->completeCallback(mgr->devAddr, mgr->epNum, 0, mgr->buf[1], &mgr->inParam.dataBuf[1]);
          nextMap.ep.isochIn[0] |= (1 << idx);
        } else {
          nextMap.ep.isochIn[1] |= (1 << idx);
        }
        extendedMap &= ~(1 << idx);
      }
    }
    if (curMap.ep.isochOut[0]){
      extendedMap = curMap.ep.isochOut[0];
      while(extendedMap){
        idx = 31 - __CLZ((uint32_t)(extendedMap));
        if (!(st_Out_iTD[idx].doubleBuf[0].iTD[HCD_PERIODIC_iTD_SINGLE_BUF - 1].TSCx[MAX_iTD_TSC - 1] & EHCI_iTD_TSCx_Status_Active)){
          mgr = &st_IsochOutMgr[idx];
          nextLen = refillIsochronousTD(mgr->devAddr, mgr->epNum, 0);
          mgr->completeCallback(mgr->devAddr, mgr->epNum, nextLen, mgr->buf[0], NULL);
          nextMap.ep.isochOut[1] |= (1 << idx);
        } else {
          nextMap.ep.isochOut[0] |= (1 << idx);
        }
        extendedMap &= ~(1 << idx);
      }      
    }
    if (curMap.ep.isochOut[1]){
      extendedMap = curMap.ep.isochOut[1];
      while(extendedMap){
        idx = 31 - __CLZ((uint32_t)(extendedMap));
        if (!(st_Out_iTD[idx].doubleBuf[1].iTD[HCD_PERIODIC_iTD_SINGLE_BUF - 1].TSCx[MAX_iTD_TSC - 1] & EHCI_iTD_TSCx_Status_Active)){
          mgr = &st_IsochOutMgr[idx];
          nextLen = refillIsochronousTD(mgr->devAddr, mgr->epNum, 1);
          mgr->completeCallback(mgr->devAddr, mgr->epNum, nextLen, mgr->buf[1], NULL);
          nextMap.ep.isochOut[0] |= (1 << idx);
        } else {
          nextMap.ep.isochOut[1] |= (1 << idx);
        }
        extendedMap &= ~(1 << idx);
      }
    }
  }
  
  st_IsochMap.whole = nextMap.whole;
}

static void usbIntHandler(void)
{
  hcd_Periodic_Msg_t msg;
  
  usbIntIsochronous();
  
  msg.msgType = PERIODIC_USBINT;
  enqueueMsg(&msg);
}


hcd_Status_t HcdPeriodic_StartInterruptTransfer(uint8_t devAddr, uint8_t epNum, uint16_t txLen)
{
  hcd_Periodic_Msg_t msg;
  msg.msgType = PERIODIC_INTR_TX_START;
  msg.devAddr = devAddr;
  msg.epNum = epNum;
  msg.txLen = txLen;
  
  return enqueueMsg(&msg);
}

hcd_Status_t HcdPeriodic_StartIsochronousTransfer(uint8_t devAddr, uint8_t epNum)
{
  hcd_Periodic_Msg_t msg;
  msg.msgType = PERIODIC_ISOCH_TX_START;
  msg.devAddr = devAddr;
  msg.epNum = epNum;
  
  return enqueueMsg(&msg);
}

static void periodicMainTask(void)
{
  hcd_Periodic_Msg_t msg;
  hcd_Status_t status;
  NVIC_ClearPendingIRQ(HcdPeriodic_IRQn);
  for(;;){
    status = dequeueMsg(&msg);
    if (status != HCD_OK){
      break;
    }
    switch(msg.msgType){
      case(PERIODIC_USBINT):{
        usbIntProc_Interrupt();
        break;
      }
      case(PERIODIC_ISOCH_TX_START):{
        startIsochronous(msg.devAddr, msg.epNum);
        break;
      }
      case(PERIODIC_INTR_TX_START):{
        startInterrupt(msg.devAddr, msg.epNum, msg.txLen);
        break;
      }
      default:
      break;
    }
  }
}

void InitPeriodicSchedule(void)
{
  EHCI_SetCallback(USB_INT_PERIODIC, usbIntHandler);
  
  NVIC_SetVector(HcdPeriodic_IRQn, (uint32_t)periodicMainTask);
  NVIC_SetPriority(HcdPeriodic_IRQn, 4);
  NVIC_EnableIRQ(HcdPeriodic_IRQn);
  
  for (int i = 0; i < HCD_PERIODIC_PFL_SIZE; i++){
    st_PeriodicFrameList[i] = (uint32_t)(&st_Out_iTD[0].doubleBuf[(i >> 2)].iTD[(i & 0x3)].DWORD0_NLP) | EHCI_PFL_TYP_iTD;
    st_Out_iTD[HCD_PERIODIC_MAX_NUM_OF_ISOCHOUT_EP - 1].doubleBuf[(i >> 2)].iTD[(i & 0x3)].DWORD0_NLP = (uint32_t)(&st_In_iTD[0].doubleBuf[(i >> 2)].iTD[(i & 0x3)].DWORD0_NLP) | EHCI_PFL_TYP_iTD;
    if ((i & 0x3) == 0){
      st_In_iTD[HCD_PERIODIC_MAX_NUM_OF_ISOCHOUT_EP - 1].doubleBuf[(i >> 2)].iTD[(i & 0x3)].DWORD0_NLP = (uint32_t)(&st_Intr_QH[0].QH.DWORD0_QHHLP) | EHCI_QH_QHHLP_TYP_QH;
    } else {
      st_In_iTD[HCD_PERIODIC_MAX_NUM_OF_ISOCHOUT_EP - 1].doubleBuf[(i >> 2)].iTD[(i & 0x3)].DWORD0_NLP = EHCI_iTD_NLP_T;
    }
  }
  
  for (int i = 1; i < HCD_PERIODIC_MAX_NUM_OF_ISOCHOUT_EP; i++){
    for (int j = 0; j < 2; j++){
      for (int k = 0; k < HCD_PERIODIC_iTD_SINGLE_BUF; k++){
        st_Out_iTD[i - 1].doubleBuf[j].iTD[k].DWORD0_NLP = (uint32_t)(&st_Out_iTD[i].doubleBuf[j].iTD[k].DWORD0_NLP) | EHCI_iTD_NLP_TYP_iTD;
      }
    }
  }
  
  for (int i = 1; i < HCD_PERIODIC_MAX_NUM_OF_ISOCHIN_EP; i++){
    for (int j = 0; j < 2; j++){
      for (int k = 0; k < HCD_PERIODIC_iTD_SINGLE_BUF; k++){
        st_In_iTD[i - 1].doubleBuf[j].iTD[k].DWORD0_NLP = (uint32_t)(&st_In_iTD[i].doubleBuf[j].iTD[k].DWORD0_NLP) | EHCI_iTD_NLP_TYP_iTD;
      }
    }
  }
  
  for (int i = 0; i < HCD_PERIODIC_MAX_NUM_OF_INTR_EP; i++){
    st_IntrMgr[i].state = HCD_UNUSED;
    st_IntrMgr[i].devAddr = 0xFF;
    st_IntrMgr[i].epNum = 0xFF;
    st_IntrMgr[i].bufPointer = 0;
    st_Intr_QH[i].QH.DWORD3_CQLP = EHCI_QH_NQLP_T;
    st_Intr_QH[i].QH.DWORD4_NQLP = EHCI_QH_NQLP_T;
    st_Intr_QH[i].QH.DWORD5_ANQLP = EHCI_QH_NQLP_T;
    if (i > 0){
      st_Intr_QH[i - 1].QH.DWORD0_QHHLP = (uint32_t)(&st_Intr_QH[i].QH.DWORD0_QHHLP) | EHCI_QH_QHHLP_TYP_QH;
    }
  }
  st_Intr_QH[HCD_PERIODIC_MAX_NUM_OF_INTR_EP - 1].QH.DWORD0_QHHLP = EHCI_QH_QHHLP_T;
  
  EHCI->USBCMD |= (USBHS_USBCMD_FS_1_MASK | USBHS_USBCMD_FS_2_MASK);
  EHCI->PERIODICLISTBASE = (uint32_t)(&st_PeriodicFrameList[0]);
  EHCI->USBCMD |= USBHS_USBCMD_PSE_MASK;
}
