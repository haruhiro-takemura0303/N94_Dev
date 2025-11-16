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


hcd_Status_t OpenIsochronousEndpoint(uint8_t devAddr, uint8_t epNum, uint32_t samplingFreq, uint8_t bytePerSample, uint32_t* buf0, uint32_t* buf1, void func(uint8_t, uint8_t, uint16_t, uint32_t*, hcd_Periodic_IsochIn_ActTxInfo_t*))
{
  uint8_t fs_10ms, fs_1ms;
  hcd_Periodic_Isoch_Mgr_t* mgr;
  int idx;

  idx = getNewIsoch(devAddr, epNum, mgr);
  if (idx < 0){
    return HCD_FULL;
  }

  


  return HCD_OK;
}
