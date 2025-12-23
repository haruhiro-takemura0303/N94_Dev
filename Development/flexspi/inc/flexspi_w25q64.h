/**
* @brief   FRDM-MCXN947 board FLEXSPI(Winbond W25Q64) Driver
* @author  masa
* @version 1.00
*/

#ifndef __FLEXSPI_W25Q64_H__
#define __FLEXSPI_W25Q64_H__

#include "flexspi_pin.h"
#include "fsl_flexspi.h"


// W25Q64 = 8MB = 8192KB
#define W25Q64_FLASH_SIZE_KB   (8192u)
#define W25Q64_PAGE_SIZE        256u
#define W25Q64_SR1_WIP          0x01u
#define W25Q64_SR2_QE           0x02u

#define W25Q64_SECTOR_SIZE      0x1000

#define FLEXSPI_AHB_BASE        0x80000000UL

typedef enum{
  LUT_READ_QIO  = 0,  // 0xEB Fast Read Quad I/O
  LUT_RDSR1     = 1,  // 0x05
  LUT_WREN      = 2,  // 0x06
  LUT_SE4K      = 3,  // 0x20
  LUT_PP_QPP    = 4,  // 0x32 Quad Input Page Program
  LUT_RDID      = 5,  // 0x9F
  LUT_RDSR2     = 6,  // 0x35
  LUT_WRSR      = 7,  // 0x01 (SR1,SR2)
  LUT_RDSR3     = 8,  // 0x15
  LUT_GULK      = 9,  // 0x98（任意）
	LUT_RESET_EN  = 10,
  LUT_RESET     = 11,
} flexSPI_W25Q64_LUT_Index_t;


typedef enum{
  FSPI_OK = 0,
  FSPI_FAIL = -1
} flexSPI_Status_t;

typedef struct{
  uint32_t intr;
  uint32_t sts1;
} flexSPI_Err_t;

void InitQSPI_FlexSpi0(void);
flexSPI_Status_t W25Q64_ReadJedecID(uint8_t id[3]);
flexSPI_Status_t W25Q64_QuadEnable(void);
flexSPI_Status_t W25Q64_ReadSR1(uint8_t *sr1);
flexSPI_Status_t W25Q64_ReadSR2(uint8_t *sr2);
flexSPI_Status_t W25Q64_ReadSR3(uint8_t *sr3);
flexSPI_Status_t W25Q64_Erase4K(uint32_t addr);
flexSPI_Status_t W25Q64_ProgramPage(uint32_t addr, const void *data, size_t len);
flexSPI_Status_t W25Q64_Read(uint32_t addr, void *data, size_t len);
void FLEXSPI_Read(uint32_t addr, void *data, size_t len);

#endif /*__FLEXSPI_W25Q64_H__*/
