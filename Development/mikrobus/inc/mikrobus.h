
#ifndef __MIKROBUS_H__
#define __MIKROBUS_H__

#include "mb_captouch2.h"
#include "mb_gyro2.h"
#include "mb_pin.h"

enum{
	eDMA_INST0 = 0,
	eDMA_INST1
};

enum{
	eDMA_CH0 = 0,
	eDMA_CH1,
	eDMA_CH2,
	eDMA_CH3,
	eDMA_CH4,
	eDMA_CH5,
	eDMA_CH6,
	eDMA_CH7,
	eDMA_CH8,
	eDMA_CH9,
	eDMA_CH10,
	eDMA_CH11,
	eDMA_CH12,
	eDMA_CH13,
	eDMA_CH14,
	eDMA_CH15,
};

void InitMikroBUS(void);


#endif /*__MIKROBUS_H__*/
