/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
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
 


#ifndef UT_TYPES_H
#define UT_TYPES_H

#ifdef NULL
#undef NULL
#endif
#define NULL 0

#ifdef __cplusplus
#define UT_BEGIN_EXTERN_C		extern "C" {
#define UT_END_EXTERN_C			}
#else
#define UT_BEGIN_EXTERN_C
#define UT_END_EXTERN_C
#endif

typedef		unsigned char		UT_Byte;
typedef		unsigned short		UT_UCSChar;	/* Unicode */

typedef		unsigned short		UT_uint16;
typedef		unsigned long		UT_uint32;
typedef		signed long			UT_sint32;

/*
	TODO we currently use plain old C 'int' all over the place.
	For many applications, this is inappropriate, and we should change
	them to UT_sint32.  Also, there are places where we are
	using it as a bool, and there are places where we are using it as
	an error code.
*/

typedef		unsigned char		UT_Bool;
#define		UT_TRUE				((UT_Bool) 1)
#define		UT_FALSE			((UT_Bool) 0)

/*
	UT_ErrorCode should be used far more than it is.  Any function
	which reasonably could fail at runtime for anything other than
	a coding error or bug should return an error code.  Error codes
	should be propogated properly.
*/
typedef		UT_sint32			UT_ErrorCode;
#define		UT_OK				((UT_ErrorCode) 0)
#define		UT_OUTOFMEM			((UT_ErrorCode) -100)


/* 
	The MSVC debug runtime library can track leaks back to the 
	original allocation via the following black magic.
*/
#if defined(_MSC_VER) && defined(_DEBUG) && defined(_CRTDBG_MAP_ALLOC)
#include <crtdbg.h>
#define UT_DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new UT_DEBUG_NEW
#endif /* _MSC_VER && _DEBUG && _CRTDBG_MAP_ALLOC */


/*
	Unicode character constants.
*/
#define UCS_TAB				((UT_UCSChar)0x0009)
#define UCS_LF				((UT_UCSChar)0x000a)
#define UCS_VTAB			((UT_UCSChar)0x000b)
#define UCS_FF				((UT_UCSChar)0x000c)
#define UCS_CR				((UT_UCSChar)0x000d)
#define UCS_SPACE			((UT_UCSChar)0x0020)
#define UCS_NBSP			((UT_UCSChar)0x00a0)


/*
** Some useful macros that we use throughout
*/

#define FREEP(p)		do { if (p) free((void *)p); (p)=NULL; } while (0)
#define DELETEP(p)		do { if (p) delete(p); (p)=NULL; } while (0)
#define CLONEP(p,q)		do { FREEP(p); if (q && *q) UT_cloneString(p,q); } while (0)

#define NrElements(a)	((sizeof(a)/sizeof(a[0])))
#define MyMax(a,b)		(((a)>(b)) ? (a) : (b))
#define MyMin(a,b)		(((a)<(b)) ? (a) : (b))

#define UT_UNUSED(v)	do { (v)=(v); } while (0)

#endif /* UT_TYPES_H */
