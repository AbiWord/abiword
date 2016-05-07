/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode:t -*- */

/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001-2016 Hubert Figuiere
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

#import <Cocoa/Cocoa.h>

#include "ut_assert.h"
#include "xap_CocoaDlg_MessageBox.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"

#include "xap_CocoaDialog_Utilities.h"


/*****************************************************************/
XAP_Dialog * XAP_CocoaDialog_MessageBox::static_constructor(XAP_DialogFactory * pFactory,
														 XAP_Dialog_Id dlgid)
{
	XAP_CocoaDialog_MessageBox * p = new XAP_CocoaDialog_MessageBox(pFactory, dlgid);
	return p;
}

XAP_CocoaDialog_MessageBox::XAP_CocoaDialog_MessageBox(XAP_DialogFactory * pDlgFactory,
												   XAP_Dialog_Id dlgid)
	: XAP_Dialog_MessageBox(pDlgFactory, dlgid)
{
}

XAP_CocoaDialog_MessageBox::~XAP_CocoaDialog_MessageBox(void)
{
}

/*****************************************************************/

void XAP_CocoaDialog_MessageBox::runModal(XAP_Frame * /*pFrame*/)
{
	XAP_App * pApp = XAP_App::getApp();

	const XAP_StringSet * pSS = pApp->getStringSet();
	
	NSString * title   = [NSString stringWithUTF8String:m_szMessage];
	const char* message = m_szSecondaryMessage ? m_szSecondaryMessage : "";

	switch (m_buttons) {
	case b_O:
		NSRunAlertPanel(title, @"%s",
						LocalizedString(pSS, XAP_STRING_ID_DLG_OK), nil, nil, 
						message);

		m_answer = a_OK;
		break;

	case b_OC:
		if (m_defaultAnswer == XAP_Dialog_MessageBox::a_OK)	{
			int btn = NSRunAlertPanel(title, @"%s",
									  LocalizedString(pSS, XAP_STRING_ID_DLG_OK),
									  LocalizedString(pSS, XAP_STRING_ID_DLG_Cancel),
									  nil, 
									  message);

			switch (btn) {
			case NSAlertDefaultReturn:
				m_answer = a_OK;
				break;
			case NSAlertAlternateReturn:
				m_answer = a_CANCEL;
				break;
			default:
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
				break;
			}
		} else {
			int btn = NSRunAlertPanel(title, @"%s",
									  LocalizedString(pSS, XAP_STRING_ID_DLG_Cancel),
									  LocalizedString(pSS, XAP_STRING_ID_DLG_OK),
									  nil, 
									  message);

			switch (btn) {
			case NSAlertDefaultReturn:
				m_answer = a_CANCEL;
				break;
			case NSAlertAlternateReturn:
				m_answer = a_OK;
				break;
			default:
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
				break;
			}
		}
		break;

	case b_YN:
		if (m_defaultAnswer == XAP_Dialog_MessageBox::a_YES) {
			int btn = NSRunAlertPanel(title, @"%s",
									  LocalizedString(pSS, XAP_STRING_ID_DLG_MB_Yes),
									  LocalizedString(pSS, XAP_STRING_ID_DLG_MB_No), 
									  nil, 
									  message);
			switch (btn) {
			case NSAlertDefaultReturn:
				m_answer = a_YES;
				break;
			case NSAlertAlternateReturn:
				m_answer = a_NO;
				break;
			default:
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
				break;
			}
		} else {
			int btn = NSRunAlertPanel(title, @"%s",
									  LocalizedString(pSS, XAP_STRING_ID_DLG_MB_No), 
									  LocalizedString(pSS, XAP_STRING_ID_DLG_MB_Yes),
									  nil, 
									  message);
			switch (btn) {
			case NSAlertDefaultReturn:
				m_answer = a_NO;
				break;
			case NSAlertAlternateReturn:
				m_answer = a_YES;
				break;
			default:
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
				break;
			}
		}
		break;

	case b_YNC:
		if (m_defaultAnswer == XAP_Dialog_MessageBox::a_YES) {
			int btn = NSRunAlertPanel(title, @"%s",
									  LocalizedString(pSS, XAP_STRING_ID_DLG_MB_Yes),
									  LocalizedString(pSS, XAP_STRING_ID_DLG_MB_No), 
									  LocalizedString(pSS, XAP_STRING_ID_DLG_Cancel),
									  message);

			switch (btn) {
			case NSAlertDefaultReturn:
				m_answer = a_YES;
				break;
			case NSAlertAlternateReturn:
				m_answer = a_NO;
				break;
			case NSAlertOtherReturn:
				m_answer = a_CANCEL;
				break;
			default:
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
				break;
			}
		} else {
			int btn = NSRunAlertPanel(title, @"%s",
									  LocalizedString(pSS, XAP_STRING_ID_DLG_Cancel),
									  LocalizedString(pSS, XAP_STRING_ID_DLG_MB_No), 
									  LocalizedString(pSS, XAP_STRING_ID_DLG_MB_Yes), 
									  message);
			switch (btn) {
			case NSAlertDefaultReturn:
				m_answer = a_CANCEL;
				break;
			case NSAlertAlternateReturn:
				m_answer = a_NO;
				break;
			case NSAlertOtherReturn:
				m_answer = a_YES;
				break;
			default:
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
				break;
			}
		}
		break;

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}
}

