/**
 *@brief    NXP MCXN947V USB Device Controller Driver Source
 *@author   masa
 *@version	1.00
*/

#include "usbd_sys.h"
#include "clock_config.h"
#include "fsl_spc.h"
#include "cmsis_armclang.h"

#ifndef VSCODE
__ALIGNED(2048)
#endif
/*static*/ dQH_t st_dQH[USBD_MAX_EP_DCI];
#ifndef VSCODE
__ALIGNED(32)
#endif
/*static*/dTD_t stRXdTD[USBD_MAX_EP_NUM];
#ifndef VSCODE
__ALIGNED(32)
#endif
/*static*/dTD_t stTXdTD[USBD_MAX_EP_NUM];

/*static*/ uint32_t stEp0RxBuf[256];
/*static*/ uint32_t stEp0TxBuf[256];

static void phy_FixDeviceMode(void)
{
    spc_active_mode_dcdc_option_t opt = {
        .DCDCVoltage = kSPC_DCDC_OverdriveVoltage,
        .DCDCDriveStrength = kSPC_DCDC_NormalDriveStrength
    };

    spc_sram_voltage_config_t cfg = {
        .operateVoltage = kSPC_sramOperateAt1P2V,
        .requestVoltageUpdate = true
    };
    
    /*DCDC Power Setting*/
    SPC0->ACTIVE_VDELAY = 0x0500;
    SPC_SetActiveModeDCDCRegulatorConfig(SPC0, &opt);
    SPC_SetSRAMOperateVoltage(SPC0, &cfg);

    while ((SPC0->SC & SPC_SC_BUSY_MASK));
    

    /*External Crystal Enable*/
    CLOCK_SetupExtClocking(BOARD_XTAL0_CLK_HZ);

    /*LDO Enable*/
    if (!(SCG0->LDOCSR & SCG_LDOCSR_LDOEN_MASK)){
        SCG0->TRIM_LOCK = 0x5A5A0001;
        SCG0->LDOCSR |= SCG_LDOCSR_LDOEN_MASK;
        while(!(SCG0->LDOCSR & SCG_LDOCSR_VOUT_OK_MASK));
    }

    /*High-Speed USB Clock Enable*/
    SYSCON->AHBCLKCTRL2 |= (SYSCON_AHBCLKCTRL2_USB_HS_MASK | SYSCON_AHBCLKCTRL2_USB_HS_PHY_MASK);
    SYSCON->CLOCK_CTRL |= (SYSCON_CLOCK_CTRL_CLKIN_ENA_MASK | SYSCON_CLOCK_CTRL_CLKIN_ENA_FM_USBH_LPT_MASK);
    CLOCK_EnableClock(kCLOCK_UsbHs);
    CLOCK_EnableClock(kCLOCK_UsbHsPhy);
	
    /*USB PHY Enable*/
    USBPHY->CTRL &= ~(USBPHY_CTRL_SFTRST_MASK | USBPHY_CTRL_CLKGATE_MASK);
    USBPHY->PWD_CLR = 1U;
    USBPHY->CTRL |= (USBPHY_CTRL_ENUTMILEVEL2_MASK | USBPHY_CTRL_ENUTMILEVEL3_MASK);
    CLOCK_EnableUsbhsPhyPllClock(kCLOCK_Usbphy480M, BOARD_XTAL0_CLK_HZ);
    CLOCK_EnableUsbhsClock();
    USBPHY->TX &= ~(USBPHY_TX_D_CAL_MASK | USBPHY_TX_TXCAL45DP_MASK | USBPHY_TX_TXCAL45DM_MASK);
    USBPHY->TX |= (USBPHY_TX_D_CAL(0x04) | USBPHY_TX_TXCAL45DP(7) | USBPHY_TX_TXCAL45DM(7));
}

void Usbd_SysInit(void)
{
    /*PHY & Clocking Configuration*/
		phy_FixDeviceMode();

    /*Controller device mode*/
    UDEV->USBMODE = USBHS_USBMODE_CM(0b10);

    /*Interrupt Enable (USBINT, USBERRINT, PORTSCINT, USBRESETINT)*/
    UDEV->USBINTR = (USBHS_USBINTR_UE_MASK | USBHS_USBINTR_UEE_MASK | USBHS_USBINTR_PCE_MASK | USBHS_USBINTR_URE_MASK);
    NVIC_SetPriority(USB1_HS_IRQn, 1);
    NVIC_EnableIRQ(USB1_HS_IRQn);

    /*Immidiate Interrupt*/
    UDEV->USBCMD &= ~USBHS_USBCMD_ITC_MASK;

    /*Set dQH List*/
    UDEV->ENDPTLISTADDR = (uint32_t)(&st_dQH);
	
    /*Endpoint 0 Settings*/
    st_dQH[USBD_EP0_OUT_DCI].endpointCapability.BIT.ios = 1;
    st_dQH[USBD_EP0_OUT_DCI].endpointCapability.BIT.maximumPacketLength = 0x40;
    st_dQH[USBD_EP0_OUT_DCI].endpointCapability.BIT.mult = 0;
    st_dQH[USBD_EP0_OUT_DCI].endpointCapability.BIT.zlt = 0;

    st_dQH[USBD_EP0_IN_DCI].endpointCapability.BIT.ios = 1;
    st_dQH[USBD_EP0_IN_DCI].endpointCapability.BIT.maximumPacketLength = 0x40;
    st_dQH[USBD_EP0_IN_DCI].endpointCapability.BIT.mult = 0;
    st_dQH[USBD_EP0_IN_DCI].endpointCapability.BIT.zlt = 0;
}

void Usbd_SysStart(void)
{
    UDEV->USBCMD |= USBHS_USBCMD_RS_MASK;
}

void USB1_HS_IRQHandler(void)
{
    uint32_t usbSts = UDEV->USBSTS;
    uint32_t stat;
    if (usbSts & USBHS_USBSTS_UI_MASK){
        /*USB Interrupt (Valid Transfer Completion)*/
        UDEV->USBSTS = USBHS_USBSTS_UI_MASK;
    } 
    if (usbSts & USBHS_USBSTS_UEI_MASK){
        /*USB Error Interrupt (Invalid Transfer Completion)*/
        UDEV->USBSTS = USBHS_USBSTS_UEI_MASK;
    } 
    if (usbSts & USBHS_USBSTS_PCI_MASK){
        /*Port Status Change*/
        UDEV->USBSTS = USBHS_USBSTS_PCI_MASK;
    } 
    if (usbSts & USBHS_USBSTS_URI_MASK){
        /*Bus Reset Received*/
        UDEV->USBSTS = USBHS_USBSTS_URI_MASK;

        stat = UDEV->ENDPTSETUPSTAT;
        UDEV->ENDPTSETUPSTAT = stat;
        
        stat = UDEV->ENDPTCOMPLETE;
        UDEV->ENDPTCOMPLETE = stat;
        
        while (UDEV->ENDPTPRIME);
        UDEV->ENDPTFLUSH = 0xFFFFFFFFUL;

        if (!(UDEV->PORTSC1 & USBHS_PORTSC1_PR_MASK)){
            return;
        }

    } 
    if (usbSts & USBHS_USBSTS_SRI_MASK){
        /*SOF Packet Received*/
        UDEV->USBSTS = USBHS_USBSTS_SRI_MASK;
    }
    if (usbSts & USBHS_USBSTS_SLI_MASK){
        /*Enter Suspend State*/
        UDEV->USBSTS = USBHS_USBSTS_SLI_MASK;
    }

}
