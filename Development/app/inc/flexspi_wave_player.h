/**
* @brief   FRDM-MCXN947 board FLEXSPI PCM Player App
* @author  masa
* @version 1.00
*/

#ifndef __FLEXSPI_WAVE_PLAYER_H__
#define __FLEXSPI_WAVE_PLAYER_H__

#include "hcd_class_audio.h"
#include "wave.h"

typedef struct{
  uint8_t deviceIndex;
  uint32_t totalSize;
  uint32_t startPoint;
  uint32_t curPointer;
} wave_Player_Table_t;

void InitWavePlayer(uint32_t flashBase);

#endif /*__FLEXSPI_WAVE_PLAYER_H__*/
