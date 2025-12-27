/**
* @brief   FRDM-MCXN947 board Voltage-Controled Amplifier for Musical Synthsizer App 
* @author  masa
* @version 1.00
*/

#include "vca.h"

pqSynth_VCA_t st_VCA;

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

static void preProc(pqSynth_t* synth, uint32_t frames)
{
  pqSynth_Voice_t* voice;
  float fVelo;
  
  for (int v = 0; v < SYNTH_MAX_VOICE; v++){
    voice = &synth->voices[v];
    if (!voice->activeFlg){
      continue;
    }
    if (voice->noteEvPendFlg & st_VCA.bitMask){
      voice->noteEvPendFlg &= ~st_VCA.bitMask;
      if (voice->velocity){
        /*Note On*/
        st_VCA.gate[v] = 1;
        fVelo = clamp01((float)voice->velocity / 127.0f);
        st_VCA.target[v] = synth->ampMax * fVelo;
        st_VCA.amp[v] = 0.0f;
      } else {
        /*Note Off*/
        st_VCA.gate[v] = 0;
      }
    }
  }
}

static void play(pqSynth_t* synth, uint32_t frames)
{
  float* buf;
  float amp;
  
  for (int v = 0; v < SYNTH_MAX_VOICE; v++){
    if (!synth->voices[v].activeFlg){
      continue;
    }
    buf = &synth->voiceBuf[v][0];
    amp = st_VCA.amp[v];
    for (int i = 0; i < frames; i++){
      if (st_VCA.gate[v]){
        amp += st_VCA.stepOn;
        if (amp > st_VCA.target[v]){
          amp = st_VCA.target[v];
        }
      } else {
        amp -= st_VCA.stepOff;
        if (amp < 0.0f){
          amp = 0.0f;
        }
      }
      buf[i] = buf[i]*amp;
    }
    st_VCA.amp[v] = amp;
    if (!st_VCA.gate[v] && (st_VCA.amp[v] == 0.0f)){
      synth->voices[v].activeFlg = 0;
    }
  }
}

void InitVCA(pqSynth_t* synth)
{
  uint32_t bitPtn;
  pqSynth_Module_t module;
  float attackSam, relSam;

  module.name = "VCA";
  module.play = play;
  module.preProc = preProc;
  bitPtn = PQSynth_RegisterModule(&module);
  if (bitPtn != 0xFFFFFFFF){
    st_VCA.bitMask = bitPtn;
  }
  attackSam = 0.005f * synth->sampleRate;
  relSam = 0.005f * synth->sampleRate;

  st_VCA.stepOn = (attackSam > 1.0f) ? (synth->ampMax / attackSam) : synth->ampMax;
  st_VCA.stepOff = (relSam > 1.0f) ? (synth->ampMax / relSam) : synth->ampMax;

  for (int v = 0; v < SYNTH_MAX_VOICE; v++){
    st_VCA.gate[v] = 0u;
    st_VCA.amp[v] = 0.0f;
    st_VCA.target[v] = 0.0f;
  }
}
