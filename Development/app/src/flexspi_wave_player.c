/**
* @brief   FRDM-MCXN947 board FLEXSPI PCM Player App
* @author  masa
* @version 1.00
*/

#include "flexspi_wave_player.h"

wave_Head_t st_Head = {0};
uint32_t st_BaseAddress = 0;
wave_Player_Table_t st_Player;

static uint32_t minU32(uint32_t a, uint32_t b)
{
  if (a < b) {
    return a;
  } else {
    return b;
  }
}

static void* readFlash(uint32_t offset, void *buf, size_t len)
{
  uint32_t ahbAddr;
  if (st_BaseAddress == 0){
    return (void*)-1;
  }
  ahbAddr = offset + st_BaseAddress;
  return memcpy(buf, (const void*)(ahbAddr), len);
}

static void wavePlay(uint8_t devIndex, uint32_t* buf, uint16_t nextTotalLen)
{
	uint32_t sentLen, curPointer, rsdl, loopRoom, rdLen;
	sentLen = 0;
	
  if (devIndex != st_Player.deviceIndex){
    return;
	}

  while (sentLen < nextTotalLen){
    curPointer = st_Player.curPointer;
    rsdl = nextTotalLen - sentLen;

    loopRoom = st_Player.totalSize - curPointer;
    rdLen = minU32(rsdl, loopRoom);

    readFlash(curPointer, &buf[sentLen], rdLen);

    sentLen += rdLen;
    st_Player.curPointer += rdLen;
    if (st_Player.curPointer == st_Player.totalSize){
      st_Player.curPointer = st_Player.startPoint;
    }
  }
}

static void deviceNotify(uint8_t deviceIndex, uint8_t dir)
{
  if ((dir == AUDIO_DIR_PLAY) && (st_Player.deviceIndex == 0)){
    st_Player.deviceIndex = deviceIndex;
    UsbhAudio_SetPlayCallback(deviceIndex, wavePlay);
    UsbhAudio_StartStreaming(deviceIndex, dir);
  }
}


void InitWavePlayer(uint32_t flashBase)
{
  st_BaseAddress = flashBase;
  readFlash(0, &st_Head, sizeof(wave_Head_t));
  if(strncmp((const char*)&st_Head.riff_ckID[0], "RIFF", 4) != 0){
    return;
  }
  st_Player.curPointer = 0;
  st_Player.totalSize = st_Head.riff_cksize - sizeof(wave_Head_t);
  st_Player.startPoint = sizeof(wave_Head_t);

  UsbhAudio_SetReadyNotify(deviceNotify);
}
