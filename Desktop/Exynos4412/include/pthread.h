#ifndef __PTHREAD_H
#define __PTHREAD_H

#define SIZE 100
#define SPORT_A9CONTROL 10010
#define SPORT_A9DATA 10086

//REQUEST[0]
#define LED 0   //灯
#define FAN 1   //风扇
#define CUR 2   //窗帘
#define AIR 3   //空调
#define DOOR 4  //门
#define LHOME 5 //离家模式

//REQUEST[1]
#define VIDEO 0  //视频监控
#define EXIT_STATUS  1 //退出状态

/*初始化socket*/
int init_socket(unsigned int port);

/**************** stm32 *******************/
/*线程：LD3320*/
//void *LD3320_Funtion(void *arg);
/*线程：STM32 write data*/
void *STM32_WriteData(void *arg);

/*线程：从stm32获取数据 ------> */
void *STM32_ReadData(void *arg);

/*************** server *****************/
/*线程：创建多线程tcp控制并发服务器*/
void *Create_control_server(void *arg);

/*线程：处理控制服务器数据*/
void *ControlHandler(void *arg);

/*线程：创建多线程tcp数据并发服务器_2*/
void *Create_data_server(void *arg);

/*线程：处理数据服务器数据*/
void *DataHandler(void *arg);

/************** video_capture **************/

/*线程：打开摄像头 发送数据*/
void *pthread_Camera_capture(void *arg);



/*************XFMT101 语音播报***************/
/*线程：语音播报线程*/
void *pthread_XFMT101(void *arg);





#endif
