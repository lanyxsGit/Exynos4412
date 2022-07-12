
/*==================================================
 * Name: 自动削皮器
 * Timer：2020/6/10
 *
 *
 *=================================================*/

#include "sys.h"
#include "delay.h"
#include "l298n.h"
#include "exti.h"
#include "led.h"
#include "usart.h"
#include "timer.h"

int main()
{
	SystemInit(); 	      /*系统时钟设置 (72M)*/
	uart_init(72,115200);  /*初始化uart串口*/
	Encoder_Init_TIM3();
	delay_init(72);       /*初始化延时模块*/
	L298N_Init();         /*L298N驱动模块初始化*/ 
	EXTIX_Init();         /*外部中断初始化*/
	LED_Init();           /*LCD灯初始化*/
	LED0 = 1;
	MotorStop();         /*默认电机处于停止状态*/
	while(1)
	{

	}
	
}

