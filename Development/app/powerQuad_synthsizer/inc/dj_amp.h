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
#define DJAMP_SS_ASYM_NEG_GAIN   (0.6f)   // <1.0 => stronger negative-side compression => more even harmonics
#define DJAMP_WS2_DRIVE    (5.0f)

#define DJAMP_PRE_HPF_HZ        (90.0f)
#define DJAMP_PRESENCE_LP1_HZ   (700.0f)
#define DJAMP_PRESENCE_LP2_HZ   (2500.0f)
#define DJAMP_PRESENCE_GAIN     (1.50f)
#define DJAMP_POST_LPF_HZ       (8000.0f)
#define DJAMP_ASYM_NEG_DRIVE    (1.60f)
#define DJAMP_HARD_CLIP         (0.85f)

#define DJAMP_ENABLE_DYNAMICS         1


typedef struct{
  float dcBlockX1;
  float dcBlockY1;
  float env;
  float sag;
  
  float postDcX1;
  float postDcY1;
  
  float preHpfX1;
  float preHpfY1;
  float presLp1;
  float presLp2;
  float postLp;
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
  float postDcCoef_R;
  
  float preHpf_a;
  float presLp1_a;
  float presLp2_a;
  float postLp_a;
  pqSynth_DjAmp_Voice_t voice[SYNTH_MAX_VOICE];
} pqSynth_DjAmp_t;

void InitDjentAmp(pqSynth_t* synth);

#endif /*__DJ_AMP_H__*/
