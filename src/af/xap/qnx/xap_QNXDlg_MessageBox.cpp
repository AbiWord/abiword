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
#include "xap_QNXFrame.h"
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
	m_pQNXFrame = (XAP_QNXFrame *)pFrame;
	UT_ASSERT(m_pQNXFrame);
	XAP_QNXApp * pApp = (XAP_QNXApp *)m_pQNXFrame->getApp();
	UT_ASSERT(pApp);

	const char * szCaption = pApp->getApplicationTitleForTitleBar();


	int ret, def_button;
	const char *str1, *str2, *str3;

	printf("Run modal dialog box \n");

	// we get all our strings from the application string set
	const XAP_StringSet * pSS = pFrame->getApp()->getStringSet();

	str1 = str2 = str3 = NULL;
	def_button = 1;
	switch (m_buttons)
	{
	case b_OC: // OK && Cancel
		str2 = pSS->getValue(XAP_STRING_ID_DLG_Cancel);
		if (m_defaultAnswer == a_CANCEL) {
			def_button = 2;
		}

	case b_O: // OK
		str1 = pSS->getValue(XAP_STRING_ID_DLG_OK);
		break;

	case b_YNC: // Yes && No && Cancel
		str3 = pSS->getValue(XAP_STRING_ID_DLG_Cancel);

	case b_YN: // Yes && No
		str1 = pSS->getValue(XAP_STRING_ID_DLG_QNXMB_Yes);
		str2 = pSS->getValue(XAP_STRING_ID_DLG_QNXMB_No);
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

	// get top level window and it's GtkWidget *
	XAP_QNXFrame * frame = static_cast<XAP_QNXFrame *>(pFrame);
	UT_ASSERT(frame);
	PtWidget_t * parent = frame->getTopLevelWindow();
	UT_ASSERT(parent);

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
}

