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

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for BeOS dialogs,
// like centering them, measuring them, etc.
//#include "ut_dialogHelper.h"

#include "xap_App.h"
#include "xap_BeOSApp.h"
#include "xap_BeOSFrame.h"

#include "xap_Dialog_Id.h"
#include "xap_Dlg_WindowMore.h"
#include "xap_BeOSDlg_WindowMore.h"

/*****************************************************************/

XAP_Dialog * XAP_BeOSDialog_WindowMore::static_constructor(XAP_DialogFactory * pFactory,
													   XAP_Dialog_Id id)
{
	XAP_BeOSDialog_WindowMore * p = new XAP_BeOSDialog_WindowMore(pFactory,id);
	return p;
}

XAP_BeOSDialog_WindowMore::XAP_BeOSDialog_WindowMore(XAP_DialogFactory * pDlgFactory,
											   XAP_Dialog_Id id)
	: XAP_Dialog_WindowMore(pDlgFactory,id)
{
}

XAP_BeOSDialog_WindowMore::~XAP_BeOSDialog_WindowMore(void)
{
}


void XAP_BeOSDialog_WindowMore::runModal(XAP_Frame * pFrame)
{
	// NOTE: this work could be done in XP code
	// Initialize member so we know where we are now
	m_ndxSelFrame = m_pApp->findFrame(pFrame);
	UT_ASSERT(m_ndxSelFrame >= 0);

}

void XAP_BeOSDialog_WindowMore::event_OK(void)
{
	m_answer = XAP_Dialog_WindowMore::a_OK;
}

void XAP_BeOSDialog_WindowMore::event_Cancel(void)
{
	m_answer = XAP_Dialog_WindowMore::a_CANCEL;
}

void XAP_BeOSDialog_WindowMore::event_DoubleClick(void)
{
	event_OK();
}

void XAP_BeOSDialog_WindowMore::event_WindowDelete(void)
{
	m_answer = XAP_Dialog_WindowMore::a_CANCEL;	
}



