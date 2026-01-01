/**
* @brief   FRDM-MCXN947 board Djent Cabinet for Musical Synthsizer App 
* @author  masa
* @version 1.00
*/

#ifndef __DJ_CAB_H__
#define __DJ_CAB_H__

#include "pq_synth.h"

#define DJCAB_NUM_OF_FILT   5
#define DJCAB_MAKEUP_GAIN (1.25f)

typedef struct{
  float b0;
  float b1;
  float b2;
  float a1;
  float a2;
} pqSynth_DjCab_BiquadCoef_t;

typedef struct{
  float z1;
  float z2;
} pqSynth_DjCab_BiquadState_t;

typedef struct{
  uint32_t bitMask;
  const pqSynth_DjCab_BiquadCoef_t coef[DJCAB_NUM_OF_FILT];
  pqSynth_DjCab_BiquadState_t state[SYNTH_MAX_VOICE][DJCAB_NUM_OF_FILT];
} pqSynth_DjCab_t;

void InitDjentCab(pqSynth_t* synth);

#endif /*__DJ_CAB_H__*/
