/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */ 
 


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
