/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (c) 2003 Hubert Figuiere
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

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for Cocoa dialogs,
// like centering them, measuring them, etc.
#include "xap_CocoaDialog_Utilities.h"

#include "xap_App.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"

#include "xap_Dialog_Id.h"
#include "xap_Dlg_WindowMore.h"
#include "xap_CocoaDlg_WindowMore.h"


class XAP_WindowMore_Proxy: public XAP_GenericListChooser_Proxy
{
public:
	XAP_WindowMore_Proxy (XAP_CocoaDialog_WindowMore* dlg)
		: XAP_GenericListChooser_Proxy(),
			m_dlg(dlg)
	{
	};
	virtual void okAction ()
	{
		m_dlg->event_OK();
	};
	virtual void cancelAction ()
	{
		m_dlg->event_Cancel();
	};
	virtual void selectAction ()
	{
		m_dlg->event_DoubleClick();
	};
private:
	XAP_CocoaDialog_WindowMore*	m_dlg;
};

/*****************************************************************/

XAP_Dialog * XAP_CocoaDialog_WindowMore::static_constructor(XAP_DialogFactory * pFactory,
													   XAP_Dialog_Id dlgid)
{
	XAP_CocoaDialog_WindowMore * p = new XAP_CocoaDialog_WindowMore(pFactory,dlgid);
	return p;
}

XAP_CocoaDialog_WindowMore::XAP_CocoaDialog_WindowMore(XAP_DialogFactory * pDlgFactory,
											   XAP_Dialog_Id dlgid)
	: XAP_Dialog_WindowMore(pDlgFactory,dlgid),
		m_dataSource(nil),
		m_dlg(nil)
{
}

XAP_CocoaDialog_WindowMore::~XAP_CocoaDialog_WindowMore(void)
{
}


/*****************************************************************/

void XAP_CocoaDialog_WindowMore::runModal(XAP_Frame * pFrame)
{
	// Initialize member so we know where we are now
	m_ndxSelFrame = m_pApp->findFrame(pFrame);
	UT_ASSERT(m_ndxSelFrame >= 0);

	NSWindow* window;
	m_dlg = [XAP_GenericListChooser_Controller loadFromNib];
	XAP_WindowMore_Proxy proxy(this);
	[m_dlg setXAPProxy:&proxy];
	m_dataSource = [[XAP_StringListDataSource alloc] init];
	
	window = [m_dlg window];
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	std::string label;
	pSS->getValueUTF8(XAP_STRING_ID_DLG_MW_MoreWindows, label);
	[m_dlg setTitle:[NSString stringWithUTF8String:label.c_str()]];
	pSS->getValueUTF8(XAP_STRING_ID_DLG_MW_Activate, label);
	[m_dlg setLabel:[NSString stringWithUTF8String:label.c_str()]];

	// Populate the window's data items
	_populateWindowData();
	[m_dlg setDataSource:m_dataSource];
	

	[NSApp runModalForWindow:window];
	[m_dlg close];

	[m_dataSource release];

	m_dlg = nil;	
}

void XAP_CocoaDialog_WindowMore::event_OK(void)
{
	// Query the list for its selection.
	int row = [m_dlg selected];

	if (row >= 0)
		m_ndxSelFrame = (UT_uint32) row;
	
	m_answer = XAP_Dialog_WindowMore::a_OK;
	[NSApp stopModal];
}

void XAP_CocoaDialog_WindowMore::event_Cancel(void)
{
	m_answer = XAP_Dialog_WindowMore::a_CANCEL;
	[NSApp stopModal];
}

void XAP_CocoaDialog_WindowMore::event_DoubleClick(void)
{
	// Query the list for its selection.	
	int row = [m_dlg selected];

	// If it found something, return with it
	if (row >= 0)
	{
		m_ndxSelFrame = (UT_uint32) row;
		event_OK();
	}
}



void XAP_CocoaDialog_WindowMore::_populateWindowData(void)
{
	// We just do one thing here, which is fill the list with
	// all the windows.

	for (UT_sint32 i = 0; i < m_pApp->getFrameCount(); i++)
	{
		XAP_Frame * f = m_pApp->getFrame(i);
		UT_ASSERT(f);
		UT_UTF8String s = f->getTitle();
		
		[m_dataSource addString:[NSString stringWithUTF8String:s.utf8_str()]];
	} 

	// Select the one we're in
	[m_dlg setSelected:m_ndxSelFrame];
}

