
/*==================================================
 * Name: �Զ���Ƥ��
 * Timer��2020/6/10
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
	SystemInit(); 	      /*ϵͳʱ������ (72M)*/
	uart_init(72,115200);  /*��ʼ��uart����*/
	Encoder_Init_TIM3();
	delay_init(72);       /*��ʼ����ʱģ��*/
	L298N_Init();         /*L298N����ģ���ʼ��*/ 
	EXTIX_Init();         /*�ⲿ�жϳ�ʼ��*/
	LED_Init();           /*LCD�Ƴ�ʼ��*/
	LED0 = 1;
	MotorStop();         /*Ĭ�ϵ������ֹͣ״̬*/
	while(1)
	{

	}
	
}

