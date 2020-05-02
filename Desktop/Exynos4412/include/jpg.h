#ifndef __JPG_H
#define __JPG_H

#define MAX_JPG_SIZE	(640 * 480 * 3)
typedef unsigned char u8;
struct jpg_buf_t {
	unsigned char buf[MAX_JPG_SIZE];
	unsigned int len;
};

#endif
