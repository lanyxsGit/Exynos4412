#include "timer.h"
#include "usart.h"
#include "l298n.h"
#include "exti.h"
#include "led.h"

u16 nCount  = 0;
/*=============================================================
 *函数功能：把TIM3初始化为编码器接口模式
 *入口参数：无
 *返回  值：无
 *===========================================================*/
void Encoder_Init_TIM3(void)
{
  RCC->APB1ENR|=1<<1;     //TIM3时钟使能
  RCC->APB2ENR|=1<<2;    //使能PORTA时钟
  GPIOA->CRL&=0X00FFFFFF; //PB6 PB7
  GPIOA->CRL|=0X44000000; //浮空输入

  /* 把定时器初始化为编码器模式 */ 
  TIM3->DIER|=1<<0;   //允许更新中断        
  TIM3->DIER|=1<<6;   //允许触发中断
  MY_NVIC_Init(1,3,TIM3_IRQn,1);

  /* Timer configuration in Encoder mode */ 
  TIM3->PSC = 0x0;//预分频器
  TIM3->ARR = ENCODER_TIM_PERIOD;//设定计数器自动重装值 
  TIM3->CR1 &=~(3<<8);// 选择时钟分频：不分频
  TIM3->CR1 &=~(3<<5);// 选择计数模式:边沿对齐模式
    
  TIM3->CCMR1 |= 1<<0; //CC1S='01' IC1FP1映射到TI1
  TIM3->CCMR1 |= 1<<8; //CC2S='01' IC2FP2映射到TI2
  TIM3->CCER &= ~(1<<1);   //CC1P='0'  IC1FP1不反相，IC1FP1=TI1
  TIM3->CCER &= ~(1<<5);   //CC2P='0'  IC2FP2不反相，IC2FP2=TI2
  TIM3->CCMR1 |= 3<<4; // IC1F='1000' 输入捕获1滤波器
  TIM3->SMCR |= 3<<0;  //SMS='011' 所有的输入均在上升沿和下降沿有效
  TIM3->CNT = 0;
  TIM3->CR1 |= 0x01;
}

/*======================================================
 *函数功能：单位时间读取编码器计数
 *入口参数：定时器
 *返回  值：速度值
 *========================================================*/
int Read_Encoder(void)
{

    int Encoder_TIM;    
    Encoder_TIM= (short)TIM3 -> CNT;
    return Encoder_TIM;
}

/*==========================================
 *函数：TIM3_IRQHandler
 *功能：定时3中断处理函数
 *
 *==========================================*/
void TIM3_IRQHandler(void)
{                         
  if(TIM3->SR&0X0001)//溢出中断
  {
		nCount++;
		if(nCount == 6)
		{
			MotorStop();
			nCount = 0;
			switchFlag = 0;
			LED0 = 1;
		}
  }          
  TIM3->SR&=~(1<<0);//清除中断标志位       
}
