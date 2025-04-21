/**
* @brief   FRDM-MCXN947 board mikroBUS Inter Integrated-Circuit[I2C] Driver for Gyro 2 Clickboard
* @author  masa
* @version 1.00
*/

#include "mb_gyro2.h"
#include "fsl_ctimer.h"
#include "board.h"
#include "pin_mux.h"

#define G2_I2C_SLAVEADDR        0x21
#define G2_I2C_SLAVEADDR_READ   ((G2_I2C_SLAVEADDR << 1) | kLPI2C_Read)
#define G2_I2C_SLAVEADDR_WRITE  ((G2_I2C_SLAVEADDR << 1) | kLPI2C_Write)

g2_txField_t s_gtxDataBuf = {
  .stSlvAddr = kI2C_GenStartAndSendAddress | G2_I2C_SLAVEADDR_WRITE
};
uint8_t s_grxDataBuf[6];

static LPI2C_Type* s_lpi2c;
static LP_FLEXCOMM_Type* s_lpflexcomm;
static DMA_Type* s_edma;
static CTIMER_Type* s_ctimer;
static uint32_t s_TxDreq;
static uint32_t s_RxDreq;
static uint8_t s_DmaRxCh;
static uint8_t s_DmaTxCh;
static uint8_t s_IrqReg;
static uint32_t s_IrqMask;

struct{
  float X;
  float Y;
  float Z;
	float dX;
	float dY;
	float dZ;
}g_AngularData;

g2_fifo_t s_gTxFifo;
g2_rx_fifo_t s_gRxFifo;

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

static inline CTIMER_Type* ctimer()
{
	return s_ctimer;
}

static inline void g2_disint(void)
{
  
  NVIC->ICER[s_IrqReg] = s_IrqMask;
}

static inline void g2_enaint(void)
{
  NVIC->ISER[s_IrqReg] = s_IrqMask;
}

static void startI2c(uint8_t rw)
{
  i2c()->MCR = (i2c()->MCR | (LPI2C_MCR_RTF_MASK | LPI2C_MCR_RRF_MASK));
	if (!rw){
		i2c()->MDER = LPI2C_MDER_TDDE_MASK;
	} else {
		i2c()->MDER = LPI2C_MDER_RDDE_MASK | LPI2C_MDER_TDDE_MASK;
	}
}

static void hookTx(g2_tx_t txCmd)
{
  if (txCmd.FIELD.readSize > G2_MAX_READ_BURST_SIZE){
    return;
  }
  s_gtxDataBuf.regAddr = txCmd.FIELD.regAddr;

  if (!txCmd.FIELD.readSize){
    s_gtxDataBuf.RW.WRITE.regValue = txCmd.FIELD.regValue;
    s_gtxDataBuf.RW.WRITE.sp = kI2C_GenStop;
    edma()->CH[s_DmaTxCh].TCD_BITER_ELINKNO = 4;
    edma()->CH[s_DmaTxCh].TCD_CITER_ELINKNO = 4;
    edma()->CH[s_DmaTxCh].TCD_SLAST_SDA = -8;
    edma()->CH[s_DmaTxCh].TCD_CSR |= DMA_CSR_INTMAJOR_MASK;
    edma()->CH[s_DmaTxCh].CH_CSR |= DMA_CH_CSR_ERQ_MASK;
  } else {
    s_gtxDataBuf.RW.READ.srSlvAddr = (kI2C_GenStartAndSendAddress | G2_I2C_SLAVEADDR_READ);
    s_gtxDataBuf.RW.READ.readCmd_Size = (kI2C_ReceiveData | (txCmd.FIELD.readSize - 1));
    s_gtxDataBuf.RW.READ.sp = kI2C_GenStop;
    edma()->CH[s_DmaTxCh].TCD_BITER_ELINKNO = 5;
    edma()->CH[s_DmaTxCh].TCD_CITER_ELINKNO = 5;
    edma()->CH[s_DmaTxCh].TCD_SLAST_SDA = -10;
    edma()->CH[s_DmaTxCh].TCD_CSR &= ~DMA_CSR_INTMAJOR_MASK;

    edma()->CH[s_DmaRxCh].TCD_BITER_ELINKNO = txCmd.FIELD.readSize;
    edma()->CH[s_DmaRxCh].TCD_CITER_ELINKNO = txCmd.FIELD.readSize;
    edma()->CH[s_DmaRxCh].TCD_DLAST_SGA = (-1 * txCmd.FIELD.readSize);
    edma()->CH[s_DmaRxCh].TCD_CSR |= DMA_CSR_INTMAJOR_MASK;

    edma()->CH[s_DmaRxCh].CH_CSR |= DMA_CH_CSR_ERQ_MASK;
    edma()->CH[s_DmaTxCh].CH_CSR |= DMA_CH_CSR_ERQ_MASK;
  }

  startI2c(txCmd.FIELD.readSize);
}

static void repeatTx(void)
{
  uint8_t dIdx;
	
  g2_disint();
  dIdx = s_gTxFifo.deqIdx;
	edma()->CH[s_DmaTxCh].TCD_SADDR = (uint32_t)&s_gtxDataBuf.stSlvAddr;
	edma()->CH[s_DmaRxCh].TCD_DADDR = (uint32_t)&s_grxDataBuf[0];
	hookTx(s_gTxFifo.fifo[dIdx]);
  
	g2_enaint();	
}

static int32_t enqueueRx(g2_rx_t* rxResult)
{
  uint8_t eIdx;
  uint8_t dIdx;
  
  eIdx = s_gRxFifo.enqIdx;
  dIdx = s_gRxFifo.deqIdx;
  
  if ((dIdx - eIdx) == 1){
    return -1;
  }

  s_gRxFifo.fifo[eIdx].WORD[0] = rxResult->WORD[0];
  s_gRxFifo.fifo[eIdx].WORD[1] = rxResult->WORD[1];

  eIdx = (eIdx + 1) % G2_RX_FIFO_SIZE;
  s_gRxFifo.enqIdx = eIdx;
  NVIC_SetPendingIRQ(g2RxTask_VDIn);
  return 0;  
}

static int32_t dequeueRx(g2_rx_t* rxBuf)
{
  uint8_t eIdx;
  uint8_t dIdx;
  g2_disint();
  eIdx = s_gRxFifo.enqIdx;
  dIdx = s_gRxFifo.deqIdx;
  
  if (dIdx == eIdx){
    g2_enaint();
    return -1;
  }

  rxBuf->WORD[0] = s_gRxFifo.fifo[dIdx].WORD[0];
  rxBuf->WORD[1] = s_gRxFifo.fifo[dIdx].WORD[1];

  dIdx = (dIdx + 1) % G2_RX_FIFO_SIZE;
  s_gRxFifo.deqIdx = dIdx;
  g2_enaint();
  return 0;  
}

static int32_t enqueueRequest(g2_tx_t txCmd)
{
  uint8_t eIdx;
  uint8_t dIdx;
  g2_disint();
  eIdx = s_gTxFifo.enqIdx;
  dIdx = s_gTxFifo.deqIdx;
  
  if ((dIdx - eIdx) == 1){
    g2_enaint();
    return -1;
  }

  s_gTxFifo.fifo[eIdx].WORD = txCmd.WORD;

  if (eIdx == dIdx){
    hookTx(txCmd);
  }

  eIdx = (eIdx + 1) % G2_I2C_FIFO_SIZE;
  s_gTxFifo.enqIdx = eIdx;
  g2_enaint();
  return 0;
}

static int32_t sendRequest(uint8_t regAddr, uint8_t regValue, uint8_t readSize)
{
	g2_tx_t txCmd = {
		.FIELD.regAddr = regAddr,
		.FIELD.regValue = regValue,
		.FIELD.readSize = readSize	
	};
	
	return enqueueRequest(txCmd);
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
			
			NVIC_EnableIRQ(LP_FLEXCOMM3_IRQn);
			NVIC_SetPriority(LP_FLEXCOMM3_IRQn, 1);
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
	
	i2c()->MIER |= (LPI2C_MIER_NDIE_MASK | LPI2C_MIER_ALIE_MASK);
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

  edma()->CH[txCh].TCD_SADDR = (uint32_t)&s_gtxDataBuf.stSlvAddr;
  edma()->CH[txCh].TCD_SOFF = 2;
  edma()->CH[txCh].TCD_DADDR = (uint32_t)&i2c()->MTDR;
  edma()->CH[txCh].TCD_DOFF = 0;
  edma()->CH[txCh].TCD_DLAST_SGA = 0;
  edma()->CH[txCh].TCD_ATTR = (DMA_ATTR_SSIZE(kEDMA_TransferSize2Bytes) | DMA_ATTR_DSIZE(kEDMA_TransferSize2Bytes));
  edma()->CH[txCh].TCD_NBYTES_MLOFFNO = 2;
  edma()->CH[txCh].TCD_CSR = DMA_CSR_DREQ_MASK;

  /*LPSPI Rx DMA*/
  edma()->CH_GRPRI[rxCh] = DMA_CH_GRPRI_GRPRI(31);
  edma()->CH[rxCh].CH_PRI = DMA_CH_PRI_ECP_MASK | DMA_CH_PRI_APL(0b111);
  edma()->CH[rxCh].CH_CSR = (DMA_CH_CSR_DONE_MASK | DMA_CH_CSR_EEI_MASK);
  edma()->CH[rxCh].CH_MUX = s_RxDreq;

  edma()->CH[rxCh].TCD_SADDR = (uint32_t)&i2c()->MRDR;
  edma()->CH[rxCh].TCD_SOFF = 0;
  edma()->CH[rxCh].TCD_DADDR = (uint32_t)&s_grxDataBuf[0];
  edma()->CH[rxCh].TCD_DOFF = 1;
  edma()->CH[rxCh].TCD_ATTR = (DMA_ATTR_SSIZE(kEDMA_TransferSize1Bytes) | DMA_ATTR_DSIZE(kEDMA_TransferSize1Bytes));
  edma()->CH[rxCh].TCD_NBYTES_MLOFFNO = 1;
  edma()->CH[rxCh].TCD_SLAST_SDA = 0;
  edma()->CH[rxCh].TCD_CSR = DMA_CSR_DREQ_MASK;

  NVIC_SetPriority(EDMA_0_CH2_IRQn, 1);
  NVIC_EnableIRQ(EDMA_0_CH2_IRQn);
  NVIC_SetPriority(EDMA_0_CH3_IRQn, 1);
  NVIC_EnableIRQ(EDMA_0_CH3_IRQn);

  NVIC_SetPriority(g2RxTask_VDIn, 3);
  NVIC_EnableIRQ(g2RxTask_VDIn);
}

static void initTimer(uint8_t instNum)
{
	switch(instNum){
		case(0):{
			CLOCK_SetClkDiv(kCLOCK_DivCtimer0Clk, 1u);
			CLOCK_AttachClk(kFRO12M_to_CTIMER0);
			NVIC_SetPriority(CTIMER0_IRQn, 2);
			s_ctimer = CTIMER0;
			break;
		}
		case(2):{
			CLOCK_SetClkDiv(kCLOCK_DivCtimer2Clk, 1u);
			CLOCK_AttachClk(kFRO12M_to_CTIMER2);
			NVIC_SetPriority(CTIMER2_IRQn, 2);
			s_ctimer = CTIMER2;
			break;
		}
		default:
			break;
	}
  ctimer_config_t c_conf = {
    .input = kCTIMER_Capture_0,
    .mode = kCTIMER_TimerMode,
    .prescale = 0
  };
  CTIMER_Init(ctimer(), &c_conf);
  ctimer_match_config_t match_conf = {
    .enableCounterReset = true,
    .enableCounterStop = false,
    .enableInterrupt = true,
    .matchValue = 120000,
    .outControl = kCTIMER_Output_NoAction,
  };
  CTIMER_SetupMatch(ctimer(), kCTIMER_Match_0, &match_conf);
}

static void startTimer(void)
{
	CTIMER_StartTimer(ctimer());
}

static void initLedApp(void)
{
	PORT_SetPinMux(BOARD_INITPINS_LED_GREEN_PORT, BOARD_INITPINS_LED_GREEN_PIN, kPORT_MuxAlt4);
	PORT_SetPinMux(BOARD_INITPINS_LED_BLUE_PORT, BOARD_INITPINS_LED_BLUE_PIN, kPORT_MuxAlt4);
	PORT_SetPinMux(BOARD_INITPINS_LED_RED_PORT, BOARD_INITPINS_LED_RED_PIN, kPORT_MuxAlt4);
	
	CLOCK_SetClkDiv(kCLOCK_DivCtimer0Clk, 1u);
	CLOCK_AttachClk(kFRO12M_to_CTIMER0);
	CLOCK_SetClkDiv(kCLOCK_DivCtimer1Clk, 1u);
	CLOCK_AttachClk(kFRO12M_to_CTIMER1);
	
	ctimer_config_t c_conf = {
    .input = kCTIMER_Capture_0,
    .mode = kCTIMER_TimerMode,
    .prescale = 0
  };
  CTIMER_Init(CTIMER0, &c_conf);
	CTIMER_Init(CTIMER1, &c_conf);
	
  ctimer_match_config_t match_conf = {
    .enableCounterReset = true,
    .enableCounterStop = false,
    .enableInterrupt = false,
    .matchValue = 12000,
    .outControl = kCTIMER_Output_NoAction,
  };
  CTIMER_SetupMatch(CTIMER0, kCTIMER_Match_2, &match_conf);
	CTIMER_SetupMatch(CTIMER1, kCTIMER_Match_2, &match_conf);
	
	match_conf.enableCounterReset = false;
	match_conf.matchValue = 600;
	CTIMER_SetupMatch(CTIMER0, kCTIMER_Match_0, &match_conf);
	CTIMER_SetupMatch(CTIMER0, kCTIMER_Match_3, &match_conf);
	CTIMER_SetupMatch(CTIMER1, kCTIMER_Match_0, &match_conf);
	
	CTIMER0->PWMC |= (CTIMER_PWMC_PWMEN0_MASK | CTIMER_PWMC_PWMEN3_MASK);
	CTIMER1->PWMC |= CTIMER_PWMC_PWMEN0_MASK;
	
	CTIMER_StartTimer(CTIMER0);
	CTIMER_StartTimer(CTIMER1);
	
}

void EDMA_0_CH2_IRQHandler(void)
{
  uint8_t eIdx = s_gTxFifo.enqIdx;
  uint8_t dIdx = s_gTxFifo.deqIdx;

	edma()->CH[s_DmaTxCh].CH_INT = DMA_CH_INT_INT_MASK;
	i2c()->MDER = 0;

  dIdx = (dIdx + 1) % G2_I2C_FIFO_SIZE;
  s_gTxFifo.deqIdx = dIdx;
  s_gTxFifo.enqIdx = eIdx;

  if (dIdx != eIdx){
    hookTx(s_gTxFifo.fifo[dIdx]);
  }
}

void EDMA_0_CH3_IRQHandler(void)
{
  uint8_t eIdx = s_gTxFifo.enqIdx;
  uint8_t dIdx = s_gTxFifo.deqIdx;
  g2_tx_t txCmd;
  g2_rx_t rxResult;

	edma()->CH[s_DmaRxCh].CH_INT = DMA_CH_INT_INT_MASK;
	i2c()->MDER = 0;
	
  txCmd.WORD = s_gTxFifo.fifo[dIdx].WORD;
  rxResult.FIELD.firstReg = txCmd.FIELD.regAddr;
  rxResult.FIELD.readSize = txCmd.FIELD.readSize;
  for (int i = 0; i < txCmd.FIELD.readSize; i++){
    rxResult.FIELD.regValue[i] = s_grxDataBuf[i];
  }
  enqueueRx(&rxResult);

  dIdx = (dIdx + 1) % G2_I2C_FIFO_SIZE;
  s_gTxFifo.deqIdx = dIdx;
  s_gTxFifo.enqIdx = eIdx;

  if (dIdx != eIdx){
    hookTx(s_gTxFifo.fifo[dIdx]);
  }
}

void LP_FLEXCOMM3_IRQHandler(void)
{
	if (flexcomm()->ISTAT & LP_FLEXCOMM_ISTAT_I2CM_MASK){
		if ((i2c()->MSR & LPI2C_MSR_NDF_MASK) || (i2c()->MSR & LPI2C_MSR_ALF_MASK)){
			i2c()->MDER = 0;
			i2c()->MCR = (i2c()->MCR | (LPI2C_MCR_RTF_MASK | LPI2C_MCR_RRF_MASK));
			i2c()->MSR = (LPI2C_MSR_NDF_MASK | LPI2C_MSR_ALF_MASK);
			repeatTx();
		}
	}
}

void g2RxTask_VDIHandler(void)
{
  g2_rx_t rxBuf;
  int ret;

  NVIC_ClearPendingIRQ(g2RxTask_VDIn);

  for (;;){
    ret = dequeueRx(&rxBuf);
    if (ret){
      return;
    }
		switch(rxBuf.FIELD.firstReg){
			case(GYRO2_OUT_X_MSB):{
        int16_t xRaw, yRaw, zRaw, xTimVal, yTimVal, zTimVal;
				float dXnew, dYnew, dZnew;
        xRaw = (uint16_t)(rxBuf.FIELD.regValue[0] << 8) | rxBuf.FIELD.regValue[1];
        yRaw = (uint16_t)(rxBuf.FIELD.regValue[2] << 8) | rxBuf.FIELD.regValue[3];
        zRaw = (uint16_t)(rxBuf.FIELD.regValue[4] << 8) | rxBuf.FIELD.regValue[5];
        dXnew = xRaw * 0.015625f / 88.0f * 3.0f;
        dYnew = yRaw * 0.015625f / 88.0f * 3.0f;
        dZnew = zRaw * 0.015625f / 88.0f * 3.0f;
				g_AngularData.X += ((g_AngularData.dX + dXnew) * 0.005);
				g_AngularData.Y += ((g_AngularData.dY + dYnew) * 0.005);
				g_AngularData.Z += ((g_AngularData.dY + dZnew) * 0.005);
				g_AngularData.dX = dXnew;
				g_AngularData.dY = dYnew;
				g_AngularData.dZ = dZnew;
				
				xTimVal = (int16_t)(g_AngularData.X * 300.0f);
				if (xTimVal > 600){
					xTimVal = 599;
				} else if (xTimVal < -600){
					xTimVal = -599;
				}
				yTimVal = (int16_t)(g_AngularData.Y * 300.0f);
				if (yTimVal > 600){
					yTimVal = 599;
				} else if (yTimVal < -600){
					yTimVal = -599;
				}
				zTimVal = (int16_t)(g_AngularData.Z * 300.0f);
				if (zTimVal > 600){
					zTimVal = 599;
				} else if (zTimVal < -600){
					zTimVal = -599;
				}
				CTIMER0->MR[0] = 600 + xTimVal;
				CTIMER0->MR[3] = 600 + yTimVal;
				CTIMER1->MR[0] = 600 + zTimVal;
        break;
      }
		}
  }
}

void GPIO50_IRQHandler(void)
{
  if (GPIO5->ISFR[0] & GPIO_ISFR_ISF7_MASK){
    LED_RED_ON();
    GPIO5->ISFR[0] = GPIO_ISFR_ISF7_MASK;
    sendRequest(GYRO2_OUT_X_MSB, 0, G2_READ_6BYTE);
  } 
}

void CTIMER2_IRQHandler(void)
{
  if (ctimer()->IR & CTIMER_IR_MR0INT_MASK){
    ctimer()->IR = CTIMER_IR_MR0INT_MASK;
		sendRequest(GYRO2_OUT_X_MSB, 0, G2_READ_6BYTE);
  }
}

void InitGyro2(mikrobus_hdr_t hdr, uint8_t instNum, uint8_t txCh, uint8_t rxCh)
{
  initLpi2c(hdr);
  initDma(instNum, txCh, rxCh);

  //GPIO_SetPinInterruptConfig(BOARD_INITPINS_INT_GPIO, BOARD_INITPINS_INT_PIN, kGPIO_InterruptFallingEdge);
  //NVIC_SetPriority(GPIO50_IRQn, 2);
  //NVIC_EnableIRQ(GPIO50_IRQn);
	initTimer(2);
	initLedApp();
	
  sendRequest(GYRO2_RT_CFG, (GYRO2_RT_CFG_XTEFE | GYRO2_RT_CFG_YTEFE | GYRO2_RT_CFG_ZTEFE), G2_WRITE);
  sendRequest(GYRO2_RT_THS, 10, G2_WRITE);
  sendRequest(GYRO2_CTRL_REG1, ((GYRO2_DR_100Hz << 2) | GYRO2_ACTIVE), G2_WRITE);
  sendRequest(GYRO2_CTRL_REG2, (GYRO2_INT_CFG_DRDY_INT1 | GYRO2_INT_EN_DRDY | GYRO2_PP_OD_OS | GYRO2_IPOL_ACTIVE_LO), G2_WRITE);
  sendRequest(GYRO2_CTRL_REG0, (GYRO2_LO_PASS_MOD2 | GYRO2_HI_PASS_OFF | GYRO2_SCALE_3), G2_WRITE);
	startTimer();
}
