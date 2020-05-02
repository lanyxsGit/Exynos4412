#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include "../include/camera.h"
#include "../include/jpg.h"
#include "../include/pthread.h"
#include "../include/serial.h"
#include "../include/xfmt101.h"
#include "../include/stm32f103.h"


struct jpg_buf_t *jpg;   //保存图片属性

/* 用于创建摄像头互斥锁 */
pthread_mutex_t jpg_mutex;

//unsigned char *LD3320_dev  = "/dev/ttyUSB1";
unsigned char *STM32_dev = "/dev/ttyUSB1";
unsigned char *XFMT101_dev = "/dev/ttyUSB0";
unsigned char STM32sendFlag = 0;
unsigned char CONTROL = 0;
unsigned char RDATA[4];

int mfd = -1; /*stm32 描述符*/
int socketID = -1;
int xfd = -1; /*XFMT101 描述符*/

/*线程：打开摄像头采集数据*/
void *pthread_Camera_capture(void *arg)
{
	jpg = (struct jpg_buf_t *)malloc(sizeof(struct jpg_buf_t));
	if (jpg == NULL)
	{
		printf("jpg malloc error\n");
		pthread_exit(0);
	}

	int fd = -1;
	fd = open_dev("/dev/video0");
	if (fd < 0)
	{
		puts("camera open error.");
		pthread_exit(0);
	}
	puts("**************Open Camera OK****************");
	check_dev_capability(fd);   //查询设备能力
	look_cap_fmt(fd);          //查询并显示所有支持的格式
	set_cap_fmt(fd);          //设置视频的制式和帧格式
	init_mmap(fd);               //初始化内存映射
	start_capturing(fd);
	main_loop(fd, jpg);  //循环采集数据
	stop_capturing(fd);
	uninit_device();
	close(fd);
	free(jpg);
}



/*线程：创建多线程tcp控制并发服务器*/
void *Create_control_server(void *arg)
{
	//定义变量
	int socketID = 0;
	int addrLength = 0;
	struct sockaddr_in addr;
	int newID = 0;
	pthread_t thID;
	//创建socket
	socketID = socket (PF_INET, SOCK_STREAM, 0);
	if (socketID < 0)
	{
		perror("Controlserver:create socket error");
		pthread_exit(0);
	}
	printf("Controlserver:socket success\r\n");

	int on = 1;
	setsockopt( socketID , SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	//绑定自己的地址
	addrLength = sizeof(addr);
	memset(&addr, 0, addrLength);
	addr.sin_family = PF_INET;
	addr.sin_port = htons(SPORT_A9CONTROL);
	addr.sin_addr.s_addr = INADDR_ANY;
	if ( 0 > bind( socketID, (struct sockaddr *)&addr, addrLength))
	{
		perror("Controlserver:bind socket error");
		pthread_exit(0);
	}
	printf("Controlserver:socket bind success\r\n");
	//建立监听
	listen (socketID, 5);
	while (1)
	{
		//接受连接请求--产生一个已经连接的socket
		newID = accept( socketID, (struct sockaddr *)&addr, &addrLength);
		if (newID < 0)
		{
			perror("Controlserver:accept error ");
			pthread_exit(0);
		}
		pthread_create(&thID, NULL, ControlHandler, (void *)&newID);
		pthread_detach(thID);
	}

	//关闭socket
	close(socketID);
	pthread_exit(0);
}

/*线程：处理控制服务器数据*/
void *ControlHandler(void *arg)
{
	int newID = *((int *)arg);
	int nread = 0;
	unsigned char REQUEST[2] = {0};
	memset(REQUEST, 0, sizeof(REQUEST)); //清空请求buf
	pthread_mutex_init(&jpg_mutex, NULL); // 动态初始化图像发送数据互斥锁
	int ret = -1;
	unsigned int total = 0;
	unsigned int piclen = 0;
	unsigned char imgbuf[640 * 480 * 3]  = {0};
	char piclen_char[10];
	printf("Control Client App connected successfully newID is <%d>\n", newID);
	while (1)
	{
		nread = 0;
		piclen = 0;
		memset(imgbuf, 0, sizeof(imgbuf));
		if ((nread = read(newID, REQUEST, sizeof(REQUEST))) > 0)
		{
			printf("REQUEST[0]:%d\n", REQUEST[0]);
			printf("REQUEST[1]:%d\n", REQUEST[1]);
			CONTROL = REQUEST[0];
			STM32sendFlag = 1;  // to stm32 write
			/* 发信号给其他线程处理REQUEST[0] */
			if (REQUEST[1] & (0x1 << EXIT_STATUS))  //请求断开连接，退出
			{
				puts("Control client Exit!");
				close(newID);
				pthread_exit(0);   //退出数据处理线程
			}
			else if (REQUEST[1] & (0x1 << VIDEO))
			{
				/* 发送采集到的照片信息 */
				pthread_mutex_lock(&jpg_mutex); //上锁
				piclen = jpg->len;   //获取到图片长度
				memcpy(imgbuf, jpg->buf, piclen); //获取图片buf
				pthread_mutex_unlock(&jpg_mutex);//解锁

				//发送图像大小
				ret = -1;
				memset(piclen_char, 0, sizeof(piclen_char));
				snprintf(piclen_char, sizeof(piclen_char), "%d", piclen);
				ret = write(newID, piclen_char, sizeof(piclen_char));
				printf("************piclen_char********* %s*********\n", piclen_char);
				if (-1 == ret)
				{
					printf("send piclen error\n");
				}

				/*发送图像数据*/
				pthread_mutex_lock(&jpg_mutex); //上锁
				total = 0;
				while (piclen > total)
				{
					ret = -1;
					//ret = write(newID,imgbuf+total,((piclen>1024)?1024:piclen)); //发送照片
					ret = write(newID, imgbuf + total, piclen - total);
					if (ret < 0)
					{
						perror("send picture error!!!");
						break;
					}
					total += ret;
					printf("total = %d\n", total);
				}
				puts("next picture");
				pthread_mutex_unlock(&jpg_mutex);//解锁
			}

		}
	}
}




/*线程：创建多线程tcp数据并发服务器2*/
void *Create_data_server(void *arg)
{
	//定义变量
	int socketID = 0;
	int addrLength = 0;
	struct sockaddr_in addr;
	int newID = 0;
	pthread_t thID;
	//创建socket
	socketID = socket (PF_INET, SOCK_STREAM, 0);
	if (socketID < 0)
	{
		perror("Dataserver:create socket error");
		pthread_exit(0);
	}
	printf("Dataserver:socket success\r\n");

	int on = 1;
	setsockopt( socketID , SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	//绑定自己的地址
	addrLength = sizeof(addr);
	memset(&addr, 0, addrLength);
	addr.sin_family = PF_INET;
	addr.sin_port = htons(SPORT_A9DATA);
	addr.sin_addr.s_addr = INADDR_ANY;
	if ( 0 > bind( socketID, (struct sockaddr *)&addr, addrLength))
	{
		perror("Dataserver:bind socket error");
		pthread_exit(0);
	}
	printf("Dataserver:socket bind success\r\n");
	//建立监听
	listen (socketID, 5);
	while (1)
	{
		//接受连接请求--产生一个已经连接的socket
		newID = accept( socketID, (struct sockaddr *)&addr, &addrLength);
		if (newID < 0)
		{
			perror("Dataserver:accept error ");
			pthread_exit(0);
		}
		pthread_create(&thID, NULL, DataHandler, (void *)&newID);
		pthread_detach(thID);
	}

	//关闭socket
	close(socketID);
	pthread_exit(0);
}

/*线程：处理数据服务器数据*/
void *DataHandler(void *arg)
{
	int newID = *((int *)arg);
	int nread = 0;
	char request = 0;
	printf("Data Client App connected successfully newID is <%d>\n", newID);
	while (1)
	{
		if ((nread = read(newID, &request, sizeof(request))) > 0)
		{
			if (request == 'Y')
			{
				write(newID, RDATA, sizeof(RDATA));
			}
			else if (request == 'E')
			{
				puts("Data client exit!");
				close(newID);
				pthread_exit(0);
			}
		}
	}
}




/*********线程：（STM32）处理函数****/
void *STM32_WriteData(void *arg)
{
	unsigned char tem = 0xF;
	unsigned char pre_command = 0xFF;
	while (1)
	{
		if (STM32sendFlag)
		{
			STM32sendFlag  = 0;
			if (pre_command != CONTROL )
			{
				pre_command = CONTROL;
				write(mfd, &CONTROL, sizeof(CONTROL));
				printf("-----------CONTROL-------%d\n", CONTROL);
			}
		}
		/*sleep(2);
		  write(mfd,&tem,sizeof(tem));
		  printf("***tem:***%d send success\n",tem);*/
	}
	pthread_exit(0);
}



/*线程：从stm32获取数据 ------>*/
void *STM32_ReadData(void *arg)
{
	int nread = 0;
	mfd = STM32_Init(STM32_dev);  /*初始化STM32串口*/
	if (mfd < 0)
	{
		puts("************stm32f103 Init Error***********");
		pthread_exit(0);
	}
	puts("************STM32F103 Init Successfully**********");

	while (1)
	{
		if ((nread = read(mfd, RDATA, sizeof(RDATA))) > 0)
		{
			printf("-----%d----%d-----%d--\n", RDATA[0], RDATA[1], RDATA[2]);
			//write(socketID,RDATA,sizeof(RDATA));  /*将接受数据直接转发到服务器*/
		}
	}
	close(mfd);
	pthread_exit(0);
}


/*线程：语音播报线程*/
void *pthread_XFMT101(void *arg)
{
	xfd = XFMT101_Init(XFMT101_dev);
	if (xfd < 0)
	{
		puts("*********XFMT101 Init Error***************");
		pthread_exit(0);
	}
	puts("************XFMT101 Init Successfully**********");
	while (1)
	{

		SmokeAlarm_Voicecps(xfd, 190);
		//BurglarAlarm_Voicecps(xfd);
		//OpenDoor_Voicecps(xfd);
		//Leave_Voicecps(xfd);
		//GoHome_Voicecps(xfd);
		puts("send successflly....");
	}
}









/*********线程（LD3320）处理函数***/
// void *LD3320_Funtion(void *arg)
// {
//   	int vfd = OpenDev(LD3320_dev);  /*初始化LD3320串口*/
// 	if(vfd < 0)
// 	{
// 		printf("Open %s Device file failure\n",LD3320_dev);
// 		pthread_exit(0);
// 	}
//   	set_speed(vfd,9600);
//   	if (set_Parity(vfd,8,1,'N') == FALSE)
//  	{
//       	printf("LD3320 Set Parity Error\n");
//       	close(vfd);
//       	pthread_exit(0);
//   	}
//   	puts("open LD3320 success");
//   	memset(TDATA,0,sizeof(TDATA));
//   	int nread;
//   	unsigned char buff[SIZE];
//   	fd_set rfds;
//   	while (1)
//   	{
//   	  	memset(buff,0,sizeof(buff));
//   	  	printf("wait...\n");
//   	  	FD_ZERO(&rfds);
//   	  	FD_SET(vfd, &rfds);
//   	  	if (select(1+vfd, &rfds, NULL, NULL, NULL) > 0)
//   	  	{
//   	    	if (FD_ISSET(vfd, &rfds))
//   	    	{
//   	      		nread=read(vfd, buff, SIZE);
//   	      		printf("readlength = %d\n", nread);
//   	      		printf("%s\n", buff);
//   	      		if(strcmp(buff,"OpenLed") == 0)
//   	      		{
//   	      		  	TDATA[0] |= 0x1<<0;
//   	      		  	STM32sendFlag = 1;
//   	      		}
//   	      		else if(strcmp(buff,"CloseLed") == 0)
//   	      		{
//   	      		  	TDATA[0] &= ~(0x1<<0);
//   	      		  	STM32sendFlag = 1;
//   	      		}
//   	      		else if(strcmp(buff,"OpenFan") == 0)
//   	      		{
//   	      		  	TDATA[0] |= 0x1<<1;
//           			STM32sendFlag = 1;
//         		}
//         		else if(strcmp(buff,"CloseFan") == 0)
//         		{
//         		  	TDATA[0] &= ~(0x1<<1);
//         		  	STM32sendFlag = 1;
//         		}
//         		else if(strcmp(buff,"Openwin") == 0)
//         		{
//         		  	TDATA[0] |= 0x1<<2;
//         		  	STM32sendFlag = 1;
//         		}
//         		else if(strcmp(buff,"Closewin") == 0)
//         		{
//         		  	TDATA[0] &= ~(0x1<<2);
//         		  	STM32sendFlag = 1;
//         		}
//         		else if(strcmp(buff,"CloseAll") == 0)
//         		{
//         		  	TDATA[0] = 0x0;
//         		  	STM32sendFlag = 1;
//         		}
//         		else if(strcmp(buff,"OpenAll") == 0)
//         		{
//         		  	TDATA[0] = 0x07;
//         		  	STM32sendFlag = 1;
//         		}
//         		printf("%s send success\n",buff);
//       		}
//     	}
//   	}

//   	close(vfd);
//   	pthread_exit(0);

// }
