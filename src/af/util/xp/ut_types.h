 
/*
** The contents of this file are subject to the AbiSource Public
** License Version 1.0 (the "License"); you may not use this file
** except in compliance with the License. You may obtain a copy
** of the License at http://www.abisource.com/LICENSE/ 
** 
** Software distributed under the License is distributed on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
** implied. See the License for the specific language governing
** rights and limitations under the License. 
** 
** The Original Code is AbiSource Utilities.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
*/

#ifndef UT_TYPES_H
#define UT_TYPES_H

#ifndef NULL
#define NULL 0
#endif

#ifdef __cplusplus
#define UT_BEGIN_EXTERN_C		extern "C" {
#define UT_END_EXTERN_C			}
#else
#define UT_BEGIN_EXTERN_C
#define UT_END_EXTERN_C
#endif

typedef		unsigned char		UT_Byte;
typedef		unsigned short		UT_UCSChar;		// Unicode

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


#endif /* UT_TYPES_H */
