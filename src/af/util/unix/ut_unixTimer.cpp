
#include "ut_UNIXTimer.h"
#include "ut_assert.h"

/*
  XtAppAddTimeOut looks like the way to do this.  It will
  be necessary to add the app_context as an argument to the constructor
  somehow, perhaps introducing the need for a factory method somewhere
  so that platform-specific timers can be created from share code.
*/

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

