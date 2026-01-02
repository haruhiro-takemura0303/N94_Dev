/**
* @brief   FRDM-MCXN947 board PowerQuad Musical Synthsizer App
* @author  masa
* @version 1.00
*/

#include "pq_synth.h"
#include "hcd_class_audio.h"
#include "midi.h"

#include "vcom_audio_recorder.h"

#include "vco.h"
#include "vco_wavetbl.h"
#include "vco_ks.h"
#include "vca.h"
#include "dj_amp.h"
#include "dj_cab.h"
#include "dj_gate.h"

pqSynth_t st_Synth[SYNTH_MAX_NUM];

// Debug (watch variables): DWT cycle counter based profiling
volatile uint32_t g_pq_dbg_last_cycles = 0;
volatile uint32_t g_pq_dbg_max_cycles = 0;
volatile uint32_t g_pq_dbg_budget_cycles = 0;
volatile uint32_t g_pq_dbg_overrun_count = 0;
volatile uint32_t g_pq_dbg_last_nrFrames = 0;

// Per-module profiling (indexed by moduleID / registration order)
volatile uint32_t g_pq_dbg_mod_pre_last[SYNTH_MAX_MODULES] = {0};
volatile uint32_t g_pq_dbg_mod_pre_max[SYNTH_MAX_MODULES]  = {0};
volatile uint32_t g_pq_dbg_mod_play_last[SYNTH_MAX_MODULES] = {0};
volatile uint32_t g_pq_dbg_mod_play_max[SYNTH_MAX_MODULES]  = {0};
volatile uint32_t g_pq_dbg_mod_total_last[SYNTH_MAX_MODULES] = {0};
volatile uint32_t g_pq_dbg_mod_total_max[SYNTH_MAX_MODULES]  = {0};
volatile uint32_t g_pq_dbg_nrModules_last = 0;
volatile uint32_t g_pq_dbg_nrModules_max  = 0;



static inline uint32_t disint(void) {
  uint32_t primask;
  __asm volatile ("MRS %0, primask" : "=r"(primask) :: "memory");
  __asm volatile ("cpsid i" ::: "memory");
  return primask;
}
static inline void enaint(uint32_t primask) {
  __asm volatile ("MSR primask, %0" :: "r"(primask) : "memory");
}

// --- DWT CYCCNT profiling (no printf; watch variables in debugger) ---
#define PQ_DEMCR_ADDR   (0xE000EDFCu)
#define PQ_DWT_CTRL_ADDR (0xE0001000u)
#define PQ_DWT_CYCCNT_ADDR (0xE0001004u)
#define PQ_DEMCR_TRCENA (1u << 24)
#define PQ_DWT_CTRL_CYCCNTENA (1u << 0)

static inline void pq_dwt_init(void)
{
  volatile uint32_t* demcr = (volatile uint32_t*)PQ_DEMCR_ADDR;
  volatile uint32_t* dwt_ctrl = (volatile uint32_t*)PQ_DWT_CTRL_ADDR;
  volatile uint32_t* dwt_cyccnt = (volatile uint32_t*)PQ_DWT_CYCCNT_ADDR;
  *demcr |= PQ_DEMCR_TRCENA;
  *dwt_cyccnt = 0;
  *dwt_ctrl |= PQ_DWT_CTRL_CYCCNTENA;
}

static inline uint32_t pq_dwt_get(void)
{
  return *(volatile uint32_t*)PQ_DWT_CYCCNT_ADDR;
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
  float mix;
  uint8_t i;
  
  synth = getSynth(idx);
  if (!synth){
    return;
  }
  
  nrFrames = nextTxSize >> 2;
  if (nrFrames > SYNTH_SAMPLE_BUF_SIZE){
    return;
  }
  
  // DWT profiling start
  uint32_t t0 = pq_dwt_get();
  g_pq_dbg_last_nrFrames = nrFrames;
  // budget cycles for this buffer (uses SystemCoreClock if available)
  extern uint32_t SystemCoreClock;
  g_pq_dbg_budget_cycles = (uint32_t)(((uint64_t)nrFrames * (uint64_t)SystemCoreClock) / (uint64_t)DEFAULT_FS);
  
  
  // Per-module profiling (watch variables, no printf)
  g_pq_dbg_nrModules_last = synth->nrModules;
  if (g_pq_dbg_nrModules_last > g_pq_dbg_nrModules_max){
    g_pq_dbg_nrModules_max = g_pq_dbg_nrModules_last;
  }
  for (int mi = 0; mi < synth->nrVoiceModules; mi++){
    i = synth->voiceModuleIds[mi];
    uint32_t mp0 = pq_dwt_get();
    synth->modules[i].preProc(synth, nrFrames);
    uint32_t mp1 = pq_dwt_get();
    synth->modules[i].play(synth, nrFrames);
    uint32_t mp2 = pq_dwt_get();
    uint32_t c_pre = (mp1 - mp0);
    uint32_t c_play = (mp2 - mp1);
    uint32_t c_total = (mp2 - mp0);
    g_pq_dbg_mod_pre_last[i] = c_pre;
    g_pq_dbg_mod_play_last[i] = c_play;
    g_pq_dbg_mod_total_last[i] = c_total;
    if (c_pre > g_pq_dbg_mod_pre_max[i]){ g_pq_dbg_mod_pre_max[i] = c_pre; }
    if (c_play > g_pq_dbg_mod_play_max[i]){ g_pq_dbg_mod_play_max[i] = c_play; }
    if (c_total > g_pq_dbg_mod_total_max[i]){ g_pq_dbg_mod_total_max[i] = c_total; }
  }
  
  for (uint32_t i = 0; i < nrFrames; i++){
    mix = 0.0f;
    for (int v = 0; v < SYNTH_MAX_VOICE; v++){
      if (synth->voices[v].activeFlg){
        mix += synth->voiceBuf[v][i];
      }
    }
    synth->mixBuf[i] = mix;
  }
  
  for (int mi = 0; mi < synth->nrPostModules; mi++){
    i = synth->postModuleIds[mi];
    uint32_t mp0 = pq_dwt_get();
    synth->modules[i].preProc(synth, nrFrames);
    uint32_t mp1 = pq_dwt_get();
    synth->modules[i].play(synth, nrFrames);
    uint32_t mp2 = pq_dwt_get();
    uint32_t c_pre = (mp1 - mp0);
    uint32_t c_play = (mp2 - mp1);
    uint32_t c_total = (mp2 - mp0);
    g_pq_dbg_mod_pre_last[i] = c_pre;
    g_pq_dbg_mod_play_last[i] = c_play;
    g_pq_dbg_mod_total_last[i] = c_total;
    if (c_pre > g_pq_dbg_mod_pre_max[i]){ g_pq_dbg_mod_pre_max[i] = c_pre; }
    if (c_play > g_pq_dbg_mod_play_max[i]){ g_pq_dbg_mod_play_max[i] = c_play; }
    if (c_total > g_pq_dbg_mod_total_max[i]){ g_pq_dbg_mod_total_max[i] = c_total; }
  }
  
  s16Out = (int16_t*)buf;
  for (int i = 0; i < nrFrames; i++) {
    mix = synth->mixBuf[i];
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
  
  // DWT profiling end
  uint32_t t1 = pq_dwt_get();
  g_pq_dbg_last_cycles = (t1 - t0);
  if (g_pq_dbg_last_cycles > g_pq_dbg_max_cycles){
    g_pq_dbg_max_cycles = g_pq_dbg_last_cycles;
  }
  if (g_pq_dbg_last_cycles > g_pq_dbg_budget_cycles){
    g_pq_dbg_overrun_count++;
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
  synth->moduleBitMask = 0;
  synth->nrModules = 0;
  synth->nrVoiceModules = 0;
  synth->nrPostModules = 0;
  
  /*Module Initialization*/
  //InitVCO();
  //InitVCOWaveTable(synth);
  InitVCOKerplusStrong(synth);
  InitDjentGate_Key(synth);
  InitDjentAmp(synth);
  InitDjentGate_Apply(synth);
  InitDjentCab(synth);
  //InitVCA(synth);
  
}

void PQSynth_Init(void)
{
  pq_dwt_init();
  g_pq_dbg_last_cycles = 0;
  g_pq_dbg_max_cycles = 0;
  g_pq_dbg_budget_cycles = 0;
  g_pq_dbg_overrun_count = 0;
  g_pq_dbg_last_nrFrames = 0;
  g_pq_dbg_nrModules_last = 0;
  g_pq_dbg_nrModules_max = 0;
  for (int i = 0; i < SYNTH_MAX_MODULES; i++){
    g_pq_dbg_mod_pre_last[i] = 0;
    g_pq_dbg_mod_pre_max[i] = 0;
    g_pq_dbg_mod_play_last[i] = 0;
    g_pq_dbg_mod_play_max[i] = 0;
    g_pq_dbg_mod_total_last[i] = 0;
    g_pq_dbg_mod_total_max[i] = 0;
  }
  
  
  PQ_Init(POWERQUAD);
  
	/*USB Host Callback*/
  UsbhAudio_SetReadyNotify(deviceNotify);
  
  /*MIDI Callback*/
  MIDI_SetCallback(MIDI_CIN_NOTE_OFF, noteOff);
  MIDI_SetCallback(MIDI_CIN_NOTE_ON, noteOn);
	
  init(44100.0f, 0.5f / (float)SYNTH_MAX_VOICE);
}

void PQSynth_InitRecMode(void)
{
  pq_dwt_init();
  g_pq_dbg_last_cycles = 0;
  g_pq_dbg_max_cycles = 0;
  g_pq_dbg_budget_cycles = 0;
  g_pq_dbg_overrun_count = 0;
  g_pq_dbg_last_nrFrames = 0;
  g_pq_dbg_nrModules_last = 0;
  g_pq_dbg_nrModules_max = 0;
  for (int i = 0; i < SYNTH_MAX_MODULES; i++){
    g_pq_dbg_mod_pre_last[i] = 0;
    g_pq_dbg_mod_pre_max[i] = 0;
    g_pq_dbg_mod_play_last[i] = 0;
    g_pq_dbg_mod_play_max[i] = 0;
    g_pq_dbg_mod_total_last[i] = 0;
    g_pq_dbg_mod_total_max[i] = 0;
  }
  PQ_Init(POWERQUAD);
  
	pqSynth_t* synth = &st_Synth[0];
 
  synth->deviceIndex = 1;
  
  VcomRec_SetNoteOn(noteOn);
  VcomRec_SetPlayCallback(play);
	
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
  synth->voiceModuleIds[synth->nrVoiceModules] = module->moduleID;
  synth->nrVoiceModules++;
  synth->nrModules++;
  synth->moduleBitMask |= retBitMap;
  
  return retBitMap;
}

uint32_t PQSynth_RegisterPostMixModule(pqSynth_Module_t* newModule)
{
  uint32_t retBitMap;
  pqSynth_Module_t* module;
  pqSynth_t* synth = &st_Synth[0];
  if (synth->nrModules == SYNTH_MAX_MODULES){
    return 0xFFFFFFFF;
  }
  if (synth->nrPostModules == SYNTH_MAX_MODULES){
    return 0xFFFFFFFF;
  }
  module = &synth->modules[synth->nrModules];
  module->name = newModule->name;
  module->play = newModule->play;
  module->preProc = newModule->preProc;
  module->moduleID = synth->nrModules;
  retBitMap = (1 << synth->nrModules);
  synth->postModuleIds[synth->nrPostModules] = module->moduleID;
  synth->nrPostModules++;
  synth->nrModules++;
  synth->moduleBitMask |= retBitMap;
  
  return retBitMap;
}

