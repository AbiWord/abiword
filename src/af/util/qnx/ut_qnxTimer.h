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
 


#ifndef UT_QNXTIMER_H
#define UT_QNXTIMER_H

#include "ut_timer.h"

#include <sys/time.h>
#include <sys/siginfo.h>

class UT_QNXTimer : public UT_Timer
{
public:
	UT_QNXTimer(UT_TimerCallback pCallback, void* pData);
	virtual ~UT_QNXTimer();

	virtual UT_sint32 set(UT_uint32 iMilliseconds);
	virtual void stop(void);
	virtual void start(void);
	virtual void resetIfStarted(void);
	
protected:
	UT_sint32 m_iMilliseconds;
	UT_Bool m_bStarted;

#if defined(USE_TIMER_THREADS) 
	timer_t timerid;
	struct sigevent event;
	struct itimerspec off;
#endif
};

#endif /* UT_QNXTIMER_H */

