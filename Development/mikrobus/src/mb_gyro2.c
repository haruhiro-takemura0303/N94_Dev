/**
* @brief   FRDM-MCXN947 board mikroBUS Inter Integrated-Circuit[I2C] Driver for Gyro 2 Clickboard
* @author  masa
* @version 1.00
*/

#include "mb_gyro2.h"

#define G2_I2C_SLAVEADDR        0x21
#define G2_I2C_SLAVEADDR_READ   ((G2_I2C_SLAVEADDR << 1) | kLPI2C_Read)
#define G2_I2C_SLAVEADDR_WRITE  ((G2_I2C_SLAVEADDR << 1) | kLPI2C_Write)

uint16_t testTxBuf[] = {kI2C_GenStartAndSendAddress | G2_I2C_SLAVEADDR_WRITE, 
                        kI2C_TransmitData | 0x0C, 
                        kI2C_GenStartAndSendAddress | G2_I2C_SLAVEADDR_READ,
                        kI2C_ReceiveData | 0x00,
                        kI2C_GenStop
                      };

uint16_t testRxBuf[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

static LPI2C_Type* s_lpi2c;
static LP_FLEXCOMM_Type* s_lpflexcomm;
static DMA_Type* s_edma;
static uint32_t s_TxDreq;
static uint32_t s_RxDreq;
static uint8_t s_DmaRxCh;
static uint8_t s_DmaTxCh;
static uint8_t s_IrqReg;
static uint32_t s_IrqMask;

static inline LPI2C_Type* i2c(void)
{
  return s_lpi2c;
}

static inline LP_FLEXCOMM_Type* flexcomm(void)
{
  return s_lpflexcomm;
}

static inline DMA_Type* edma(void)
{
  return s_edma;
}

static inline void g2_disint(void)
{
  
  NVIC->ICER[s_IrqReg] = s_IrqMask;
}

static inline void g2_enaint(void)
{
  NVIC->ISER[s_IrqReg] = s_IrqMask;
}

void initLpi2c(mikrobus_hdr_t hdr)
{
  switch(hdr){
    case(DEFUALT_MIKROBUS):{
      s_lpi2c = LPI2C3;
      s_lpflexcomm = LP_FLEXCOMM3;
      s_TxDreq = kDma0RequestMuxLpFlexcomm3Tx;
      s_RxDreq = kDma0RequestMuxLpFlexcomm3Rx;
			
      /*FLEXCOMM Clock Enable*/
      CLOCK_SetClkDiv(kCLOCK_DivFlexcom3Clk, 1u);
      CLOCK_AttachClk(kFRO12M_to_FLEXCOMM3);
    
      /*FLEXCOMM Enable*/
      CLOCK_EnableClock(kCLOCK_LPFlexComm3);
    
      /*LPSPI Clock Enable*/
      CLOCK_EnableClock(kCLOCK_LPI2c3);
      break;
    }
  }

  lpi2c_master_config_t _masterConfig = {
    .enableMaster = true,
    .enableDoze = false,
    .debugEnable = false,
    .ignoreAck = false,
    .pinConfig = kLPI2C_2PinOpenDrain,
    .baudRate_Hz = 300000,
    .busIdleTimeout_ns = 0,
    .pinLowTimeout_ns = 0,
    .sclGlitchFilterWidth_ns = 0,
    .sdaGlitchFilterWidth_ns = 0,
    .hostRequest.enable = false,
  };
  
  LPI2C_MasterInit(i2c(), &_masterConfig, 12000000);

}
uint16_t rxCount;
uint16_t txCount;
void ReadTest()
{
  for (int i = 0; i < 5; i++){
    i2c()->MTDR = testTxBuf[i];
  }
	txCount = (i2c()->MFSR & LPI2C_MFSR_TXCOUNT_MASK) >> LPI2C_MFSR_TXCOUNT_SHIFT;
  rxCount = (i2c()->MFSR & LPI2C_MFSR_RXCOUNT_MASK) >> LPI2C_MFSR_RXCOUNT_SHIFT;
	while (rxCount == 0){
		rxCount = (i2c()->MFSR & LPI2C_MFSR_RXCOUNT_MASK) >> LPI2C_MFSR_RXCOUNT_SHIFT;
	}
  for (int i = 0; i < rxCount; i++){
    testRxBuf[i] = (i2c()->MRDR & 0xFF);
  }
	//i2c()->MTDR = kI2C_GenStop;
  
}

status_t stat;

void InitGyro2(mikrobus_hdr_t hdr, uint8_t instNum, uint8_t txCh, uint8_t rxCh)
{
  initLpi2c(hdr);
	ReadTest();
	ReadTest();
	
}
