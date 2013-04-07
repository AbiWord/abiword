/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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

#include <stdlib.h>
#include <time.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_CocoaDialog_Utilities.h"

#include "xap_App.h"
#include "xap_CocoaApp.h"
#include "xap_Frame.h"

#include "xap_Strings.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_ListDocuments.h"
#include "xap_CocoaDlg_ListDocuments.h"
#import "xap_Cocoa_NSTableUtils.h"
#import "xap_GenericListChooser_Controller.h"

/*****************************************************************/

class XAP_DocList_Proxy: public XAP_GenericListChooser_Proxy
{
public:
	XAP_DocList_Proxy (XAP_CocoaDialog_ListDocuments* dlg)
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
		m_dlg->event_OK();
	};
private:
	XAP_CocoaDialog_ListDocuments*	m_dlg;
};


/*****************************************************************/

XAP_Dialog * XAP_CocoaDialog_ListDocuments::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id dlgid)
{
	XAP_CocoaDialog_ListDocuments * p = new XAP_CocoaDialog_ListDocuments(pFactory,dlgid);
	return p;
}

XAP_CocoaDialog_ListDocuments::XAP_CocoaDialog_ListDocuments(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id dlgid)
	: XAP_Dialog_ListDocuments(pDlgFactory,dlgid)
{
}

XAP_CocoaDialog_ListDocuments::~XAP_CocoaDialog_ListDocuments(void)
{
}

void XAP_CocoaDialog_ListDocuments::event_OK(void)
{
	_setSelDocumentIndx([m_dlg selected]);
	[NSApp stopModal];
}


void XAP_CocoaDialog_ListDocuments::event_Cancel(void)
{
	[NSApp stopModal];
}


void XAP_CocoaDialog_ListDocuments::_populateWindowData(void)
{
	const char * untitled = "(untitled)";

	UT_uint32 c = _getDocumentCount();
	UT_uint32 i;
	
	[m_dataSource removeAllStrings];
	for (i = 0; i < c; i++)
	{
		const char * name = _getNthDocumentName(i);
		[m_dataSource addString:[NSString stringWithUTF8String:(name ? name : untitled)]];
	}
}


void XAP_CocoaDialog_ListDocuments::runModal(XAP_Frame * /*pFrame*/)
{
	NSWindow* window;
	m_dlg = [XAP_GenericListChooser_Controller loadFromNib];
	XAP_DocList_Proxy proxy(this);
	[m_dlg setXAPProxy:&proxy];
	m_dataSource = [[XAP_StringListDataSource alloc] init];

	window = [m_dlg window];
//	const XAP_StringSet * pSS = m_pApp->getStringSet();
	[m_dlg setTitle:[NSString stringWithUTF8String:_getTitle()]];
	[m_dlg setLabel:[NSString stringWithUTF8String:_getHeading()]];

	// Populate the window's data items
	_populateWindowData();
	[m_dlg setDataSource:m_dataSource];
	

	[NSApp runModalForWindow:window];
	[m_dlg close];

	[m_dataSource release];

	m_dlg = nil;	
}
