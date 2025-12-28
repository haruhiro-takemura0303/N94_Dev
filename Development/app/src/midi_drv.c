/**
* @brief   FRDM-MCXN947 MIDI Driver
* @author  masa
* @version 1.00
*/

#include "hcd_class_midi.h"
#include "midi.h"

static midi_Callback_t st_Callback;

void midiIn(usb_MidiPacket_t* buf, uint16_t nrMidiPkt)
{
  for (int i = 0; i < nrMidiPkt; i++){
    switch(buf[i].FIELD.cin){
      case(MIDI_CIN_NOTE_OFF):{
        if (st_Callback.noteOff){
          st_Callback.noteOff(buf[i].FIELD.noteNum, buf[i].FIELD.velocity);
        }
        break;
      }
      case(MIDI_CIN_NOTE_ON):{
        if (st_Callback.noteOn){
          st_Callback.noteOn(buf[i].FIELD.noteNum, buf[i].FIELD.velocity);
        }
        break;
      }
      case(MIDI_CIN_POLY_KEY_PRESS):{
        if (st_Callback.polyAft){
          st_Callback.polyAft(buf[i].FIELD.noteNum, buf[i].FIELD.velocity);
        }
        break;
      }
      case(MIDI_CIN_CONTROL_CHANGE):{
        if (st_Callback.ctrlChg){
          st_Callback.ctrlChg(buf[i].FIELD.noteNum, buf[i].FIELD.velocity);
        }
        break;
      }
      case(MIDI_CIN_PROGRAM_CHANGE):{
        if (st_Callback.progChg){
          st_Callback.progChg(buf[i].FIELD.noteNum, buf[i].FIELD.velocity);
        }
        break;
      }
      case(MIDI_CIN_CHANNEL_PRESSURE):{
        if (st_Callback.chanAft){
          st_Callback.chanAft(buf[i].FIELD.noteNum, buf[i].FIELD.velocity);
        }
        break;
      }
      case(MIDI_CIN_PITCH_BEND_CHANGE):{
        if (st_Callback.pitchBend){
          st_Callback.pitchBend(buf[i].FIELD.noteNum, buf[i].FIELD.velocity);
        }
        break;
      }
      default:
      break;
    }
  }
}

static void midiDeviceReady(uint8_t index, uint8_t dir)
{
  if (dir == MIDI_DIR_IN){
    UsbhMIDI_SetInCallback(index, midiIn);
  }
}

void MIDI_SetCallback(uint8_t cin, genericMidiFunc_t func)
{
  switch(cin){
    case(MIDI_CIN_NOTE_OFF):          st_Callback.noteOff = func;   break;
    case(MIDI_CIN_NOTE_ON):           st_Callback.noteOn = func;    break;
    case(MIDI_CIN_POLY_KEY_PRESS):    st_Callback.polyAft = func;   break;
    case(MIDI_CIN_CONTROL_CHANGE):    st_Callback.ctrlChg = func;   break;
    case(MIDI_CIN_PROGRAM_CHANGE):    st_Callback.progChg = func;   break;
    case(MIDI_CIN_CHANNEL_PRESSURE):  st_Callback.chanAft = func;   break;
    case(MIDI_CIN_PITCH_BEND_CHANGE): st_Callback.pitchBend = func; break;
    default: break;
  }
}

void InitMidiDriver(void)
{
  UsbhMIDI_SetReadyNotify(midiDeviceReady);
}
