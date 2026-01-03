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
float st_BurstRemMax;
static uint32_t st_AtkWinMinSam;
static uint32_t st_AtkWinMaxSam;


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
  
  st_KS.voice[v].ageSamples = 0;
  st_KS.voice[v].atkWinSamples = 0;
  st_KS.voice[v].invAtkWin = 0.0f;
  st_KS.voice[v].burstRem = 0;
  st_KS.voice[v].burstEnv = 0.0f;
  st_KS.voice[v].burstDecay = 0.0f;
  st_KS.voice[v].thumpLP = 0.0f;
  st_KS.voice[v].hfLP = 0.0f;
  
  st_KS.voice[v].dispAp.en = 0;
  st_KS.voice[v].dispAp.z = 0.0f;
  st_KS.voice[v].dispAp.a = 0.0f;
  
  st_KS.voice[v].muteEnv = 0.0f;
  st_KS.voice[v].muteLP = 0.0f;
  st_KS.voice[v].pickWp = 0;
  st_KS.voice[v].pickOfs = 0;
  
  memset(&st_KS.delayLine[v][0], 0, sizeof(st_KS.delayLine[v]));
}


static void noteOn(pqSynth_t* synth, uint32_t v, uint8_t noteNum, uint8_t velocity)
{ 
  float freq, D, frac, decay, vel, n0, n1;
  float lp, baseExc, delayed;
  float thump, click, atk, pickMix, mean;
  float sum, div;
  uint32_t N, wp, atkWin;
  pqSynth_KS_Voice_t* kVoice = &st_KS.voice[v];
  
  freq = note2Freq_Hz(noteNum);
  
  D = synth->sampleRate / freq;
  N = (uint32_t)D;
  if (N < 2){
    N = 2;
  }
  
  if (N > KS_MAX_DELAY_SAMPLES){
    N = KS_MAX_DELAY_SAMPLES;
  }
  
  frac = D - (float)N;

  #if KS_ATK_PITCH_EN
  atkWin = (uint32_t)(KS_ATK_WIN_CYCLES * (float)N);
  if (atkWin < st_AtkWinMinSam){
    atkWin = st_AtkWinMinSam;
  }
  if (atkWin > st_AtkWinMaxSam){
    atkWin = st_AtkWinMaxSam;
  }
  kVoice->atkWinSamples =  atkWin;
  #else
  kVoice->atkWinSamples = (uint32_t)((KS_ATK_WIN_MS * 0.001f) * synth->sampleRate);
  #endif
  
  if (N < 64){
    decay = 0.990f;
  } else if (N < 128){
    decay = 0.992f;
  } else if (N < 256){
    decay = 0.994f;
  } else if (N < 512){
    decay = 0.996f;
  } else {
    decay = 0.997f;
  }
  kVoice->delayLen = N;
  kVoice->index = 0;
  kVoice->decayCoef = decay;
  kVoice->lastSample = 0.0f;
  kVoice->quietCount = 0;
  kVoice->releasing = 0;
  
  kVoice->allpass.z = 0.0f;
  kVoice->allpass.a = frac2AllpassCoef_a(frac);
  kVoice->allpass.enable = (frac > 1.0e-4f) ? 1 : 0;
  
  kVoice->ageSamples = 0;
  kVoice->burstRem = st_BurstRemMax;
  kVoice->burstEnv = 0.0f;
  kVoice->burstDecay = 0.0f;
  kVoice->thumpLP = 0.0f;
  kVoice->hfLP = 0.0f;
  
  kVoice->dispAp.en = 1;
  kVoice->dispAp.z = 0.0f;
  kVoice->dispAp.a = KS_DISP_AP_A;
  
  kVoice->muteEnv = 0.0f;
  kVoice->muteLP = 0.0f;
  
  vel = (float)velocity * (1.0f / 127.0f);
  kVoice->burstEnv = KS_BURST_GAIN * vel;
  
  div = (float)KS_PICK_OFFSET_DIV + (KS_PICK_OFS_DIV_VAR * (1.0f - vel));
  if (div < 1.0f){
    div = 1.0f;
  }
  kVoice->pickOfs = (uint32_t)((float)N / div);
  if (kVoice->pickOfs < 1){
    kVoice->pickOfs = 1;
  }
  if (kVoice->pickOfs > KS_PICKBUF_SIZE){
    kVoice->pickOfs = KS_PICKBUF_SIZE;
  }
  kVoice->pickWp = 0;
  for (int i = 0; i < kVoice->pickOfs; i++){
    kVoice->pickBuf[i] = 0.0f;
  }
  
  lp = 0.0f;
  sum = 0.0f;
  
  for (int i = 0; i < N; i++){
    n0 = rngF32Singed();
    n1 = rngF32Singed();
    
    if (i < kVoice->atkWinSamples){
      atk = 1.0f - ((float)i * kVoice->invAtkWin);
      if (atk < 0.0f){
        atk = 0.0f;
      }
    } else {
      atk = 0.0f;
    }
    
    thump = 0.5f * (n0 + n1);
    lp += KS_THUMP_LP_ALPHA * (thump - lp);
    thump = lp;
    
    click = 0.5f * (n0 - n1);
    baseExc = vel * ((KS_THUMP_LP_GAIN * thump) + (KS_CLICK_HP_GAIN * click * atk)); 
    wp = kVoice->pickWp;
    delayed = kVoice->pickBuf[wp];
    kVoice->pickBuf[wp] = baseExc;
    wp++;
    if (wp >= kVoice->pickOfs){
      wp = 0;
    }
    kVoice->pickWp = wp;
    pickMix = KS_PICK_MIX + (KS_PICK_MIX_ATK_BOOST * atk);
    baseExc = baseExc - (pickMix * delayed);
    
    st_KS.delayLine[v][i] = baseExc;
    sum += baseExc;
  }
  
  mean = sum * (1.0f / (float)N);
  for (int i = 0; i < N; i++){
    st_KS.delayLine[v][i] -= mean;
  }
}

static void noteOff(pqSynth_t* synth, uint32_t v)
{
  pqSynth_KS_Voice_t* kVoice = &st_KS.voice[v];
  kVoice->releasing = 1;
  kVoice->muteEnv = 1.0f;
  kVoice->muteLP = 0.0f;
  kVoice->burstRem = 0;
  kVoice->burstEnv = 0.0f;
  
  if (kVoice->decayCoef > 0.975f){
    kVoice->decayCoef = 0.975f;
  }
  kVoice->quietCount = 0;
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
  float decay, decayEff, a, b, filt, y, y_ap, lp2, mix, step;
  float atk, thump, click, n0, n1;
  float low, hf, loss, injAtk;
  pqSynth_KS_Voice_t* kVoice;
  uint32_t N, index, index1, index2, r2;
  uint16_t quietTh;
  
  for (int v = 0; v < SYNTH_MAX_VOICE; v++){
    out = synth->voiceBuf[v];
    if (!synth->voices[v].activeFlg){
      for (int i = 0; i < frames; i++){
        out[i] = 0.0f;
      }
      continue;
    }
    kVoice = &st_KS.voice[v];
    N = kVoice->delayLen;
    
    if (N < 2){
      for (int i = 0; i < frames; i++){
        out[i] = 0.0f;
      }
      continue;      
    }
    index = kVoice->index;
    decay = kVoice->decayCoef;
    for (int i = 0; i < frames; i++){
      index1 = index;
      index2 = index + 1;
      r2 = (index2 < N) ? index2 : 0;
      
      a = st_KS.delayLine[v][index1];
      b = st_KS.delayLine[v][r2];
      filt = 0.5f * (a + b);
      
      atk = 0.0f;
      if (kVoice->ageSamples < kVoice->atkWinSamples){
        atk = 1.0 - ((float)kVoice->ageSamples * kVoice->invAtkWin);
        if (atk < 0.0f){
          atk = 0.0f;
        }
        filt += (0.5f * atk) * (a - filt);
      }
      
      low = kVoice->hfLP;
      low += KS_HFLOSS_LP_ALPHA * (filt - low);
      kVoice->hfLP = low;
      hf = filt - low;
      loss = KS_HFLOSS_BASE - (KS_HFLOSS_ATK_RED * atk);
      if (loss < 0.0f){
        loss = 0.0f;
      }
      if (loss > 0.95f){
        loss = 0.95f;
      }
      filt = filt - (loss * hf);
      
      if (kVoice->releasing){
        filt = 0.25f * (a + b) + 0.5f * kVoice->lastSample;
        if (kVoice->muteEnv > 0.0f){
          lp2 = kVoice->muteLP;
          lp2 += KS_MUTE_LP_ALPHA * (filt - lp2);
          kVoice->muteLP = lp2;
          
          mix = KS_MUTE_MIX_MAX * kVoice->muteEnv;
          filt = (filt * (1.0f - mix)) + (lp2 * mix);
          
          step = st_MuteEnvStep;
          kVoice->muteEnv -= step;
          if (kVoice->muteEnv < 0.0f){
            kVoice->muteEnv = 0.0f;
          }
        }
      }
      decayEff = decay;
      if (kVoice->releasing){
        decayEff = decay * (1.0f - (KS_REL_EX_DECAY * kVoice->muteEnv));
        if (decayEff < 0.0f){
          decayEff = 0.0f;
        }
      }
      
      y = filt * decayEff;
      kVoice->lastSample = filt;

      if (kVoice->burstRem > 0){
        injAtk = atk;
        n0 = rngF32Singed();
        n1 = rngF32Singed();
        thump = 0.5f * (n0 + n1);
        kVoice->thumpLP += KS_THUMP_LP_ALPHA * (thump - kVoice->thumpLP);
        click = 0.5f *  (n0 - n1);
        y += kVoice->burstEnv * ((0.25f * KS_THUMP_LP_GAIN * kVoice->thumpLP) + (KS_CLICK_HP_GAIN * click * injAtk));
        kVoice->burstEnv = kVoice->burstEnv * kVoice->burstDecay;
        kVoice->burstRem--;
      }

      
      y_ap = y;
      if (kVoice->allpass.enable){
        y_ap = allpassProc(&kVoice->allpass.z, y, kVoice->allpass.a);
      }
      if (kVoice->dispAp.en && (kVoice->ageSamples < kVoice->atkWinSamples)){
        y_ap = allpassProc(&kVoice->dispAp.z, y_ap, kVoice->dispAp.a);
      }

      st_KS.delayLine[v][index1] = y_ap;
      out[i] = y_ap;

      if (kVoice->ageSamples < 0xFFFFFFFF){
        kVoice->ageSamples++;
      }
      
      if (kVoice->releasing){
        if (fabsf(y_ap) < 1.0e-4f){
          if (kVoice->quietCount < 0xFFFF){
            kVoice->quietCount++;
          }
        } else {
          kVoice->quietCount = 0;
        }
        quietTh = 256;
        if (N >= 256){
          quietTh = 384;
        }
        if (N >= 512){
          quietTh = 512;
        }
        if (kVoice->quietCount >= quietTh){
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
    kVoice->index = index;
    
  }
}

void InitVCOKerplusStrong(pqSynth_t* synth)
{
  uint32_t bitPtn;
  pqSynth_Module_t module;
  
  initNoteFreqTbl();
  st_AtkWinMinSam = (uint32_t)((KS_ATK_WIN_MIN_MS * 0.001f) * synth->sampleRate);
  st_AtkWinMaxSam = (uint32_t)((KS_ATK_WIN_MAX_MS * 0.001f) * synth->sampleRate);
  if (st_AtkWinMinSam < 1){
    st_AtkWinMinSam = 1;
  }
  if (st_AtkWinMaxSam < st_AtkWinMinSam){
    st_AtkWinMaxSam = st_AtkWinMinSam;
  }
  
  for (int v = 0; v <SYNTH_MAX_VOICE; v++){
    resetVoice(v);
    st_KS.voice[v].burstDecay = expf(-1.0f / (KS_BURST_TAU_SEC * (float)synth->sampleRate));
  }
  st_BurstRemMax = (uint32_t)((KS_BURST_INJECT_MS * 0.001f) * synth->sampleRate);
  st_MuteEnvStep = 1.0f / ((KS_REL_MUTE_MS * 0.001f) * synth->sampleRate);
  
  module.name = "VCO_KS";
  module.play = play;
  module.preProc = preProc;
  bitPtn = PQSynth_RegisterModule(&module);
  if (bitPtn != 0xFFFFFFFF){
    st_KS.bitMask = bitPtn;
  }
}
