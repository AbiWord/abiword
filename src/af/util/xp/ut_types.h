
#ifndef UT_TYPES_H
#define UT_TYPES_H

#include "prtypes.h"


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
#define		UT_TRUE				(1)
#define		UT_FALSE			(0)

/*
	UT_ErrorCode should be used far more than it is.  Any function
	which reasonably could fail at runtime for anything other than
	a coding error or bug should return an error code.  Error codes
	should be propogated properly.
*/
typedef		UT_sint32			UT_ErrorCode;
#define		UT_OK				(0)
#define		UT_OUTOFMEM			(-100)

#endif /* UT_TYPES_H */
