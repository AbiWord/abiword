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

#include <stdio.h>

#include <Timer.h>

#include "ut_MacTimer.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// BPF 19 June, 2001 - feel free to modify, this is a first cut

// Note:
//
// The specification for the function set( ) and the operations
// concerning the Identifier seem not to be well specified, and
// these (togther with start( ) and stop( )) are not used the way 
// that I expected. My implementation (copied from the Windows 
// one) is not necessarily correct.
//
// Also, (1) this class is entirely in the hands of whatever is passed
// as pCallback, and (2) conceivably bad things might happen if the
// destructor, which removes the task from the queue is not called 
// (though C++ should guarantee that in this case the taskptr and
// timerproc remain valid).
//
// See note in stop( ).
//
// Should there be an unset( ) or clear( ) operation?
// 
// What do we use the pGR_Graphics for?

//  Define the interface for a completion function. 
#if TARGET_CPU_PPC
	
#elif TARGET_CPU_68K
// This inline function returns the extended Time Manager task pointer,
// which is passed to the completion routine in register A1.
pascal ExtendedTimerPtr GetTMTaskPtr(void) = 0x2E89;
	
#else
	#error "Huh?"
	
#endif //  TARGET_CPU_PPC 


/*****************************************************************/
	
UT_Timer* UT_Timer::static_constructor(UT_TimerCallback pCallback,
									   void* pData,
									   GR_Graphics *pG)
{
	UT_ASSERT(pCallback);

	UT_MacTimer * p = new UT_MacTimer(pCallback,pData,pG); 
	return p;
}

/*****************************************************************/

UT_MacTimer::UT_MacTimer( UT_TimerCallback pCallback, void* pData, GR_Graphics *pG )
{
	( void )pG;

	setCallback( pCallback );
	setInstanceData( pData );
	m_bStarted 		= false;
	m_iMilliseconds = 0;
	setIdentifier( _createIdentifier() ); // should this be here ...
}


UT_MacTimer::~UT_MacTimer()
{
	stop();
	RmvTime((QElemPtr) &m_et.tmTask);	// Should there be a check for valid id?
}

UT_sint32 UT_MacTimer::set (UT_uint32 iMilliseconds)
{
	// set the period and start firing events.
	UT_uint32 idTimer = rand( ) *1000;

 	m_iMilliseconds 	= iMilliseconds;
	m_bStarted 			= true;
	setIdentifier( idTimer ); // ... or here?
	
#if( TARGET_API_MAC_OS8 || TARGET_API_MAC_CARBON )
	// Not needed if it is known that we are running under certain
	// later versions of the OS.
	
#else
	long gestaltResponse;
	
	if (Gestalt(gestaltTimeMgrVersion, &gestaltResponse) != noErr
		|| (gestaltResponse < gestaltExtendedTimeMgr) ) 	// Always true for OS > 7.0
			//  The extended Time Manager is not present. 
			goto failure;
	if (Gestalt(gestaltOSAttr, &gestaltResponse) != noErr
		|| (gestaltResponse & (1L << gestaltLaunchControl)) == 0)  	// Always true for OS > 7.0
			//  The Process Manager is not present. 
			goto failure;
			
#endif // ( TARGET_API_MAC_OS8 || TARGET_API_MAC_CARBON )
			
	// Configure the structure that stores the timing information.	
	m_et.tmTask.qLink		= NULL;
	m_et.tmTask.qType 		= 0;
	m_et.tmTask.tmAddr 		= NewTimerUPP ((TimerProcPtr)&UT_MacTimer::TimerCallbackProc);
	m_et.tmTask.tmCount 	= 0;  // must do this
	m_et.tmTask.tmWakeUp 	= 0;
	m_et.tmTask.tmReserved 	= 0;
	
#if TARGET_CPU_PPC
	//  Nothing needed for PowerPC 
	
#elif TARGET_CPU_68K
	m_et.applicationA5 		= SetCurrentA5();
	
#else
	#error "Huh?"
	
#endif //  TARGET_CPU_PPC 

	// Note that we do not allow the timer to be 'set' without
	// also being 'started'
	m_et.pTimer = this;
	InsXTime( (QElemPtr)&m_et.tmTask );
	start( ); 
	
#if( TARGET_API_MAC_OS8 || TARGET_API_MAC_CARBON )
	// Not needed if it is known that we are running under certain
	// later versions of the OS.
	
#else
failure:
			
#endif // ( TARGET_API_MAC_OS8 || TARGET_API_MAC_CARBON )
	;
	
	return 0; // why
}

void UT_MacTimer::start (void)
{
	// resume the delivery of events using the last period set.

	UT_ASSERT(m_iMilliseconds > 0);
	
	if (!m_bStarted)
		set(m_iMilliseconds);
// DISABLED
//	PrimeTime( (QElemPtr) &m_et.tmTask, m_iMilliseconds ); 
	UT_DEBUGMSG (("Timer Task Disabled !!\n"));
}

void UT_MacTimer::stop (void)
{
	// stop the delivery of timer events.
	// stop the OS timer from firing, but do not delete the class.
	
	UT_uint32 idTimer = getIdentifier( );

	if (idTimer == 0)
		return;
		
	// (We don't Rmv the task here, but perhaps we should ...)
	
	m_bStarted = false;
}


UT_uint32 UT_MacTimer::_createIdentifier(void)
{
	UT_Timer::_getVecTimers().qsort(_compareIdentifiers); 

	// Take the first unused identifier number different from zero
	UT_uint32 iIdentifier = 0;
	UT_uint32 count = _getVecTimers().getItemCount();
	for (UT_uint32 i=0; i<count; i++, iIdentifier++)
	{
		UT_Timer* pTimer = (UT_Timer*) _getVecTimers().getNthItem(i);
		UT_ASSERT(pTimer);
		
		UT_uint32 iTimerId = pTimer->getIdentifier();
		if (iTimerId && iTimerId != iIdentifier)
		{
			break;
		}
	}

	// Should be 16 bits maximum
	UT_ASSERT((iIdentifier & 0xFFFF0000) == 0);

	return iIdentifier;
}

int UT_MacTimer::_compareIdentifiers(const void* p1, const void* p2)
{
	UT_MacTimer** ppTimer1 = (UT_MacTimer**) p1;
	UT_MacTimer** ppTimer2 = (UT_MacTimer**) p2;

	if ((*ppTimer1)->getIdentifier() < (*ppTimer2)->getIdentifier())
	{
		return -1;
	}
	
	if ((*ppTimer1)->getIdentifier() > (*ppTimer2)->getIdentifier())
	{
		return 1;
	}
	
	return 0;
}

#if TARGET_CPU_PPC
pascal void UT_MacTimer::TimerCallbackProc(
    ExtendedTimerPtr tmTaskPtr
)
{
	
#elif TARGET_CPU_68K
pascal void UT_MacTimer::TimerCallbackProc(void)
{
    ExtendedTimerPtr tmTaskPtr;
    long oldA5;
    
    tmTaskPtr = GetTMTaskPtr();
    oldA5 = SetA5( tmTaskPtr->applicationA5 );
	
#else
	#error "Huh?"
	
#endif //  TARGET_CPU_PPC 

	// 
	// Interrupt time
	//
	
	//  Restart timer 
	if( tmTaskPtr->pTimer->bGetStarted( ) ) {
//	DISABLED
//		PrimeTime((QElemPtr) &( tmTaskPtr->tmTask ), tmTaskPtr->pTimer->msGetInterval( ) );
		UT_DEBUGMSG (("Timer Task Disabled !!\n"));
	}

	//  Do the work 
	tmTaskPtr->pTimer->fire();

#if TARGET_CPU_PPC
    //  Nothing needed at completion routine exit. 
	
#elif TARGET_CPU_68K
    SetA5(oldA5);
	
#else
	#error "Huh?"
	
#endif //  TARGET_CPU_PPC 
}

