/**
* @brief   MCXN947V USB Host Controller Audio Class Driver
* @author  masa
* @version 1.00 
*/

#ifndef __HCD_CLASS_AUDIO_H__
#define __HCD_CLASS_AUDIO_H__

#include "hcd_class.h"

typedef enum {
  HEADER = 1,
  INPUT_TERMINAL,
  OUTPUT_TERMINAL,
  MIXER_UNIT,
  SELECTOR_UNIT,
  FEATURE_UNIT,
  EFFECT_UNIT,
  PROCESSING_UNIT,
  EXTENSION_UNIT,
  CLOCK_SOURCE,
  CLOCK_SELECTOR,
  CLOCK_MULTIPLIER,
  SAMPLE_RATE_CONVERTER, 
}UAC_ControlIntf_t;

typedef enum {
  IF_GENERAL = 1,
  FORMAT_TYPE,
  FORMAT_SPECIFIC,
}UAC_StreamIntf_t;

typedef enum {
  EP_GENERAL = 1,
}UAC_StreamingEp_t;

typedef enum {
  DEVICE = 1,
  CONFIG = 2,
  STRING = 3,
  INTERFACE = 4,
  ENDPOINT = 5,
  INTERFACEASSOC = 11,
  CS_INTERFACE = 0x24,
  CS_ENDPOINT = 0x25,
}UAC_Desctype_t;

enum {
  CONTROL = 1,
  STREAMING,
  MIDI
}UAC_SubClass_t;



#endif /*__HCD_CLASS_AUDIO_H__*/
