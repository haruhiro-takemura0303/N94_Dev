/**
* @brief   FRDM-MCXN947 board mikroBUS header port initialization
* @author  masa
* @version 1.00
*/

#include "mb_pin.h"

void MikroBusPins_InitLPSPI6(void)
{
   /* Enables the clock for PORT3: Enables clock */
   CLOCK_EnableClock(kCLOCK_Port3);

   CLOCK_EnableClock(kCLOCK_Gpio3);

   /* PORT3_20 (pin M17) is configured as FC6_P0 */
   PORT_SetPinMux(PORT3, 20U, kPORT_MuxAlt3);

   PORT3->PCR[20] = ((PORT3->PCR[20] &
                      /* Mask bits to zero which are setting */
                      (~(PORT_PCR_PS_MASK | PORT_PCR_IBE_MASK)))

                     /* Pull Select: Enables internal pullup resistor. */
                     | PORT_PCR_PS(PCR_PS_ps1)

                     //| PORT_PCR_PE_MASK

                     /* Input Buffer Enable: Enables. */
                     | PORT_PCR_IBE(PCR_IBE_ibe1));

   /* PORT3_21 (pin L16) is configured as FC6_P1 */
   PORT_SetPinMux(PORT3, 21U, kPORT_MuxAlt3);

   PORT3->PCR[21] = ((PORT3->PCR[21] &
                      /* Mask bits to zero which are setting */
                      (~(PORT_PCR_PS_MASK | PORT_PCR_IBE_MASK)))

                     /* Pull Select: Enables internal pullup resistor. */
                     | PORT_PCR_PS(PCR_PS_ps1)

                     //| PORT_PCR_PE_MASK

                     /* Input Buffer Enable: Enables. */
                     | PORT_PCR_IBE(PCR_IBE_ibe1));

   /* PORT3_22 (pin M16) is configured as FC6_P2 */
   PORT_SetPinMux(PORT3, 22U, kPORT_MuxAlt3);

   PORT3->PCR[22] = ((PORT3->PCR[22] &
                      /* Mask bits to zero which are setting */
                      (~(PORT_PCR_PS_MASK | PORT_PCR_IBE_MASK)))

                     /* Pull Select: Enables internal pullup resistor. */
                     | PORT_PCR_PS(PCR_PS_ps1)

                     //| PORT_PCR_PE_MASK

                     /* Input Buffer Enable: Enables. */
                     | PORT_PCR_IBE(PCR_IBE_ibe1));

   /* PORT3_23 (pin M15) is configured as FC6_P3 */
   PORT_SetPinMux(PORT3, 23U, kPORT_MuxAlt3);

   PORT3->PCR[23] = ((PORT3->PCR[23] &
                      /* Mask bits to zero which are setting */
                      (~(PORT_PCR_PS_MASK | PORT_PCR_IBE_MASK)))

                     /* Pull Select: Enables internal pullup resistor. */
                     | PORT_PCR_PS(PCR_PS_ps1)

                     //| PORT_PCR_PE_MASK

                     /* Input Buffer Enable: Enables. */
                     | PORT_PCR_IBE(PCR_IBE_ibe1));

   PORT5->PCR[7] = ((PORT5->PCR[7] &
                     /* Mask bits to zero which are setting */
                     (~(PORT_PCR_PS_MASK | PORT_PCR_MUX_MASK | PORT_PCR_IBE_MASK)))

                    /* Pull Select: Enables internal pullup resistor. */
                    | PORT_PCR_PS(PCR_PS_ps1)

                    //| PORT_PCR_PE_MASK

                    /* Pin Multiplex Control: PORT5_7 (pin L13) is configured as PIO5_7. */
                    | PORT_PCR_MUX(PORT5_PCR_MUX_mux00)

                    /* Input Buffer Enable: Enables. */
                    | PORT_PCR_IBE(PCR_IBE_ibe1));
}

void MikroBusPins_InitLPI2C3(void)
{
  /* Enables the clock for GPIO1: Enables clock */
  CLOCK_EnableClock(kCLOCK_Gpio1);
  /* Enables the clock for PORT1: Enables clock */
  CLOCK_EnableClock(kCLOCK_Port1);

  const port_pin_config_t port1_0_pinP1_config = {/* Internal pull-up resistor is enabled */
      kPORT_PullUp,
      /* Low internal pull resistor value is selected. */
      kPORT_LowPullResistor,
      /* Fast slew rate is configured */
      kPORT_FastSlewRate,
      /* Passive input filter is disabled */
      kPORT_PassiveFilterDisable,
      /* Open drain output is enabled */
      kPORT_OpenDrainEnable,
      /* Low drive strength is configured */
      kPORT_LowDriveStrength,
      /* Pin is configured as FC2_P0 */
      kPORT_MuxAlt2,
      /* Digital input enabled */
      kPORT_InputBufferEnable,
      /* Digital input is not inverted */
      kPORT_InputNormal,
      /* Pin Control Register fields [15:0] are not locked */
      kPORT_UnlockRegister};
  /* PORT1_0 (pin P1) is configured as FC2_P0 */
  PORT_SetPinConfig(PORT1, 0U, &port1_0_pinP1_config);

  const port_pin_config_t port1_1_pinP2_config = {/* Internal pull-up resistor is enabled */
      kPORT_PullUp,
      /* Low internal pull resistor value is selected. */
      kPORT_LowPullResistor,
      /* Fast slew rate is configured */
      kPORT_FastSlewRate,
      /* Passive input filter is disabled */
      kPORT_PassiveFilterDisable,
      /* Open drain output is enabled */
      kPORT_OpenDrainEnable,
      /* Low drive strength is configured */
      kPORT_LowDriveStrength,
      /* Pin is configured as FC2_P1 */
      kPORT_MuxAlt2,
      /* Digital input enabled */
      kPORT_InputBufferEnable,
      /* Digital input is not inverted */
      kPORT_InputNormal,
      /* Pin Control Register fields [15:0] are not locked */
      kPORT_UnlockRegister};
  /* PORT4_1 (pin P2) is configured as FC2_P1 */
  PORT_SetPinConfig(PORT1, 1U, &port1_1_pinP2_config);
}

void MikroBusPins_InitInt5_7(bool pol_default)
{
  gpio_pin_config_t INT_config = {
      .pinDirection = kGPIO_DigitalInput,
      .outputLogic = 0U
  };
  /* Initialize GPIO functionality on pin PIO5_7 (pin L13)  */
  GPIO_PinInit(BOARD_INITPINS_INT_GPIO, BOARD_INITPINS_INT_PIN, &INT_config);


  PORT5->PCR[7] =
      ((PORT5->PCR[7] &
        /* Mask bits to zero which are setting */
        (~(PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_ODE_MASK | PORT_PCR_MUX_MASK | PORT_PCR_IBE_MASK)))

        /* Pull Select: Enables internal pullup resistor. */
        | PORT_PCR_PS(PCR_PS_ps1)

        /* Pull Enable: Enables. */
        | PORT_PCR_PE(pol_default)

        /* Open Drain Enable: Enables. */
        | PORT_PCR_ODE(1)

        /* Pin Multiplex Control: PORT5_7 (pin L13) is configured as PIO5_7. */
        | PORT_PCR_MUX(PORT5_PCR_MUX_mux00)

        /* Input Buffer Enable: Enables. */
        | PORT_PCR_IBE(PCR_IBE_ibe1));	
}

void MikroBusPins_InitReset1_3(bool pol_default)
{
  /* Enables the clock for GPIO1: Enables clock */
  CLOCK_EnableClock(kCLOCK_Gpio1);
  /* Enables the clock for PORT1: Enables clock */
  CLOCK_EnableClock(kCLOCK_Port1);
	
	gpio_pin_config_t RST_config = {
    .pinDirection = kGPIO_DigitalOutput,
    .outputLogic = 0U
  };
	
  /* Initialize GPIO functionality on pin PIO1_3 (pin B4)  */
  GPIO_PinInit(BOARD_INITPINS_RST_GPIO, BOARD_INITPINS_RST_PIN, &RST_config);

  /* PORT1_3 (pin B4) is configured as PIO1_3, WUU0_IN7 */
  PORT_SetPinMux(BOARD_INITPINS_RST_PORT, BOARD_INITPINS_RST_PIN, kPORT_MuxAlt0);

  PORT1->PCR[3] = ((PORT1->PCR[3] &
                    /* Mask bits to zero which are setting */
                    (~(PORT_PCR_PS_MASK | PORT_PCR_IBE_MASK)))

                  /* Pull Select: Enables internal pullup resistor. */
                  | PORT_PCR_PS(pol_default)

                  | PORT_PCR_PE_MASK

                  /* Input Buffer Enable: Enables. */
                  | PORT_PCR_IBE(PCR_IBE_ibe1));	
}
