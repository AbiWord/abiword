
#ifndef UT_TIMER_H
#define UT_TIMER_H

#include "ut_types.h"
#include "ut_vector.h"

class UT_Timer;

typedef void (*UT_TimerCallback)(UT_Timer* pTimer);

class UT_Timer
{
public:
	UT_Timer();
	
	void setCallback(UT_TimerCallback p);
	UT_TimerCallback getCallback();
	
	void setInstanceData(void*);
	void* getInstanceData();
	
	virtual UT_sint32 set(UT_uint32 iMilliseconds) = 0;
	void fire();
	
	void setIdentifier(UT_uint32);
	UT_uint32 getIdentifier();
	
	static UT_Timer* findTimer(UT_uint32 iIdentifier);
	
protected:
	void* m_pInstanceData;
	UT_TimerCallback m_pCallback;
	UT_uint32 m_iIdentifier;

	static UT_Vector static_vecTimers;
};

#endif /* UT_TIMER_H */
