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
 


#ifndef UT_TIMER_H
#define UT_TIMER_H

#include "ut_types.h"
#include "ut_vector.h"

class UT_Timer;

typedef void (*UT_TimerCallback)(UT_Timer* pTimer);

/*
	UT_Timer is an abstract class which encapsulates the platform-specific 
	details for managing timers.    
*/
class UT_Timer
{
public:
	virtual ~UT_Timer();
	
	void setCallback(UT_TimerCallback p);
	UT_TimerCallback getCallback();
	
	void setInstanceData(void*);
	void* getInstanceData();
	
	virtual UT_sint32 set(UT_uint32 iMilliseconds) = 0;
	void fire();
	
	void setIdentifier(UT_uint32);
	UT_uint32 getIdentifier();
	
	static UT_Timer* findTimer(UT_uint32 iIdentifier);

	/*
		Note that the static_constructor is actually implemented in 
		*platform* code, so that it can instantiate the appropriate 
		platform-specific subclass.
	*/
	static UT_Timer* static_constructor(UT_TimerCallback pCallback, void* pData);
	
protected:
	UT_Timer();		// should only be called from static_constructor()

	void* m_pInstanceData;
	UT_TimerCallback m_pCallback;
	UT_uint32 m_iIdentifier;

	static UT_Vector static_vecTimers;
};

#endif /* UT_TIMER_H */
