/**
 * @brief   MCXN947V USB Host Controller Driver
 * @author  masa
 * @version 1.00 
 */

#include "hcd.h"

static hcd_DeviceInfo_t st_DeviceInfo[MAX_DEVICE_NUM];

static usbDesc_Device_t st_DeviceDescriptorContainer[MAX_DEVICE_NUM];
static usbDesc_Config_t st_ConfigDescriptorContainer[MAX_DEVICE_NUM];
static config_rawdesc_t st_ConfigRawDesc[MAX_DEVICE_NUM];
static string_info_t st_StingInfo[MAX_DEVICE_NUM];

static struct{
  uint32_t buf[256];
}st_Ep0DatBuf[MAX_DEVICE_NUM];

static struct{
  uint32_t count_us;
  void (*completeCb)(uint8_t miscVal);
  uint8_t miscVal;
}st_GpTimerTable;

static int32_t enqueueMsg(hcd_Async_Msg_t* msg)
{
  int32_t ret;
  EHCI_DisInt();
  if (st_MsgBox.deqPtr - st_MsgBox.enqPtr != 1){
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

static void initGpTimer(void)
{
  EHCI->GPTIMER0CTRL = USBHS_GPTIMER0CTL_MODE(0);
  EHCI->USBINTR |= USBHS_USBINTR_TIE0_MASK;
}

static void gpTimerComplete(void)
{

}

void 
