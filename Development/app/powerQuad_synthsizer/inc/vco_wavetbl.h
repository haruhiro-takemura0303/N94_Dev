/**
* @brief   FRDM-MCXN947 board Voltage-Controled Oscillator for Musical Synthsizer App 
* @author  masa
* @version 1.00
*/

#ifndef __VCO_WAVETBL_H__
#define __VCO_WAVETBL_H__

#include "pq_synth.h"
#include "vco.h"

#define VCOWT_TABLE_BITSIZE          (9u)
#define VCOWT_WAVETABLE_SIZE         (1 << VCOWT_TABLE_BITSIZE)  // power of two
#define VCOWT_NUM_OF_HARMONICS       (8u)


#define VCOWT_PHASE_FRAC_BITSIZE     (32u - VCOWT_TABLE_BITSIZE)
#define VCOWT_PHASE_FRAC_BITMASK     ((uint32_t)((1ull << VCOWT_PHASE_FRAC_BITSIZE) - 1ull))
#define VCOWT_PHASE_TO_INDEX(p)      ((uint32_t)(p >> VCOWT_PHASE_FRAC_BITSIZE))
#define VCOWT_FRAC_TO_FLOAT(p)       ((float)(((uint32_t)(p) & VCOWT_PHASE_FRAC_BITMASK) * (1.0f / (float)(1ull << VCOWT_PHASE_FRAC_BITSIZE))))

typedef struct{
  uint32_t phaseInc;
  uint32_t phaseFixedFrac;
} pqSynth_VCOWT_Voice_t;

typedef struct{
  uint32_t bitMask;
  pqSynth_VCOWT_Voice_t voice[SYNTH_MAX_VOICE];
}pqSynth_VCOWT_t;

void InitVCOWaveTable(pqSynth_t* synth);

#endif /*__VCO_WAVETBL_H__*/
