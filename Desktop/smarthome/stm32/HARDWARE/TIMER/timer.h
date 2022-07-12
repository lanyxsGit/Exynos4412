#ifndef __TIMER_H
#define __TIMER_H
#include "sys.h"
#define ENCODER_TIM_PERIOD 63500

extern u16 nCount;
void Encoder_Init_TIM3(void);
int Read_Encoder(void);

#endif
