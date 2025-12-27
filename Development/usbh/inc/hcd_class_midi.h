/**
* @brief   MCXN947V USB Host Controller MIDI Class Driver
* @author  masa
* @version 1.00 
*/

#ifndef __HCD_CLASS_MIDI_H__
#define __HCD_CLASS_MIDI_H__

#include "hcd_class_mgr_audio.h"

#define HCD_MIDI_MAX_CABLE_PER_EP     4
#define HCD_MIDI_MAX_JACK_NUM         8
#define NUM_OF_MAX_MIDI_DEVICE        1
#define HCD_MIDI_MSGBOX_SIZE          64
#define HCD_MIDI_OUT_RINGBUF_SIZE     256



typedef struct{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubtype;
	uint8_t bcdMSC_lsB;
	uint8_t bcdMSC_msB;
	uint8_t wTotalLength_lsB;
	uint8_t wTotalLength_msB;
} csUsbDesc_MidiStrmIfHdr_t;   // Midi Streaming Interface Header Desc.

typedef struct{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubtype;
	uint8_t bJackType;
	uint8_t bJackID;
	uint8_t iJack;
} csUsbDesc_MidiInJk_t;		//	Midi In Jack Desc.

typedef struct{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubtype;
	uint8_t bJackType;
	uint8_t bJackID;
	uint8_t bNrInputPins;
	uint8_t baSourceID;
	uint8_t baSourcePin;
	uint8_t iJack;
} csUsbDesc_MidiOutJk_t;		// Midi Out Jack Desc.

typedef struct{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubtype;
	uint8_t bNumEmbMIDIJack;
	uint8_t baAssocJackID;
} csUsbDesc_MidiStrmBulkEndpt_t; // Class Spesific MIDI Streaming Bulk Data Endpoint Desc.

typedef struct{
  csUsbDesc_MidiInJk_t* inJack;
  csUsbDesc_MidiOutJk_t* outJk;
} hcd_MIDI_Cable_t;

typedef struct{
  usbDesc_Endpoint_t* epDesc;
  csUsbDesc_MidiStrmBulkEndpt_t* csEpDesc;
  uint8_t nrCables;
  hcd_MIDI_Cable_t cables[HCD_MIDI_MAX_CABLE_PER_EP];
} hcd_MIDI_Ep_t;

typedef struct{
  hcd_DeviceInfo_t* device;
  usbDesc_Interface_t* intf;
  hcd_MIDI_Ep_t bulkOut;
  hcd_MIDI_Ep_t bulkIn;
  csUsbDesc_MidiInJk_t *inJackTbl[HCD_MIDI_MAX_JACK_NUM];
  csUsbDesc_MidiOutJk_t *outJackTbl[HCD_MIDI_MAX_JACK_NUM];
} hcd_MIDI_Info_t;

typedef union{
  struct{
    uint8_t cn:4;
    uint8_t cin:4;
    uint8_t status;
    uint8_t noteNum;
    uint8_t velocity;
  } FIELD;
  uint32_t DWORD;
} usb_MidiPacket_t;

enum{
  HCD_MIDI_BULK_IN = 1,
  HCD_MIDI_BULK_OUT,
};

typedef struct{
  uint8_t msgType;
  uint8_t devAddr;
  usb_MidiPacket_t umidiPkt;
} hcd_MIDI_Msg_t;

typedef struct{
  hcd_MIDI_Msg_t msg[HCD_MIDI_MSGBOX_SIZE];
  uint8_t enqPtr;
  uint8_t deqPtr;
} hcd_MIDI_MsgBox_t;

#endif /*__HCD_CLASS_MIDI_H__*/
