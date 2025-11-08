/**
 * @brief   MCXN947V USBH(EHCI) Low Layer
 * @author  masa
 * @version 1.00 
 */

#include "ehci.h"

static ehci_int_cb_t st_EhciInterrupt;

static void ehciHandler(void)
{
  uint32_t usbSts = EHCI->USBSTS;

  if (usbSts & USBHS_USBSTS_UI_MASK){
    /*USB Interrupt (Valid Transfer Completion)*/
    EHCI->USBSTS = USBHS_USBSTS_UI_MASK;
    if (st_EhciInterrupt.ehciUsbIntPeridicCb){
      st_EhciInterrupt.ehciUsbIntPeridicCb();
    }
    if (st_EhciInterrupt.ehciUsbIntAsyncCb){
      st_EhciInterrupt.ehciUsbIntAsyncCb();
    }
  } 
  if (usbSts & USBHS_USBSTS_UEI_MASK){
    /*USB Error Interrupt (Invalid Transfer Completion)*/
    EHCI->USBSTS = USBHS_USBSTS_UEI_MASK;
  } 
  if (usbSts & USBHS_USBSTS_PCI_MASK){
    /*Port Status Change*/
    EHCI->USBSTS = USBHS_USBSTS_PCI_MASK;
    if (st_EhciInterrupt.ehciPortConnectStatChgCb){
      st_EhciInterrupt.ehciPortConnectStatChgCb();
    }
  } 
  if (usbSts & USBHS_USBSTS_URI_MASK){
    /*Bus Reset*/
    EHCI->USBSTS = USBHS_USBSTS_URI_MASK;
    
  } 
  if (usbSts & USBHS_USBSTS_SRI_MASK){
    /*SOF Packet Received*/
    EHCI->USBSTS = USBHS_USBSTS_SRI_MASK;
  }
  if (usbSts & USBHS_USBSTS_SLI_MASK){
    /*Enter Suspend State*/
    EHCI->USBSTS = USBHS_USBSTS_SLI_MASK;
  }
  if (usbSts & USBHS_USBSTS_TI0_MASK){
    /*General Purpose Timer*/
    EHCI->USBSTS = USBHS_USBSTS_TI0_MASK;
    if (st_EhciInterrupt.ehciGpTimerCb){
      st_EhciInterrupt.ehciGpTimerCb();
    }
  }
}


void EHCI_SysInit(void)
{
  /*Enable High-Speed PHY*/
  UsbPhy_HighSpeedInit();
  NVIC_SetPriority(USB1_HS_IRQn, 0);
  NVIC_EnableIRQ(USB1_HS_IRQn);
  NVIC_SetVector(USB1_HS_IRQn, (uint32_t)(ehciHandler));

  /*Set Host Mode*/
  EHCI->USBMODE = USBHS_USBMODE_CM(0b11);

  /*EHCI Init*/
  EHCI->USBCMD = (USBHS_USBCMD_ITC(0x01) | USBHS_USBCMD_RS_MASK);
  EHCI->USBINTR = (USBHS_USBINTR_UE_MASK | USBHS_USBINTR_UEE_MASK | USBHS_USBINTR_PCE_MASK | USBHS_USBINTR_SEE_MASK);

  /*NXP-EHCI don't use config flag because companion controller is absent*/

  /*Port Start*/
  EHCI->PORTSC1 |= USBHS_PORTSC1_PP_MASK;

}

void EHCI_SetCallback(ehci_int_type_t type, void func(void))
{
  switch(type){
    case(USB_INT_ASYNC):{st_EhciInterrupt.ehciUsbIntAsyncCb = func; break;}
    case(USB_INT_PERIODIC):{st_EhciInterrupt.ehciUsbIntPeridicCb = func; break;}
    case(USB_ERROR_INT):{st_EhciInterrupt.ehciUsbErrIntCb = func; break;}
    case(FRAME_LIST_ROLLOVER):{st_EhciInterrupt.ehciFrameListRlovCb = func; break;}
    case(SYS_ERROR):{st_EhciInterrupt.ehciHostSysErrCb = func; break;}
    case(ASYNC_ADVANCE):{st_EhciInterrupt.ehciIntrOnAsyncAdvCb = func; break;}
    case(PORT_CSC):{st_EhciInterrupt.ehciPortConnectStatChgCb = func; break;}
    case(PORT_PED):{st_EhciInterrupt.ehciPortEnDisCb = func; break;}
    case(PORT_OCC):{st_EhciInterrupt.ehciPortOvCurrCb = func; break;}
    case(GPTIMER0):{st_EhciInterrupt.ehciGpTimerCb = func; break;}
    default: break;
  }
}
