/* AbiSource Application Framework
 * Copyright (C) 1998-2001 AbiSource, Inc.
 * Copyright (C) 2003 Hubert Figuiere
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
#include <stdio.h>
#include <string.h>
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "xap_CocoaDialog_Utilities.h"

#include "xap_App.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"

#include "xap_Dialog_Id.h"
#include "xap_CocoaDlg_Encoding.h"


class XAP_Encoding_Proxy: public XAP_GenericListChooser_Proxy
{
public:
	XAP_Encoding_Proxy (XAP_CocoaDialog_Encoding* dlg)
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
	XAP_CocoaDialog_Encoding*	m_dlg;
};

/*****************************************************************/

XAP_Dialog * XAP_CocoaDialog_Encoding::static_constructor(XAP_DialogFactory * pFactory,
													   XAP_Dialog_Id dlgid)
{
	XAP_CocoaDialog_Encoding * p = new XAP_CocoaDialog_Encoding(pFactory,dlgid);
	return p;
}

XAP_CocoaDialog_Encoding::XAP_CocoaDialog_Encoding(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid)
	: XAP_Dialog_Encoding(pDlgFactory,dlgid),
		m_dataSource(nil),
		m_dlg(nil)
{

}

XAP_CocoaDialog_Encoding::~XAP_CocoaDialog_Encoding(void)
{
}


/*****************************************************************/

void XAP_CocoaDialog_Encoding::runModal(XAP_Frame * /*pFrame*/)
{

	NSWindow* window;
	m_dlg = [XAP_GenericListChooser_Controller loadFromNib];
	XAP_Encoding_Proxy proxy(this);
	[m_dlg setXAPProxy:&proxy];
	m_dataSource = [[XAP_StringListDataSource alloc] init];
	
	window = [m_dlg window];
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	[m_dlg setTitle:LocalizedString(pSS, XAP_STRING_ID_DLG_UENC_EncTitle)];
	[m_dlg setLabel:LocalizedString(pSS, XAP_STRING_ID_DLG_UENC_EncLabel)];

	// Populate the window's data items
	_populateWindowData();
	[m_dlg setDataSource:m_dataSource];
	

	[NSApp runModalForWindow:window];
	[m_dlg close];

	[m_dataSource release];

	m_dlg = nil;	
}

void XAP_CocoaDialog_Encoding::event_OK(void)
{
	// Query the list for its selection.
	int row = [m_dlg selected];

	if (row >= 0)
		_setSelectionIndex((UT_uint32) row);
	
	_setEncoding (_getAllEncodings()[row]);
	_setAnswer (XAP_Dialog_Encoding::a_OK);
	[NSApp stopModal];
}

void XAP_CocoaDialog_Encoding::event_Cancel(void)
{
	_setAnswer (XAP_Dialog_Encoding::a_CANCEL);
	[NSApp stopModal];
}

/*****************************************************************/


void XAP_CocoaDialog_Encoding::_populateWindowData(void)
{
	// We just do one thing here, which is fill the list with
	// all the windows.

	for (UT_uint32 i = 0; i < _getEncodingsCount(); i++)
	{
		const gchar* s = _getAllEncodings()[i];

		[m_dataSource addString:[NSString stringWithUTF8String:s]];
	} 

	// Select the one we're in
	[m_dlg setSelected:_getSelectionIndex()];
}



