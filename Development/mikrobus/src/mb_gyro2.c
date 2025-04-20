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
    .baudRate_Hz = 380000,
    .busIdleTimeout_ns = 0,
    .pinLowTimeout_ns = 0,
    .sclGlitchFilterWidth_ns = 0,
    .sdaGlitchFilterWidth_ns = 0,
    .hostRequest.enable = false,
  };
  
  LPI2C_MasterInit(i2c(), &_masterConfig, 12000000);
}

static void initDma(uint8_t instNum, uint8_t txCh, uint8_t rxCh)
{
  if (instNum == 0){
    CLOCK_EnableClock(kCLOCK_Dma0);
    RESET_ReleasePeripheralReset(kDMA0_RST_SHIFT_RSTn);
    s_IrqReg = 0;
    s_IrqMask = (1 << (txCh + 1)) | (1 << (rxCh + 1));
		s_edma = DMA0;
  } else {
    CLOCK_EnableClock(kCLOCK_Dma1);
    RESET_ReleasePeripheralReset(kDMA1_RST_SHIFT_RSTn);    
    s_IrqReg = 2;
    s_IrqMask = (1 << (txCh + 13)) | (1 << (rxCh + 13));
		s_edma = DMA1;
  }
  s_DmaTxCh = txCh;
  s_DmaRxCh = rxCh;

  /*LPSPI Tx DMA*/
  edma()->CH_GRPRI[txCh] = DMA_CH_GRPRI_GRPRI(31);
  edma()->CH[txCh].CH_PRI = DMA_CH_PRI_ECP_MASK | DMA_CH_PRI_APL(0b110);
  edma()->CH[txCh].CH_CSR = (DMA_CH_CSR_DONE_MASK | DMA_CH_CSR_EEI_MASK);
  edma()->CH[txCh].CH_MUX = s_TxDreq;

  edma()->CH[txCh].TCD_SADDR = (uint32_t)&testTxBuf[0];
  edma()->CH[txCh].TCD_SOFF = 2;
  edma()->CH[txCh].TCD_DADDR = (uint32_t)&i2c()->MTDR;
  edma()->CH[txCh].TCD_DOFF = 0;
  edma()->CH[txCh].TCD_DLAST_SGA = 0;
  edma()->CH[txCh].TCD_ATTR = (DMA_ATTR_SSIZE(kEDMA_TransferSize2Bytes) | DMA_ATTR_DSIZE(kEDMA_TransferSize2Bytes));
  edma()->CH[txCh].TCD_CITER_ELINKNO = 5;
  edma()->CH[txCh].TCD_BITER_ELINKNO = 5;
  edma()->CH[txCh].TCD_NBYTES_MLOFFNO = 2;
  edma()->CH[txCh].TCD_SLAST_SDA = -10;
  edma()->CH[txCh].TCD_CSR = DMA_CSR_DREQ_MASK;

  /*LPSPI Rx DMA*/
  edma()->CH_GRPRI[rxCh] = DMA_CH_GRPRI_GRPRI(31);
  edma()->CH[rxCh].CH_PRI = DMA_CH_PRI_ECP_MASK | DMA_CH_PRI_APL(0b111);
  edma()->CH[rxCh].CH_CSR = (DMA_CH_CSR_DONE_MASK | DMA_CH_CSR_EEI_MASK);
  edma()->CH[rxCh].CH_MUX = s_RxDreq;

  edma()->CH[rxCh].TCD_SADDR = (uint32_t)&i2c()->MRDR;
  edma()->CH[rxCh].TCD_SOFF = 0;
  edma()->CH[rxCh].TCD_DADDR = (uint32_t)&testRxBuf[0];
  edma()->CH[rxCh].TCD_DOFF = 2;
  edma()->CH[rxCh].TCD_DLAST_SGA = -2;
  edma()->CH[rxCh].TCD_ATTR = (DMA_ATTR_SSIZE(kEDMA_TransferSize1Bytes) | DMA_ATTR_DSIZE(kEDMA_TransferSize1Bytes));
  edma()->CH[rxCh].TCD_CITER_ELINKNO = 1;
  edma()->CH[rxCh].TCD_BITER_ELINKNO = 1;
  edma()->CH[rxCh].TCD_NBYTES_MLOFFNO = 1;
  edma()->CH[rxCh].TCD_SLAST_SDA = 0;
  edma()->CH[rxCh].TCD_CSR = (DMA_CSR_DREQ_MASK | DMA_CSR_INTMAJOR_MASK);

  NVIC_SetPriority(EDMA_0_CH2_IRQn, 1);
  NVIC_EnableIRQ(EDMA_0_CH2_IRQn);
  NVIC_SetPriority(EDMA_0_CH3_IRQn, 1);
  NVIC_EnableIRQ(EDMA_0_CH3_IRQn);

  //NVIC_SetPriority(rxTask_VDIn, 3);
  //NVIC_EnableIRQ(rxTask_VDIn);
}

//uint16_t rxCount;
//uint16_t txCount;

void EDMA_0_CH3_IRQHandler(void)
{

	edma()->CH[s_DmaRxCh].CH_INT = DMA_CH_INT_INT_MASK;
	i2c()->MDER = 0;
}

void ReadTest()
{
  /*for (int i = 0; i < 5; i++){
    i2c()->MTDR = testTxBuf[i];
  }
	txCount = (i2c()->MFSR & LPI2C_MFSR_TXCOUNT_MASK) >> LPI2C_MFSR_TXCOUNT_SHIFT;
  rxCount = (i2c()->MFSR & LPI2C_MFSR_RXCOUNT_MASK) >> LPI2C_MFSR_RXCOUNT_SHIFT;
	while (rxCount == 0){
		rxCount = (i2c()->MFSR & LPI2C_MFSR_RXCOUNT_MASK) >> LPI2C_MFSR_RXCOUNT_SHIFT;
	}
  for (int i = 0; i < rxCount; i++){
    testRxBuf[i] = (i2c()->MRDR & 0xFF);
  }*/
	i2c()->MCR |= (LPI2C_MCR_RTF_MASK | LPI2C_MCR_RRF_MASK);
  edma()->CH[s_DmaTxCh].CH_CSR |= DMA_CH_CSR_ERQ_MASK;
  edma()->CH[s_DmaRxCh].CH_CSR |= DMA_CH_CSR_ERQ_MASK;
  i2c()->MDER = (LPI2C_MDER_RDDE_MASK | LPI2C_MDER_TDDE_MASK);
  
}

void InitGyro2(mikrobus_hdr_t hdr, uint8_t instNum, uint8_t txCh, uint8_t rxCh)
{
  initLpi2c(hdr);
  initDma(instNum, txCh, rxCh);
	ReadTest();
}
