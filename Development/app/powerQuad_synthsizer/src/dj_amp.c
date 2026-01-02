/**
* @brief   FRDM-MCXN947 board Djent Amp for Musical Synthsizer App 
* @author  masa
* @version 1.00
*/

#include "dj_amp.h"
static const float st_wsPosScale = ((float)DJAMP_SHAPERLUT_SIZE) / DJAMP_SHAPERLUT_XMAX;
pqSynth_DjAmp_t st_DjAmp;

float st_waveshaperLut_SS[DJAMP_SHAPERLUT_SIZE + 1];

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

static float postLpf(uint32_t v, float x)
{
  float y;
  y = st_DjAmp.voice[v].postLp + st_DjAmp.postLp_a * (x - st_DjAmp.voice[v].postLp);
  st_DjAmp.voice[v].postLp = y;
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
  float e = st_DjAmp.voice[v].env;
  e = e + st_DjAmp.envAlpha * (x_abs - e);
  st_DjAmp.voice[v].env = e;
  return e;
}

static float sagFollow(uint32_t v, float env)
{
  float s = st_DjAmp.voice[v].sag;
  s = s + st_DjAmp.sagAlpha * (env - s);
  st_DjAmp.voice[v].sag = s;
  return s;
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

        st_DjAmp.voice[v].postDcX1 = 0.0f;
        st_DjAmp.voice[v].postDcY1 = 0.0f;

        st_DjAmp.voice[v].preHpfX1 = 0.0f;
        st_DjAmp.voice[v].preHpfY1 = 0.0f;
        st_DjAmp.voice[v].presLp1 = 0.0f;
        st_DjAmp.voice[v].presLp2 = 0.0f;
        st_DjAmp.voice[v].postLp = 0.0f;
      }
    }
  }
}

static void play(pqSynth_t* synth, uint32_t frames)
{
  float* buf;
  float x, drive, bias, env, sag, u, u1, y, y1, y2;
  for (int v = 0; v < SYNTH_MAX_VOICE; v++){
    if (!synth->voices[v].activeFlg){
      continue;
    }
    buf = &synth->voiceBuf[v][0];
    for (int i = 0; i < frames; i++){
      x = buf[i];
      x = dcBlock(v, x);
      x = preHpf(v, x);
      x = presence(v, x);
      #if DJAMP_ENABLE_DYNAMICS      
      env = envFollow(v, fabsf(x));
      sag = sagFollow(v, env);
      
      drive = st_DjAmp.driveBase + st_DjAmp.drvEnvAmt * env;
      bias = st_DjAmp.biasAmt * sag;
      #else
      drive = st_DjAmp.driveBase;
      bias = 0.0f;
      #endif
      u = drive * x + bias;
      y1 = waveShaperSoftsign(u);
      u1 = y1 * DJAMP_WS2_DRIVE;
      if (u < 0.0f){
        u1 = u1 * DJAMP_ASYM_NEG_DRIVE;
      }
      y2 = tanhApprox(u1);
      y = hardClip(y2, DJAMP_HARD_CLIP);
      #if DJAMP_ENABLE_DYNAMICS
      y = y * (1.0f - st_DjAmp.sagAmt * st_DjAmp.voice[v].sag);
      #endif
      x = y * st_DjAmp.postGain;
      x = postLpf(v, x);
      x = postDcBlock(v, x);
      buf[i] = x;
    }
  }
}

void InitDjentAmp(pqSynth_t* synth)
{
  uint32_t bitPtn;
  pqSynth_Module_t module;
  float fs, k;
  
  st_DjAmp.driveBase = 64.0f;
  st_DjAmp.postGain = 0.3f;
  st_DjAmp.dcBlockCoef_R = 0.9995f;
  st_DjAmp.postDcCoef_R = st_DjAmp.dcBlockCoef_R;
  st_DjAmp.drvEnvAmt = 2.5f;
  st_DjAmp.sagAmt = 0.25f;
  st_DjAmp.biasAmt = 0.15f;
  st_DjAmp.envAlpha = 0.008f;
  st_DjAmp.sagAlpha = 0.002f;

  fs = synth->sampleRate;
  k = 2.0f * (float)M_PI / fs;
  st_DjAmp.preHpf_a = expf(-k * DJAMP_PRE_HPF_HZ);
  st_DjAmp.presLp1_a = 1.0f - expf(-k * DJAMP_PRESENCE_LP1_HZ);
  st_DjAmp.presLp2_a = 1.0f - expf(-k * DJAMP_PRESENCE_LP2_HZ);
  st_DjAmp.postLp_a = 1.0f - expf(-k * DJAMP_POST_LPF_HZ);


  initSSWaveshaper();
  
  module.name = "Dj_AMP";
  module.play = play;
  module.preProc = preProc;
  bitPtn = PQSynth_RegisterModule(&module);
  if (bitPtn != 0xFFFFFFFF){
    st_DjAmp.bitMask = bitPtn;
  }  
}
