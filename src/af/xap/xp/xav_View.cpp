/* AbiSource Application Framework
 * Copyright (C) 1998-2000 AbiSource, Inc.
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


#include "ut_debugmsg.h"

#include "xav_View.h"
#include "xav_Listener.h"

#include "xap_Frame.h"
#include "xap_App.h"

AV_View::AV_View(XAP_App * pApp, void* pParentData)
:	m_pApp(pApp),
	m_pParentData(pParentData),
	m_xScrollOffset(0),
	m_yScrollOffset(0),
	m_iWindowHeight(0),
	m_iWindowWidth(0),
	m_focus(AV_FOCUS_NONE),
	m_iTick(0),
	m_bInsertMode(true),
	m_bIsLayoutFilling(false)
{
}

AV_View::~AV_View()
{
	UT_DEBUGMSG(("Deleting view %x \n",this));
}

void* AV_View::getParentData() const
{
	return m_pParentData;
}

bool AV_View::addListener(AV_Listener * pListener, 
							 AV_ListenerId * pListenerId)
{
	UT_uint32 kLimit = m_vecListeners.getItemCount();
	UT_uint32 k;

	// see if we can recycle a cell in the vector.
	
	for (k=0; k<kLimit; k++)
		if (m_vecListeners.getNthItem(k) == 0)
		{
			static_cast<void>(m_vecListeners.setNthItem(k,pListener,NULL));
			goto ClaimThisK;
		}

	// otherwise, extend the vector for it.
	
	if (m_vecListeners.addItem(pListener,&k) != 0)
	{
		return false;				// could not add item to vector
	}

  ClaimThisK:

	// give our vector index back to the caller as a "Listener Id".
	
	*pListenerId = k;
	return true;
}

bool AV_View::removeListener(AV_ListenerId listenerId)
{
	if (listenerId == (AV_ListenerId) -1)
		return false;
		
	return (m_vecListeners.setNthItem(listenerId,NULL,NULL) == 0);
}

bool AV_View::notifyListeners(const AV_ChangeMask hint)
{
	/*
		App-specific logic calls this virtual method when relevant portions of 
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
	if(!isDocumentPresent())
	{
		return false;
	}
	m_iTick++;

	// make sure there's something left

	if (hint == AV_CHG_NONE)
	{
		return false;
	}
	
	// notify listeners of a change.
		
	AV_ListenerId lid;
	AV_ListenerId lidCount = m_vecListeners.getItemCount();

	// for each listener in our vector, we send a notification.
	// we step over null listners (for listeners which have been
	// removed (views that went away)).
	bool bIsLayoutFilling = isLayoutFilling();
	for (lid=0; lid<lidCount; lid++)
	{
		AV_Listener * pListener = static_cast<AV_Listener *>(m_vecListeners.getNthItem(lid));
		if(pListener && (!bIsLayoutFilling
						 || (pListener->getType()== AV_LISTENER_STATUSBAR)
						 || (pListener->getType()== AV_LISTENER_SCROLLBAR)))
		{
				pListener->notify(this,hint);
		}
	}

	return true;
}

bool AV_View::isActive(void) 
{
	AV_View* pActiveView = NULL;
	XAP_Frame* lff = getApp()->getLastFocussedFrame();
	if(lff) 
	{
		pActiveView = lff->getCurrentView();
	}
	else 
	{
		pActiveView = this;
	}
	
	return pActiveView == this;
}

UT_uint32 AV_View::getTick(void)
{
        return m_iTick;
}

void   AV_View::incTick(void)
{
        m_iTick++;
}

void AV_View::setInsertMode(bool bInsert)
{
	m_bInsertMode = bInsert; 

	notifyListeners(AV_CHG_INSERTMODE); 
};

/*! the input is in device units, but internal storage is in logical units
 */
void AV_View::setWindowSize(UT_sint32 width, UT_sint32 height)
{
	_UUL(width);
	_UUL(height);
	m_iWindowWidth = width;
	m_iWindowHeight = height;

	notifyListeners(AV_CHG_WINDOWSIZE);
}

void AV_View::addScrollListener(AV_ScrollObj* pObj)
{
	m_scrollListeners.addItem(static_cast<void *>(pObj));
}

void AV_View::removeScrollListener(AV_ScrollObj* pObj)
{
	UT_sint32 count = m_scrollListeners.getItemCount();

	for (UT_sint32 i = 0; i < count; i++)
	{
		AV_ScrollObj* obj = static_cast<AV_ScrollObj*>(m_scrollListeners.getNthItem(i));

		if (obj == pObj)
		{
			m_scrollListeners.deleteNthItem(i);
			break;
		}
	}
}

/*! the input is in device units
 */
void AV_View::sendVerticalScrollEvent(UT_sint32 yoff, UT_sint32 ylimit)
{
	if(getWindowHeight() < _UL(20))
		return;
	UT_sint32 count = m_scrollListeners.getItemCount();

	for (UT_sint32 i = 0; i < count; i++)
	{
		AV_ScrollObj* pObj = static_cast<AV_ScrollObj*>(m_scrollListeners.getNthItem(i));
		pObj->m_pfnY(pObj->m_pData, yoff, ylimit);
	}
}

/*! the input is in device units
 */
void AV_View::sendHorizontalScrollEvent(UT_sint32 xoff, UT_sint32 xlimit)
{
	if(getWindowHeight() < _UL(20))
		return;

	UT_sint32 count = m_scrollListeners.getItemCount();

	for (UT_sint32 i = 0; i < count; i++)
	{
		AV_ScrollObj* pObj = static_cast<AV_ScrollObj*>(m_scrollListeners.getNthItem(i));

		pObj->m_pfnX(pObj->m_pData, xoff, xlimit);
	}
}






