#include "exti.h"
#include "sys.h"
#include "delay.h"
#include "led.h"
#include "l298n.h"

u8 switchFlag = 0;

/*==========================================
 *������EXTI0_IRQHandler
 *���ܣ��ⲿ�ж�0 �����ӳ���
 *
 *==========================================*/
void EXTI0_IRQHandler(void)
{
	delay_ms(10);     /*����*/		
	if(KEY == 1)    /*����*/
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
	EXTI->PR = 0x1<<0;    /*���LINE0�ϵ��жϱ�־λ*/
}


/*==========================================
 *������EXTIX_Init
 *���ܣ��ⲿ�жϳ�ʼ������
 *
 *==========================================*/
void EXTIX_Init(void)
{
	RCC->APB2ENR|=1<<2;     //ʹ��PORTAʱ��
	GPIOA->CRL&=0XFF0FFFF0;	  
	GPIOA->CRL|=0X00300008;//PA0���ó����룬Ĭ������	 ����PA5 ���
	GPIOA->ODR|= 0x1 << 5;
	
	Ex_NVIC_Config(GPIO_A,0,FTIR);   /*�½��ش���*/
	MY_NVIC_Init(2,3,EXTI0_IRQn,2);
}



