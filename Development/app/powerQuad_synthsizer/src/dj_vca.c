/**
* @brief   FRDM-MCXN947 board Djent-Specific Voltage-Controled Amplifier for Musical Synthsizer App 
* @author  masa
* @version 1.00
*/

#include "dj_vca.h"

pqSynth_DjVCA_t st_DjVCA;

static float clamp01(float x)
{
  if (x < 0.0f){
    return 0.0f;
  }
  if (x > 1.0f) {
    return 1.0f;
  }
  return x;
}

static float absf(float x)
{
  return (x < 0.0f) ? -x : x;
}

static void preProc(pqSynth_t* synth, uint32_t frames)
{
  pqSynth_Voice_t* voice;
  float fVelo;
  
  for (int v = 0; v < SYNTH_MAX_VOICE; v++){
    voice = &synth->voices[v];
    if (!voice->activeFlg){
      continue;
    }
    if (voice->noteEvPendFlg & st_DjVCA.bitMask){
      voice->noteEvPendFlg &= ~st_DjVCA.bitMask;
      if (voice->velocity){
        /*Note On*/

        st_DjVCA.voice[v].gate = 1;
        st_DjVCA.voice[v].relState = REL_WAIT_ZC;
        st_DjVCA.voice[v].zcCount = 0;

        fVelo = clamp01((float)voice->velocity / 127.0f);
        st_DjVCA.voice[v].target = synth->ampMax * fVelo;
        st_DjVCA.voice[v].amp = st_DjVCA.voice[v].target;
      } else {
        /*Note Off*/
        st_DjVCA.voice[v].gate = 0;
        st_DjVCA.voice[v].relState = REL_WAIT_ZC;
        st_DjVCA.voice[v].zcCount = 0;
      }
    }
  }
}

static void play(pqSynth_t* synth, uint32_t frames)
{
  float* buf;
  float amp;
  pqSynth_DjVCA_Voice_t* aVoice;
  
  for (int v = 0; v < SYNTH_MAX_VOICE; v++){
    if (!synth->voices[v].activeFlg){
      continue;
    }
    buf = &synth->voiceBuf[v][0];
    aVoice = &st_DjVCA.voice[v];
    amp = aVoice->amp;

    if (aVoice->gate){
      amp = aVoice->target;
      for (int i = 0; i < frames; i++){
        buf[i] = buf[i] * amp;
      }
      aVoice->amp = amp;
      continue;
    }

    for (int i = 0; i < frames; i++){
      if (aVoice->relState == REL_WAIT_ZC){
        aVoice->zcCount++;
        if ((absf(buf[i]) <= st_DjVCA.zcThres) || (aVoice->zcCount >= st_DjVCA.zcHoldMax)){
          aVoice->relState = REL_RELEASE;
        }
      }
      if (aVoice->relState == REL_RELEASE){
        amp = amp * st_DjVCA.relMultiFast;
        if (amp < st_DjVCA.relThres){
          amp = 0.0f;
        }
      }
      buf[i] = buf[i]*amp;
    }

    aVoice->amp = amp;
    if (aVoice->amp == 0.0f){
      synth->voices[v].activeFlg = 0u;
    }
  }
}

void InitDjentVCA(pqSynth_t* synth)
{
  uint32_t bitPtn;
  pqSynth_Module_t module;
  float relMs, relEps, relSam, relSamInv, zcHoldSam;

  module.name = "Dj_VCA";
  module.play = play;
  module.preProc = preProc;
  bitPtn = PQSynth_RegisterModule(&module);
  if (bitPtn != 0xFFFFFFFF){
    st_DjVCA.bitMask = bitPtn;
  }

  relMs = 6.0f;
  relEps = 1.0e-4f;
  relSam = (relMs * 0.001f) * synth->sampleRate;
  if (relSam < 1.0f){
    relSamInv = 1.0f / relSam;
  }
  st_DjVCA.relMultiFast = expf(logf(relEps) * relSamInv);

  st_DjVCA.relThres = synth->ampMax * 1.0e-5f;

  st_DjVCA.zcThres = 0.0025f;

  zcHoldSam = (1.0f * 0.001f) * synth->sampleRate;
  if (zcHoldSam < 1.0f){
   zcHoldSam = 1.0f; 
  }
  if (zcHoldSam > 65535.0f){
   zcHoldSam = 65535.0f; 
  }
  st_DjVCA.zcHoldMax = (uint16_t)zcHoldSam;

  
  for (int v = 0; v < SYNTH_MAX_VOICE; v++){
    st_DjVCA.voice[v].gate = 0u;
    st_DjVCA.voice[v].relState = REL_WAIT_ZC;
    st_DjVCA.voice[v].zcCount = 0;
    st_DjVCA.voice[v].amp = 0.0f;
    st_DjVCA.voice[v].target = 0.0f;
  }
}
