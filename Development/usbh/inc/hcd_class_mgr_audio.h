/**
* @brief   MCXN947V USB Host Controller Audio Class Driver Distribution Manager
* @author  masa
* @version 1.00 
*/

#ifndef __HCD_CLASS_MGR_AUDIO_H__
#define __HCD_CLASS_MGR_AUDIO_H__

#include "hcd_class.h"

#define USB_CLASSCODE_AUDIO   0x01
typedef enum {
  UAC_CONTROL = 1,
  UAC_STREAMING,
  UAC_MIDI
}UAC_SubClass_t;

typedef struct{
  struct{
    uint8_t existAudio:4;
    uint8_t existMidi:4;
  }clsDist;
} hcd_AudioMgr_t;

hcd_Status_t HcdAudioMgr_RegisterAudioDriver(hcd_ClassDriver_t* map);
hcd_Status_t HcdAudioMgr_RegisterMidiDriver(hcd_ClassDriver_t* map);
hcd_Status_t HcdAudioMgr_InitUAC(void);

#endif /*__HCD_CLASS_MGR_AUDIO_H__*/
