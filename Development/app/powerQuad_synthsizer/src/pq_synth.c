/**
* @brief   FRDM-MCXN947 board PowerQuad Musical Synthsizer App
* @author  masa
* @version 1.00
*/

#include "pq_synth.h"
#include "hcd_class_audio.h"
#include "midi.h"

#include "vco.h"
#include "vco_wavetbl.h"
#include "vco_ks.h"
#include "vca.h"
#include "dj_amp.h"
#include "dj_cab.h"

pqSynth_t st_Synth[SYNTH_MAX_NUM];

static inline uint32_t disint(void) {
  uint32_t primask;
  __asm volatile ("MRS %0, primask" : "=r"(primask) :: "memory");
  __asm volatile ("cpsid i" ::: "memory");
  return primask;
}
static inline void enaint(uint32_t primask) {
  __asm volatile ("MSR primask, %0" :: "r"(primask) : "memory");
}

static inline pqSynth_t* getSynth(uint8_t devIdx)
{
  pqSynth_t* ret = NULL;
  for (int i = 0; i < SYNTH_MAX_NUM; i++){
    if (st_Synth[i].deviceIndex == devIdx){
      ret = &st_Synth[i];
      break;
    }
  }
  return ret;
}

static inline int16_t float2S16(float x)
{
  float y = x * 32767.0f;
  int32_t v = (int32_t)lrintf(y);
  v = __SSAT(v, 16);
  return (int16_t)v;
}

static void play(uint8_t idx, uint32_t* buf, uint16_t nextTxSize)
{
  uint32_t nrFrames;
  pqSynth_t* synth;
  int16_t* s16Out;
  int16_t s16Val;
  
  synth = getSynth(idx);
  if (!synth){
    return;
  }
  
  nrFrames = nextTxSize >> 2;
  if (nrFrames > SYNTH_SAMPLE_BUF_SIZE){
    return;
  }
  
  for (int i = 0; i < synth->nrModules; i++){
    synth->modules[i].preProc(synth, nrFrames);
    synth->modules[i].play(synth, nrFrames);
  }
  
  
  s16Out = (int16_t*)buf;
  for (int i = 0; i < nrFrames; i++) {
    float mix = 0.0f;
    for (int v = 0; v < SYNTH_MAX_VOICE; v++) {
      if (synth->voices[v].activeFlg){
        mix += synth->voiceBuf[v][i];
      }
    }
    if (mix > 1.0f){
      mix = 1.0f;
    }
    if (mix < -1.0f) {
      mix = -1.0f;
    }
    s16Val = float2S16(mix);
    s16Out[i * 2u + 0u] = s16Val;
    s16Out[i * 2u + 1u] = s16Val;
  }
}

static int allocateVoice(pqSynth_t* synth)
{
  for (int v = 0; v < SYNTH_MAX_VOICE; v++){
    if (!synth->voices[v].activeFlg){
      return v;
    }
  }
  return 0;
}

static int findVoice(pqSynth_t* synth, uint8_t note)
{
  for (int v = 0; v < SYNTH_MAX_VOICE;  v++){
    if (synth->voices[v].activeFlg && synth->voices[v].noteNum == note){
      return v;
    }
  }
  return -1;
}

static void noteOn(uint8_t noteNum, uint8_t velocity)
{
  pqSynth_t* synth = &st_Synth[0];
  uint32_t premask = disint();
  int v = allocateVoice(synth);
  synth->voices[v].activeFlg = 1;
  synth->voices[v].noteNum = noteNum;
  synth->voices[v].velocity = velocity;
  synth->voices[v].noteEvPendFlg = synth->moduleBitMask;
  enaint(premask);
}

static void noteOff(uint8_t noteNum, uint8_t velocity)
{
  pqSynth_t* synth = &st_Synth[0];
  uint32_t premask = disint();
  int v = findVoice(synth, noteNum);
  if (v >= 0){
    synth->voices[v].velocity = 0;
    synth->voices[v].noteEvPendFlg = synth->moduleBitMask;
  }
  enaint(premask);
}

static void deviceNotify(uint8_t deviceIndex, uint8_t dir)
{
  pqSynth_t* synth = &st_Synth[0];
  if ((dir == AUDIO_DIR_PLAY) && (synth->deviceIndex == 0)){
    synth->deviceIndex = deviceIndex;
    UsbhAudio_SetPlayCallback(deviceIndex, play);
    UsbhAudio_StartStreaming(deviceIndex, dir);
  }
}

static void init(float samFreq, float ampCoef)
{
  pqSynth_t* synth = &st_Synth[0];
  
  synth->sampleRate = samFreq;
  synth->ampMax = ampCoef;
  
  /*USB Host Callback*/
  UsbhAudio_SetReadyNotify(deviceNotify);
  
  /*MIDI Callback*/
  MIDI_SetCallback(MIDI_CIN_NOTE_OFF, noteOff);
  MIDI_SetCallback(MIDI_CIN_NOTE_ON, noteOn);
  
  /*Module Initialization*/
  //InitVCO();
  //InitVCOWaveTable(synth);
  InitVCOKerplusStrong(synth);
  InitDjentAmp(synth);
  InitDjentCab(synth);
  InitVCA(synth);
  
}

void PQSynth_Init(void)
{
  PQ_Init(POWERQUAD);
  init(44100.0f, 0.5f / (float)SYNTH_MAX_VOICE);
}

uint32_t PQSynth_RegisterModule(pqSynth_Module_t* newModule)
{
  uint32_t retBitMap;
  pqSynth_Module_t* module;
  pqSynth_t* synth = &st_Synth[0];
  if (synth->nrModules == SYNTH_MAX_MODULES){
    return 0xFFFFFFFF;
  }
  module = &synth->modules[synth->nrModules];
  module->name = newModule->name;
  module->play = newModule->play;
  module->preProc = newModule->preProc;
  module->moduleID = synth->nrModules;
  retBitMap = (1 << synth->nrModules);
  synth->nrModules++;
  synth->moduleBitMask |= retBitMap;
  
  return retBitMap;
}
