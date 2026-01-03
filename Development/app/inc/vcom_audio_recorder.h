/**
* @brief   FRDM-MCXN947 board USBD PCM Recorder App
* @author  masa
* @version 1.00
*/

#ifndef __VCOM_AUDIO_RECORDER_H__
#define __VCOM_AUDIO_RECORDER_H__

#include "usbd_dual_vcom.h"

#define SAMPLE_UNIT       176
#define SAMPLE_COUNT      1000
#define VcomRec_IRQn      CTI0_IRQn

void InitVcomRecorder(void);
void VcomRec_SetPlayCallback(void func(uint8_t idx, uint32_t* buf, uint16_t nextTxSize));
void VcomRec_SetNoteOn(void func(uint8_t, uint8_t));
void VcomRec_SetNoteOff(void func(uint8_t, uint8_t));

#endif /*__VCOM_AUDIO_RECORDER_H__*/
