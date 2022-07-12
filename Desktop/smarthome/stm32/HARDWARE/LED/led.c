#include "led.h"


/*==========================================
 *函数：LED_Init
 *功能：LED初始化
 *
 *==========================================*/
void LED_Init(void)
{

	RCC->APB2ENR |= 0x1 << 4; //使能PORTC时钟
	GPIOC->CRH &= 0XFF0FFFFF; //清除第13位
	GPIOC->CRH |= 0X00300000; //设置PC13为推挽输出
}

