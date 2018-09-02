/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2005 Hubert Figuiere
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

#include <string.h>

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string_class.h"

#include "xap_Dialog.h"
#include "xap_DialogFactory.h"
#include "xap_App.h"
#include "xap_Frame.h"
#include "xap_Widget.h"

/*****************************************************************/

XAP_Dialog::XAP_Dialog(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id,
		       const char * helpUrl )
  : m_pApp ( NULL ), m_pDlgFactory ( pDlgFactory ), m_id ( id )
{
  m_pApp = pDlgFactory->getApp();

  if (helpUrl)
    m_helpUrl = helpUrl;
}

XAP_Dialog::~XAP_Dialog(void)
{
}


int XAP_Dialog::getWidgetValueInt(xap_widget_id wid)
{
	XAP_Widget *w =	getWidget(wid);
	int retval = w->getValueInt();
	delete w;
	return retval;
}


void XAP_Dialog::setWidgetValueInt(xap_widget_id wid, int value)
{
	XAP_Widget *w = getWidget(wid);
	w->setValueInt(value);
	delete w;
}

void XAP_Dialog::setWidgetLabel(xap_widget_id wid, const UT_UTF8String &val)
{
	XAP_Widget *w = getWidget(wid);
	w->setLabel(val);
	delete w;
}

void XAP_Dialog::setWidgetLabel(xap_widget_id wid, const std::string &val)
{
	XAP_Widget *w = getWidget(wid);
	w->setLabel(val);
	delete w;
}

void
XAP_Dialog::maybeClosePopupPreviewBubbles()
{    
}

void
XAP_Dialog::maybeReallowPopupPreviewBubbles()
{
}



/*****************************************************************/

XAP_Dialog_NonPersistent::XAP_Dialog_NonPersistent(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id, const char * helpUrl )
	: XAP_Dialog(pDlgFactory,id, helpUrl)
{
}

XAP_Dialog_NonPersistent::~XAP_Dialog_NonPersistent(void)
{
}



/*****************************************************************/

XAP_TabbedDialog_NonPersistent::XAP_TabbedDialog_NonPersistent(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id, const char * helpUrl )
	: XAP_Dialog_NonPersistent(pDlgFactory,id, helpUrl),
	  m_pageNum(-1)
{
}

XAP_TabbedDialog_NonPersistent::~XAP_TabbedDialog_NonPersistent(void)
{
}

/*****************************************************************/

XAP_Dialog_Persistent::XAP_Dialog_Persistent(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id, const char * helpUrl)
	: XAP_Dialog(pDlgFactory,id, helpUrl),
	  m_bInUse(false)
{
}

XAP_Dialog_Persistent::~XAP_Dialog_Persistent(void)
{
}

void XAP_Dialog_Persistent::useStart(void)
{
	UT_ASSERT_HARMLESS(!m_bInUse);
	m_bInUse = true;
}

void XAP_Dialog_Persistent::useEnd(void)
{
	UT_ASSERT_HARMLESS(m_bInUse);
	m_bInUse = false;
}

/*****************************************************************/

XAP_Dialog_FramePersistent::XAP_Dialog_FramePersistent(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id, const char * helpUrl)
	: XAP_Dialog_Persistent(pDlgFactory,id, helpUrl)
{
}

XAP_Dialog_FramePersistent::~XAP_Dialog_FramePersistent(void)
{
}

void XAP_Dialog_FramePersistent::useStart(void)
{
	XAP_Dialog_Persistent::useStart();
}

void XAP_Dialog_FramePersistent::useEnd(void)
{
	XAP_Dialog_Persistent::useEnd();
}

/*****************************************************************/

XAP_Dialog_AppPersistent::XAP_Dialog_AppPersistent(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id, const char * helpUrl)
	: XAP_Dialog_Persistent(pDlgFactory,id, helpUrl)
{
}

XAP_Dialog_AppPersistent::~XAP_Dialog_AppPersistent(void)
{
}

void XAP_Dialog_AppPersistent::useStart(void)
{
	XAP_Dialog_Persistent::useStart();
}

void XAP_Dialog_AppPersistent::useEnd(void)
{
	XAP_Dialog_Persistent::useEnd();
}



XAP_Dialog_Modeless::XAP_Dialog_Modeless(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id, const char * helpUrl)
	: XAP_Dialog_AppPersistent(pDlgFactory,id, helpUrl)
{
	UT_return_if_fail(pDlgFactory);

	m_pDlgFactory = pDlgFactory;
	m_id = id;
	m_pApp = pDlgFactory->getApp();
        m_pDialog = (XAP_Dialog_Modeless *) this;

	UT_ASSERT_HARMLESS(m_pApp);
}

XAP_Dialog_Modeless::~XAP_Dialog_Modeless(void)
{
}

void XAP_Dialog_Modeless::useStart(void)
{
  xxx_UT_DEBUGMSG(("Called generic useStart in XAP_DIALOG_Modeless \n"));
}

void XAP_Dialog_Modeless::useEnd(void)
{
  xxx_UT_DEBUGMSG(("Called generic useEnd in XAP_DIALOG_Modeless \n"));
}

void XAP_Dialog_Modeless::modeless_cleanup(void)
{
	UT_sint32 sid = (UT_sint32) getDialogId();
	m_pApp->forgetModelessId( (UT_sint32) sid);
	m_pDlgFactory->releaseDialog(m_pDialog);
    maybeReallowPopupPreviewBubbles();
}

bool XAP_Dialog_Modeless::isRunning(void) const
{
 
	UT_sint32 sid = (UT_sint32) getDialogId();
	return m_pApp->isModelessRunning(sid);
}

XAP_Frame *   XAP_Dialog_Modeless::getActiveFrame(void) const
{
	// This function returns the frame currently connected to a modeless dialog

	XAP_Frame * pFrame = m_pApp->getLastFocussedFrame();
	if(pFrame == (XAP_Frame *) NULL)
	{
		pFrame = m_pApp->getFrame(0);
	}
	return pFrame;
}


void XAP_Dialog_Modeless::setActiveFrame(XAP_Frame *pFrame)
{
	notifyActiveFrame(pFrame);
}


std::string
XAP_Dialog_Modeless::BuildWindowName( const char * pDialogName ) const
{
    const UT_uint32 width = 100;
 	char pWindowName[width];
    
    BuildWindowName( pWindowName, pDialogName, width );
    std::string ret = pWindowName;
    return ret;
}


void
XAP_Dialog_Modeless::BuildWindowName(char * pWindowName, const char * pDialogName, UT_uint32 width ) const
{
// This function constructs and returns the window name of a modeless dialog by
// concatenating the active frame with the dialog name

	*pWindowName = (char) NULL;
	UT_UTF8String wn = UT_UTF8String(pDialogName);

	XAP_Frame* pFrame = getActiveFrame();
	if (pFrame)
	{
		wn += " - ";
		wn += pFrame->getTitle();
	}

	UT_uint32 len = UT_MIN(wn.byteLength(), width);
	strncpy(pWindowName, wn.utf8_str(), len);
	pWindowName[len] = '\0';
}


XAP_NotebookDialog::Page::Page(const gchar *_title, AbiNativeWidget * _widget) 
{
	title = g_strdup(_title);
	widget = _widget;
}

XAP_NotebookDialog::Page::~Page() 
{
	g_free(title);
}
