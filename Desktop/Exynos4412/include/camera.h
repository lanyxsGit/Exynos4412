#ifndef __CAMERA_H__
#define __CAMERA_H__
struct jpg_buf_t;

/*用于存储摄像头采集出的图片信息。用于帧缓冲*/
struct vbuffer
{
	void *start;
	size_t length;
};

/*
 *函数名：open_dev
 *函数功能：打开摄像头文件
 *函数返回值：int 成功返回一个大于0的文件描述符，失败返回-1
 *函数参数：const char *dev_name
 */
int open_dev(const char *dev_name);

/*
 *函数名：check_dev_capability
 *函数功能：查看设备功能
 *函数返回值：int 成功返回0，失败返回-1
 *函数参数：int fd 摄像头设备文件的文件描述符
 */
int check_dev_capability(int fd);

/*
 *函数名：look_cap_fmt
 *函数功能：查看设备采集视频的格式
 *函数返回值：int 成功返回0，失败返回-1
 *函数参数：int fd 摄像头设备文件的文件描述符
 */
int look_cap_fmt(int fd);

/*
 *函数名：set_cap_fmt
 *函数功能：设置视频采集格式
 *函数返回值：int 成功返回0，失败返回-1
 *函数参数：int fd 摄像头设备文件的文件描述符
 */
int set_cap_fmt(int fd);

/*
 *函数名：init_mmap
 *函数功能：申请帧缓冲并映射缓冲区到用户空间
 *函数返回值：int 成功返回0，失败返回-1
 *函数参数：int fd：摄像头设备文件的文件描述符
 */

int init_mmap(int fd);

/*
 *函数名：start_capturing(int fd);
 *函数功能：帧缓冲入队，并开启视频采集流
 *函数返回值：int 成功返回0，失败返回-1
 *函数参数：int fd 摄像头设备文件的文件描述符
 */
int start_capturing(int fd);

/*
 *函数名：main_loop
 *函数功能：循环采集，出队并做图像处理
 *函数返回值：int 成功返回0，失败返回-1
 *函数参数：int fd 摄像头设备文件的文件描述符
 */
int main_loop(int fd, struct jpg_buf_t *jpg);

/*
 *函数名：read_frame
 *函数功能：读取缓冲区的内容
 *函数返回值：int 成功返回0，失败返回-1
 *函数参数：int fd 摄像头设备文件的文件描述符
 */
int read_frame(int fd, struct jpg_buf_t *jpg);

/*
 *函数名：process_image
 *函数功能：处理采集出来的图片
 *函数返回值：void
 *函数参数：const void *p
 */
//void process_image(const void *p);

/*保存jpg缓存*/

void save_jpgvbuff(const void *p, unsigned int len, struct jpg_buf_t *jpg);

/*
 *函数名：stop_capturing
 *函数功能：停止视频采集
 *函数返回值：int 成功返回0，失败返回-1
 *函数参数：int fd 摄像头设备文件的文件描述符
 */
int stop_capturing(int fd);

/*
 *函数名：uninit_device
 *函数功能：取消缓冲区映射
 *函数返回值：void
 *函数参数：void
 */
void uninit_device(void);

#endif
