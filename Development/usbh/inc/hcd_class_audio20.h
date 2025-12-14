/**
* @brief   MCXN947V USB Host Controller Audio Class 2.0 Driver
* @author  masa
* @version 1.00 
*/

#ifndef __HCD_CLASS_AUDIO_20_H__
#define __HCD_CLASS_AUDIO_20_H__

#include "hcd_class_audio.h"

#define HCD_UAC20_MAX_CLK_SRC_SUBRANGE 8
#define HCD_UAC20_MAX_CLK_SRC       4
#define HCD_UAC20_MAX_VOL_SUBRANGE 16

#define HCD_UAC20_MAX_ENTITY_ID   32
#define HCD_UAC20_MAX_ALTSET      8

enum{
	TERMINAL_TYPE_USB = 0x0101,
	TERMINAL_TYPE_MIC = 0x0201,
	TERMINAL_TYPE_SPK = 0x0301,
};

typedef struct{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubtype;
	uint8_t bcdADC_lsB;
	uint8_t bcdADC_msB;
	uint8_t bCategory;
	uint8_t wTotalLength_lsB;
	uint8_t wTotalLength_msB;
	uint8_t bmControls;
} csUsbDesc_AudioCtrlIfHdr2_t; // Audio Control Interface Header (ADC2.0) Desc.

typedef struct{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubtype;
	uint8_t bClockID;
	uint8_t bmAttributes;
	uint8_t bmControls;
	uint8_t bAssocTerminal;
	uint8_t iClockSource;
} csUsbDesc_AudioCtrlIfClkSrc_t; // Audio Control Interface Clock Source Desc.

typedef struct{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubtype;
	uint8_t bClockID;
	uint8_t bNrInPins;
	uint8_t baCSourceID;
	//bmControls, iClockSelector
} csUsbDesc_AudioCtrlIfClkSel_t; // Audio Control Interface Clock Selector Desc.

typedef struct{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubtype;
	uint8_t bClockID;
	uint8_t bCSourceID;
	uint8_t bmControls;
	uint8_t iClockMultplier;
} csUsbDesc_AudioCtrlIfClkMult_t; // Audio Control Interface Clock Multiplier Desc.

typedef struct{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubtype;
	uint8_t bTerminalID;
	uint8_t wTerminalType_lsB;
	uint8_t wTerminalType_msB;
	uint8_t bAssocTerminal;
	uint8_t bCSourceID;
	uint8_t bNrChannels;
	uint8_t bmChannelConfig_0thB;
	uint8_t bmChannelConfig_1stB;
	uint8_t bmChannelConfig_2ndB;
	uint8_t bmChannelConfig_3rdB;
	uint8_t iChannelNames;
	uint8_t bmControls_lsB;
	uint8_t bmControls_msB;
	uint8_t iTerminal;
} csUsbDesc_AudioCtrlInputTerm2_t;  // Audio Control Input Terminal (ADC2.0) Desc.

typedef struct{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubtype;
	uint8_t bTerminalID;
	uint8_t wTerminalType_lsB;
	uint8_t wTerminalType_msB;
	uint8_t bAssocTerminal;
	uint8_t bSourceID;
	uint8_t bCSourceID;
	uint8_t bmControls_lsB;
	uint8_t bmControls_msB;
	uint8_t iTerminal;
} csUsbDesc_AudioCtrlOutputTerm2_t;  // Audio Control Output Terminal (ADC2.0) Desc.

typedef struct{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubtype;
	uint8_t bUnitID;
	uint8_t bNrInPins;
	uint8_t baSourceID;
	/*After baSourceID -> bNrChannels, bmChannelConfig, iChannelNames, bmMixerControls, bmControl, iMixer*/
}csUsbDesc_AudioCtrlMixUnit2_t;		//Audio Control Mixer Unit (ADC2.0) Desc.

typedef struct{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubtype;
	uint8_t bUnitID;
	uint8_t bSourceID;
	uint8_t bmaControls_0thB;
	uint8_t bmaControls_1stB;
	uint8_t bmaControls_2ndB;
	uint8_t bmaControls_3rdB;
	/*After bmaControls -> iFeature*/
}csUsbDesc_AudioCtrlFeatUnit2_t; //Audio Control Feature Unit Desc.

typedef struct{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubtype;
	uint8_t bUnitID;
	uint8_t bNrInPins;
	uint8_t baSourceID;
	/*After baSourceID -> bmControls, iSelector*/
}csUsbDesc_AudioCtrlSelUnit2_t;  // Audio Control Selector Unit Desc.

typedef struct{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubtype;
	uint8_t bUnitID;
	uint8_t wExtensionCode_lsB;
	uint8_t wExtensionCode_msB;
	uint8_t bNrInPins;
	uint8_t baSourceID;
	/*After baSourceID -> bNrChannels, bmChannelConfig, iChannelNames, bmControls, iExtension*/
}csUsbDesc_AudioCtrlExtUnit_t;  // Audio Control Selector Unit Desc.

typedef struct{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubtype;
	uint8_t bTerminalLink;
	uint8_t bmControls;
	uint8_t bFormatType;
	uint8_t bmFormats_0thB;
	uint8_t bmFormats_1stB;
	uint8_t bmFormats_2ndB;
	uint8_t bmFormats_3rdB;
	uint8_t bNrChannels;
	uint8_t bmChannelConfig_0thB;
	uint8_t bmChannelConfig_1stB;
	uint8_t bmChannelConfig_2ndB;
	uint8_t bmChannelConfig_3rdB;
	uint8_t iChannelNames;
} csUsbDesc_AudioStrmIf2_t;  // Audio Streaming Class Specific Interface (ADC2.0) Desc.

typedef struct{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubtype;
	uint8_t bFormatType;
	uint8_t bSubslotSize;
	uint8_t bBitResolution;
} csUsbDesc_AudioStrmFmtTypI2_t;		// Audio Streaming Format Type I (ADC2.0) Desc.

typedef struct{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubtype;
	uint8_t bmAttributes;
	uint8_t bmControls;
	uint8_t bLockDelayUnit;
	uint8_t wLockDelay_lsB;
	uint8_t wLockDelay_msB;
} csUsbDesc_AudioStrmDataEndpt2_t;		// Audio Streaming Class Specific Audio Data Endpoint (ADC2.0) Desc.

typedef struct{
  uint8_t type;
  uint8_t sourceID;
  uint8_t cSourceID;
  uint8_t usbTerm;
  void* descPtr;
} hcd_UAC20_EntityMap_t;

typedef struct{
  usbDesc_Interface_t* intfPtr;
  csUsbDesc_AudioStrmIf2_t* strmIfPtr;
  csUsbDesc_AudioStrmFmtTypI2_t* fmtPtr;
  usbDesc_Endpoint2_t* epPtr;
  csUsbDesc_AudioStrmDataEndpt2_t* csEpPtr;
} hcd_UAC20_AltSet_t;

typedef struct{
  uint32_t dMin;
  uint32_t dMax;
  uint32_t dRes;
} hcd_UAC20_ClockSubrange_t;

typedef struct{
  uint16_t clockID;
  uint16_t numOfSubrange;
  hcd_UAC20_ClockSubrange_t subRange[HCD_UAC20_MAX_CLK_SRC_SUBRANGE];
} hcd_UAC20_ClockSrcInfo_t;

typedef struct{
  hcd_DeviceInfo_t* device;
  struct{
    usbDesc_Interface_t* intfPtr;
    usbDesc_Endpoint2_t* epPtr;
    hcd_UAC20_EntityMap_t entity[HCD_UAC20_MAX_ENTITY_ID];
    hcd_UAC20_ClockSrcInfo_t clockSrc[HCD_UAC20_MAX_CLK_SRC];
  } control;
  struct{
    uint8_t numOfAltSet;
    hcd_UAC20_AltSet_t altSet[HCD_UAC20_MAX_ALTSET];
  } streamOut;
  struct{
    uint8_t numOfAltSet;
    hcd_UAC20_AltSet_t altSet[HCD_UAC20_MAX_ALTSET];
  } streamIn;
} hcd_UAC20_Info_t;


#endif /*__HCD_CLASS_AUDIO_20_H__*/
