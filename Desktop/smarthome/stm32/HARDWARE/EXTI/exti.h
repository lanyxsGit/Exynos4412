#ifndef __EXTI_H
#define __EXTI_H

#include "sys.h"

extern u8 switchFlag;
#define KEY	PAin(0) //PA0

void EXTIX_Init(void);   /*外部中断程序初始化*/



#endif


