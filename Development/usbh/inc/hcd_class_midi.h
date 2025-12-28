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
#define HCD_MIDI_OUT_RINGBUF_SIZE     64

#define MIDI_DIR_IN                   0x80
#define MIDI_DIR_OUT                  0x00

#define HcdMIDI_IRQn HSCMP2_IRQn

typedef union{
  struct{
    uint8_t cin:4;
    uint8_t cn:4;
    uint8_t status;
    uint8_t noteNum;
    uint8_t velocity;
  } FIELD;
  uint32_t DWORD;
} usb_MidiPacket_t;

typedef enum {
  MS_HEADER = 1,
  MIDI_IN_JACK,
  MIDI_OUT_JACK,
  ELEMENT,
}UMC_Intf_t;

typedef enum {
  MS_GENERAL = 1,
}UMC_Ep_t;

typedef enum{
  JACK_EMBEDDED = 1,
  JACK_EXTERNAL,
}UMC_Jack_t;

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
  csUsbDesc_MidiOutJk_t* outJack;
} hcd_MIDI_Cable_t;

typedef struct{
  usbDesc_Endpoint_t* epDesc;
  csUsbDesc_MidiStrmBulkEndpt_t* csEpDesc;
  uint8_t nrCables;
  hcd_MIDI_Cable_t cables[HCD_MIDI_MAX_CABLE_PER_EP];
  usb_MidiPacket_t* dataBuf;
  void (*completeCallback)(usb_MidiPacket_t* buf, uint16_t nrMidiPkt);
} hcd_MIDI_Ep_t;

typedef struct{
  uint8_t index;
  hcd_DeviceInfo_t* device;
  usbDesc_Interface_t* intf;
  hcd_MIDI_Ep_t bulkOut;
  hcd_MIDI_Ep_t bulkIn;
  void (*midiDeviceReady)(uint8_t idx, uint8_t dir);
  csUsbDesc_MidiInJk_t *inJackTbl[HCD_MIDI_MAX_JACK_NUM];
  csUsbDesc_MidiOutJk_t *outJackTbl[HCD_MIDI_MAX_JACK_NUM];
} hcd_MIDI_Info_t;

enum{
  HCD_MIDI_BULK_IN = 1,
  HCD_MIDI_BULK_OUT_COMP,
  HCD_MIDI_BULK_OUT_SEND
};

typedef struct{
  uint8_t msgType;
  uint8_t devAddr;
  uint16_t transLen;
  usb_MidiPacket_t umidiPkt;
} hcd_MIDI_Msg_t;

typedef struct{
  hcd_MIDI_Msg_t msg[HCD_MIDI_MSGBOX_SIZE];
  uint8_t enqPtr;
  uint8_t deqPtr;
} hcd_MIDI_MsgBox_t;

typedef struct{
  uint8_t devAddr;
  usb_MidiPacket_t umidi[HCD_MIDI_OUT_RINGBUF_SIZE];
  uint32_t enqPtr;
  uint32_t deqPtr;
} hcd_MIDI_RingBuf_t;

void HcdMIDI_InitMidiClass(void);
void UsbhMIDI_SetReadyNotify(void func(uint8_t, uint8_t));
void UsbhMIDI_SetInCallback(uint8_t index, void func(usb_MidiPacket_t* buf, uint16_t nrMidiPkt));

#endif /*__HCD_CLASS_MIDI_H__*/
