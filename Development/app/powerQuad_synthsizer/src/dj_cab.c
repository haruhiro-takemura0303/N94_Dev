/**
* @brief   FRDM-MCXN947 board Djent Cabinet for Musical Synthsizer App 
* @author  masa
* @version 1.00
*/

#include "dj_cab.h"

pqSynth_DjCab_t st_DjCab = {
  .bitMask = 0,
  .coef[0] = { 0.99197156f, -1.98394310f, 0.99197156f, -1.98387861f, 0.98400754f },
  .coef[1] = { 1.00370347f, -1.97088110f, 0.96768969f, -1.97088110f, 0.97139323f },
  .coef[2] = { 0.84717423f, -1.27877641f, 0.64500850f, -1.27877641f, 0.49218276f },
  .coef[3] = { 0.12746720f,  0.25493440f, 0.12746720f, -0.76787388f, 0.27774271f },
  .coef[4] = { 0.12746720f,  0.25493440f, 0.12746720f, -0.76787388f, 0.27774271f },
  .state = {0}
};

static inline float biquadProcDF2T(const pqSynth_DjCab_BiquadCoef_t* coef, pqSynth_DjCab_BiquadState_t* state, float x)
{
  float y, z1, z2;
  y = (coef->b0 * x) + state->z1;
  z1 = (coef->b1 * x) - (coef->a1 * y) + state->z2;
  z2 = (coef->b2 * x) - (coef->a2 * y);
  state->z1 = z1;
  state->z2 = z2;
  return y;
}

static void preProc(pqSynth_t* synth, uint32_t frames)
{
  for (int v = 0; v < SYNTH_MAX_VOICE; v++){
    if (synth->voices[v].noteEvPendFlg & st_DjCab.bitMask){
      synth->voices[v].noteEvPendFlg &= ~st_DjCab.bitMask;
      if (synth->voices[v].velocity != 0){
        for (int k = 0; k < DJCAB_NUM_OF_FILT; k++){
          st_DjCab.state[v][k].z1 = 0.0f;
          st_DjCab.state[v][k].z2 = 0.0f;
        }
      }
    }
  }
}

static void play(pqSynth_t* synth, uint32_t frames)
{
  float* buf;
  float x;
  
  for (int v = 0; v < SYNTH_MAX_VOICE; v++){
    if (!synth->voices[v].activeFlg){
      continue;
    }
    buf = &synth->voiceBuf[v][0];
    for (int i = 0; i < frames; i++){
      x = buf[i];
      for (int k = 0; k < DJCAB_NUM_OF_FILT; k++){
        x = biquadProcDF2T(&st_DjCab.coef[k], &st_DjCab.state[v][k], x);
      }
      buf[i] = x * DJCAB_MAKEUP_GAIN;
    }
  } 
}

void InitDjentCab(pqSynth_t* synth)
{
  uint32_t bitPtn;
  pqSynth_Module_t module;
  
  module.name = "Dj_CAB";
  module.play = play;
  module.preProc = preProc;
  bitPtn = PQSynth_RegisterModule(&module);
  if (bitPtn != 0xFFFFFFFF){
    st_DjCab.bitMask = bitPtn;
  }
}
