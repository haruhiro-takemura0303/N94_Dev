
/**
 *@brief		NXP MCXN947V USBPHY Initialize
 *@author		masa
 *@version	1.00
*/

#include "usbphy.h"

void UsbPhy_HighSpeedInit(void)
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
