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

#define REPLACEP(p,q)	do { if (p) delete p; p = q; } while (0)
#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

/*****************************************************************/

XAP_MacFrame::XAP_MacFrame(XAP_MacApp * app)
	: XAP_Frame(static_cast<XAP_App *>(app))
{
	SetRect(&theBounds, 100, 100, 500, 500);
	theWP = NewWindow(0, &theBounds, "\pUntitled", 0, 0, (GrafPtr) -1, 0, (long) this);
}

// TODO when cloning a new frame from an existing one
// TODO should we also clone any frame-persistent
// TODO dialog data ??

XAP_MacFrame::XAP_MacFrame(XAP_MacFrame * f)
	: XAP_Frame(static_cast<XAP_Frame *>(f))
{
	SetRect(&theBounds, 100, 100, 500, 500);
	theWP = NewWindow(0, &theBounds, "\pUntitled", 0, 0, (GrafPtr) -1, 0, (long) this);
}

XAP_MacFrame::~XAP_MacFrame(void)
{
}

XAP_Frame *	XAP_MacFrame::cloneFrame(void)
{
	return 0;
}

UT_Bool	XAP_MacFrame::loadDocument(const char * szFilename)
{
	return UT_TRUE;
}

UT_Bool	XAP_MacFrame::close(void)
{
	return UT_TRUE;
}

UT_Bool	XAP_MacFrame::raise(void)
{
	return UT_TRUE;
}

UT_Bool	XAP_MacFrame::show(void)
{
	return UT_TRUE;
}

AP_DialogFactory *XAP_MacFrame::getDialogFactory(void)
{
	return 0;
}

void XAP_MacFrame::setXScrollRange(void)
{
}

void XAP_MacFrame::setYScrollRange(void)
{
}
