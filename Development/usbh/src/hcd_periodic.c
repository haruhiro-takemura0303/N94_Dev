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


static int getNewIsoch(uint8_t devAddr, uint8_t epNum, hcd_Periodic_Isoch_Mgr_t* mgr)
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
      mgr = &mgrArray[i];
      ret = i;
      break;
    }
    return ret;
  }
}

static int getIsochMgr(uint8_t devAddr, uint8_t epNum, hcd_Periodic_Isoch_Mgr_t* mgr)
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
      mgr = &mgrArray[i];
      ret = i;
      break;
    }
    return ret;
  }
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
  
  idx = getNewIsoch(devAddr, epNum, mgr);
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

hcd_Status_t CloseIsochronousEndpoint(uint8_t devAddr, uint8_t epNum)
{
  int idx;
  hcd_Periodic_Isoch_Mgr_t* mgr;
  hcd_Periodic_iTD_buf_t* iTDs;
  
  EHCI_DisInt();
  
  idx = getIsochMgr(devAddr, epNum, mgr);
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
  
  idx = getIsochMgr(devAddr, epNum, mgr);
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
