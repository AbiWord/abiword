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
#include "xap_BeOSDlg_MessageBox.h"
#include "xap_BeOSApp.h"
#include "xap_BeOSFrame.h"
#include <string.h>

/*****************************************************************/
AP_Dialog * XAP_BeOSDialog_MessageBox::static_constructor(AP_DialogFactory * pFactory,
														 AP_Dialog_Id id)
{
	XAP_BeOSDialog_MessageBox * p = new XAP_BeOSDialog_MessageBox(pFactory,id);
	return p;
}

XAP_BeOSDialog_MessageBox::XAP_BeOSDialog_MessageBox(AP_DialogFactory * pDlgFactory,
												   AP_Dialog_Id id)
	: XAP_Dialog_MessageBox(pDlgFactory,id)
{
}

XAP_BeOSDialog_MessageBox::~XAP_BeOSDialog_MessageBox(void)
{
}

void XAP_BeOSDialog_MessageBox::runModal(XAP_Frame * pFrame)
{
	char msgs[3][20];
	printf("Called the message box \n");
	//Build a message box based on m_szMessage, m_buttons
	//returning m_answer
	strcpy(msgs[0], "");
	strcpy(msgs[1], "");
	strcpy(msgs[2], "");
	switch (m_buttons) {
	case b_OC:
		strcpy(msgs[1], "Cancel");
	case b_O:
		strcpy(msgs[0], "OK");
		break;
	case b_YNC:
		strcpy(msgs[2], "Cancel");
	case b_YN:
		strcpy(msgs[0], "Yes");
		strcpy(msgs[1], "No");
		break;
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}
	
	BAlert *alert = new BAlert("Abi Word -- Message", m_szMessage, 
                     (strlen(msgs[0])) ? msgs[0] : NULL,
                     (strlen(msgs[1])) ? msgs[1] : NULL,
                     (strlen(msgs[2])) ? msgs[2] : NULL);

   int32 button_index = alert->Go();
   printf("Returned an index %d %s\n", button_index, msgs[button_index]);
	if (strcmp(msgs[button_index], "Yes") == 0) 
		m_answer = a_YES;
	else if (strcmp(msgs[button_index], "No") == 0)
		m_answer = a_NO;
	else if (strcmp(msgs[button_index], "OK") == 0) 
		m_answer = a_OK;
	else if (strcmp(msgs[button_index], "Cancel") == 0) 
		m_answer = a_CANCEL;
}

//These break encapsulation
UT_Vector * XAP_BeOSDialog_MessageBox::_getBindingsVector()
{
	return &m_keyBindings;
}

void XAP_BeOSDialog_MessageBox::_setAnswer(XAP_Dialog_MessageBox::tAnswer answer)
{
	m_answer = answer;
}


