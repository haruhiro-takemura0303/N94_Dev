/**
* @brief   FRDM-MCXN947 board Voltage-Controled Oscillator for Musical Synthsizer App 
* @author  masa
* @version 1.00
*/

#include "vco_wavetbl.h"

static pqSynth_VCOWT_t st_VCOWT;
static float st_NoteFreqTbl[128];

static float st_WaveTable[VCOWT_WAVETABLE_SIZE];
static float st_AngleTbl[VCOWT_WAVETABLE_SIZE];
static float st_SineTbl[VCOWT_WAVETABLE_SIZE];

static const float st_HarmAmp[VCOWT_NUM_OF_HARMONICS] = {
  1.0000000000f, // 1st
  0.5000000000f, // 2nd
  0.3333333333f, // 3rd
  0.2500000000f, // 4th
  0.2000000000f, // 5th
  0.1666666667f, // 6th
  0.1428571429f, // 7th
  0.1250000000f  // 8th
};

static void initNoteFreqTbl(void)
{
  const float ratio = 1.0594630943592952646f;
  st_NoteFreqTbl[69] = 440.0f;
  
  for (int n = 70; n < 128; n++){
    st_NoteFreqTbl[n] = st_NoteFreqTbl[n - 1] * ratio;
  }
  for (int n = 68; n >= 0; n--){
    st_NoteFreqTbl[n] = st_NoteFreqTbl[n + 1] / ratio;
  }
}

static float note2Freq_Hz(uint8_t note)
{
  return st_NoteFreqTbl[(note & 0x7F)];
}

static uint32_t freq2PhaseInc(float freq_Hz, float samFreq_Hz)
{
  const double scale = 4294967296.0;
  return (uint32_t)((freq_Hz * scale) / samFreq_Hz);
}

static void initWaveTable(void)
{
  float peak;
  
  for (int i = 0; i < VCOWT_WAVETABLE_SIZE; i++){
    st_WaveTable[i] = 0.0f;
  }
  
  for (int k = 0; k < VCOWT_NUM_OF_HARMONICS; k++){
    const float amp = st_HarmAmp[k];
    const float mult = (float)(k + 1);
    for (int i = 0; i < VCOWT_WAVETABLE_SIZE; i++){
      st_AngleTbl[i] = mult * (2.0f * (float)M_PI) * ((float)i / (float)VCOWT_WAVETABLE_SIZE);
    }
    
    PQ_VectorSinF32(st_AngleTbl, st_SineTbl, VCOWT_WAVETABLE_SIZE);
    
    for (int i = 0; i < VCOWT_WAVETABLE_SIZE; i++){
      st_WaveTable[i] += amp * st_SineTbl[i];
    }
  }
  
  peak = 0.0f;
  for (int i = 0; i < VCOWT_WAVETABLE_SIZE; i++){
    const float a = fabsf(st_WaveTable[i]);
    if (a > peak){
      peak = a;
    }
  }
  
  if (peak > 0.0f){
    const float inv = 1.0 / peak;
    for (int i = 0; i < VCOWT_WAVETABLE_SIZE; i++){
      st_WaveTable[i] = st_WaveTable[i] * inv;
    }
  }
  
}

static float waveTblLerp(uint32_t phase)
{
  const uint32_t i0 = VCOWT_PHASE_TO_INDEX(phase);
  const uint32_t i1 = (i0 + 1) & (VCOWT_WAVETABLE_SIZE - 1);
  
  const float frac = VCOWT_FRAC_TO_FLOAT(phase);
  const float a = st_WaveTable[i0];
  const float b = st_WaveTable[i1];
  
  return (a + (b - a) * frac);
}

static void preProc(pqSynth_t* synth, uint32_t frames)
{
  float freq;
  
  for (uint32_t v = 0; v < SYNTH_MAX_VOICE; v++){
    pqSynth_Voice_t* voice = &synth->voices[v];
    if (!voice->activeFlg){
      continue;
    }
    
    if (voice->noteEvPendFlg & st_VCOWT.bitMask){
      voice->noteEvPendFlg &= ~st_VCOWT.bitMask;
      if (voice->velocity){
        freq = note2Freq_Hz(voice->noteNum);
        st_VCOWT.voice[v].phaseInc = freq2PhaseInc(freq, synth->sampleRate);
        st_VCOWT.voice[v].phaseFixedFrac = 0u;
      } 
    }
  }
}

static void play(pqSynth_t* synth, uint32_t frames)
{
  uint32_t phase, inc;
  for (int v = 0; v < SYNTH_MAX_VOICE; v++)
  {
    float* buf = &synth->voiceBuf[v][0];
    if (!synth->voices[v].activeFlg){
      for (int i = 0; i < frames; i++){
        buf[i] = 0.0f;
      }
      continue;
    }
    phase = st_VCOWT.voice[v].phaseFixedFrac;
    inc = st_VCOWT.voice[v].phaseInc;
    for (int i = 0; i < frames; i++){
      buf[i] = waveTblLerp(phase);
      phase += inc;
    }
    st_VCOWT.voice[v].phaseFixedFrac = phase;
  }
}

void InitVCOWaveTable(pqSynth_t* synth)
{
  uint32_t bitPtn;
  pqSynth_Module_t module;
  
  initNoteFreqTbl();
  initWaveTable();
  
  module.name = "VCO_WT";
  module.play = play;
  module.preProc = preProc;
  bitPtn = PQSynth_RegisterModule(&module);
  if (bitPtn != 0xFFFFFFFF){
    st_VCOWT.bitMask = bitPtn;
  }
  
  for (int v = 0; v < SYNTH_MAX_VOICE; v++){
    st_VCOWT.voice[v].phaseFixedFrac = 0u;
    st_VCOWT.voice[v].phaseInc = freq2PhaseInc(440.0f, synth->sampleRate);
  }
  
}
