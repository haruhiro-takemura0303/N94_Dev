/**
* @brief   FRDM-MCXN947 board Kerplus-Strong Model Oscillator for Musical Synthsizer App 
* @author  masa
* @version 1.00
*/

#ifndef __VCO_KS_H__
#define __VCO_KS_H__

#include "pq_synth.h"

#define KS_MAX_DELAY_SAMPLES    4096
#define KS_USE_LP               0
#define KS_PICK_OFFSET_DIV      12
#define KS_PICK_MIX             0.75f
#define KS_PICKBUF_SIZE         512
#define KS_MUTE_TIME_SEC        0.060f
#define KS_MUTE_LP_ALPHA        0.45f
#define KS_MUTE_MIX_MAX         0.85f

typedef struct{
  uint32_t delayLen;
  uint32_t index;
  float decayCoef;
  float lastSample;
  struct{
    uint8_t enable;
    float z;
    float a;
  }allpass;
  uint16_t quietCount;
  uint16_t releasing;

  float muteEnv;
  float muteLP;
  float pickBuf[KS_PICKBUF_SIZE];
  uint32_t pickWp;
  uint32_t pickOfs;
} pqSynth_KS_Voice_t;

typedef struct{
  uint32_t bitMask;
  pqSynth_KS_Voice_t voice[SYNTH_MAX_VOICE];
  float delayLine[SYNTH_MAX_VOICE][KS_MAX_DELAY_SAMPLES];
} pqSynth_KS_t;

void InitVCOKerplusStrong(pqSynth_t* synth);

#endif /*__VCO_KS_H__*/
