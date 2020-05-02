#ifndef __SERIAL_H
#define __SERIAL_H

#define FALSE  -1
#define TRUE   0
/*打开设备文件*/
int OpenDev(char *Dev);
/*设置波特率*/
void set_speed(int fd, int speed);
/*设置校验位*/
int set_Parity(int fd,int databits,int stopbits,int parity);


#endif