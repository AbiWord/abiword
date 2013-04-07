/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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
#include <time.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_CocoaDialog_Utilities.h"

#include "xap_App.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Insert_DateTime.h"
#include "ap_CocoaDialog_Insert_DateTime.h"

/*****************************************************************/

class XAP_Date_Proxy: public XAP_GenericListChooser_Proxy
{
public:
	XAP_Date_Proxy (AP_CocoaDialog_Insert_DateTime* dlg)
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
	AP_CocoaDialog_Insert_DateTime*	m_dlg;
};
/*****************************************************************/

XAP_Dialog * AP_CocoaDialog_Insert_DateTime::static_constructor(XAP_DialogFactory * pFactory,
															   XAP_Dialog_Id dlgid)
{
	AP_CocoaDialog_Insert_DateTime * p = new AP_CocoaDialog_Insert_DateTime(pFactory, dlgid);
	return p;
}

AP_CocoaDialog_Insert_DateTime::AP_CocoaDialog_Insert_DateTime(XAP_DialogFactory * pDlgFactory,
															 XAP_Dialog_Id dlgid)
	: AP_Dialog_Insert_DateTime(pDlgFactory,dlgid),
		m_dlg(nil)
{
}

AP_CocoaDialog_Insert_DateTime::~AP_CocoaDialog_Insert_DateTime(void)
{
}


/*****************************************************************/

void AP_CocoaDialog_Insert_DateTime::runModal(XAP_Frame * /*pFrame*/)
{
	NSWindow* window;
	m_dlg = [XAP_GenericListChooser_Controller loadFromNib];
	XAP_Date_Proxy proxy(this);
	[m_dlg setXAPProxy:&proxy];
	m_dataSource = [[XAP_StringListDataSource alloc] init];

	window = [m_dlg window];
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	[m_dlg setTitle:LocalizedString(pSS, AP_STRING_ID_DLG_DateTime_DateTimeTitle)];
	[m_dlg setLabel:LocalizedString(pSS, AP_STRING_ID_DLG_DateTime_AvailableFormats)];

	// Populate the window's data items
	_populateWindowData();
	[m_dlg setDataSource:m_dataSource];
	

	[NSApp runModalForWindow:window];
	[m_dlg close];

	[m_dataSource release];

	m_dlg = nil;	
}

void AP_CocoaDialog_Insert_DateTime::event_OK(void)
{
	// Query the list for its selection.
	int row = [m_dlg selected];

	m_iFormatIndex = row;
	
	m_answer = AP_Dialog_Insert_DateTime::a_OK;
	[NSApp stopModal];
}

void AP_CocoaDialog_Insert_DateTime::event_Cancel(void)
{
	m_answer = AP_Dialog_Insert_DateTime::a_CANCEL;
	[NSApp stopModal];
}


void AP_CocoaDialog_Insert_DateTime::_populateWindowData(void)
{
	// NOTE : this code is similar to the Windows dialog code to do
	// NOTE : the same thing.  if you are implementing this dialog
	// NOTE : for a new front end, this is the formatting logic 
	// NOTE : you'll want to use to populate your list
	
	int i;

	// this constant comes from ap_Dialog_Insert_DateTime.h
    char szCurrentDateTime[CURRENT_DATE_TIME_SIZE];
	
    time_t tim = time(NULL);
	
    struct tm *pTime = localtime(&tim);

    for (i = 0; InsertDateTimeFmts[i] != NULL; i++)
	{
        strftime(szCurrentDateTime, CURRENT_DATE_TIME_SIZE, InsertDateTimeFmts[i], pTime);

		[m_dataSource addString:[NSString stringWithUTF8String:szCurrentDateTime]];
	}

	[m_dlg setSelected:0];
}

