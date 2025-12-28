/**
* @brief   FRDM-MCXN947 MIDI Definitions
* @author  masa
* @version 1.00
*/

#ifndef __MIDI_H__
#define __MIDI_H__

#include <stdint.h>

enum{
  MIDI_CIN_MISC_FUNCTION        = 0x0, /* Miscellaneous function codes (reserved) */
  MIDI_CIN_CABLE_EVENT          = 0x1, /* Cable events (reserved) */
  
  MIDI_CIN_SYSCOM_2BYTE         = 0x2, /* System Common, 2 bytes (e.g. MTC, Song Select) */
  MIDI_CIN_SYSCOM_3BYTE         = 0x3, /* System Common, 3 bytes (e.g. SPP) */
  
  MIDI_CIN_SYSEX_START_CONTINUE = 0x4, /* SysEx starts or continues */
  MIDI_CIN_SYSEX_END_1BYTE      = 0x5, /* SysEx ends with 1 byte */
  MIDI_CIN_SYSEX_END_2BYTE      = 0x6, /* SysEx ends with 2 bytes */
  MIDI_CIN_SYSEX_END_3BYTE      = 0x7, /* SysEx ends with 3 bytes */
  
  MIDI_CIN_NOTE_OFF             = 0x8, /* Note Off */
  MIDI_CIN_NOTE_ON              = 0x9, /* Note On */
  MIDI_CIN_POLY_KEY_PRESS       = 0xA, /* Polyphonic Key Pressure */
  
  MIDI_CIN_CONTROL_CHANGE       = 0xB, /* Control Change */
  MIDI_CIN_PROGRAM_CHANGE       = 0xC, /* Program Change */
  MIDI_CIN_CHANNEL_PRESSURE     = 0xD, /* Channel Pressure */
  
  MIDI_CIN_PITCH_BEND_CHANGE    = 0xE, /* Pitch Bend Change */
  
  MIDI_CIN_SINGLE_BYTE          = 0xF  /* Single Byte (unparsed / real-time, etc.) */
};

typedef void genericMidiFunc_t (uint8_t, uint8_t);

typedef struct{
  genericMidiFunc_t* noteOff;
  genericMidiFunc_t* noteOn;
  genericMidiFunc_t* polyAft;
  genericMidiFunc_t* ctrlChg;
  genericMidiFunc_t* progChg;
  genericMidiFunc_t* chanAft;
  genericMidiFunc_t* pitchBend;
} midi_Callback_t;

void InitMidiDriver(void);
void MIDI_SetCallback(uint8_t cin, genericMidiFunc_t func);

#endif /*__MIDI_H__*/
