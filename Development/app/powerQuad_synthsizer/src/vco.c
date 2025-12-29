/**
* @brief   FRDM-MCXN947 board Voltage-Controled Oscillator for Musical Synthsizer App 
* @author  masa
* @version 1.00
*/

#include "vco.h"

static pqSynth_VCO_t st_VCO;
static float st_NoteFreqTbl[128];

static void initNoteFreqTbl(void)
{
  // MIDI note 0 (C-1) frequency in Hz (A4=440Hz, equal temperament)
  const float f0 = 8.175798915643707f;
  
  // Semitone ratio: 2^(1/12)
  const float r  = 1.0594630943592953f;
  
  float f = f0;
  for (int note = 0; note < 128; note++) {
    st_NoteFreqTbl[note] = f;
    f = f * r;
  }
}

static float note2Freq_Hz(uint8_t note)
{
  return st_NoteFreqTbl[note];
}

static float freq2PhaseStep(float freq_Hz, float samFreq_Hz)
{
  return 2.0f * (float)M_PI * (freq_Hz / samFreq_Hz);
}

static void preProc(pqSynth_t* synth, uint32_t frames)
{
  float freq;
  
  for (uint32_t v = 0; v < SYNTH_MAX_VOICE; v++){
    pqSynth_Voice_t* voice = &synth->voices[v];
    if (!voice->activeFlg){
      continue;
    }
    
    if (voice->noteEvPendFlg & st_VCO.bitMask){
      voice->noteEvPendFlg &= ~st_VCO.bitMask;
      if (voice->velocity){
        freq = note2Freq_Hz(voice->noteNum);
        st_VCO.voice[v].phaseStep = freq2PhaseStep(freq, synth->sampleRate);
        st_VCO.voice[v].phaseRadian = 0.0f;
      } 
    }
  }
}

static void play(pqSynth_t* synth, uint32_t frames)
{
  float* voicePtr;
  float* angularBuf_v;
  float* angularBuf = &synth->gpWorkMem0[0];
  float* sineBuf = &synth->gpWorkMem1[0];
  float curPhase, step;
  
  for (int v = 0; v < SYNTH_MAX_VOICE; v++){
    angularBuf_v = &angularBuf[v * frames];
    
    if (!synth->voices[v].activeFlg){
      for (int i = 0; i < frames; i++){
        angularBuf_v[i] = 0.0f;
      }
      continue;
    }
    curPhase = st_VCO.voice[v].phaseRadian;
    step = st_VCO.voice[v].phaseStep;
    for (int i = 0; i < frames; i++){
      angularBuf_v[i] = curPhase;
      curPhase += step;
    }
    // Keep phase bounded
    if (curPhase > 8.0f * (float)M_PI){
      curPhase = fmodf(curPhase, 2.0f * (float)M_PI);
    }
    st_VCO.voice[v].phaseRadian = curPhase;
  }
  
  PQ_VectorSinF32(angularBuf, sineBuf, frames*SYNTH_MAX_VOICE);
  
  // Copy to voice buffers
  for (int v = 0; v < SYNTH_MAX_VOICE; v++){
    voicePtr = &synth->voiceBuf[v][0];
    
    if (!synth->voices[v].activeFlg){
      for (int i = 0; i < frames; i++){
        voicePtr[i] = 0.0f;
      }
      continue;
    }
    
    for (int i = 0; i < frames; i++){
      voicePtr[i] = sineBuf[v * frames + i];
    }
  }
}

void InitVCO(void)
{
  uint32_t bitPtn;
  pqSynth_Module_t module;
  
  initNoteFreqTbl();
  
  module.name = "VCO";
  module.play = play;
  module.preProc = preProc;
  bitPtn = PQSynth_RegisterModule(&module);
  if (bitPtn != 0xFFFFFFFF){
    st_VCO.bitMask = bitPtn;
  }
}
