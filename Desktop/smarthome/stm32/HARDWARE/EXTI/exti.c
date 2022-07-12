#include "exti.h"
#include "sys.h"
#include "delay.h"
#include "led.h"
#include "l298n.h"

u8 switchFlag = 0;

/*==========================================
 *函数：EXTI0_IRQHandler
 *功能：外部中断0 服务子程序
 *
 *==========================================*/
void EXTI0_IRQHandler(void)
{
	delay_ms(10);     /*消抖*/		
	if(KEY == 1)    /*按键*/
	{
		if(switchFlag == 0)
		{
			MotorRun();
			switchFlag = 1;
			LED0 = 0;
		}
		else if(switchFlag == 1)
		{
			MotorStop();
			switchFlag = 0;
			LED0 = 1;
		}
	}
	EXTI->PR = 0x1<<0;    /*清楚LINE0上的中断标志位*/
}


/*==========================================
 *函数：EXTIX_Init
 *功能：外部中断初始化程序
 *
 *==========================================*/
void EXTIX_Init(void)
{
	RCC->APB2ENR|=1<<2;     //使能PORTA时钟
	GPIOA->CRL&=0XFF0FFFF0;	  
	GPIOA->CRL|=0X00300008;//PA0设置成输入，默认上拉	 设置PA5 输出
	GPIOA->ODR|= 0x1 << 5;
	
	Ex_NVIC_Config(GPIO_A,0,FTIR);   /*下降沿触发*/
	MY_NVIC_Init(2,3,EXTI0_IRQn,2);
}



