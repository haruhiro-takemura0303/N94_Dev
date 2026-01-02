/**
* @brief   FRDM-MCXN947 board Djent Gate for Musical Synthsizer App 
* @author  masa
* @version 1.00
*/

#include "dj_gate.h"

pqSynth_DjGate_t st_DjGate;

static inline float alphaFromTau(float tau_sec, float samFreq)
{
  if (tau_sec <= 0.0f){
    return 1.0f;
  }
  return 1.0f - expf(-1.0f / (tau_sec * samFreq));
}

static void preProc_Key(pqSynth_t* synth, uint32_t frames)
{
  pqSynth_Voice_t* voice;
  pqSynth_DjGate_Voice_t* gVoice;
  
  for (int v = 0; v < SYNTH_MAX_VOICE; v++){
    voice = &synth->voices[v];
    if (voice->noteEvPendFlg & st_DjGate.key_bitMask){
      voice->noteEvPendFlg &= ~st_DjGate.key_bitMask;
      if (voice->velocity){
        gVoice = &st_DjGate.voice[v];
        gVoice->env = 0.0f;
        gVoice->gain = 1.0f;
        gVoice->holdCount = st_DjGate.holdSam;
      }
    }
  }
}

static void play_Key(pqSynth_t* synth, uint32_t frames)
{
  float* buf;
  float env, gain, a;
  uint32_t hold;
  uint8_t open;
  pqSynth_DjGate_Voice_t* gVoice;
  
  for (int v = 0; v < SYNTH_MAX_VOICE; v++){
    if (!synth->voices[v].activeFlg){
      continue;
    }
    buf = &synth->voiceBuf[v][0];
    gVoice = &st_DjGate.voice[v];
    env = gVoice->env;
    gain = gVoice->gain;
    hold = gVoice->holdCount;
    
    for (int i = 0; i < frames; i++){
			if (synth->voices[v].velocity != 0){
				hold = st_DjGate.holdSam;
			}
      a = fabsf(buf[i]);
      env = env + st_DjGate.envAlpha * (a - env);
      if (env >= st_DjGate.thresOpen){
        hold = st_DjGate.holdSam;
      } else if (env <= st_DjGate.thresClose){
        if (hold > 0){
          hold--;
        }
      }
      
      if (hold > 0){
        open = 1;
      } else {
        open = 0;
      }
      if (open){
        gain = gain + st_DjGate.atkAlpha * (1.0f - gain);
      } else {
        gain = gain + st_DjGate.relAlpha * (0.0f - gain);
      }
      gVoice->gateGainBuf[i] = gain;
    }
    gVoice->env = env;
    gVoice->gain = gain;
    gVoice->holdCount = hold;
  }
}

static void preProc_Apply(pqSynth_t* synth, uint32_t frames)
{
  for (int v = 0; v < SYNTH_MAX_VOICE; v++){
    if (synth->voices[v].noteEvPendFlg & st_DjGate.apply_bitMask){
      synth->voices[v].noteEvPendFlg &= ~st_DjGate.apply_bitMask;
    }
  }  
}

static void play_Apply(pqSynth_t* synth, uint32_t frames)
{
  float* buf;
  float* gainBuf;
  for (int v = 0; v < SYNTH_MAX_VOICE; v++){
    if (!synth->voices[v].activeFlg){
      continue;
    }
    buf = &synth->voiceBuf[v][0];
    gainBuf = &st_DjGate.voice[v].gateGainBuf[0];
    for (int i = 0; i < frames; i++){
      buf[i] = buf[i] * gainBuf[i];
    }
  }
}

void InitDjentGate_Key(pqSynth_t* synth)
{
  uint32_t bitPtn;
  pqSynth_Module_t module;
  float fs;
  fs = synth->sampleRate;
  st_DjGate.envAlpha = alphaFromTau(0.002f, fs);
  st_DjGate.atkAlpha = alphaFromTau(0.0002f, fs);
  st_DjGate.relAlpha = alphaFromTau(0.025f, fs);
  st_DjGate.holdSam = (uint32_t)(0.010f * fs);
  if (st_DjGate.holdSam < 1){
    st_DjGate.holdSam = 1;
  }
  st_DjGate.thresOpen = 0.020f;
  st_DjGate.thresClose = 0.015f;

  for (int v = 0; v < SYNTH_MAX_VOICE; v++){
    st_DjGate.voice[v].env = 0.0f;
    st_DjGate.voice[v].gain = 1.0f;
    st_DjGate.voice[v].holdCount = 0;
  }
  
  module.name = "Dj_GATE_Key";
  module.play = play_Key;
  module.preProc = preProc_Key;
  bitPtn = PQSynth_RegisterModule(&module);
  if (bitPtn != 0xFFFFFFFF){
    st_DjGate.key_bitMask = bitPtn;
  }    
}

void InitDjentGate_Apply(pqSynth_t* synth)
{
  uint32_t bitPtn;
  pqSynth_Module_t module;

  module.name = "Dj_GATE_Apply";
  module.play = play_Apply;
  module.preProc = preProc_Apply;
  bitPtn = PQSynth_RegisterModule(&module);
  if (bitPtn != 0xFFFFFFFF){
    st_DjGate.apply_bitMask = bitPtn;
  }
}
