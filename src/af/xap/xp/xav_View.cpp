/* AbiWord
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


#include "ut_types.h"
#include "av_View.h"
#include "av_Listener.h"

AV_View::AV_View(void* pParentData)
{
	m_pParentData = pParentData;
	
	m_xScrollOffset = 0;
	m_yScrollOffset = 0;
	m_iWindowHeight = 0;
	m_iWindowWidth = 0;
}

AV_View::~AV_View()
{
}
	
void* AV_View::getParentData() const
{
	return m_pParentData;
}

UT_Bool AV_View::addListener(AV_Listener * pListener, 
							 AV_ListenerId * pListenerId)
{
	UT_uint32 kLimit = m_vecListeners.getItemCount();
	UT_uint32 k;

	// see if we can recycle a cell in the vector.
	
	for (k=0; k<kLimit; k++)
		if (m_vecListeners.getNthItem(k) == 0)
		{
			(void)m_vecListeners.setNthItem(k,pListener,NULL);
			goto ClaimThisK;
		}

	// otherwise, extend the vector for it.
	
	if (m_vecListeners.addItem(pListener,&k) != 0)
	{
		return UT_FALSE;				// could not add item to vector
	}

  ClaimThisK:

	// give our vector index back to the caller as a "Listener Id".
	
	*pListenerId = k;
	return UT_TRUE;
}

UT_Bool AV_View::removeListener(AV_ListenerId listenerId)
{
	return (m_vecListeners.setNthItem(listenerId,NULL,NULL) == 0);
}

UT_Bool AV_View::notifyListeners(const AV_ChangeMask hint)
{
	/*
		App-specific logic calls this vritual method when relevant portions of 
		the view state *may* have changed.  (That's why it's called a hint.)

		This base class implementation doesn't do any filtering of those 
		hints, it just broadcasts those hints to all listeners.  

		Subclasses are encouraged to improve the quality of those hints by 
		filtering out mask bits which haven't *actually* changed since the 
		last notification.  To do so, they would 

			- copy the hint (it's passed as a const),
			- clear any irrelevant bits, and 
			- call this implementation on the way out

		Good hinting logic is app-specific, and non-trivial to tune, but it's 
		worth doing, because it helps minimizes flicker for things like 
		toolbar button state.  
	*/

	// make sure there's something left

	if (hint == AV_CHG_NONE)
	{
		return UT_FALSE;
	}
	
	// notify listeners of a change.
		
	AV_ListenerId lid;
	AV_ListenerId lidCount = m_vecListeners.getItemCount();

	// for each listener in our vector, we send a notification.
	// we step over null listners (for listeners which have been
	// removed (views that went away)).
	
	for (lid=0; lid<lidCount; lid++)
	{
		AV_Listener * pListener = (AV_Listener *)m_vecListeners.getNthItem(lid);
		if (pListener)
			pListener->notify(this,hint);
	}

	return UT_TRUE;
}

void AV_View::setWindowSize(UT_sint32 width, UT_sint32 height)
{
	m_iWindowWidth = width;
	m_iWindowHeight = height;
}

void AV_View::addScrollListener(AV_ScrollObj* pObj)
{
	m_scrollListeners.addItem((void *)pObj);
}

void AV_View::removeScrollListener(AV_ScrollObj* pObj)
{
	UT_sint32 count = m_scrollListeners.getItemCount();

	for (UT_sint32 i = 0; i < count; i++)
	{
		AV_ScrollObj* obj = (AV_ScrollObj*) m_scrollListeners.getNthItem(i);

		if (obj == pObj)
		{
			m_scrollListeners.deleteNthItem(i);
			break;
		}
	}
}

void AV_View::sendScrollEvent(UT_sint32 xoff, UT_sint32 yoff)
{
	UT_sint32 count = m_scrollListeners.getItemCount();

	for (UT_sint32 i = 0; i < count; i++)
	{
		AV_ScrollObj* pObj = (AV_ScrollObj*) m_scrollListeners.getNthItem(i);

		pObj->m_pfn(pObj->m_pData, xoff, yoff);
	}
}
