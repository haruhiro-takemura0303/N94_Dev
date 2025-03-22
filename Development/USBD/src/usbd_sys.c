/**
 *@brief    NXP MCXN947V USB Device Controller Driver Source
 *@author   masa
 *@version	1.00
*/

#include "usbd_sys.h"

#ifndef VSCODE
__ALIGNED(2048) /*static*/ dQH_t st_dQH[USBD_MAX_EP_DCI];
#else 
/*static*/ dQH_t st_dQH[USBD_MAX_EP_DCI];
#endif

#ifndef VSCODE
__ALIGNED(32) /*static*/dTD_t stRXdTD[USBD_MAX_EP_NUM];
#else
/*static*/ dTD_t stRXdTD[USBD_MAX_EP_NUM];
#endif

#ifndef VSCODE
__ALIGNED(32) /*static*/dTD_t stTXdTD[USBD_MAX_EP_NUM];
#else
/*static*/ dTD_t stTXdTD[USBD_MAX_EP_NUM];
#endif


/*static*/ uint32_t stEp0RxBuf[256];
/*static*/ uint32_t stEp0TxBuf[256];

/*static*/ usbDcd_Device_info_t stDevice;

#ifndef VSCODE
__STATIC_FORCEINLINE usbDcd_Device_info_t* device(void)
#else
usbDcd_Device_info_t* device(void)
#endif
{
    return &stDevice;
}

#ifndef VSCODE
__STATIC_FORCEINLINE uint8_t epNumToDCI(uint8_t epNum)
#else 
uint8_t epNumToDCI(uint8_t epNum)
#endif
{
	return (2 * (epNum & 0x0F) + ((epNum & 0x80) >> 7));
}

static inline void ep0Stall(void)
{
    Usbd_SetEpStall(USBD_EP0_OUT);
    Usbd_SetEpStall(USBD_EP0_IN);
}

static inline void ep0StatusOut(bool ioc)
{
    Usbd_StartNextTransfer(USBD_EP0_OUT, ioc, 0);
}

static inline void ep0StatusIn(bool ioc)
{
    Usbd_StartNextTransfer(USBD_EP0_IN, ioc, 0);
}

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

static void setupHandler(void)
{
    usb_SetupPacket_t* setup = &device()->lastSetup;
    device()->rxEp[DEFAULT_EP].halt = 0;
    device()->txEp[DEFAULT_EP].halt = 0;
    switch(setup->BIT.bmRequestType.type){
        case(BMREQ_TYPE_STANDARD):{
            switch(setup->BIT.bRequest){
                case(BREQ_GET_STATUS):{
                    break;
                }
                case(BREQ_CLEAR_FEATURE):{
                    break;
                }
                case(BREQ_SET_FEATURE):{
                    break;
                }
                case(BREQ_SET_ADDRESS):{
                    __NOP();
                    break;
                }
                case(BREQ_GET_DESCRIPTOR):{
                    uint8_t descType = (setup->BIT.wValue & 0xFF00) >> 8;
                    uint16_t txSize;
                    usbDcd_Descriptor_Info_t* descInfo = NULL;
                    switch(descType){
                        case(DESCTYPE_DEVICE):{
                            descInfo = &device()->deviceDesc;
                            break;
                        }
                        case(DESCTYPE_CONFIG):{
                            descInfo = &device()->configDesc;
                            break;
                        }
                        case(DESCTYPE_STRING):{
                            if (setup->BIT.wIndex < device()->strMaxIndex){
                                descInfo = &device()->strDescArray[setup->BIT.wIndex];
                            }
                            break;
                        }
                        default:{
                            break;
                        }             
                    }
                    if (descInfo){
                        if (descInfo->descriptor){
                            memcpy(stEp0TxBuf, descInfo->descriptor, descInfo->size);
                            txSize = (setup->BIT.wLength < descInfo->size) ? setup->BIT.wLength : descInfo->size;
                            Usbd_StartNextTransfer(USBD_EP0_IN, USB_IOC_ENABLE, txSize);
                        } else {
                            ep0Stall();
                        }
                    } else {
                        ep0Stall();
                    }
                    break;
                }
                case(BREQ_SET_DESCRIPTOR):{
                    break;
                }
                case(BREQ_GET_CONFIGURATION):{
                    break;
                }
                case(BREQ_SET_CONFIGURATION):{
                    break;
                }
                case(BREQ_GET_INTERFACE):{
                    break;
                }
                case(BREQ_SET_INTERFACE):{
                    break;
                }
                case(BREQ_SYNCH_FRAME):{
                    break;
                }
            }
            break;
        }
        case(BMREQ_TYPE_CLASS):{
            break;
        }
        case(BMREQ_TYPE_VENDOR):{
            break;
        }
    }
}

static void ep0OutHandler(uint16_t size)
{
    usb_SetupPacket_t* setup = &device()->lastSetup;
    switch(setup->BIT.bmRequestType.type){
        case(BMREQ_TYPE_STANDARD):{
            switch(setup->BIT.bRequest){
                case(BREQ_GET_STATUS):{
                    break;
                }
                case(BREQ_CLEAR_FEATURE):{
                    break;
                }
                case(BREQ_SET_FEATURE):{
                    break;
                }
                case(BREQ_SET_ADDRESS):{
                    break;
                }
                case(BREQ_GET_DESCRIPTOR):{
                    break;
                }
                case(BREQ_SET_DESCRIPTOR):{
                    break;
                }
                case(BREQ_GET_CONFIGURATION):{
                    break;
                }
                case(BREQ_SET_CONFIGURATION):{
                    break;
                }
                case(BREQ_GET_INTERFACE):{
                    break;
                }
                case(BREQ_SET_INTERFACE):{
                    break;
                }
                case(BREQ_SYNCH_FRAME):{
                    break;
                }
            }
            break;
        }
        case(BMREQ_TYPE_CLASS):{
            break;
        }
        case(BMREQ_TYPE_VENDOR):{
            break;
        }
    }    
}

static void ep0InHandler(uint16_t size)
{
    usb_SetupPacket_t* setup = &device()->lastSetup;
    switch(setup->BIT.bmRequestType.type){
        case(BMREQ_TYPE_STANDARD):{
            switch(setup->BIT.bRequest){
                case(BREQ_GET_STATUS):{
                    break;
                }
                case(BREQ_CLEAR_FEATURE):{
                    break;
                }
                case(BREQ_SET_FEATURE):{
                    break;
                }
                case(BREQ_SET_ADDRESS):{
                    break;
                }
                case(BREQ_GET_DESCRIPTOR):{
                    /*Data Stage -> Status Stage*/
                    ep0StatusOut(USB_IOC_ENABLE);
                    break;
                }
                case(BREQ_SET_DESCRIPTOR):{
                    break;
                }
                case(BREQ_GET_CONFIGURATION):{
                    break;
                }
                case(BREQ_SET_CONFIGURATION):{
                    break;
                }
                case(BREQ_GET_INTERFACE):{
                    break;
                }
                case(BREQ_SET_INTERFACE):{
                    break;
                }
                case(BREQ_SYNCH_FRAME):{
                    break;
                }
            }
            break;
        }
        case(BMREQ_TYPE_CLASS):{
            break;
        }
        case(BMREQ_TYPE_VENDOR):{
            break;
        }
    }    
}

void Usbd_SysInit(void)
{
    uint32_t sts;
    /*PHY & Clocking Configuration*/
    phy_FixDeviceMode();

    /*Controller device mode*/
    UDEV->USBMODE = USBHS_USBMODE_CM(0b10);

    /*Interrupt Enable (USBINT, USBERRINT, PORTSCINT, USBRESETINT)*/
    sts = UDEV->USBSTS;
    UDEV->USBSTS = sts;
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
    st_dQH[USBD_EP0_OUT_DCI].nextdTDPointer = USBD_dQH_dTD_T;

    st_dQH[USBD_EP0_IN_DCI].endpointCapability.BIT.ios = 1;
    st_dQH[USBD_EP0_IN_DCI].endpointCapability.BIT.maximumPacketLength = 0x40;
    st_dQH[USBD_EP0_IN_DCI].endpointCapability.BIT.mult = 0;
    st_dQH[USBD_EP0_IN_DCI].endpointCapability.BIT.zlt = 0;
    st_dQH[USBD_EP0_IN_DCI].nextdTDPointer = USBD_dQH_dTD_T;

    device()->rxEp[DEFAULT_EP].bufPtr = stEp0RxBuf;
    device()->rxEp[DEFAULT_EP].valid = 1;
    device()->rxEp[DEFAULT_EP].handlerCallback = ep0OutHandler;
    device()->txEp[DEFAULT_EP].bufPtr = stEp0TxBuf;
    device()->txEp[DEFAULT_EP].valid = 1;
    device()->txEp[DEFAULT_EP].handlerCallback = ep0InHandler;
}

void Usbd_SysStart(void)
{
    UDEV->USBCMD |= USBHS_USBCMD_RS_MASK;
}

void USB1_HS_IRQHandler(void)
{
    uint32_t usbSts = UDEV->USBSTS;
    uint32_t stat;
    uint8_t epIdx;
    uint16_t actSize;
    if (usbSts & USBHS_USBSTS_UI_MASK){
        /*USB Interrupt (Valid Transfer Completion)*/
        UDEV->USBSTS = USBHS_USBSTS_UI_MASK;
        if (UDEV->ENDPTSETUPSTAT){
            stat = UDEV->ENDPTSETUPSTAT;
            if (stat == (1 << DEFAULT_EP)){
                UDEV->ENDPTSETUPSTAT = stat;
                UDEV->USBCMD |= USBHS_USBCMD_SUTW_MASK;
                device()->lastSetup.DWORD[0] = st_dQH[0].setupBuffer[0];
                device()->lastSetup.DWORD[1] = st_dQH[0].setupBuffer[1];
                while (!(UDEV->USBCMD & USBHS_USBCMD_SUTW_MASK)){
                    device()->lastSetup.DWORD[0] = st_dQH[0].setupBuffer[0];
                    device()->lastSetup.DWORD[1] = st_dQH[0].setupBuffer[1];
                }
                UDEV->USBCMD &= ~USBHS_USBCMD_SUTW_MASK;
                setupHandler();
            } else {
                UDEV->ENDPTSETUPSTAT = stat;
            }
        } else {
            stat = UDEV->ENDPTCOMPLETE & (USBHS_ENDPTCOMPLETE_ETCE_MASK | USBHS_ENDPTCOMPLETE_ERCE_MASK);
            UDEV->ENDPTCOMPLETE = stat;
            while (stat){
                epIdx = 31 - __CLZ(stat);
                stat &= ~(1 << epIdx);
                if (epIdx >= USBHS_ENDPTCOMPLETE_ETCE_SHIFT){
                    epIdx -= USBHS_ENDPTCOMPLETE_ETCE_SHIFT;
                    actSize = device()->txEp[epIdx].lastTxSize - stTXdTD[epIdx].token.BIT.totalBytes;
                    device()->txEp[epIdx].handlerCallback(actSize);
                } else {
                    actSize = device()->rxEp[epIdx].lastTxSize - stRXdTD[epIdx].token.BIT.totalBytes;
                    device()->rxEp[epIdx].handlerCallback(actSize);                    
                }
            }
        }
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

        device()->busState = DEFAULT;

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

void Usbd_SetDescriptor(int descType, const uint8_t* descPtr, uint16_t descSize)
{
    switch(descType){
        case(DESCTYPE_DEVICE):{
            device()->deviceDesc.descriptor = descPtr;
            device()->deviceDesc.size = descSize;
            break;
        }
        case(DESCTYPE_CONFIG):{
            device()->configDesc.descriptor = descPtr;
            device()->configDesc.size = descSize;
            break;
        }
        default:
            break;
    }
}

void Usbd_SetStringDescriptor(usbDcd_Descriptor_Info_t* descArray, uint8_t maxIndex)
{
    device()->strDescArray = descArray;
    device()->strMaxIndex = maxIndex;
}

usbDcd_Status_t Usbd_StartNextTransfer(uint8_t epNum, bool ioc, uint16_t txSize)
{
    dTD_t* dTDArray;
    dQH_t* dQH;
    usbDcd_Endpoint_Info_t* epArray;
    uint8_t epIdx = (epNum & 0xF);
    uint8_t epDir = (epNum & 0x80) >> 7;
    if (epIdx >= USBD_MAX_EP_NUM){
        return USBD_INVALID_PARAM;
    }
    if (epNum & 0x80){
        dTDArray = stTXdTD;
        epArray = device()->txEp;
    } else {
        dTDArray = stRXdTD;
        epArray = device()->rxEp;
    }
    if (!epArray[epIdx].valid){
        return USBD_DISABLED_EP;
    }

    epArray[epIdx].lastTxSize = txSize;
    
    memset(&dTDArray[epIdx], 0, sizeof(dTD_t));
    dQH = &st_dQH[epNumToDCI(epNum)];

    if(ioc == USB_IOC_ENABLE){
        dTDArray[epIdx].token.BIT.ioc = 1;
    }
    dTDArray[epIdx].nextLinkPointer = USBD_dQH_dTD_T;
    dTDArray[epIdx].token.BIT.totalBytes = (txSize & 0x7FFF);
    dTDArray[epIdx].bufferPointer[0] = (uint32_t)epArray[epIdx].bufPtr;
    dTDArray[epIdx].bufferPointer[1] = ((uint32_t)(epArray[epIdx].bufPtr) & 0xFFFFF000) + 0x1000;

    dTDArray[epIdx].token.BIT.status = USBD_dTD_Token_Active;
    dQH->overray[0] &= ~USBD_dTD_Token_Mask;
    dQH->nextdTDPointer = (uint32_t)&dTDArray[epIdx];

    UDEV->ENDPTPRIME |= ((1 << epIdx) << (16 * epDir));

    return USBD_OK;
}

void Usbd_SetEpStall(uint8_t epNum)
{
    usbDcd_Endpoint_Info_t* epArray;
    uint8_t epIdx = (epNum & 0xF);
    uint8_t epDir = (epNum & 0x80) >> 7;
    if (epDir){
        epArray = device()->txEp;
    } else {
        epArray = device()->rxEp;
    }
    if (!epIdx){
        UDEV->ENDPTCTRL0 |= (1 << (epDir * USBHS_ENDPTCTRL0_TXS_SHIFT));
    } else {
        UDEV->ENDPTCTRL[epIdx - 1] |= (1 << (epDir * USBHS_ENDPTCTRL_TXS_SHIFT));
    }
    epArray[epIdx].halt = 1;
}
