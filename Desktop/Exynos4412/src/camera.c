
#include "../include/jpg.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h> /* for videodev2.h */
#include "../include/camera.h"


/*用于存储映射指针(通过指针加偏移的方式来存储四个帧)*/
struct vbuffer *buffers = NULL;

/*用于给申请的缓冲区初始化用的循环变量*/
unsigned int n_buffers = 0;

/*用于存储yuv图片*/
u8 yuvbuf[640 * 480 * 2];
/*用于存储rgb图片*/
u8 rgbbuf[640 * 480 * 3];
/*用于存储bmp图片*/
u8 bmpbuf[54 + 640 * 480 * 3];
/*用于存储jpg图片*/
u8 *jpgbuf = NULL;

int open_dev(const char *dev_name)
{
	if (NULL == dev_name)
	{
		printf("The %s device is no exist.\n", dev_name);
		return -1;
	}
	int fd = open(dev_name, O_RDWR | O_NONBLOCK);
	if (fd > 0)
	{
		return fd;
	}
	else
	{
		return -1;
	}
}

int check_dev_capability(int fd)
{
	if (fd < 0)
	{
		puts("fd < 0");
		return -1;
	}
	struct v4l2_capability cap;
	int ret = ioctl(fd, VIDIOC_QUERYCAP, &cap);
	if (ret == 0)
	{
		if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)
		{
			puts("This camera can capture picture.");
		}
	}
	return 0;
}

int look_cap_fmt(int fd)
{
	if (fd < 0)
	{
		puts("fd < 0");
		return -1;
	}
	struct v4l2_fmtdesc fmt;
	memset(&fmt, 0, sizeof(fmt));
	fmt.index = 0;
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	int ret = -1;
	while ((ret = ioctl(fd, VIDIOC_ENUM_FMT, &fmt)) == 0)
	{
		fmt.index++;
		printf("{pixelformat = %c%c%c%c description = %s}\n",
		       fmt.pixelformat & 0xFF, (fmt.pixelformat >> 8) & 0xFF,
		       (fmt.pixelformat >> 16) & 0xFF,
		       (fmt.pixelformat >> 24) & 0xFF,
		       fmt.description);
	}
	return 0;
}

int set_cap_fmt(int fd)
{
	if (fd < 0)
	{
		puts("fd < 0");
		return -1;
	}
	struct v4l2_format capfmt;
	memset(&capfmt, 0, sizeof(struct v4l2_format));
	capfmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	capfmt.fmt.pix.width = 640;
	capfmt.fmt.pix.height = 480;
	capfmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
	capfmt.fmt.pix.field = V4L2_FIELD_NONE;

	int ret = ioctl(fd, VIDIOC_S_FMT, &capfmt);
	if (ret == 0)
	{
		puts("set capture format ok.");
	}
	else
	{
		puts("set capture format failed.");
	}
	return 0;
}

int init_mmap(int fd)
{
	struct v4l2_requestbuffers req;
	memset(&req, 0, sizeof(struct v4l2_requestbuffers));
	req.count  = 4;
	req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;

	int ret = ioctl(fd, VIDIOC_REQBUFS, &req);
	if (ret == 0)
	{
		puts("requestbuffer success.");
	}
	else
	{
		perror("reqbuf");
		return -1;
	}

#ifdef __DEBUG__
	printf("req.count = %d\n", req.count);
#endif
	/*如果ioctl后的count小于2，说明ioctl并没有达到缓冲区的要求*/
	if (req.count < 2)
	{
		puts("req failed.");
		return -1;
	}

	buffers = calloc(req.count, sizeof(*buffers));
	if (NULL == buffers)
	{
		puts("calloc framebuffer error.");
		return -1;
	}

	for (n_buffers = 0; n_buffers < req.count; n_buffers++)
	{
		struct v4l2_buffer buf;
		memset(&buf, 0, sizeof(struct v4l2_buffer));

		buf.type 	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory	= V4L2_MEMORY_MMAP;
		buf.index	= n_buffers;

		ret = -1;
		/*用于查询buf的状态*/
		ret = ioctl(fd, VIDIOC_QUERYBUF, &buf);
		if (ret != 0)
		{
			puts("VIDIOC_QUERYBUF error");
			return -1;
		}

		buffers[n_buffers].length = buf.length;
		buffers[n_buffers].start  = mmap(NULL /*start anywhere*/,
		                                 buf.length,
		                                 PROT_READ | PROT_WRITE /*required*/,
		                                 MAP_SHARED /*recommended*/,
		                                 fd, buf.m.offset);
		if (MAP_FAILED == buffers[n_buffers].start)
		{
			puts("mmap error.");
			close(fd);
			exit(-1);
		}
	}
	puts("querybuf and mmap success.");
	return 0;
}

int start_capturing(int fd)
{
	int i = 0;
	enum v4l2_buf_type type;
	for (i = 0; i < n_buffers; i++)
	{
		struct v4l2_buffer buf;
		memset(&buf, 0, sizeof(struct v4l2_buffer));

		buf.type	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory	= V4L2_MEMORY_MMAP;
		buf.index	= i;

		if ( -1 == ioctl(fd, VIDIOC_QBUF, &buf))
		{
			puts("VIDIOC_QBUF error");
			return -1;
		}

		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (-1 == ioctl(fd, VIDIOC_STREAMON, &type))
		{
			puts("VIDIOC_STREAMON error");
			return -1;
		}
		puts("VIDIOC_STREAMON success.");
		return 0;
	}
}

int main_loop(int fd, struct jpg_buf_t *jpg)
{
	if (fd < 0)
	{
		puts("fd < 0");
		return -1;
	}
	for (;;)
	{
		fd_set fds;
		struct timeval tv;
		int r;

		FD_ZERO(&fds);
		FD_SET(fd, &fds);

		/*Timeout*/
		tv.tv_sec = 2;
		tv.tv_usec = 0;

		r = select(fd + 1, &fds, NULL, NULL, &tv);

		if (-1 == r)
		{
			if (EINTR == errno)
			{
				continue;
			}
			exit(-1);
		}

		if (0 == r)
		{
			puts("select timeout.");
			exit(-1);
		}

		if (read_frame(fd, jpg))
		{
			break;
		}
	}
}

int read_frame(int fd, struct jpg_buf_t *jpg)
{
	if (fd < 0)
	{
		puts("fd < 0");
		return -1;
	}

	struct v4l2_buffer buf;
	int i = 0;

	memset(&buf, 0, sizeof(struct v4l2_buffer));
	buf.type	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory 	= V4L2_MEMORY_MMAP;

	if (-1 == ioctl(fd, VIDIOC_DQBUF, &buf))
	{
		switch (errno)
		{
		case EAGAIN:
			return 0;
		case EIO:
			puts("EIO error.");
			break;
		default:
			exit(-1);
		}
	}

	if (buf.index < n_buffers);
	//process_image(buffers[buf.index].start);
	save_jpgvbuff(buffers[buf.index].start, buf.bytesused, jpg);

	if (-1 == ioctl(fd, VIDIOC_QBUF, &buf))
	{
		puts("VIDIOC_QBUF IN READ_FRAME error.");
		return -1;
	}
	return 0;
}

void save_jpgvbuff(const void *p, unsigned int len, struct jpg_buf_t *jpg)
{
	unsigned char * pbuff  = (unsigned char *)p;
	jpg->len = len;
	memcpy(jpg->buf, pbuff, jpg->len);
//	printf("len is %d\n", len);
}

// void process_image(const void *p)
// {
// 	if(NULL == p)
// 	{
// 		puts("process_image: p is null.");
// 		return;
// 	}

// 	char *pbuf = (char *)p;
// 	memcpy(yuvbuf,pbuf,640*480*2);
// 	init_conv(640,480);
// 	yuvtorgb24(rgbbuf,yuvbuf,640,480);
// 	getbmp(bmpbuf,640,480);

// 	unsigned long int size_jpg = 0;
// 	size_jpg = rgbtojpeg(&jpgbuf,rgbbuf,640,480);
// 	uninit_conv();
// 	int file = -1;
// 	file = open("1.jpg", O_RDWR | O_CREAT, 0666);
// 	if(file > 0)
// 	{
// 		write(file,jpgbuf,size_jpg);
// 		close(file);
// 		//exit(0);
// 	}
// }

int stop_capturing(int fd)
{
	if (fd < 0)
	{
		puts("fd < 0");
		return -1;
	}
	enum v4l2_buf_type type;
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == ioctl(fd, VIDIOC_STREAMOFF, &type))
	{
		puts("VIDIOC_STREAMOFF error");
		return -1;
	}
	puts("VIDIOC_STREAMOFF success.");
	return 0;
}

void uninit_device(void)
{
	int i = 0;
	for (i = 0; i < n_buffers; i++)
	{
		if (-1 == munmap(buffers[i].start, buffers[i].length))
		{
			puts("munmap error.");
			exit(-1);
		}
	}
}
