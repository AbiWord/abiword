/* AbiSource Program Utilities
 * Copyright (C) 2001 Tomas Frydrych
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

/*
 !!! PLEASE, USE ONLY C-STYLE COMMENTS IN THIS FILE !!!
*/

#ifndef UT_ENDIAN_H
#define UT_ENDIAN_H

/* autoconf checks */
#ifdef CHECKED_ENDIANNESS
#  if defined(WORDS_BIGENDIAN)
#    define UT_BIG_ENDIAN
#  else
#    define UT_LITTLE_ENDIAN
#  endif
#else

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
	#if defined(__LITTLEENDIAN__)
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
#elif defined(__MACH__) && defined(__APPLE__)
	#if defined(__BIG_ENDIAN__)
		#define UT_BIG_ENDIAN
	#else
		#define UT_LITTLE_ENDIAN
	#endif
#elif defined(__sun)
        #include <sys/isa_defs.h>
        #if defined(_BIG_ENDIAN)
                #define UT_BIG_ENDIAN
        #elif defined(_LITTLE_ENDIAN)
                #define UT_LITTLE_ENDIAN
        #endif
#elif defined(__NetBSD__) || defined(__OpenBSD__)
	#include <machine/endian.h>
	#if BYTE_ORDER == LITTLE_ENDIAN         
		#define UT_LITTLE_ENDIAN
	#elif BYTE_ORDER == BIG_ENDIAN
		#define UT_BIG_ENDIAN
	#endif
#elif defined(_AIX)
        #include <sys/machine.h>
        #if BYTE_ORDER == LITTLE_ENDIAN
                #define UT_LITTLE_ENDIAN
        #elif BYTE_ORDER == BIG_ENDIAN
                #define UT_BIG_ENDIAN
        #endif
#elif defined(__osf__)
        #include <machine/endian.h>
        #if BYTE_ORDER == LITTLE_ENDIAN
                #define UT_LITTLE_ENDIAN
        #elif BYTE_ORDER == BIG_ENDIAN
                #define UT_BIG_ENDIAN
        #endif
#elif defined(_WIN32)
	/* We should probably do some check as the WinAPI could be on other computers */
	#if !defined(UT_LITTLE_ENDIAN) && !defined(UT_BIG_ENDIAN)
		#define UT_LITTLE_ENDIAN
	#endif		
#else /* this is for Linux */
	#include <endian.h>
	#if __BYTE_ORDER == __LITTLE_ENDIAN		
		#define UT_LITTLE_ENDIAN
	#else
		#define UT_BIG_ENDIAN
	#endif
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

