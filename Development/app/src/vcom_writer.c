/**
* @brief   FRDM-MCXN947 board USBD + FLEXSPI Flash Writing App
* @author  masa
* @version 1.00
*/

#include "vcom_writer.h"

vcom_Writer_MsgBox_t st_MsgBox;
vcom_Writer_LogBuf_t st_LogBuf;
uint8_t st_Init = 0;
uint8_t st_Fail = 0;
uint8_t st_Comp = 0;
vcom_Writer_Wave_Head_t st_waveTester;
vcom_Writer_Data_Table_t st_dataTable;
uint8_t st_TempBuf[256];

static inline uint32_t disint(void) {
  uint32_t primask;
  __asm volatile ("MRS %0, primask" : "=r"(primask) :: "memory");
  __asm volatile ("cpsid i" ::: "memory");
  return primask;
}
static inline void enaint(uint32_t primask) {
  __asm volatile ("MSR primask, %0" :: "r"(primask) : "memory");
}

static int32_t enqueueMsg(vcom_Writer_Msg_t* msg)
{
  int32_t ret;
  uint32_t primask;
  primask = disint();
  if (st_MsgBox.deqPtr - st_MsgBox.enqPtr != 1){
    memcpy(&st_MsgBox.msg[st_MsgBox.enqPtr], msg, sizeof(vcom_Writer_Msg_t));
    st_MsgBox.enqPtr++;
    if (st_MsgBox.enqPtr == VCOM_WRITER_MSGBOX_SIZE){
      st_MsgBox.enqPtr = 0;
    }
    ret = 0;
  } else {
    ret = -1;
  }
  
  enaint(primask);
  NVIC_SetPendingIRQ(VCOM_WRITER_IRQn);
  
  return ret;
}

static int32_t dequeueMsg(vcom_Writer_Msg_t* msg)
{
  int32_t ret;
  uint32_t primask;
  primask = disint();
  if (st_MsgBox.deqPtr != st_MsgBox.enqPtr){
    memcpy(msg, &st_MsgBox.msg[st_MsgBox.deqPtr], sizeof(vcom_Writer_Msg_t));
    st_MsgBox.deqPtr++;
    if (st_MsgBox.deqPtr == VCOM_WRITER_MSGBOX_SIZE){
      st_MsgBox.deqPtr = 0;
    }
    ret = 0;
  } else {
    ret = -1;
  }
  enaint(primask);
  return ret;
}

static uint32_t minU32(uint32_t a, uint32_t b)
{
  if (a < b) {
    return a;
  } else {
    return b;
  }
}

static void portEnabled(uint8_t comIdx)
{
  vcom_Writer_Msg_t msg;
  if (comIdx == 0){
    msg.msgType = VCOM_WRITER_START;
    enqueueMsg(&msg);
  }
}

static inline uint32_t getCurrentLogSize(void)
{  
  uint32_t enq = st_LogBuf.enqPtr;
  uint32_t deq = st_LogBuf.deqPtr;
  return (enq >= deq) ? (enq - deq) : (VCOM_WRITER_LOG_BUF - (deq - enq));
}

static inline uint32_t getCurrentFreeSize(void)
{
  return (VCOM_WRITER_LOG_BUF - 1u) - getCurrentLogSize();
}

static void writeLog(const uint8_t* log, uint16_t size)
{
  uint32_t enq, first;
  enq = st_LogBuf.enqPtr;
  first = VCOM_WRITER_LOG_BUF - (enq % VCOM_WRITER_LOG_BUF);
  if (first > size){
    first = size;
  }
  for (int i = 0; i < first; i++){
    st_LogBuf.logBuf[(enq + i) % VCOM_WRITER_LOG_BUF] = log[i];
  }
  for (int i = first; i < size; i++){
    st_LogBuf.logBuf[(enq + i) % VCOM_WRITER_LOG_BUF] = log[i];
  }
  st_LogBuf.enqPtr += size;
}

static void startLogTransfer(void)
{
  uint32_t size, premask, offset;
  premask = disint();
  size = getCurrentLogSize();
  if (size > USB_CDC_DATAEP_MPS){
    size = USB_CDC_DATAEP_MPS;
  }
  offset = st_LogBuf.deqPtr % VCOM_WRITER_LOG_BUF;
  DualVcom_StartInTransfer(0, &st_LogBuf.logBuf[offset], size);
  enaint(premask);
}

static void writeLogAndKickSend(const uint8_t* log, uint16_t size)
{
  uint32_t offset, vacant, chunk, premask;
  uint8_t kick = 0;
  
  if (size == 0){
    return;
  }
  offset = 0;
  while(offset < size){
    premask = disint();
    vacant = getCurrentFreeSize();
    if (vacant == VCOM_WRITER_LOG_BUF - 1u){
      kick = 1;
    }
    
    chunk = size - offset;
    if (chunk > vacant){
      chunk = vacant;
    }
    
    if (chunk){
      writeLog(&log[offset], chunk);
      offset += chunk;
    }
    
    enaint(premask);
    if (chunk == 0){
      break;
    }
  }
  if (kick){
    startLogTransfer();
  }
}

int vcomLogPrint(const char* fmt, ...)
{
  char tmp[512];
  va_list ap;
  uint32_t actual;
  
  va_start(ap, fmt);
  int want = vsnprintf(tmp, sizeof(tmp), fmt, ap);
  va_end(ap);
  
  if (want <= 0){
    return want;
  }
  
  actual = (uint32_t)want;
  if (actual >= (uint32_t)sizeof(tmp)) {
    actual = (uint32_t)sizeof(tmp) - 1u;
  }
  
  writeLogAndKickSend((const uint8_t*)tmp, actual);
  return (int)actual;
}

#define VCOM_PRINTF vcomLogPrint

static void vcomInCallback(uint8_t comIdx, uint8_t* dataBuf, uint16_t transLen)
{
  vcom_Writer_Msg_t msg;
  msg.msgType = VCOM_WRITER_LOG_NEXT;
  msg.datSize = transLen;
  enqueueMsg(&msg);
}

static inline uint32_t loadLe32(const uint8_t* p)
{
  return ((uint32_t)p[0]) |
  ((uint32_t)p[1] << 8) |
  ((uint32_t)p[2] << 16) |
  ((uint32_t)p[3] << 24);
}

static void flashPageWrite(uint8_t* dataBuf, uint16_t transLen)
{
  uint32_t offSet, curAddr, rem, pageRem,wrLen;
  vcom_Writer_Msg_t msg;
  
  offSet = 0;
  msg.msgType = VCOM_WRITER_RECEIVED;
  
  while(offSet < transLen){
    curAddr = st_dataTable.curPointer;
    rem = transLen - offSet;
    if ((curAddr % W25Q64_SECTOR_SIZE) == 0){
      W25Q64_Erase4K(st_dataTable.curPointer);
    }
    
    pageRem = W25Q64_PAGE_SIZE - (curAddr % W25Q64_PAGE_SIZE);
    wrLen = minU32(pageRem, rem);
    
    W25Q64_ProgramPage(curAddr, &dataBuf[offSet], (uint16_t)wrLen);
    W25Q64_Read(curAddr, st_TempBuf, (uint16_t)wrLen);
    if (memcmp(&dataBuf[offSet], st_TempBuf, wrLen) != 0){
      st_Fail = 1;
      msg.totalSize = 0xFFFFFFFF;
      enqueueMsg(&msg);
      return;
    }
    st_dataTable.curPointer += wrLen;
    offSet += wrLen;
  }
  msg.totalSize = st_dataTable.curPointer;
  enqueueMsg(&msg);
}

static void waveWrite(uint8_t* dataBuf, uint16_t transLen)
{
  vcom_Writer_Msg_t msg;
  if (!st_dataTable.dataSize){
    memcpy(&st_waveTester, dataBuf, transLen);
    if (strncmp((const char*)&st_waveTester.riff_ckID[0], "RIFF", 4) == 0){
      st_dataTable.dataSize = st_waveTester.riff_cksize + 8;
      msg.msgType = VCOM_WRITER_KICK;
      msg.totalSize = st_dataTable.dataSize;
      enqueueMsg(&msg);
      flashPageWrite(dataBuf, transLen);
    }
  } else {
    flashPageWrite(dataBuf, transLen);
    if (st_dataTable.dataSize == st_dataTable.curPointer){
      msg.msgType = VCOM_WRITER_COMPLETE;
      msg.totalSize = st_dataTable.dataSize;
      st_Comp = 1;
      enqueueMsg(&msg);
    }
  }
}

static void vcomOutCallback(uint8_t comIdx, uint8_t* dataBuf, uint16_t transLen)
{
  if (comIdx == 0){
    if (!st_Init){
      st_Init = 1;
      portEnabled(0);
    } else if (!st_Fail && !st_Comp){
      waveWrite(dataBuf, transLen);
    }
  }
}

static void vcomWriterTask(void)
{
  vcom_Writer_Msg_t msg;
  int32_t status;
  
  NVIC_ClearPendingIRQ(VCOM_WRITER_IRQn);
  for(;;){
    status = dequeueMsg(&msg);
    if (status != 0){
      break;
    }
    switch(msg.msgType){
      case(VCOM_WRITER_START):{
        uint8_t id[3] = {0};
        VCOM_PRINTF("Welcome to VCOM File Write\n");
        W25Q64_ReadJedecID(id);
        VCOM_PRINTF("JedecID: 0x%x, 0x%x, 0x%x\n", id[0], id[1], id[2]);
        break;
      }
      case(VCOM_WRITER_LOG_NEXT):{
        uint32_t pre = disint();
        st_LogBuf.deqPtr += msg.datSize;
        if (getCurrentLogSize()){
          startLogTransfer();
        }
        enaint(pre);
        break;
      }
      case(VCOM_WRITER_KICK):{
        VCOM_PRINTF("File Writing Start, Size:%ld\n", msg.totalSize);
        break;
      }
      case(VCOM_WRITER_RECEIVED):{
        if (msg.totalSize != 0xFFFFFFFF){
          if ((msg.totalSize & 0xFFFFu) == 0u) {
            VCOM_PRINTF("Current Size:%ld\n", msg.totalSize);
          }
        } else {
          VCOM_PRINTF("Write Failed.\n");
        }
        break;
      }
      case(VCOM_WRITER_COMPLETE):{
        VCOM_PRINTF("Write Completed. Written Size is %dbyte.\n", st_dataTable.curPointer);
        break;
      }
      
      default:
      break;
    }
  }  
}

void InitVcomWriter(void)
{
  DualVcom_SetInCallBack(0, vcomInCallback);
  DualVcom_SetOutCallBack(0, vcomOutCallback);
  DualVcom_SetOutCallBack(1, vcomOutCallback);
  DualVcom_SetPortCallBack(portEnabled);
  NVIC_SetVector(VCOM_WRITER_IRQn, (uint32_t)vcomWriterTask);
  NVIC_SetPriority(VCOM_WRITER_IRQn, 6);
  NVIC_EnableIRQ(VCOM_WRITER_IRQn);
}
