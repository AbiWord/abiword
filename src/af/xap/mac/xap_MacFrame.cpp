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

#include <QuickDraw.h>
#include <MacWindows.h>

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
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
	: XAP_Frame(static_cast<XAP_App *>(app))
{
	m_MacWindow = NULL;
	MacWindowInit ();
}

// TODO when cloning a new frame from an existing one
// TODO should we also clone any frame-persistent
// TODO dialog data ??

XAP_MacFrame::XAP_MacFrame(XAP_MacFrame * f)
	: XAP_Frame(static_cast<XAP_Frame *>(f))
{
	MacWindowInit ();
}

void XAP_MacFrame::MacWindowInit ()
{
	UT_ASSERT (XAP_MacApp::m_NotInitialized == false);
	
	::SetRect(&theBounds, 100, 100, 500, 500);
	m_MacWindow = ::NewWindow(NULL, &theBounds, "\pUntitled", 0, 0, (WindowPtr) -1, 0, (long) this);
	UT_ASSERT (m_MacWindow != NULL);

#ifdef XP_MAC_TARGET_CARBON
	::SetWindowKind (m_MacWindow, XAP_MACFRAME_WINDOW_KIND);
#else
	((WindowPeek)m_MacWindow)->windowKind = XAP_MACFRAME_WINDOW_KIND;
#endif
}


XAP_MacFrame::~XAP_MacFrame(void)
{
}

XAP_Frame *	XAP_MacFrame::cloneFrame(void)
{
	UT_ASSERT(UT_NOT_IMPLEMENTED);

	return 0;
}

UT_Bool	XAP_MacFrame::close(void)
{
	::DisposeWindow ((WindowPtr)m_MacWindow);
	return UT_TRUE;
}

UT_Bool	XAP_MacFrame::raise(void)
{
	::BringToFront ((WindowPtr)m_MacWindow);
	return UT_TRUE;
}

UT_Bool	XAP_MacFrame::show(void)
{
	::ShowWindow ((WindowPtr)m_MacWindow);
	return UT_TRUE;
}

UT_Bool XAP_MacFrame::openURL(const char * /*szURL*/)
{
	// TODO: use GURL or InternetConfig to open the specified URL
	UT_ASSERT(UT_NOT_IMPLEMENTED);

	return UT_FALSE;
}

XAP_DialogFactory *XAP_MacFrame::getDialogFactory(void)
{
	UT_ASSERT(UT_NOT_IMPLEMENTED);

	return 0;
}

void XAP_MacFrame::_createTopLevelWindow(void)
{
	UT_ASSERT(UT_NOT_IMPLEMENTED);
}


void XAP_MacFrame::setXScrollRange(void)
{
	UT_ASSERT(UT_NOT_IMPLEMENTED);
}

void XAP_MacFrame::setYScrollRange(void)
{
	UT_ASSERT(UT_NOT_IMPLEMENTED);
}

UT_Bool XAP_MacFrame::runModalContextMenu(AV_View * /* pView */, const char * /*szMenuName*/, UT_sint32 /*x*/, UT_sint32 /*y*/) {
	return(UT_FALSE);
}
