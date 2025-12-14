/**
* @brief   MCXN947V USB Host Controller Audio Class 2.0 Driver
* @author  masa
* @version 1.00 
*/

#include "hcd_class_audio20.h"

static uint8_t st_NumOfDevice;
static hcd_UAC20_Info_t st_Info[NUM_OF_MAX_AUDIO_DEVICE];

static uint16_t parseControlInterface(config_rawdesc_t *confRaw, hcd_Audio_Endpoint_Info_t* intf, hcd_DeviceInfo_t* device)
{

}

