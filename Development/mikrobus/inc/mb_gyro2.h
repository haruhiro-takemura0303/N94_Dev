
#ifndef __MB_GYRO2_H__
#define __MB_GYRO2_H__

#include "fsl_lpflexcomm.h"
#include "fsl_lpi2c.h"
#include "fsl_edma.h"
#include "mb_pin.h"

enum{
  kI2C_TransmitData                         = (0b000 << 8),
  kI2C_ReceiveData                          = (0b001 << 8),
  kI2C_GenStop                              = (0b010 << 8),
  kI2C_ReceiveDataDiscard                   = (0b011 << 8),
  kI2C_GenStartAndSendAddress               = (0b100 << 8),
  kI2C_GenStartAndSendAddressExpectNack     = (0b101 << 8),
  kI2C_GenStartAndSendAddressHS             = (0b110 << 8),
  kI2C_GenStartAndSendAddressExpectNackHS   = (0b111 << 8),
};

#define G2_I2C_FIFO_SIZE 32
#define G2_RX_FIFO_SIZE 32
#define G2_MAX_READ_BURST_SIZE 6

#define g2RxTask_VDIn   DAC1_IRQn
#define g2RxTask_VDIHandler DAC1_IRQHandler

enum{
  G2_WRITE = 0,
  G2_READ_1BYTE,
  G2_READ_2BYTE,
  G2_READ_3BYTE,
  G2_READ_4BYTE,
  G2_READ_5BYTE,
  G2_READ_6BYTE,
};

typedef union{
  struct{
      uint8_t regAddr;
      uint8_t regValue;
      uint8_t readSize;
      uint8_t rsvd;
  }FIELD;
  uint32_t WORD;
}g2_tx_t;

typedef union{
  struct{
    uint8_t firstReg;
    uint8_t readSize;
    uint8_t regValue[G2_MAX_READ_BURST_SIZE];
  }FIELD;
  uint32_t WORD[2];
}g2_rx_t;

typedef struct{
  const uint16_t stSlvAddr;
  uint16_t regAddr;
  union{
    struct{
      uint16_t regValue;
      uint16_t sp;
      uint16_t rsvd;
    }WRITE;
    struct{
      uint16_t srSlvAddr;
      uint16_t readCmd_Size;
      uint16_t sp;
    }READ;
  }RW;
}g2_txField_t;

typedef struct{
  g2_tx_t fifo[G2_I2C_FIFO_SIZE];
  uint8_t enqIdx;
  uint8_t deqIdx;
}g2_fifo_t;

typedef struct{
  g2_rx_t fifo[G2_RX_FIFO_SIZE];
  uint8_t enqIdx;
  uint8_t deqIdx;
}g2_rx_fifo_t;


// -------------------------------------------------------------- PUBLIC MACROS 
/**
 * \defgroup macros Macros
 * \{
 */

/**
 * \defgroup slave_addr Slave Address
 * \{
 */
#define GYRO2_ADDR0    0x20
#define GYRO2_ADDR1    0x21

/**
 * \defgroup regs Registers
 * \{
 */
#define GYRO2_STATUS           0x00
#define GYRO2_OUT_X_MSB        0x01
#define GYRO2_OUT_X_LSB        0x02
#define GYRO2_OUT_Y_MSB        0x03
#define GYRO2_OUT_Y_LSB        0x04
#define GYRO2_OUT_Z_MSB        0x05
#define GYRO2_OUT_Z_LSB        0x06
#define GYRO2_DR_STATUS        0x07
#define GYRO2_F_STATUS         0x08
#define GYRO2_F_SETUP          0x09
#define GYRO2_F_EVENT          0x0A
#define GYRO2_INT_SRC_FLAG     0x0B
#define GYRO2_WHO_AM_I         0x0C
#define GYRO2_CTRL_REG0        0x0D
#define GYRO2_CTRL_REG1        0x13

/**
 * \defgroup bits_start Bits start
 * \{
 */
#define  GYRO2_SCALE_0    0
#define  GYRO2_SCALE_1    1
#define  GYRO2_SCALE_2    2
#define  GYRO2_SCALE_3    3

#define  GYRO2_HI_PASS_OFF     0x00
#define  GYRO2_HI_PASS_MOD0    0x04
#define  GYRO2_HI_PASS_MOD1    0x0C
#define  GYRO2_HI_PASS_MOD2    0x14
#define  GYRO2_HI_PASS_MOD3    0x1C

#define  GYRO2_LO_PASS_OFF     0x00
#define  GYRO2_LO_PASS_MOD0    0x40
#define  GYRO2_LO_PASS_MOD1    0x60
#define  GYRO2_LO_PASS_MOD2    0xC0

/**
 * \defgroup bits_end Bits end
 * \{
 */
#define GYRO2_RT_CFG     0x0E

/**
 * \defgroup bits_start Bits start
 * \{
 */
#define GYRO2_RT_CFG_ELE      0x08
#define GYRO2_RT_CFG_ZTEFE    0x04
#define GYRO2_RT_CFG_YTEFE    0x02
#define GYRO2_RT_CFG_XTEFE    0x01

/**
 * \defgroup bits_end Bits end
 * \{
 */
#define GYRO2_RT_SRC          0x0F
#define GYRO2_RT_THS          0x10
#define GYRO2_RT_COUNT        0x11
#define GYRO2_TEMP            0x12

/**
 * \defgroup bits Bits
 * \{
 */
#define  GYRO2_DR_800Hz          0
#define  GYRO2_DR_400Hz          1
#define  GYRO2_DR_200Hz          2
#define  GYRO2_DR_100Hz          3
#define  GYRO2_DR_50Hz           4
#define  GYRO2_DR_25Hz           5
#define  GYRO2_DR_12_5Hz         6
#define  GYRO2_STANDBY           0
#define  GYRO2_READY             1
#define  GYRO2_ACTIVE            3

/**
 * \defgroup reg Reg
 * \{
 */
#define GYRO2_CTRL_REG2       0x14

/**
 * \defgroup bits Bits
 * \{
 */
#define GYRO2_INT_CFG_FIFO_INT1    0x80
#define GYRO2_INT_CFG_FIFO_INT2    0x00
#define GYRO2_INT_EN_FIFO          0x40
#define GYRO2_INT_DIS_FIFO         0x00
#define GYRO2_INT_CFG_RT_INT1      0x20
#define GYRO2_INT_CFG_RT_INT2      0x00
#define GYRO2_INT_EN_RT            0x10
#define GYRO2_DIS_EN_RT            0x00
#define GYRO2_INT_CFG_DRDY_INT1    0x08
#define GYRO2_INT_CFG_DRDY_INT2    0x00
#define GYRO2_INT_EN_DRDY          0x04
#define GYRO2_INT_DIS_DRDY         0x00
#define GYRO2_IPOL_ACTIVE_HI       0x02
#define GYRO2_IPOL_ACTIVE_LO       0x00
#define GYRO2_PP_OD_OS             0x01
#define GYRO2_PP_OD_PUSH_PULL      0x00

/**
 * \defgroup end_bits End bits
 * \{
 */
#define GYRO2_CTRL_REG3            0x15

void InitGyro2(mikrobus_hdr_t hdr, uint8_t instNum, uint8_t txCh, uint8_t rxCh);

#endif /*__MB_GYRO2_H__*/
