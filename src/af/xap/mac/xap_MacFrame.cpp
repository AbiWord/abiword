/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 1999 John Brewer DBA Jera Design
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

#ifndef XP_MAC_TARGET_QUARTZ
# include <QuickDraw.h>
#endif
// HIToolbox headers
#include <MacWindows.h>
#include <Appearance.h>
#include <ControlDefinitions.h>
#include <Controls.h>

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_vector.h"
#include "ut_string.h"
#include "ut_growbuf.h"
#include "xap_App.h"
#include "xap_Frame.h"
#include "xap_Prefs.h"
#include "xap_ViewListener.h"
#include "xap_MacApp.h"
#include "xap_MacFrame.h"
#include "ev_MacKeyboard.h"
#include "ev_MacMouse.h"
#include "ev_MacMenu.h"
#include "ev_MacToolbar.h"
#include "ev_EditMethod.h"
#include "xav_View.h"
#include "xad_Document.h"

/*****************************************************************/

#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

	

/*****************************************************************/

XAP_MacFrame::XAP_MacFrame(XAP_MacApp * app)
	: XAP_Frame(static_cast<XAP_App *>(app)),
          m_dialogFactory(static_cast<XAP_Frame *>(this), static_cast<XAP_App *>(app))
{
	m_MacWindow = NULL;
        m_pKeyboard = NULL;
        m_pMacMenu = NULL;
        m_pMouse = NULL;
        m_HScrollBar = NULL;
        m_VScrollBar = NULL;
	MacWindowInit ();
}

// TODO when cloning a new frame from an existing one
// TODO should we also clone any frame-persistent
// TODO dialog data ??

XAP_MacFrame::XAP_MacFrame(XAP_MacFrame * f)
	: XAP_Frame(static_cast<XAP_Frame *>(f)),
          m_dialogFactory(static_cast<XAP_Frame *>(this), static_cast<XAP_App *>(f->m_app))
{
        m_MacWindow = NULL;
        m_pKeyboard = NULL;
        m_pMacMenu = NULL;
        m_pMouse = NULL;
        m_HScrollBar = NULL;
        m_VScrollBar = NULL;
	MacWindowInit ();
}

void XAP_MacFrame::MacWindowInit ()
{
	UT_ASSERT (XAP_MacApp::m_NotInitialized == false);
}


XAP_MacFrame::~XAP_MacFrame(void)
{
    if (m_HScrollBar) {
        ::DisposeControl (m_HScrollBar);
    }
    if (m_VScrollBar) {
        ::DisposeControl (m_VScrollBar);
    }
}

bool XAP_MacFrame::initialize(const char * szKeyBindingsKey, 
				  const char * szKeyBindingsDefaultValue,
				  const char * szMenuLayoutKey, 
				  const char * szMenuLayoutDefaultValue,
				  const char * szMenuLabelSetKey, 
				  const char * szMenuLabelSetDefaultValue,
				  const char * szToolbarLayoutsKey, 
				  const char * szToolbarLayoutsDefaultValue,
				  const char * szToolbarLabelSetKey, 
				  const char * szToolbarLabelSetDefaultValue) {
	bool bResult;

	// invoke our base class first.
	
	bResult = XAP_Frame::initialize(szKeyBindingsKey, 
					szKeyBindingsDefaultValue,
					szMenuLayoutKey, 
					szMenuLayoutDefaultValue,
					szMenuLabelSetKey, 
					szMenuLabelSetDefaultValue,
					szToolbarLayoutsKey, 
					szToolbarLayoutsDefaultValue,
					szToolbarLabelSetKey, 
					szToolbarLabelSetDefaultValue);
	UT_ASSERT(bResult);

	// get a handle to our keyboard binding mechanism
	// and to our mouse binding mechanism.
	EV_EditEventMapper * pEEM = getEditEventMapper();
	UT_ASSERT(pEEM);

	//These are actually "attached" in the ap_Frame code
	//since they require that all the beos classes be
	//instantiated.
	m_pKeyboard = new ev_MacKeyboard(pEEM);
	UT_ASSERT(m_pKeyboard);

	m_pMouse = new EV_MacMouse(pEEM);
	UT_ASSERT(m_pMouse);

	return true;
}

XAP_Frame *	XAP_MacFrame::cloneFrame(void)
{
	UT_ASSERT(UT_NOT_IMPLEMENTED);

	return 0;
}

bool	XAP_MacFrame::close(void)
{
	::DisposeWindow ((WindowPtr)m_MacWindow);
	return true;
}

bool	XAP_MacFrame::raise(void)
{
	::BringToFront ((WindowPtr)m_MacWindow);
	return true;
}

bool	XAP_MacFrame::show(void)
{
	::ShowWindow ((WindowPtr)m_MacWindow);
	return true;
}

bool XAP_MacFrame::openURL(const char * /*szURL*/)
{
	// TODO: use GURL or InternetConfig to open the specified URL
	UT_ASSERT(UT_NOT_IMPLEMENTED);

	return false;
}

bool	XAP_MacFrame::updateTitle(void)
{
	if (!XAP_Frame::updateTitle())
	{
		// no relevant change, so skip it
		return false;
	}

	char buf[256];
	buf[0] = 0;

	const char * szAppName = m_app->getApplicationTitleForTitleBar();

	int len = 256 - strlen(szAppName) - 4;
	
	const char * szTitle = getTitle(len);

	sprintf(buf, (char *)"\p%s - %s", szTitle, szAppName);
			
        UT_ASSERT (m_MacWindow);
        // TODO make conditionnal use of SetWindowTitleWithCFString when target is Carbon
        ::SetWTitle (m_MacWindow, (ConstStr255Param)buf);

	return true;
}

UT_sint32 XAP_MacFrame::setInputMode(const char * szName)
{
	UT_ASSERT (UT_NOT_IMPLEMENTED);
}


XAP_DialogFactory *XAP_MacFrame::getDialogFactory(void)
{
	return &m_dialogFactory;
}

void XAP_MacFrame::_createTopLevelWindow(void)
{
	::SetRect(&m_winBounds, 100, 100, 500, 500);
	m_MacWindow = ::NewWindow(NULL, &m_winBounds, "\pUntitled", 0, 0, (WindowPtr) -1, 0, (long) this);
	UT_ASSERT (m_MacWindow != NULL);
#if TARGET_API_MAC_CARBON
	m_MacWindowPort = ::GetWindowPort (m_MacWindow);
#else
	m_MacWindowPort = (GrafPtr)::GetWindowPort (m_MacWindow);
#endif
	::SetWindowKind (m_MacWindow, XAP_MACFRAME_WINDOW_KIND);
    ::SetWRefCon (m_MacWindow, (long)this);

#if TARGET_API_MAC_CARBON
    ::GetWindowPortBounds (m_MacWindow, &m_winBounds);
#else
	m_winBounds = m_MacWindowPort->portRect;
#endif
    m_pMacMenu = new EV_MacMenu (dynamic_cast<XAP_MacApp*>(m_app), this,
				      m_szMenuLayoutName, m_szMenuLabelSetName);
    m_pMacMenu->synthesizeMenuBar();


    _createToolbars ();
    _createDocumentWindow();
//        m_pMacStatusBarView = _createStatusBarWindow ();
}

void XAP_MacFrame::_createToolbars(void)
{
        Rect rect;
       	ThemeDrawState			drawState;

        // Get theme state
        drawState = ::IsWindowHilited (m_MacWindow) ?
                        (ThemeDrawState)kThemeStateActive :
                        (ThemeDrawState)kThemeStateDisabled;
        // Draw the window header where toolbar reside
        rect = m_winBounds;
        ::InsetRect( &rect, -1, -1 );
        rect.bottom = rect.top + 40;
        ::DrawThemeWindowHeader (&rect, drawState);
        
        // TODO: place the buttons
        // TODO 2: make sure it is redrawn during refresh
}


void XAP_MacFrame::_createDocumentWindow (void)
{
	Rect rect;
        
    // create HScrollbar
    _calcVertScrollBarRect (rect);
#if UNIVERSAL_INTERFACE_VERSION >= 0x0335
	::CreateScrollBarControl (m_MacWindow, &rect, 0, 0, 100, 0, false, nil, &m_VScrollBar );
#else
	::SetRect (&rect,  m_winBounds.right - 16, 0,  m_winBounds.right, m_winBounds.bottom - 16);
	::NewControl (m_MacWindow, &rect, "\p", true, 0, 0, 100, kControlScrollBarProc, 0);
#endif

    // create VScrollbar
	_calcHorizScrollBarRect (rect);
#if UNIVERSAL_INTERFACE_VERSION >= 0x0335
	::CreateScrollBarControl (m_MacWindow, &rect, 0, 0, 100, 0, false, nil, &m_HScrollBar );
#else
	::SetRect (&rect, 0, m_winBounds.bottom - 16, m_winBounds.right, m_winBounds.bottom);
	::NewControl (m_MacWindow, &rect, "\p", true, 0, 0, 100, kControlScrollBarProc, 0);
#endif
        
    // TODO: make the placard OR the status bar. Status bar will be better IMHO.
}


void XAP_MacFrame::_calcVertScrollBarRect (Rect & rect)
{
#if TARGET_API_MAC_CARBON
    ::GetPortBounds (m_MacWindowPort, &rect);
#else
	rect = m_MacWindowPort->portRect;
#endif
    rect.right++;
	rect.left = rect.right - 16;
	rect.bottom -= 14;
	rect.top = 38;
}


void XAP_MacFrame::_calcHorizScrollBarRect (Rect & rect)
{
#if TARGET_API_MAC_CARBON
    ::GetPortBounds (m_MacWindowPort, &rect);
#else
	rect = m_MacWindowPort->portRect;
#endif
	rect.bottom++;
	rect.left = rect.left + 120;
	rect.top = rect.bottom - 16;
	rect.right -= 14;
}


void XAP_MacFrame::setXScrollRange(void)
{
        // TODO: now that we have scroll, implement these.
	UT_ASSERT(UT_NOT_IMPLEMENTED);
}

void XAP_MacFrame::setYScrollRange(void)
{
        // TODO: now that we have scroll, implement these.
	UT_ASSERT(UT_NOT_IMPLEMENTED);
}

bool XAP_MacFrame::runModalContextMenu(AV_View * /* pView */, const char * /*szMenuName*/, UT_sint32 /*x*/, UT_sint32 /*y*/) {
	UT_ASSERT(UT_NOT_IMPLEMENTED);

	return(false);
}
