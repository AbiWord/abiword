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
 


#ifndef UT_MACTIMER_H
#define UT_MACTIMER_H

#include "ut_timer.h"

// Forward declaration
class UT_MacTimer;

//  Define an extended task record. 
struct ExtendedTimerRec {
	TMTask tmTask;
	UT_MacTimer* pTimer;
#if TARGET_CPU_PPC
	//  Nothing needed for PowerPC 
	
#elif TARGET_CPU_68K
	long applicationA5;
	
#else
	#error "Huh?"
	
#endif //  TARGET_CPU_PPC 
};

typedef struct ExtendedTimerRec ExtendedTimerRec, *ExtendedTimerPtr;


class UT_MacTimer : public UT_Timer
{
public:
	UT_MacTimer(UT_TimerCallback pCallback, void* pData, GR_Graphics * pG);
	~UT_MacTimer();

	virtual UT_sint32 set(UT_uint32 iMilliseconds);
	virtual void stop(void);		//  suspend calling timer task 
	virtual void start(void);		//  resume calling timer task 
	virtual void setAsFastAsPossible(void); 
	// Accessors
	bool bGetStarted( void ) { return m_bStarted; };
	UT_sint32 msGetInterval( void ) { return m_iMilliseconds; };

	
private:
	UT_sint32			m_iMilliseconds;  // why not in  base class UT_Timer ???
	bool				m_bStarted;
	ExtendedTimerRec 	m_et;

#if TARGET_CPU_PPC
	static pascal void TimerCallbackProc( ExtendedTimerPtr tmTaskPtr );
	
#elif TARGET_CPU_68K
	static pascal void TimerCallbackProc( void );
	
#else
	#error "Huh?"
	
#endif //  TARGET_CPU_PPC 

	static int 			_compareIdentifiers(const void* p1, const void* p2);
	UT_uint32 			_createIdentifier(void);

};

#endif //  UT_MACTIMER_H 

