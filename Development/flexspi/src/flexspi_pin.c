/**
* @brief   FRDM-MCXN947 board mikroBUS header port initialization
* @author  masa
* @version 1.00
*/

#include "flexspi_pin.h"

void FlexSpi0_InitPins(void)
{
  
  /* Enables the clock for PORT3: Enables clock */
  CLOCK_EnableClock(kCLOCK_Port3);
  
  CLOCK_EnableClock(kCLOCK_Gpio3);
  
  const port_pin_config_t port3_0_pinB17_config = 
  {/* Internal pull-up/down resistor is disabled */
    kPORT_PullDisable,
    /* Low internal pull resistor value is selected. */
    kPORT_LowPullResistor,
    /* Fast slew rate is configured */
    kPORT_FastSlewRate,
    /* Passive input filter is disabled */
    kPORT_PassiveFilterDisable,
    /* Open drain output is disabled */
    kPORT_OpenDrainDisable,
    /* Low drive strength is configured */
    kPORT_LowDriveStrength,
    /* Pin is configured as FLEXSPI0_A_SS0_b */
    kPORT_MuxAlt8,
    /* Digital input enabled */
    kPORT_InputBufferEnable,
    /* Digital input is not inverted */
    kPORT_InputNormal,
    /* Pin Control Register fields [15:0] are not locked */
    kPORT_UnlockRegister
  };
  /* PORT3_0 (pin B17) is configured as FLEXSPI0_A_SS0_b */
  PORT_SetPinConfig(PORT3, 0U, &port3_0_pinB17_config);
  
  const port_pin_config_t port3_10_pinF17_config = {/* Internal pull-up resistor is enabled */
    kPORT_PullUp,
    /* Low internal pull resistor value is selected. */
    kPORT_LowPullResistor,
    /* Fast slew rate is configured */
    kPORT_FastSlewRate,
    /* Passive input filter is disabled */
    kPORT_PassiveFilterDisable,
    /* Open drain output is disabled */
    kPORT_OpenDrainDisable,
    /* Low drive strength is configured */
    kPORT_LowDriveStrength,
    /* Pin is configured as FLEXSPI0_A_DATA2 */
    kPORT_MuxAlt8,
    /* Digital input enabled */
    kPORT_InputBufferEnable,
    /* Digital input is not inverted */
    kPORT_InputNormal,
    /* Pin Control Register fields [15:0] are not locked */
    kPORT_UnlockRegister
  };
  /* PORT3_10 (pin F17) is configured as FLEXSPI0_A_DATA2 */
  PORT_SetPinConfig(PORT3, 10U, &port3_10_pinF17_config);
  
  const port_pin_config_t port3_11_pinF16_config = {/* Internal pull-up resistor is enabled */
    kPORT_PullUp,
    /* Low internal pull resistor value is selected. */
    kPORT_LowPullResistor,
    /* Fast slew rate is configured */
    kPORT_FastSlewRate,
    /* Passive input filter is disabled */
    kPORT_PassiveFilterDisable,
    /* Open drain output is disabled */
    kPORT_OpenDrainDisable,
    /* Low drive strength is configured */
    kPORT_LowDriveStrength,
    /* Pin is configured as FLEXSPI0_A_DATA3 */
    kPORT_MuxAlt8,
    /* Digital input enabled */
    kPORT_InputBufferEnable,
    /* Digital input is not inverted */
    kPORT_InputNormal,
    /* Pin Control Register fields [15:0] are not locked */
    kPORT_UnlockRegister
  };
  /* PORT3_11 (pin F16) is configured as FLEXSPI0_A_DATA3 */
  PORT_SetPinConfig(PORT3, 11U, &port3_11_pinF16_config);
  
  const port_pin_config_t port3_7_pinD14_config = {/* Internal pull-up/down resistor is disabled */
    kPORT_PullDisable,
    /* Low internal pull resistor value is selected. */
    kPORT_LowPullResistor,
    /* Fast slew rate is configured */
    kPORT_FastSlewRate,
    /* Passive input filter is disabled */
    kPORT_PassiveFilterDisable,
    /* Open drain output is disabled */
    kPORT_OpenDrainDisable,
    /* Low drive strength is configured */
    kPORT_LowDriveStrength,
    /* Pin is configured as FLEXSPI0_A_SCLK */
    kPORT_MuxAlt8,
    /* Digital input enabled */
    kPORT_InputBufferEnable,
    /* Digital input is not inverted */
    kPORT_InputNormal,
    /* Pin Control Register fields [15:0] are not locked */
    kPORT_UnlockRegister
  };
  /* PORT3_7 (pin D14) is configured as FLEXSPI0_A_SCLK */
  PORT_SetPinConfig(PORT3, 7U, &port3_7_pinD14_config);
  
  const port_pin_config_t port3_8_pinE14_config = {/* Internal pull-up/down resistor is disabled */
    kPORT_PullDisable,
    /* Low internal pull resistor value is selected. */
    kPORT_LowPullResistor,
    /* Fast slew rate is configured */
    kPORT_FastSlewRate,
    /* Passive input filter is disabled */
    kPORT_PassiveFilterDisable,
    /* Open drain output is disabled */
    kPORT_OpenDrainDisable,
    /* Low drive strength is configured */
    kPORT_LowDriveStrength,
    /* Pin is configured as FLEXSPI0_A_DATA0 */
    kPORT_MuxAlt8,
    /* Digital input enabled */
    kPORT_InputBufferEnable,
    /* Digital input is not inverted */
    kPORT_InputNormal,
    /* Pin Control Register fields [15:0] are not locked */
    kPORT_UnlockRegister
  };
  /* PORT3_8 (pin E14) is configured as FLEXSPI0_A_DATA0 */
  PORT_SetPinConfig(PORT3, 8U, &port3_8_pinE14_config);
  
  const port_pin_config_t port3_9_pinF15_config = {/* Internal pull-up/down resistor is disabled */
    kPORT_PullDisable,
    /* Low internal pull resistor value is selected. */
    kPORT_LowPullResistor,
    /* Fast slew rate is configured */
    kPORT_FastSlewRate,
    /* Passive input filter is disabled */
    kPORT_PassiveFilterDisable,
    /* Open drain output is disabled */
    kPORT_OpenDrainDisable,
    /* Low drive strength is configured */
    kPORT_LowDriveStrength,
    /* Pin is configured as FLEXSPI0_A_DATA1 */
    kPORT_MuxAlt8,
    /* Digital input enabled */
    kPORT_InputBufferEnable,
    /* Digital input is not inverted */
    kPORT_InputNormal,
    /* Pin Control Register fields [15:0] are not locked */
    kPORT_UnlockRegister
  };
  /* PORT3_9 (pin F15) is configured as FLEXSPI0_A_DATA1 */
  PORT_SetPinConfig(PORT3, 9U, &port3_9_pinF15_config);
}