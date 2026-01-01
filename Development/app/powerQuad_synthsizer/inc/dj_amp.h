/**
* @brief   FRDM-MCXN947 board Djent Amp for Musical Synthsizer App 
* @author  masa
* @version 1.00
*/

#ifndef __DJ_AMP_H__
#define __DJ_AMP_H__

#include "pq_synth.h"

#define DJAMP_SHAPERLUT_SIZE   (1024)
#define DJAMP_SHAPERLUT_XMAX   (4.000000f)
#define DJAMP_SHAPERLUT_YMAX   (0.6666667f) // 2/3

#define DJAMP_ENABLE_DYNAMICS         1


typedef struct{
  float dcBlockX1;
  float dcBlockY1;
  float env;
  float sag;
} pqSynth_DjAmp_Voice_t;

typedef struct{
  uint32_t bitMask;
  float driveBase;
  float postGain;
  float drvEnvAmt;
  float sagAmt;
  float biasAmt;
  float envAlpha;
  float sagAlpha;
  float dcBlockCoef_R;
  pqSynth_DjAmp_Voice_t voice[SYNTH_MAX_VOICE];
} pqSynth_DjAmp_t;

void InitDjentAmp(pqSynth_t* synth);

#endif /*__DJ_AMP_H__*/
