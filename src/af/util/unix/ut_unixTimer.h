
#ifndef UT_UNIXTIMER_H
#define UT_UNIXTIMER_H

#include "ut_timer.h"

class UT_UNIXTimer : public UT_Timer
{
public:
	virtual UT_sint32 set(UT_uint32 iMilliseconds);
};

#endif /* UT_UNIXTIMER_H */

