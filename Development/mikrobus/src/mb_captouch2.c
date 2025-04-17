/**
* @brief   FRDM-MCXN947 board mikroBUS Serial Peripheral Interface Driver
* @author  masa
* @version 1.00
*/

#include "mb_captouch2.h"

ct2_txField_t s_txDataBuf = {
  .rst0 = 0x7A,
  .rst1 = 0x7A,
  .addrSet = 0x7D,
};
ct2_rxField_t s_rxDataBuf;
static LPSPI_Type* s_lpspi;
static LP_FLEXCOMM_Type* s_lpflexcomm;
static DMA_Type* s_edma;
static uint32_t s_TxDreq;
static uint32_t s_RxDreq;
static uint8_t s_DmaRxCh;
static uint8_t s_DmaTxCh;
static uint8_t s_IrqReg;
static uint32_t s_IrqMask;

ct2_fifo_t s_TxFifo;
ct2_rx_fifo_t s_RxFifo;

static inline LPSPI_Type* spi(void)
{
  return s_lpspi;
}

static inline LP_FLEXCOMM_Type* flexcomm(void)
{
  return s_lpflexcomm;
}

static inline DMA_Type* edma(void)
{
  return s_edma;
}

static inline void ct2_disint(void)
{
  
  NVIC->ICER[s_IrqReg] = s_IrqMask;
}

static inline void ct2_enaint(void)
{
  NVIC->ISER[s_IrqReg] = s_IrqMask;
}

static void startSpi(uint8_t rw)
{
  spi()->CR = (spi()->CR | (LPSPI_CR_RTF_MASK | LPSPI_CR_RRF_MASK));
	spi()->TCR = (spi()->TCR | (LPSPI_TCR_CONT_MASK | LPSPI_TCR_CONTC_MASK));
	if (!rw){
		spi()->DER = LPSPI_DER_TDDE_MASK;
	} else {
		spi()->DER = LPSPI_DER_RDDE_MASK | LPSPI_DER_TDDE_MASK;
	}
}

static void hookTx(ct2_tx_t txCmd)
{
  uint8_t txCount = 0;
  s_txDataBuf.addr = txCmd.FIELD.regAddr;

  if (!txCmd.FIELD.readSize){
    s_txDataBuf.readWrite = 0x7E;
    s_txDataBuf.RW_FIELD.WRITE.regValue = txCmd.FIELD.regValue;
    txCount = 6;
    edma()->CH[s_DmaTxCh].TCD_BITER_ELINKNO = txCount;
    edma()->CH[s_DmaTxCh].TCD_CITER_ELINKNO = txCount;
    edma()->CH[s_DmaTxCh].TCD_SLAST_SDA = (-1 * 4 * txCount);
    edma()->CH[s_DmaTxCh].TCD_CSR |= DMA_CSR_INTMAJOR_MASK;
    edma()->CH[s_DmaTxCh].CH_CSR |= DMA_CH_CSR_ERQ_MASK;
  } else {
    switch(txCmd.FIELD.readSize){
      case(1):{
        s_txDataBuf.readWrite = 0x7F;
        s_txDataBuf.RW_FIELD.READ.readContinue[0] = 0;
        s_txDataBuf.RW_FIELD.READ.readContinue[1] = 0;
        txCount = 6;
        break;
      }
      case(2):{
        s_txDataBuf.readWrite = 0x7F;
        s_txDataBuf.RW_FIELD.READ.readContinue[0] = 0;
        s_txDataBuf.RW_FIELD.READ.readContinue[1] = 0;
        txCount = 7;      
        break;
      }
      case(3):{
        s_txDataBuf.readWrite = 0x7F;
        s_txDataBuf.RW_FIELD.READ.readContinue[0] = 0x7F;
        s_txDataBuf.RW_FIELD.READ.readContinue[1] = 0;
        txCount = 8;      
        break;
      }
      default:
        break;
    }
    if (txCount){
      edma()->CH[s_DmaTxCh].TCD_BITER_ELINKNO = txCount;
      edma()->CH[s_DmaTxCh].TCD_CITER_ELINKNO = txCount;
      edma()->CH[s_DmaTxCh].TCD_SLAST_SDA = (-1 * 4 * txCount);
      edma()->CH[s_DmaTxCh].TCD_CSR &= ~DMA_CSR_INTMAJOR_MASK;

      edma()->CH[s_DmaRxCh].TCD_BITER_ELINKNO = txCount;
      edma()->CH[s_DmaRxCh].TCD_CITER_ELINKNO = txCount;
      edma()->CH[s_DmaRxCh].TCD_DLAST_SGA = (-1 * 4 * txCount);
      edma()->CH[s_DmaRxCh].TCD_CSR |= DMA_CSR_INTMAJOR_MASK;

			edma()->CH[s_DmaRxCh].CH_CSR |= DMA_CH_CSR_ERQ_MASK;
      edma()->CH[s_DmaTxCh].CH_CSR |= DMA_CH_CSR_ERQ_MASK;
    }
  }

  if (!txCount){
    return;
  } else {
    startSpi(txCmd.FIELD.readSize);
  }
}

static int32_t enqueueRx(ct2_rx_t* rxResult)
{
  uint8_t eIdx;
  uint8_t dIdx;
  
  eIdx = s_RxFifo.enqIdx;
  dIdx = s_RxFifo.deqIdx;
  
  if ((dIdx - eIdx) == 1){
    return -1;
  }

  s_RxFifo.fifo[eIdx].WORD[0] = rxResult->WORD[0];
  s_RxFifo.fifo[eIdx].WORD[1] = rxResult->WORD[1];

  eIdx = (eIdx + 1) % CT2_RX_FIFO_SIZE;
  s_RxFifo.enqIdx = eIdx;
  NVIC_SetPendingIRQ(rxTask_VDIn);
  return 0;  
}

static int32_t dequeueRx(ct2_rx_t* rxBuf)
{
  uint8_t eIdx;
  uint8_t dIdx;
  ct2_disint();
  eIdx = s_RxFifo.enqIdx;
  dIdx = s_RxFifo.deqIdx;
  
  if (dIdx == eIdx){
    ct2_enaint();
    return -1;
  }

  rxBuf->WORD[0] = s_RxFifo.fifo[dIdx].WORD[0];
  rxBuf->WORD[1] = s_RxFifo.fifo[dIdx].WORD[1];

  dIdx = (dIdx + 1) % CT2_RX_FIFO_SIZE;
  s_RxFifo.deqIdx = dIdx;
  ct2_enaint();
  return 0;  
}

static int32_t enqueueRequest(ct2_tx_t txCmd)
{
  uint8_t eIdx;
  uint8_t dIdx;
  ct2_disint();
  eIdx = s_TxFifo.enqIdx;
  dIdx = s_TxFifo.deqIdx;
  
  if ((dIdx - eIdx) == 1){
    ct2_enaint();
    return -1;
  }

  s_TxFifo.fifo[eIdx].WORD = txCmd.WORD;

  if (eIdx == dIdx){
    hookTx(txCmd);
  }

  eIdx = (eIdx + 1) % CT2_SPI_FIFO_SIZE;
  s_TxFifo.enqIdx = eIdx;
  ct2_enaint();
  return 0;
}

static int32_t sendRequest(uint8_t regAddr, uint8_t regValue, uint8_t readSize)
{
	ct2_tx_t txCmd = {
		.FIELD.regAddr = regAddr,
		.FIELD.regValue = regValue,
		.FIELD.readSize = readSize	
	};
	
	return enqueueRequest(txCmd);
}

void initLpspi(mikrobus_hdr_t hdr)
{
  switch(hdr){
    case(DEFUALT_MIKROBUS):{
      s_lpspi = LPSPI6;
      s_lpflexcomm = LP_FLEXCOMM6;
      s_TxDreq = kDma0RequestMuxLpFlexcomm6Tx;
      s_RxDreq = kDma0RequestMuxLpFlexcomm6Rx;
      /*FLEXCOMM Clock Enable*/
      CLOCK_SetClkDiv(kCLOCK_DivFlexcom6Clk, 1u);
      CLOCK_AttachClk(kFRO12M_to_FLEXCOMM6);
    
      /*FLEXCOMM Enable*/
      CLOCK_EnableClock(kCLOCK_LPFlexComm6);
    
      /*LPSPI Clock Enable*/
      CLOCK_EnableClock(kCLOCK_LPSpi6);
      break;
    }
  }

  /*FLEXCOMM SPI Enable*/
  flexcomm()->PSELID |= LP_FLEXCOMM_PSELID_PERSEL(0b010);
  flexcomm()->PSELID |= LP_FLEXCOMM_PSELID_LOCK_MASK;

  lpspi_master_config_t _masterConfig = {
    .baudRate = 3000000,
    .bitsPerFrame = 8,
    .cpol = kLPSPI_ClockPolarityActiveLow,
    .cpha = kLPSPI_ClockPhaseSecondEdge,
    .direction = kLPSPI_MsbFirst,
    .pcsToSckDelayInNanoSec = 15000,
    .lastSckToPcsDelayInNanoSec = 15000,
    .betweenTransferDelayInNanoSec = 200,
    .whichPcs = kLPSPI_Pcs0,
    .pcsActiveHighOrLow = kLPSPI_PcsActiveLow,
    .pinCfg = kLPSPI_SdiInSdoOut,
    .pcsFunc = kLPSPI_PcsAsCs,
    .dataOutConfig = kLpspiDataOutRetained,
    .enableInputDelay = false
  };
  
  LPSPI_MasterInit(spi(), &_masterConfig, 12000000);

}

void initDma(uint8_t instNum, uint8_t txCh, uint8_t rxCh)
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

  edma()->CH[txCh].TCD_SADDR = (uint32_t)&s_txDataBuf.rst0;
  edma()->CH[txCh].TCD_SOFF = 4;
  edma()->CH[txCh].TCD_DADDR = (uint32_t)&spi()->TDR;
  edma()->CH[txCh].TCD_DOFF = 0;
  edma()->CH[txCh].TCD_DLAST_SGA = 0;
  edma()->CH[txCh].TCD_ATTR = (DMA_ATTR_SSIZE(kEDMA_TransferSize4Bytes) | DMA_ATTR_DSIZE(kEDMA_TransferSize4Bytes));
  edma()->CH[txCh].TCD_CITER_ELINKNO = 8;
  edma()->CH[txCh].TCD_BITER_ELINKNO = 8;
  edma()->CH[txCh].TCD_NBYTES_MLOFFNO = 4;
  edma()->CH[txCh].TCD_SLAST_SDA = -32;
  edma()->CH[txCh].TCD_CSR = DMA_CSR_DREQ_MASK;

  /*LPSPI Rx DMA*/
  edma()->CH_GRPRI[rxCh] = DMA_CH_GRPRI_GRPRI(31);
  edma()->CH[rxCh].CH_PRI = DMA_CH_PRI_ECP_MASK | DMA_CH_PRI_APL(0b111);
  edma()->CH[rxCh].CH_CSR = (DMA_CH_CSR_DONE_MASK | DMA_CH_CSR_EEI_MASK);
  edma()->CH[rxCh].CH_MUX = s_RxDreq;

  edma()->CH[rxCh].TCD_SADDR = (uint32_t)&spi()->RDR;
  edma()->CH[rxCh].TCD_SOFF = 0;
  edma()->CH[rxCh].TCD_DADDR = (uint32_t)&s_rxDataBuf.rsvd[0];
  edma()->CH[rxCh].TCD_DOFF = 4;
  edma()->CH[rxCh].TCD_DLAST_SGA = -32;
  edma()->CH[rxCh].TCD_ATTR = (DMA_ATTR_SSIZE(kEDMA_TransferSize4Bytes) | DMA_ATTR_DSIZE(kEDMA_TransferSize4Bytes));
  edma()->CH[rxCh].TCD_CITER_ELINKNO = 8;
  edma()->CH[rxCh].TCD_BITER_ELINKNO = 8;
  edma()->CH[rxCh].TCD_NBYTES_MLOFFNO = 4;
  edma()->CH[rxCh].TCD_SLAST_SDA = 0;
  edma()->CH[rxCh].TCD_CSR = (DMA_CSR_DREQ_MASK);

  NVIC_SetPriority(EDMA_0_CH0_IRQn, 1);
  NVIC_EnableIRQ(EDMA_0_CH0_IRQn);
  NVIC_SetPriority(EDMA_0_CH1_IRQn, 1);
  NVIC_EnableIRQ(EDMA_0_CH1_IRQn);

  NVIC_SetPriority(rxTask_VDIn, 3);
  NVIC_EnableIRQ(rxTask_VDIn);
}

void EDMA_0_CH0_IRQHandler(void)
{
  uint8_t eIdx = s_TxFifo.enqIdx;
  uint8_t dIdx = s_TxFifo.deqIdx;

	edma()->CH[s_DmaTxCh].CH_INT = DMA_CH_INT_INT_MASK;
  spi()->TCR = (spi()->TCR & ~(LPSPI_TCR_CONT_MASK | LPSPI_TCR_CONTC_MASK));
	spi()->DER = 0;
	while (spi()->FSR & LPSPI_FSR_TXCOUNT_MASK){
	}

  dIdx = (dIdx + 1) % CT2_SPI_FIFO_SIZE;
  s_TxFifo.deqIdx = dIdx;
  s_TxFifo.enqIdx = eIdx;

  if (dIdx != eIdx){
    hookTx(s_TxFifo.fifo[dIdx]);
  }

}

void EDMA_0_CH1_IRQHandler(void)
{
  uint8_t eIdx = s_TxFifo.enqIdx;
  uint8_t dIdx = s_TxFifo.deqIdx;
  ct2_tx_t txCmd;
  ct2_rx_t rxResult;

	edma()->CH[s_DmaRxCh].CH_INT = DMA_CH_INT_INT_MASK;
  spi()->TCR = (spi()->TCR & ~(LPSPI_TCR_CONT_MASK | LPSPI_TCR_CONTC_MASK));
	spi()->DER = 0;
	while (spi()->FSR & LPSPI_FSR_TXCOUNT_MASK){
	}
	
  txCmd.WORD = s_TxFifo.fifo[dIdx].WORD;
  rxResult.FIELD.firstReg = txCmd.FIELD.regAddr;
  rxResult.FIELD.readSize = txCmd.FIELD.readSize;
  rxResult.FIELD.regValue[0] = s_rxDataBuf.regValue[0];
  rxResult.FIELD.regValue[1] = s_rxDataBuf.regValue[1];
  rxResult.FIELD.regValue[2] = s_rxDataBuf.regValue[2];
  enqueueRx(&rxResult);

  dIdx = (dIdx + 1) % CT2_SPI_FIFO_SIZE;
  s_TxFifo.deqIdx = dIdx;
  s_TxFifo.enqIdx = eIdx;

  if (dIdx != eIdx){
    hookTx(s_TxFifo.fifo[dIdx]);
  }
}

void rxTask_VDIHandler(void)
{
  ct2_rx_t rxBuf;
  int ret;

  NVIC_ClearPendingIRQ(rxTask_VDIn);

  for (;;){
    ret = dequeueRx(&rxBuf);
    if (ret){
      return;
    }
    

  }
}

void InitCapTouch2(mikrobus_hdr_t hdr, uint8_t instNum, uint8_t txCh, uint8_t rxCh)
{
	initLpspi(hdr);
	initDma(instNum, txCh, rxCh);

	
  sendRequest(CAPTOUCH2_LED_BEHAVIOR1_REG, 
    (CAPTOUCH2_LED1_PULSE2_BEHAVIOR |
    CAPTOUCH2_LED2_PULSE2_BEHAVIOR |
    CAPTOUCH2_LED3_PULSE2_BEHAVIOR |
    CAPTOUCH2_LED4_DIRECT_BEHAVIOR), CT2_WRITE);
  
  sendRequest(CAPTOUCH2_LED_BEHAVIOR2_REG,
    (CAPTOUCH2_LED5_DIRECT_BEHAVIOR |
    CAPTOUCH2_LED6_DIRECT_BEHAVIOR), CT2_WRITE);
  
  sendRequest(CAPTOUCH2_LED_PULSE2_PERIOD_REG, 0x08, CT2_WRITE);
  
  sendRequest(CAPTOUCH2_LED_CONFIG_REG, CAPTOUCH2_PULSE2_1_PULSE, CT2_WRITE);
  
  sendRequest(CAPTOUCH2_LED_PULSE2_DUTY_REG, 
    (CAPTOUCH2_MIN_DUTY_0_PERCENT |
    CAPTOUCH2_MAX_DUTY_100_PERCENTS), CT2_WRITE);
  
  sendRequest(CAPTOUCH2_LED_DIRECT_DUTY_REG,
    (CAPTOUCH2_MIN_DUTY_0_PERCENT |
    CAPTOUCH2_MAX_DUTY_40_PERCENTS), CT2_WRITE);
  
  sendRequest(CAPTOUCH2_REPEAT_RATE_EN_REG, 
    (CAPTOUCH2_INPUT4_REPEAT_RATE_EN |
    CAPTOUCH2_INPUT5_REPEAT_RATE_EN |
    CAPTOUCH2_INPUT6_REPEAT_RATE_EN), CT2_WRITE);
  
  sendRequest(CAPTOUCH2_CONFIG2_REG,
    (CAPTOUCH2_ALERT_ACTIVE_LOW |
    CAPTOUCH2_SHOW_LOW_FREQ_NOISE |
    CAPTOUCH2_RF_NOISE_FILTER_EN |
    CAPTOUCH2_DETECT_RELEASE_EN), CT2_WRITE);
  
  sendRequest(CAPTOUCH2_CONFIG_REG,
    (CAPTOUCH2_DIG_NOISE_THRESHOLD_DIS |
    CAPTOUCH2_AN_NOISE_FILTER_EN |
    CAPTOUCH2_WAKE_PIN_NOT_ASSERTED), CT2_WRITE);
  
  sendRequest(CAPTOUCH2_SENSITIVITY_CON_REG,
    (CAPTOUCH2_SENSITIVITY_MULTIPLIER_32X |
    CAPTOUCH2_DATA_SCALING_FACTOR_256X), CT2_WRITE);
  
  sendRequest(CAPTOUCH2_AVRG_AND_SAMPL_CONFIG_REG,
    (CAPTOUCH2_8_SAMPLES |
    CAPTOUCH2_SAMPLE_TIME_1280MICROSEC |
    CAPTOUCH2_CYCLE_TIME_70MILISEC), CT2_WRITE);
  
  sendRequest(CAPTOUCH2_SENS_IN_CONFIG2_REG, CAPTOUCH2_PRESS_AND_HOLD_EVENT_AFTER_280MILISEC, CT2_WRITE);

  sendRequest(CAPTOUCH2_SENS_IN_CONFIG_REG, 
    (CAPTOUCH2_5600MILISEC_BEFORE_RECALIB |
    CAPTOUCH2_INTERR_REPEAT_RATE_175MILISEC), CT2_WRITE);
  
  
  sendRequest(CAPTOUCH2_MAIN_CONTROL_REG, 0, CT2_WRITE);
  sendRequest(CAPTOUCH2_SENS_IN_EN_REG, CAPTOUCH2_ALL_INPUTS_ENABLE, CT2_WRITE);
  sendRequest(CAPTOUCH2_INTERR_EN_REG, CAPTOUCH2_ALL_INPUTS_ENABLE, CT2_WRITE);
  sendRequest(CAPTOUCH2_SENS_IN_LED_LINK_REG, CAPTOUCH2_ALL_INPUTS_ENABLE, CT2_WRITE);

}
