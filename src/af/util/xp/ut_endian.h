/*
 !!! PLEASE, USE ONLY C-STYLE COMMENTS IN THIS FILE !!!
*/

#ifndef UT_ENDIAN_H
#define UT_ENDIAN_H

#if defined(__hpux)
	/* #define UT_BIG_ENDIAN */
	#include <machine/param.h>
	#if defined(_BIG_ENDIAN)
		#define UT_BIG_ENDIAN
	#endif
	#if defined(_LITTLE_ENDIAN)
		#define UT_LITTLE_ENDIAN
	#endif
#elif defined(__QNXNTO__)
	#include <sys/platform.h>
	#if defined(__LITTLENDIAN__)
		#define UT_LITTLE_ENDIAN
	#else
		#define UT_BIG_ENDIAN
	#endif
#elif defined(__FreeBSD__)
	#include <machine/endian.h>
	#if __BYTE_ORDER == __LITTLE_ENDIAN		
		#define UT_LITTLE_ENDIAN
	#else
		#define UT_BIG_ENDIAN
	#endif
#elif defined(__sgi)
	#include <sys/endian.h>
	#if BYTE_ORDER == LITTLE_ENDIAN
		#define UT_LITTLE_ENDIAN
	#else
		#define UT_BIG_ENDIAN
	#endif
#else /* this is for Linux */
	#include <endian.h>
	#if __BYTE_ORDER == __LITTLE_ENDIAN		
		#define UT_LITTLE_ENDIAN
	#else
		#define UT_BIG_ENDIAN
	#endif
#endif


/*  Make sure we got a definition for our platform:  */
#if defined(UT_BIG_ENDIAN)
#elif defined(UT_LITTLE_ENDIAN)
#else
	#error Must define UT_BIG_ENDIAN or UT_LITTLE_ENDIAN in ut_endian.h
#endif

/*XP macros

 convert single UCS character
 x,y are pointers to UT_UCSChar
 we will use a temporary variable, so that x and y
 can be the same
*/

#ifdef UT_LITTLE_ENDIAN		
#define LE2BE16(x,y)                                  \
char * lb1;                                           \
UT_UCSChar tucs;                                      \
tucs = * ((UT_UCSChar *)(x)); lb1 = (char*) (&tucs);  \
*((char*)(y)) = *(lb1+1); *(((char*)(y)+1)) = *lb1;
#else
#define LE2BE16(x,y)
#endif

#endif /*UT_ENDIAN_H*/

