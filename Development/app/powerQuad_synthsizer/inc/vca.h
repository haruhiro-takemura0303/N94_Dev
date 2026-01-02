/**
* @brief   FRDM-MCXN947 board Voltage-Controled Amplifier for Musical Synthsizer App 
* @author  masa
* @version 1.00
*/

#ifndef __VCA_H__
#define __VCA_H__

#include "pq_synth.h"

typedef struct{
  uint32_t bitMask;
  uint8_t gate[SYNTH_MAX_VOICE];
  float amp[SYNTH_MAX_VOICE];
  float target[SYNTH_MAX_VOICE];
  float stepOn;
  float relMult;
  float relThres;
}pqSynth_VCA_t;

void InitVCA(pqSynth_t* synth);

#endif /*__VCA_H__*/
