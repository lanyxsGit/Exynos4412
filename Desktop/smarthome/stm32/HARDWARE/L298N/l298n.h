#ifndef __L298N_H
#define __L298N_H

#include "sys.h"




//PA0 - PA1 端口定义
#define IN1 PAout(1)	// PA1
#define IN2 PAout(2)	// PA2
#define EA PAout(3)   //PA3

void L298N_Init(void);	//初始化		
void MotorRun(void);
void MotorStop(void);


#endif
