
#ifndef UT_ENDIAN_H
#define UT_ENDIAN_H

#ifdef __hpux

#elif __FreeBSD__
	#include <machine/endian.h>
	#if __BYTE_ORDER == __LITTLE_ENDIAN		
		#define UT_LITTLE_ENDIAN
	#else
		#define UT_BIG_ENDIAN
	#endif
#else
	#include <endian.h>
	#if __BYTE_ORDER == __LITTLE_ENDIAN		
		#define UT_LITTLE_ENDIAN
	#else
		#define UT_BIG_ENDIAN
	#endif
#endif


//XP macros

// convert single UCS character
// x,y are pointers to UT_UCSChar
// we will use a temporary variable, so that x and y
// can be the same
#ifdef UT_LITTLE_ENDIAN		
#define LE2BE16(x,y)                                  \
char * lb1;                                           \
UT_UCSChar tucs;                                      \
tucs = * ((UT_UCSChar *)(x)); lb1 = (char*) (&tucs);  \
*((char*)(y)) = *(lb1+1); *(((char*)(y)+1)) = *lb1;
#else
#define LE2BE16(x,y)
#endif

#endif //UT_ENDIAN_H
