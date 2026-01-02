/**
* @brief   FRDM-MCXN947 board Voltage-Controled Oscillator for Musical Synthsizer App 
* @author  masa
* @version 1.00
*/

#ifndef __VCO_H__
#define __VCO_H__

#include "pq_synth.h"

typedef struct{
  float phaseStep;
  float phaseRadian;
} pqSynth_VCO_Voice_t;

typedef struct{
  uint32_t bitMask;
  pqSynth_VCO_Voice_t voice[SYNTH_MAX_VOICE];
}pqSynth_VCO_t;

void InitVCO(void);

#endif /*__VCO_H__*/
