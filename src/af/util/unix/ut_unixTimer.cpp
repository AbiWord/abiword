
#include "ut_UNIXTimer.h"
#include "ut_assert.h"

UT_sint32 UT_UNIXTimer::set(UT_uint32 iMilliseconds)
{
	/*
	  TODO

	  The goal here is to set this timer to go off after iMilliseconds
	  have passed.  This method should not block.  It should call some
	  OS routine which provides timing facilities.  It is assumed that this
	  routine requires a C callback.  That callback, when it is called,
	  must look up the UT_Timer object which corresponds to it, and
	  call its fire() method.  See dg_Win32Timer.cpp for an example
	  of how it's done on Windows.  We're hoping that something similar will work
	  for other platforms.
	*/

	UT_ASSERT(UT_NOT_IMPLEMENTED);
	return -1;
}

