/**
* @brief   FRDM-MCXN947 board Djent Gate for Musical Synthsizer App 
* @author  masa
* @version 1.00
*/

#ifndef __DJ_GATE_H__
#define __DJ_GATE_H__

#include "pq_synth.h"

typedef struct{
  float env;
  float gain;
  uint32_t holdCount;
  float gateGainBuf[SYNTH_SAMPLE_BUF_SIZE];
}pqSynth_DjGate_Voice_t;

typedef struct{
  uint32_t key_bitMask;
  uint32_t apply_bitMask;
  pqSynth_DjGate_Voice_t voice[SYNTH_MAX_VOICE];
  float envAlpha;
  float thresOpen;
  float thresClose;
  float atkAlpha;
  float relAlpha;
  uint32_t holdSam;
} pqSynth_DjGate_t;

void InitDjentGate_Key(pqSynth_t* synth);
void InitDjentGate_Apply(pqSynth_t* synth);

#endif /*__DJ_GATE_H__*/
