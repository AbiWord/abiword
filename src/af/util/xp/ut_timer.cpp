
#include "ut_timer.h"

#include "ut_assert.h"

UT_Timer::UT_Timer(UT_TimerCallback pCallback, void* pInstanceData)
{
	m_pCallback = pCallback;
	m_pInstanceData = pInstanceData;
	m_iIdentifier = 0;
	
	static_vecTimers.addItem(this);
}

void UT_Timer::setIdentifier(UT_uint32 iIdentifier)
{
	m_iIdentifier = iIdentifier;
}

UT_uint32 UT_Timer::getIdentifier()
{
	return m_iIdentifier;
}

void* UT_Timer::getInstanceData()
{
	return m_pInstanceData;
}

UT_TimerCallback UT_Timer::getCallback()
{
	return m_pCallback;
}

UT_Timer* UT_Timer::findTimer(UT_uint32 iIdentifier)
{
	int count = static_vecTimers.getItemCount();
	for (int i=0; i<count; i++)
	{
		UT_Timer* pTimer = (UT_Timer*) static_vecTimers.getNthItem(i);
		UT_ASSERT(pTimer);
		
		if (pTimer->getIdentifier() == iIdentifier)
		{
			return pTimer;
		}
	}

	return NULL;
}

void UT_Timer::fire()
{
	m_pCallback(this);
}
