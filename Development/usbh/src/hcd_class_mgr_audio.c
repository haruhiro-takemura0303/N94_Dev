/**
* @brief   MCXN947V USB Host Controller Audio Class Driver Distribution Manager
* @author  masa
* @version 1.00 
*/

#include "hcd_class_mgr_audio.h"

static hcd_AudioMgr_t st_AudioDist[MAX_DEVICE_NUM];
static hcd_ClassDriver_t st_AudioDriver;
static hcd_ClassDriver_t st_MidiDriver;

static uint16_t parseInterface(config_rawdesc_t *confRaw, hcd_DeviceInfo_t* device)
{
  uint16_t instReadPtr = 0;
  uint16_t rtnReadBytes = 0;
  uint8_t bLen, devIdx;
  UAC_SubClass_t bIntfSub;
  usbDesc_Interface_t* descPtr;
  
  devIdx = device->devAddr;
  
  descPtr = (usbDesc_Interface_t*)(&confRaw->rawDesc[confRaw->readPtr]);
  if (descPtr->bInterfaceClass != USB_CLASSCODE_AUDIO){
    return 0;
  }
  
  bIntfSub = (UAC_SubClass_t)descPtr->bInterfaceSubclass;
  
  switch(bIntfSub){
    case(UAC_CONTROL):{
      instReadPtr = confRaw->readPtr;
      bLen = confRaw->rawDesc[instReadPtr];
      instReadPtr += bLen;
      while(confRaw->rawDesc[instReadPtr + 1] != DESCTYPE_INTERFACE){
        bLen = confRaw->rawDesc[instReadPtr];
        instReadPtr += bLen;
      }
      descPtr = (usbDesc_Interface_t*)(&confRaw->rawDesc[instReadPtr]);
      if (descPtr->bInterfaceClass != USB_CLASSCODE_AUDIO){
        /*Audio Control Interface must have at least 1 Streaming/MIDI Interface*/
        rtnReadBytes = 0;
        break;
      }
      if (descPtr->bInterfaceSubclass == (uint8_t)UAC_STREAMING){
        if (!st_AudioDist[devIdx].clsDist.existAudio){
          st_AudioDist[devIdx].clsDist.existAudio = 1;
        }
        if (st_AudioDriver.parseInterface){
          rtnReadBytes = st_AudioDriver.parseInterface(confRaw, device);
          if (rtnReadBytes == 0){
            /*Streaming Interface ois Invalid*/
            st_AudioDist[devIdx].clsDist.existAudio = 0;
          }
        }        
      } else if (descPtr->bInterfaceSubclass == (uint8_t)UAC_MIDI){
        if (!st_AudioDist[devIdx].clsDist.existMidi){
          if (rtnReadBytes == 0){
            /*MIDI Interface is Invalid*/
            st_AudioDist[devIdx].clsDist.existMidi = 0;
          }
          st_AudioDist[devIdx].clsDist.existMidi = 1;
        }
        if (st_AudioDriver.parseInterface){
          rtnReadBytes = st_MidiDriver.parseInterface(confRaw, device);
        }        
      } else {
        rtnReadBytes = 0;
      }
      break;
    }
    case(UAC_STREAMING):{
      if (!st_AudioDist[devIdx].clsDist.existAudio){
        st_AudioDist[devIdx].clsDist.existAudio = 1;
      }
      if (st_AudioDriver.parseInterface){
        rtnReadBytes = st_AudioDriver.parseInterface(confRaw, device);
        if (rtnReadBytes == 0){
          /*Streaming Interface ois Invalid*/
          st_AudioDist[devIdx].clsDist.existAudio = 0;
        }
      }
      break;
    }
    case(UAC_MIDI):{
      if (!st_AudioDist[devIdx].clsDist.existMidi){
        st_AudioDist[devIdx].clsDist.existMidi = 1;
      }
      if (st_AudioDriver.parseInterface){
        rtnReadBytes = st_MidiDriver.parseInterface(confRaw, device);
        if (rtnReadBytes == 0){
          /*MIDI Interface is Invalid*/
          st_AudioDist[devIdx].clsDist.existMidi = 0;
        }
      }
      break;
    }
  }
  return rtnReadBytes;
}

static uint16_t parseIAD(config_rawdesc_t *confRaw, hcd_DeviceInfo_t* device)
{
  uint16_t rtnReadBytes = 0;
  uint8_t devIdx;
  
  devIdx = device->devAddr;

  if (!st_AudioDist[devIdx].clsDist.existAudio){
    st_AudioDist[devIdx].clsDist.existAudio = 1;
  }
  if (st_AudioDriver.parseIAD){
    rtnReadBytes = st_AudioDriver.parseIAD(confRaw, device);
    if (rtnReadBytes == 0){
      /*Streaming Interface ois Invalid*/
      st_AudioDist[devIdx].clsDist.existAudio = 0;
    }
  }
  return rtnReadBytes;
}

static void initClass(hcd_DeviceInfo_t* device)
{
  uint8_t devIdx = device->devAddr - 1;
  if (st_AudioDist[devIdx].clsDist.existAudio){
    st_AudioDriver.initClass(device);
  }
  if (st_AudioDist[devIdx].clsDist.existMidi){
    st_MidiDriver.initClass(device);
  }
}

static void terminateClass(hcd_DeviceInfo_t* device)
{
  uint8_t devIdx = device->devAddr - 1;
  if (st_AudioDist[devIdx].clsDist.existAudio){
    st_AudioDriver.terinateClass(device);
    st_AudioDist[devIdx].clsDist.existAudio = 0;
  }
  if (st_AudioDist[devIdx].clsDist.existMidi){
    st_MidiDriver.terinateClass(device);
    st_AudioDist[devIdx].clsDist.existMidi = 0;
  }
}

hcd_Status_t HcdAudioMgr_RegisterAudioDriver(hcd_ClassDriver_t* map)
{
  st_AudioDriver.parseInterface = map->parseInterface;
  st_AudioDriver.parseIAD= map->parseIAD;
  st_AudioDriver.initClass = map->initClass;
  st_AudioDriver.terinateClass = map->terinateClass;

  return HCD_OK;
}

hcd_Status_t HcdAudioMgr_RegisterMidiDriver(hcd_ClassDriver_t* map)
{
  st_MidiDriver.parseInterface = map->parseInterface;
  st_MidiDriver.parseIAD= map->parseIAD;
  st_MidiDriver.initClass = map->initClass;
  st_MidiDriver.terinateClass = map->terinateClass;

  return HCD_OK;
}

hcd_Status_t HcdAudioMgr_InitUAC(void)
{
  hcd_ClassDriver_t drv;
  drv.parseInterface = parseInterface;
  drv.parseIAD = parseIAD;
  drv.initClass = initClass;
  drv.terinateClass = terminateClass;
  
  return RegisterClassDriver(&drv, USB_CLASSCODE_AUDIO);
}
