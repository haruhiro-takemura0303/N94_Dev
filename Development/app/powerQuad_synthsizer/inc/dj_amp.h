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

#define DJAMP_PRE_HPF_HZ        (65.0f)
#define DJAMP_PRESENCE_LP1_HZ   (2000.0f)
#define DJAMP_PRESENCE_LP2_HZ   (6500.0f)
#define DJAMP_PRESENCE_GAIN     (0.60f)

#define DJAMP_BITE_PEAK_HZ        (4500.0f)
#define DJAMP_BITE_PEAK_Q         (2.30f)
#define DJAMP_BITE_PEAK_GAIN_DB   (11.0f)

#define DJAMP_MID_PEAK_HZ         (3700.0f)
#define DJAMP_MID_PEAK_Q          (1.25f)
#define DJAMP_MID_PEAK_GAIN_DB    (3.5f)

#define DJAMP_BODY_LP_HZ          (160.0f)
#define DJAMP_BODY_SHELF_GAIN_DB  (0.8f)
#define DJAMP_POST_LPF_HZ         (5800.0f)
#define DJAMP_ASYM_NEG_DRIVE      (1.60f)
#define DJAMP_HARD_CLIP           (0.85f)

#define DJAMP_ENABLE_DYNAMICS         1

enum{
  ENV_MODE_ATK_HOLD_DECAY = 1,
  ENV_MODE_LEGACY
};

enum{
  SAG_MODE_OFF = 0,
  SAG_MODE_FAST_TRACK,
  SAG_MODE_LEGACY
};

enum{
  POSTDC_PLACE_END = 0,
  POSTDC_PLACE_PRE_POSTLPF
};


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
  float postLp2;

  float biteZ1;
  float biteZ2;

  float midZ1;
  float midZ2;

  float bodyLp;

  float envPeak;
  uint32_t envHoldCount;
  float driveSm;
  float biasSm;
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

  float inGain;
  float outGain;

  uint32_t envMode;
  uint32_t sagMode;

  float envHoldMs;
  float envDecayCoef;
  uint32_t envHoldSamples;

  float driveMin;
  float driveMax;
  float biasMin;
  float biasMax;

  float driveSmthAlpha;
  float biasSmthAlpha;

  uint32_t enPreDc;
  uint32_t postDcPlace;
  
  float preHpf_a;
  float presLp1_a;
  float presLp2_a;
  float postLp_a;

  struct{
    float b0;
    float b1;
    float b2;
    float a1;
    float a2;
  } biteCoef;

  struct{
    float b0;
    float b1;
    float b2;
    float a1;
    float a2;
  } midCoef;

  float bodyLp_a;
  float bodyShelfGainLin_Minus1;
  pqSynth_DjAmp_Voice_t voice[SYNTH_MAX_VOICE];
} pqSynth_DjAmp_t;

void InitDjentAmp(pqSynth_t* synth);

#endif /*__DJ_AMP_H__*/
