/* AbiWord
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

// TODO get rid of this (it's used for printf, which should be changed to
// UT_DEBUGMSG())
#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_Dialog_Id.h"
#include "xap_BeOSApp.h"
#include "xap_BeOSFrame.h"

#include "ap_Dialog_Replace.h"
#include "ap_BeOSDialog_Replace.h"

/*****************************************************************/
XAP_Dialog * AP_BeOSDialog_Replace::static_constructor(XAP_DialogFactory * pFactory,
													  XAP_Dialog_Id id)
{
	//AP_BeOSDialog_Replace * p = new AP_BeOSDialog_Replace(pFactory,id);
	//return p;
	return(NULL);
}

AP_BeOSDialog_Replace::AP_BeOSDialog_Replace(XAP_DialogFactory * pDlgFactory,
														   XAP_Dialog_Id id)
	: AP_Dialog_Replace(pDlgFactory,id)
{

	m_findString = NULL;
	m_replaceString = NULL;
    m_matchCase = UT_TRUE;
}

AP_BeOSDialog_Replace::~AP_BeOSDialog_Replace(void)
{
}

void AP_BeOSDialog_Replace::runModal(XAP_Frame * pFrame)
{
/*
	AP_Win32App * pWin32App = static_cast<AP_Win32App *>(pFrame->getApp());
	XAP_Win32Frame * pWin32Frame = static_cast<XAP_Win32Frame *>(pFrame);

	LPCTSTR lpTemplate = NULL;
	if (m_id == AP_DIALOG_ID_REPLACE)
		lpTemplate = MAKEINTRESOURCE(AP_RID_DIALOG_REPLACE);
	else if (m_id == AP_DIALOG_ID_FIND)
		lpTemplate = MAKEINTRESOURCE(AP_RID_DIALOG_FIND);
	else
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);	
		return;
	}

	setView(static_cast<FV_View *> (pFrame->getCurrentView()) );
	
	int result = DialogBoxParam(pWin32App->getInstance(),lpTemplate,
								pWin32Frame->getTopLevelWindow(),
								(DLGPROC)s_dlgProc,(LPARAM)this);
	UT_ASSERT((result != -1));
*/
}


