/**
* @brief   FRDM-MCXN947 board Djent Amp for Musical Synthsizer App 
* @author  masa
* @version 1.00
*/

#include "dj_amp.h"
#include "dj_gate.h"

static const float st_wsPosScale = ((float)DJAMP_SHAPERLUT_SIZE) / DJAMP_SHAPERLUT_XMAX;
pqSynth_DjAmp_t st_DjAmp;
float st_waveshaperLut_SS[DJAMP_SHAPERLUT_SIZE + 1];

static float bodyShelf(uint32_t v, float x)
{
  float lp;
  lp = st_DjAmp.voice[v].bodyLp + st_DjAmp.bodyLp_a * (x - st_DjAmp.voice[v].bodyLp);
  st_DjAmp.voice[v].bodyLp = lp;
  return x + st_DjAmp.bodyShelfGainLin_Minus1 * lp;
}

static void initSSWaveshaper(void)
{
  float g, xmax, ymax, x, t, y;
  g = 2.5f;
  xmax = DJAMP_SHAPERLUT_XMAX;
  ymax = (g * xmax) / (1.0f + (g * xmax));
  
  for (int i = 0; i <= (int)DJAMP_SHAPERLUT_SIZE; i++){
    x = (xmax * (float)i) / (float)DJAMP_SHAPERLUT_SIZE;
    t = g * x;
    y = t / (1.0f + t);
    st_waveshaperLut_SS[i] = y / ymax;
  }
}

static float clampf(float x, float lo, float hi)
{
  if (x < lo){
    return lo;
  }
  if (x > hi){
    return hi;
  }
  return x;
}

static float dcBlock(uint32_t v, float x)
{
  float y = (x - st_DjAmp.voice[v].dcBlockX1) + (st_DjAmp.dcBlockCoef_R * st_DjAmp.voice[v].dcBlockY1);
  st_DjAmp.voice[v].dcBlockX1 = x;
  st_DjAmp.voice[v].dcBlockY1 = y;
  return y;
}

static float postDcBlock(uint32_t v, float x)
{
  float y = (x - st_DjAmp.voice[v].postDcX1) + (st_DjAmp.postDcCoef_R * st_DjAmp.voice[v].postDcY1);
  st_DjAmp.voice[v].postDcX1 = x;
  st_DjAmp.voice[v].postDcY1 = y;
  return y;
}

static float preHpf(uint32_t v, float x)
{
  float y;
  y = st_DjAmp.preHpf_a * (st_DjAmp.voice[v].preHpfY1 + x - st_DjAmp.voice[v].preHpfX1);
  st_DjAmp.voice[v].preHpfX1 = x;
  st_DjAmp.voice[v].preHpfY1 = y;
  return y;
}

static float presence(uint32_t v, float x)
{
  float lp1, lp2, band;
  lp1 = st_DjAmp.voice[v].presLp1 + st_DjAmp.presLp1_a * (x - st_DjAmp.voice[v].presLp1);
  lp2 = st_DjAmp.voice[v].presLp2 + st_DjAmp.presLp2_a * (x - st_DjAmp.voice[v].presLp2);
  st_DjAmp.voice[v].presLp1 = lp1;
  st_DjAmp.voice[v].presLp2 = lp2;
  band = lp2 - lp1;
  return (x + DJAMP_PRESENCE_GAIN * band);
}

static inline float biteEqDFT2(uint32_t v, float x)
{
  float y, z1, z2;
  y = (st_DjAmp.biteCoef.b0 * x) + st_DjAmp.voice[v].biteZ1;
  z1 = (st_DjAmp.biteCoef.b1 * x) - (st_DjAmp.biteCoef.a1 * y) + st_DjAmp.voice[v].biteZ2;
  z2 = (st_DjAmp.biteCoef.b2 * x) - (st_DjAmp.biteCoef.a2 * y);
  st_DjAmp.voice[v].biteZ1 = z1;
  st_DjAmp.voice[v].biteZ2 = z2;
  return y;
}

static inline float midEqDFT2(uint32_t v, float x)
{
  float y, z1, z2;
  y = (st_DjAmp.midCoef.b0 * x) + st_DjAmp.voice[v].midZ1;
  z1 = (st_DjAmp.midCoef.b1 * x) - (st_DjAmp.midCoef.a1 * y) + st_DjAmp.voice[v].midZ2;
  z2 = (st_DjAmp.midCoef.b2 * x) - (st_DjAmp.midCoef.a2 * y);
  st_DjAmp.voice[v].midZ1 = z1;
  st_DjAmp.voice[v].midZ2 = z2;
  return y;
}

static float postLpf(uint32_t v, float x)
{
  float y;
  y = st_DjAmp.voice[v].postLp + st_DjAmp.postLp_a * (x - st_DjAmp.voice[v].postLp);
  st_DjAmp.voice[v].postLp = y;
  return y;
}

static float postLpf2(uint32_t v, float x)
{
  float y;
  y = st_DjAmp.voice[v].postLp2 + st_DjAmp.postLp_a * (x - st_DjAmp.voice[v].postLp2);
  st_DjAmp.voice[v].postLp2 = y;
  return y;
}



static float tanhApprox(float x)
{
  float xpow2 = x * x;
  return (x * (27.0f + xpow2) / (27.0f + 9.0f * xpow2));
}

static float hardClip(float x, float lim)
{
  if (x > lim){
    return lim;
  }
  if (x < -lim){
    return -lim;
  }
  return x;
}

static float waveShaperSoftsign(float x)
{
  float x_abs, pos, frac, y0, y1, y;
  int index;
  
  x = clampf(x, -DJAMP_SHAPERLUT_XMAX, +DJAMP_SHAPERLUT_XMAX);
  x_abs = fabsf(x);
  pos = x_abs * st_wsPosScale;
  index = (int)pos;
  if (index >= (int)DJAMP_SHAPERLUT_SIZE){
    index = (int)DJAMP_SHAPERLUT_SIZE - 1;
    frac = 1.0f;
  } else {
    frac = pos - (float)index;
  }
  
  y0 = st_waveshaperLut_SS[index];
  y1 = st_waveshaperLut_SS[index + 1];
  y = y0 + (y1 - y0) * frac;
  
  if (x < 0.0f){
    y = -y;
  }
  
  return y;
}

static float envFollow(uint32_t v, float x_abs)
{
  float e, p;
  uint32_t hold;
  if (st_DjAmp.envMode == ENV_MODE_ATK_HOLD_DECAY){
    p = st_DjAmp.voice[v].envPeak;
    hold = st_DjAmp.voice[v].envHoldCount;
    
    if (x_abs > p){
      p = x_abs;
      hold = st_DjAmp.envHoldSamples;
    } else {
      if (hold != 0){
        hold--;
      } else {
        p = p * st_DjAmp.envDecayCoef;
        if (p < 0.0f){
          p = 0.0f;
        }
      }
    }
    st_DjAmp.voice[v].envPeak = p;
    st_DjAmp.voice[v].envHoldCount = hold;
    st_DjAmp.voice[v].env = p;
    return p;
  }
  
  e = st_DjAmp.voice[v].env;
  e = e + st_DjAmp.envAlpha * (x_abs - e);
  st_DjAmp.voice[v].env = e;
  return e;
}

static float sagFollow(uint32_t v, float env)
{
  float ret;
  switch(st_DjAmp.sagMode){
    case(SAG_MODE_OFF):{
      st_DjAmp.voice[v].sag = 0.0f;
      ret = 0.0f;
      break;
    }
    case(SAG_MODE_FAST_TRACK):{
      float s = st_DjAmp.voice[v].sag;
      s = s + (st_DjAmp.sagAlpha * 20.0f) * (env - s);
      st_DjAmp.voice[v].sag = s;
      ret = s;
      break;
    }
    case(SAG_MODE_LEGACY):{
      float s = st_DjAmp.voice[v].sag;
      s = s + st_DjAmp.sagAlpha * (env - s);
      st_DjAmp.voice[v].sag = s;
      ret = s;
      break;
    }
    default:
    ret = 0.0f;
    break;
  }
  return ret;
}

static void preProc(pqSynth_t* synth, uint32_t frames)
{
  for (int v = 0; v < SYNTH_MAX_VOICE; v++){
    if (synth->voices[v].noteEvPendFlg & st_DjAmp.bitMask){
      synth->voices[v].noteEvPendFlg &= ~st_DjAmp.bitMask;
      if (synth->voices[v].velocity != 0){
        st_DjAmp.voice[v].dcBlockX1 = 0.0f;
        st_DjAmp.voice[v].dcBlockY1 = 0.0f;
        st_DjAmp.voice[v].env = 0.0f;
        st_DjAmp.voice[v].sag = 0.0f;
        st_DjAmp.voice[v].envPeak = 0.0f;
        st_DjAmp.voice[v].envHoldCount = 0U;
        st_DjAmp.voice[v].driveSm = 0.0f;
        st_DjAmp.voice[v].biasSm = 0.0f;
        
        st_DjAmp.voice[v].postDcX1 = 0.0f;
        st_DjAmp.voice[v].postDcY1 = 0.0f;
        
        st_DjAmp.voice[v].preHpfX1 = 0.0f;
        st_DjAmp.voice[v].preHpfY1 = 0.0f;
        st_DjAmp.voice[v].presLp1 = 0.0f;
        st_DjAmp.voice[v].presLp2 = 0.0f;
        st_DjAmp.voice[v].postLp = 0.0f;
        st_DjAmp.voice[v].postLp2 = 0.0f;
      }
    }
  }
}

static void play(pqSynth_t* synth, uint32_t frames)
{
  float* buf;
  float* gateGainBuf;
  float x, drive, bias, env, sag, u, u1, y, y1, y2, ds, bs;
  for (int v = 0; v < SYNTH_MAX_VOICE; v++){
    if (!synth->voices[v].activeFlg){
      continue;
    }
    buf = &synth->voiceBuf[v][0];
    gateGainBuf =  &st_DjGate.voice[v].gateGainBuf[0];

    for (int i = 0; i < frames; i++){
      x = buf[i];
      x = x * st_DjAmp.inGain;
      x = dcBlock(v, x);
      x = preHpf(v, x);
      //x = presence(v, x);
      
      #if DJAMP_ENABLE_DYNAMICS      
      env = envFollow(v, fabsf(x));
      sag = sagFollow(v, env);
      
      drive = st_DjAmp.driveBase + st_DjAmp.drvEnvAmt * env;
      drive = clampf(drive, st_DjAmp.driveMin, st_DjAmp.driveMax);
      ds = st_DjAmp.voice[v].driveSm;
      ds = ds + st_DjAmp.driveSmthAlpha * (drive - ds);
      st_DjAmp.voice[v].driveSm = ds;
      drive = ds;
      
      bias = st_DjAmp.biasAmt * sag;
      bias = clampf(bias, st_DjAmp.biasMin, st_DjAmp.biasMax);
      bs = st_DjAmp.voice[v].biasSm;
      bs = bs + st_DjAmp.biasSmthAlpha * (bias - bs);
      st_DjAmp.voice[v].biasSm = bs;
      bias = bs;
      
      #else
      drive = st_DjAmp.driveBase;
      bias = 0.0f;
      #endif
      u = drive * x + bias;
      y1 = waveShaperSoftsign(u);
      u1 = y1 * DJAMP_WS2_DRIVE;
      if (u1 < 0.0f){
        u1 = u1 * DJAMP_ASYM_NEG_DRIVE;
      }
      y2 = tanhApprox(u1);
      y = hardClip(y2, DJAMP_HARD_CLIP);
      #if DJAMP_ENABLE_DYNAMICS
      y = y * (1.0f - st_DjAmp.sagAmt * st_DjAmp.voice[v].sag);
      #endif
      x = y * st_DjAmp.outGain;
      x = biteEqDFT2(v, x);
      x = midEqDFT2(v, x);
      x = postLpf(v, x);
      x = postLpf2(v, x);
      x = postDcBlock(v, x);
      x = bodyShelf(v, x);
      
      x = x * st_DjAmp.postGain;
      x = x * gateGainBuf[i];
      
      buf[i] = x;
    }
  }
}

void InitDjentAmp(pqSynth_t* synth)
{
  uint32_t bitPtn;
  pqSynth_Module_t module;
  float fs, k, A, w0, alpha, cosw;
  float b0, b1, b2, a0, a1, a2;
  
  st_DjAmp.inGain = 1.0f;
  st_DjAmp.outGain = 1.0f;
  
  st_DjAmp.postGain = 0.45f;
  
  st_DjAmp.driveBase = 13.5f;
  st_DjAmp.drvEnvAmt = -1.5f;
  
  st_DjAmp.sagMode = SAG_MODE_OFF;
  st_DjAmp.sagAmt = 0.08f;
  st_DjAmp.sagAlpha = 0.02f;
  
  st_DjAmp.biasAmt = 0.12f;
  
  st_DjAmp.envMode = ENV_MODE_ATK_HOLD_DECAY;
  st_DjAmp.envHoldMs = 12.0f;
  st_DjAmp.envDecayCoef = 0.990f;
  st_DjAmp.envAlpha = 0.30f;
  
  st_DjAmp.enPreDc = 1;
  st_DjAmp.dcBlockCoef_R = 0.9995f;
  st_DjAmp.postDcCoef_R = st_DjAmp.dcBlockCoef_R;
  
  st_DjAmp.postDcPlace = POSTDC_PLACE_END;
  
  st_DjAmp.driveMin = 5.0f;
  st_DjAmp.driveMax = 15.0f;
  st_DjAmp.biasMin = 0.0f;
  st_DjAmp.biasMax = 0.22f;
  
  st_DjAmp.driveSmthAlpha = 0.10f;
  st_DjAmp.biasSmthAlpha = 0.08f;
  
  fs = synth->sampleRate;
  
  st_DjAmp.envHoldSamples = (uint32_t)((st_DjAmp.envHoldMs * fs) / 1000.0f);
  if (st_DjAmp.envHoldSamples < 1U){
    st_DjAmp.envHoldSamples = 1U;
  }
  
  k = 2.0f * (float)M_PI / fs;
  st_DjAmp.preHpf_a = expf(-k * DJAMP_PRE_HPF_HZ);
  st_DjAmp.presLp1_a = 1.0f - expf(-k * DJAMP_PRESENCE_LP1_HZ);
  st_DjAmp.presLp2_a = 1.0f - expf(-k * DJAMP_PRESENCE_LP2_HZ);
  st_DjAmp.postLp_a = 1.0f - expf(-k * DJAMP_POST_LPF_HZ);
  
  st_DjAmp.bodyLp_a = 1.0f - expf(-k * DJAMP_BODY_LP_HZ);
  st_DjAmp.bodyShelfGainLin_Minus1 = powf(10.0f, (DJAMP_BODY_SHELF_GAIN_DB / 20.0f)) - 1.0f;
  for (int v = 0; v < SYNTH_MAX_VOICE; v++){
    st_DjAmp.voice[v].bodyLp = 0.0f;
    st_DjAmp.voice[v].biteZ1 = 0.0f;
    st_DjAmp.voice[v].biteZ2 = 0.0f;
    st_DjAmp.voice[v].midZ1 = 0.0f;
    st_DjAmp.voice[v].midZ2 = 0.0f;
  }
  
  A = powf(10.0f, (DJAMP_BITE_PEAK_GAIN_DB / 40.0f));
  w0 = 2.0f * (float)M_PI * (DJAMP_BITE_PEAK_HZ / fs);
  cosw = cosf(w0);
  alpha = sinf(w0) / (2.0f * DJAMP_BITE_PEAK_Q);
  
  b0 = 1.0f + (alpha * A);
  b1 = -2.0f * cosw;
  b2 = 1.0f - (alpha * A);
  a0 = 1.0f + (alpha / A);
  a1 = -2.0f * cosw;
  a2 = 1.0f - (alpha / A);
  
  st_DjAmp.biteCoef.b0 = b0 / a0;
  st_DjAmp.biteCoef.b1 = b1 / a0;
  st_DjAmp.biteCoef.b2 = b2 / a0;
  st_DjAmp.biteCoef.a1 = a1 / a0;
  st_DjAmp.biteCoef.a2 = a2 / a0;
  
  A = powf(10.0f, DJAMP_MID_PEAK_GAIN_DB / 40.0f);
  w0 = 2.0f * (float)M_PI * (DJAMP_MID_PEAK_HZ / fs);
  alpha = sinf(w0) / (2.0f * DJAMP_MID_PEAK_Q);
  cosw = cosf(w0);
  
  b0 = 1.0f + (alpha * A);
  b1 = -2.0f * cosw;
  b2 = 1.0f - (alpha * A);
  a0 = 1.0f + (alpha / A);
  a1 = -2.0f * cosw;
  a2 = 1.0f - (alpha / A);
  
  st_DjAmp.midCoef.b0 = b0 / a0;
  st_DjAmp.midCoef.b1 = b1 / a0;
  st_DjAmp.midCoef.b2 = b2 / a0;
  st_DjAmp.midCoef.a1 = a1 / a0;
  st_DjAmp.midCoef.a2 = a2 / a0;
  
  initSSWaveshaper();
  
  module.name = "Dj_AMP";
  module.play = play;
  module.preProc = preProc;
  bitPtn = PQSynth_RegisterModule(&module);
  if (bitPtn != 0xFFFFFFFF){
    st_DjAmp.bitMask = bitPtn;
  }  
}
