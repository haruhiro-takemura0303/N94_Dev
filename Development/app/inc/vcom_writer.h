/**
* @brief   FRDM-MCXN947 board USBD + FLEXSPI Flash Writing App
* @author  masa
* @version 1.00
*/

#ifndef __VCOM_WRITER_H__
#define __VCOM_WRITER_H__

#include <stdarg.h>
#include <stdio.h>

#include "flexspi_w25q64.h"
#include "usbd_dual_vcom.h"
#include "wave.h"

#define VCOM_WRITER_MSGBOX_SIZE  32
#define VCOM_WRITER_IRQn CDOG0_IRQn
#define VCOM_WRITER_LOG_BUF     8192

typedef struct{
  uint8_t msgType;
  uint16_t datSize;
  uint32_t totalSize;
} vcom_Writer_Msg_t;

typedef struct{
  vcom_Writer_Msg_t msg[VCOM_WRITER_MSGBOX_SIZE];
  uint8_t enqPtr;
  uint8_t deqPtr;
} vcom_Writer_MsgBox_t;

typedef struct{
  uint8_t logBuf[VCOM_WRITER_LOG_BUF];
  uint32_t enqPtr;
  uint32_t deqPtr;
} vcom_Writer_LogBuf_t;

enum{
  VCOM_WRITER_START = 1,
  VCOM_WRITER_LOG_NEXT,
  VCOM_WRITER_KICK,
  VCOM_WRITER_RECEIVED,
  VCOM_WRITER_COMPLETE,
};

typedef struct{
  uint32_t dataSize;
  uint32_t curPointer;
} vcom_Writer_Data_Table_t;

void InitVcomWriter(void);

#endif /*__VCOM_WRITER_H__*/
