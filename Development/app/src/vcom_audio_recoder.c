/**
* @brief   FRDM-MCXN947 board USBD PCM Recorder App
* @author  masa
* @version 1.00
*/

#include "vcom_audio_recorder.h"

uint32_t st_RecordBuf[176];
const char st_Header[4] = {'P', 'Q', 'S', '1'};
uint8_t st_HeaderSentFlg = 0;
uint32_t st_currentPtr = 0;
static void (*playCallback)(uint8_t idx, uint32_t* buf, uint16_t nextTxSize);
static void (*noteOn)(uint8_t noteNum, uint8_t velocity);
static void (*noteOff)(uint8_t noteNum, uint8_t velocity);

uint8_t noteNum = 36;

static void vcomOutCallback(uint8_t comIdx, uint8_t* dataBuf, uint16_t transLen)
{
  NVIC_SetPendingIRQ(VcomRec_IRQn);
}

static void vcomInCallback(uint8_t comIdx, uint8_t* dataBuf, uint16_t transLen)
{
  NVIC_SetPendingIRQ(VcomRec_IRQn);
}

static void recHandler(void)
{
  NVIC_ClearPendingIRQ(VcomRec_IRQn);
  if (!st_HeaderSentFlg){
    if (*noteOn){
      noteOn(noteNum, 127);
    }
    DualVcom_StartInTransfer(0, st_Header, 4);
    st_HeaderSentFlg = 1;
  } else if (st_currentPtr < SAMPLE_COUNT){
		if (st_currentPtr == (SAMPLE_COUNT / 2)){
			if (noteOff){
				noteOff(noteNum, 0);
			}
		}
    if (playCallback){
      playCallback(1, st_RecordBuf, 176*4);
      DualVcom_StartInTransfer(0, st_RecordBuf, 176*4);
      st_currentPtr++;
    }
  }
}

void VcomRec_SetPlayCallback(void func(uint8_t idx, uint32_t* buf, uint16_t nextTxSize))
{
  playCallback = func;
}

void VcomRec_SetNoteOn(void func(uint8_t, uint8_t))
{
  noteOn = func;
}

void VcomRec_SetNoteOff(void func(uint8_t, uint8_t))
{
  noteOff = func;
}

void InitVcomRecorder(void)
{
  DualVcom_SetInCallBack(0, vcomInCallback);
  DualVcom_SetOutCallBack(0, vcomOutCallback);
  NVIC_SetVector(VcomRec_IRQn, (uint32_t)recHandler);
  NVIC_SetPriority(VcomRec_IRQn, 6);
  NVIC_EnableIRQ(VcomRec_IRQn);  
}
