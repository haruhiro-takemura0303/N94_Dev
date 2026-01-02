/**
* @brief   FRDM-MCXN947 board Kerplus-Strong Model Oscillator for Musical Synthsizer App 
* @author  masa
* @version 1.00
*/

#include "vco_ks.h"

static pqSynth_KS_t st_KS;
static float st_NoteFreqTbl[128];
static uint32_t st_Rng = 0x12345678u;
float st_MuteEnvStep;

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

static uint32_t clampU32(uint32_t v, uint32_t vmin, uint32_t vmax)
{
    if (v < vmin){
      return vmin;
    }
    if (v > vmax) {
      return vmax;
    }
    return v;
}

static uint32_t rngU32(void)
{
  st_Rng = (1664525u * st_Rng) + 1013904223u;
  return st_Rng;
}

static float rngF32Singed(void)
{
  const uint32_t r = rngU32();
  const float u = (float)(r >> 8) * (1.0f / 16777215.0f);
  return (u * 2.0f) - 1.0f;
}

static inline float allpassProc(float* z1, float x, float a)
{
  float y = (*z1) + (a * x);
  *z1 = x - (a * y);
  return y;
}

static inline float frac2AllpassCoef_a(float delta)
{
  const float epsilon = 1.0e-6f;
  if (delta < 0.0f){
    delta = 0.0f;
  }
  if (delta > 0.999999f){
    delta = 0.999999f;
  }
  float a = (1.0f - delta) / (1.0f + delta + epsilon);
  if (a > 0.9999f){
    a = 0.9999f;
  }
  if (a < -0.9999f){
    a = -0.9999f;
  }
  return a;
}

static void resetVoice(uint32_t v)
{
  st_KS.voice[v].delayLen = 0;
  st_KS.voice[v].index = 0;
  st_KS.voice[v].decayCoef = 0.0f;
  st_KS.voice[v].lastSample = 0.0f;
  st_KS.voice[v].quietCount = 0;
  st_KS.voice[v].releasing = 0;

  st_KS.voice[v].allpass.enable = 0;
  st_KS.voice[v].allpass.z = 0.0f;
  st_KS.voice[v].allpass.a = 0.0f;

  memset(&st_KS.delayLine[v][0], 0, sizeof(st_KS.delayLine[v]));
}


static void noteOn(pqSynth_t* synth, uint32_t v, uint8_t noteNum, uint8_t velocity)
{ 
  float freq, D, frac, decay, vel, n0, n1, lp, g, x, m_up, baseExc, delayed;
  uint32_t N0, N, wp;

  freq = note2Freq_Hz(noteNum);
  if (freq < 1.0f){
    resetVoice(v);
    return;
  }
  D = synth->sampleRate / freq;
  N0 = (uint32_t)floorf(D);
  frac = D - (float)N0;

  N = clampU32(N0, 2, KS_MAX_DELAY_SAMPLES - 2);
  if (N != N0){
    frac = 0.0f;
  }

  decay = 0.996f;
  if (N < 128){
    decay = 0.992f;
  } else if (N < 256){
    decay = 0.994f;
  } else if (N < 512){
    decay = 0.996f;
  } else {
    decay = 0.997f;
  }
  st_KS.voice[v].delayLen = N;
  st_KS.voice[v].index = 0;
  st_KS.voice[v].decayCoef = decay;
  st_KS.voice[v].lastSample = 0.0f;
  st_KS.voice[v].quietCount = 0;
  st_KS.voice[v].releasing = 0;
  st_KS.voice[v].allpass.z = 0.0f;
  st_KS.voice[v].allpass.a = frac2AllpassCoef_a(frac);
  st_KS.voice[v].allpass.enable = (frac > 1.0e-4f) ? 1 : 0;

  vel = (float)velocity * (1.0f / 127.0f);
#if KS_USE_LP
  lp = 0.0f;
  g = 2.0f * freq / synth->sampleRate;
  if (g < 0.0f){
    g = 0.0f;
  }
  if (g > 0.25f){
    g = 0.25f;
  }

  if (N >= 512){
    m_up = 3.5f;
  } else if (N >= 256){
    m_up = 3.0f;
  } else if (N >= 128){
    m_up = 2.5f;
  } else {
    m_up = 2.0f;
  }
#endif

  st_KS.voice[v].pickOfs = N / KS_PICK_OFFSET_DIV;
  if (st_KS.voice[v].pickOfs < 1){
    st_KS.voice[v].pickOfs = 1;
  }
  if (st_KS.voice[v].pickOfs > KS_PICKBUF_SIZE){
    st_KS.voice[v].pickOfs = KS_PICKBUF_SIZE;
  }
  st_KS.voice[v].pickWp = 0;
  for (int i = 0; i < st_KS.voice[v].pickOfs; i++){
    st_KS.voice[v].pickBuf[i] = 0.0f;
  }

  st_KS.voice[v].muteEnv = 1.0f;
  st_KS.voice[v].muteLP = 0.0f;

  for (int i = 0; i < N; i++){
    n0 = rngF32Singed();
    n1 = rngF32Singed();
#if KS_USE_LP
    x = vel * 0.5f * (n0 + n1);
    lp += g * (x - lp);
    st_KS.delayLine[v][i] = lp * m_up;
#else
    baseExc = vel * 0.5f * (n0 + n1);
    wp = st_KS.voice[v].pickWp;
    delayed = st_KS.voice[v].pickBuf[wp];
    st_KS.voice[v].pickBuf[wp] = baseExc;
    wp++;
    if (wp >= st_KS.voice[v].pickOfs){
      wp = 0;
    }
    st_KS.voice[v].pickWp = wp;
    st_KS.delayLine[v][i] =  baseExc - (KS_PICK_MIX * delayed);
#endif
  }
}

static void noteOff(pqSynth_t* synth, uint32_t v)
{
  st_KS.voice[v].releasing = 1;
  if (st_KS.voice[v].decayCoef > 0.980f){
    st_KS.voice[v].decayCoef = 0.980f;
  }
  st_KS.voice[v].quietCount = 0;
}

static void preProc(pqSynth_t* synth, uint32_t frames)
{
  float freq;
  
  for (uint32_t v = 0; v < SYNTH_MAX_VOICE; v++){
    pqSynth_Voice_t* voice = &synth->voices[v];
    if (!voice->activeFlg){
      continue;
    }
    
    if (voice->noteEvPendFlg & st_KS.bitMask){
      voice->noteEvPendFlg &= ~st_KS.bitMask;
      if (voice->velocity){
        noteOn(synth, v, voice->noteNum, voice->velocity);
      } else {
        noteOff(synth, v);
      }
    }
  }
}

static void play(pqSynth_t* synth, uint32_t frames)
{
  float* out;
  float decay, a, b, filt, y, y_ap, lp2, mix, step;
  pqSynth_KS_Voice_t* vState;
  uint32_t N, index, index1, index2, r2;

  for (int v = 0; v < SYNTH_MAX_VOICE; v++){
    out = synth->voiceBuf[v];
    if (!synth->voices[v].activeFlg){
      for (int i = 0; i < frames; i++){
        out[i] = 0.0f;
      }
      continue;
    }
    vState = &st_KS.voice[v];
    N = vState->delayLen;

    if (N < 2){
      for (int i = 0; i < frames; i++){
        out[i] = 0.0f;
      }
      continue;      
    }
    index = vState->index;
    decay = vState->decayCoef;
    for (int i = 0; i < frames; i++){
      index1 = index;
      index2 = index + 1;
      r2 = (index2 < N) ? index2 : 0;

      a = st_KS.delayLine[v][index1];
      b = st_KS.delayLine[v][r2];
      filt = 0.5f * (a + b);
      if (vState->releasing){
        filt = 0.25f * (a + b) + 0.5f * vState->lastSample;
      }
      if (st_KS.voice[v].muteEnv > 0.0f){
        lp2 = st_KS.voice[v].muteLP;
        lp2 += KS_MUTE_LP_ALPHA * (filt - lp2);
        st_KS.voice[v].muteLP = lp2;

        mix = KS_MUTE_MIX_MAX * st_KS.voice[v].muteEnv;
        filt = (filt * (1.0f - mix)) + (lp2 * mix);

        step = st_MuteEnvStep;
        st_KS.voice[v].muteEnv -= step;
        if (st_KS.voice[v].muteEnv < 0.0f){
          st_KS.voice[v].muteEnv = 0.0f;
        }
      }

      y = filt * decay;
      vState->lastSample = filt;
      
      y_ap = y;
      if (vState->allpass.enable){
        y_ap = allpassProc(&vState->allpass.z, y, vState->allpass.a);
      }
      st_KS.delayLine[v][index1] = y_ap;
      out[i] = y_ap;

      if (vState->releasing){
        if (fabsf(y_ap) < 1.0e-4f){
          if (vState->quietCount < 0xFFFF){
            vState->quietCount++;
          }
        } else {
          vState->quietCount = 0;
        }
        if (vState->quietCount >= 512){
          synth->voices[v].activeFlg = 0;
          resetVoice(v);
          for (int k = i + 1; k < frames; k++){
            out[k] = 0.0f;
          }
          break;
        }
      }
      index++;
      if (index >= N){
        index = 0;
      }

    }
    vState->index = index;

  }
}

void InitVCOKerplusStrong(pqSynth_t* synth)
{
  uint32_t bitPtn;
  pqSynth_Module_t module;
  
  initNoteFreqTbl();

  for (int v = 0; v <SYNTH_MAX_VOICE; v++){
    resetVoice(v);
  }
  st_MuteEnvStep = 1.0f / (KS_MUTE_TIME_SEC * synth->sampleRate);
  
  module.name = "VCO_KS";
  module.play = play;
  module.preProc = preProc;
  bitPtn = PQSynth_RegisterModule(&module);
  if (bitPtn != 0xFFFFFFFF){
    st_KS.bitMask = bitPtn;
  }
}
