#ifndef _BASICS_H
#define _BASICS_H

#include <stdio.h>		/* for NULL */
#include <stdlib.h>		/* for free */

#define true 1
#define false 0

#define forever for (;;)

typedef int		int32;
typedef short		int16;
typedef signed char	int8;
typedef unsigned int	uint32;
typedef unsigned short	uint16;
typedef unsigned char	uint8;

typedef unsigned short unichar_t;

extern void *myalloc(long size);
extern void *mycalloc(int cnt, long size);
extern void *myrealloc(void *,long size);
extern void myfree(void *);
#endif
