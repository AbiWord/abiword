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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#include <stdlib.h>
#include <stdio.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrameImpl.h"
#include "xap_Frame.h"

#include "xap_Strings.h"
#include "xap_Dialog_Id.h"
#include "xap_QNXDlg_Password.h"
#include "ut_qnxHelper.h"


/*****************************************************************/

int pass_validate( XAP_QNXDialog_Password *dlg,
              char const *password_entered )
{
dlg->SetPassword((char*)password_entered);
return Pt_PWD_ACCEPT;
}

void XAP_QNXDialog_Password::SetPassword(char* pass)
{
m_pass=g_strdup(pass);
}

XAP_Dialog * XAP_QNXDialog_Password::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	XAP_QNXDialog_Password * p = new XAP_QNXDialog_Password(pFactory,id);
	return p;
}

XAP_QNXDialog_Password::XAP_QNXDialog_Password(XAP_DialogFactory * pDlgFactory,
											   XAP_Dialog_Id id)
	: XAP_Dialog_Password(pDlgFactory,id)
{
m_pass = NULL;
}

XAP_QNXDialog_Password::~XAP_QNXDialog_Password(void)
{
}

void XAP_QNXDialog_Password::runModal(XAP_Frame * pFrame)
{	
	UT_ASSERT(pFrame);
	int pwdreturn;
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	UT_UTF8String s,s1;
	
	char **buttons=(char**)UT_calloc(2,sizeof(char*));
	pSS->getValueUTF8(XAP_STRING_ID_DLG_Cancel,s);
	buttons[0]= g_strdup((char*) s.utf8_str());
	pSS->getValueUTF8(XAP_STRING_ID_DLG_OK,s);
	buttons[1]= g_strdup((char*)s.utf8_str());
    
	XAP_QNXFrameImpl * pQNXFrameImpl = (XAP_QNXFrameImpl*)pFrame->getFrameImpl();
	PtWidget_t *parentWindow =	pQNXFrameImpl->getTopLevelWindow();	
	UT_ASSERT(parentWindow);

	pSS->getValueUTF8(XAP_STRING_ID_DLG_Password_Title,s);
	pSS->getValueUTF8(XAP_STRING_ID_DLG_Password_Title,s1);
	pwdreturn=PtPassword(parentWindow, /* Parent */
						 NULL, /* Location */
						 s.utf8_str(), /* Title*/
							NULL, /* Image*/
						 s1.utf8_str(), /* Msg */
							NULL, /* font*/
							(const char **)buttons, /* button strings */
							NULL, /* font*/
							NULL, /* font*/
							pass_validate,		/* fake validation.*/
							this, /* validate data.*/
							"*",
							Pt_CENTER|Pt_BLOCK_ALL);
	if(pwdreturn == Pt_PWD_CANCEL)
			FREEP(m_pass);
		  setAnswer(XAP_Dialog_Password::a_Cancel);
	if(pwdreturn == Pt_PWD_ACCEPT) {
		setPassword(m_pass);
		setAnswer(XAP_Dialog_Password::a_OK);
	}

	FREEP(buttons[0]);
	FREEP(buttons[1]);
	FREEP(buttons);
}

