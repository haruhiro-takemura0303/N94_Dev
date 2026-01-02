/**
* @brief   FRDM-MCXN947 board PowerQuad Musical Synthsizer App
* @author  masa
* @version 1.00
*/

#ifndef __PQ_SYNTH_H__
#define __PQ_SYNTH_H__

#include <stdint.h>
#include <math.h>
#include <cmsis_armclang.h>
#include "fsl_powerquad.h"


#define M_PI 3.14159265358979323846

#define DEFAULT_FS              44100

#define SYNTH_MAX_NUM           1
#define SYNTH_MAX_VOICE         4
#define SYNTH_SAMPLE_BUF_SIZE   ((uint32_t)((((DEFAULT_FS / 100) * 4) + 9) / 10))
#define SYNTH_MAX_MODULES       8

typedef struct __pqsynth pqSynth_t;

typedef struct{
  uint8_t activeFlg;
  uint8_t noteNum;
  uint8_t velocity;
  volatile uint32_t noteEvPendFlg;
} pqSynth_Voice_t;

typedef void audioProcFunc (pqSynth_t* synth, uint32_t frames);

typedef struct{
  uint8_t moduleID;
  uint32_t moduleBit;
  const char* name;
  audioProcFunc* preProc;
  audioProcFunc* play;
} pqSynth_Module_t;

struct __pqsynth{
  uint8_t deviceIndex;
  float sampleRate;
  float ampMax;
  uint32_t moduleBitMask;
  pqSynth_Voice_t voices[SYNTH_MAX_VOICE];
  float voiceBuf[SYNTH_MAX_VOICE][SYNTH_SAMPLE_BUF_SIZE];
  float mixBuf[SYNTH_SAMPLE_BUF_SIZE];
  uint8_t voiceModuleIds[SYNTH_MAX_MODULES];
  uint8_t postModuleIds[SYNTH_MAX_MODULES];
  uint8_t nrVoiceModules;
  uint8_t nrPostModules;
  float gpWorkMem0[SYNTH_MAX_VOICE * SYNTH_SAMPLE_BUF_SIZE];
  float gpWorkMem1[SYNTH_MAX_VOICE * SYNTH_SAMPLE_BUF_SIZE];
  pqSynth_Module_t modules[SYNTH_MAX_MODULES];
  uint8_t nrModules;
};

void PQSynth_Init(void);
void PQSynth_InitRecMode(void);
uint32_t PQSynth_RegisterModule(pqSynth_Module_t* newModule);
uint32_t PQSynth_RegisterPostMixModule(pqSynth_Module_t* newModule);


// Debug (watch variables): DWT cycle counter based profiling
extern volatile uint32_t g_pq_dbg_last_cycles;
extern volatile uint32_t g_pq_dbg_max_cycles;
extern volatile uint32_t g_pq_dbg_budget_cycles;
extern volatile uint32_t g_pq_dbg_overrun_count;
extern volatile uint32_t g_pq_dbg_last_nrFrames;

extern volatile uint32_t g_pq_dbg_mod_pre_last[SYNTH_MAX_MODULES];
extern volatile uint32_t g_pq_dbg_mod_pre_max[SYNTH_MAX_MODULES];
extern volatile uint32_t g_pq_dbg_mod_play_last[SYNTH_MAX_MODULES];
extern volatile uint32_t g_pq_dbg_mod_play_max[SYNTH_MAX_MODULES];
extern volatile uint32_t g_pq_dbg_mod_total_last[SYNTH_MAX_MODULES];
extern volatile uint32_t g_pq_dbg_mod_total_max[SYNTH_MAX_MODULES];
extern volatile uint32_t g_pq_dbg_nrModules_last;
extern volatile uint32_t g_pq_dbg_nrModules_max;

#endif /*__PQ_SYNTH_H__*/
