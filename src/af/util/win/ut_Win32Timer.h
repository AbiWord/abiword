
#ifndef UT_WIN32TIMER_H
#define UT_WIN32TIMER_H

#include "ut_timer.h"

class UT_Win32Timer : public UT_Timer
{
public:
	virtual UT_sint32 set(UT_uint32 iMilliseconds);
};

#endif /* UT_WIN32TIMER_H */

