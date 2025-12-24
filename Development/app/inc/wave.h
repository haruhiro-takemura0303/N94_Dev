/**
* @brief   Wave File Definition
* @author  masa
* @version 1.00
*/

#ifndef __WAVE_H__
#define __WAVE_H__

#include <stdint.h>

typedef struct{
	uint8_t riff_ckID[4];
	uint32_t riff_cksize;
	uint8_t waveID[4];
	uint8_t fmt_ckID[4];
	uint32_t fmt_cksize;
	uint16_t wFormatTag;
	uint16_t nChannels;
	uint32_t nSamplePerSec;
	uint32_t nAvgBytesPerSec;
	uint16_t nBlockAlign;
	uint16_t wBitsPerSample;
	uint8_t data_ckID[4];
	uint32_t data_cksize;
} wave_Head_t;

#endif /*__WAVE_H__*/
