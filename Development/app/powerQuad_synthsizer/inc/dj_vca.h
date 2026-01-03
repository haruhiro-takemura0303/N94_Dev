/**
* @brief   FRDM-MCXN947 board Djent-Specific Voltage-Controled Amplifier for Musical Synthsizer App 
* @author  masa
* @version 1.00
*/

#ifndef __DJ_VCA_H__
#define __DJ_VCA_H__

#include "pq_synth.h"

typedef enum{
  REL_WAIT_ZC = 0,
  REL_RELEASE = 1,
}pqSynth_DjVCA_RelState_t;

typedef struct{
  uint8_t gate;
  pqSynth_DjVCA_RelState_t relState;
  uint16_t zcCount;
  float amp;
  float target;
} pqSynth_DjVCA_Voice_t;

typedef struct{
  uint32_t bitMask;
  pqSynth_DjVCA_Voice_t voice[SYNTH_MAX_VOICE];
  float relMultiFast;
  float relThres;
  float zcThres;
  uint16_t zcHoldMax;
} pqSynth_DjVCA_t;

void InitDjentVCA(pqSynth_t* synth);

#endif /*__DJ_VCA_H__*/
