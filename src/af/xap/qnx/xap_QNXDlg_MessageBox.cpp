/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
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

#include "ut_assert.h"
#include "ut_vector.h"
#include "xap_QNXDlg_MessageBox.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrameImpl.h"
#include "xap_Frame.h"
#include <stdio.h>

/*****************************************************************/
XAP_Dialog * XAP_QNXDialog_MessageBox::static_constructor(XAP_DialogFactory * pFactory,
														 XAP_Dialog_Id id)
{
	XAP_QNXDialog_MessageBox * p = new XAP_QNXDialog_MessageBox(pFactory,id);
	return p;
}

XAP_QNXDialog_MessageBox::XAP_QNXDialog_MessageBox(XAP_DialogFactory * pDlgFactory,
												   XAP_Dialog_Id id)
	: XAP_Dialog_MessageBox(pDlgFactory,id)
{
}

XAP_QNXDialog_MessageBox::~XAP_QNXDialog_MessageBox(void)
{
}

/*****************************************************************/

void XAP_QNXDialog_MessageBox::runModal(XAP_Frame * pFrame)
{
	XAP_QNXFrameImpl * pQNXFrameImpl = (XAP_QNXFrameImpl*)pFrame->getFrameImpl();
	PtWidget_t *parent =	pQNXFrameImpl->getTopLevelWindow();	
	UT_ASSERT(parent);
	XAP_App *pApp =  pFrame->getApp();
	const char * szCaption = pApp->getApplicationTitleForTitleBar();

	int ret, def_button;
	const char *str1, *str2, *str3;

	// we get all our strings from the application string set
	const XAP_StringSet * pSS = pFrame->getApp()->getStringSet();
	UT_UTF8String s;

	PtSetParentWidget(parent);

	str1 = str2 = str3 = NULL;
	def_button = 1;
	switch (m_buttons)
	{
	case b_OC: // OK && Cancel
		pSS->getValueUTF8(XAP_STRING_ID_DLG_Cancel,s);
		str2 = g_strdup(s.utf8_str());
		if (m_defaultAnswer == a_CANCEL) {
			def_button = 2;
		}

	case b_O: // OK
		pSS->getValueUTF8(XAP_STRING_ID_DLG_OK,s);
		str1 = g_strdup(s.utf8_str());
		break;

	case b_YNC: // Yes && No && Cancel
		pSS->getValueUTF8(XAP_STRING_ID_DLG_Cancel,s);
		str3 = g_strdup(s.utf8_str());

	case b_YN: // Yes && No
		pSS->getValueUTF8(XAP_STRING_ID_DLG_QNXMB_Yes,s);
		str1 = g_strdup(s.utf8_str());
		pSS->getValueUTF8(XAP_STRING_ID_DLG_QNXMB_No,s);
		str2 = g_strdup(s.utf8_str());
		if (m_defaultAnswer == a_NO) {
			def_button = 2;
		}
		if (m_defaultAnswer == a_CANCEL) {
			def_button = 3;
		}
		break;

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}



	ret = PtAskQuestion(parent,
						szCaption,
						m_szMessage,
						NULL,
						str1,
						str2,
						str3,
						def_button);	

	switch (m_buttons)
	{
	case b_O: // OK
		m_answer = XAP_Dialog_MessageBox::a_OK;	
		break;

	case b_OC: // OK && Cancel
		if (ret == 1) {
			m_answer = XAP_Dialog_MessageBox::a_OK; 
		}
		else {
			m_answer = XAP_Dialog_MessageBox::a_CANCEL;	
		}
		break;

	case b_YN: // Yes && No
		if (ret == 1) {
			m_answer = XAP_Dialog_MessageBox::a_YES; 
		}
		else {
			m_answer = XAP_Dialog_MessageBox::a_NO;	
		}
		break;

	case b_YNC: // Yes && No && Cancel
		if (ret == 1) {
			m_answer = XAP_Dialog_MessageBox::a_YES; 
		}
		else if (ret == 2) {
			m_answer = XAP_Dialog_MessageBox::a_NO;	
		}
		else {
			m_answer = XAP_Dialog_MessageBox::a_CANCEL;	
		}
		break;

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}

FREEP(str1);
FREEP(str2);
FREEP(str3);
}

