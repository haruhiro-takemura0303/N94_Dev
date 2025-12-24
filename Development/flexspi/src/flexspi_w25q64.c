/**
* @brief   FRDM-MCXN947 board FLEXSPI(Winbond W25Q64) Driver
* @author  masa
* @version 1.00
*/

#include "flexspi_w25q64.h"

static inline FLEXSPI_Type* flexspi()
{
  return FLEXSPI0;
}


static const uint32_t st_LUT[4 * 16] =
{
  // Fast Read Quad I/O (EBh)
  [4 * LUT_READ_QIO] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0xEB, kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_4PAD, 0x18),
  [4 * LUT_READ_QIO + 1] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_MODE8_SDR, kFLEXSPI_4PAD, 0xFF, kFLEXSPI_Command_DUMMY_SDR, kFLEXSPI_4PAD, 0x04), // 4 dummy clocks
  [4 * LUT_READ_QIO + 2] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_READ_SDR, kFLEXSPI_4PAD, 0x00, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0x00),
  
  // 0x05 Read SR1
  [4 * LUT_RDSR1] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x05, kFLEXSPI_Command_READ_SDR, kFLEXSPI_1PAD, 0x00),
  
  // 0x06 WREN
  [4 * LUT_WREN] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x06, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0x00),
  
  // 0x20 Sector Erase 4KB
  [4 * LUT_SE4K] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x20, kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_1PAD, 0x18),
  
  // Page Program (32h): addr 1pad、data 4pad
  [4 * LUT_PP_QPP] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x32, kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_1PAD, 0x18),
  [4 * LUT_PP_QPP + 1] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_WRITE_SDR, kFLEXSPI_4PAD, 0x00, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0x00),
  
  // 0x9F JEDEC ID (3 bytes)
  [4 * LUT_RDID] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x9F, kFLEXSPI_Command_READ_SDR, kFLEXSPI_1PAD, 0x00),
  
  // 0x35 Read SR2
  [4 * LUT_RDSR2] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x35, kFLEXSPI_Command_READ_SDR, kFLEXSPI_1PAD, 0x00),
  
  // 0x01 Write SR1+SR2
  [4 * LUT_WRSR] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x01, kFLEXSPI_Command_WRITE_SDR, kFLEXSPI_1PAD, 0x00),
  
  // 0x15 Read SR3
  [4 * LUT_RDSR3] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x15, kFLEXSPI_Command_READ_SDR, kFLEXSPI_1PAD, 0x00),
  
  // 0x98 Global Block Unlock
  [4 * LUT_GULK] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x98, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0x00),
  
  [4 * LUT_RESET_EN] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x66, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0x00),
  
  [4 * LUT_RESET] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x99, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0x00),
};

void InitQSPI_FlexSpi0(void)
{
  
  flexspi_config_t cfg;
  flexspi_device_config_t devCfg;
  
  FlexSpi0_InitPins();
  
  CLOCK_SetClkDiv(kCLOCK_DivFlexspiClk, 2u);
  CLOCK_AttachClk(kPLL0_to_FLEXSPI);
  CLOCK_EnableClock(kCLOCK_Flexspi);
  
  FLEXSPI_GetDefaultConfig(&cfg);
  FLEXSPI_Init(flexspi(), &cfg);
  
  memset(&devCfg, 0, sizeof(flexspi_device_config_t));
  
  devCfg.flexspiRootClk = 75000000U,
  devCfg.flashSize = W25Q64_FLASH_SIZE_KB;
  devCfg.CSIntervalUnit = kFLEXSPI_CsIntervalUnit1SckCycle;
  devCfg.CSInterval = 2;
  devCfg.CSHoldTime = 3;
  devCfg.CSSetupTime = 3;
  devCfg.dataValidTime = 2;
  devCfg.columnspace = 0;
  devCfg.enableWordAddress = false;
  devCfg.ARDSeqIndex = LUT_READ_QIO;
  devCfg.ARDSeqNumber = 1;
  devCfg.AWRSeqIndex = LUT_PP_QPP;
  devCfg.AWRSeqNumber = 1;
  
  flexspi()->IPTXFCR = FLEXSPI_IPTXFCR_TXWMRK(15);
	flexspi()->IPRXFCR = FLEXSPI_IPRXFCR_RXWMRK(15);
  
  FLEXSPI_SetFlashConfig(flexspi(), &devCfg, kFLEXSPI_PortA1);
  FLEXSPI_UpdateLUT(flexspi(), 0, &st_LUT[0], sizeof(st_LUT)/sizeof(st_LUT[0]));
  
  FLEXSPI_SoftwareReset(flexspi());
}

static inline void waitIdle(void)
{
  while(!(flexspi()->STS0 & FLEXSPI_STS0_ARBIDLE_MASK));
}

static inline void clearInterrupts(void)
{
  uint32_t intr = flexspi()->INTR;
  flexspi()->INTR = intr;
}

static inline void txFifoClear(void)
{
  flexspi()->IPTXFCR |= FLEXSPI_IPTXFCR_CLRIPTXF_MASK;
}

static inline void rxFifoClear(void)
{
  flexspi()->IPRXFCR |= FLEXSPI_IPRXFCR_CLRIPRXF_MASK;
}

static flexSPI_Status_t waitDone(flexSPI_Err_t* err)
{
  const uint32_t errMask = FLEXSPI_INTR_IPCMDERR_MASK | FLEXSPI_INTR_AHBCMDERR_MASK;
  while(1) {
    uint32_t intr = flexspi()->INTR;
    
    if (intr & errMask){
      if (err) {
        err->intr = intr; err->sts1 = flexspi()->STS1;
      }
      flexspi()->INTR = (intr & errMask); // W1C
      return FSPI_FAIL;
    }
    if (intr & FLEXSPI_INTR_IPCMDDONE_MASK){
      if (err) { 
        err->intr = intr; err->sts1 = flexspi()->STS1; 
      }
      flexspi()->INTR = FLEXSPI_INTR_IPCMDDONE_MASK; // W1C
      return FSPI_OK;
    }
  } 
}

static inline void startIpCmd(void)
{
  flexspi()->IPCMD = FLEXSPI_IPCMD_TRG_MASK;
}

static flexSPI_Status_t ipCmd(uint8_t seqID, uint8_t seqNum, uint32_t addr, flexSPI_Err_t *err)
{
  waitIdle();
  clearInterrupts();
  
  flexspi()->IPCR0 = addr;
  flexspi()->IPCR1 = (FLEXSPI_IPCR1_ISEQID(seqID) | FLEXSPI_IPCR1_ISEQNUM((seqNum - 1)) | FLEXSPI_IPCR1_IDATSZ(0));
  
  startIpCmd();
  
  return waitDone(err);
}

static flexSPI_Status_t ipRead(void *bufPtr, uint16_t transLen, uint8_t seqID, uint8_t seqNum, uint32_t addr, flexSPI_Err_t *err)
{
  flexSPI_Status_t stat;
  uint8_t* u8Ptr;
  uint16_t count = 0;
  union{
    uint8_t byte[4];
    uint32_t dword;
  }rfdr0;
  uint16_t rsdlLen = transLen;
  
  waitIdle();
  clearInterrupts();
  rxFifoClear();
  
  flexspi()->IPCR0 = addr;
  flexspi()->IPCR1 = (FLEXSPI_IPCR1_ISEQID(seqID) | FLEXSPI_IPCR1_ISEQNUM((seqNum - 1)) | FLEXSPI_IPCR1_IDATSZ(transLen));
  
  startIpCmd();
  
  stat = waitDone(err);
  if (stat){
    return stat;
  }
  
  u8Ptr = (uint8_t*)bufPtr;
  while (rsdlLen >= sizeof(uint32_t)){
    if (count == 32){
      flexspi()->INTR = (uint32_t)kFLEXSPI_IpRxFifoWatermarkAvailableFlag;
      while (rsdlLen > ((((flexspi()->IPRXFSTS) & FLEXSPI_IPRXFSTS_FILL_MASK) >> FLEXSPI_IPRXFSTS_FILL_SHIFT) * 8U));
      count = 0;
    }
    rfdr0.dword = flexspi()->RFDR[count];
    for (int i = 0; i < sizeof(uint32_t); i++){
      u8Ptr[i] = rfdr0.byte[i];
    }
    u8Ptr += sizeof(uint32_t);
    rsdlLen -= sizeof(uint32_t);
    count++;
  }
  
  if (count == 32){
    flexspi()->INTR = (uint32_t)kFLEXSPI_IpRxFifoWatermarkAvailableFlag;
    while (rsdlLen > ((((flexspi()->IPRXFSTS) & FLEXSPI_IPRXFSTS_FILL_MASK) >> FLEXSPI_IPRXFSTS_FILL_SHIFT) * 8U));
    count = 0;
  }
  
  if (rsdlLen){
    rfdr0.dword = flexspi()->RFDR[count];
    for (int i = 0; i < rsdlLen; i++){
      u8Ptr[i] = rfdr0.byte[i];
    }    
  }
  return FSPI_OK;
}

static flexSPI_Status_t wren(void)
{
  return ipCmd((uint8_t)LUT_WREN, 1, 0, NULL);
}

static flexSPI_Status_t waitReady(void)
{
  uint8_t sr1;
  while(1){
    sr1 = 0xFF;
    if (ipRead(&sr1, 1, (uint8_t)LUT_RDSR1, 1, 0, NULL)){
      return FSPI_FAIL;
    }
    if(!(sr1 & W25Q64_SR1_WIP)){
      return FSPI_OK;
    }
  }
}

flexSPI_Status_t SR1Reset(void)
{
  
  flexSPI_Err_t err;
  
  if (wren() != FSPI_OK) {
    return FSPI_FAIL;
  }
  
  waitIdle();
  clearInterrupts();
  txFifoClear();
  
  flexspi()->TFDR[0] = 0x00000000;
  
  flexspi()->INTR = (uint32_t)kFLEXSPI_IpTxFifoWatermarkEmptyFlag;
  
  flexspi()->IPCR0 = 0;
  flexspi()->IPCR1 =
  FLEXSPI_IPCR1_ISEQID(LUT_WRSR) |
  FLEXSPI_IPCR1_ISEQNUM(0) |
  FLEXSPI_IPCR1_IDATSZ(1);
  
  
  startIpCmd();
  
  if (waitDone(&err) != FSPI_OK) {
    return FSPI_FAIL;
  }
  
  if (waitReady() != 0) {
    return FSPI_FAIL;
  }
  
  return FSPI_OK;
}



static flexSPI_Status_t ipWrite(const void *bufPtr, uint16_t transLen, uint8_t seqID, uint8_t seqNum, uint32_t addr, flexSPI_Err_t *err)
{
  flexSPI_Status_t stat;
  const uint8_t* u8Ptr;
  union{
    uint8_t byte[4];
    uint32_t dword;
  }tfdr0;
  uint16_t rsdlLen = transLen;
  uint16_t u32Block;
  uint8_t count = 0;
  
  waitIdle();
  clearInterrupts();
  txFifoClear();
  
  while((flexspi()->INTR & (uint32_t)kFLEXSPI_IpTxFifoWatermarkEmptyFlag) == 0);
  
  u8Ptr = (const uint8_t*)bufPtr;
  while(rsdlLen){
    tfdr0.dword = 0;
    u32Block = (rsdlLen >= sizeof(uint32_t)) ? sizeof(uint32_t) : rsdlLen;
    for (int i = 0; i < u32Block; i++){
      tfdr0.byte[i] = u8Ptr[i];
    }
    flexspi()->TFDR[count] = tfdr0.dword;
    u8Ptr += u32Block;
    rsdlLen -= u32Block;
    count++;
    if (count >= 32){
      flexspi()->INTR = (uint32_t)kFLEXSPI_IpTxFifoWatermarkEmptyFlag;
      count = 0;
      while((flexspi()->INTR & (uint32_t)kFLEXSPI_IpTxFifoWatermarkEmptyFlag) == 0);
    }
  }
  
  if (count % 32){
    flexspi()->INTR = (uint32_t)kFLEXSPI_IpTxFifoWatermarkEmptyFlag;
  }
  
  flexspi()->IPCR0 = addr;
  flexspi()->IPCR1 = (FLEXSPI_IPCR1_ISEQID(seqID) | FLEXSPI_IPCR1_ISEQNUM((seqNum - 1)) | FLEXSPI_IPCR1_IDATSZ(transLen));
  
  startIpCmd();
  
  waitDone(err);
  
  if (waitReady() != 0) {
    return FSPI_FAIL;
  }
  
  return FSPI_OK;
}

static flexSPI_Status_t setQuadEnable(void)
{
  uint8_t sr1 = 0;
  uint8_t sr2 = 0;
  uint8_t buf[2];
  
  if (ipRead(&sr1, 1u, (uint8_t)LUT_RDSR1, 1u, 0u, NULL)){
    return FSPI_FAIL;
  }
  if (ipRead(&sr2, 1u, (uint8_t)LUT_RDSR2, 1u, 0u, NULL)){
    return FSPI_FAIL;
  }
  
  sr2 |= W25Q64_SR2_QE;
  
  if (wren()){
    return FSPI_FAIL;
  }
  
  buf[0] = 0x00;
  buf[1] = sr2;
  
  if (ipWrite(buf, 2u, (uint8_t)LUT_WRSR, 1u, 0u, NULL)){
    return FSPI_FAIL;
  }
  
  if (waitReady()){
    return FSPI_FAIL;
  }
  
  return FSPI_OK;
}

flexSPI_Status_t W25Q64_ReadJedecID(uint8_t id[3])
{
  memset(id, 0, 3);
  return ipRead(id, 3u, (uint8_t)LUT_RDID, 1u, 0u, NULL);
}

flexSPI_Status_t W25Q64_QuadEnable(void)
{
  return setQuadEnable();
}


flexSPI_Status_t W25Q64_ReadSR1(uint8_t *sr1)
{
  return ipRead(sr1, 1u, (uint8_t)LUT_RDSR1, 1u, 0u, NULL);
}

flexSPI_Status_t W25Q64_ReadSR2(uint8_t *sr2)
{
  return ipRead(sr2, 1u, (uint8_t)LUT_RDSR2, 1u, 0u, NULL);
}

flexSPI_Status_t W25Q64_ReadSR3(uint8_t *sr3)
{
  return ipRead(sr3, 1u, (uint8_t)LUT_RDSR3, 1u, 0u, NULL);
}

flexSPI_Status_t W25Q64_Erase4K(uint32_t addr)
{
  if (wren()){
    return FSPI_FAIL;
  }
  
  if (ipCmd((uint8_t)LUT_SE4K, 1u, addr, NULL)){
    return FSPI_FAIL;
  }
  
  if (waitReady()){
    return FSPI_FAIL;
  }
  
  return FSPI_OK;
}

flexSPI_Status_t W25Q64_ProgramPage(uint32_t addr, const void *data, size_t len)
{
  
  flexSPI_Err_t err;
  
  if (wren()){
    return FSPI_FAIL;
  }
  
  if (ipWrite(data, len, (uint8_t)LUT_PP_QPP, 1u, addr, &err)){
    return FSPI_FAIL;
  }
  
  if (waitReady()){
    return FSPI_FAIL;
  }
  
  return FSPI_OK;
}


flexSPI_Status_t W25Q64_Read(uint32_t addr, void *data, size_t len)
{
  return ipRead(data, len, (uint8_t)LUT_READ_QIO, 1, addr, NULL); 
}
