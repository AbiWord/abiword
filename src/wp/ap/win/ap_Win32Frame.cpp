/* AbiWord
 * Copyright (C) 1998-2002 AbiSource, Inc.
 * Copyright (C) 1998-2002 
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

#include "ap_Win32Frame.h"
#include "xap_Win32FrameImpl.h"
#include "ut_debugmsg.h"

#include "gr_Win32Graphics.h"
#include "xav_View.h"
#include "xad_Document.h"
#include "xap_Scrollbar_ViewListener.h"

#ifdef _MSC_VER
#pragma warning(disable: 4355)	// 'this' used in base member initializer list
#endif

AP_Win32Frame::AP_Win32Frame(XAP_Win32App * app) :
	AP_Frame(new AP_Win32FrameImpl(this), app)
{
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

	UT_ASSERT(pClone);
	return pClone;
}

bool AP_Win32Frame::initialize(XAP_FrameMode frameMode)
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

	_showOrHideToolbars();
	_showOrHideStatusbar();

	return true;
}

void AP_Win32Frame::toggleBar(UT_uint32 iBarNb, bool bBarOn)
{
	UT_DEBUGMSG(("AP_Win32Frame::toggleBar %d, %d\n", iBarNb, bBarOn));

#if INPROGRESS
	// TMN: Yes, this cast is correct. We store EV_Win32Toolbar in
	// m_vecToolbars, but we only need the EV_Toolbar part of it.
	EV_Toolbar* pToolbar = (EV_Toolbar*)m_vecToolbars.getNthItem(iBarNb);

	UT_ASSERT(pToolbar);

	if (!pToolbar)	// release build paranoia
	{
		return;
	}

	if (bBarOn)
	{
		pToolbar->show();
	}
	else
	{
		pToolbar->hide();
	}

	int iToolbarCount = 0;
	for (UT_uint32 i = 0; i < m_vecToolbarLayoutNames.getItemCount(); i++)
	{
		EV_Win32Toolbar *pThisToolbar = (EV_Win32Toolbar *)m_vecToolbars.getNthItem(i);

		UT_ASSERT(pThisToolbar);

		if (pThisToolbar)	// release build paranoia
			if ( pThisToolbar->bVisible() )
				iToolbarCount++;
	}

	if( !iToolbarCount || ((iToolbarCount == 1) && bBarOn) )
	{
		RECT r;

		ShowWindow( m_hwndRebar, (iToolbarCount ? SW_SHOW : SW_HIDE) );

		MoveWindow(m_hwndRebar, 0, 0, (iToolbarCount ? m_iSizeWidth : 1),
									  (iToolbarCount ? m_iSizeHeight : 1), TRUE);

		if( iToolbarCount )
		{
			GetClientRect(m_hwndRebar, &r);
			m_iBarHeight = r.bottom - r.top + 6;
		}
		else
			m_iBarHeight = 1;

		GetClientRect(m_hwndContainer, &r);
		_onSize(r.right - r.left, r.bottom - r.top);
	}

	// We *need* to make the window recalc its layout after adding/removing a
	// toolbar in the rebar control. Since we have no "recalcLayout" I'm
	// aware of we use this not-so-good-but-old idiom of resizing the window.
	RECT rc;
	GetWindowRect(m_hwndFrame, &rc);
	const int cx = rc.right - rc.left;
	const int cy = rc.bottom - rc.top;
	const UINT fFlags =
		SWP_FRAMECHANGED	|
		SWP_NOACTIVATE		|
		SWP_NOCOPYBITS		|
		SWP_NOMOVE			|
		SWP_NOOWNERZORDER	|
		SWP_NOZORDER;
	SetWindowPos(m_hwndFrame, 0, 0, 0, cx - 1, cy - 1, fFlags | SWP_NOREDRAW);
	SetWindowPos(m_hwndFrame, 0, 0, 0, cx, cy, fFlags);

#endif /* INPROGRESS */
}

void AP_Win32Frame::toggleStatusBar(bool bStatusBarOn)
{
	AP_FrameData *pFrameData = static_cast<AP_FrameData *>(getFrameData());
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

	UpdateWindow(static_cast<AP_Win32FrameImpl *>(getFrameImpl())->_getHwndContainer());
}

void AP_Win32Frame::_showOrHideToolbars(void)
{
	bool *bShowBar = static_cast<AP_FrameData*>(getFrameData())->m_bShowBar;

	for (UT_uint32 i = 0; i < m_vecToolbarLayoutNames.getItemCount(); i++)
	{
		if (!bShowBar[i])
			toggleBar( i, false );
	}
}

void AP_Win32Frame::_showOrHideStatusbar(void)
{
	bool bShowStatusBar = static_cast<AP_FrameData*>(getFrameData())->m_bShowStatusBar;
	toggleStatusBar(bShowStatusBar);
}

void AP_Win32Frame::setZoomPercentage(UT_uint32 iZoom)
{
	_showDocument(iZoom);
}

UT_uint32 AP_Win32Frame::getZoomPercentage(void)
{
	return static_cast<AP_FrameData*>(m_pData)->m_pG->getZoomPercentage();
}

/************** helper methods for _showDocument ************************/
bool AP_Win32Frame::_createViewGraphics(GR_Graphics *& pG, UT_uint32 iZoom)
{
	HWND hwndDocument = static_cast<AP_Win32FrameImpl *>(getFrameImpl())->_getHwndDocument();
	pG = new GR_Win32Graphics(GetDC(hwndDocument), hwndDocument, getApp());
	UT_return_val_if_fail(pG, false);

	pG->setZoomPercentage(iZoom);

	return true;
}

void AP_Win32Frame::_setViewFocus(AV_View *pView)
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

#if INPROGRESS
	pViewListener = new ap_ViewListener(this);
	UT_return_val_if_fail(pViewListener, false);

	pScrollbarViewListener = new ap_Scrollbar_ViewListener(this,pView);
	UT_return_val_if_fail(pScrollbarViewListener, false);

	if (!pView->addListener(pViewListener,&lid) ||
		!pView->addListener(pScrollbarViewListener, &lidScrollbarViewListener))
	{
		UT_ASSERT(0);
		return false;
	}
#endif

	return true;
}

void AP_Win32Frame::_bindToolbars(AV_View *pView)
{
	const UT_uint32 nrToolbars = m_vecToolbarLayoutNames.getItemCount();
	for (UT_uint32 k = 0; k < nrToolbars; ++k)
	{
		// TODO Toolbars are a frame-level item, but a view-listener is
		// TODO a view-level item.  I've bound the toolbar-view-listeners
		// TODO to the current view within this frame and have code in the
		// TODO toolbar to allow the view-listener to be rebound to a different
		// TODO view.  in the future, when we have support for multiple views
		// TODO in the frame (think splitter windows), we will need to have
		// TODO a loop like this to help change the focus when the current
		// TODO view changes.

		EV_Win32Toolbar* pWin32Toolbar = (EV_Win32Toolbar *)m_vecToolbars.getNthItem(k);
		pWin32Toolbar->bindListenerToView(pView);
	}
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

	/* With the Frame refactoring into Frame & FrameImpl, the AP_Frame class
	 * replaced _showDocument, however in the new calling hierarchy there was
	 * no place for this chunk, given its former location, this is roughly the
	 * the same place.
	 * For now it seems appropriate here, however, feel free to move it
	 * to somewhere you feel more appropriate and if so, remove _replaceView(...)
	 * from ap_Win32Frame.   KJD
	 */

	// WHY would we want to do this ??? (either we have loading an
	// existing document, and then the text in it has its own lang
	// property or we are creating a new one, in which case this has
	// already been taken care of when the document was created) Tomas

	// what we want to do here is to set the default language
	// that we're editing in
	
	// 27/10/2002 - If we do not have this piece of code, Abiword does not honor the documentlocale
	// setting under win32 

	const XML_Char * doc_locale = NULL;
	if (pView && XAP_App::getApp()->getPrefs()->getPrefsValue(XAP_PREF_KEY_DocumentLocale,&doc_locale))
	{
		if (doc_locale)
		{
			const XML_Char * props[3];
			props[0] = "lang";
			props[1] = doc_locale;
			props[2] = 0;
			static_cast<FV_View *>(m_pView)->setCharFormat(props);
		}
		
		static_cast<FV_View *>(m_pView)->notifyListeners(AV_CHG_ALL);
		static_cast<FV_View *>(m_pView)->focusChange(AV_FOCUS_HERE);
	}	
}


/*** helper methods for helper methods for _showDocument (meta-helper-methods?) :-) ***/
/* called by AP_Frame::_replaceView which is called by AP_Frame::_showDocument
 * Prior to refactoring this was:
	RECT r;
	GetClientRect(hwnd, &r);
	m_pView->setWindowSize(r.right - r.left, r.bottom - r.top);
	InvalidateRect(hwnd, NULL, TRUE);
 * oh well
 */

UT_sint32 AP_Win32Frame::_getDocumentAreaWidth(void)
{
	HWND hwnd = static_cast<AP_Win32FrameImpl *>(getFrameImpl())->_getHwndDocument();
	RECT r;
	GetClientRect(hwnd, &r);
	InvalidateRect(hwnd, NULL, TRUE);
	return r.right - r.left;
}

UT_sint32 AP_Win32Frame::_getDocumentAreaHeight(void)
{
	HWND hwnd = static_cast<AP_Win32FrameImpl *>(getFrameImpl())->_getHwndDocument();
	RECT r;
	GetClientRect(hwnd, &r);
	InvalidateRect(hwnd, NULL, TRUE);
	return r.bottom - r.top;
}


void AP_Win32Frame::_scrollFuncY(void* pData, UT_sint32 yoff, UT_sint32 /*ylimit*/)
{
	// this is a static callback function and doesn't have a 'this' pointer.

	// scroll event came in (probably from an EditMethod (like a PageDown
	// or insertData or something).  update the on-screen scrollbar and
	// then warp the document window contents.

	AP_Win32Frame * pWin32Frame = static_cast<AP_Win32Frame *>(pData);
	UT_ASSERT(pWin32Frame);
	AP_Win32FrameImpl * pWin32FrameImpl = static_cast<AP_Win32FrameImpl *>(pWin32Frame->getFrameImpl());
	UT_ASSERT(pWin32FrameImpl);

	SCROLLINFO si = { 0 };
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;

	pWin32FrameImpl->_getVerticalScrollInfo(&si);
	si.nPos = _UD(yoff);
	pWin32FrameImpl->_setVerticalScrollInfo(&si);
	pWin32FrameImpl->_getVerticalScrollInfo(&si); // values may have been clamped
	pWin32Frame->m_pView->setYScrollOffset(_UL(si.nPos));
}

void AP_Win32Frame::_scrollFuncX(void* pData, UT_sint32 xoff, UT_sint32 /*xlimit*/)
{
	// this is a static callback function and doesn't have a 'this' pointer.

	AP_Win32Frame * pWin32Frame = static_cast<AP_Win32Frame *>(pData);
	UT_ASSERT(pWin32Frame);

	SCROLLINFO si = { 0 };
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;

	HWND hwndH = static_cast<AP_Win32FrameImpl *>(pWin32Frame->getFrameImpl())->_getHwndHScroll();
	GetScrollInfo(hwndH, SB_CTL, &si);

	si.nPos = _UD(xoff);
	SetScrollInfo(hwndH, SB_CTL, &si, TRUE);

	GetScrollInfo(hwndH, SB_CTL, &si);	// may have been clamped
	pWin32Frame->m_pView->setXScrollOffset(_UL(si.nPos));
}

