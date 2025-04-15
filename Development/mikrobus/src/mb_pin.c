/**
* @brief   FRDM-MCXN947 board mikroBUS header port initialization
* @author  masa
* @version 1.00
*/

#include "mb_pin.h"

static void lpspi6_InitPins(void)
{
   /* Enables the clock for GPIO1: Enables clock */
   CLOCK_EnableClock(kCLOCK_Gpio1);
   /* Enables the clock for PORT1: Enables clock */
   CLOCK_EnableClock(kCLOCK_Port1);
   /* Enables the clock for PORT3: Enables clock */
   CLOCK_EnableClock(kCLOCK_Port3);

   CLOCK_EnableClock(kCLOCK_Gpio3);

   gpio_pin_config_t RST_config = {
       .pinDirection = kGPIO_DigitalOutput,
       .outputLogic = 0U
   };
   /* Initialize GPIO functionality on pin PIO1_3 (pin B4)  */
   GPIO_PinInit(BOARD_INITPINS_RST_GPIO, BOARD_INITPINS_RST_PIN, &RST_config);

   gpio_pin_config_t INT_config = {
       .pinDirection = kGPIO_DigitalOutput,
       .outputLogic = 0U
   };
   /* Initialize GPIO functionality on pin PIO5_7 (pin L13)  */
   GPIO_PinInit(BOARD_INITPINS_INT_GPIO, BOARD_INITPINS_INT_PIN, &INT_config);

   /* PORT1_3 (pin B4) is configured as PIO1_3, WUU0_IN7 */
   PORT_SetPinMux(BOARD_INITPINS_RST_PORT, BOARD_INITPINS_RST_PIN, kPORT_MuxAlt0);

   PORT1->PCR[3] = ((PORT1->PCR[3] &
                     /* Mask bits to zero which are setting */
                     (~(PORT_PCR_PS_MASK | PORT_PCR_IBE_MASK)))

                    /* Pull Select: Enables internal pullup resistor. */
                    | PORT_PCR_PS(0)

                    //| PORT_PCR_PE_MASK

                    /* Input Buffer Enable: Enables. */
                    | PORT_PCR_IBE(PCR_IBE_ibe1));

   const port_pin_config_t DEBUG_UART_RX = {/* Internal pull-up/down resistor is disabled */
                                            .pullSelect = kPORT_PullDisable,
                                            /* Low internal pull resistor value is selected. */
                                            .pullValueSelect = kPORT_LowPullResistor,
                                            /* Fast slew rate is configured */
                                            .slewRate = kPORT_FastSlewRate,
                                            /* Passive input filter is disabled */
                                            .passiveFilterEnable = kPORT_PassiveFilterDisable,
                                            /* Open drain output is disabled */
                                            .openDrainEnable = kPORT_OpenDrainDisable,
                                            /* Low drive strength is configured */
                                            .driveStrength = kPORT_LowDriveStrength,
                                            /* Pin is configured as FC4_P0 */
                                            .mux = kPORT_MuxAlt2,
                                            /* Digital input enabled */
                                            .inputBuffer = kPORT_InputBufferEnable,
                                            /* Digital input is not inverted */
                                            .invertInput = kPORT_InputNormal,
                                            /* Pin Control Register fields [15:0] are not locked */
                                            .lockRegister = kPORT_UnlockRegister};
   /* PORT1_8 (pin A1) is configured as FC4_P0 */
   PORT_SetPinConfig(BOARD_INITPINS_DEBUG_UART_RX_PORT, BOARD_INITPINS_DEBUG_UART_RX_PIN, &DEBUG_UART_RX);

   const port_pin_config_t DEBUG_UART_TX = {/* Internal pull-up/down resistor is disabled */
                                            .pullSelect = kPORT_PullDisable,
                                            /* Low internal pull resistor value is selected. */
                                            .pullValueSelect = kPORT_LowPullResistor,
                                            /* Fast slew rate is configured */
                                            .slewRate = kPORT_FastSlewRate,
                                            /* Passive input filter is disabled */
                                            .passiveFilterEnable = kPORT_PassiveFilterDisable,
                                            /* Open drain output is disabled */
                                            .openDrainEnable = kPORT_OpenDrainDisable,
                                            /* Low drive strength is configured */
                                            .driveStrength = kPORT_LowDriveStrength,
                                            /* Pin is configured as FC4_P1 */
                                            .mux = kPORT_MuxAlt2,
                                            /* Digital input enabled */
                                            .inputBuffer = kPORT_InputBufferEnable,
                                            /* Digital input is not inverted */
                                            .invertInput = kPORT_InputNormal,
                                            /* Pin Control Register fields [15:0] are not locked */
                                            .lockRegister = kPORT_UnlockRegister};
   /* PORT1_9 (pin B1) is configured as FC4_P1 */
   PORT_SetPinConfig(BOARD_INITPINS_DEBUG_UART_TX_PORT, BOARD_INITPINS_DEBUG_UART_TX_PIN, &DEBUG_UART_TX);

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

void InitMikroBusPort(mikrobus_hdr_t hdr)
{
  switch(hdr){
    case(DEFUALT_MIKROBUS):{
      lpspi6_InitPins();
      break;
    }
  }
}
