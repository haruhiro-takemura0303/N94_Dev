/**
* @brief   FRDM-MCXN947 board mikroBUS header port initialization
* @author  masa
* @version 1.00
*/
#ifndef __MB_PIN_H__
#define __MB_PIN_H__

#include "fsl_port.h"
#include "fsl_gpio.h"

#define PCR_IBE_ibe1 0x01u        /*!<@brief Input Buffer Enable: Enables */
#define PCR_PS_ps1 0x01u          /*!<@brief Pull Select: Enables internal pullup resistor */
#define PORT5_PCR_MUX_mux00 0x00u /*!<@brief Pin Multiplex Control: Alternative 0 (GPIO) */

/*! @name PORT1_8 (coord A1), P1_8/J9[32]
  @{ */

/* Symbols to be used with PORT driver */
#define BOARD_INITPINS_DEBUG_UART_RX_PORT PORT1               /*!<@brief PORT peripheral base pointer */
#define BOARD_INITPINS_DEBUG_UART_RX_PIN 8U                   /*!<@brief PORT pin number */
#define BOARD_INITPINS_DEBUG_UART_RX_PIN_MASK (1U << 8U)      /*!<@brief PORT pin mask */
                                                              /* @} */

/*! @name PORT1_9 (coord B1), P1_9/J9[30]
  @{ */

/* Symbols to be used with PORT driver */
#define BOARD_INITPINS_DEBUG_UART_TX_PORT PORT1               /*!<@brief PORT peripheral base pointer */
#define BOARD_INITPINS_DEBUG_UART_TX_PIN 9U                   /*!<@brief PORT pin number */
#define BOARD_INITPINS_DEBUG_UART_TX_PIN_MASK (1U << 9U)      /*!<@brief PORT pin mask */
                                                              /* @} */

/*! @name PORT5_7 (coord L13), P5_7/J5[2]
  @{ */

/* Symbols to be used with GPIO driver */
#define BOARD_INITPINS_INT_GPIO GPIO5               /*!<@brief GPIO peripheral base pointer */
#define BOARD_INITPINS_INT_INIT_GPIO_VALUE 0U       /*!<@brief GPIO output initial state */
#define BOARD_INITPINS_INT_GPIO_PIN 7U              /*!<@brief GPIO pin number */
#define BOARD_INITPINS_INT_GPIO_PIN_MASK (1U << 7U) /*!<@brief GPIO pin mask */

/* Symbols to be used with PORT driver */
#define BOARD_INITPINS_INT_PORT PORT5               /*!<@brief PORT peripheral base pointer */
#define BOARD_INITPINS_INT_PIN 7U                   /*!<@brief PORT pin number */
#define BOARD_INITPINS_INT_PIN_MASK (1U << 7U)      /*!<@brief PORT pin mask */
                                                    /* @} */

/*! @name PORT1_3 (coord B4), P1_3/J6[2]
  @{ */

/* Symbols to be used with GPIO driver */
#define BOARD_INITPINS_RST_GPIO GPIO1               /*!<@brief GPIO peripheral base pointer */
#define BOARD_INITPINS_RST_INIT_GPIO_VALUE 0U       /*!<@brief GPIO output initial state */
#define BOARD_INITPINS_RST_GPIO_PIN 3U              /*!<@brief GPIO pin number */
#define BOARD_INITPINS_RST_GPIO_PIN_MASK (1U << 3U) /*!<@brief GPIO pin mask */

/* Symbols to be used with PORT driver */
#define BOARD_INITPINS_RST_PORT PORT1               /*!<@brief PORT peripheral base pointer */
#define BOARD_INITPINS_RST_PIN 3U                   /*!<@brief PORT pin number */
#define BOARD_INITPINS_RST_PIN_MASK (1U << 3U)      /*!<@brief PORT pin mask */
                                                    /* @} */

typedef enum{
  DEFUALT_MIKROBUS,
} mikrobus_hdr_t;

void InitMikroBusPort(mikrobus_hdr_t hdr);

#endif /*__MB_PIN_H__*/
