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

#define KS_ATK_WIN_MS           18.0f

#define KS_ATK_PITCH_EN         1
#define KS_ATK_WIN_CYCLES       2.0f
#define KS_ATK_WIN_MIN_MS       8.0f
#define KS_ATK_WIN_MAX_MS       25.0f

#define KS_BURST_INJECT_MS      3.5f
#define KS_BURST_GAIN           0.12f
#define KS_CLICK_HP_GAIN        0.70f
#define KS_THUMP_LP_GAIN        0.35f
#define KS_THUMP_LP_ALPHA       0.18f
#define KS_PICK_MIX_ATK_BOOST   0.45f
#define KS_PICK_OFS_DIV_VAR     6.0f

#define KS_BURST_TAU_MS         1.0f
#define KS_BURST_TAU_SEC        ((float)KS_BURST_TAU_MS * 0.001f)

#define KS_HFLOSS_BASE          0.040f
#define KS_HFLOSS_ATK_RED       0.65f
#define KS_HFLOSS_LP_ALPHA      0.10f

#define KS_DISP_EN              1
#define KS_DISP_AP_A            0.55f
#define KS_REL_MUTE_MS          6.0f
#define KS_REL_EX_DECAY         0.94f

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

  uint32_t ageSamples;
  uint32_t atkWinSamples;
  float invAtkWin;
  uint32_t burstRem;
  float burstEnv;
  float burstDecay;
  float thumpLP;
  float hfLP;
  struct{
    uint8_t en;
    float z;
    float a;
  }dispAp;

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
