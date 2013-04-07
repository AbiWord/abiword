/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (C) 1998-2000 
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#include "ap_Win32Frame.h"
#include "xap_Win32FrameImpl.h"
#include "ut_debugmsg.h"

#include "gr_Win32Graphics.h"
#include "xad_Document.h"
#include "xav_View.h"
#include "xap_ViewListener.h"
#include "xap_Scrollbar_ViewListener.h"

#ifdef _MSC_VER
#pragma warning(disable: 4355)	// 'this' used in base member initializer list
#endif

AP_Win32Frame::AP_Win32Frame() :
	AP_Frame(new AP_Win32FrameImpl(this))
{
	this->m_pData = NULL;
}

AP_Win32Frame::AP_Win32Frame(AP_Win32Frame * f) :
	AP_Frame(static_cast<AP_Frame *>(f))
{
}

AP_Win32Frame::~AP_Win32Frame(void)
{
	killFrameData();
}

XAP_Frame* AP_Win32Frame::cloneFrame(void)
{
	AP_Win32Frame* pClone = new AP_Win32Frame(this);

	UT_ASSERT_HARMLESS(pClone);
	return pClone;
}

void AP_Win32Frame::setStatusMessage(const char * szMsg) 
{  
	if((getFrameMode() == XAP_NormalFrame) && (m_pData))
	{
		static_cast<AP_FrameData*>(m_pData)->m_pStatusBar->setStatusMessage(szMsg);  
	}
}

bool AP_Win32Frame::initialize(XAP_FrameMode /*frameMode*/)
{
	if (!initFrameData())
		return false;

	// this will call XAP_FrameImpl->_initialize() for us (aka static_cast<AP_Win32FrameImpl*>(getFrameImpl())->_initialize(); )
	if (!XAP_Frame::initialize(AP_PREF_KEY_KeyBindings,AP_PREF_DEFAULT_KeyBindings,
									AP_PREF_KEY_MenuLayout, AP_PREF_DEFAULT_MenuLayout,
									AP_PREF_KEY_StringSet, AP_PREF_DEFAULT_StringSet,
									AP_PREF_KEY_ToolbarLayouts, AP_PREF_DEFAULT_ToolbarLayouts,
									AP_PREF_KEY_StringSet, AP_PREF_DEFAULT_StringSet))
		return false;

	getAPWin32FrameImpl()->_showOrHideToolbars();
	getAPWin32FrameImpl()->_showOrHideStatusbar();

	return true;
}

void AP_Win32Frame::toggleStatusBar(bool bStatusBarOn)
{
	AP_FrameData *pFrameData = getAPFrameData();
	UT_return_if_fail(pFrameData);
	UT_return_if_fail(pFrameData->m_pStatusBar);

	if (bStatusBarOn)
	{
		pFrameData->m_pStatusBar->show();
	}
	else
	{
		pFrameData->m_pStatusBar->hide();
	}

	getAPWin32FrameImpl()->_updateContainerWindow();
}


/************** helper methods for _showDocument ************************/
bool AP_Win32Frame::_createViewGraphics(GR_Graphics *& pG, UT_uint32 iZoom)
{
	pG = getAPWin32FrameImpl()->createDocWndGraphics();
	UT_return_val_if_fail(pG, false);

	pG->setZoomPercentage(iZoom);

	return true;
}

void AP_Win32Frame::_setViewFocus(AV_View * /*pView*/)
{
	/* Nothing todo for Win32 at this time */
}

bool AP_Win32Frame::_createScrollBarListeners(AV_View * pView, AV_ScrollObj *& pScrollObj, 
				       ap_ViewListener *& pViewListener, 
				       ap_Scrollbar_ViewListener *& pScrollbarViewListener,
				       AV_ListenerId &lid, 
				       AV_ListenerId &lidScrollbarViewListener)
{
	// The "AV_ScrollObj pScrollObj" receives
	// send{Vertical,Horizontal}ScrollEvents
	// from both the scroll-related edit methods
	// and from the UI callbacks.
	//
	// The "ap_ViewListener pViewListener" receives
	// change notifications as the document changes.
	// This ViewListener is responsible for keeping
	// the title-bar up to date (primarily title
	// changes, dirty indicator, and window number).
	//
	// The "ap_Scrollbar_ViewListener pScrollbarViewListener"
	// receives change notifications as the doucment changes.
	// This ViewListener is responsible for recalibrating the
	// scrollbars as pages are added/removed from the document.
	//
	// Each Toolbar will also get a ViewListener so that
	// it can update toggle buttons, and other state-indicating
	// controls on it.
	//
	// TODO we ***really*** need to re-do the whole scrollbar thing.
	// TODO we have an addScrollListener() using an m_pScrollObj
	// TODO and a View-Listener, and a bunch of other widget stuff.
	// TODO and its very confusing.

	pScrollObj = new AV_ScrollObj(this, _scrollFuncX, _scrollFuncY);
	UT_return_val_if_fail(pScrollObj, false);

	pViewListener = new ap_ViewListener(this);
	UT_return_val_if_fail(pViewListener, false);

	pScrollbarViewListener = new ap_Scrollbar_ViewListener(this,pView);
	UT_return_val_if_fail(pScrollbarViewListener, false);

	if (!pView->addListener(pViewListener,&lid) ||
		!pView->addListener(pScrollbarViewListener, &lidScrollbarViewListener))
	{
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return false;
	}

	return true;
}

void AP_Win32Frame::_replaceView(GR_Graphics * pG, FL_DocLayout *pDocLayout,
			  AV_View *pView, AV_ScrollObj * pScrollObj,
			  ap_ViewListener *pViewListener, AD_Document *pOldDoc,
			  ap_Scrollbar_ViewListener *pScrollbarViewListener,
			  AV_ListenerId lid, AV_ListenerId lidScrollbarViewListener,
			  UT_uint32 iZoom)
{
	AP_Frame::_replaceView( pG, pDocLayout, 
							pView, pScrollObj, 
							pViewListener, pOldDoc, 
							pScrollbarViewListener,
							lid, lidScrollbarViewListener, 
							iZoom
	);
}

void AP_Win32Frame::_scrollFuncY(void* pData, UT_sint32 yoff, UT_sint32 ylimit)
{
	// this is a static callback function and doesn't have a 'this' pointer.
	AP_Win32Frame * pWin32Frame = static_cast<AP_Win32Frame *>(pData);
	UT_return_if_fail (pWin32Frame);

	pWin32Frame->getAPWin32FrameImpl()->_scrollFuncY(yoff, ylimit);
}

void AP_Win32Frame::_scrollFuncX(void* pData, UT_sint32 xoff, UT_sint32 xlimit)
{
	// this is a static callback function and doesn't have a 'this' pointer.
	AP_Win32Frame * pWin32Frame = static_cast<AP_Win32Frame *>(pData);
	UT_return_if_fail (pWin32Frame);

	pWin32Frame->getAPWin32FrameImpl()->_scrollFuncX(xoff, xlimit);
}
