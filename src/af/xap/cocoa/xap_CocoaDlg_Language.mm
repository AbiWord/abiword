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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */
/* $Id */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "xap_CocoaDialog_Utilities.h"
#include "xap_CocoaDlg_Language.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"
#include "gr_CocoaGraphics.h"


class XAP_LanguageList_Proxy: public XAP_GenericListChooser_Proxy
{
public:
	XAP_LanguageList_Proxy (XAP_CocoaDialog_Language* dlg)
		: XAP_GenericListChooser_Proxy(),
			m_dlg(dlg)
	{
	};
	virtual void okAction ()
	{
		m_dlg->okAction();
	};
	virtual void cancelAction ()
	{
		m_dlg->cancelAction();
	};
	virtual void selectAction ()
	{
		m_dlg->okAction();
	};
private:
	XAP_CocoaDialog_Language*	m_dlg;
};


/*****************************************************************/
XAP_Dialog * XAP_CocoaDialog_Language::static_constructor(XAP_DialogFactory * pFactory,
														 XAP_Dialog_Id dlgid)
{
	XAP_CocoaDialog_Language * p = new XAP_CocoaDialog_Language(pFactory,dlgid);
	return p;
}

XAP_CocoaDialog_Language::XAP_CocoaDialog_Language(XAP_DialogFactory * pDlgFactory,
												   XAP_Dialog_Id dlgid)
	: XAP_Dialog_Language(pDlgFactory,dlgid),
		m_dataSource(nil),
		m_dlg(nil)
{
}

XAP_CocoaDialog_Language::~XAP_CocoaDialog_Language(void)
{
}


void XAP_CocoaDialog_Language::okAction(void)
{	
	m_answer = XAP_Dialog_Language::a_OK;
	[NSApp stopModal];
}


void XAP_CocoaDialog_Language::cancelAction(void)
{
	m_answer = XAP_Dialog_Language::a_CANCEL;
	[NSApp stopModal];
}


void XAP_CocoaDialog_Language::runModal(XAP_Frame * pFrame)
{
	NSWindow* window;
	m_dlg = [XAP_GenericListChooser_Controller loadFromNib];
	UT_ASSERT(m_pApp);
	XAP_LanguageList_Proxy proxy(this);
	[m_dlg setXAPProxy:&proxy];
	m_dataSource = [[XAP_StringListDataSource alloc] init];
	
	window = [m_dlg window];
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	[m_dlg setTitle:LocalizedString(pSS, XAP_STRING_ID_DLG_ULANG_LangTitle)];
	[m_dlg setLabel:LocalizedString(pSS, XAP_STRING_ID_DLG_ULANG_LangLabel)];

	for (UT_uint32 k = 0; k < m_iLangCount; k++)
	{
		[m_dataSource addString:[NSString stringWithUTF8String:m_ppLanguages[k]]];
	}
	[m_dlg setDataSource:m_dataSource];
	
	// Set the defaults in the list boxes according to dialog data
	int foundAt = 0;

	// is this safe with an XML_Char * string?
	foundAt = [m_dataSource rowWithCString:m_pLanguage];
	if (foundAt >= 0)
	{
		[m_dlg setSelected:foundAt];
	}
	
	[NSApp runModalForWindow:window];

	if (m_answer == XAP_Dialog_Language::a_OK)
	{
		int row = [m_dlg selected];
		NSString* str = [[m_dataSource array] objectAtIndex:row];
		const char* 	utf8str = [str UTF8String];
		if (!m_pLanguage || UT_stricmp(m_pLanguage, utf8str))
		{
			_setLanguage((XML_Char*)utf8str);
			m_bChangedLanguage = true;
		}
	}
	[m_dlg close];

	[m_dataSource release];

	m_dlg = nil;
}

