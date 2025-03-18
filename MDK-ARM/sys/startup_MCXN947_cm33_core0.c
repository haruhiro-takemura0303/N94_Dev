/**************************************************************************//**
 * @file     startup_MCXN947_cm33_core0.c
 * @version  V1.00
 * @brief    CMSIS Device Startup File for MCX-N947V
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2025 Roland Corp. All rights reserved.
 *****************************************************************************/

 #include <inttypes.h>
 #include <stdio.h>
 #include "fsl_device_registers.h"
 #include "cmsis_armclang.h"
 
 /*----------------------------------------------------------------------------
   External References
  *----------------------------------------------------------------------------*/
 extern uint32_t __INITIAL_SP;
 extern uint32_t __STACK_LIMIT;
 #if defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
     extern uint32_t __STACK_SEAL;
 #endif
 extern void SystemInit(void);
 extern __NO_RETURN void __PROGRAM_START(void);
 
 
 /*----------------------------------------------------------------------------
   Internal References
  *----------------------------------------------------------------------------*/
 __NO_RETURN void Reset_Handler(void);
 void Default_Handler(void);
 
 /*----------------------------------------------------------------------------
   Type Definitions
  *----------------------------------------------------------------------------*/
 typedef void(*VECTOR_TABLE_Type)(void);
 
 /*----------------------------------------------------------------------------
   Exception / Interrupt Handler
  *----------------------------------------------------------------------------*/
 /* Exceptions */
 void NMI_Handler(void) __attribute__((weak, alias("Default_Handler")));
 void HardFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
 void MemManage_Handler(void) __attribute__((weak, alias("Default_Handler")));
 void BusFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
 void UsageFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
 void SecureFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
 void SVC_Handler(void) __attribute__((weak, alias("Default_Handler")));
 void DebugMon_Handler(void) __attribute__((weak, alias("Default_Handler")));
 void PendSV_Handler(void) __attribute__((weak, alias("Default_Handler")));
 void SysTick_Handler(void) __attribute__((weak, alias("Default_Handler")));
 
 void OR_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                                   /* OR IRQ*/
 void EDMA_0_CH0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                           /* eDMA_0_CH0 error or transfer complete*/
 void EDMA_0_CH1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                           /* eDMA_0_CH1 error or transfer complete*/
 void EDMA_0_CH2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                           /* eDMA_0_CH2 error or transfer complete*/
 void EDMA_0_CH3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                           /* eDMA_0_CH3 error or transfer complete*/
 void EDMA_0_CH4_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                           /* eDMA_0_CH4 error or transfer complete*/
 void EDMA_0_CH5_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                           /* eDMA_0_CH5 error or transfer complete*/
 void EDMA_0_CH6_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                           /* eDMA_0_CH6 error or transfer complete*/
 void EDMA_0_CH7_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                           /* eDMA_0_CH7 error or transfer complete*/
 void EDMA_0_CH8_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                           /* eDMA_0_CH8 error or transfer complete*/
 void EDMA_0_CH9_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                           /* eDMA_0_CH9 error or transfer complete*/
 void EDMA_0_CH10_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                          /* eDMA_0_CH10 error or transfer complete*/
 void EDMA_0_CH11_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                          /* eDMA_0_CH11 error or transfer complete*/
 void EDMA_0_CH12_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                          /* eDMA_0_CH12 error or transfer complete*/
 void EDMA_0_CH13_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                          /* eDMA_0_CH13 error or transfer complete*/
 void EDMA_0_CH14_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                          /* eDMA_0_CH14 error or transfer complete*/
 void EDMA_0_CH15_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                          /* eDMA_0_CH15 error or transfer complete*/
 void GPIO00_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                               /* GPIO0 interrupt 0*/
 void GPIO01_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                               /* GPIO0 interrupt 1*/
 void GPIO10_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                               /* GPIO1 interrupt 0*/
 void GPIO11_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                               /* GPIO1 interrupt 1*/
 void GPIO20_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                               /* GPIO2 interrupt 0*/
 void GPIO21_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                               /* GPIO2 interrupt 1*/
 void GPIO30_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                               /* GPIO3 interrupt 0*/
 void GPIO31_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                               /* GPIO3 interrupt 1*/
 void GPIO40_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                               /* GPIO4 interrupt 0*/
 void GPIO41_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                               /* GPIO4 interrupt 1*/
 void GPIO50_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                               /* GPIO5 interrupt 0*/
 void GPIO51_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                               /* GPIO5 interrupt 1*/
 void UTICK0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                               /* Micro-Tick Timer interrupt*/
 void MRT0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                                 /* Multi-Rate Timer interrupt*/
 void CTIMER0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                              /* Standard counter/timer 0 interrupt*/
 void CTIMER1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                              /* Standard counter/timer 1 interrupt*/
 void SCT0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                                 /* SCTimer/PWM interrupt*/
 void CTIMER2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                              /* Standard counter/timer 2 interrupt*/
 void LP_FLEXCOMM0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                         /* LP_FLEXCOMM0 (LPSPI interrupt or LPI2C interrupt or LPUART Receive/Transmit interrupt)*/
 void LP_FLEXCOMM1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                         /* LP_FLEXCOMM1 (LPSPI interrupt or LPI2C interrupt or LPUART Receive/Transmit interrupt)*/
 void LP_FLEXCOMM2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                         /* LP_FLEXCOMM2 (LPSPI interrupt or LPI2C interrupt or LPUART Receive/Transmit interrupt)*/
 void LP_FLEXCOMM3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                         /* LP_FLEXCOMM3 (LPSPI interrupt or LPI2C interrupt or LPUART Receive/Transmit interrupt)*/
 void LP_FLEXCOMM4_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                         /* LP_FLEXCOMM4 (LPSPI interrupt or LPI2C interrupt or LPUART Receive/Transmit interrupt)*/
 void LP_FLEXCOMM5_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                         /* LP_FLEXCOMM5 (LPSPI interrupt or LPI2C interrupt or LPUART Receive/Transmit interrupt)*/
 void LP_FLEXCOMM6_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                         /* LP_FLEXCOMM6 (LPSPI interrupt or LPI2C interrupt or LPUART Receive/Transmit interrupt)*/
 void LP_FLEXCOMM7_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                         /* LP_FLEXCOMM7 (LPSPI interrupt or LPI2C interrupt or LPUART Receive/Transmit interrupt)*/
 void LP_FLEXCOMM8_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                         /* LP_FLEXCOMM8 (LPSPI interrupt or LPI2C interrupt or LPUART Receive/Transmit interrupt)*/
 void LP_FLEXCOMM9_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                         /* LP_FLEXCOMM9 (LPSPI interrupt or LPI2C interrupt or LPUART Receive/Transmit interrupt)*/
 void ADC0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                                 /* Analog-to-Digital Converter 0 - General Purpose interrupt*/
 void ADC1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                                 /* Analog-to-Digital Converter 1 - General Purpose interrupt*/
 void PINT0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                                /* Pin Interrupt Pattern Match Interrupt*/
 void PDM_EVENT_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                            /* Microphone Interface interrupt*/
 void Reserved65_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                           /* Reserved interrupt*/
 void USB0_FS_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                              /* Universal Serial Bus - Full Speed interrupt*/
 void USB0_DCD_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                             /* Universal Serial Bus - Device Charge Detect interrupt*/
 void RTC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                                  /* RTC Subsystem interrupt (RTC interrupt or Wake timer interrupt)*/
 void SMARTDMA_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                             /* SmartDMA_IRQ*/
 void MAILBOX_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                              /* Inter-CPU Mailbox interrupt0 for CPU0 Inter-CPU Mailbox interrupt1 for CPU1*/
 void CTIMER3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                              /* Standard counter/timer 3 interrupt*/
 void CTIMER4_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                              /* Standard counter/timer 4 interrupt*/
 void OS_EVENT_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                             /* OS event timer interrupt*/
 void FLEXSPI0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                             /* Flexible Serial Peripheral Interface interrupt*/
 void SAI0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                                 /* Serial Audio Interface 0 interrupt*/
 void SAI1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                                 /* Serial Audio Interface 1 interrupt*/
 void USDHC0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                               /* Ultra Secured Digital Host Controller interrupt*/
 void CAN0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                                 /* Controller Area Network 0 interrupt*/
 void CAN1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                                 /* Controller Area Network 1 interrupt*/
 void Reserved80_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                           /* Reserved interrupt*/
 void Reserved81_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                           /* Reserved interrupt*/
 void USB1_HS_PHY_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                          /* USBHS DCD or USBHS Phy interrupt*/
 void USB1_HS_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                              /* USB High Speed OTG Controller interrupt */
 void SEC_HYPERVISOR_CALL_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                  /* AHB Secure Controller hypervisor call interrupt*/
 void Reserved85_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                           /* Reserved interrupt*/
 void PLU_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                                  /* Programmable Logic Unit interrupt*/
 void Freqme_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                               /* Frequency Measurement interrupt*/
 void SEC_VIO_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                              /* Secure violation interrupt (Memory Block Checker interrupt or secure AHB matrix violation interrupt)*/
 void ELS_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                                  /* ELS interrupt*/
 void PKC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                                  /* PKC interrupt*/
 void PUF_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                                  /* Physical Unclonable Function interrupt*/
 void PQ_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                                   /* Power Quad interrupt*/
 void EDMA_1_CH0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                           /* eDMA_1_CH0 error or transfer complete*/
 void EDMA_1_CH1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                           /* eDMA_1_CH1 error or transfer complete*/
 void EDMA_1_CH2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                           /* eDMA_1_CH2 error or transfer complete*/
 void EDMA_1_CH3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                           /* eDMA_1_CH3 error or transfer complete*/
 void EDMA_1_CH4_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                           /* eDMA_1_CH4 error or transfer complete*/
 void EDMA_1_CH5_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                           /* eDMA_1_CH5 error or transfer complete*/
 void EDMA_1_CH6_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                           /* eDMA_1_CH6 error or transfer complete*/
 void EDMA_1_CH7_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                           /* eDMA_1_CH7 error or transfer complete*/
 void EDMA_1_CH8_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                           /* eDMA_1_CH8 error or transfer complete*/
 void EDMA_1_CH9_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                           /* eDMA_1_CH9 error or transfer complete*/
 void EDMA_1_CH10_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                          /* eDMA_1_CH10 error or transfer complete*/
 void EDMA_1_CH11_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                          /* eDMA_1_CH11 error or transfer complete*/
 void EDMA_1_CH12_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                          /* eDMA_1_CH12 error or transfer complete*/
 void EDMA_1_CH13_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                          /* eDMA_1_CH13 error or transfer complete*/
 void EDMA_1_CH14_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                          /* eDMA_1_CH14 error or transfer complete*/
 void EDMA_1_CH15_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                          /* eDMA_1_CH15 error or transfer complete*/
 void CDOG0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                                /* Code Watchdog Timer 0 interrupt*/
 void CDOG1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                                /* Code Watchdog Timer 1 interrupt*/
 void I3C0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                                 /* Improved Inter Integrated Circuit interrupt 0*/
 void I3C1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                                 /* Improved Inter Integrated Circuit interrupt 1*/
 void NPU_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                                  /* NPU interrupt*/
 void GDET_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                                 /* Digital Glitch Detect 0 interrupt  or Digital Glitch Detect 1 interrupt*/
 void VBAT0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                                /* VBAT interrupt( VBAT interrupt or digital tamper interrupt)*/
 void EWM0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                                 /* External Watchdog Monitor interrupt*/
 void TSI_END_OF_SCAN_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                      /* TSI End of Scan interrupt*/
 void TSI_OUT_OF_SCAN_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                      /* TSI Out of Scan interrupt*/
 void EMVSIM0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                              /* EMVSIM0 interrupt*/
 void EMVSIM1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                              /* EMVSIM1 interrupt*/
 void FLEXIO_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                               /* Flexible Input/Output interrupt*/
 void DAC0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                                 /* Digital-to-Analog Converter 0 - General Purpose interrupt*/
 void DAC1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                                 /* Digital-to-Analog Converter 1 - General Purpose interrupt*/
 void DAC2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                                 /* 14-bit Digital-to-Analog Converter interrupt*/
 void HSCMP0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                               /* High-Speed comparator0 interrupt*/
 void HSCMP1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                               /* High-Speed comparator1 interrupt*/
 void HSCMP2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                               /* High-Speed comparator2 interrupt*/
 void FLEXPWM0_RELOAD_ERROR_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                /* FlexPWM0_reload_error interrupt*/
 void FLEXPWM0_FAULT_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                       /* FlexPWM0_fault interrupt*/
 void FLEXPWM0_SUBMODULE0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                  /* FlexPWM0 Submodule 0 capture/compare/reload interrupt*/
 void FLEXPWM0_SUBMODULE1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                  /* FlexPWM0 Submodule 1 capture/compare/reload interrupt*/
 void FLEXPWM0_SUBMODULE2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                  /* FlexPWM0 Submodule 2 capture/compare/reload interrupt*/
 void FLEXPWM0_SUBMODULE3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                  /* FlexPWM0 Submodule 3 capture/compare/reload interrupt*/
 void FLEXPWM1_RELOAD_ERROR_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                /* FlexPWM1_reload_error interrupt*/
 void FLEXPWM1_FAULT_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                       /* FlexPWM1_fault interrupt*/
 void FLEXPWM1_SUBMODULE0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                  /* FlexPWM1 Submodule 0 capture/compare/reload interrupt*/
 void FLEXPWM1_SUBMODULE1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                  /* FlexPWM1 Submodule 1 capture/compare/reload interrupt*/
 void FLEXPWM1_SUBMODULE2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                  /* FlexPWM1 Submodule 2 capture/compare/reload interrupt*/
 void FLEXPWM1_SUBMODULE3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                  /* FlexPWM1 Submodule 3 capture/compare/reload interrupt*/
 void QDC0_COMPARE_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                         /* QDC0_Compare interrupt*/
 void QDC0_HOME_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                            /* QDC0_Home interrupt*/
 void QDC0_WDG_SAB_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                         /* QDC0_WDG_IRQ/SAB interrupt*/
 void QDC0_IDX_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                             /* QDC0_IDX interrupt*/
 void QDC1_COMPARE_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                         /* QDC1_Compare interrupt*/
 void QDC1_HOME_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                            /* QDC1_Home interrupt*/
 void QDC1_WDG_SAB_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                         /* QDC1_WDG_IRQ/SAB interrupt*/
 void QDC1_IDX_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                             /* QDC1_IDX interrupt*/
 void ITRC0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                                /* Intrusion and Tamper Response Controller interrupt*/
 void BSP32_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                                /* CoolFlux BSP32 interrupt*/
 void ELS_ERR_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                              /* ELS error interrupt*/
 void PKC_ERR_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                              /* PKC error interrupt*/
 void ERM_SINGLE_BIT_ERROR_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                 /* ERM Single Bit error interrupt*/
 void ERM_MULTI_BIT_ERROR_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                  /* ERM Multi Bit error interrupt*/
 void FMU0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                                 /* Flash Management Unit interrupt*/
 void ETHERNET_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                             /* Ethernet QoS interrupt*/
 void ETHERNET_PMT_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                         /* Ethernet QoS power management interrupt*/
 void ETHERNET_MACLP_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                       /* Ethernet QoS MAC interrupt*/
 void SINC_FILTER_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                          /* SINC Filter interrupt */
 void LPTMR0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                               /* Low Power Timer 0 interrupt*/
 void LPTMR1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                               /* Low Power Timer 1 interrupt*/
 void SCG_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                                  /* System Clock Generator interrupt*/
 void SPC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                                  /* System Power Controller interrupt*/
 void WUU_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                                  /* Wake Up Unit interrupt*/
 void PORT_EFT_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                             /* PORT0~5 EFT interrupt*/
 void ETB0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                                 /* ETB counter expires interrupt*/
 void Reserved166_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                          /* Reserved interrupt*/
 void Reserved167_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                          /* Reserved interrupt*/
 void WWDT0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                                /* Windowed Watchdog Timer 0 interrupt*/
 void WWDT1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                                /* Windowed Watchdog Timer 1 interrupt*/
 void CMC0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                                 /* Core Mode Controller interrupt*/
 void CTI0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));                                 /* Cross Trigger Interface interrupt*/


 /*----------------------------------------------------------------------------
   Exception / Interrupt Vector table
  *----------------------------------------------------------------------------*/
 #if defined ( __GNUC__ )
     #pragma GCC diagnostic push
     #pragma GCC diagnostic ignored "-Wpedantic"
 #endif
 
 /* Static vector table
  * For performance, M55M1 places vector table in DTCM by default.
  * User can define NVT_VECTOR_ON_FLASH to place vector table in Flash.
  *
  * If NVT_VECTOR_ON_FLASH is defined and use IAR,
  *   IRQ handlers referenced in __VECTOR_TABLE (__vector_table) are protected and
  *   not affected by 'initialize by copy'. It means IRQ handler must placed in Flash.
  */
 const VECTOR_TABLE_Type __VECTOR_TABLE[] __VECTOR_TABLE_ATTRIBUTE =
 {
     (VECTOR_TABLE_Type)(&__INITIAL_SP),       /*       Initial Stack Pointer                            */
     Reset_Handler,                            /*       Reset Handler                                    */
     NMI_Handler,                              /*   -14 NMI Handler                                      */
     HardFault_Handler,                        /*   -13 Hard Fault Handler                               */
     MemManage_Handler,                        /*   -12 MPU Fault Handler                                */
     BusFault_Handler,                         /*   -11 Bus Fault Handler                                */
     UsageFault_Handler,                       /*   -10 Usage Fault Handler                              */
     SecureFault_Handler,                      /*    -9 Secure Fault Handler                             */
     0,                                        /*       Reserved                                         */
     0,                                        /*       Reserved                                         */
     0,                                        /*       Reserved                                         */
     SVC_Handler,                              /*    -5 SVC Handler                                      */
     DebugMon_Handler,                         /*    -4 Debug Monitor Handler                            */
     0,                                        /*       Reserved                                         */
     PendSV_Handler,                           /*    -2 PendSV Handler Handler                           */
     SysTick_Handler,                          /*    -1 SysTick Handler                                  */
 
     /* Interrupts */
     OR_IRQHandler,                                   /* OR IRQ*/
     EDMA_0_CH0_IRQHandler,                           /* eDMA_0_CH0 error or transfer complete*/
     EDMA_0_CH1_IRQHandler,                           /* eDMA_0_CH1 error or transfer complete*/
     EDMA_0_CH2_IRQHandler,                           /* eDMA_0_CH2 error or transfer complete*/
     EDMA_0_CH3_IRQHandler,                           /* eDMA_0_CH3 error or transfer complete*/
     EDMA_0_CH4_IRQHandler,                           /* eDMA_0_CH4 error or transfer complete*/
     EDMA_0_CH5_IRQHandler,                           /* eDMA_0_CH5 error or transfer complete*/
     EDMA_0_CH6_IRQHandler,                           /* eDMA_0_CH6 error or transfer complete*/
     EDMA_0_CH7_IRQHandler,                           /* eDMA_0_CH7 error or transfer complete*/
     EDMA_0_CH8_IRQHandler,                           /* eDMA_0_CH8 error or transfer complete*/
     EDMA_0_CH9_IRQHandler,                           /* eDMA_0_CH9 error or transfer complete*/
     EDMA_0_CH10_IRQHandler,                          /* eDMA_0_CH10 error or transfer complete*/
     EDMA_0_CH11_IRQHandler,                          /* eDMA_0_CH11 error or transfer complete*/
     EDMA_0_CH12_IRQHandler,                          /* eDMA_0_CH12 error or transfer complete*/
     EDMA_0_CH13_IRQHandler,                          /* eDMA_0_CH13 error or transfer complete*/
     EDMA_0_CH14_IRQHandler,                          /* eDMA_0_CH14 error or transfer complete*/
     EDMA_0_CH15_IRQHandler,                          /* eDMA_0_CH15 error or transfer complete*/
     GPIO00_IRQHandler,                               /* GPIO0 interrupt 0*/
     GPIO01_IRQHandler,                               /* GPIO0 interrupt 1*/
     GPIO10_IRQHandler,                               /* GPIO1 interrupt 0*/
     GPIO11_IRQHandler,                               /* GPIO1 interrupt 1*/
     GPIO20_IRQHandler,                               /* GPIO2 interrupt 0*/
     GPIO21_IRQHandler,                               /* GPIO2 interrupt 1*/
     GPIO30_IRQHandler,                               /* GPIO3 interrupt 0*/
     GPIO31_IRQHandler,                               /* GPIO3 interrupt 1*/
     GPIO40_IRQHandler,                               /* GPIO4 interrupt 0*/
     GPIO41_IRQHandler,                               /* GPIO4 interrupt 1*/
     GPIO50_IRQHandler,                               /* GPIO5 interrupt 0*/
     GPIO51_IRQHandler,                               /* GPIO5 interrupt 1*/
     UTICK0_IRQHandler,                               /* Micro-Tick Timer interrupt*/
     MRT0_IRQHandler,                                 /* Multi-Rate Timer interrupt*/
     CTIMER0_IRQHandler,                              /* Standard counter/timer 0 interrupt*/
     CTIMER1_IRQHandler,                              /* Standard counter/timer 1 interrupt*/
     SCT0_IRQHandler,                                 /* SCTimer/PWM interrupt*/
     CTIMER2_IRQHandler,                              /* Standard counter/timer 2 interrupt*/
     LP_FLEXCOMM0_IRQHandler,                         /* LP_FLEXCOMM0 (LPSPI interrupt or LPI2C interrupt or LPUART Receive/Transmit interrupt)*/
     LP_FLEXCOMM1_IRQHandler,                         /* LP_FLEXCOMM1 (LPSPI interrupt or LPI2C interrupt or LPUART Receive/Transmit interrupt)*/
     LP_FLEXCOMM2_IRQHandler,                         /* LP_FLEXCOMM2 (LPSPI interrupt or LPI2C interrupt or LPUART Receive/Transmit interrupt)*/
     LP_FLEXCOMM3_IRQHandler,                         /* LP_FLEXCOMM3 (LPSPI interrupt or LPI2C interrupt or LPUART Receive/Transmit interrupt)*/
     LP_FLEXCOMM4_IRQHandler,                         /* LP_FLEXCOMM4 (LPSPI interrupt or LPI2C interrupt or LPUART Receive/Transmit interrupt)*/
     LP_FLEXCOMM5_IRQHandler,                         /* LP_FLEXCOMM5 (LPSPI interrupt or LPI2C interrupt or LPUART Receive/Transmit interrupt)*/
     LP_FLEXCOMM6_IRQHandler,                         /* LP_FLEXCOMM6 (LPSPI interrupt or LPI2C interrupt or LPUART Receive/Transmit interrupt)*/
     LP_FLEXCOMM7_IRQHandler,                         /* LP_FLEXCOMM7 (LPSPI interrupt or LPI2C interrupt or LPUART Receive/Transmit interrupt)*/
     LP_FLEXCOMM8_IRQHandler,                         /* LP_FLEXCOMM8 (LPSPI interrupt or LPI2C interrupt or LPUART Receive/Transmit interrupt)*/
     LP_FLEXCOMM9_IRQHandler,                         /* LP_FLEXCOMM9 (LPSPI interrupt or LPI2C interrupt or LPUART Receive/Transmit interrupt)*/
     ADC0_IRQHandler,                                 /* Analog-to-Digital Converter 0 - General Purpose interrupt*/
     ADC1_IRQHandler,                                 /* Analog-to-Digital Converter 1 - General Purpose interrupt*/
     PINT0_IRQHandler,                                /* Pin Interrupt Pattern Match Interrupt*/
     PDM_EVENT_IRQHandler,                            /* Microphone Interface interrupt*/
     Reserved65_IRQHandler,                           /* Reserved interrupt*/
     USB0_FS_IRQHandler,                              /* Universal Serial Bus - Full Speed interrupt*/
     USB0_DCD_IRQHandler,                             /* Universal Serial Bus - Device Charge Detect interrupt*/
     RTC_IRQHandler,                                  /* RTC Subsystem interrupt (RTC interrupt or Wake timer interrupt)*/
     SMARTDMA_IRQHandler,                             /* SmartDMA_IRQ*/
     MAILBOX_IRQHandler,                              /* Inter-CPU Mailbox interrupt0 for CPU0 Inter-CPU Mailbox interrupt1 for CPU1*/
     CTIMER3_IRQHandler,                              /* Standard counter/timer 3 interrupt*/
     CTIMER4_IRQHandler,                              /* Standard counter/timer 4 interrupt*/
     OS_EVENT_IRQHandler,                             /* OS event timer interrupt*/
     FLEXSPI0_IRQHandler,                             /* Flexible Serial Peripheral Interface interrupt*/
     SAI0_IRQHandler,                                 /* Serial Audio Interface 0 interrupt*/
     SAI1_IRQHandler,                                 /* Serial Audio Interface 1 interrupt*/
     USDHC0_IRQHandler,                               /* Ultra Secured Digital Host Controller interrupt*/
     CAN0_IRQHandler,                                 /* Controller Area Network 0 interrupt*/
     CAN1_IRQHandler,                                 /* Controller Area Network 1 interrupt*/
     Reserved80_IRQHandler,                           /* Reserved interrupt*/
     Reserved81_IRQHandler,                           /* Reserved interrupt*/
     USB1_HS_PHY_IRQHandler,                          /* USBHS DCD or USBHS Phy interrupt*/
     USB1_HS_IRQHandler,                              /* USB High Speed OTG Controller interrupt */
     SEC_HYPERVISOR_CALL_IRQHandler,                  /* AHB Secure Controller hypervisor call interrupt*/
     Reserved85_IRQHandler,                           /* Reserved interrupt*/
     PLU_IRQHandler,                                  /* Programmable Logic Unit interrupt*/
     Freqme_IRQHandler,                               /* Frequency Measurement interrupt*/
     SEC_VIO_IRQHandler,                              /* Secure violation interrupt (Memory Block Checker interrupt or secure AHB matrix violation interrupt)*/
     ELS_IRQHandler,                                  /* ELS interrupt*/
     PKC_IRQHandler,                                  /* PKC interrupt*/
     PUF_IRQHandler,                                  /* Physical Unclonable Function interrupt*/
     PQ_IRQHandler,                                   /* Power Quad interrupt*/
     EDMA_1_CH0_IRQHandler,                           /* eDMA_1_CH0 error or transfer complete*/
     EDMA_1_CH1_IRQHandler,                           /* eDMA_1_CH1 error or transfer complete*/
     EDMA_1_CH2_IRQHandler,                           /* eDMA_1_CH2 error or transfer complete*/
     EDMA_1_CH3_IRQHandler,                           /* eDMA_1_CH3 error or transfer complete*/
     EDMA_1_CH4_IRQHandler,                           /* eDMA_1_CH4 error or transfer complete*/
     EDMA_1_CH5_IRQHandler,                           /* eDMA_1_CH5 error or transfer complete*/
     EDMA_1_CH6_IRQHandler,                           /* eDMA_1_CH6 error or transfer complete*/
     EDMA_1_CH7_IRQHandler,                           /* eDMA_1_CH7 error or transfer complete*/
     EDMA_1_CH8_IRQHandler,                           /* eDMA_1_CH8 error or transfer complete*/
     EDMA_1_CH9_IRQHandler,                           /* eDMA_1_CH9 error or transfer complete*/
     EDMA_1_CH10_IRQHandler,                          /* eDMA_1_CH10 error or transfer complete*/
     EDMA_1_CH11_IRQHandler,                          /* eDMA_1_CH11 error or transfer complete*/
     EDMA_1_CH12_IRQHandler,                          /* eDMA_1_CH12 error or transfer complete*/
     EDMA_1_CH13_IRQHandler,                          /* eDMA_1_CH13 error or transfer complete*/
     EDMA_1_CH14_IRQHandler,                          /* eDMA_1_CH14 error or transfer complete*/
     EDMA_1_CH15_IRQHandler,                          /* eDMA_1_CH15 error or transfer complete*/
     CDOG0_IRQHandler,                                /* Code Watchdog Timer 0 interrupt*/
     CDOG1_IRQHandler,                                /* Code Watchdog Timer 1 interrupt*/
     I3C0_IRQHandler,                                 /* Improved Inter Integrated Circuit interrupt 0*/
     I3C1_IRQHandler,                                 /* Improved Inter Integrated Circuit interrupt 1*/
     NPU_IRQHandler,                                  /* NPU interrupt*/
     GDET_IRQHandler,                                 /* Digital Glitch Detect 0 interrupt  or Digital Glitch Detect 1 interrupt*/
     VBAT0_IRQHandler,                                /* VBAT interrupt( VBAT interrupt or digital tamper interrupt)*/
     EWM0_IRQHandler,                                 /* External Watchdog Monitor interrupt*/
     TSI_END_OF_SCAN_IRQHandler,                      /* TSI End of Scan interrupt*/
     TSI_OUT_OF_SCAN_IRQHandler,                      /* TSI Out of Scan interrupt*/
     EMVSIM0_IRQHandler,                              /* EMVSIM0 interrupt*/
     EMVSIM1_IRQHandler,                              /* EMVSIM1 interrupt*/
     FLEXIO_IRQHandler,                               /* Flexible Input/Output interrupt*/
     DAC0_IRQHandler,                                 /* Digital-to-Analog Converter 0 - General Purpose interrupt*/
     DAC1_IRQHandler,                                 /* Digital-to-Analog Converter 1 - General Purpose interrupt*/
     DAC2_IRQHandler,                                 /* 14-bit Digital-to-Analog Converter interrupt*/
     HSCMP0_IRQHandler,                               /* High-Speed comparator0 interrupt*/
     HSCMP1_IRQHandler,                               /* High-Speed comparator1 interrupt*/
     HSCMP2_IRQHandler,                               /* High-Speed comparator2 interrupt*/
     FLEXPWM0_RELOAD_ERROR_IRQHandler,                /* FlexPWM0_reload_error interrupt*/
     FLEXPWM0_FAULT_IRQHandler,                       /* FlexPWM0_fault interrupt*/
     FLEXPWM0_SUBMODULE0_IRQHandler,                  /* FlexPWM0 Submodule 0 capture/compare/reload interrupt*/
     FLEXPWM0_SUBMODULE1_IRQHandler,                  /* FlexPWM0 Submodule 1 capture/compare/reload interrupt*/
     FLEXPWM0_SUBMODULE2_IRQHandler,                  /* FlexPWM0 Submodule 2 capture/compare/reload interrupt*/
     FLEXPWM0_SUBMODULE3_IRQHandler,                  /* FlexPWM0 Submodule 3 capture/compare/reload interrupt*/
     FLEXPWM1_RELOAD_ERROR_IRQHandler,                /* FlexPWM1_reload_error interrupt*/
     FLEXPWM1_FAULT_IRQHandler,                       /* FlexPWM1_fault interrupt*/
     FLEXPWM1_SUBMODULE0_IRQHandler,                  /* FlexPWM1 Submodule 0 capture/compare/reload interrupt*/
     FLEXPWM1_SUBMODULE1_IRQHandler,                  /* FlexPWM1 Submodule 1 capture/compare/reload interrupt*/
     FLEXPWM1_SUBMODULE2_IRQHandler,                  /* FlexPWM1 Submodule 2 capture/compare/reload interrupt*/
     FLEXPWM1_SUBMODULE3_IRQHandler,                  /* FlexPWM1 Submodule 3 capture/compare/reload interrupt*/
     QDC0_COMPARE_IRQHandler,                         /* QDC0_Compare interrupt*/
     QDC0_HOME_IRQHandler,                            /* QDC0_Home interrupt*/
     QDC0_WDG_SAB_IRQHandler,                         /* QDC0_WDG_IRQ/SAB interrupt*/
     QDC0_IDX_IRQHandler,                             /* QDC0_IDX interrupt*/
     QDC1_COMPARE_IRQHandler,                         /* QDC1_Compare interrupt*/
     QDC1_HOME_IRQHandler,                            /* QDC1_Home interrupt*/
     QDC1_WDG_SAB_IRQHandler,                         /* QDC1_WDG_IRQ/SAB interrupt*/
     QDC1_IDX_IRQHandler,                             /* QDC1_IDX interrupt*/
     ITRC0_IRQHandler,                                /* Intrusion and Tamper Response Controller interrupt*/
     BSP32_IRQHandler,                                /* CoolFlux BSP32 interrupt*/
     ELS_ERR_IRQHandler,                              /* ELS error interrupt*/
     PKC_ERR_IRQHandler,                              /* PKC error interrupt*/
     ERM_SINGLE_BIT_ERROR_IRQHandler,                 /* ERM Single Bit error interrupt*/
     ERM_MULTI_BIT_ERROR_IRQHandler,                  /* ERM Multi Bit error interrupt*/
     FMU0_IRQHandler,                                 /* Flash Management Unit interrupt*/
     ETHERNET_IRQHandler,                             /* Ethernet QoS interrupt*/
     ETHERNET_PMT_IRQHandler,                         /* Ethernet QoS power management interrupt*/
     ETHERNET_MACLP_IRQHandler,                       /* Ethernet QoS MAC interrupt*/
     SINC_FILTER_IRQHandler,                          /* SINC Filter interrupt */
     LPTMR0_IRQHandler,                               /* Low Power Timer 0 interrupt*/
     LPTMR1_IRQHandler,                               /* Low Power Timer 1 interrupt*/
     SCG_IRQHandler,                                  /* System Clock Generator interrupt*/
     SPC_IRQHandler,                                  /* System Power Controller interrupt*/
     WUU_IRQHandler,                                  /* Wake Up Unit interrupt*/
     PORT_EFT_IRQHandler,                             /* PORT0~5 EFT interrupt*/
     ETB0_IRQHandler,                                 /* ETB counter expires interrupt*/
     Reserved166_IRQHandler,                          /* Reserved interrupt*/
     Reserved167_IRQHandler,                          /* Reserved interrupt*/
     WWDT0_IRQHandler,                                /* Windowed Watchdog Timer 0 interrupt*/
     WWDT1_IRQHandler,                                /* Windowed Watchdog Timer 1 interrupt*/
     CMC0_IRQHandler,                                 /* Core Mode Controller interrupt*/
     CTI0_IRQHandler,                                 /* Cross Trigger Interface interrupt*/
 };
 
 #if defined ( __GNUC__ )
     #pragma GCC diagnostic pop
 #endif

 
 /*----------------------------------------------------------------------------
   Reset Handler called on controller reset
  *----------------------------------------------------------------------------*/
 
 __NO_RETURN void Reset_Handler(void)
 {
	 __set_PSP((uint32_t)(&__INITIAL_SP));
	 __set_MSPLIM((uint32_t)(&__STACK_LIMIT));
	 __set_PSPLIM((uint32_t)(&__STACK_LIMIT));
 
    SystemInit();
     __PROGRAM_START();      // Enter PreMain (C library entry point)
 }
 
 #if defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
     #pragma clang diagnostic push
     #pragma clang diagnostic ignored "-Wmissing-noreturn"
 #endif
 
 /*----------------------------------------------------------------------------
   Default Handler for Exceptions / Interrupts
  *----------------------------------------------------------------------------*/
 void Default_Handler(void)
 {
     while (1);
 }
 
 #if defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
     #pragma clang diagnostic pop
 #endif
 