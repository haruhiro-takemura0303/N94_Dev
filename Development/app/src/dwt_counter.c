/**
* @brief   MCXN947 General-Purpose DWT Performance Counter 
* @author  masa
* @version 1.00
*/

#include "dwt_counter.h"
#include <string.h>

uint32_t g_DwtLimitCyc;
dwt_Counter_t g_DwtLast;
dwt_Counter_t g_DwtMax;

dwt_Counter_Internal_t st_DwtInternal;
dwt_Counter_t st_DwtInternalMem;

static inline volatile uint32_t dwtGet(void)
{
  return DWT->CYCCNT;
}

void DwtCounter_Init(void)
{
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

void DwtCounter_Hook(void)
{
  st_DwtInternal.startCount = dwtGet();
  st_DwtInternal.curLap = 0;
  memset(&st_DwtInternalMem, 0, sizeof(dwt_Counter_t));
}

void DwtCounter_SetLimit(uint32_t limit)
{
  g_DwtLimitCyc = limit;
}

void DwtCounter_Lap(void)
{
  uint32_t cur;
  uint8_t lapIdx;
  
  cur = dwtGet();
  lapIdx = st_DwtInternal.curLap;
  if (lapIdx < DWT_COUNTER_MAX_LAP){
    st_DwtInternalMem.lap[lapIdx] = cur;
    if (lapIdx == 0){
      g_DwtLast.lap[lapIdx] = cur - st_DwtInternal.startCount;
    } else {
      g_DwtLast.lap[lapIdx] = cur - st_DwtInternalMem.lap[lapIdx - 1];
    }
    if (g_DwtLast.lap[lapIdx] > g_DwtMax.lap[lapIdx]){
      g_DwtMax.lap[lapIdx] = g_DwtLast.lap[lapIdx];
    }
  }
  st_DwtInternal.curLap++;
}

void DwtCounter_End(void)
{
  uint32_t cur;
  uint8_t lapIdx;
  
  cur = dwtGet();
  lapIdx = st_DwtInternal.curLap;
  
  if (lapIdx < DWT_COUNTER_MAX_LAP){
    st_DwtInternalMem.lap[lapIdx] = cur;
    if (lapIdx == 0){
      g_DwtLast.lap[lapIdx] = cur - st_DwtInternal.startCount;
    } else {
      g_DwtLast.lap[lapIdx] = cur - st_DwtInternalMem.lap[lapIdx - 1];
    }
    if (g_DwtLast.lap[lapIdx] > g_DwtMax.lap[lapIdx]){
      g_DwtMax.lap[lapIdx] = g_DwtLast.lap[lapIdx];
    }
  }
  st_DwtInternalMem.total = cur;
  g_DwtLast.total = cur - st_DwtInternal.startCount;
  if (g_DwtLast.total > g_DwtMax.total){
    g_DwtMax.total = g_DwtLast.total;
  }
}
