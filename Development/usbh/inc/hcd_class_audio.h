/**
* @brief   MCXN947V USB Host Controller Audio Class Driver
* @author  masa
* @version 1.00 
*/

#ifndef __HCD_CLASS_AUDIO_H__
#define __HCD_CLASS_AUDIO_H__

#include "hcd_class.h"

#define HCD_AUDIO_MSGBOX_SIZE 32
#define NUM_OF_MAX_AUDIO_DEVICE 1
#define DEFAULT_SAMPLING_RATE 44100
#define DEFAULT_NUM_OF_CHANNEL  2
#define DEFAULT_BIT_RESO_DIV8   2
#define HCD_AUDIO_EXPECTED_ISOCH_IN_DATA_PER_MFRAME ((((((DEFAULT_SAMPLING_RATE / 100) >> 2) / 10) + 1) >> 1) * DEFAULT_BIT_RESO_DIV8 * DEFAULT_NUM_OF_CHANNEL)
#define HcdAudio_IRQn HSCMP0_IRQn

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

enum{
  HCD_AUDIO_CTRL_REQ = 1,
  HCD_AUDIO_CTRL_REQ_DONE,
  HCD_INTERRUPT_COMPLETE,
  HCD_AUDIO_INITIAL_REQ,
  HCD_AUDIO_INITIAL_REQ_DONE,
  HCD_AUDIO_SET_SAMPLING_RATE,
  HCD_AUDIO_SAMPLING_RATE_UPDATED,
  HCD_AUDIO_STREAMING_START,
  HCD_AUDIO_CLASS_TERMINATED,
};

typedef struct {
  uint8_t msgType;
  uint8_t devAddr;
  uint8_t epNum;
  uint8_t intfNum;
  uint32_t* bufPtr;
  union{
    struct{
      uint32_t fs;
      uint16_t mps;
      uint8_t bitReso;
      uint8_t numOfChannels;
    }audio;
    usb_SetupPacket_t setup;
  }other;
}hcd_Audio_Msg_t;

typedef struct{
  hcd_Audio_Msg_t msg[HCD_AUDIO_MSGBOX_SIZE];
  uint8_t enqPtr;
  uint8_t deqPtr;
}hcd_Audio_MsgBox_t;

typedef struct{
  uint8_t init;
  uint8_t num;
  uint8_t intfNum;
  uint8_t interval;
  uint8_t numOfChannels;
  uint8_t bitReso;
  uint32_t fs;
  uint16_t mps;
} hcd_Audio_Endpoint_Info_t;

typedef struct{
  struct{
    struct{
      uint32_t buf[HCD_AUDIO_EXPECTED_ISOCH_IN_DATA_PER_MFRAME];
    }mFrame[MAX_iTD_TSC];
  }frame[HCD_PERIODIC_iTD_SINGLE_BUF];
} hcd_Audio_IsochIn_Raw_Buf_t;

typedef struct{
  hcd_DeviceInfo_t* device;
  struct{
    hcd_Audio_Endpoint_Info_t interrupt;
    hcd_Audio_Endpoint_Info_t isochOut;
    hcd_Audio_Endpoint_Info_t isochIn; 
  }ep;
  uint32_t *interruptBuf;
  uint32_t *isochOutBuf[2];
  void (*isochOutCallback)(uint32_t* buf, uint16_t nextTxSize);
  void (*isochInCallback)(uint32_t* buf, uint16_t currentTxSize);
  uint32_t *isochInContinuousBuf[2];
  hcd_Audio_IsochIn_Raw_Buf_t *isochInRaw[2];
}hcd_Audio_Transfer_Driver_t;

typedef struct{
  uint16_t (*parseControlInterface)(config_rawdesc_t *confRaw, hcd_Audio_Endpoint_Info_t* intr, hcd_DeviceInfo_t* device);
  uint16_t (*parseStreamingInterface)(config_rawdesc_t *confRaw, hcd_Audio_Endpoint_Info_t* isochOutEp, hcd_Audio_Endpoint_Info_t* isochInEp, hcd_DeviceInfo_t* device);
  hcd_Status_t (*sendInitialRequest)(hcd_DeviceInfo_t* device);
  hcd_Status_t (*setSamplingRate)(uint32_t fs, uint8_t bitReso, uint8_t ifNum, hcd_DeviceInfo_t* device);
  void (*requestDoneFromISR)(hcd_DeviceInfo_t* device, uint32_t* ep0Buf);
  void (*requestDone)(hcd_DeviceInfo_t* device, uint32_t setup0, uint32_t setup1);
}hcd_Audio_Protocol_Driver_t;

void InitAudioClass(void);
void InitUACProtocol(uint8_t revision, hcd_Audio_Protocol_Driver_t* protocol);
hcd_Status_t HcdAudio_SendMsg(hcd_Audio_Msg_t* msg);


#endif /*__HCD_CLASS_AUDIO_H__*/
