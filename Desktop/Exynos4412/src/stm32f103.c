#include <stdio.h>
#include <string.h>
#include "../include/serial.h"
#include "../include/stm32f103.h"



/******stm32f103初始化 ******
*return：mfd
*****************************/
int STM32_Init(char *STM32_dev)
{
	int mfd = OpenDev(STM32_dev);  /*初始化XFMT101串口*/
	if (mfd < 0)
	{
		printf("Open %s Device file failure\n", STM32_dev);
		return -1;
	}
	set_speed(mfd, 115200);
	if (set_Parity(mfd, 8, 1, 'N') == FALSE)
	{
		printf("STM32F103 Set Parity Error\n");
		close(mfd);
		return -1;
	}
	return mfd;

}
