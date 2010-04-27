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
 


#ifndef UT_UNIXTIMER_H
#define UT_UNIXTIMER_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ut_timer.h"

#ifdef TOOLKIT_COCOA
typedef struct NSMutableDictionary;
typedef struct NSLock;
#endif


class UT_UNIXTimer : public UT_Timer
{
public:
	UT_UNIXTimer(UT_WorkerCallback pCallback, void* pData);
	virtual ~UT_UNIXTimer();

	virtual UT_sint32 set(UT_uint32 iMilliseconds);
	virtual void stop();
	virtual void start();
private:
	UT_sint32 m_iMilliseconds;
	UT_uint32 m_iGtkTimerId;
#ifdef TOOLKIT_COCOA
	/* these are here for Cocoa timer */
	static NSLock* s_timerMutex;
	static NSMutableDictionary* s_timerIds;
	static int s_lastTimerId;

	friend void _checkLock(void);
	friend void XAP_stopCocoaTimer (UT_uint32 timerId);
	friend UT_uint32 XAP_newCocoaTimer (UT_uint32 time, int (*proc)(void *), void *p);
#endif
};

#endif /* UT_UNIXTIMER_H */

