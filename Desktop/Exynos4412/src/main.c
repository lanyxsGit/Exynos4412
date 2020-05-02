#include <stdio.h>
#include <pthread.h>
#include <fcntl.h>
#include <string.h>
#include "../include/jpg.h"
#include "../include/pthread.h"
int main(int argc, char const *argv[])
{
	/*创建线程*/
	pthread_t thID_STM32Write, thID_STM32Read, thID_Client, thID_video, thID_control_server, thID_data_server, thID_XFMT101;
	//pthread_t thID_LD3320;
	int ret = -1;
	/*ret = pthread_create(&thID_LD3320, NULL, LD3320_Funtion, NULL);
	if(ret < 0)
	{
		printf("LD3320 pthread create error!\n");
		return -1;
	}*/
	ret = -1;
	/*线程：STM32读数据*/
	ret = pthread_create(&thID_STM32Read, NULL, STM32_ReadData, NULL);
	if (ret < 0)
	{
		printf("STM32Read pthread create error!\n");
		return -1;
	}
	ret = -1;
	/*线程：STM32写数据*/
	ret = pthread_create(&thID_STM32Write, NULL, STM32_WriteData, NULL);
	if (ret < 0)
	{
		printf("STM32Write pthread create error!\n");
		return -1;
	}
	ret = -1;
	/*线程：摄像头采集数据*/
	ret = pthread_create(&thID_video, NULL, pthread_Camera_capture, NULL);
	if (ret < 0)
	{
		printf("Camera Capture pthread create error!\n");
		return -1;
	}
	ret = -1;
	/*线程：创建tcp控制并发服务器*/
	ret = pthread_create(&thID_control_server, NULL, Create_control_server, NULL);
	if (ret < 0)
	{
		printf("Control_server pthread create error!\n");
		return -1;
	}
	ret = -1;
	/*线程：创建tcp数据并发服务器*/
	ret = pthread_create(&thID_data_server, NULL, Create_data_server, NULL);
	if (ret < 0)
	{
		printf("Data_server pthread create error!\n");
		return -1;
	}

	ret = -1;
	/*线程：创建语音播报线程*/
	ret = pthread_create(&thID_XFMT101, NULL, pthread_XFMT101, NULL);
	if (ret < 0)
	{
		printf("XFMT101 pthread create error!\n");
		return -1;
	}

	pthread_detach(thID_STM32Write);
	pthread_detach(thID_STM32Read);
	pthread_detach(thID_video);
	pthread_detach(thID_control_server);
	pthread_detach(thID_data_server);
	pthread_detach(thID_XFMT101);
	while (1);
	return 0;
}
